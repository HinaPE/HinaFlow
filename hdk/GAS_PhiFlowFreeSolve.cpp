#include "GAS_PhiFlowFreeSolve.h"

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
#include <fstream>
#include <sstream>

const SIM_DopDescription* GAS_PhiFlowFreeSolve::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    PARAMETER_INT(StartFrame, 1)
    PARAMETER_FILE(InitCode, "")
    PARAMETER_FILE(StepCode, "")
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

bool GAS_PhiFlowFreeSolve::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    const int frame = engine.getSimulationFrame(time);
    if (frame < getStartFrame())
        return true;

    if (frame == getStartFrame())
    {
        std::ifstream init(getInitCode().toStdString());
        std::stringstream init_buffer;
        init_buffer << init.rdbuf();
        UT_WorkBuffer expr;
        expr.sprintf(init_buffer.str().c_str());
        HinaFlow::Python::PhiFlowSmoke::CompileFunction(expr);
    }
    else
    {
        std::ifstream step(getStepCode().toStdString());
        std::stringstream step_buffer;
        step_buffer << step.rdbuf();
        UT_WorkBuffer expr;
        expr.sprintf(step_buffer.str().c_str());
        HinaFlow::Python::PhiFlowSmoke::CompileFunction(expr);
    }

    return true;
}
