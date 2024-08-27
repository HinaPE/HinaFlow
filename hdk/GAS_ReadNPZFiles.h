#ifndef GAS_READNPZFILES_H
#define GAS_READNPZFILES_H

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

class GAS_ReadNPZFiles final : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "ReadNPZFiles";
    inline static const char* DOP_ENGLISH = "Read NPZ Files";
    inline static const char* DATANAME = "ReadNPZFiles";
    inline static const bool UNIQUE_DATANAME = false;

protected:
    explicit GAS_ReadNPZFiles(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_ReadNPZFiles, GAS_SubSolver, "This is a Read NPZ Files provided by HinaFlow.", getDopDescription());
};


#endif //GAS_READNPZFILES_H
