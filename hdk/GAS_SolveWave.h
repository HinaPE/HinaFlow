#ifndef GAS_SOLVEWAVE_H
#define GAS_SOLVEWAVE_H


#include <GAS/GAS_SubSolver.h>

class GAS_SolveWave : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "SolveWave";
    inline static const char* DOP_ENGLISH = "Solve Wave";
    inline static const char* DATANAME = "SolveWave";
    inline static const bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_I("PCG_METHOD", PCG_METHOD)
    GETSET_DATA_FUNCS_B("MultiThreaded", MultiThreaded)
    GETSET_DATA_FUNCS_F("Wave", Wave)

protected:
    explicit GAS_SolveWave(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_SolveWave, GAS_SubSolver, "This is a Solve Wave Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_SOLVEWAVE_H
