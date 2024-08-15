#include "GAS_PhiFlowVisulizeField.h"

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

const SIM_DopDescription* GAS_PhiFlowVisulizeField::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_FIELD
    ACTIVATE_GAS_VELOCITY
    PARAMETER_INT(StartFrame, 1)
    PARAMETER_STRING(TargetFieldName, "")
    PARAMETER_STRING(Code, "")
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

bool GAS_PhiFlowVisulizeField::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    const int frame = engine.getSimulationFrame(time);
    if (frame < getStartFrame())
        return true;

    SIM_ScalarField* F = getScalarField(obj, GAS_NAME_FIELD);
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY);
    if (!F && !V)
    {
        addError(obj, SIM_MESSAGE, "Missing fields", UT_ERROR_FATAL);
        return false;
    }

    if (frame == getStartFrame())
    {
        {
            UT_WorkBuffer expr;
            expr.sprintf("from phi.torch.flow import *");
            HinaFlow::Python::PhiFlowSmoke::CompileFunction(expr);
        }

        auto str = getCode().toStdString();
        printf("Compiling PhiFlow code:\n%s\n", str.c_str());
        {
            UT_WorkBuffer expr(str);
            HinaFlow::Python::PhiFlowSmoke::CompileFunction(expr);
        }

        if (F)
        {
            const auto name = getTargetFieldName().toStdString();
            F->setSize(HinaFlow::Python::PhiFlowSmoke::FetchFieldSize(name));
            F->setDivisions(HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution(name));
            HinaFlow::Python::PhiFlowSmoke::FetchScalarField2D(name, F);
        }
        if (V)
        {
            const auto name = getTargetFieldName().toStdString();
            V->setSize(HinaFlow::Python::PhiFlowSmoke::FetchFieldSize(name));
            V->setDivisions(HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution(name));
            HinaFlow::Python::PhiFlowSmoke::FetchVectorField2D(name, V);
        }
    }

    return true;
}
