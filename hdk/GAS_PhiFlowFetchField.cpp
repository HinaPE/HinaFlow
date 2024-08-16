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
#include "python/phiflow_smoke.h"

const SIM_DopDescription* GAS_PhiFlowFetchField::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_FIELD
    PARAMETER_STRING(Target, "")
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
    if (const int frame = engine.getSimulationFrame(time); frame < getStartFrame())
        return true;

    SIM_ScalarField* F = getScalarField(obj, GAS_NAME_FIELD);
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_FIELD);
    if (!F && !V)
    {
        addError(obj, SIM_MESSAGE, "Missing fields", UT_ERROR_FATAL);
        return false;
    }

    const std::string& name = getTarget().toStdString();
    const bool is2D = F == nullptr ? V->getTwoDField() : F->getTwoDField();

    if (F)
    {
        F->setSize(HinaFlow::Python::PhiFlowSmoke::FetchFieldSize(name));
        F->setDivisions(HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution(name));
        F->setTwoDField(is2D);
        if (is2D)
            HinaFlow::Python::PhiFlowSmoke::FetchScalarField2D(name, F);
        else
            HinaFlow::Python::PhiFlowSmoke::FetchScalarField(name, F);
    }

    if (V)
    {
        V->setSize(HinaFlow::Python::PhiFlowSmoke::FetchFieldSize(name));
        V->setDivisions(HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution(name));
        V->setTwoDField(is2D);
        if (is2D)
            HinaFlow::Python::PhiFlowSmoke::FetchVectorField2D(name, V);
        else
            HinaFlow::Python::PhiFlowSmoke::FetchVectorField(name, V);
    }

    return true;
}
