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

namespace HinaFlow::Internal::FLIP
{
    void KnBuildExtrapolateIndexPartial(SIM_RawIndexField* EX_INDEX_OUTPUT, const SIM_RawIndexField* EX_INDEX_INPUT, const SIM_IndexField* MARKER, const int layer, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorI vit;
        vit.setArray(EX_INDEX_OUTPUT->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            if (layer == 0)
            {
                if (CHECK_CELL_TYPE<CellType::Fluid>(MARKER, cell))
                    vit.setValue(0);
            }
            else
            {
                if (SIM::FieldUtils::getFieldValue(*EX_INDEX_INPUT, cell) < layer)
                    continue;
                for (const int AXIS : GET_AXIS_ITER(EX_INDEX_INPUT))
                {
                    for (const int DIR : {0, 1})
                    {
                        const UT_Vector3I cell1 = SIM::FieldUtils::cellToCellMap(cell, AXIS, DIR);
                        if (CHECK_CELL_VALID(EX_INDEX_INPUT, cell1) && SIM::FieldUtils::getFieldValue(*EX_INDEX_INPUT, cell1) == layer - 1)
                            vit.setValue(layer);
                    }
                }
            }
        }
    }

    THREADED_METHOD4(, EX_INDEX_OUTPUT->shouldMultiThread(), KnBuildExtrapolateIndex, SIM_RawIndexField *, EX_INDEX_OUTPUT, const SIM_RawIndexField* ,EX_INDEX_INPUT, const SIM_IndexField*, MARKER, const int, layer);

    void KnExtrapolatePartial(SIM_RawField* FIELD, const SIM_RawIndexField* EX_INDEX, const int distance, const int AXIS, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(FIELD->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I face(vit.x(), vit.y(), vit.z());
            const UT_Vector3I cell1 = SIM::FieldUtils::faceToCellMap(face, AXIS, 0);
            const UT_Vector3I cell2 = SIM::FieldUtils::faceToCellMap(face, AXIS, 1);

            const bool cell1_valid = CHECK_CELL_VALID(EX_INDEX, cell1);
            const bool cell2_valid = CHECK_CELL_VALID(EX_INDEX, cell2);

            if (cell1_valid && SIM::FieldUtils::getFieldValue(*EX_INDEX, cell1) == distance || cell2_valid && SIM::FieldUtils::getFieldValue(*EX_INDEX, cell2) == distance)
            {
                float sum = 0;
                int count = 0;
                for (const int A_AXIS : GET_AXIS_ITER(FIELD))
                {
                    for (const int DIR : {0, 1})
                    {
                        const UT_Vector3I face_around = SIM::FieldUtils::cellToCellMap(face, A_AXIS, DIR);
                        const UT_Vector3I cell1_around = SIM::FieldUtils::faceToCellMap(face_around, AXIS, 0);
                        const UT_Vector3I cell2_around = SIM::FieldUtils::faceToCellMap(face_around, AXIS, 1);

                        const bool cell1_around_valid = CHECK_CELL_VALID(EX_INDEX, cell1_around);
                        const bool cell2_around_valid = CHECK_CELL_VALID(EX_INDEX, cell2_around);

                        if (cell1_valid && cell1_around_valid && SIM::FieldUtils::getFieldValue(*EX_INDEX, cell1_around) == distance - 1
                            || cell2_valid && cell2_around_valid && SIM::FieldUtils::getFieldValue(*EX_INDEX, cell2_around) == distance - 1)
                        {
                            sum += SIM::FieldUtils::getFieldValue(*FIELD, face_around);
                            ++count;
                        }
                    }
                }
                vit.setValue(sum / static_cast<float>(count));
            }
        }
    }

    THREADED_METHOD4(, EX_INDEX->shouldMultiThread(), KnExtrapolate, SIM_RawField*, FIELD, const SIM_RawIndexField*, EX_INDEX, const int, distance, const int, AXIS);

    static void BuildMarker(SIM_IndexField* MARKER, const GU_Detail* gdp)
    {
        MARKER->getField()->makeConstant(static_cast<exint>(CellType::Empty));
        GA_Offset pt_off;
        GA_FOR_ALL_PTOFF(gdp, pt_off)
        {
            const UT_Vector3 pos = gdp->getPos3(pt_off);
            SIM::FieldUtils::setFieldValue(*MARKER->getField(), MARKER->getField()->posToIndex(pos), static_cast<exint>(CellType::Fluid));
        }
    }

    static void Extrapolate(SIM_VectorField* FLOW, const SIM_IndexField* MARKER, SIM_IndexField* EX_INDEX, const int distance)
    {
        SIM_RawIndexField* EX_INDEX_RAW;
        if (EX_INDEX)
            EX_INDEX_RAW = EX_INDEX->getField();
        else
            EX_INDEX_RAW = new SIM_RawIndexField(*MARKER->getField());
        EX_INDEX_RAW->makeConstant(100); // TODO:
        for (int layer = 0; layer <= distance; ++layer)
        {
            SIM_RawIndexField TEMP(*EX_INDEX_RAW);
            KnBuildExtrapolateIndex(EX_INDEX_RAW, &TEMP, MARKER, layer);
        }
        for (const int AXIS : GET_AXIS_ITER(FLOW))
        {
            for (int layer = 1; layer < distance; ++layer)
                KnExtrapolate(FLOW->getField(AXIS), EX_INDEX_RAW, layer, AXIS);
        }
    }
}

void HinaFlow::FLIP::P2G(const Input& input, const Param& param, Result& result)
{
    Internal::FLIP::BuildMarker(input.MARKER, input.gdp);
    Internal::FLIP::Extrapolate(input.FLOW, input.MARKER, result.EX_INDEX, param.extrapolate_depth);
}

void HinaFlow::FLIP::G2P(const Input& input, const Param& param, Result& result)
{
}
