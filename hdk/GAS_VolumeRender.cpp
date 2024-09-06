#include "GAS_VolumeRender.h"

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
#include "src/image.h"

const SIM_DopDescription* GAS_VolumeRender::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_COLOR
    ACTIVATE_GAS_DENSITY
    ACTIVATE_GAS_STENCIL
    PARAMETER_FLOAT(Step, 0.1)
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

bool GAS_VolumeRender::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    SIM_VectorField* COLOR = getVectorField(obj, GAS_NAME_COLOR);
    SIM_ScalarField* D = getScalarField(obj, GAS_NAME_DENSITY);
    SIM_IndexField* MARKER = getIndexField(obj, GAS_NAME_STENCIL);

    if (!HinaFlow::CHECK_NOT_NULL(D, COLOR))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    HinaFlow::FILL_FIELD(COLOR, 0.0);
    HinaFlow::Image::Render(COLOR, D, VGEO_Ray({0, 0, -1000}, {0, 0, 1}), static_cast<float>(getStep()));

    return true;
}

SIM_Guide* GAS_VolumeRender::createGuideObjectSubclass() const
{
    return new SIM_GuideShared(this, true);
}

void GAS_VolumeRender::buildGuideGeometrySubclass(const SIM_RootData& root, const SIM_Options& options, const GU_DetailHandle& gdh, UT_DMatrix4* xform, const SIM_Time& t) const
{
    if (gdh.isNull())
        return;
    GU_DetailHandleAutoWriteLock gdl(gdh);
    GU_Detail* gdp = gdl.getGdp();
    gdp->clearAndDestroy();
    GAS_SubSolver::buildGuideGeometrySubclass(root, options, gdh, xform, t);
}
