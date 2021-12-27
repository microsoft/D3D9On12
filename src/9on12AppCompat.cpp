// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    D3D9ON12_APP_COMPAT_INFO g_AppCompatInfo =
    {
        0, // AnythingTimes0Equals0ShaderMask
        MAXDWORD, // PSOCacheTrimLimitSize
        MAXDWORD, // PSOCacheTrimLimitAge
        MAXDWORD, // MaxAllocatedUploadHeapSpacePerCommandList
        MAXDWORD, // MaxSRVHeapSize
    };

    void APIENTRY SetAppCompatData(const D3D9ON12_APP_COMPAT_INFO *pAppCompatData)
    {
        g_AppCompatInfo = *pAppCompatData;
    }

};
