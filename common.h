#ifndef COMMON_H
#define COMMON_H

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

#include <SIM/SIM_Engine.h>
#include <SIM/SIM_Object.h>
#include <SIM/SIM_GeometryCopy.h>
#include <SIM/SIM_DopDescription.h>

#include <SIM/SIM_ScalarField.h>
#include <SIM/SIM_VectorField.h>
#include <SIM/SIM_IndexField.h>
#include <SIM/SIM_FieldUtils.h>

#include <GAS/GAS_SPH.h>

#include <PRM/PRM_Template.h>
#include <PRM/PRM_Default.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_TemplateBuilder.h>

#include <OP/OP_Operator.h>
#include <OP/OP_AutoLockInputs.h>

#include <SOP/SOP_NodeVerb.h>

#include <GU/GU_Detail.h>
#include <GEO/GEO_PrimVolume.h>
#include <GU/GU_PrimVolume.h>
#include <GU/GU_NeighbourList.h>

#include <UT/UT_MultigridArray.h>
#include <UT/UT_ThreadedAlgorithm.h>
#include <UT/UT_SparseMatrix.h>
#include <UT/UT_StringHolder.h>

#include <SYS/SYS_Math.h>

#include <PY/PY_Python.h>
#include <PY/PY_CPythonAPI.h>


#define ACTIVATE_GAS_GEOMETRY static PRM_Name GeometryName(GAS_NAME_GEOMETRY, SIM_GEOMETRY_DATANAME); static PRM_Default GeometryNameDefault(0, SIM_GEOMETRY_DATANAME); PRMs.emplace_back(PRM_STRING, 1, &GeometryName, &GeometryNameDefault);
#define ACTIVATE_GAS_DENSITY static PRM_Name DensityName(GAS_NAME_DENSITY, "Density"); static PRM_Default DensityNameDefault(0, GAS_NAME_DENSITY); PRMs.emplace_back(PRM_STRING, 1, &DensityName, &DensityNameDefault);
#define ACTIVATE_GAS_TEMPERATURE static PRM_Name TemperatureName(GAS_NAME_TEMPERATURE, "Temperature"); static PRM_Default TemperatureNameDefault(0, GAS_NAME_TEMPERATURE); PRMs.emplace_back(PRM_STRING, 1, &TemperatureName, &TemperatureNameDefault);
#define ACTIVATE_GAS_VELOCITY static PRM_Name VelocityName(GAS_NAME_VELOCITY, "Velocity"); static PRM_Default VelocityNameDefault(0, GAS_NAME_VELOCITY); PRMs.emplace_back(PRM_STRING, 1, &VelocityName, &VelocityNameDefault);
#define ACTIVATE_GAS_SOURCE static PRM_Name SourceName(GAS_NAME_SOURCE, "Source"); static PRM_Default SourceNameDefault(0, GAS_NAME_SOURCE); PRMs.emplace_back(PRM_STRING, 1, &SourceName, &SourceNameDefault);
#define ACTIVATE_GAS_DIVERGENCE static PRM_Name DivergenceName(GAS_NAME_DIVERGENCE, "Divergence"); static PRM_Default DivergenceNameDefault(0, GAS_NAME_DIVERGENCE); PRMs.emplace_back(PRM_STRING, 1, &DivergenceName, &DivergenceNameDefault);
#define ACTIVATE_GAS_PRESSURE static PRM_Name PressureName(GAS_NAME_PRESSURE, "Pressure"); static PRM_Default PressureNameDefault(0, GAS_NAME_PRESSURE); PRMs.emplace_back(PRM_STRING, 1, &PressureName, &PressureNameDefault);
#define ACTIVATE_GAS_STENCIL static PRM_Name StencilName(GAS_NAME_STENCIL, "Stencil"); static PRM_Default StencilNameDefault(0, GAS_NAME_STENCIL); PRMs.emplace_back(PRM_STRING, 1, &StencilName, &StencilNameDefault);
#define ACTIVATE_GAS_FIELD static PRM_Name FieldName(GAS_NAME_FIELD, "Field"); static PRM_Default FieldNameDefault(0, GAS_NAME_FIELD); PRMs.emplace_back(PRM_STRING, 1, &FieldName, &FieldNameDefault);
#define ACTIVATE_GAS_FIELDDEST static PRM_Name FieldDestName(GAS_NAME_FIELDDEST, "FieldDest"); static PRM_Default FieldDestNameDefault(0, GAS_NAME_FIELDDEST); PRMs.emplace_back(PRM_STRING, 1, &FieldDestName, &FieldDestNameDefault);
#define ACTIVATE_GAS_FIELDSOURCE static PRM_Name FieldSourceName(GAS_NAME_FIELDSOURCE, "FieldSource"); static PRM_Default FieldSourceNameDefault(0, GAS_NAME_FIELDSOURCE); PRMs.emplace_back(PRM_STRING, 1, &FieldSourceName, &FieldSourceNameDefault);
#define ACTIVATE_GAS_COLLISION static PRM_Name CollisionName(GAS_NAME_COLLISION, "Collision"); static PRM_Default CollisionNameDefault(0, GAS_NAME_COLLISION); PRMs.emplace_back(PRM_STRING, 1, &CollisionName, &CollisionNameDefault);

