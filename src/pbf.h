#ifndef HINAFLOW_PBF_H
#define HINAFLOW_PBF_H

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

namespace HinaFlow
{
    struct PBF
    {
        struct Input
        {
            GU_Detail* gdp = nullptr; // required
            float dt = 1.f; // required
        };

        struct Param
        {
            float kernel_radius = 0.04f;
            float viscosity = 0.01f;
            float gravity = -9.8f;
            float rest_density = 1000.f;
        };

        struct Result // Results
        {
            GU_Detail* gdp = nullptr; // required
        };

        static void Solve(const Input& input, const Param& param, Result& result);
        static void SolveMultiThreaded(const Input& input, const Param& param, Result& result);
    };
}


#endif //HINAFLOW_PBF_H
