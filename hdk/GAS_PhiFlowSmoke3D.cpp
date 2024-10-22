#include "GAS_PhiFlowSmoke3D.h"

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
#include "../src/phiflow_smoke.h"

const SIM_DopDescription* GAS_PhiFlowSmoke3D::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_DENSITY
    ACTIVATE_GAS_VELOCITY
    ACTIVATE_GAS_PRESSURE
    ACTIVATE_GAS_SOURCE
    PARAMETER_INT(StartFrame, 1)
    PARAMETER_VECTOR_INT_N(Resolution, 3, 100, 100, 100)
    PARAMETER_VECTOR_FLOAT_N(Size, 3, 1, 1, 1)
    PARAMETER_VECTOR_FLOAT_N(Center, 3, 0, 0, 0)
    PARAMETER_BOOL(DebugMode, false)

    PARAMETER_PATH(SourceVolumePath, "")
    PARAMETER_INT(SourcePrimIndex, 0)
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

bool GAS_PhiFlowSmoke3D::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    const int frame = engine.getSimulationFrame(time);
    if (frame < getStartFrame())
        return true;

    SIM_ScalarField* D = getScalarField(obj, GAS_NAME_DENSITY);
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY);
    SIM_ScalarField* P = getScalarField(obj, GAS_NAME_PRESSURE);
    SIM_ScalarField* S = getScalarField(obj, GAS_NAME_SOURCE);
    if (!D)
    {
        addError(obj, SIM_MESSAGE, "Missing density fields", UT_ERROR_FATAL);
        return false;
    }

    if (frame == getStartFrame())
    {
        int resx = static_cast<int>(getResolution().x());
        int resy = static_cast<int>(getResolution().y());
        int resz = static_cast<int>(getResolution().z());
        float sizex = static_cast<float>(getSize().x());
        float sizey = static_cast<float>(getSize().y());
        float sizez = static_cast<float>(getSize().z());
        float centerx = static_cast<float>(getCenter().x());
        float centery = static_cast<float>(getCenter().y());
        float centerz = static_cast<float>(getCenter().z());


        HinaFlow::Python::PhiFlowSmoke::DebugMode(getDebugMode());
        HinaFlow::Python::PhiFlowSmoke::ImportHou();
        HinaFlow::Python::PhiFlowSmoke::ImportPhiFlow(HinaFlow::Python::PhiFlowSmoke::Backend::Torch);
        HinaFlow::Python::PhiFlowSmoke::CreateFromExternalVolume("SOURCE", "Houdini_SOURCE", getSourceVolumePath().toStdString(), static_cast<int>(getSourcePrimIndex()));
        HinaFlow::Python::PhiFlowSmoke::CreateScalarField(
            "DENSITY",
            {resx, resy, resz},
            {sizex, sizey, sizez},
            {centerx, centery, centerz},
            HinaFlow::Python::PhiFlowSmoke::Extrapolation::Neumann,
            0);
        HinaFlow::Python::PhiFlowSmoke::CreateVectorField(
            "VELOCITY",
            {resx, resy, resz},
            {sizex, sizey, sizez},
            {centerx, centery, centerz},
            HinaFlow::Python::PhiFlowSmoke::Extrapolation::Dirichlet,
            0);
        // HinaFlow::Python::PhiFlowSmoke::CreateSphereInflow("INFLOW", "DENSITY", {0, 0, 0}, 0.1);
        HinaFlow::Python::PhiFlowSmoke::CreateInflowFluids("INFLOW", "DENSITY", "SOURCE");
        HinaFlow::Python::PhiFlowSmoke::FetchScalarField("DENSITY", D);
        if (S)
            HinaFlow::Python::PhiFlowSmoke::FetchScalarField("INFLOW", S);

        UT_WorkBuffer expr;
        expr.sprintf(R"(
@jit_compile  # Only for PyTorch, TensorFlow and Jax
def step(v, s, p, src, dt=%f):
    s = advect.mac_cormack(s, v, dt) + src
    buoyancy = resample(s * (0, 0.1, 0), to=v)
    v = advect.semi_lagrangian(v, v, dt) + buoyancy * dt
    v, p = fluid.make_incompressible(v, (), Solve('auto', 1e-2, x0=p))
    return v, s, p
)", static_cast<float>(timestep));
        HinaFlow::Python::PhiFlowSmoke::CompileFunction(expr);
    }
    else
    {
        HinaFlow::Python::PhiFlowSmoke::RunFunction("step", "VELOCITY, DENSITY, None, INFLOW", "VELOCITY, DENSITY, PRESSURE");
        HinaFlow::Python::PhiFlowSmoke::FetchScalarField("DENSITY", D);
        if (V)
            HinaFlow::Python::PhiFlowSmoke::FetchVectorField("VELOCITY", V);
        if (P)
            HinaFlow::Python::PhiFlowSmoke::FetchScalarField("PRESSURE", P);
    }

    return true;
}
