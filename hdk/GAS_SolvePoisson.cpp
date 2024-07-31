#include "GAS_SolvePoisson.h"

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
#include "src/poisson.h"

const SIM_DopDescription* GAS_SolvePoisson::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_VELOCITY
    ACTIVATE_GAS_DIVERGENCE
    ACTIVATE_GAS_PRESSURE
    ACTIVATE_GAS_STENCIL

    static std::array<PRM_Name, 5> PCG_METHOD = {
        PRM_Name("0", "PCG_NONE"),
        PRM_Name("1", "PCG_JACOBI"),
        PRM_Name("2", "PCG_CHOLESKY"),
        PRM_Name("3", "PCG_MIC"),
        PRM_Name(nullptr),
    };
    static PRM_Name PCG_METHODName("PCG_METHOD", "PCG METHOD");
    static PRM_Default PCG_METHODNameDefault(3);
    static PRM_ChoiceList CLPCG_METHOD(PRM_CHOICELIST_SINGLE, PCG_METHOD.data());
    PRMs.emplace_back(PRM_ORD, 1, &PCG_METHODName, &PCG_METHODNameDefault, &CLPCG_METHOD);
    PARAMETER_BOOL(MultiThreaded, false)
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

bool GAS_SolvePoisson::solveGasSubclass(SIM_Engine&, SIM_Object* obj, SIM_Time, SIM_Time)
{
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY); // required
    SIM_ScalarField* DIV = getScalarField(obj, GAS_NAME_DIVERGENCE); // optional
    SIM_ScalarField* PRS = getScalarField(obj, GAS_NAME_PRESSURE); // required
    SIM_IndexField* MARKER = getIndexField(obj, GAS_NAME_STENCIL); // required

    if (!HinaFlow::CHECK_NOT_NULL(V, PRS, MARKER))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    if (!HinaFlow::CHECK_THE_SAME_DIMENSION(V, PRS, MARKER))
    {
        addError(obj, SIM_MESSAGE, "GAS fields have different dimensions", UT_ERROR_FATAL);
        return false;
    }

    if (!HinaFlow::CHECK_IS_FACE_SAMPLED(V))
    {
        addError(obj, SIM_MESSAGE, "Velocity field is not face sampled", UT_ERROR_FATAL);
        return false;
    }

    /**
    * For smoke applications, we assume "air" is the same as "smoke",
    * ie. all cells are filled with "smoke", the only difference is the density value.
    */
    HinaFlow::FILL_FIELD(MARKER, static_cast<exint>(HinaFlow::CellType::Fluid));

    HinaFlow::Possion::Input input{V, MARKER};
    HinaFlow::Possion::Param param;
    switch (getPCG_METHOD())
    {
    case 0: param.preconditioner = SIM_RawField::PCG_METHOD::PCG_NONE;
        break;
    case 1: param.preconditioner = SIM_RawField::PCG_METHOD::PCG_JACOBI;
        break;
    case 2: param.preconditioner = SIM_RawField::PCG_METHOD::PCG_CHOLESKY;
        break;
    case 3: param.preconditioner = SIM_RawField::PCG_METHOD::PCG_MIC;
        break;
    default:
        throw std::runtime_error("Invalid PCG_METHOD");
        break;
    }
    HinaFlow::Possion::Result result{V, PRS, DIV};

    if (getMultiThreaded())
        HinaFlow::Possion::SolveMultiThreaded(input, param, result);
    else
        HinaFlow::Possion::Solve(input, param, result);

    return true;
}
