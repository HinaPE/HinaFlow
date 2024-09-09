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

void KnInitDensityPartial(SIM_RawField* TARGET, const SIM_VectorField* IMAGE, const UT_JobInfo& info)
{
    UT_VoxelArrayIteratorF vit;
    vit.setArray(TARGET->fieldNC());
    vit.setCompressOnExit(true);
    vit.setPartialRange(info.job(), info.numJobs());

    for (vit.rewind(); !vit.atEnd(); vit.advance())
    {
        const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
        const UT_Vector3 pos = TARGET->indexToPos(cell);
        vit.setValue(IMAGE->getValue({pos.x(), pos.y(), 0}).x());

        // TODO: support any view angle
    }
}

THREADED_METHOD2(, TARGET->shouldMultiThread(), KnInitDensity, SIM_RawField*, TARGET, const SIM_VectorField*, IMAGE);

void KnIntersectionPartial(SIM_RawField* TARGET, const SIM_RawField* T1, const SIM_RawField* T2, const SIM_RawField* T3, const SIM_RawField* T4, const UT_JobInfo& info)
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
        if (const float res = (v1 + v2 + v3 + v4) / 4.f; res < 0.1f)
            vit.setValue(0.f);
        else
            vit.setValue(res);
    }
}

THREADED_METHOD5(, T1->shouldMultiThread(), KnIntersection, SIM_RawField*, TARGET, const SIM_RawField*, T1, const SIM_RawField*, T2, const SIM_RawField*, T3, const SIM_RawField*, T4);

void HinaFlow::Tomography::Solve(const Input& input, const Param& param, Result& result)
{
    std::vector<std::pair<int, int>> view;
    view.emplace_back(input.VIEW1->getDivisions().x(), input.VIEW1->getDivisions().y());
    if (input.VIEW2)
        view.emplace_back(input.VIEW2->getDivisions().x(), input.VIEW2->getDivisions().y());
    if (input.VIEW3)
        view.emplace_back(input.VIEW3->getDivisions().x(), input.VIEW3->getDivisions().y());
    if (input.VIEW4)
        view.emplace_back(input.VIEW4->getDivisions().x(), input.VIEW4->getDivisions().y());

    SIM_RawField T1(*result.TARGET->getField());
    SIM_RawField T2(*result.TARGET->getField());
    SIM_RawField T3(*result.TARGET->getField());
    SIM_RawField T4(*result.TARGET->getField());
    KnInitDensity(&T1, input.VIEW1);
    if (input.VIEW2)
        KnInitDensity(&T2, input.VIEW2);
    if (input.VIEW3)
        KnInitDensity(&T3, input.VIEW3);
    if (input.VIEW4)
        KnInitDensity(&T4, input.VIEW4);

    KnIntersection(result.TARGET->getField(), &T1, &T2, &T3, &T4);
}
