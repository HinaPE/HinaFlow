#include "GAS_PhiFlowSmoke2D.h"

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
#include "python/phiflow_smoke.h"

const SIM_DopDescription* GAS_PhiFlowSmoke2D::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_DENSITY
    ACTIVATE_GAS_VELOCITY
    ACTIVATE_GAS_PRESSURE
    PARAMETER_INT(StartFrame, 1)
    PARAMETER_VECTOR_INT_N(Resolution, 2, 100, 100)
    PARAMETER_VECTOR_FLOAT_N(Size, 2, 1, 1)
    PARAMETER_VECTOR_FLOAT_N(Center, 2, 0, 0)
    PARAMETER_BOOL(DebugMode, false)
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

bool GAS_PhiFlowSmoke2D::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    const int frame = engine.getSimulationFrame(time);
    if (frame < getStartFrame())
        return true;

    SIM_ScalarField* D = getScalarField(obj, GAS_NAME_DENSITY);
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY);
    SIM_ScalarField* P = getScalarField(obj, GAS_NAME_PRESSURE);
    if (!D)
    {
        addError(obj, SIM_MESSAGE, "Missing density fields", UT_ERROR_FATAL);
        return false;
    }

    return true;
}
