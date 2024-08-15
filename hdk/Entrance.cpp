#include <UT/UT_DSOVersion.h> // Very Important!!! Include this first
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>

#include "GAS_DebugSPHKernel.h"
#include "GAS_PhiFlowSmoke2D.h"
#include "GAS_PhiFlowSmoke3D.h"
#include "GAS_PhiFlowVisulizeField.h"
#include "GAS_SolveDiffusion.h"
#include "GAS_SolvePoisson.h"
#include "GAS_SolverFLIP.h"
#include "GAS_SolverPBF.h"
#include "GAS_SolverPhiFlow.h"
#include "GAS_SolveWave.h"

#include "SOP_Image2Volume.h"

void initializeSIM(void*)
{
    IMPLEMENT_DATAFACTORY(GAS_DebugSPHKernel)
    IMPLEMENT_DATAFACTORY(GAS_PhiFlowSmoke2D)
    IMPLEMENT_DATAFACTORY(GAS_PhiFlowSmoke3D)
    IMPLEMENT_DATAFACTORY(GAS_PhiFlowVisulizeField)
    IMPLEMENT_DATAFACTORY(GAS_SolveDiffusion)
    IMPLEMENT_DATAFACTORY(GAS_SolvePoisson)
    IMPLEMENT_DATAFACTORY(GAS_SolverFLIP)
    IMPLEMENT_DATAFACTORY(GAS_SolverPBF)
    IMPLEMENT_DATAFACTORY(GAS_SolverPhiFlow)
    IMPLEMENT_DATAFACTORY(GAS_SolveWave)
}


void newSopOperator(OP_OperatorTable* table)
{
    auto* op = new OP_Operator(
        SOP_Image2Volume::DOP_NAME, // Internal name
        SOP_Image2Volume::DOP_ENGLISH, // UI name
        SOP_Image2Volume::myConstructor, // How to build the SOP
        SOP_Image2Volume::buildTemplates(), // My parameters
        1, // Min # of sources
        2, // Max # of sources
        nullptr, // Custom local variables (none)
        OP_FLAG_GENERATOR); // Flag it as generator

    op->setOpTabSubMenuPath("HinaFlow");
    table->addOperator(op);
}
