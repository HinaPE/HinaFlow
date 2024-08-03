#ifndef HINAFLOW_FLIP_H
#define HINAFLOW_FLIP_H

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


namespace HinaFlow
{
    struct FLIP
    {
        struct Input
        {
        };

        struct Param
        {
        };

        struct Result // Results
        {
        };


        static void P2G(const Input& input, const Param& param, Result& result);
        static void G2P(const Input& input, const Param& param, Result& result);
    };
}


#endif //HINAFLOW_FLIP_H
