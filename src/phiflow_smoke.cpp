#include "phiflow_smoke.h"

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


#include "../common.h"

#include <filesystem>
#include <fstream>

static bool DEBUG_MODE = false;
static const std::string DEBUG_PY_FILENAME = "debug_phiflow.py";

#define RECORD_EXPRESSION(expr) \
    if (DEBUG_MODE) \
    { \
        std::ofstream file(DEBUG_PY_FILENAME, std::ios::app); \
        file << expr << "\n"; \
        file.close(); \
    }

namespace HinaFlow::Internal::Python::PhiFlowSmoke
{
    void WriteField3DPartial(SIM_RawField* TARGET, const std::vector<double>& SOURCE, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(TARGET->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        const UT_Vector3I res = TARGET->getVoxelRes();
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const auto idx = TO_1D_IDX(cell, res);
            vit.setValue(static_cast<float>(SOURCE[idx]));
        }
    }

    THREADED_METHOD2(, true, WriteField3D, SIM_RawField *, TARGET, const std::vector<double>&, SOURCE);

    void WriteField2DPartial(SIM_RawField* TARGET, const std::vector<double>& SOURCE, const int AXIS1, const int AXIS2, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(TARGET->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        const UT_Vector2I res = {TARGET->getVoxelRes()[AXIS1], TARGET->getVoxelRes()[AXIS2]};
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            UT_Vector2I cell(vit.idx(AXIS1), vit.idx(AXIS2));
            const auto idx = TO_1D_IDX(cell, res);
            vit.setValue(static_cast<float>(SOURCE[idx]));
        }
    }

    THREADED_METHOD4(, true, WriteField2D, SIM_RawField *, TARGET, const std::vector<double>&, SOURCE, const int, AXIS1, const int, AXIS2);

    void WriteHoudiniField(SIM_RawField* TARGET, const std::vector<double>& SOURCE)
    {
        if (TARGET->field()->numVoxels() != SOURCE.size())
        {
            printf("Error: Field size mismatch\n");
            printf("voxels: %lld, size: %lld\n", TARGET->field()->numVoxels(), SOURCE.size());
            return;
        }
        if (TARGET->getXRes() == 1)
            WriteField2D(TARGET, SOURCE, 2, 1);
        else if (TARGET->getYRes() == 1)
            WriteField2D(TARGET, SOURCE, 2, 0);
        else if (TARGET->getZRes() == 1)
            WriteField2D(TARGET, SOURCE, 1, 0);
        else
            WriteField3D(TARGET, SOURCE);
    }
}

void HinaFlow::Python::PhiFlowSmoke::ImportHou()
{
    UT_WorkBuffer expr;
    expr.sprintf(R"(
import hou
)");
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr);
}

void HinaFlow::Python::PhiFlowSmoke::DebugMode(const bool enable)
{
    DEBUG_MODE = enable;
    if (enable)
    {
        std::ofstream file(DEBUG_PY_FILENAME, std::ios::trunc); // create & clear file
        file << "\n# >>> This file is auto generated by HinaFlow <<< #\n" << "\n";
        printf("Debug file created at: %ls\n", std::filesystem::absolute(DEBUG_PY_FILENAME).c_str());
    }
}

void HinaFlow::Python::PhiFlowSmoke::ImportPhiFlow(const Backend& backend)
{
    UT_WorkBuffer expr;
    switch (backend)
    {
    case Backend::CPU:
        expr.sprintf(R"(
from phi.flow import *
)");
        break;
    case Backend::Torch:
        expr.sprintf(R"(
from phi.torch.flow import *
)");
        break;
    case Backend::JAX:
        expr.sprintf(R"(
from phi.jax.flow import *
)");
        break;
    case Backend::TensorFlow:
        expr.sprintf(R"(
from phi.tf.flow import *
)");
        break;
    default:
        throw std::runtime_error("Invalid backend");
    }
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr);
}

