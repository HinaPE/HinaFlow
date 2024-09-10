#ifndef GAS_VOLUMERENDER_H
#define GAS_VOLUMERENDER_H

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

class GAS_VolumeRender final : public GAS_SubSolver
{
public:
    static constexpr bool GEN_NODE = true;
    inline static auto DOP_NAME = "VolumeRender";
    inline static auto DOP_ENGLISH = "Volume Render";
    inline static auto DATANAME = "VolumeRender";
    static constexpr bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_F("Step", Step)
    GETSET_DATA_FUNCS_F("FocalLength", FocalLength)
    GETSET_DATA_FUNCS_I("Coeff", Coeff)

protected:
    explicit GAS_VolumeRender(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) override;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_VolumeRender, GAS_SubSolver, "This is a Volume Renderer provided by HinaFlow.", getDopDescription());
};

#endif //GAS_VOLUMERENDER_H
