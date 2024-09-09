#include "GAS_ReconstructDensity.h"

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
#include "src/tomography.h"

const SIM_DopDescription* GAS_ReconstructDensity::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_DENSITY
    ACTIVATE_GAS_VECTOR_FIELD_1
    ACTIVATE_GAS_VECTOR_FIELD_2
    ACTIVATE_GAS_VECTOR_FIELD_3
    ACTIVATE_GAS_VECTOR_FIELD_4
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

bool GAS_ReconstructDensity::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    SIM_ScalarField* TARGET = getScalarField(obj, GAS_NAME_DENSITY);
    const SIM_VectorField* VIEW1 = getConstVectorField(obj, GAS_NAME_VECTOR_FIELD_1);
    const SIM_VectorField* VIEW2 = getConstVectorField(obj, GAS_NAME_VECTOR_FIELD_2);
    const SIM_VectorField* VIEW3 = getConstVectorField(obj, GAS_NAME_VECTOR_FIELD_3);
    const SIM_VectorField* VIEW4 = getConstVectorField(obj, GAS_NAME_VECTOR_FIELD_4);

    if (!HinaFlow::CHECK_NOT_NULL(TARGET, VIEW1))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    HinaFlow::Tomography::Input input;
    HinaFlow::Tomography::Param param;
    HinaFlow::Tomography::Result result;
    result.TARGET = TARGET;
    {
        const std::string& path = VIEW1->getPositionPath().toStdString();
        const size_t lsp = path.find_last_of('/');
        SIM_Position* pos = SIM_DATA_GET(*obj, lsp != std::string::npos?path.substr(lsp + 1).c_str():path.c_str(), SIM_Position);
        pos->getPosition(param.pos1);
        input.VIEW1 = VIEW1;
    }

    if (VIEW2)
    {
        const std::string& path = VIEW2->getPositionPath().toStdString();
        const size_t lsp = path.find_last_of('/');
        SIM_Position* pos = SIM_DATA_GET(*obj, lsp != std::string::npos?path.substr(lsp + 1).c_str():path.c_str(), SIM_Position);
        pos->getPosition(param.pos2);
        input.VIEW2 = VIEW2;
    }

    if (VIEW3)
    {
        const std::string& path = VIEW3->getPositionPath().toStdString();
        const size_t lsp = path.find_last_of('/');
        SIM_Position* pos = SIM_DATA_GET(*obj, lsp != std::string::npos?path.substr(lsp + 1).c_str():path.c_str(), SIM_Position);
        pos->getPosition(param.pos3);
        input.VIEW3 = VIEW3;
    }

    if (VIEW4)
    {
        const std::string& path = VIEW4->getPositionPath().toStdString();
        const size_t lsp = path.find_last_of('/');
        SIM_Position* pos = SIM_DATA_GET(*obj, lsp != std::string::npos?path.substr(lsp + 1).c_str():path.c_str(), SIM_Position);
        pos->getPosition(param.pos4);
        input.VIEW4 = VIEW4;
    }

    HinaFlow::Tomography::Solve(input, param, result);

    return true;
}
