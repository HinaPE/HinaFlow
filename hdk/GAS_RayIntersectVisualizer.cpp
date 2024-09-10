#include "GAS_RayIntersectVisualizer.h"

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

const SIM_DopDescription* GAS_RayIntersectVisualizer::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_FIELD
    PARAMETER_VECTOR_FLOAT_N(Origin, 3, 0, 0, 10)
    PARAMETER_VECTOR_FLOAT_N(Direction, 3, 0, 0, -10)
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

bool GAS_RayIntersectVisualizer::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    SIM_ScalarField* FIELD = getScalarField(obj, GAS_NAME_FIELD);

    if (!HinaFlow::CHECK_NOT_NULL(FIELD))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    UT_Vector3 origin = getOriginF();
    UT_Vector3 dir = getDirectionF();
    dir.normalize();
    const VGEO_Ray ray{origin, dir};
    float tmin = 0, tmax = std::numeric_limits<float>::max();
    UT_BoundingBox bbox;
    FIELD->getBBox(bbox);
    const bool intersect = ray.getBoxRange(bbox, tmin, tmax);
    const int frame = engine.getSimulationFrame(time);
    const float step = static_cast<float>(getStep());

    FIELD->getField()->makeConstant(0);
    if (intersect)
    {
        float cur = tmin;
        cur += (frame - 1) * step;

        const UT_Vector3 pt = ray.getPt(cur);
        int ix, iy, iz;
        FIELD->getField()->posToIndex(pt, ix, iy, iz);
        if (HinaFlow::CHECK_CELL_VALID(FIELD->getField(), {ix, iy, iz}))
            SIM::FieldUtils::setFieldValue(*FIELD->getField(), UT_Vector3I{ix, iy, iz}, 100.f);
    }

    return true;
}

SIM_Guide* GAS_RayIntersectVisualizer::createGuideObjectSubclass() const
{
    return new SIM_GuideShared(this, false);
}

void GAS_RayIntersectVisualizer::buildGuideGeometrySubclass(const SIM_RootData& root, const SIM_Options& options, const GU_DetailHandle& gdh, UT_DMatrix4* xform, const SIM_Time& t) const
{
    if (gdh.isNull())
        return;
    GU_DetailHandleAutoWriteLock gdl(gdh);
    GU_Detail* gdp = gdl.getGdp();

    const GA_Offset center_off = gdp->appendPoint();
    UT_Vector3 origin = getOriginF();
    gdp->setPos3(center_off, getOriginF());
    const GA_Offset dir_off = gdp->appendPoint();
    UT_Vector3 dir = getDirectionF();
    dir.normalize();
    gdp->setPos3(dir_off, origin + dir);
    GEO_PrimPoly* poly = GU_PrimPoly::build(gdp, 2, true, false);
    poly->setPointOffset(0, center_off);
    poly->setPointOffset(1, dir_off);
}
