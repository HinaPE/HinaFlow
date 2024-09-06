#ifndef GAS_SOLVEDIFFUSION_H
#define GAS_SOLVEDIFFUSION_H

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

class GAS_SolveDiffusion final : public GAS_SubSolver
{
public:
    static constexpr bool GEN_NODE = true;
    inline static auto DOP_NAME = "SolveDiffusion";
    inline static auto DOP_ENGLISH = "Solve Diffusion";
    inline static auto DATANAME = "SolveDiffusion";
    static constexpr bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("PCG_METHOD", PCG_METHOD)
    GETSET_DATA_FUNCS_B("MultiThreaded", MultiThreaded)
    GETSET_DATA_FUNCS_F("Diffusion", Diffusion)

protected:
    explicit GAS_SolveDiffusion(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) override;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_SolveDiffusion, GAS_SubSolver, "This is a Solve Diffusion Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_SOLVEDIFFUSION_H
