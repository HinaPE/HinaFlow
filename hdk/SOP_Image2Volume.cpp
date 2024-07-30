#include "SOP_Image2Volume.h"

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

#include <PRM/PRM_Include.h>

OP_ERROR SOP_Image2Volume::cookMySop(OP_Context& context)
{
    return cookMyselfAsVerb(context);
}

PRM_Template* SOP_Image2Volume::buildTemplates()
{
    static PRM_Template myTemplateList[] = {
        PRM_Template(),
    };
    return myTemplateList;
}
