#ifndef GAS_ADAPTIVEDOMAIN_H
#define GAS_ADAPTIVEDOMAIN_H

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

class GAS_AdaptiveDomain final : public GAS_SubSolver
{
public:
    static constexpr bool GEN_NODE = true;
    inline static auto DOP_NAME = "AdaptiveDomain";
    inline static auto DOP_ENGLISH = "Adaptive Domain";
    inline static auto DATANAME = "AdaptiveDomain";
    static constexpr bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("Depth", Depth);

protected:
    explicit GAS_AdaptiveDomain(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) override;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_AdaptiveDomain, GAS_SubSolver, "This is a Adaptive Domain provided by HinaFlow.", getDopDescription());
};


#endif //GAS_ADAPTIVEDOMAIN_H
