#include "GAS_ReadNPZFiles.h"

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

const SIM_DopDescription* GAS_ReadNPZFiles::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_FIELD
    PARAMETER_INT(StartFrame, 1)
    PARAMETER_INT(MaxFrame, 150)
    PARAMETER_STRING(FilePath, "")
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
        const auto idx = HinaFlow::TO_1D_IDX(cell, res);
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
        const auto idx = HinaFlow::TO_1D_IDX(cell, res);
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

bool GAS_ReadNPZFiles::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    const int frame = engine.getSimulationFrame(time);
    if (frame < getStartFrame() || frame > getMaxFrame())
        return true;

    SIM_ScalarField* F = getScalarField(obj, GAS_NAME_FIELD);
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY);
    if (!F && !V)
    {
        addError(obj, SIM_MESSAGE, "Missing fields", UT_ERROR_FATAL);
        return false;
    }

    UT_WorkBuffer expr;
    if (frame == getStartFrame())
    {
        expr.sprintf(R"(
import numpy as np
import os
import re
def get_file_path(file_path, frame):
    directory = os.path.dirname(file_path)
    file_name = os.path.basename(file_path)
    match = re.search(r'(\D+)(\d+)(\.\w+)', file_name)

    if match:
        prefix = match.group(1)
        number = match.group(2)
        suffix = match.group(3)

        new_file_name = f"{prefix}{frame:0{len(number)}d}{suffix}"

        new_file_path = os.path.join(directory, new_file_name)
        new_file_path = new_file_path.replace('\\', '/')

        return new_file_path
    else:
        raise ValueError("format not recognized")
)");
        PYrunPythonStatementsAndExpectNoErrors(expr.buffer());
    }


    if (F)
    {
        expr.sprintf(R"(
res = np.squeeze(np.load(get_file_path('%s', %d))['data'])
)", getFilePath().toStdString().c_str(), frame);
        PYrunPythonStatementsAndExpectNoErrors(expr.buffer());

        expr.sprintf(R"(
res.shape
)");
        {
            const PY_Result result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::INT_ARRAY);
            if (result.myResultType != PY_Result::INT_ARRAY)
            {
                printf("Error: %s\n", result.myErrValue.buffer());
                return false;
            }
            const auto shape = result.myIntArray;
            F->setSize({static_cast<float>(shape[2]), static_cast<float>(shape[1]), static_cast<float>(shape[0])});
            F->setDivisions({static_cast<float>(shape[2]), static_cast<float>(shape[1]), static_cast<float>(shape[0])});
        }

        {
            expr.sprintf(R"(
res.flatten().tolist()
)");
            const PY_Result result = PYrunPythonExpressionAndExpectNoErrors(expr.buffer(), PY_Result::DOUBLE_ARRAY);
            if (result.myResultType != PY_Result::DOUBLE_ARRAY)
            {
                printf("Error: %s\n", result.myErrValue.buffer());
                return false;
            }
            WriteHoudiniField(F->getField(), result.myDoubleArray);
        }
    }
    if (V)
    {
    }

    return true;
}
