// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

extern "C" PFND3DDDI_OPENADAPTER Get9on12OpenAdapter()
{
    return D3D9on12::OpenAdapter;
}

const IID GUID_NULL = {};
