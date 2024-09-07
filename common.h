#ifndef HINAFLOW_COMMON_H
#define HINAFLOW_COMMON_H

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
#include <SIM/SIM_GuideShared.h>

#include <SIM/SIM_ScalarField.h>
#include <SIM/SIM_VectorField.h>
#include <SIM/SIM_IndexField.h>
#include <SIM/SIM_FieldUtils.h>

#include <GAS/GAS_SPH.h>

#include <PRM/PRM_Template.h>
#include <PRM/PRM_Default.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <PRM/PRM_SpareData.h>

#include <OP/OP_Operator.h>
#include <OP/OP_AutoLockInputs.h>

#include <SOP/SOP_NodeVerb.h>

#include <GU/GU_Detail.h>
#include <GEO/GEO_PrimVolume.h>
#include <GU/GU_PrimVolume.h>
#include <GU/GU_PrimPoly.h>
#include <GU/GU_PrimSphere.h>
#include <GU/GU_NeighbourList.h>

#include <UT/UT_MultigridArray.h>
#include <UT/UT_ThreadedAlgorithm.h>
#include <UT/UT_ParallelUtil.h>
#include <UT/UT_SparseMatrix.h>
#include <UT/UT_StringHolder.h>

#include <VGEO/VGEO_Ray.h>
#include <BRAY/BRAY_Interface.h>

#include <SYS/SYS_Math.h>

#include <PY/PY_Python.h>


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

#define GAS_NAME_WEIGHT "weight"
#define ACTIVATE_GAS_WEIGHT static PRM_Name WeightName(GAS_NAME_WEIGHT, "Weight"); static PRM_Default WeightNameDefault(0, GAS_NAME_WEIGHT); PRMs.emplace_back(PRM_STRING, 1, &WeightName, &WeightNameDefault);

#define GAS_NAME_EXTRAPOLATION "extrapolation"
#define ACTIVATE_GAS_EXTRAPOLATION static PRM_Name ExtrapolationName(GAS_NAME_EXTRAPOLATION, "Extrapolation"); static PRM_Default ExtrapolationNameDefault(0, GAS_NAME_EXTRAPOLATION); PRMs.emplace_back(PRM_STRING, 1, &ExtrapolationName, &ExtrapolationNameDefault);

#define GAS_NAME_COLOR "color"
#define ACTIVATE_GAS_COLOR static PRM_Name ColorName(GAS_NAME_COLOR, "Color"); static PRM_Default ColorNameDefault(0, GAS_NAME_COLOR); PRMs.emplace_back(PRM_STRING, 1, &ColorName, &ColorNameDefault);

#define GAS_NAME_ADAPTIVE_DOMAIN "adaptive_domain"
#define ACTIVATE_GAS_ADAPTIVE_DOMAIN static PRM_Name AdaptiveDomainName(GAS_NAME_ADAPTIVE_DOMAIN, "Adaptive Domain"); static PRM_Default AdaptiveDomainNameDefault(0, GAS_NAME_ADAPTIVE_DOMAIN); PRMs.emplace_back(PRM_STRING, 1, &AdaptiveDomainName, &AdaptiveDomainNameDefault);

#define GAS_NAME_SCALAR_FIELD_1 "scalar_field_1"
#define ACTIVATE_GAS_SCALAR_FIELD_1 static PRM_Name ScalarField1Name(GAS_NAME_SCALAR_FIELD_1, "Scalar Field 1"); static PRM_Default ScalarField1NameDefault(0, GAS_NAME_SCALAR_FIELD_1); PRMs.emplace_back(PRM_STRING, 1, &ScalarField1Name, &ScalarField1NameDefault);
#define GAS_NAME_SCALAR_FIELD_2 "scalar_field_2"
#define ACTIVATE_GAS_SCALAR_FIELD_2 static PRM_Name ScalarField2Name(GAS_NAME_SCALAR_FIELD_2, "Scalar Field 2"); static PRM_Default ScalarField2NameDefault(0, GAS_NAME_SCALAR_FIELD_2); PRMs.emplace_back(PRM_STRING, 1, &ScalarField2Name, &ScalarField2NameDefault);
#define GAS_NAME_SCALAR_FIELD_3 "scalar_field_3"
#define ACTIVATE_GAS_SCALAR_FIELD_3 static PRM_Name ScalarField3Name(GAS_NAME_SCALAR_FIELD_3, "Scalar Field 3"); static PRM_Default ScalarField3NameDefault(0, GAS_NAME_SCALAR_FIELD_3); PRMs.emplace_back(PRM_STRING, 1, &ScalarField3Name, &ScalarField3NameDefault);
#define GAS_NAME_SCALAR_FIELD_4 "scalar_field_4"
#define ACTIVATE_GAS_SCALAR_FIELD_4 static PRM_Name ScalarField4Name(GAS_NAME_SCALAR_FIELD_4, "Scalar Field 4"); static PRM_Default ScalarField4NameDefault(0, GAS_NAME_SCALAR_FIELD_4); PRMs.emplace_back(PRM_STRING, 1, &ScalarField4Name, &ScalarField4NameDefault);

