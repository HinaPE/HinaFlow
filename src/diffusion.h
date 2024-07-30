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
            SIM_ScalarField* FIELDS = nullptr; // required
            SIM_VectorField* FIELDV = nullptr; // required
            SIM_IndexField* MARKER = nullptr; // required
            float dt = 1.f; // required
        };

        struct Param
        {
            SIM_RawField::PCG_METHOD preconditioner = SIM_RawField::PCG_METHOD::PCG_MIC;
            float diffusion = 0.1f;
        };

        struct Result // Results
        {
            SIM_ScalarField* FIELDS = nullptr; // required
            SIM_VectorField* FIELDV = nullptr; // required
        };

        static void Solve(const Input& input, const Param& param, Result& result);
        static void SolveMultiThreaded(const Input& input, const Param& param, Result& result);
    };
}


#endif //HINAFLOW_DIFFUSION_H
