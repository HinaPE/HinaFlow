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

class GAS_DebugSPHKernel final : public GAS_SubSolver
{
public:
    static constexpr bool GEN_NODE = true;
    inline static auto DOP_NAME = "DebugSPHKernel";
    inline static auto DOP_ENGLISH = "Debug SPHKernel";
    inline static auto DATANAME = "DebugSPHKernel";
    static constexpr bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("KernelType", KernelType)
    GETSET_DATA_FUNCS_F("KernelRadius", KernelRadius)

protected:
    explicit GAS_DebugSPHKernel(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) override;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_DebugSPHKernel, GAS_SubSolver, "This is a Debug SPH Kernel Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_DEBUGSPHKERNEL_H
