#ifndef HINAFLOW_POISSON_H
#define HINAFLOW_POISSON_H

#include <SIM/SIM_ScalarField.h>
#include <SIM/SIM_VectorField.h>
#include <SIM/SIM_IndexField.h>

namespace HinaFlow
{
    struct Possion
    {
        struct Input
        {
            SIM_VectorField* FLOW = nullptr; // required
            SIM_IndexField* MARKER = nullptr; // required
        };

        struct Param
        {
            SIM_RawField::PCG_METHOD preconditioner = SIM_RawField::PCG_METHOD::PCG_MIC;
        };

        struct Result // Results
        {
            SIM_VectorField* FLOW = nullptr; // required
            SIM_ScalarField* PRESSURE = nullptr; // required
            SIM_ScalarField* DIVERGENCE = nullptr; // optional
        };

        static void Solve(const Input& input, const Param& param, Result& result);
        static void SolveMultiThreaded(const Input& input, const Param& param, Result& result);
    };
}


#endif //HINAFLOW_POISSON_H
