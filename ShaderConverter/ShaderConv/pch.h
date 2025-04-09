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
#undef WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <atlbase.h>
#include <strsafe.h>
#include <ShaderConvCommon.h>

#pragma warning(push, 3)
#include <d3d9.h>
#include <d3dhal.h>
#define _d3d9TYPES_H_    // Fix d3d9 types redefinitions errors in d3d10umddi.h
#define D3D12_TOKENIZED_PROGRAM_FORMAT_HEADER
#include <d3d10umddi.h>
#pragma warning(pop)

#include <math.h>

#include <ShaderBinary.h>

#include "ShaderBinaryEx.hpp"
#include "disasm.hpp"

#include <vector>

