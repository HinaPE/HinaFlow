#ifndef HINAFLOW_DIFFUSION_H
#define HINAFLOW_DIFFUSION_H

#include <SIM/SIM_ScalarField.h>
#include <SIM/SIM_VectorField.h>
#include <SIM/SIM_IndexField.h>

namespace HinaFlow
{
    struct Diffusion
    {
        struct Input
        {
            SIM_ScalarField* FIELDS = nullptr; // optional, but required if FIELDV is not provided
            SIM_VectorField* FIELDV = nullptr; // optional, but required if FIELDS is not provided
            SIM_IndexField* MARKER = nullptr; // required
            float dt = 1.f; // required
        };

        struct Param
        {
            SIM_RawField::PCG_METHOD preconditioner = SIM_RawField::PCG_METHOD::PCG_MIC;
            float diffusion = 0.01f;
        };

        struct Result // Results
        {
            SIM_ScalarField* FIELDS = nullptr; // optional, but required if FIELDV is not provided
            SIM_VectorField* FIELDV = nullptr; // optional, but required if FIELDS is not provided
        };

        static void Solve(const Input& input, const Param& param, Result& result);
        static void SolveMultiThreaded(const Input& input, const Param& param, Result& result);
    };
}


#endif //HINAFLOW_DIFFUSION_H
