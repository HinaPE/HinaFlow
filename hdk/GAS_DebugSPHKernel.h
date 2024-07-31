#ifndef GAS_DEBUGSPHKERNEL_H
#define GAS_DEBUGSPHKERNEL_H

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

class GAS_DebugSPHKernel : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "DebugSPHKernel";
    inline static const char* DOP_ENGLISH = "Debug SPHKernel";
    inline static const char* DATANAME = "DebugSPHKernel";
    inline static const bool UNIQUE_DATANAME = false;

protected:
    explicit GAS_DebugSPHKernel(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_DebugSPHKernel, GAS_SubSolver, "This is a Debug SPH Kernel Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_DEBUGSPHKERNEL_H
