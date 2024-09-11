#include "image.h"

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

void KnWriteFieldPartial(SIM_RawField* TARGET, const std::vector<std::vector<float>>& CACHE, const UT_JobInfo& info)
{
    UT_VoxelArrayIteratorF vit;
    vit.setArray(TARGET->fieldNC());
    vit.setCompressOnExit(true);
    vit.setPartialRange(info.job(), info.numJobs());

    for (vit.rewind(); !vit.atEnd(); vit.advance())
    {
        const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
        vit.setValue(CACHE[cell[0]][cell[1]]);
    }
}

THREADED_METHOD2(, TARGET->shouldMultiThread(), KnWriteField, SIM_RawField*, TARGET, const std::vector<std::vector<float>>&, CACHE);


void KnResamplePartial(SIM_RawField* TARGET, const SIM_RawField* SOURCE, const UT_JobInfo& info)
{
    UT_VoxelArrayIteratorF vit;
    vit.setArray(TARGET->fieldNC());
    vit.setCompressOnExit(true);
    vit.setPartialRange(info.job(), info.numJobs());

    for (vit.rewind(); !vit.atEnd(); vit.advance())
    {
        const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
        const UT_Vector3& pos = TARGET->indexToPos(cell);
        vit.setValue(static_cast<float>(SOURCE->getValue(pos)));
    }
}

THREADED_METHOD2(, TARGET->shouldMultiThread(), KnResample, SIM_RawField*, TARGET, const SIM_RawField*, SOURCE);

void HinaFlow::Image::Render(SIM_VectorField* TARGET, const SIM_ScalarField* FIELD, const VGEO_Ray& view, const float step, const float coeff)
{
    const float width = TARGET->getSize().x();
    const float height = TARGET->getSize().y();
    const int resx = TARGET->getXField()->field()->getXRes();
    const int resy = TARGET->getXField()->field()->getYRes();

    UT_Vector2 delta = {width / static_cast<float>(resx), height / static_cast<float>(resy)};
    UT_BoundingBox bbox;
    FIELD->getBBox(bbox);

    const UT_Vector3 center = view.getP();
    UT_Vector3 right_dir = cross(view.getD(), UT_Vector3{0, 1, 0});
    UT_Vector3 up_dir = cross(right_dir, view.getD());
    right_dir.normalize();
    up_dir.normalize();

    if (view.getD() == UT_Vector3{0, -1, 0} || view.getD() == UT_Vector3{0, 1, 0})
    {
        right_dir = UT_Vector3{1, 0, 0};
        up_dir = UT_Vector3{0, 0, -1};
    }

    static std::vector<std::vector<float>> cache;
    cache.resize(resx);
    for (int i = 0; i < resx; ++i)
        cache[i].resize(resy);

    const UT_BlockedRange2D range(0, resy, 0, resx);
    const UT_Vector2i offset = {resy / 2, resx / 2};
    UTparallelFor(range, [&](const UT_BlockedRange2D<int>& r)
    {
        for (int i = r.rows().begin(); i < r.rows().end(); ++i)
        {
            for (int j = r.cols().begin(); j < r.cols().end(); ++j)
            {
                const VGEO_Ray ray{UT_Vector3{center + up_dir * delta.x() * (i - offset.x()) + right_dir * delta.y() * (j - offset.y())}, view.getD()};
                float tmin = 0, tmax = std::numeric_limits<float>::max();
                const bool intersect = ray.getBoxRange(bbox, tmin, tmax);
                float cur = tmin + step / 2.f;
                float res = 0;
                if (intersect)
                {
                    int sum = 0;
                    while (cur < tmax)
                    {
                        const UT_Vector3 pt = ray.getPt(cur);
                        res += static_cast<float>(FIELD->getValue(pt));
                        cur += step;
                        ++sum;
                    }
                    if (sum > 0)
                        res /= static_cast<float>(sum);
                }
                cache[j][i] = coeff * res;
            }
        }
    });

    KnWriteField(TARGET->getXField(), cache);
    KnWriteField(TARGET->getYField(), cache);
    KnWriteField(TARGET->getZField(), cache);
}

