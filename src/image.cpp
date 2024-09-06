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

#include <BRAY/BRAY_Interface.h>

void HinaFlow::Image::Render(SIM_VectorField* TARGET, const SIM_ScalarField* FIELD, const VGEO_Ray& view, const float step, const int layer)
{
    const float width = TARGET->getSize().x();
    const float height = TARGET->getSize().y();
    const int resx = TARGET->getXField()->field()->getXRes();
    const int resy = TARGET->getXField()->field()->getYRes();

    UT_Vector2 delta = {width / static_cast<float>(resx), height / static_cast<float>(resy)};
    UT_BoundingBox bbox;
    FIELD->getBBox(bbox);

    const UT_Vector3 center = view.getP();
    UT_Vector3 right_dir = cross(view.getD(),UT_Vector3{0, 1, 0});
    UT_Vector3 up_dir = cross(right_dir, view.getD());
    right_dir.normalize();
    up_dir.normalize();

    const UT_BlockedRange2D range( 0, resx, 0, resy );
    const UT_Vector2i offset = {resx / 2, resy / 2};
    UTserialFor(range, [&](const UT_BlockedRange2D<int>& r)
    {
        for (int i = r.rows().begin(); i < r.rows().end(); ++i)
        {
            for (int j = r.cols().begin(); j < r.cols().end(); ++j)
            {
                const VGEO_Ray ray{UT_Vector3{center + right_dir * delta.x() * (i - offset.x()) + up_dir * delta.y() * (j - offset.y())}, view.getD()};
                float tmin = 0, tmax = std::numeric_limits<float>::max();
                const bool intersect = ray.getBoxRange(bbox, tmin, tmax);
                float cur = tmin;
                float res = 0;
                if (intersect)
                {
                    while (cur < tmax)
                    {
                        const UT_Vector3 pt = ray.getPt(cur);
                        res += static_cast<float>(FIELD->getValue(pt));
                        cur += step;
                    }
                }
                TARGET->getXField()->fieldNC()->setValue(i, j, layer, res);
                TARGET->getYField()->fieldNC()->setValue(i, j, layer, res);
                TARGET->getZField()->fieldNC()->setValue(i, j, layer, res);
            }
        }
    });
}
