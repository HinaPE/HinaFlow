#ifndef GAS_RECONSTRUCTDENSITY_H
#define GAS_RECONSTRUCTDENSITY_H

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

class GAS_ReconstructDensity final : public GAS_SubSolver
{
public:
    static constexpr bool GEN_NODE = true;
    inline static auto DOP_NAME = "ReconstructDensity";
    inline static auto DOP_ENGLISH = "Reconstruct Density";
    inline static auto DATANAME = "ReconstructDensity";
    static constexpr bool UNIQUE_DATANAME = false;

protected:
    explicit GAS_ReconstructDensity(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) override;
    static const SIM_DopDescription* getDopDescription();
    SIM_Guide* createGuideObjectSubclass() const override;
    void buildGuideGeometrySubclass(const SIM_RootData& root, const SIM_Options& options, const GU_DetailHandle& gdh, UT_DMatrix4* xform, const SIM_Time& t) const override;
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_ReconstructDensity, GAS_SubSolver, "This is for Reconstructing Density provided by HinaFlow.", getDopDescription());
};

#endif //GAS_RECONSTRUCTDENSITY_H
