#ifndef HINAFLOW_WAVE_H
#define HINAFLOW_WAVE_H

/******************************************************************************
 *
 * HinaFlow fluid solver framework
 * Copyright 2024 Xayah Hina
 *
 * This program is free software, distributed under the terms of the
 * Apache License, Version 2.0
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 ******************************************************************************/

#include <SIM/SIM_ScalarField.h>
#include <SIM/SIM_VectorField.h>
#include <SIM/SIM_IndexField.h>

namespace HinaFlow
{
    struct Wave
    {
        struct Input
        {
            SIM_ScalarField* FIELDS = nullptr; // required
            SIM_ScalarField* FIELDS_PREV = nullptr; // required
            SIM_IndexField* MARKER = nullptr; // required
            float dt = 1.f; // required
        };

        struct Param
        {
            SIM_RawField::PCG_METHOD preconditioner = SIM_RawField::PCG_METHOD::PCG_MIC;
            float wave = 0.01f;
        };

        struct Result // Results
        {
            SIM_ScalarField* FIELDS = nullptr; // required
        };

        static void Solve(const Input& input, const Param& param, Result& result);
        static void SolveMultiThreaded(const Input& input, const Param& param, Result& result);
    };
}

#endif //HINAFLOW_WAVE_H
