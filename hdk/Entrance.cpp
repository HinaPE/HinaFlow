#include <UT/UT_DSOVersion.h> // Very Important!!! Include this first

#include "GAS_SolvePoisson.h"
#include "GAS_SolveDiffusion.h"
#include "GAS_SolveWave.h"

void initializeSIM(void *)
{
    IMPLEMENT_DATAFACTORY(GAS_SolvePoisson)
    IMPLEMENT_DATAFACTORY(GAS_SolveDiffusion)
    IMPLEMENT_DATAFACTORY(GAS_SolveWave)
}
