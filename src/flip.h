#ifndef HINAFLOW_FLIP_H
#define HINAFLOW_FLIP_H

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


#include <GU/GU_Detail.h>
#include <SIM/SIM_VectorField.h>
#include <SIM/SIM_IndexField.h>

namespace HinaFlow
{
    struct FLIP
    {
        struct Input
        {
            GU_Detail* gdp = nullptr; // required
            SIM_VectorField* FLOW = nullptr; // required
            SIM_IndexField* MARKER = nullptr; // required
        };

        struct Param
        {
            int extrapolate_depth = 6;
            float ratio = 0.97f;
        };

        struct Result // Results
        {
            SIM_VectorField* WEIGHT = nullptr; // required
            SIM_ScalarField* PRESSURE = nullptr; // required
            SIM_ScalarField* DIVERGENCE = nullptr; // optional
            SIM_IndexField* EX_INDEX = nullptr; // optional
        };


        static void P2G(const Input& input, const Param& param, Result& result);
        static void SolvePressure(const Input& input, const Param& param, Result& result);
        static void G2P(const Input& input, const Param& param, Result& result);

        static std::array<SIM_RawField, 3> FLOW_CACHE;
    };
}


#endif //HINAFLOW_FLIP_H