void HinaFlow::Python::PhiFlowSmoke::CompileFunction(const UT_WorkBuffer& expr)
{
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());
    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::RunFunction(const std::string& func, const std::string& args, const std::string& res)
{
    UT_WorkBuffer expr;
    expr.sprintf(R"(
%s = %s(%s)
)", res.c_str(), func.c_str(), args.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::CreateScalarField(const std::string& name, const UT_Vector3i& resolution, const UT_Vector3& size, const UT_Vector3& center, const Extrapolation& extrapolation, const float init_value)
{
    std::string extrapolation_str;
    switch (extrapolation)
    {
    case Extrapolation::Dirichlet:
        extrapolation_str = "extrapolation.ZERO";
        break;
    case Extrapolation::Neumann:
        extrapolation_str = "extrapolation.ZERO_GRADIENT";
        break;
    default:
        throw std::runtime_error("Invalid extrapolation");
    }
    UT_WorkBuffer expr;
    expr.sprintf(R"(
center = (%f, %f, %f)
size = (%f, %f, %f)
res = (%d, %d, %d)
%s = CenteredGrid(values=0, boundary=%s, bounds=Box(x=(-size[0]/2.0 + center[0], size[0]/2.0 + center[0]), y=(-size[1]/2.0 + center[1], size[1]/2.0 + center[1]), z=(-size[2]/2.0 + center[2], size[2]/2.0 + center[2])), resolution=spatial(x=res[0], y=res[1], z=res[2]))
)", center[0], center[1], center[2], size[0], size[1], size[2], resolution[0], resolution[1], resolution[2], name.c_str(), extrapolation_str.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::CreateVectorField(const std::string& name, const UT_Vector3i& resolution, const UT_Vector3& size, const UT_Vector3& center, const Extrapolation& extrapolation, const float init_value)
{
    std::string extrapolation_str;
    switch (extrapolation)
    {
    case Extrapolation::Dirichlet:
        extrapolation_str = "extrapolation.ZERO";
        break;
    case Extrapolation::Neumann:
        extrapolation_str = "extrapolation.ZERO_GRADIENT";
        break;
    default:
        throw std::runtime_error("Invalid extrapolation");
    }
    UT_WorkBuffer expr;
    expr.sprintf(R"(
center = (%f, %f, %f)
size = (%f, %f, %f)
res = (%d, %d, %d)
%s = StaggeredGrid(values=0, boundary=%s, bounds=Box(x=(-size[0]/2.0 + center[0], size[0]/2.0 + center[0]), y=(-size[1]/2.0 + center[1], size[1]/2.0 + center[1]), z=(-size[2]/2.0 + center[2], size[2]/2.0 + center[2])), resolution=spatial(x=res[0], y=res[1], z=res[2]))
)", center[0], center[1], center[2], size[0], size[1], size[2], resolution[0], resolution[1], resolution[2], name.c_str(), extrapolation_str.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::CreateScalarField2D(const std::string& name, const UT_Vector2i& resolution, const UT_Vector2& size, const UT_Vector2& center, const Extrapolation& extrapolation, const float init_value)
{
    std::string extrapolation_str;
    switch (extrapolation)
    {
    case Extrapolation::Dirichlet:
        extrapolation_str = "extrapolation.ZERO";
        break;
    case Extrapolation::Neumann:
        extrapolation_str = "extrapolation.ZERO_GRADIENT";
        break;
    default:
        throw std::runtime_error("Invalid extrapolation");
    }
    UT_WorkBuffer expr;
    expr.sprintf(R"(
center = (%f, %f)
size = (%f, %f)
res = (%d, %d)
%s = CenteredGrid(values=0, boundary=%s, bounds=Box(x=(-size[0]/2.0 + center[0], size[0]/2.0 + center[0]), y=(-size[1]/2.0 + center[1], size[1]/2.0 + center[1])), resolution=spatial(x=res[0], y=res[1]))
)", center[0], center[1], size[0], size[1], resolution[0], resolution[1], name.c_str(), extrapolation_str.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::CreateVectorField2D(const std::string& name, const UT_Vector2i& resolution, const UT_Vector2& size, const UT_Vector2& center, const Extrapolation& extrapolation, const float init_value)
{
    std::string extrapolation_str;
    switch (extrapolation)
    {
    case Extrapolation::Dirichlet:
        extrapolation_str = "extrapolation.ZERO";
        break;
    case Extrapolation::Neumann:
        extrapolation_str = "extrapolation.ZERO_GRADIENT";
        break;
    default:
        throw std::runtime_error("Invalid extrapolation");
    }
    UT_WorkBuffer expr;
    expr.sprintf(R"(
center = (%f, %f)
size = (%f, %f)
res = (%d, %d)
%s = StaggeredGrid(values=0, boundary=%s, bounds=Box(x=(-size[0]/2.0 + center[0], size[0]/2.0 + center[0]), y=(-size[1]/2.0 + center[1], size[1]/2.0 + center[1])), resolution=spatial(x=res[0], y=res[1]))
)", center[0], center[1], size[0], size[1], resolution[0], resolution[1], name.c_str(), extrapolation_str.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::CreateFromExternalVolume(const std::string& name, const std::string& houdini_name, const std::string& path, const int prim_idx)
{
    UT_WorkBuffer expr;
    expr.sprintf(R"(
node = hou.node("%s")
geo = node.geometry()
%s = geo.prim(%d)
res = %s.resolution()
bbox = %s.boundingBox()
center = bbox.center()
lc = bbox.minvec()
uc = bbox.maxvec()

tmp = np.zeros((res[0], res[1], res[2]))
for i in range(res[0]):
    for j in range(res[1]):
        for k in range(res[2]):
            tmp[i,j,k] = %s.voxel((i,j,k))

%s = CenteredGrid(values=tensor(tmp, spatial('x,y,z')), boundary=extrapolation.ZERO, bounds=Box(x=(lc[0] + center[0], uc[0] + center[0]), y=(lc[1] + center[1], uc[1] + center[1]), z=(lc[2] + center[2], uc[2] + center[2])), resolution=spatial(x=res[0], y=res[1], z=res[2]))
)", path.c_str(), houdini_name.c_str(), prim_idx, houdini_name.c_str(), houdini_name.c_str(), houdini_name.c_str(), name.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::CreateSphereInflow(const std::string& name, const std::string& match_field, const UT_Vector3& center, const float radius)
{
    UT_WorkBuffer expr;
    expr.sprintf(R"(
%s = resample(Sphere(x=%f, y=%f, z=%f, radius=%f), to=%s, soft=True)
)", name.c_str(), center[0], center[1], center[2], radius, match_field.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::CreateSphereInflow2D(const std::string& name, const std::string& match_field, const UT_Vector2& center, const float radius)
{
    UT_WorkBuffer expr;
    expr.sprintf(R"(
%s = resample(Sphere(x=%f, y=%f, radius=%f), to=%s, soft=True)
)", name.c_str(), center[0], center[1], radius, match_field.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::CreateInflowFluids(const std::string& name, const std::string& target, const std::string& source)
{
    UT_WorkBuffer expr;
    expr.sprintf(R"(
%s = resample(value=%s, to=%s)
)", name.c_str(), source.c_str(), target.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    RECORD_EXPRESSION(expr)
}


void HinaFlow::Python::PhiFlowSmoke::FetchScalarField(const std::string& name, SIM_ScalarField* FIELD, const std::string& batch, const int batch_num)
{
    UT_WorkBuffer expr;
    if (batch.empty())
    {
        expr.sprintf(R"(
%s.data.native('x,y,z').cpu().detach().numpy().flatten().tolist()
)", name.c_str());
    }
    else
    {
        expr.sprintf(R"(
%s.%s[%d].data.native('x,y,z').cpu().detach().numpy().flatten().tolist()
)", name.c_str(), batch.c_str(), batch_num);
    }
    const PY_Result result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
    if (result.myResultType != PY_Result::DOUBLE_ARRAY)
    {
        printf("Error: %s\n", result.myErrValue.buffer());
        return;
    }
    Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getField(), result.myDoubleArray);

    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::FetchVectorField(const std::string& name, SIM_VectorField* FIELD, const std::string& batch, const int batch_num)
{
    UT_WorkBuffer expr;
    PY_Result result;

    if (batch.empty())
    {
        expr.sprintf(R"(
_vc = %s.at_centers()
)", name.c_str());
    }
    else
    {
        expr.sprintf(R"(
_vc = %s.%s[%d].at_centers()
)", name.c_str(), batch.c_str(), batch_num);
    }
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());
    RECORD_EXPRESSION(expr)

    expr.sprintf(R"(
_vc.vector['x'].data.native('x,y,z').cpu().detach().numpy().flatten().tolist()
)");
    result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
    if (result.myResultType != PY_Result::DOUBLE_ARRAY)
    {
        printf("Error: %s\n", result.myErrValue.buffer());
        return;
    }
    Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getXField(), result.myDoubleArray);
    RECORD_EXPRESSION(expr)


    expr.sprintf(R"(
_vc.vector['y'].data.native('x,y,z').cpu().detach().numpy().flatten().tolist()
)");
    result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
    if (result.myResultType != PY_Result::DOUBLE_ARRAY)
    {
        printf("Error: %s\n", result.myErrValue.buffer());
        return;
    }
    Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getYField(), result.myDoubleArray);
    RECORD_EXPRESSION(expr)


    expr.sprintf(R"(
_vc.vector['z'].data.native('x,y,z').cpu().detach().numpy().flatten().tolist()
)");
    result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
    if (result.myResultType != PY_Result::DOUBLE_ARRAY)
    {
        printf("Error: %s\n", result.myErrValue.buffer());
        return;
    }
    Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getZField(), result.myDoubleArray);
    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::FetchScalarField2D(const std::string& name, SIM_ScalarField* FIELD, const std::string& batch, const int batch_num)
{
    UT_WorkBuffer expr;
    if (batch.empty())
    {
        expr.sprintf(R"(
%s.data.native('x,y').cpu().detach().numpy().flatten().tolist()
)", name.c_str());
    }
    else
    {
        expr.sprintf(R"(
%s.%s[%d].data.native('x,y').cpu().detach().numpy().flatten().tolist()
)", name.c_str(), batch.c_str(), batch_num);
    }
    const PY_Result result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
    if (result.myResultType != PY_Result::DOUBLE_ARRAY)
    {
        printf("Error: %s\n", result.myErrValue.buffer());
        return;
    }
    Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getField(), result.myDoubleArray);
    RECORD_EXPRESSION(expr)
}

void HinaFlow::Python::PhiFlowSmoke::FetchVectorField2D(const std::string& name, SIM_VectorField* FIELD, const std::string& batch, const int batch_num)
{
    UT_WorkBuffer expr;
    PY_Result result;

    if (batch.empty())
    {
        expr.sprintf(R"(
_vc = %s.at_centers()
)", name.c_str());
    }
    else
    {
        expr.sprintf(R"(
_vc = %s.%s[%d].at_centers()
)", name.c_str(), batch.c_str(), batch_num);
    }
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());
    RECORD_EXPRESSION(expr)

    expr.sprintf(R"(
_vc.vector['x'].data.native('x,y').cpu().detach().numpy().flatten().tolist()
)");
    result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
    if (result.myResultType != PY_Result::DOUBLE_ARRAY)
    {
        printf("Error: %s\n", result.myErrValue.buffer());
        return;
    }
    if (FIELD->getTotalVoxelRes().x() == 1)
        Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getYField(), result.myDoubleArray);
    else
        Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getXField(), result.myDoubleArray);
    RECORD_EXPRESSION(expr)


    expr.sprintf(R"(
_vc.vector['y'].data.native('x,y').cpu().detach().numpy().flatten().tolist()
)");
    result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
    if (result.myResultType != PY_Result::DOUBLE_ARRAY)
    {
        printf("Error: %s\n", result.myErrValue.buffer());
        return;
    }
    if (FIELD->getTotalVoxelRes().y() == 1)
        Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getZField(), result.myDoubleArray);
    else
        Internal::Python::PhiFlowSmoke::WriteHoudiniField(FIELD->getYField(), result.myDoubleArray);
    RECORD_EXPRESSION(expr)
}

