#ifndef GAS_PHIFLOWSMOKE3D_H
#define GAS_PHIFLOWSMOKE3D_H

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

class GAS_PhiFlowSmoke3D final : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "PhiFlowSmoke3D";
    inline static const char* DOP_ENGLISH = "PhiFlow Smoke 3D";
    inline static const char* DATANAME = "PhiFlowSmoke3D";
    inline static const bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("StartFrame", StartFrame)
    GETSET_DATA_FUNCS_V3("Resolution", Resolution)
    GETSET_DATA_FUNCS_V3("Size", Size)
    GETSET_DATA_FUNCS_V3("Center", Center)
    GETSET_DATA_FUNCS_B("DebugMode", DebugMode)
    GETSET_DATA_FUNCS_S("SourceVolumePath", SourceVolumePath)
    GETSET_DATA_FUNCS_I("SourcePrimIndex", SourcePrimIndex)

protected:
    explicit GAS_PhiFlowSmoke3D(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_PhiFlowSmoke3D, GAS_SubSolver, "This is a PhiFlow Smoke 3D Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_PHIFLOWSMOKE3D_H