#define GAS_NAME_VECTOR_FIELD_1 "vector_field_1"
#define ACTIVATE_GAS_VECTOR_FIELD_1 static PRM_Name VectorField1Name(GAS_NAME_VECTOR_FIELD_1, "Vector Field 1"); static PRM_Default VectorField1NameDefault(0, GAS_NAME_VECTOR_FIELD_1); PRMs.emplace_back(PRM_STRING, 1, &VectorField1Name, &VectorField1NameDefault);
#define GAS_NAME_VECTOR_FIELD_2 "vector_field_2"
#define ACTIVATE_GAS_VECTOR_FIELD_2 static PRM_Name VectorField2Name(GAS_NAME_VECTOR_FIELD_2, "Vector Field 2"); static PRM_Default VectorField2NameDefault(0, GAS_NAME_VECTOR_FIELD_2); PRMs.emplace_back(PRM_STRING, 1, &VectorField2Name, &VectorField2NameDefault);
#define GAS_NAME_VECTOR_FIELD_3 "vector_field_3"
#define ACTIVATE_GAS_VECTOR_FIELD_3 static PRM_Name VectorField3Name(GAS_NAME_VECTOR_FIELD_3, "Vector Field 3"); static PRM_Default VectorField3NameDefault(0, GAS_NAME_VECTOR_FIELD_3); PRMs.emplace_back(PRM_STRING, 1, &VectorField3Name, &VectorField3NameDefault);
#define GAS_NAME_VECTOR_FIELD_4 "vector_field_4"
#define ACTIVATE_GAS_VECTOR_FIELD_4 static PRM_Name VectorField4Name(GAS_NAME_VECTOR_FIELD_4, "Vector Field 4"); static PRM_Default VectorField4NameDefault(0, GAS_NAME_VECTOR_FIELD_4); PRMs.emplace_back(PRM_STRING, 1, &VectorField4Name, &VectorField4NameDefault);

#define PARAMETER_FILE(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME); static PRM_Default Default##NAME(0, DEFAULT_VALUE); PRMs.emplace_back(PRM_CMDFILE, 1, &NAME, &Default##NAME);
#define PARAMETER_STRING(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME); static PRM_Default Default##NAME(0, DEFAULT_VALUE); PRMs.emplace_back(PRM_STRING, 1, &NAME, &Default##NAME);
#define PARAMETER_STRING_PYTHON(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME); static PRM_Default Default##NAME(0, DEFAULT_VALUE); PRMs.emplace_back(PRM_STRING, 1, &NAME, &Default##NAME, nullptr, nullptr, nullptr, &PRM_SpareData::stringEditorLangPython);
#define PARAMETER_PATH(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME); PRMs.emplace_back(PRM_STRING, PRM_TYPE_DYNAMIC_PATH, 1, &NAME);
#define PARAMETER_BOOL(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME); static PRM_Default Default##NAME(DEFAULT_VALUE); PRMs.emplace_back(PRM_TOGGLE, 1, &NAME, &Default##NAME);
#define PARAMETER_INT(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME); static PRM_Default Default##NAME(DEFAULT_VALUE); PRMs.emplace_back(PRM_INT, 1, &NAME, &Default##NAME);
#define PARAMETER_FLOAT(NAME, DEFAULT_VALUE) static PRM_Name NAME(#NAME, #NAME); static PRM_Default Default##NAME(DEFAULT_VALUE); PRMs.emplace_back(PRM_FLT, 1, &NAME, &Default##NAME);
#define PARAMETER_VECTOR_INT_N(NAME, SIZE, ...) static PRM_Name NAME(#NAME, #NAME); static std::array<PRM_Default, SIZE> Default##NAME{__VA_ARGS__}; PRMs.emplace_back(PRM_INT, SIZE, &NAME, Default##NAME.data());
#define PARAMETER_VECTOR_FLOAT_N(NAME, SIZE, ...) static PRM_Name NAME(#NAME, #NAME); static std::array<PRM_Default, SIZE> Default##NAME{__VA_ARGS__}; PRMs.emplace_back(PRM_FLT, SIZE, &NAME, Default##NAME.data());

