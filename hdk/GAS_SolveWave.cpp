#include "GAS_SolveWave.h"

#include "common.h"

const SIM_DopDescription* GAS_SolveWave::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_GEOMETRY
    ACTIVATE_GAS_DENSITY
    ACTIVATE_GAS_TEMPERATURE
    ACTIVATE_GAS_VELOCITY
    ACTIVATE_GAS_SOURCE
    ACTIVATE_GAS_DIVERGENCE
    ACTIVATE_GAS_PRESSURE
    ACTIVATE_GAS_STENCIL
    ACTIVATE_GAS_COLOR
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
    SIM_VectorField* V = getVectorField(obj, GAS_NAME_VELOCITY);
    SIM_ScalarField* S = getScalarField(obj, GAS_NAME_SOURCE);
    SIM_ScalarField* DIV = getScalarField(obj, GAS_NAME_DIVERGENCE);
    SIM_ScalarField* PRS = getScalarField(obj, GAS_NAME_PRESSURE);
    SIM_IndexField* MARKER = getIndexField(obj, GAS_NAME_STENCIL);

    if (!HinaFlow::CHECK_NOT_NULL(D, T, V, S, DIV, PRS, MARKER))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    if (!HinaFlow::CHECK_THE_SAME_DIMENSION(D, T, V, S, DIV, PRS, MARKER))
    {
        addError(obj, SIM_MESSAGE, "GAS fields have different dimensions", UT_ERROR_FATAL);
        return false;
    }

    if (!HinaFlow::CHECK_IS_FACE_SAMPLED(V))
    {
        addError(obj, SIM_MESSAGE, "Velocity field is not face sampled", UT_ERROR_FATAL);
        return false;
    }

    return true;
}
