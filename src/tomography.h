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

namespace HinaFlow
{
    struct Tomography
    {
        struct Input
        {
            const SIM_VectorField* VIEW1 = nullptr; // required
            const SIM_VectorField* VIEW2 = nullptr; // optional
            const SIM_VectorField* VIEW3 = nullptr; // optional
            const SIM_VectorField* VIEW4 = nullptr; // optional
        };

        struct Param
        {
            UT_Vector3 pos1{};
            UT_Vector3 pos2{};
            UT_Vector3 pos3{};
            UT_Vector3 pos4{};
        };

        struct Result
        {
            SIM_ScalarField* TARGET = nullptr; // required
        };

        static void Solve(const Input& input, const Param& param, Result& result);
    };
}

#endif //HINAFLOW_TOMOGRAPHY_H