#define POINT_ATTRIBUTE_V3(NAME) GA_RWAttributeRef NAME##_attr = gdp.findGlobalAttribute(#NAME); if (!NAME##_attr.isValid()) NAME##_attr = gdp.addFloatTuple(GA_ATTRIB_POINT, #NAME, 3, GA_Defaults(0)); GA_RWHandleV3 NAME##_handle(NAME##_attr);
#define POINT_ATTRIBUTE_F(NAME) GA_RWAttributeRef NAME##_attr = gdp.findGlobalAttribute(#NAME); if (!NAME##_attr.isValid()) NAME##_attr = gdp.addFloatTuple(GA_ATTRIB_POINT, #NAME, 1, GA_Defaults(0)); GA_RWHandleF NAME##_handle(NAME##_attr);
#define POINT_ATTRIBUTE_I(NAME) GA_RWAttributeRef NAME##_attr = gdp.findGlobalAttribute(#NAME); if (!NAME##_attr.isValid()) NAME##_attr = gdp.addIntTuple(GA_ATTRIB_POINT, #NAME, 1, GA_Defaults(0)); GA_RWHandleI NAME##_handle(NAME##_attr);
#define GLOBAL_ATTRIBUTE_F(NAME) GA_RWAttributeRef NAME##_attr = gdp.findGlobalAttribute(#NAME); if (!NAME##_attr.isValid()) NAME##_attr = gdp.addFloatTuple(GA_ATTRIB_DETAIL, #NAME, 1, GA_Defaults(0)); GA_RWHandleF NAME##_handle(NAME##_attr);
#define GLOBAL_ATTRIBUTE_I(NAME) GA_RWAttributeRef NAME##_attr = gdp.findGlobalAttribute(#NAME); if (!NAME##_attr.isValid()) NAME##_attr = gdp.addIntTuple(GA_ATTRIB_DETAIL, #NAME, 1, GA_Defaults(0)); GA_RWHandleI NAME##_handle(NAME##_attr);

namespace HinaFlow
{
    enum class CellType : unsigned char
    {
        Fluid = 1,
        Solid = 2,
        Inflow = 3,
        Outflow = 4,
        Empty = 5,
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
    bool CHECK_IS_CELL_SAMPLED(FieldType arg)
    {
        if constexpr (std::is_same_v<FieldType, SIM_VectorField*>) { return arg->isCenterSampled(); }
        return true;
    }

    template <typename FieldType>
    bool CHECK_IS_FACE_SAMPLED(FieldType arg)
    {
        if constexpr (std::is_same_v<FieldType, SIM_VectorField*>) { return arg->isFaceSampled(); }
        return true;
    }

    template <typename FieldType>
    bool CHECK_CELL_VALID(const FieldType* FIELD, const UT_Vector3I& cell) { return FIELD->field()->isValidIndex(static_cast<int>(cell.x()), static_cast<int>(cell.y()), static_cast<int>(cell.z())); }

    template <CellType TYPE>
    bool CHECK_CELL_TYPE(const SIM_IndexField* MARKERS, const UT_Vector3I cell) { return SIM::FieldUtils::getFieldValue(*MARKERS->getField(), cell) == static_cast<exint>(TYPE); }

    template <typename T>
    T TO_1D_IDX(const UT_Vector3T<T>& idx, const UT_Vector3T<T>& res) { return idx.x() + res.x() * (idx.y() + res.y() * idx.z()); }

    template <typename T>
    T TO_1D_IDX(const UT_Vector2T<T>& idx, const UT_Vector2T<T>& res) { return idx.x() + res.x() * idx.y(); }

    template <typename FieldType>
    std::vector<int> GET_AXIS_ITER(const FieldType* FIELD)
    {
        std::vector<int> res;
        int xres, yres, zres;
        FIELD->getVoxelRes(xres, yres, zres);
        if (xres > 1) res.push_back(0);
        if (yres > 1) res.push_back(1);
        if (zres > 1) res.push_back(2);
        return res;
    }

