#include "GAS_PhiFlowFetchField.h"

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

const SIM_DopDescription* GAS_PhiFlowFetchField::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
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

bool GAS_PhiFlowFetchField::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    const int frame = engine.getSimulationFrame(time);
    if (frame < getStartFrame())
        return true;

    SIM_VectorField* V1 = getVectorField(obj, GAS_NAME_VECTOR_FIELD_1);
    SIM_VectorField* V2 = getVectorField(obj, GAS_NAME_VECTOR_FIELD_2);
    SIM_VectorField* V3 = getVectorField(obj, GAS_NAME_VECTOR_FIELD_3);
    SIM_VectorField* V4 = getVectorField(obj, GAS_NAME_VECTOR_FIELD_4);
    if (!V1 || !V2 || !V3 || !V4)
    {
        addError(obj, SIM_MESSAGE, "Missing fields", UT_ERROR_FATAL);
        return false;
    }


    return true;
}
