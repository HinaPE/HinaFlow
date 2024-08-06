#include "GAS_SolverPhiFlow.h"

/******************************************************************************
 *
 * HinaFlow fluid solver framework
 * Copyright 2024 Xayah Hina
 *
 * This program is free software, distributed under the terms of the
 * Mozilla Public License, Version 2.0
 * https://www.mozilla.org/en-US/MPL/2.0/
 *
 ******************************************************************************/


#include "common.h"

const SIM_DopDescription* GAS_SolverPhiFlow::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_DENSITY
    ACTIVATE_GAS_VELOCITY
    PARAMETER_INT(StartFrame, 1)
    PARAMETER_VECTOR_FLOAT_N(Resolution, 3, 100, 100, 100)
    PARAMETER_VECTOR_FLOAT_N(Size, 3, 1, 1, 1)
    PRMs.emplace_back();

    static SIM_DopDescription DESC(GEN_NODE,
                                   DOP_NAME,
                                   DOP_ENGLISH,
                                   DATANAME,
                                   classname(),
                                   PRMs.data());
    DESC.setDefaultUniqueDataName(UNIQUE_DATANAME);
    setGasDescription(DESC);
    return &DESC;
}

void WriteFieldPartial(SIM_RawField* TARGET, const std::vector<double>& SOURCE, const UT_JobInfo& info)
{
    UT_VoxelArrayIteratorF vit;
    vit.setArray(TARGET->fieldNC());
    vit.setCompressOnExit(true);
    vit.setPartialRange(info.job(), info.numJobs());

    for (vit.rewind(); !vit.atEnd(); vit.advance())
    {
        UT_Vector3I cell(vit.x(), vit.y(), vit.z());
        int idx = HinaFlow::TO_1D_IDX(cell, TARGET->getVoxelRes());
        vit.setValue(static_cast<float>(SOURCE[idx]));
    }
}

THREADED_METHOD2(, true, WriteField, SIM_RawField *, TARGET, const std::vector<double>&, SOURCE);

void WriteHoudiniField(SIM_RawField* TARGET, const std::vector<double>& SOURCE)
{
    const auto f = TARGET->field();
    auto voxels = f->getXRes() * f->getYRes() * f->getZRes();
    if (voxels != SOURCE.size())
    {
        printf("Error: Field size mismatch\n");
        printf("voxels: %d, size: %lld\n", voxels, SOURCE.size());
        return;
    }
    WriteField(TARGET, SOURCE);
}

bool GAS_SolverPhiFlow::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    const int frame = engine.getSimulationFrame(time);
    if (frame < getStartFrame())
        return true;

    SIM_ScalarField* D = getScalarField(obj, GAS_NAME_DENSITY);
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY);
    if (!D || !V)
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }


    if (frame == getStartFrame())
    {
        int resx = static_cast<int>(getResolution().x());
        int resy = static_cast<int>(getResolution().y());
        int resz = static_cast<int>(getResolution().z());
        int sizex = static_cast<int>(getSize().x());
        int sizey = static_cast<int>(getSize().y());
        int sizez = static_cast<int>(getSize().z());

        UT_WorkBuffer expr;
        expr.sprintf(R"(
from phi.torch.flow import *
size = (%d, %d, %d)
res = (%d, %d, %d)
velocity = StaggeredGrid(values=0, boundary=extrapolation.ZERO, bounds=Box(x=(-size[0]/2.0, size[0]/2.0), y=(-size[1]/2.0, size[1]/2.0), z=(-size[2]/2.0, size[2]/2.0)), resolution=spatial(x=res[0], y=res[1], z=res[2]))
smoke = CenteredGrid(values=0, boundary=extrapolation.ZERO_GRADIENT, bounds=Box(x=(-size[0]/2.0, size[0]/2.0), y=(-size[1]/2.0, size[1]/2.0), z=(-size[2]/2.0, size[2]/2.0)), resolution=spatial(x=res[0], y=res[1], z=res[2]))
INFLOW = resample(Sphere(x=0, y=0, z=0, radius=0.1), to=smoke, soft=True)
pressure = None

@jit_compile  # Only for PyTorch, TensorFlow and Jax
def step(v, s, p, dt=0.1):
    s = advect.mac_cormack(s, v, dt) + INFLOW
    buoyancy = resample(s * (0, 0.1, 0), to=v)
    v = advect.semi_lagrangian(v, v, dt) + buoyancy * dt
    v, p = fluid.make_incompressible(v, (), Solve('auto', 1e-2, x0=p))
    return v, s, p
)", sizex, sizey, sizez, resx, resy, resz);
        PYrunPythonStatementsAndExpectNoErrors(expr.buffer());
    }
    else
    {
        UT_WorkBuffer expr;
        PY_Result result;
        expr.sprintf(R"(
velocity, smoke, pressure = step(velocity, smoke, pressure)
smoke_output = smoke.data.native('x,y,z').cpu().numpy().flatten()
vc = velocity.at_centers()
velocityx_output = vc.vector['x'].data.native('x,y,z').cpu().detach().numpy().flatten()
velocityy_output = vc.vector['y'].data.native('x,y,z').cpu().detach().numpy().flatten()
velocityz_output = vc.vector['z'].data.native('x,y,z').cpu().detach().numpy().flatten()
)");
        PYrunPythonStatementsAndExpectNoErrors(expr.buffer());
        expr.sprintf("smoke_output.tolist()\n");
        result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
        if (result.myResultType != PY_Result::DOUBLE_ARRAY)
        {
            printf("Error: %s\n", result.myErrValue.buffer());
            return false;
        }
        WriteHoudiniField(D->getField(), result.myDoubleArray);

        expr.sprintf("velocityx_output.tolist()\n");
        result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
        if (result.myResultType != PY_Result::DOUBLE_ARRAY)
        {
            printf("Error: %s\n", result.myErrValue.buffer());
            return false;
        }
        WriteHoudiniField(V->getXField(), result.myDoubleArray);

        expr.sprintf("velocityy_output.tolist()\n");
        result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
        if (result.myResultType != PY_Result::DOUBLE_ARRAY)
        {
            printf("Error: %s\n", result.myErrValue.buffer());
            return false;
        }
        WriteHoudiniField(V->getYField(), result.myDoubleArray);

        expr.sprintf("velocityz_output.tolist()\n");
        result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
        if (result.myResultType != PY_Result::DOUBLE_ARRAY)
        {
            printf("Error: %s\n", result.myErrValue.buffer());
            return false;
        }
        WriteHoudiniField(V->getZField(), result.myDoubleArray);
    }

    return true;
}
