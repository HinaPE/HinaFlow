#ifndef GAS_PHIFLOWVISULIZEFIELD_H
#define GAS_PHIFLOWVISULIZEFIELD_H

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

class GAS_PhiFlowVisulizeField final : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "PhiFlowVisulizeField";
    inline static const char* DOP_ENGLISH = "PhiFlow Visulize Field";
    inline static const char* DATANAME = "PhiFlowVisulizeField";
    inline static const bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("StartFrame", StartFrame)
    GETSET_DATA_FUNCS_S("TargetFieldName", TargetFieldName)
    GETSET_DATA_FUNCS_S("Code", Code)

protected:
    explicit GAS_PhiFlowVisulizeField(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_PhiFlowVisulizeField, GAS_SubSolver, "This is a PhiFlow Visulize Field provided by HinaFlow.", getDopDescription());
};


#endif //GAS_PHIFLOWVISULIZEFIELD_H
