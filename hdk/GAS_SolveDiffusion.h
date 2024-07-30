#ifndef GAS_SOLVEDIFFUSION_H
#define GAS_SOLVEDIFFUSION_H


#include <GAS/GAS_SubSolver.h>

class GAS_SolveDiffusion : public GAS_SubSolver
{
public:
    inline static const bool GEN_NODE = true;
    inline static const char* DOP_NAME = "SolveDiffusion";
    inline static const char* DOP_ENGLISH = "Solve Diffusion";
    inline static const char* DATANAME = "SolveDiffusion";
    inline static const bool UNIQUE_DATANAME = false;

protected:
    explicit GAS_SolveDiffusion(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) final;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_SolveDiffusion, GAS_SubSolver, "This is a Solve Diffusion Solver provided by HinaFlow.", getDopDescription());
};


#endif //GAS_SOLVEDIFFUSION_H
