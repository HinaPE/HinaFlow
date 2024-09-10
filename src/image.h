#ifndef HINAFLOW_IMAGE_H
#define HINAFLOW_IMAGE_H

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

#include <SIM/SIM_ScalarField.h>
#include <SIM/SIM_VectorField.h>
#include <VGEO/VGEO_Ray.h>

namespace HinaFlow
{
    struct Image
    {
        static void Render(SIM_VectorField* TARGET, const SIM_ScalarField* FIELD, const VGEO_Ray& view, const float step, const float coeff = 1.f);
        static void RenderBTB(SIM_VectorField* TARGET, const SIM_ScalarField* FIELD, const VGEO_Ray& view, const float step, const float coeff = 1.f);
    };
}

#endif //HINAFLOW_IMAGE_H
