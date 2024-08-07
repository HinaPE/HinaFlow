#ifndef HINAFLOW_PHIFLOW_SMOKE_H
#define HINAFLOW_PHIFLOW_SMOKE_H

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


#include <SIM/SIM_ScalarField.h>
#include <SIM/SIM_VectorField.h>
#include <SIM/SIM_IndexField.h>
#include <string>
#include <vector>

class UT_WorkBuffer;

namespace HinaFlow::Python
{
    struct PhiFlowSmoke
    {
        enum class Backend : unsigned char
        {
            CPU = 0,
            Torch = 1,
            JAX = 2,
            TensorFlow = 3,
        };

        enum class Extrapolation : unsigned char
        {
            Dirichlet = 0,
            Neumann = 1,
        };

        static void DebugMode(const bool enable);
        static void ImportPhiFlow(const Backend& backend = Backend::Torch);
        static void CreateScalarField(const std::string& name
                                      , const UT_Vector3i& resolution = {100, 100, 100}
                                      , const UT_Vector3& size = {1, 1, 1}
                                      , const UT_Vector3& center = {0, 0, 0}
                                      , const Extrapolation& extrapolation = Extrapolation::Dirichlet
                                      , const float init_value = 0);
        static void CreateVectorField(const std::string& name
                                      , const UT_Vector3i& resolution = {100, 100, 100}
                                      , const UT_Vector3& size = {1, 1, 1}
                                      , const UT_Vector3& center = {0, 0, 0}
                                      , const Extrapolation& extrapolation = Extrapolation::Dirichlet
                                      , const float init_value = 0);
        static void CreateSphereInflow(const std::string& name, const std::string& match_field, const UT_Vector3& center = {0, 0, 0}, const float radius = 0.1);
        static void CompileFunction(const UT_WorkBuffer& expr);
        static void RunFunction(const std::string& func, const std::string& args, const std::string& res);
        static void FetchScalarField(const std::string& name, SIM_ScalarField* FIELD);
        static void FetchVectorField(const std::string& name, SIM_VectorField* FIELD);


        static void CreateScalarField2D(const std::string& name
                                      , const UT_Vector2i& resolution = {100, 100}
                                      , const UT_Vector2& size = {1, 1}
                                      , const UT_Vector2& center = {0, 0}
                                      , const Extrapolation& extrapolation = Extrapolation::Dirichlet
                                      , const float init_value = 0);
        static void CreateVectorField2D(const std::string& name
                                      , const UT_Vector2i& resolution = {100, 100}
                                      , const UT_Vector2& size = {1, 1}
                                      , const UT_Vector2& center = {0, 0}
                                      , const Extrapolation& extrapolation = Extrapolation::Dirichlet
                                      , const float init_value = 0);
        static void CreateSphereInflow2D(const std::string& name, const std::string& match_field, const UT_Vector2& center = {0, 0}, const float radius = 0.1);
        static void FetchScalarField2D(const std::string& name, SIM_ScalarField* FIELD);
        static void FetchVectorField2D(const std::string& name, SIM_VectorField* FIELD);
    };
}


#endif //HINAFLOW_PHIFLOW_SMOKE_H
