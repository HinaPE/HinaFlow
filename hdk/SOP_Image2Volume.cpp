#include "SOP_Image2Volume.h"
#include "SOP_Image2Volume.proto.h"

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

using UT::Literal::operator ""_sh;

class SOP_Image2VolumeVerb : public SOP_NodeVerb
{
public:
    SOP_NodeParms* allocParms() const final { return new SOP_Image2VolumeParms(); }
    UT_StringHolder name() const final { return theSOPTypeName; }
    void cook(const CookParms& cookparms) const final;
    static const UT_StringHolder theSOPTypeName;
    static const SOP_NodeVerb::Register<SOP_Image2VolumeVerb> theVerb;
    static const char* const theDsFile;
};

const UT_StringHolder SOP_Image2VolumeVerb::theSOPTypeName("hdk_Image2Volume"_sh);
const SOP_NodeVerb::Register<SOP_Image2VolumeVerb> SOP_Image2VolumeVerb::theVerb;
const char* const SOP_Image2VolumeVerb::theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "sourcegroup"
        cppname "SourceGroup"
        label   "Source Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('sourcegrouptype')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "sourcegrouptype"
        cppname "SourceGroupType"
        label   "Source Group Type"
        type    ordinal
        default { "0" }
        menu {
            "guess"     "Guess from Group"
            "prims"     "Primitives"
            "points"    "Points"
        }
    }
    parm {
        name    "targetgroup"
        cppname "TargetGroup"
        label   "Target Points"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Points,)\nkwargs['inputindex'] = 1\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "1" }
    }
    parm {
        name        "useidattrib"
        cppname     "UseIDAttrib"
        label       "Piece Attribute"
        type        toggle
        joinnext
        nolabel
        default     { "0" }
    }
    parm {
        name        "idattrib"
        cppname     "IDAttrib"
        label       "Piece Attribute"
        type        string
        default     { "variant" }
        parmtag     { "sop_input" "1" }
        disablewhen "{ useidattrib == 0 }"
    }
    parm {
        name    "pack"
        label   "Pack and Instance"
        type    toggle
        default { "off" }
    }
    parm {
        name    "pivot"
        label   "Pivot Location"
        type    ordinal
        default { "centroid" }
        hidewhen "{ pack == 0 }"
        menu {
            "origin"    "Origin"
            "centroid"  "Centroid"
        }
    }
    parm {
        name    "viewportlod"
        cppname "ViewportLOD"
        label   "Display As"
        type    ordinal
        default { "full" }
        hidewhen "{ pack == 0 }"
        menu {
            "full"      "Full Geometry"
            "points"    "Point Cloud"
            "box"       "Bounding Box"
            "centroid"  "Centroid"
            "hidden"    "Hidden"
        }
    }
    parm {
        name    "transform"
        label   "Transform Using Target Point Orientations"
        type    toggle
        default { "on" }
    }
    parm {
        name    "useimplicitn"
        cppname "UseImplicitN"
        label   "Transform Using Implicit Target Point Normals If No Point N Attribute"
        type    toggle
        default { "on" }
        disablewhen "{ transform == 0 }"
    }
    parm {
        name    "resettargetattribs"
        label   "Reset Attributes from Target"
        type    button
        default { "0" }
    }
    multiparm {
        name    "targetattribs"
        cppname "TargetAttribs"
        label   "Attributes from Target"
        default 0

        parm {
            name        "useapply#"
            label       "Apply Attributes"
            type        toggle
            joinnext
            nolabel
            default     { "1" }
        }
        parm {
            name    "applyto#"
            label   "Apply to"
            type    ordinal
            joinnext
            default { "0" }
            menu {
                "points" "Points"
                "verts"  "Vertices"
                "prims"  "Primitives"
            }
        }
        parm {
            name    "applymethod#"
            label   "by"
            type    ordinal
            joinnext
            default { "0" }
            menu {
                "copy"  "Copying"
                "none"  "Nothing"
                "mult"  "Multiplying"
                "add"   "Adding"
                "sub"   "Subtracting"
            }
        }
        parm {
            name    "applyattribs#"
            label   "Attributes"
            type    string
            parmtag { "sop_input" "1" }
            default { "" }
        }
    }
}
)THEDSFILE";

void SOP_Image2VolumeVerb::cook(const CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_Image2VolumeParms>();
    GU_Detail* output_gdp = cookparms.gdh().gdpNC();
    const GU_Detail* source = cookparms.inputGeo(0);
    const GU_Detail* target = cookparms.inputGeo(1);

    std::cout << "SOP_Image2VolumeVerb::cook" << '\n';

    {
        GA_Primitive* prim;
        GA_FOR_ALL_PRIMITIVES(output_gdp, prim)
        {
            if (prim->getTypeId() == GEO_PRIMVOLUME)
            {
                auto* volume = dynamic_cast<GEO_PrimVolume*>(prim);
            }
        }
    }

    if (source)
    {
        const GA_Primitive* prim;
        GA_FOR_ALL_PRIMITIVES(source, prim)
        {
            if (prim->getTypeId() == GEO_PRIMVOLUME)
                printf("Source Volume Name: %s\n", prim->getTypeName());
        }
    }

    if (target)
    {
        const GA_Primitive* prim;
        GA_FOR_ALL_PRIMITIVES(target, prim)
        {
            if (prim->getTypeId() == GEO_PRIMVOLUME)
                printf("Target Volume Name: %s\n", prim->getTypeName());
        }
    }
}

PRM_Template* SOP_Image2Volume::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_Image2VolumeVerb.cpp"_sh, SOP_Image2VolumeVerb::theDsFile);
    return templ.templates();
}

const SOP_NodeVerb* SOP_Image2Volume::cookVerb() const { return SOP_Image2VolumeVerb::theVerb.get(); }
