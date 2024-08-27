#ifndef GAS_PHIFLOWVISUALIZEFIELD_H
#define GAS_PHIFLOWVISUALIZEFIELD_H

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


#include <GAS/GAS_SubSolver.h>

class GAS_PhiFlowVisualizeField final : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "PhiFlowVisualizeField";
    inline static const char* DOP_ENGLISH = "PhiFlow Visualize Field";
    inline static const char* DATANAME = "PhiFlowVisualizeField";
    inline static const bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("StartFrame", StartFrame)
    GETSET_DATA_FUNCS_S("TargetFieldName", TargetFieldName)
    GETSET_DATA_FUNCS_S("Code", Code)

protected:
    explicit GAS_PhiFlowVisualizeField(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_PhiFlowVisualizeField, GAS_SubSolver, "This is a PhiFlow Visualize Field provided by HinaFlow.", getDopDescription());
};


#endif //GAS_PHIFLOWVISUALIZEFIELD_H