    inline static std::vector<int> GET_AXIS_ITER(const SIM_VectorField* FIELD)
    {
        std::vector<int> res;
        auto _ = FIELD->getTotalVoxelRes();
        int xres = static_cast<int>(_.x()), yres = static_cast<int>(_.y()), zres = static_cast<int>(_.z());
        if (xres > 1) res.push_back(0);
        if (yres > 1) res.push_back(1);
        if (zres > 1) res.push_back(2);
        return res;
    }

    template <typename FieldType, typename T>
    void FILL_FIELD(FieldType* FIELD, T value)
    {
        if constexpr (std::is_same_v<FieldType, SIM_VectorField>)
            for (const int AXIS : GET_AXIS_ITER(FIELD))
                FIELD->getField(AXIS)->makeConstant(value);
        else
            FIELD->getField()->makeConstant(value);
    }

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

    template <typename T>
    UT_Vector3T<T> PROJECT_POINT(const UT_Vector3T<T>& p, const UT_Vector3T<T>& o, const UT_Vector3T<T>& d)
    {
        const UT_Vector3T<T> p_o = p - o;
        const T dist = p_o.dot(d);
        const UT_Vector3T<T> q = p - dist * d;
        return q;
    }

    template <typename T>
    std::pair<UT_Vector2T<T>, UT_Vector2T<T>> PROJECT_BOUNDINGBOX(const UT_BoundingBoxT<T>& bbox, const VGEO_Ray& ray)
    {
        UT_Vector3 ptarray[8]{};
        bbox.getBBoxPoints(ptarray);
        UT_Vector3T<T> l(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
        UT_Vector3T<T> u(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());
        for (auto i : ptarray)
        {
            const UT_Vector2T<T> _ = PROJECT_POINT(i, ray.getP(), ray.getD());
            l.x() = std::min(l.x(), _.x());
            l.y() = std::min(l.y(), _.y());
            u.x() = std::max(u.x(), _.x());
            u.y() = std::max(u.y(), _.y());
        }
        return {l, u};
    }

    inline static std::function Poly6 = [](const UT_Vector3& r, const float h) -> float
    {
        if (const float r_length = r.length(); r_length <= h)
            return 315.0f / (64.0f * static_cast<float>(M_PI) * std::powf(h, 9)) * std::powf(h * h - r_length * r_length, 3);
        return 0.0f;
    };

    inline static std::function gradPoly6 = [](const UT_Vector3& r, const float h) -> UT_Vector3
    {
        if (const float r_length = r.length(); r_length <= h)
            return -945.0f / (32.0f * static_cast<float>(M_PI) * std::powf(h, 9)) * std::powf(h * h - r_length * r_length, 2) * r;
        return {0.0f, 0.0f, 0.0f};
    };

    inline static std::function Spiky = [](const UT_Vector3& r, const float h) -> float
    {
        if (const float r_length = r.length(); r_length <= h)
            return 15.0f / (static_cast<float>(M_PI) * std::powf(h, 6)) * std::powf(h - r_length, 3);
        return 0.0f;
    };

    inline static std::function gradSpiky = [](const UT_Vector3& r, const float h) -> UT_Vector3
    {
        if (const float r_length = r.length(); r_length <= h)
            return -45.0f / (static_cast<float>(M_PI) * std::powf(h, 6)) * std::powf((h - r_length), 2) * r / r_length;
        return {0.0f, 0.0f, 0.0f};
    };

    inline static std::function Cubic = [](const UT_Vector3& r, const float h) -> float
    {
        const float q = r.length() / h;

        if (0 < q && q < 1)
            return 1.0f / (static_cast<float>(M_PI) * h * h * h) * (1.0f - 1.5f * q * q + 0.75f * q * q * q);
        if (1 < q && q < 2)
            return 1.0f / (static_cast<float>(M_PI) * h * h * h) * 0.25f * std::powf(2.0f - q, 3);
        return 0.0f;
    };

    inline static std::function gradCubic = [](const UT_Vector3& r, const float h) -> UT_Vector3
    {
        const float q = r.length() / h;

        if (0 < q && q < 1)
            return 1.0f / (static_cast<float>(M_PI) * h * h * h) * (-3.0f * q + 2.25f * q * q) * r / r.length();
        if (1 < q && q < 2)
            return 1.0f / (static_cast<float>(M_PI) * h * h * h) * (-0.75f * std::pow(2.0f - q, 2)) * r / r.length();

        return {0.0f, 0.0f, 0.0f};
    };
}

#endif //HINAFLOW_COMMON_H
