#ifndef GAS_RAYINTERSECTVISUALIZER_H
#define GAS_RAYINTERSECTVISUALIZER_H

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


#include <GAS/GAS_SubSolver.h>

class GAS_RayIntersectVisualizer final : public GAS_SubSolver
{
public:
    static constexpr bool GEN_NODE = true;
    inline static auto DOP_NAME = "RayIntersectVisualizer";
    inline static auto DOP_ENGLISH = "Ray Intersect Visualizer";
    inline static auto DATANAME = "RayIntersectVisualizer";
    static constexpr bool UNIQUE_DATANAME = false;

    GETSET_DATA_FUNCS_V3("Origin", Origin)
    GETSET_DATA_FUNCS_V3("Direction", Direction)
    GETSET_DATA_FUNCS_F("Step", Step)

protected:
    explicit GAS_RayIntersectVisualizer(const SIM_DataFactory* factory): BaseClass(factory) {}
    bool solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep) override;
    SIM_Guide* createGuideObjectSubclass() const override;
    void buildGuideGeometrySubclass(const SIM_RootData& root, const SIM_Options& options, const GU_DetailHandle& gdh, UT_DMatrix4* xform, const SIM_Time& t) const override;
    static const SIM_DopDescription* getDopDescription();
    DECLARE_STANDARD_GETCASTTOTYPE();
    DECLARE_DATAFACTORY(GAS_RayIntersectVisualizer, GAS_SubSolver, "This is a Ray Intersect Visualizer provided by HinaFlow.", getDopDescription());
};

#endif //GAS_RAYINTERSECTVISUALIZER_H
