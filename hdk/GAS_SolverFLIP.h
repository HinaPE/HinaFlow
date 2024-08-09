#ifndef GAS_SOLVERFLIP_H
#define GAS_SOLVERFLIP_H

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

class GAS_SolverFLIP final : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "SolverFLIP";
    inline static const char* DOP_ENGLISH = "Solver FLIP";
    inline static const char* DATANAME = "SolverFLIP";
    inline static const bool UNIQUE_DATANAME = false;

protected:
    explicit GAS_SolverFLIP(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_SolverFLIP, GAS_SubSolver, "This is a FLIP Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_SOLVERFLIP_H
