#include "tomography.h"

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

void KnInitDensityPartial(SIM_RawField* TARGET, const SIM_VectorField* IMAGE, const VGEO_Ray& view, const UT_JobInfo& info)
{
    UT_VoxelArrayIteratorF vit;
    vit.setArray(TARGET->fieldNC());
    vit.setCompressOnExit(true);
    vit.setPartialRange(info.job(), info.numJobs());

    UT_QuaternionF q;
    q.updateFromVectors(-view.getD(), UT_Vector3(0, 0, 1));
    for (vit.rewind(); !vit.atEnd(); vit.advance())
    {
        const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
        const UT_Vector3 pos = TARGET->indexToPos(cell);
        const UT_Vector3 rot_pos = q.rotate(pos);
        vit.setValue(IMAGE->getValue({rot_pos.x(), rot_pos.y(), 0}).x());
    }
}

THREADED_METHOD3(, TARGET->shouldMultiThread(), KnInitDensity, SIM_RawField*, TARGET, const SIM_VectorField*, IMAGE, const VGEO_Ray&, view);

void KnIntersectionPartial(SIM_RawField* TARGET, const SIM_RawField* T1, const SIM_RawField* T2, const SIM_RawField* T3, const SIM_RawField* T4, const float threshold, const UT_JobInfo& info)
{
    UT_VoxelArrayIteratorF vit;
    vit.setArray(TARGET->fieldNC());
    vit.setCompressOnExit(true);
    vit.setPartialRange(info.job(), info.numJobs());

    for (vit.rewind(); !vit.atEnd(); vit.advance())
    {
        const float v1 = T1->field()->getValue(vit.x(), vit.y(), vit.z());
        const float v2 = T2->field()->getValue(vit.x(), vit.y(), vit.z());
        const float v3 = T3->field()->getValue(vit.x(), vit.y(), vit.z());
        const float v4 = T4->field()->getValue(vit.x(), vit.y(), vit.z());
        if (v1 < threshold || v2 < threshold || v3 < threshold || v4 < threshold)
            vit.setValue(0.f);
        else
            vit.setValue((v1 + v2 + v3 + v4) / 4.f);
    }
}

THREADED_METHOD6(, T1->shouldMultiThread(), KnIntersection, SIM_RawField*, TARGET, const SIM_RawField*, T1, const SIM_RawField*, T2, const SIM_RawField*, T3, const SIM_RawField*, T4, const float, threshold);

void HinaFlow::Tomography::Solve(const Input& input, const Param& param, Result& result)
{
    std::vector<std::pair<int, int>> view;
    view.emplace_back(static_cast<int>(input.VIEW1->getDivisions().x()), static_cast<int>(input.VIEW1->getDivisions().y()));
    if (input.VIEW2)
        view.emplace_back(static_cast<int>(input.VIEW2->getDivisions().x()), static_cast<int>(input.VIEW2->getDivisions().y()));
    if (input.VIEW3)
        view.emplace_back(static_cast<int>(input.VIEW3->getDivisions().x()), static_cast<int>(input.VIEW3->getDivisions().y()));
    if (input.VIEW4)
        view.emplace_back(static_cast<int>(input.VIEW4->getDivisions().x()), static_cast<int>(input.VIEW4->getDivisions().y()));

    T1 = *result.TARGET->getField();
    T2 = *result.TARGET->getField();
    T3 = *result.TARGET->getField();
    T4 = *result.TARGET->getField();
    T1.makeConstant(0.f);
    T2.makeConstant(0.f);
    T3.makeConstant(0.f);
    T4.makeConstant(0.f);
    {
        UT_Vector3 center = param.focus - param.pos1;
        UT_Vector3 dir = param.pos1 - param.focus;
        dir.normalize();
        KnInitDensity(&T1, input.VIEW1, {center, dir});
    }
    if (input.VIEW2)
    {
        UT_Vector3 center = param.focus - param.pos2;
        UT_Vector3 dir = param.pos2 - param.focus;
        dir.normalize();
        KnInitDensity(&T2, input.VIEW2, {center, dir});
    }
    if (input.VIEW3)
    {
        UT_Vector3 center = param.focus - param.pos3;
        UT_Vector3 dir = param.pos3 - param.focus;
        dir.normalize();
        KnInitDensity(&T3, input.VIEW3, {center, dir});
    }
    if (input.VIEW4)
    {
        UT_Vector3 center = param.focus - param.pos4;
        UT_Vector3 dir = param.pos4 - param.focus;
        dir.normalize();
        KnInitDensity(&T4, input.VIEW4, {center, dir});
    }

    KnIntersection(result.TARGET->getField(), &T1, &T2, &T3, &T4, param.threshold);
}