#define GAS_NAME_COLOR "color"
#define ACTIVATE_GAS_COLOR static PRM_Name ColorName(GAS_NAME_COLOR, "Color"); static PRM_Default ColorNameDefault(0, GAS_NAME_COLOR); PRMs.emplace_back(PRM_STRING, 1, &ColorName, &ColorNameDefault);

#define PARAMETER_BOOL(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME);static PRM_Default Default##NAME(DEFAULT_VALUE);PRMs.emplace_back(PRM_TOGGLE, 1, &NAME, &Default##NAME);
#define PARAMETER_INT(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME);static PRM_Default Default##NAME(DEFAULT_VALUE);PRMs.emplace_back(PRM_INT, 1, &NAME, &Default##NAME);
#define PARAMETER_FLOAT(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME);static PRM_Default Default##NAME(DEFAULT_VALUE);PRMs.emplace_back(PRM_FLT, 1, &NAME, &Default##NAME);
#define PARAMETER_VECTOR_FLOAT_N(NAME, SIZE, ...) static PRM_Name NAME(#NAME, #NAME); static std::array<PRM_Default, SIZE> Default##NAME{__VA_ARGS__}; PRMs.emplace_back(PRM_FLT, SIZE, &NAME, Default##NAME.data());

#define POINT_ATTRIBUTE_V3(NAME) GA_RWAttributeRef NAME##_attr = gdp.findGlobalAttribute(#NAME); if (!NAME##_attr.isValid()) NAME##_attr = gdp.addFloatTuple(GA_ATTRIB_POINT, #NAME, 3, GA_Defaults(0)); GA_RWHandleV3 NAME##_handle(NAME##_attr);
#define POINT_ATTRIBUTE_F(NAME) GA_RWAttributeRef NAME##_attr = gdp.findGlobalAttribute(#NAME); if (!NAME##_attr.isValid()) NAME##_attr = gdp.addFloatTuple(GA_ATTRIB_POINT, #NAME, 1, GA_Defaults(0)); GA_RWHandleF NAME##_handle(NAME##_attr);
#define POINT_ATTRIBUTE_I(NAME) GA_RWAttributeRef NAME##_attr = gdp.findGlobalAttribute(#NAME); if (!NAME##_attr.isValid()) NAME##_attr = gdp.addIntTuple(GA_ATTRIB_POINT, #NAME, 1, GA_Defaults(0)); GA_RWHandleI NAME##_handle(NAME##_attr);

namespace HinaFlow
{
    enum class CellType : unsigned char
    {
        Empty = 0,
        Fluid = 1,
        Solid = 2,
        Inflow = 3,
        Outflow = 4,
    };

    template <typename... Args>
    bool CHECK_NOT_NULL(Args... args) { return ((args != nullptr) && ...); }

