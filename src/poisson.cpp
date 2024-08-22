#include "poisson.h"

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

void HinaFlow::Poisson::Solve(const Input& input, const Param& param, Result& result)
{
    const int size = static_cast<int>(input.MARKER->getField()->field()->numVoxels());
    const UT_Vector3I res = input.MARKER->getField()->getVoxelRes();
    const float h = input.MARKER->getVoxelSize().maxComponent();


    // Build A
    UT_SparseMatrixF A(size, size);
    {
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(input.MARKER->getField()->field());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            int idx = static_cast<int>(TO_1D_IDX(cell, res));
            if (!CHECK_CELL_TYPE<CellType::Fluid>(input.MARKER, cell))
                continue;

            for (const int AXIS : GET_AXIS_ITER(input.MARKER->getField()))
            {
                for (const int DIR : {0, 1})
                {
                    UT_Vector3I cell0 = SIM::FieldUtils::cellToCellMap(cell, AXIS, DIR);
                    int idx0 = static_cast<int>(TO_1D_IDX(cell0, res));

                    if (CHECK_CELL_VALID(input.MARKER->getField(), cell0))
                    {
                        A.addToElement(idx, idx, 1.0f);
                        A.addToElement(idx, idx0, -1.0f);
                    }
                }
            }
        }
    }
    A.compile();
    UT_SparseMatrixRowF AImpl;
    AImpl.buildFrom(A);
    UT_VectorF x(0, size - 1);


    // Build b (Store Divergence Optional)
    UT_VectorF b(0, size - 1);
    {
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(input.MARKER->getField()->field());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const auto idx = TO_1D_IDX(cell, res);

            fpreal32 divergence = 0;
            if (CHECK_CELL_TYPE<CellType::Fluid>(input.MARKER, cell))
            {
                for (const int AXIS : GET_AXIS_ITER(input.MARKER->getField()))
                {
                    constexpr int dir0 = 0, dir1 = 1;
                    const UT_Vector3I face0 = SIM::FieldUtils::cellToFaceMap(cell, AXIS, dir0);
                    const UT_Vector3I face1 = SIM::FieldUtils::cellToFaceMap(cell, AXIS, dir1);
                    const fpreal32 v0 = SIM::FieldUtils::getFieldValue(*input.FLOW->getField(AXIS), face0);
                    const fpreal32 v1 = SIM::FieldUtils::getFieldValue(*input.FLOW->getField(AXIS), face1);
                    divergence += (v1 - v0) * h;
                }
            }
            b(idx) = -divergence;

            if (result.DIVERGENCE)
                result.DIVERGENCE->getField()->fieldNC()->setValue(cell, divergence);
        }
    }


    // Solve System
    x = b;
    AImpl.solveConjugateGradient(x, b, nullptr);


    // Store Pressure
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(result.PRESSURE->getField()->fieldNC());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const auto idx = TO_1D_IDX(cell, result.PRESSURE->getField()->getVoxelRes());
            vit.setValue(x(idx));
        }
    }


    // Subtract Pressure Gradient
    for (const int AXIS : GET_AXIS_ITER(input.FLOW))
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(result.FLOW->getField(AXIS)->fieldNC());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I face(vit.x(), vit.y(), vit.z());
            constexpr int DIR_0 = 0, DIR_1 = 1;
            const UT_Vector3I cell0 = SIM::FieldUtils::faceToCellMap(face, AXIS, DIR_0);
            const UT_Vector3I cell1 = SIM::FieldUtils::faceToCellMap(face, AXIS, DIR_1);
            // This will clamp the bounds to fit within the voxel array, using the border type to resolve out of range values.
            const fpreal32 p0 = result.PRESSURE->getField()->field()->getValue(static_cast<int>(cell0.x()), static_cast<int>(cell0.y()), static_cast<int>(cell0.z()));
            // This will clamp the bounds to fit within the voxel array, using the border type to resolve out of range values.
            const fpreal32 p1 = result.PRESSURE->getField()->field()->getValue(static_cast<int>(cell1.x()), static_cast<int>(cell1.y()), static_cast<int>(cell1.z()));
            fpreal32 v = input.FLOW->getField(AXIS)->field()->getValue(vit.x(), vit.y(), vit.z());
            v -= (p1 - p0) / h;
            vit.setValue(v);
        }
    }
}


