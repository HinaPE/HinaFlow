#include "GAS_ReconstructDensity.h"

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


#include "common.h"

const SIM_DopDescription* GAS_ReconstructDensity::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    PRMs.emplace_back();

    static SIM_DopDescription DESC(GEN_NODE,
                                   DOP_NAME,
                                   DOP_ENGLISH,
                                   DATANAME,
                                   classname(),
                                   PRMs.data());
    DESC.setDefaultUniqueDataName(UNIQUE_DATANAME);
    setGasDescription(DESC);
    return &DESC;
}

bool GAS_ReconstructDensity::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    return true;
}

SIM_Guide* GAS_ReconstructDensity::createGuideObjectSubclass() const
{
    return new SIM_GuideShared(this, true);
}

void GAS_ReconstructDensity::buildGuideGeometrySubclass(const SIM_RootData& root, const SIM_Options& options, const GU_DetailHandle& gdh, UT_DMatrix4* xform, const SIM_Time& t) const
{
    GAS_SubSolver::buildGuideGeometrySubclass(root, options, gdh, xform, t);
}
