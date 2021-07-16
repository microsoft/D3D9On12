// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    D3D9ON12_APP_COMPAT_INFO g_AppCompatInfo =
    {
        0, // AnythingTimes0Equals0ShaderMask
    };

    void APIENTRY SetAppCompatData(const D3D9ON12_APP_COMPAT_INFO *pAppCompatData)
    {
        g_AppCompatInfo = *pAppCompatData;
    }

};