namespace HinaFlow::Internal::Poisson
{
    void KnBuildLaplaceMatrixPartial(UT_SparseMatrixF& A, const SIM_IndexField* MARKER, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(MARKER->getField()->field());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        const UT_Vector3I res = MARKER->getField()->getVoxelRes();

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const int idx = static_cast<int>(TO_1D_IDX(cell, res));
            if (!CHECK_CELL_TYPE<CellType::Fluid>(MARKER, cell))
                continue;

            for (const int AXIS : GET_AXIS_ITER(MARKER->getField()))
            {
                for (const int DIR : {0, 1})
                {
                    UT_Vector3I cell0 = SIM::FieldUtils::cellToCellMap(cell, AXIS, DIR);
                    const int idx0 = static_cast<int>(TO_1D_IDX(cell0, res));

                    if (CHECK_CELL_VALID(MARKER->getField(), cell0))
                    {
                        A.addToElement(idx, idx, 1.0f);
                        A.addToElement(idx, idx0, -1.0f);
                    }
                }
            }
        }
    }

    THREADED_METHOD2(, false /* DO NOT USE MULTI THREAD HERE */, KnBuildLaplaceMatrix, UT_SparseMatrixF&, A, const SIM_IndexField*, MARKER);


    void KnBuildRhsPartial(UT_VectorF& b, const SIM_VectorField* FLOW, const SIM_IndexField* MARKER, SIM_ScalarField* DIVERGENCE, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(MARKER->getField()->field());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        const UT_Vector3I res = MARKER->getField()->getVoxelRes();
        const float h = MARKER->getVoxelSize().maxComponent();

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const auto idx = TO_1D_IDX(cell, res);

            fpreal32 divergence = 0;
            if (CHECK_CELL_TYPE<CellType::Fluid>(MARKER, cell))
            {
                for (const int AXIS : GET_AXIS_ITER(MARKER->getField()))
                {
                    constexpr int dir0 = 0, dir1 = 1;
                    const UT_Vector3I face0 = SIM::FieldUtils::cellToFaceMap(cell, AXIS, dir0);
                    const UT_Vector3I face1 = SIM::FieldUtils::cellToFaceMap(cell, AXIS, dir1);
                    const fpreal32 v0 = SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), face0);
                    const fpreal32 v1 = SIM::FieldUtils::getFieldValue(*FLOW->getField(AXIS), face1);
                    divergence += (v1 - v0) * h;
                }
            }
            b(idx) = -divergence;

            if (DIVERGENCE)
                DIVERGENCE->getField()->fieldNC()->setValue(cell, divergence);
        }
    }

    THREADED_METHOD4(, MARKER->getField()->shouldMultiThread(), KnBuildRhs, UT_VectorF&, b, const SIM_VectorField*, FLOW, const SIM_IndexField*, MARKER, SIM_ScalarField*, DIVERGENCE);

    void KnStorePressurePartial(SIM_ScalarField* PRESSURE, const UT_VectorF& x, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(PRESSURE->getField()->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        const UT_Vector3I res = PRESSURE->getField()->getVoxelRes();

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const auto idx = TO_1D_IDX(cell, res);
            vit.setValue(x(idx));
        }
    }

    THREADED_METHOD2(, PRESSURE->getField()->shouldMultiThread(), KnStorePressure, SIM_ScalarField*, PRESSURE, const UT_VectorF&, x);

    void KnSubtractPressureGradientPartial(SIM_VectorField* FLOW, const SIM_ScalarField* PRESSURE, const int AXIS, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(FLOW->getField(AXIS)->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        const float h = PRESSURE->getVoxelSize().maxComponent();

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I face(vit.x(), vit.y(), vit.z());
            constexpr int DIR_0 = 0, DIR_1 = 1;
            const UT_Vector3I cell0 = SIM::FieldUtils::faceToCellMap(face, AXIS, DIR_0);
            const UT_Vector3I cell1 = SIM::FieldUtils::faceToCellMap(face, AXIS, DIR_1);
            // This will clamp the bounds to fit within the voxel array, using the border type to resolve out of range values.
            const fpreal32 p0 = PRESSURE->getField()->field()->getValue(static_cast<int>(cell0.x()), static_cast<int>(cell0.y()), static_cast<int>(cell0.z()));
            // This will clamp the bounds to fit within the voxel array, using the border type to resolve out of range values.
            const fpreal32 p1 = PRESSURE->getField()->field()->getValue(static_cast<int>(cell1.x()), static_cast<int>(cell1.y()), static_cast<int>(cell1.z()));
            fpreal32 v = FLOW->getField(AXIS)->field()->getValue(vit.x(), vit.y(), vit.z());
            v -= (p1 - p0) / h;
            vit.setValue(v);
        }
    }

    THREADED_METHOD3(, PRESSURE->getField()->shouldMultiThread(), KnSubtractPressureGradient, SIM_VectorField*, FLOW, const SIM_ScalarField*, PRESSURE, const int, AXIS);
}

