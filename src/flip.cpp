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
#include "poisson.h"

std::array<SIM_RawField, 3> HinaFlow::FLIP::FLOW_CACHE;

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

    THREADED_METHOD4(, EX_INDEX_OUTPUT->shouldMultiThread(), KnBuildExtrapolateIndex, SIM_RawIndexField *, EX_INDEX_OUTPUT, const SIM_RawIndexField*, EX_INDEX_INPUT, const SIM_IndexField*, MARKER, const int, layer);

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
                if (count)
                    vit.setValue(sum / static_cast<float>(count));
                else
                    vit.setValue(0); // TODO: check here
            }
        }
    }

    THREADED_METHOD4(, EX_INDEX->shouldMultiThread(), KnExtrapolate, SIM_RawField*, FIELD, const SIM_RawIndexField*, EX_INDEX, const int, distance, const int, AXIS);


    void KnBuildMarkerPartial(SIM_IndexField* MARKER, const SIM_VectorField* FLOW, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorI vit;
        vit.setArray(MARKER->getField()->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            if (FLOW->getCellValue(vit.x(), vit.y(), vit.z()).length2() > 0)
                vit.setValue(static_cast<exint>(CellType::Fluid));
            else
                vit.setValue(static_cast<exint>(CellType::Empty));
        }
    }

    THREADED_METHOD2(, MARKER->getField()->shouldMultiThread(), KnBuildMarker, SIM_IndexField*, MARKER, const SIM_VectorField*, FLOW);

    static void BuildMarker(SIM_IndexField* MARKER, const SIM_VectorField* FLOW)
    {
        KnBuildMarker(MARKER, FLOW);
    }

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

    static void BuildWeight(SIM_VectorField* WEIGHT, SIM_VectorField* FLOW, GU_Detail* in_gdp)
    {
        GU_Detail& gdp = *in_gdp;
        POINT_ATTRIBUTE_V3(v)
        for (const int AXIS : GET_AXIS_ITER(FLOW))
        {
            SIM_RawField* WEIGHT_RAW = nullptr;
            if (WEIGHT)
                WEIGHT_RAW = WEIGHT->getField(AXIS);
            else
                WEIGHT_RAW = new SIM_RawField(*FLOW->getField(AXIS));
            FLOW->getField(AXIS)->makeConstant(0);

            const auto voxvol = static_cast<float>(WEIGHT_RAW->getVoxelVolume());
            GA_Offset pt_off;
            GA_FOR_ALL_PTOFF(in_gdp, pt_off)
            {
                const UT_Vector3 pos = in_gdp->getPos3(pt_off);
                const UT_Vector3 vel = v_handle.get(pt_off);
                const UT_Vector3I cell = WEIGHT_RAW->posToIndex(pos);
                const UT_Vector3 center = WEIGHT_RAW->indexToPos(cell);
                const UT_Vector3 delta = pos - center;

                std::vector<UT_Vector3> cells;
                cells.resize(8, cell);
                for (int i = 1; i < 8; ++i)
                {
                    if (i & 0x1)
                        cells[i].x() += delta[0] < 0 ? -1 : 1;
                    else if (i & 0x2)
                        cells[i].y() += delta[1] < 0 ? -1 : 1;
                    else if (i & 0x4)
                        cells[i].z() += delta[2] < 0 ? -1 : 1;
                }

                if (const auto dim = GET_AXIS_ITER(FLOW).size(); dim == 3)
                {
                    float w000 = delta.x() * delta.y() * delta.z() / voxvol;
                    float w001 = (1 - delta.x()) * delta.y() * delta.z() / voxvol;
                    float w010 = delta.x() * (1 - delta.y()) * delta.z() / voxvol;
                    float w011 = (1 - delta.x()) * (1 - delta.y()) * delta.z() / voxvol;
                    float w100 = delta.x() * delta.y() * (1 - delta.z()) / voxvol;
                    float w101 = (1 - delta.x()) * delta.y() * (1 - delta.z()) / voxvol;
                    float w110 = delta.x() * (1 - delta.y()) * (1 - delta.z()) / voxvol;
                    float w111 = (1 - delta.x()) * (1 - delta.y()) * (1 - delta.z()) / voxvol;

                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[0]))
                        w000 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[1]))
                        w001 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[2]))
                        w010 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[3]))
                        w011 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[4]))
                        w100 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[5]))
                        w101 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[6]))
                        w110 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[7]))
                        w111 = 0;

                    const float sum = w000 + w001 + w010 + w011 + w100 + w101 + w110 + w111;
                    if (sum > 0)
                    {
                        if (w000 > 0)
                        {
                            const float weight = w000 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[0], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[0]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[0], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[0]) + weight * vel[AXIS]);
                        }
                        if (w001 > 0)
                        {
                            const float weight = w001 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[1], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[1]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[1], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[1]) + weight * vel[AXIS]);
                        }
                        if (w010 > 0)
                        {
                            const float weight = w010 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[2], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[2]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[2], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[2]) + weight * vel[AXIS]);
                        }
                        if (w011 > 0)
                        {
                            const float weight = w011 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[3], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[3]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[3], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[3]) + weight * vel[AXIS]);
                        }
                        if (w100 > 0)
                        {
                            const float weight = w100 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[4], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[4]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[4], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[4]) + weight * vel[AXIS]);
                        }
                        if (w101 > 0)
                        {
                            const float weight = w101 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[5], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[5]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[5], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[5]) + weight * vel[AXIS]);
                        }
                        if (w110 > 0)
                        {
                            const float weight = w110 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[6], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[6]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[6], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[6]) + weight * vel[AXIS]);
                        }
                        if (w111 > 0)
                        {
                            const float weight = w111 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[7], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[7]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[7], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[7]) + weight * vel[AXIS]);
                        }
                    }
                }
                else if (dim == 2)
                {
                    const int AXIS1 = GET_AXIS_ITER(FLOW)[0];
                    const int AXIS2 = GET_AXIS_ITER(FLOW)[1];
                    float w00 = delta[AXIS1] * delta[AXIS2] / voxvol;
                    float w01 = (1 - delta[AXIS1]) * delta[AXIS2] / voxvol;
                    float w10 = delta[AXIS1] * (1 - delta[AXIS2]) / voxvol;
                    float w11 = (1 - delta[AXIS1]) * (1 - delta[AXIS2]) / voxvol;

                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[0]))
                        w00 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[1 << AXIS1]))
                        w01 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[1 << AXIS2]))
                        w10 = 0;
                    if (!CHECK_CELL_VALID(WEIGHT_RAW, cells[1 << AXIS1 | 1 << AXIS2]))
                        w11 = 0;

                    const float sum = w00 + w01 + w10 + w11;
                    if (sum > 0)
                    {
                        if (w00 > 0)
                        {
                            const float weight = w00 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[0], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[0]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[0], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[0]) + weight * vel[AXIS]);
                        }
                        if (w01 > 0)
                        {
                            const float weight = w01 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[1 << AXIS1], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[1 << AXIS1]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[1 << AXIS1], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[1 << AXIS1]) + weight * vel[AXIS]);
                        }
                        if (w10 > 0)
                        {
                            const float weight = w10 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[1 << AXIS2], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[1 << AXIS2]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[1 << AXIS2], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[1 << AXIS2]) + weight * vel[AXIS]);
                        }
                        if (w11 > 0)
                        {
                            const float weight = w11 / sum;
                            SIM::FieldUtils::setFieldValue(*WEIGHT_RAW, cells[1 << AXIS1 | 1 << AXIS2], SIM::FieldUtils::getFieldValue(*WEIGHT_RAW, cells[1 << AXIS1 | 1 << AXIS2]) + weight);
                            SIM::FieldUtils::setFieldValue(*FLOW->getField(AXIS), cells[1 << AXIS1 | 1 << AXIS2], SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), cells[1 << AXIS1 | 1 << AXIS2]) + weight * vel[AXIS]);
                        }
                    }
                }
                else
                    throw std::runtime_error("Invalid dimension");
            }

            if (!WEIGHT)
                delete WEIGHT_RAW;
        }
    }

    static void Extrapolate(SIM_VectorField* FLOW, const SIM_IndexField* MARKER, SIM_IndexField* EX_INDEX, const int distance)
    {
        SIM_RawIndexField* EX_INDEX_RAW = nullptr;
        if (EX_INDEX)
            EX_INDEX_RAW = EX_INDEX->getField();
        else
            EX_INDEX_RAW = new SIM_RawIndexField(*MARKER->getField());
        EX_INDEX_RAW->makeConstant(100); // TODO: check here
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
        if (!EX_INDEX)
            delete EX_INDEX_RAW;
    }
}