UT_Vector3 HinaFlow::Python::PhiFlowSmoke::FetchFieldSize(const std::string& name)
{
    UT_WorkBuffer expr;
    PY_Result result;
    expr.sprintf(R"(
size = %s.resolution.shape.size
res = [1, 1, 1]
res[0] = int(%s.bounds.size[0])
res[1] = int(%s.bounds.size[1])
if size == 3:
    res[2] = int(%s.bounds.size[2])
)", name.c_str(), name.c_str(), name.c_str(), name.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    expr.sprintf(R"(
res
)");
    result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
    return {static_cast<float>(result.myDoubleArray[0]), static_cast<float>(result.myDoubleArray[1]), static_cast<float>(result.myDoubleArray[2])};
}

UT_Vector3I HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution(const std::string& name)
{
    UT_WorkBuffer expr;
    PY_Result result;
    expr.sprintf(R"(
size = %s.resolution.shape.size
res = [1, 1, 1]
res[0] = int(%s.resolution[0])
res[1] = int(%s.resolution[1])
if size == 3:
    res[2] = int(%s.resolution[2])
)", name.c_str(), name.c_str(), name.c_str(), name.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

    expr.sprintf(R"(
res
)");
    result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::INT_ARRAY);
    return {result.myIntArray[0], result.myIntArray[1], result.myIntArray[2]};
}

void HinaFlow::Python::PhiFlowSmoke::DebugHoudiniVolume(const std::string& name)
{
    UT_WorkBuffer expr;
    expr.sprintf(R"(
print(%s.resolution())
)", name.c_str());
    PYrunPythonStatementsAndExpectNoErrors(expr.buffer());
}