void HinaFlow::Poisson::SolveMultiThreaded(const Input& input, const Param& param, Result& result)
{
    const int size = static_cast<int>(input.MARKER->getField()->field()->numVoxels());


    // Build A
    UT_SparseMatrixF A(size, size);
    Internal::Poisson::KnBuildLaplaceMatrix(A, input.MARKER);
    A.compile();
    UT_SparseMatrixRowF AImpl;
    AImpl.buildFrom(A);
    UT_VectorF x(0, size - 1);


    // Build b (Store Divergence Optional)
    UT_VectorF b(0, size - 1);
    Internal::Poisson::KnBuildRhs(b, input.FLOW, input.MARKER, result.DIVERGENCE);


    // Solve System
    x = b;
    AImpl.solveConjugateGradient(x, b, nullptr);


    // Store Pressure
    Internal::Poisson::KnStorePressure(result.PRESSURE, x);


    // Subtract Pressure Gradient
    for (const int AXIS : GET_AXIS_ITER(input.FLOW))
        Internal::Poisson::KnSubtractPressureGradient(result.FLOW, result.PRESSURE, AXIS);
}

void HinaFlow::Poisson::SolveDifferential(const Input& input, const Param& param, Result& result) { Solve(input, param, result); }

void HinaFlow::Poisson::SolveDifferentialMultiThreaded(const Input& input, const Param& param, Result& result) { SolveMultiThreaded(input, param, result); }

