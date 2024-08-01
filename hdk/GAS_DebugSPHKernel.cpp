#include "GAS_DebugSPHKernel.h"

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

const SIM_DopDescription* GAS_DebugSPHKernel::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_GEOMETRY
    ACTIVATE_GAS_FIELD
    ACTIVATE_GAS_VELOCITY

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

    PARAMETER_FLOAT(KernelRadius, 0.04)
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


namespace HinaFlow::Internal
{
    void KnContributeSPHWeightPartial(SIM_ScalarField* F, const std::function<float(const UT_Vector3&, const float)>& kernel, const UT_Vector3& pos, const float kr, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(F->getField()->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const UT_Vector3 p = F->getField()->indexToPos(cell);

            if (const float weight = kernel(p - pos, kr); weight > 0)
                vit.setValue(vit.getValue() + weight);
        }
    }

    THREADED_METHOD4(, F->getField()->shouldMultiThread(), KnContributeSPHWeight, SIM_ScalarField*, F, const std::function<float(const UT_Vector3&, const float)> &, kernel, const UT_Vector3&, pos, const float, kr);


    void KnContributeSPHWeightGradPartial(SIM_VectorField* F, const std::function<UT_Vector3(const UT_Vector3&, const float)>& kernel, const UT_Vector3& pos, const float kr, const int AXIS, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(F->getField(AXIS)->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const UT_Vector3 p = F->getField(AXIS)->indexToPos(cell);

            const float weight = kernel(p - pos, kr)[AXIS];
            vit.setValue(vit.getValue() + weight);
        }
    }

    THREADED_METHOD5(, F->getField(AXIS)->shouldMultiThread(), KnContributeSPHWeightGrad, SIM_VectorField*, F, const std::function<UT_Vector3(const UT_Vector3&, const float)> &, kernel, const UT_Vector3&, pos, const float, kr, const int,
                       AXIS);
}

bool GAS_DebugSPHKernel::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    SIM_GeometryCopy* G = getGeometryCopy(obj, GAS_NAME_GEOMETRY);
    SIM_ScalarField* F = getScalarField(obj, GAS_NAME_FIELD);
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY);

    if (!HinaFlow::CHECK_NOT_NULL(G, F, V))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    SIM_GeometryAutoWriteLock lock(G);
    GU_Detail& gdp = lock.getGdp();
    if (gdp.getNumPoints() == 0)
        return true;

    F->getField()->makeConstant(0);
    for (const int AXIS : V->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2})
        V->getField(AXIS)->makeConstant(0);

    GA_Offset pt_off;
    GA_FOR_ALL_PTOFF(&gdp, pt_off)
    {
        switch (getKernelType())
        {
        case 0:
            HinaFlow::Internal::KnContributeSPHWeight(F, HinaFlow::Poly6, gdp.getPos3(pt_off), static_cast<float>(getKernelRadius()));
            for (const int AXIS : V->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2})
                HinaFlow::Internal::KnContributeSPHWeightGrad(V, HinaFlow::gradPoly6, gdp.getPos3(pt_off), static_cast<float>(getKernelRadius()), AXIS);
            break;
        case 1:
            HinaFlow::Internal::KnContributeSPHWeight(F, HinaFlow::Spiky, gdp.getPos3(pt_off), static_cast<float>(getKernelRadius()));
            for (const int AXIS : V->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2})
                HinaFlow::Internal::KnContributeSPHWeightGrad(V, HinaFlow::gradSpiky, gdp.getPos3(pt_off), static_cast<float>(getKernelRadius()), AXIS);
            break;
        case 2:
            HinaFlow::Internal::KnContributeSPHWeight(F, HinaFlow::Cubic, gdp.getPos3(pt_off), static_cast<float>(getKernelRadius()));
            for (const int AXIS : V->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2})
                HinaFlow::Internal::KnContributeSPHWeightGrad(V, HinaFlow::gradCubic, gdp.getPos3(pt_off), static_cast<float>(getKernelRadius()), AXIS);
            break;
        default:
            throw std::runtime_error("Invalid kernel type");
        }
    }

    return true;
}