    template <typename T, typename... Args>
    bool CHECK_THE_SAME_DIMENSION(T first, Args... args)
    {
        bool first_value = first->getTwoDField();
        return ((args->getTwoDField() == first_value) && ...);
    }

    template <typename FieldType>
    bool CHECK_IS_FACE_SAMPLED(FieldType arg)
    {
        if constexpr (std::is_same_v<FieldType, SIM_VectorField*>) { return arg->isFaceSampled(); }
        return true;
    }

    template <typename FieldType>
    bool CHECK_CELL_VALID(const FieldType* FIELD, const UT_Vector3I& cell) { return FIELD->field()->isValidIndex(int(cell.x()), int(cell.y()), int(cell.z())); }

    template <CellType TYPE>
    bool CHECK_CELL_TYPE(const SIM_IndexField* MARKERS, const UT_Vector3I cell) { return SIM::FieldUtils::getFieldValue(*MARKERS->getField(), cell) == static_cast<exint>(TYPE); }

    template <typename FieldType, typename T>
    void FILL_FIELD(FieldType* FIELD, T value)
    {
        if constexpr (std::is_same_v<T, SIM_VectorField*>)
            for (const int AXIS : FIELD->getTwoDField() ? std::vector{0, 1} : std::vector{0, 1, 2})
                FIELD->getField(AXIS)->makeConstant(value);
        else
            FIELD->getField()->makeConstant(value);
    }

    template <typename T>
    T TO_1D_IDX(const UT_Vector3T<T>& idx, const UT_Vector3T<T>& res) { return idx.x() + res.x() * (idx.y() + res.y() * idx.z()); }

    template <typename T>
    T TO_1D_IDX(const UT_Vector2T<T>& idx, const UT_Vector2T<T>& res) { return idx.x() + res.x() * idx.y(); }


    template <typename T>
    UT_Vector3T<T> TO_3D_IDX(const T idx, const UT_Vector3T<T>& res)
    {
        UT_Vector3T<T> result;
        result.z() = idx / (res.x() * res.y());
        result.y() = (idx - result.z() * res.x() * res.y()) / res.x();
        result.x() = idx - result.z() * res.x() * res.y() - result.y() * res.x();
        return result;
    }

    template <typename T>
    UT_Vector2T<T> TO_2D_IDX(const T idx, const UT_Vector2T<T>& res)
    {
        UT_Vector2T<T> result;
        result.y() = idx / res.x();
        result.x() = idx - result.y() * res.x();
        return result;
    }


    // @formatter:off
    using float3 = UT_Vector3;
    struct Cubic
    {
        inline static void SetRadius(float r) { _r = r; const float pi = 3.14159265358979323846f; const float h3 = _r * _r * _r; _k = 8.f / (pi * h3); _l = 48.f / (pi * h3); _W_0 = W(float3{0, 0, 0}); }
        inline static float W(const float r) { float res = 0.f; const float q = r / _r; if (q <= 1.0) { if (q <= 0.5) { const float q2 = q * q; const float q3 = q2 * q; res = _k * (6.f * q3 - 6.f * q2 + 1.f); } else { res = _k * (2.f * pow(1.f - q, 3.f)); } } return res; }
        inline static float W(const float3 &r) { return W(r.length()); }
        inline static float3 gradW(const float3 &r) { float3 res; const float rl = r.length(); const float q = rl / _r; if ((rl > 1.0e-9) && (q <= 1.f)) { float3 gradq = r / rl; gradq /= _r; if (q <= 0.5) { res = _l * q * (3.f * q - 2.f) * gradq; } else { const float factor = 1.f - q; res = _l * (-factor * factor) * gradq; } } else res = float3{0, 0, 0}; return res; }
        inline static float W_zero() { return _W_0; }
        inline static float _r;
        inline static float _k;
        inline static float _l;
        inline static float _W_0;
    };
    using Kernel = Cubic;
    // @formatter:on
}

#endif //COMMON_H
