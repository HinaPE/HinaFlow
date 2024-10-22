#ifndef SOP_IMAGE2VOLUME_H
#define SOP_IMAGE2VOLUME_H

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


#include <SOP/SOP_Node.h>

class SOP_Image2Volume : public SOP_Node
{
public:
    inline static auto DOP_NAME = "Image2Volume";
    inline static auto DOP_ENGLISH = "Image to Volume";

public:
    static PRM_Template* buildTemplates();
    static OP_Node* myConstructor(OP_Network* net, const char* name, OP_Operator* op) { return new SOP_Image2Volume(net, name, op); }
    const SOP_NodeVerb* cookVerb() const final;

protected:
    SOP_Image2Volume(OP_Network* net, const char* name, OP_Operator* op) : SOP_Node(net, name, op) {}
    OP_ERROR cookMySop(OP_Context& context) final { return cookMyselfAsVerb(context); }
};


#endif //SOP_IMAGE2VOLUME_H
