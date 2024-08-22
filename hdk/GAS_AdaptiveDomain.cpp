#include "GAS_AdaptiveDomain.h"

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

const SIM_DopDescription* GAS_AdaptiveDomain::getDopDescription()
{
    static std::vector<PRM_Template> PRMs;
    PRMs.clear();
    ACTIVATE_GAS_ADAPTIVE_DOMAIN
    ACTIVATE_GAS_DENSITY
    PARAMETER_INT(Depth, 10)
    PRMs.emplace_back();

    static SIM_DopDescription DESC(GEN_NODE,
                                   DOP_NAME,
                                   DOP_ENGLISH,
                                   DATANAME,
                                   classname(),
                                   PRMs.data());
    DESC.setDefaultUniqueDataName(UNIQUE_DATANAME);
    setGasDescription(DESC);
    return &DESC;
}


void KnComputeAdaptiveDomainPartial(SIM_IndexField* ADAPTIVE_DOMAIN, const SIM_ScalarField* DENSITY, const int depth, const UT_JobInfo& info)
{
    UT_VoxelArrayIteratorI vit;
    vit.setArray(ADAPTIVE_DOMAIN->getField()->fieldNC());
    vit.setCompressOnExit(true);
    vit.setPartialRange(info.job(), info.numJobs());

    for (vit.rewind(); !vit.atEnd(); vit.advance())
    {
    }
}

THREADED_METHOD3(, ADAPTIVE_DOMAIN->getField()->shouldMultiThread(), KnComputeAdaptiveDomain, SIM_IndexField*, ADAPTIVE_DOMAIN, const SIM_ScalarField*, DENSITY, const int, depth);

bool GAS_AdaptiveDomain::solveGasSubclass(SIM_Engine& engine, SIM_Object* obj, SIM_Time time, SIM_Time timestep)
{
    SIM_IndexField* ADAPTIVE_DOMAIN = getIndexField(obj, GAS_NAME_ADAPTIVE_DOMAIN); // required
    const SIM_ScalarField* D = getConstScalarField(obj, GAS_NAME_DENSITY); // required

    if (!HinaFlow::CHECK_NOT_NULL(ADAPTIVE_DOMAIN, D))
    {
        addError(obj, SIM_MESSAGE, "Missing GAS fields", UT_ERROR_FATAL);
        return false;
    }

    if (!HinaFlow::CHECK_THE_SAME_DIMENSION(ADAPTIVE_DOMAIN, D))
    {
        addError(obj, SIM_MESSAGE, "GAS fields have different dimensions", UT_ERROR_FATAL);
        return false;
    }

    HinaFlow::FILL_FIELD(ADAPTIVE_DOMAIN, static_cast<exint>(-1));

    const auto res = D->getField()->field()->getVoxelRes();
    int64 minx = res[0], miny = res[1], minz = res[2];
    int64 maxz = 1, maxy = 1, maxx = 1;

    {
        int64 depth = getDepth();
        UT_VoxelArrayIteratorF vit;
        vit.setConstArray(D->getField()->field());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            if (vit.getValue() > 0 && vit.x() < minx) minx = vit.x();
            if (vit.getValue() > 0 && vit.y() < miny) miny = vit.y();
            if (vit.getValue() > 0 && vit.z() < minz) minz = vit.z();
            if (vit.getValue() > 0 && vit.x() > maxx) maxx = vit.x();
            if (vit.getValue() > 0 && vit.y() > maxy) maxy = vit.y();
            if (vit.getValue() > 0 && vit.z() > maxz) maxz = vit.z();
        }

        minx -= depth;
        minx = std::max(minx, 0LL);
        miny -= depth;
        miny = std::max(miny, 0LL);
        minz -= depth;
        minz = std::max(minz, 0LL);
        maxx += depth;
        maxx = std::min(maxx, res[0]);
        maxy += depth;
        maxy = std::min(maxy, res[1]);
        maxz += depth;
        maxz = std::min(maxz, res[2]);
    }

    printf("x:[%lld, %lld], y:[%lld, %lld], z:[%lld, %lld]\n", minx, maxx, miny, maxy, minz, maxz);

    {
        int index = 0;
        UT_VoxelArrayIteratorI vit;
        vit.setArray(ADAPTIVE_DOMAIN->getField()->fieldNC());
        for (vit.rewind(); !vit.atEnd(); vit.advance())
        {
            if (vit.x() >= minx && vit.x() <= maxx && vit.y() >= miny && vit.y() <= maxy && vit.z() >= minz && vit.z() <= maxz)
                vit.setValue(index++);
        }
    }

    return true;
}
