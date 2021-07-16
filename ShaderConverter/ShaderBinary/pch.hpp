// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All rights reserved.
*
*  Precompiled header file
*
****************************************************************************/
#pragma once

//
// Disable optimize for speed and instead optimize for size in this library
//
#pragma optimize ("s", on)
#pragma optimize ("t", off)

//
// DDK headers
//
#pragma warning( push,3 )
#undef WIN32_LEAN_AND_MEAN
#include "d3d11.h"
#include "d3d10_1.h"
#define D3D12_TOKENIZED_PROGRAM_FORMAT_HEADER
#include "d3d10umddi.h"
#pragma warning( pop )

#include <ShaderConvCommon.h>

//
// Other external headers
//
#include <stdio.h>

//
// WARP headers
//
#include <ShaderBinary.h>
