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
    PARAMETER_FLOAT(Step, 1)
    PARAMETER_FLOAT(FocalLength, 300)

    static std::array<PRM_Name, 5> View = {
        PRM_Name("0", "Front"),
        PRM_Name("1", "Right"),
        PRM_Name("2", "Top"),
        PRM_Name("3", "Free"),
        PRM_Name(nullptr),
    };
    static PRM_Name ViewName("View", "View");
    static PRM_Default ViewNameDefault(0);
    static PRM_ChoiceList CLView(PRM_CHOICELIST_SINGLE, View.data());
    PARAMETER_VECTOR_FLOAT_N(Center, 3, 0, 300, 0)
    PRMs.emplace_back(PRM_ORD, 1, &ViewName, &ViewNameDefault, &CLView);

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

    switch (getView())
    {
    case 0: // Front
        HinaFlow::Image::Render(COLOR, D, VGEO_Ray({0, 0, static_cast<float>(getFocalLength())}, {0, 0, -1}), static_cast<float>(getStep()));
        break;
    case 1: // Right
        HinaFlow::Image::Render(COLOR, D, VGEO_Ray({static_cast<float>(getFocalLength()), 0, 0}, {-1, 0, 0}), static_cast<float>(getStep()));
        break;
    case 2: // Top
        HinaFlow::Image::Render(COLOR, D, VGEO_Ray({0, static_cast<float>(getFocalLength()), 0}, {0, -1, 0}), static_cast<float>(getStep()));
        break;
    case 3: // Free
        {
            UT_Vector3 dir = getCenter();
            dir.normalize();
            dir = -dir;
            HinaFlow::Image::Render(COLOR, D, VGEO_Ray(-getFocalLength() * dir, dir), static_cast<float>(getStep()));
        }
        break;
    default:
        throw std::runtime_error("Invalid view");
    }

    return true;
}

SIM_Guide* GAS_VolumeRender::createGuideObjectSubclass() const
{
    return new SIM_GuideShared(this, false);
}

void GAS_VolumeRender::buildGuideGeometrySubclass(const SIM_RootData& root, const SIM_Options& options, const GU_DetailHandle& gdh, UT_DMatrix4* xform, const SIM_Time& t) const
{
    if (gdh.isNull())
        return;
    GU_DetailHandleAutoWriteLock gdl(gdh);
    GU_Detail* gdp = gdl.getGdp();

    UT_Vector3 center;
    switch (getView())
    {
    case 0: // Front
        center = UT_Vector3{0, 0, static_cast<float>(getFocalLength())};
        break;
    case 1: // Right
        center = UT_Vector3{static_cast<float>(getFocalLength()), 0, 0};
        break;
    case 2: // Top
        center = UT_Vector3{0, static_cast<float>(getFocalLength()), 0};
        break;
    case 3: // Free
        center = getCenter();
        break;
    default:
        throw std::runtime_error("Invalid view");
    }


    GA_Offset center_off = gdp->appendPoint();
    gdp->setPos3(center_off, center);
    GA_Offset dir_off = gdp->appendPoint();
    gdp->setPos3(dir_off, UT_Vector3{0, 0, 0});
    GEO_PrimPoly* poly = GU_PrimPoly::build(gdp, 2, true, false);
    poly->setPointOffset(0, center_off);
    poly->setPointOffset(1, dir_off);

    GU_PrimSphereParms params;
    params.gdp = gdp;
    params.ptoff = center_off;
    params.xform.scale(1, 1, 1);
    params.xform.translate(center);
    GEO_PrimSphere* sphere = (GEO_PrimSphere*)GU_PrimSphere::build(params);
}