void HinaFlow::Poisson::SolveFastDomain(const Input& input, const Param& param, Result& result, const SIM_IndexField* ADAPTIVE_DOMAIN)
{
    const float h = input.MARKER->getVoxelSize().maxComponent();

    int size = 0;
    {
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(ADAPTIVE_DOMAIN->getField()->field());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            if (vit.getValue() != -1)
                ++size;
            else
            {
                const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
                for (const int AXIS : GET_AXIS_ITER(ADAPTIVE_DOMAIN->getField()))
                {
                    constexpr int dir0 = 0, dir1 = 1;
                    const UT_Vector3I face0 = SIM::FieldUtils::cellToFaceMap(cell, AXIS, dir0);
                    const UT_Vector3I face1 = SIM::FieldUtils::cellToFaceMap(cell, AXIS, dir1);
                    SIM::FieldUtils::setFieldValue(*input.FLOW->getField(AXIS), face0, 0);
                    SIM::FieldUtils::setFieldValue(*input.FLOW->getField(AXIS), face1, 0);
                }
            }
        }
    }

    // Build A
    UT_SparseMatrixF A(size, size);
    {
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(ADAPTIVE_DOMAIN->getField()->field());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const int idx = static_cast<int>(SIM::FieldUtils::getFieldValue(*ADAPTIVE_DOMAIN->getField(), cell));
            if (idx == -1)
                continue;

            for (const int AXIS : GET_AXIS_ITER(ADAPTIVE_DOMAIN->getField()))
            {
                for (const int DIR : {0, 1})
                {
                    UT_Vector3I cell0 = SIM::FieldUtils::cellToCellMap(cell, AXIS, DIR);
                    if (!CHECK_CELL_VALID(ADAPTIVE_DOMAIN->getField(), cell0))
                        continue;

                    const int idx0 = static_cast<int>(SIM::FieldUtils::getFieldValue(*ADAPTIVE_DOMAIN->getField(), cell0));

                    if (idx0 == -1)
                        continue;
                    A.addToElement(idx, idx, 1.0f);
                    A.addToElement(idx, idx0, -1.0f);
                }
            }
        }
    }
    A.compile();
    UT_SparseMatrixRowF AImpl;
    AImpl.buildFrom(A);
    UT_VectorF x(0, size - 1);


    // Build b (Store Divergence Optional)
    UT_VectorF b(0, size - 1);
    {
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(ADAPTIVE_DOMAIN->getField()->field());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());

            fpreal32 divergence = 0;
            if (const int idx = static_cast<int>(vit.getValue()); idx != -1)
            {
                for (const int AXIS : GET_AXIS_ITER(ADAPTIVE_DOMAIN->getField()))
                {
                    constexpr int dir0 = 0, dir1 = 1;
                    const UT_Vector3I face0 = SIM::FieldUtils::cellToFaceMap(cell, AXIS, dir0);
                    const UT_Vector3I face1 = SIM::FieldUtils::cellToFaceMap(cell, AXIS, dir1);
                    const fpreal32 v0 = SIM::FieldUtils::getFieldValue(*input.FLOW->getField(AXIS), face0);
                    const fpreal32 v1 = SIM::FieldUtils::getFieldValue(*input.FLOW->getField(AXIS), face1);
                    divergence += (v1 - v0) * h;
                }
                b(idx) = -divergence;
            }

            if (result.DIVERGENCE)
                result.DIVERGENCE->getField()->fieldNC()->setValue(cell, divergence);
        }
    }


    // Solve System
    x = b;
    AImpl.solveConjugateGradient(x, b, nullptr);


    // Store Pressure
    {
        result.PRESSURE->getField()->makeConstant(0);
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(ADAPTIVE_DOMAIN->getField()->field());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());

            if (const int idx = static_cast<int>(SIM::FieldUtils::getFieldValue(*ADAPTIVE_DOMAIN->getField(), cell)); idx != -1)
                SIM::FieldUtils::setFieldValue(*result.PRESSURE->getField(), cell, x(idx));
        }
    }


    // Subtract Pressure Gradient
    for (const int AXIS : GET_AXIS_ITER(input.FLOW))
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(result.FLOW->getField(AXIS)->fieldNC());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I face(vit.x(), vit.y(), vit.z());
            constexpr int DIR_0 = 0, DIR_1 = 1;
            const UT_Vector3I cell0 = SIM::FieldUtils::faceToCellMap(face, AXIS, DIR_0);
            const UT_Vector3I cell1 = SIM::FieldUtils::faceToCellMap(face, AXIS, DIR_1);
            // This will clamp the bounds to fit within the voxel array, using the border type to resolve out of range values.
            const fpreal32 p0 = result.PRESSURE->getField()->field()->getValue(static_cast<int>(cell0.x()), static_cast<int>(cell0.y()), static_cast<int>(cell0.z()));
            // This will clamp the bounds to fit within the voxel array, using the border type to resolve out of range values.
            const fpreal32 p1 = result.PRESSURE->getField()->field()->getValue(static_cast<int>(cell1.x()), static_cast<int>(cell1.y()), static_cast<int>(cell1.z()));
            fpreal32 v = input.FLOW->getField(AXIS)->field()->getValue(vit.x(), vit.y(), vit.z());
            v -= (p1 - p0) / h;
            vit.setValue(v);
        }
    }
}
