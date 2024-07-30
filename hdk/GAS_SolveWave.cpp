#include "GAS_SolveWave.h"

#include "common.h"
#include "src/wave.h"

const SIM_DopDescription* GAS_SolveWave::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_DENSITY
    ACTIVATE_GAS_TEMPERATURE
    ACTIVATE_GAS_STENCIL
    ACTIVATE_GAS_COLOR

    static std::array<PRM_Name, 5> PCG_METHOD = {
        PRM_Name("0", "PCG_NONE"),
        PRM_Name("1", "PCG_JACOBI"),
        PRM_Name("2", "PCG_CHOLESKY"),
        PRM_Name("3", "PCG_MIC"),
        PRM_Name(nullptr),
    };
    static PRM_Name PCG_METHODName("PCG_METHOD", "PCG METHOD");
    static PRM_Default PCG_METHODNameDefault(3);
    static PRM_ChoiceList CLTestPCG_METHOD(PRM_CHOICELIST_SINGLE, PCG_METHOD.data());
    PRMs.emplace_back(PRM_ORD, 1, &PCG_METHODName, &PCG_METHODNameDefault, &CLTestPCG_METHOD);
    PARAMETER_BOOL(MultiThreaded, false)

    PARAMETER_FLOAT(Wave, 0.01)
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

bool GAS_SolveWave::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    SIM_ScalarField* D = getScalarField(obj, GAS_NAME_DENSITY);
    SIM_ScalarField* T = getScalarField(obj, GAS_NAME_TEMPERATURE);
    SIM_IndexField* MARKER = getIndexField(obj, GAS_NAME_STENCIL);
    SIM_VectorField* COLOR = getVectorField(obj, GAS_NAME_COLOR);

    if (!HinaFlow::CHECK_NOT_NULL(D, T, MARKER, COLOR))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    if (!HinaFlow::CHECK_THE_SAME_DIMENSION(D, T, MARKER, COLOR))
    {
        addError(obj, SIM_MESSAGE, "GAS fields have different dimensions", UT_ERROR_FATAL);
        return false;
    }

    /**
    * For smoke applications, we assume "air" is the same as "smoke",
    * ie. all cells are filled with "smoke", the only difference is the density value.
    */
    HinaFlow::FILL_FIELD(MARKER, static_cast<exint>(HinaFlow::CellType::Fluid));

    HinaFlow::Wave::Input input{D, T, MARKER};
    HinaFlow::Wave::Param param;
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
    param.wave = float(getWave());
    HinaFlow::Wave::Result result{D};

    if (getMultiThreaded())
        HinaFlow::Wave::SolveMultiThreaded(input, param, result);
    else
        HinaFlow::Wave::Solve(input, param, result);

    return true;
}