void HinaFlow::FLIP::P2G(const Input& input, const Param& param, Result& result)
{
    for (const int i : GET_AXIS_ITER(input.FLOW))
        FLOW_CACHE[i] = SIM_RawField(*input.FLOW->getField(i));
    Internal::FLIP::BuildWeight(result.WEIGHT, input.FLOW, input.gdp);
    Internal::FLIP::BuildMarker(input.MARKER, input.gdp);
    Internal::FLIP::Extrapolate(input.FLOW, input.MARKER, result.EX_INDEX, param.extrapolate_depth);
}

void HinaFlow::FLIP::SolvePressure(const Input& input, const Param& param, Result& result)
{
    Internal::FLIP::BuildMarker(input.MARKER, input.FLOW);
    input.FLOW->enforceBoundary();
    Possion::Input I{input.FLOW, input.MARKER};
    Possion::Param P;
    Possion::Result R{input.FLOW, result.PRESSURE, result.DIVERGENCE};
    Possion::SolveMultiThreaded(I, P, R);
    input.FLOW->enforceBoundary();
}

void HinaFlow::FLIP::G2P(const Input& input, const Param& param, Result& result)
{
    Internal::FLIP::Extrapolate(input.FLOW, input.MARKER, result.EX_INDEX, param.extrapolate_depth);

    GU_Detail& gdp = *input.gdp;
    POINT_ATTRIBUTE_V3(v)
    GA_Offset pt_off;
    GA_FOR_ALL_PTOFF(input.gdp, pt_off)
    {
        const UT_Vector3 pos = gdp.getPos3(pt_off);
        UT_Vector3 v = input.FLOW->getValue(pos);
        UT_Vector3 delta = v - UT_Vector3F{
            static_cast<float>(FLOW_CACHE[0].getValue(pos)),
            static_cast<float>(FLOW_CACHE[1].getValue(pos)),
            static_cast<float>(FLOW_CACHE[2].getValue(pos))
        };
        UT_Vector3 vel = v_handle.get(pt_off);
        vel = param.ratio * (vel + delta) + (1 - param.ratio) * v;
    }
}
