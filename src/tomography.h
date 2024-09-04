#ifndef HINAFLOW_TOMOGRAPHY_H
#define HINAFLOW_TOMOGRAPHY_H

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
#include <SIM/SIM_IndexField.h>
#include <VGEO/VGEO_Ray.h>

namespace HinaFlow
{
    struct Tomography
    {
    };

    void Render(SIM_ScalarField* D, const VGEO_Ray& Ray, const float step);
}


#endif //HINAFLOW_TOMOGRAPHY_H
