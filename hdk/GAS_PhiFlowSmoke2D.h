#ifndef GAS_PHIFLOWSMOKE2D_H
#define GAS_PHIFLOWSMOKE2D_H

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

class GAS_PhiFlowSmoke2D : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "PhiFlowSmoke2D";
    inline static const char* DOP_ENGLISH = "PhiFlow Smoke 2D";
    inline static const char* DATANAME = "PhiFlowSmoke2D";
    inline static const bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("StartFrame", StartFrame)
    GETSET_DATA_FUNCS_V2("Resolution", Resolution)
    GETSET_DATA_FUNCS_V2("Size", Size)
    GETSET_DATA_FUNCS_V2("Center", Center)
    GETSET_DATA_FUNCS_B("DebugMode", DebugMode)

protected:
    explicit GAS_PhiFlowSmoke2D(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_PhiFlowSmoke2D, GAS_SubSolver, "This is a PhiFlow Smoke 2D Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_PHIFLOWSMOKE2D_H
