#ifndef GAS_PHIFLOWFETCHFIELD_H
#define GAS_PHIFLOWFETCHFIELD_H

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

class GAS_PhiFlowFetchField final : public GAS_SubSolver
{
public:
    static constexpr bool GEN_NODE = true;
    inline static auto DOP_NAME = "PhiFlowFetchField";
    inline static auto DOP_ENGLISH = "PhiFlow Fetch Field";
    inline static auto DATANAME = "PhiFlowFetchField";
    static constexpr bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("StartFrame", StartFrame)
    GETSET_DATA_FUNCS_S("Target", Target)
    GETSET_DATA_FUNCS_S("BATCH", BATCH)
    GETSET_DATA_FUNCS_I("BATCH_NUM", BATCH_NUM)

protected:
    explicit GAS_PhiFlowFetchField(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) override;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_PhiFlowFetchField, GAS_SubSolver, "This is a PhiFlow Fetch Field provided by HinaFlow.", getDopDescription());
};


#endif //GAS_PHIFLOWFETCHFIELD_H
