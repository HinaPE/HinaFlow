#include "GAS_SolverFLIP.h"

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
#include "src/flip.h"

const SIM_DopDescription* GAS_SolverFLIP::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_GEOMETRY
    ACTIVATE_GAS_VELOCITY
    ACTIVATE_GAS_DIVERGENCE
    ACTIVATE_GAS_PRESSURE
    ACTIVATE_GAS_STENCIL
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

bool GAS_SolverFLIP::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    SIM_GeometryCopy* G = getGeometryCopy(obj, GAS_NAME_GEOMETRY);
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY); // required
    SIM_ScalarField* DIV = getScalarField(obj, GAS_NAME_DIVERGENCE); // optional
    SIM_ScalarField* PRS = getScalarField(obj, GAS_NAME_PRESSURE); // required
    SIM_IndexField* MARKER = getIndexField(obj, GAS_NAME_STENCIL); // required

    if (!HinaFlow::CHECK_NOT_NULL(G, V, PRS, MARKER))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    if (!HinaFlow::CHECK_THE_SAME_DIMENSION(V, PRS, MARKER))
    {
        addError(obj, SIM_MESSAGE, "GAS fields have different dimensions", UT_ERROR_FATAL);
        return false;
    }

    if (!HinaFlow::CHECK_IS_FACE_SAMPLED(V))
    {
        addError(obj, SIM_MESSAGE, "Velocity field is not face sampled", UT_ERROR_FATAL);
        return false;
    }

    return true;
}
