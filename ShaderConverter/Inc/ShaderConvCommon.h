// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All rights reserved.
*
*  Common header file
*
****************************************************************************/
#pragma once

#ifndef SHADER_CONV_COMMON
#define SHADER_CONV_COMMON

#include <assert.h>
#include <cstddef>

#define SHADER_CONV_ASSERT(x) assert(x)

#define NO_DEFAULT  {__assume(0);}

#define IntMax(a,b) (((a) > (b)) ? (a) : (b))
#define IntMin(a,b) (((a) < (b)) ? (a) : (b))

#define IFC(x) {hr = (x); if (FAILED(hr)) { goto Cleanup;} }

#define IFCOOM(x) {if ((x) == NULL) {hr = E_OUTOFMEMORY; goto Cleanup;}}

#define RRETURN(x) return x;
#endif