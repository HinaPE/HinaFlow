#include "diffusion.h"

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

void HinaFlow::Diffusion::Solve(const Input& input, const Param& param, Result& result)
{
    const int size = static_cast<int>(input.MARKER->getField()->field()->numVoxels());
    const UT_Vector3I res = input.MARKER->getField()->getVoxelRes();
    const float h = input.MARKER->getVoxelSize().maxComponent();


    // Build A
    UT_SparseMatrixF A(size, size);
    {
        UT_VoxelArrayIteratorI vit;
        vit.setConstArray(input.MARKER->getField()->field());

        float factor = param.diffusion * input.dt / (h * h);

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            int idx = static_cast<int>(TO_1D_IDX(cell, res));
            if (!CHECK_CELL_TYPE<CellType::Fluid>(input.MARKER, cell))
                continue;

            A.addToElement(idx, idx, 1.0f);

            for (const int AXIS : GET_AXIS_ITER(input.MARKER->getField()))
            {
                for (const int DIR : {0, 1})
                {
                    UT_Vector3I cell0 = SIM::FieldUtils::cellToCellMap(cell, AXIS, DIR);
                    int idx0 = static_cast<int>(TO_1D_IDX(cell0, res));

                    if (CHECK_CELL_VALID(input.MARKER->getField(), cell0))
                    {
                        A.addToElement(idx, idx, factor);
                        A.addToElement(idx, idx0, -factor);
                    }
                }
            }
        }
    }
    A.compile();
    UT_SparseMatrixRowF AImpl;
    AImpl.buildFrom(A);
    UT_VectorF x(0, size - 1);


    if (input.FIELDS && result.FIELDS)
    {
        // Build b
        UT_VectorF b(0, size - 1);
        {
            UT_VoxelArrayIteratorF vit;
            vit.setConstArray(input.FIELDS->getField()->field());
            for (vit.rewind(); !vit.atEnd(); vit.advance())
            {
                const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
                const auto idx = TO_1D_IDX(cell, res);

                fpreal32 rhs = 0;
                if (CHECK_CELL_TYPE<CellType::Fluid>(input.MARKER, cell))
                    rhs = vit.getValue();
                b(idx) = rhs;
            }
        }


        // Solve System
        x = b;
        AImpl.solveConjugateGradient(x, b, nullptr);


        // Store Diffused Field
        {
            UT_VoxelArrayIteratorF vit;
            vit.setArray(result.FIELDS->getField()->fieldNC());
            for (vit.rewind(); !vit.atEnd(); vit.advance())
            {
                const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
                const auto idx = TO_1D_IDX(cell, result.FIELDS->getField()->getVoxelRes());
                vit.setValue(x(idx));
            }
        }
    }


    if (input.FIELDV && result.FIELDV)
    {
        for (const int AXIS : GET_AXIS_ITER(input.FIELDV))
        {
            // Build b
            UT_VectorF b(0, size - 1);
            {
                UT_VoxelArrayIteratorF vit;
                vit.setConstArray(input.FIELDV->getField(AXIS)->field());
                for (vit.rewind(); !vit.atEnd(); vit.advance())
                {
                    const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
                    const auto idx = TO_1D_IDX(cell, res);

                    fpreal32 rhs = 0;
                    if (CHECK_CELL_TYPE<CellType::Fluid>(input.MARKER, cell))
                        rhs = vit.getValue();
                    b(idx) = rhs;
                }
            }


            // Solve System
            x = b;
            AImpl.solveConjugateGradient(x, b, nullptr);


            // Store Diffused Field
            {
                UT_VoxelArrayIteratorF vit;
                vit.setArray(result.FIELDV->getField(AXIS)->fieldNC());
                for (vit.rewind(); !vit.atEnd(); vit.advance())
                {
                    const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
                    const auto idx = TO_1D_IDX(cell, result.FIELDV->getField(AXIS)->getVoxelRes());
                    vit.setValue(x(idx));
                }
            }
        }
    }
}


