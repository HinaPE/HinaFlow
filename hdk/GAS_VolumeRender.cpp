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
#include "GAS_SolvePoisson.h"
#include "src/image.h"

const SIM_DopDescription* GAS_VolumeRender::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_COLOR
    ACTIVATE_GAS_DENSITY
    ACTIVATE_GAS_STENCIL
    PARAMETER_FLOAT(Step, 1)
    PARAMETER_FLOAT(FocalLength, 150)
    PARAMETER_FLOAT(Coeff, 30)
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

    const std::string& path = COLOR->getPositionPath().toStdString();
    const size_t lsp = path.find_last_of('/');
    SIM_Position* pos = SIM_DATA_GET(*obj, lsp != std::string::npos?path.substr(lsp + 1).c_str():path.c_str(), SIM_Position);
    UT_Vector3 center;
    UT_Quaternion orient;
    pos->getPosition(center);
    pos->getOrientation(orient);
    UT_Vector3 dir = -center;
    dir.normalize();

    HinaFlow::Image::Render(COLOR, D, VGEO_Ray(center, dir), static_cast<float>(getStep()), getCoeff());
    return true;
}
