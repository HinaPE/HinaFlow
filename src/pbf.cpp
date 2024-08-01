#include "pbf.h"

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

void HinaFlow::PBF::Solve(const Input& input, const Param& param, Result& result)
{
    GU_Detail& gdp = *input.gdp;
    const int64 size = gdp.getNumPoints();
    POINT_ATTRIBUTE_V3(v)
    POINT_ATTRIBUTE_V3(a)
    POINT_ATTRIBUTE_F(rho)
    POINT_ATTRIBUTE_F(mass)


    // Build Neighbors
    GU_NeighbourList Searcher;
    GU_NeighbourListParms NLP;
    NLP.setRadius(param.kernel_radius);
    NLP.setOverrideRadius(true);
    NLP.setMode(GU_NeighbourListParms::InteractionMode::UNIFORM);
    Searcher.build(&gdp, NLP);


    // Compute Density
    UTparallelForEachNumber(size, [&](const UT_BlockedRange<int64>& range)
    {
        for (int64 i = range.begin(); i != range.end(); ++i)
        {
            UT_Array<GA_Offset> neighbor_list;
            Searcher.getNeighbours(i, &gdp, neighbor_list);

            float density = 0;
            for (int j = 0; j < neighbor_list.size(); ++j)
            {
                UT_Vector3 pi = gdp.getPos3(i);
                UT_Vector3 pj = gdp.getPos3(j);
                float mi = mass_handle.get(i);
                float mj = mass_handle.get(j);
                density += mi * Poly6(pi - pj, param.kernel_radius);
            }
            rho_handle.set(i, density);
        }
    });
}

void HinaFlow::PBF::SolveMultiThreaded(const Input& input, const Param& param, Result& result)
{
}

void HinaFlow::PBF::SolveCUDA(const Input& input, const Param& param, Result& result)
{
}
