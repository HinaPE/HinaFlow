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

void HinaFlow::Possion::Solve(const Input& input, const Param& param, Result& result)
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

            for (const int AXIS : input.MARKER->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2})
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
                for (const int AXIS : input.MARKER->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2})
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
    for (const int AXIS : (input.FLOW->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2}))
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
            const fpreal32 p0 = result.PRESSURE->getField()->field()->getValue(int(cell0.x()), int(cell0.y()), int(cell0.z()));
            // This will clamp the bounds to fit within the voxel array, using the border type to resolve out of range values.
            const fpreal32 p1 = result.PRESSURE->getField()->field()->getValue(int(cell1.x()), int(cell1.y()), int(cell1.z()));
            fpreal32 v = input.FLOW->getField(AXIS)->field()->getValue(vit.x(), vit.y(), vit.z());
            v -= (p1 - p0) / h;
            vit.setValue(v);
        }
    }
}


namespace HinaFlow::Internal::Possion
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
            int idx = static_cast<int>(TO_1D_IDX(cell, res));
            if (!CHECK_CELL_TYPE<CellType::Fluid>(MARKER, cell))
                continue;

            for (const int AXIS : (MARKER->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2}))
            {
                for (const int DIR : {0, 1})
                {
                    UT_Vector3I cell0 = SIM::FieldUtils::cellToCellMap(cell, AXIS, DIR);
                    int idx0 = static_cast<int>(TO_1D_IDX(cell0, res));

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
                for (const int AXIS : (MARKER->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2}))
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
            const fpreal32 p0 = PRESSURE->getField()->field()->getValue(int(cell0.x()), int(cell0.y()), int(cell0.z()));
            // This will clamp the bounds to fit within the voxel array, using the border type to resolve out of range values.
            const fpreal32 p1 = PRESSURE->getField()->field()->getValue(int(cell1.x()), int(cell1.y()), int(cell1.z()));
            fpreal32 v = FLOW->getField(AXIS)->field()->getValue(vit.x(), vit.y(), vit.z());
            v -= (p1 - p0) / h;
            vit.setValue(v);
        }
    }

    THREADED_METHOD3(, PRESSURE->getField()->shouldMultiThread(), KnSubtractPressureGradient, SIM_VectorField*, FLOW, const SIM_ScalarField*, PRESSURE, const int, AXIS);
}

void HinaFlow::Possion::SolveMultiThreaded(const Input& input, const Param& param, Result& result)
{
    const int size = static_cast<int>(input.MARKER->getField()->field()->numVoxels());


    // Build A
    UT_SparseMatrixF A(size, size);
    Internal::Possion::KnBuildLaplaceMatrix(A, input.MARKER);
    A.compile();
    UT_SparseMatrixRowF AImpl;
    AImpl.buildFrom(A);
    UT_VectorF x(0, size - 1);


    // Build b (Store Divergence Optional)
    UT_VectorF b(0, size - 1);
    Internal::Possion::KnBuildRhs(b, input.FLOW, input.MARKER, result.DIVERGENCE);


    // Solve System
    x = b;
    AImpl.solveConjugateGradient(x, b, nullptr);


    // Store Pressure
    Internal::Possion::KnStorePressure(result.PRESSURE, x);


    // Subtract Pressure Gradient
    for (const int AXIS : (input.FLOW->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2}))
        Internal::Possion::KnSubtractPressureGradient(result.FLOW, result.PRESSURE, AXIS);
}

void HinaFlow::Possion::SolveDifferential(const Input& input, const Param& param, Result& result) { Solve(input, param, result); }

void HinaFlow::Possion::SolveDifferentialMultiThreaded(const Input& input, const Param& param, Result& result) { SolveMultiThreaded(input, param, result); }