namespace HinaFlow::Internal::Diffusion
{
    void KnBuildLaplaceMatrixPartial(UT_SparseMatrixF& A, const SIM_IndexField* MARKER, float factor, const UT_JobInfo& info)
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

            A.addToElement(idx, idx, 1.0f);

            for (const int AXIS : GET_AXIS_ITER(MARKER->getField()))
            {
                for (const int DIR : {0, 1})
                {
                    UT_Vector3I cell0 = SIM::FieldUtils::cellToCellMap(cell, AXIS, DIR);
                    int idx0 = static_cast<int>(TO_1D_IDX(cell0, res));

                    if (CHECK_CELL_VALID(MARKER->getField(), cell0))
                    {
                        A.addToElement(idx, idx, factor);
                        A.addToElement(idx, idx0, -factor);
                    }
                }
            }
        }
    }

    THREADED_METHOD3(, false /* DO NOT USE MULTI THREAD HERE */, KnBuildLaplaceMatrix, UT_SparseMatrixF&, A, const SIM_IndexField*, MARKER, float, factor);


    void KnBuildRhsPartial(UT_VectorF& b, const SIM_RawField* FIELD, const SIM_IndexField* MARKER, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setConstArray(FIELD->field());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        const UT_Vector3I res = MARKER->getField()->getVoxelRes();

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const auto idx = TO_1D_IDX(cell, res);

            fpreal32 rhs = 0;
            if (CHECK_CELL_TYPE<CellType::Fluid>(MARKER, cell))
                rhs = vit.getValue();
            b(idx) = rhs;
        }
    }

    THREADED_METHOD3(, MARKER->getField()->shouldMultiThread(), KnBuildRhs, UT_VectorF&, b, const SIM_RawField*, FIELD, const SIM_IndexField*, MARKER);


    void KnStoreDiffusionPartial(SIM_RawField* FIELD, const UT_VectorF& x, const UT_JobInfo& info)
    {
        UT_VoxelArrayIteratorF vit;
        vit.setArray(FIELD->fieldNC());
        vit.setCompressOnExit(true);
        vit.setPartialRange(info.job(), info.numJobs());

        const UT_Vector3I res = FIELD->getVoxelRes();

        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            const UT_Vector3I cell(vit.x(), vit.y(), vit.z());
            const auto idx = TO_1D_IDX(cell, res);
            vit.setValue(x(idx));
        }
    }

    THREADED_METHOD2(, FIELD->shouldMultiThread(), KnStoreDiffusion, SIM_RawField*, FIELD, const UT_VectorF&, x);
}

void HinaFlow::Diffusion::SolveMultiThreaded(const Input& input, const Param& param, Result& result)
{
    const int size = static_cast<int>(input.MARKER->getField()->field()->numVoxels());
    const float h = input.MARKER->getVoxelSize().maxComponent();


    // Build A
    UT_SparseMatrixF A(size, size);
    Internal::Diffusion::KnBuildLaplaceMatrix(A, input.MARKER, param.diffusion * input.dt / (h * h));
    UT_SparseMatrixRowF AImpl;
    AImpl.buildFrom(A);
    UT_VectorF x(0, size - 1);
    UT_VectorF b(0, size - 1);


    if (input.FIELDS && result.FIELDS)
    {
        // Build b
        Internal::Diffusion::KnBuildRhs(b, input.FIELDS->getField(), input.MARKER);

        // Solve System
        x = b;
        AImpl.solveConjugateGradient(x, b, nullptr);

        // Store Diffused Field
        Internal::Diffusion::KnStoreDiffusion(result.FIELDS->getField(), x);
    }


    if (input.FIELDV && result.FIELDV)
    {
        for (const int AXIS : GET_AXIS_ITER(input.FIELDV))
        {
            // Build b
            Internal::Diffusion::KnBuildRhs(b, input.FIELDV->getField(AXIS), input.MARKER);

            // Solve System
            x = b;
            AImpl.solveConjugateGradient(x, b, nullptr);

            // Store Diffused Field
            Internal::Diffusion::KnStoreDiffusion(result.FIELDV->getField(AXIS), x);
        }
    }
}
