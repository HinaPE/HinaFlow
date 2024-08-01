#include "GAS_SolverPBF.h"

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
#include "src/pbf.h"

const SIM_DopDescription* GAS_SolverPBF::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_GEOMETRY

    static std::array<PRM_Name, 5> KernelType = {
        PRM_Name("0", "Poly6"),
        PRM_Name("1", "Spiky"),
        PRM_Name("2", "Cubic"),
        PRM_Name(nullptr),
    };
    static PRM_Name KernelTypeName("KernelType", "Kernel Type");
    static PRM_Default KernelTypeNameDefault(0);
    static PRM_ChoiceList CLKernelType(PRM_CHOICELIST_SINGLE, KernelType.data());
    PRMs.emplace_back(PRM_ORD, 1, &KernelTypeName, &KernelTypeNameDefault, &CLKernelType);

    PARAMETER_INT(PressureIteration, 20)
    PARAMETER_FLOAT(KernelRadius, 0.04)
    PARAMETER_FLOAT(Viscosity, 0.01)
    PARAMETER_FLOAT(DPScale, 1)
    PARAMETER_FLOAT(EPS, 0.01)
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

bool GAS_SolverPBF::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    SIM_GeometryCopy* G = getGeometryCopy(obj, GAS_NAME_GEOMETRY);

    if (!HinaFlow::CHECK_NOT_NULL(G))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    SIM_GeometryAutoWriteLock lock(G);
    GU_Detail& gdp = lock.getGdp();
    if (gdp.getNumPoints() == 0)
        return true;

    HinaFlow::PBF::Input input{&gdp, static_cast<float>(timestep)};
    HinaFlow::PBF::Param param;
    param.kernel_radius = static_cast<float>(getKernelRadius());
    param.epsilon = static_cast<float>(getEPS());
    param.viscosity = static_cast<float>(getViscosity());
    param.dpscale = static_cast<float>(getDPScale());
    switch (getKernelType())
    {
    case 0:
        param.kernel_type = HinaFlow::PBF::Param::KernelType::Poly6;
        break;
    case 1:
        param.kernel_type = HinaFlow::PBF::Param::KernelType::Spiky;
        break;
    case 2:
        param.kernel_type = HinaFlow::PBF::Param::KernelType::Cubic;
        break;
    default:
        throw std::runtime_error("Invalid kernel type");
    }
    HinaFlow::PBF::Result result{&gdp};
    // HinaFlow::PBF::Advect(input, param, result);
    // for (int _ = 0; _ < getPressureIteration(); ++_)


    // For Debug
    GA_RWHandleV3 p_handle = gdp.getP();
    POINT_ATTRIBUTE_F(mass)
    POINT_ATTRIBUTE_I(nn)
    POINT_ATTRIBUTE_F(rho1)
    POINT_ATTRIBUTE_F(rho2)
    auto& Kernel = HinaFlow::Poly6;
    if (param.kernel_type == HinaFlow::PBF::Param::KernelType::Spiky)
        Kernel = HinaFlow::Spiky;
    else if (param.kernel_type == HinaFlow::PBF::Param::KernelType::Cubic)
        Kernel = HinaFlow::Cubic;
    auto& gradKernel = HinaFlow::gradPoly6;
    if (param.kernel_type == HinaFlow::PBF::Param::KernelType::Spiky)
        gradKernel = HinaFlow::gradSpiky;
    else if (param.kernel_type == HinaFlow::PBF::Param::KernelType::Cubic)
        gradKernel = HinaFlow::gradCubic;
    GU_NeighbourList Searcher;
    GU_NeighbourListParms NLP;
    NLP.setRadius(param.kernel_radius);
    NLP.setOverrideRadius(true);
    NLP.setMode(GU_NeighbourListParms::InteractionMode::UNIFORM);
    Searcher.build(&gdp, NLP);
    GA_Offset i;
    std::vector<UT_Array<GA_Offset>> neighbors(gdp.getNumPoints());
    {
        GA_FOR_ALL_PTOFF(&gdp, i)
        {
            Searcher.getNeighbours(static_cast<int>(i), &gdp, neighbors[gdp.pointIndex(i)]);
            nn_handle.set(i, static_cast<int>(neighbors[gdp.pointIndex(i)].size()));
        }
    }

    {
        GA_FOR_ALL_PTOFF(&gdp, i)
        {
            float density = 0;
            for (GA_Offset j : neighbors[i])
                density += mass_handle.get(j) * Kernel(p_handle.get(i) - p_handle.get(j), param.kernel_radius);
            rho1_handle.set(i, density);
        }
    }
    for (int _ = 0; _ < getPressureIteration(); ++_)
        HinaFlow::PBF::SolvePressure(input, param, result);
    {
        GA_FOR_ALL_PTOFF(&gdp, i)
        {
            float density = 0;
            for (GA_Offset j : neighbors[i])
                density += mass_handle.get(j) * Kernel(p_handle.get(i) - p_handle.get(j), param.kernel_radius);
            rho2_handle.set(i, density);
        }
    }

    return true;
}
