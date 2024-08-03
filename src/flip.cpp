#include "flip.h"

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

void HinaFlow::FLIP::P2G(const Input& input, const Param& param, Result& result)
{
}

void HinaFlow::FLIP::G2P(const Input& input, const Param& param, Result& result)
{
}

namespace HinaFlow::Internal::FLIP
{
    void KnExtrapolatePartial(SIM_RawField* FIELD, SIM_RawIndexField* EX_INDEX, const int distance, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorI vit;
        vit.setArray(EX_INDEX->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            for (const int AXIS : GET_AXIS_ITER(EX_INDEX))
            {
                for (const int DIR : {0, 1})
                {
                }
            }
        }
    }

    THREADED_METHOD3(, EX_INDEX->shouldMultiThread(), KnExtrapolate, SIM_RawField*, FIELD, SIM_RawIndexField*, EX_INDEX, const int, distance);

    static void Extrapolate(const SIM_RawField* FIELD, const SIM_RawIndexField* EX_INDEX, const int distance)
    {
    }
}
