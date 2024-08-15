#include "GAS_SolverPhiFlow.h"

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
#include "python/phiflow_smoke.h"

const SIM_DopDescription* GAS_SolverPhiFlow::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_VECTOR_FIELD_1
    ACTIVATE_GAS_VECTOR_FIELD_2
    ACTIVATE_GAS_VECTOR_FIELD_3
    ACTIVATE_GAS_VECTOR_FIELD_4
    PARAMETER_INT(StartFrame, 1)
    PARAMETER_FILE(InitCode, "")
    PARAMETER_FILE(StepCode, "")
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

bool GAS_SolverPhiFlow::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    const int frame = engine.getSimulationFrame(time);
    if (frame < getStartFrame())
        return true;

    SIM_VectorField* V1 = getVectorField(obj, GAS_NAME_VECTOR_FIELD_1);
    SIM_VectorField* V2 = getVectorField(obj, GAS_NAME_VECTOR_FIELD_2);
    SIM_VectorField* V3 = getVectorField(obj, GAS_NAME_VECTOR_FIELD_3);
    SIM_VectorField* V4 = getVectorField(obj, GAS_NAME_VECTOR_FIELD_4);
    if (!V1 || !V2 || !V3 || !V4)
    {
        addError(obj, SIM_MESSAGE, "Missing fields", UT_ERROR_FATAL);
        return false;
    }

    if (frame == getStartFrame())
    {
        UT_WorkBuffer expr;
        expr.sprintf(R"(
from phi.torch.flow import *

DOMAIN = dict(x=80, y=64)
LEFT = StaggeredGrid(Box(x=(-INF, 40), y=None), 0, **DOMAIN)
RIGHT = StaggeredGrid(Box(x=(40, INF), y=None), 0, **DOMAIN)
TARGET = RIGHT * StaggeredGrid(lambda x: math.exp(-0.5 * math.vec_squared(x - (50, 10), 'vector') / 32**2), 0, **DOMAIN) * (0, 2)


def loss(v0, p0):
    v1, p = fluid.make_incompressible(v0 * LEFT, solve=Solve('CG-adaptive', 1e-5, x0=p0))
    return field.l2_loss((v1 - TARGET) * RIGHT), v1, p

eval_grad_v0 = field.functional_gradient(loss, 'v0', get_output=True)
p0 = gradient = incompressible_velocity = remaining_divergence = None
velocity_fit = StaggeredGrid(Noise(), 0, **DOMAIN) * 0.1 * LEFT
)");
        HinaFlow::Python::PhiFlowSmoke::CompileFunction(expr);


        V1->setSize(HinaFlow::Python::PhiFlowSmoke::FetchFieldSize("LEFT"));
        V1->setDivisions(HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution("LEFT"));
        V2->setSize(HinaFlow::Python::PhiFlowSmoke::FetchFieldSize("RIGHT"));
        V2->setDivisions(HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution("RIGHT"));
        V3->setSize(HinaFlow::Python::PhiFlowSmoke::FetchFieldSize("TARGET"));
        V3->setDivisions(HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution("TARGET"));
        V4->setSize(HinaFlow::Python::PhiFlowSmoke::FetchFieldSize("velocity_fit"));
        V4->setDivisions(HinaFlow::Python::PhiFlowSmoke::FetchFieldResolution("velocity_fit"));
    }
    else
    {
        UT_WorkBuffer expr;
        expr.sprintf(R"(
(loss, incompressible_velocity, pressure_guess), gradient = eval_grad_v0(velocity_fit, p0)
remaining_divergence = field.divergence(incompressible_velocity)
velocity_fit -= gradient
)");
        HinaFlow::Python::PhiFlowSmoke::CompileFunction(expr);
    }

    HinaFlow::Python::PhiFlowSmoke::FetchVectorField2D("LEFT", V1);
    HinaFlow::Python::PhiFlowSmoke::FetchVectorField2D("RIGHT", V2);
    HinaFlow::Python::PhiFlowSmoke::FetchVectorField2D("TARGET", V3);
    HinaFlow::Python::PhiFlowSmoke::FetchVectorField2D("velocity_fit", V4);

    return true;
}