void HinaFlow::Image::RenderBTB(SIM_VectorField* TARGET, const SIM_ScalarField* FIELD, const VGEO_Ray& view, const float step, const float coeff)
{
    // TODO: ensure TARGET's res is the same as FIELD's res
    // maybe no need?
    const float width = TARGET->getSize().x();
    const float height = TARGET->getSize().y();
    const int resx = TARGET->getXField()->field()->getXRes();
    const int resy = TARGET->getXField()->field()->getYRes();
    const int resz = static_cast<int>(FIELD->getSize().z() / step);

    UT_SparseMatrixF A(resx * resy, resx * resy * resz);
    UT_VectorF x(0, resx * resy * resz - 1);
    UT_VectorF b(0, resx * resy - 1);

    // calculate directions
    UT_BoundingBox bbox;
    UT_Vector3 center;
    UT_Vector3 right_dir;
    UT_Vector3 up_dir;
    {
        FIELD->getBBox(bbox);
        center = view.getP();
        right_dir = cross(view.getD(), UT_Vector3{0, 1, 0});
        up_dir = cross(right_dir, view.getD());
        right_dir.normalize();
        up_dir.normalize();
        if (view.getD() == UT_Vector3{0, -1, 0} || view.getD() == UT_Vector3{0, 1, 0})
        {
            right_dir = UT_Vector3{1, 0, 0};
            up_dir = UT_Vector3{0, 0, -1};
        }
    }


    const UT_BlockedRange2D range(0, resy, 0, resx);
    const UT_Vector2i offset = {resy / 2, resx / 2};
    const UT_Vector2 delta = {width / static_cast<float>(resx), height / static_cast<float>(resy)};
    UTserialFor(range, [&](const UT_BlockedRange2D<int>& r)
    {
        for (int i = r.rows().begin(); i < r.rows().end(); ++i)
        {
            for (int j = r.cols().begin(); j < r.cols().end(); ++j)
            {
                const int idx1 = i * resx + j;

                const VGEO_Ray ray{UT_Vector3{center + up_dir * delta.x() * (i - offset.x()) + right_dir * delta.y() * (j - offset.y())}, view.getD()};
                float tmin = 0, tmax = std::numeric_limits<float>::max();
                const bool intersect = ray.getBoxRange(bbox, tmin, tmax);
                float cur = tmin + step / 2.f;
                if (intersect)
                {
                    int k = 0;
                    while (cur < tmax)
                    {
                        const UT_Vector3 pt = ray.getPt(cur);
                        const auto value = static_cast<float>(FIELD->getValue(pt));

                        const int idx2 = i * resx + j + k * resx * resy;
                        A.addToElement(idx1, idx2, 1.f);
                        if (idx2 >= 0 && idx2 < resx * resy * resz)
                            x(idx2) = value;
                        else
                            printf("idx2: %d, resx: %d, resy: %d, resz: %d, i: %d, j: %d, k: %d\n", idx2, resx, resy, resz, i, j, k);
                        cur += step;
                        ++k;
                    }
                }
            }
        }
    });
    A.compile();
    // UT_SparseMatrixRowF AImpl;
    // AImpl.buildFrom(A);
    // AImpl.multVec(x, b);

    // {
    //     UT_VoxelArrayIteratorF vit;
    //     vit.setArray(TARGET->getXField()->fieldNC());
    //     for (vit.rewind(); !vit.atEnd(); vit.advance())
    //     {
    //         const int idx1 = vit.y() * resx + vit.x();
    //         vit.setValue(b(idx1));
    //     }
    //     vit.setArray(TARGET->getYField()->fieldNC());
    //     for (vit.rewind(); !vit.atEnd(); vit.advance())
    //     {
    //         const int idx1 = vit.y() * resx + vit.x();
    //         vit.setValue(b(idx1));
    //     }
    //     vit.setArray(TARGET->getZField()->fieldNC());
    //     for (vit.rewind(); !vit.atEnd(); vit.advance())
    //     {
    //         const int idx1 = vit.y() * resx + vit.x();
    //         vit.setValue(b(idx1));
    //     }
    // }
}
