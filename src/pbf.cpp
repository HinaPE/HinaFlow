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

static std::vector<UT_Vector3> temp;

void HinaFlow::PBF::Advect(const Input& input, const Param& param, Result& result)
{
    GU_Detail& gdp = *input.gdp;
    GA_Offset i;
    GA_RWHandleV3 p_handle = gdp.getP();
    POINT_ATTRIBUTE_V3(v)

    temp.resize(gdp.getNumPoints());
    GA_FOR_ALL_PTOFF(&gdp, i)
    {
        temp[gdp.pointIndex(i)] = p_handle.get(i);
        v_handle.set(i, v_handle.get(i) + UT_Vector3{0.f, param.gravity, 0.f} * input.dt);
        p_handle.set(i, p_handle.get(i) + v_handle.get(i) * input.dt);
    }
}

void HinaFlow::PBF::SolvePressure(const Input& input, const Param& param, Result& result)
{
    GU_Detail& gdp = *input.gdp;
    const int64 size = gdp.getNumPoints();
    GA_Offset i;
    GA_RWHandleV3 p_handle = gdp.getP();
    POINT_ATTRIBUTE_V3(v)
    POINT_ATTRIBUTE_F(mass)
    POINT_ATTRIBUTE_F(lambda)
    POINT_ATTRIBUTE_I(nn)

    auto& Kernel = Poly6;
    if (param.kernel_type == Param::KernelType::Spiky)
        Kernel = Spiky;
    else if (param.kernel_type == Param::KernelType::Cubic)
        Kernel = Cubic;
    auto& gradKernel = gradPoly6;
    if (param.kernel_type == Param::KernelType::Spiky)
        gradKernel = gradSpiky;
    else if (param.kernel_type == Param::KernelType::Cubic)
        gradKernel = gradCubic;


    // Build Neighbors
    GU_NeighbourList Searcher;
    GU_NeighbourListParms NLP;
    NLP.setRadius(param.kernel_radius);
    NLP.setOverrideRadius(true);
    NLP.setMode(GU_NeighbourListParms::InteractionMode::UNIFORM);
    Searcher.build(&gdp, NLP);
    std::vector<UT_Array<GA_Offset>> neighbors(size);
    {
        GA_FOR_ALL_PTOFF(&gdp, i)
        {
            Searcher.getNeighbours(static_cast<int>(i), &gdp, neighbors[gdp.pointIndex(i)]);
            nn_handle.set(i, static_cast<int>(neighbors[gdp.pointIndex(i)].size()));
        }
    }


    // Compute lambda
    {
        GA_FOR_ALL_PTOFF(&gdp, i)
        {
            float lambda = 0;
            float density = 0;
            UT_Vector3 gradi{0.f, 0.f, 0.f};
            for (GA_Offset j : neighbors[i])
            {
                density += mass_handle.get(j) * Kernel(p_handle.get(i) - p_handle.get(j), param.kernel_radius);
                if (i == j)
                    gradi += gradKernel(p_handle.get(i) - p_handle.get(j), param.kernel_radius);
                else
                    lambda += gradKernel(p_handle.get(i) - p_handle.get(j), param.kernel_radius).length2();
            }
            if (density > param.rest_density)
            {
                lambda += gradi.length2();
                lambda += param.epsilon;
                lambda = -(density - param.rest_density) / lambda;
            }
            else
                lambda = 0;
            lambda_handle.set(i, lambda);
        }
    }


    // Compute and Apply deltaP
    {
        GA_FOR_ALL_PTOFF(&gdp, i)
        {
            UT_Vector3 deltap = {0.f, 0.f, 0.f};
            for (GA_Offset j : neighbors[i])
                deltap += (lambda_handle.get(i) + lambda_handle.get(j)) * gradKernel(p_handle.get(i) - p_handle.get(j), param.kernel_radius);
            p_handle.set(i, p_handle.get(i) + param.dpscale * deltap);
        }
    }


    // Enforce Boundary
    {
        GA_FOR_ALL_PTOFF(&gdp, i)
        {
            UT_Vector3 pos = p_handle.get(i);
            if (pos.x() > param.HalfBound.x())
                pos.x() = param.HalfBound.x();
            if (pos.x() < -param.HalfBound.x())
                pos.x() = -param.HalfBound.x();
            if (!param.TopOpen)
                if (pos.y() > param.HalfBound.y())
                    pos.y() = param.HalfBound.y();
            if (pos.y() < -param.HalfBound.y())
                pos.y() = -param.HalfBound.y();
            if (pos.z() > param.HalfBound.z())
                pos.z() = param.HalfBound.z();
            if (pos.z() < -param.HalfBound.z())
                pos.z() = -param.HalfBound.z();
            p_handle.set(i, pos);
        }
    }

    {
        float sumC = 0;
        GA_FOR_ALL_PTOFF(&gdp, i)
        {
            float density = 0;
            for (GA_Offset j : neighbors[i])
                density += mass_handle.get(j) * Kernel(p_handle.get(i) - p_handle.get(j), param.kernel_radius);
            sumC += density > param.rest_density ? (density / param.rest_density - 1) : 0;
        }
        if (sumC < 1)
        {
            result.done = true;
            return;
        }
    }
}

void HinaFlow::PBF::UpdateVelocity(const Input& input, const Param& param, Result& result)
{
    GU_Detail& gdp = *input.gdp;
    GA_Offset i;
    GA_RWHandleV3 p_handle = gdp.getP();
    POINT_ATTRIBUTE_V3(v)

    GA_FOR_ALL_PTOFF(&gdp, i)
    {
        v_handle.set(i, (p_handle.get(i) - temp[gdp.pointIndex(i)]) / input.dt * (1 - param.viscosity));
    }
}
