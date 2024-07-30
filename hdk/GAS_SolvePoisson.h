#ifndef GAS_SOLVEPOISSON_H
#define GAS_SOLVEPOISSON_H


#include <GAS/GAS_SubSolver.h>

class GAS_SolvePoisson : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "SolvePoisson";
    inline static const char* DOP_ENGLISH = "Solve Poisson";
    inline static const char* DATANAME = "SolvePoisson";
    inline static const bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("PCG_METHOD", PCG_METHOD)
    GETSET_DATA_FUNCS_B("MultiThreaded", MultiThreaded)

protected:
    explicit GAS_SolvePoisson(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_SolvePoisson, GAS_SubSolver, "This is a Solve Poisson Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_SOLVEPOISSON_H
