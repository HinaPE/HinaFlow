#ifndef GAS_SOLVERPBF_H
#define GAS_SOLVERPBF_H

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

class GAS_SolverPBF final : public GAS_SubSolver
{
public:
    static constexpr bool GEN_NODE = true;
    inline static auto DOP_NAME = "SolverPBF";
    inline static auto DOP_ENGLISH = "Solver PBF";
    inline static auto DATANAME = "SolverPBF";
    static constexpr bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_V3("MaxBound", MaxBound)
    GETSET_DATA_FUNCS_B("TopOpen", TopOpen)
    GETSET_DATA_FUNCS_I("KernelType", KernelType)
    GETSET_DATA_FUNCS_F("KernelRadius", KernelRadius)
    GETSET_DATA_FUNCS_I("PressureIteration", PressureIteration)
    GETSET_DATA_FUNCS_F("EPS", EPS)
    GETSET_DATA_FUNCS_F("Viscosity", Viscosity)
    GETSET_DATA_FUNCS_F("DPScale", DPScale)

protected:
    explicit GAS_SolverPBF(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) override;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_SolverPBF, GAS_SubSolver, "This is a Position Based Fluids Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_SOLVERPBF_H
