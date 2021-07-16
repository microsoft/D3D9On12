// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Implementation for The Shader Binary Extension class
*
****************************************************************************/

#include "pch.h"

#if DBG && WARP_INTERNAL

namespace ShaderConv
{

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
bool
DisassembleInstruction( const CInstruction& instruction,
                        __out_ecount(StringSize) __nullterminated LPSTR pString,
                        __in_range(>,0) UINT StringSize )
{
    char szName[33] = {'\0'};

    //if ( !const_cast<CInstruction&>( instruction ).Disassemble( szName, _countof( szName ) ) )
    //{
    //    return false;
    //}

    int Ret = -1;

    switch ( instruction.OpCode() )
    {
    case D3D10_SB_OPCODE_DCL_TEMPS:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "%s %u", szName, instruction.m_TempsDecl.NumTemps );
        break;

    default:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "%s", szName );
        break;
    }

    return Ret != -1;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
bool
DisassembleOperand( const COperandBase& operand,
                    __out_ecount(StringSize) LPSTR pString,
                    __in_range(>,0) UINT StringSize )
{
    int Ret = -1;

    *pString = '\0';

    switch ( operand.OperandType() )
    {
    case D3D10_SB_OPERAND_TYPE_TEMP:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "r%u", operand.RegIndex(0));
        break;

    case D3D10_SB_OPERAND_TYPE_INPUT:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "v[%u]", operand.RegIndex(0));
        break;

    case D3D10_SB_OPERAND_TYPE_OUTPUT:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "o[%u]", operand.RegIndex(0));
        break;

    case D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "x[Unknown Index]");
        break;

    case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "vec4(%u, %u, %u, %u)", operand.m_Value[0], operand.m_Value[1], operand.m_Value[2], operand.m_Value[3]);
        break;

    case D3D10_SB_OPERAND_TYPE_SAMPLER:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "s%u", operand.RegIndex(0));
        break;

    case D3D10_SB_OPERAND_TYPE_RESOURCE:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "t%u", operand.RegIndex(0));
        break;

    case D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER:
        if ( D3D10_SB_OPERAND_INDEX_IMMEDIATE32 == operand.OperandIndexType( 1 ) )
        {
            Ret = WarpPlatform::StringPrintf(pString, StringSize, "cb%u[%u]", operand.RegIndex(0), operand.RegIndex(1));
        }
        else
        {
            Ret = WarpPlatform::StringPrintf(pString, StringSize, "cb%u", operand.RegIndex(0));
        }
        break;

    case D3D10_SB_OPERAND_TYPE_IMMEDIATE_CONSTANT_BUFFER:
        if ( D3D10_SB_OPERAND_INDEX_IMMEDIATE32 == operand.OperandIndexType( 1 ) )
        {
            Ret = WarpPlatform::StringPrintf(pString, StringSize, "icb%u[%u]", operand.RegIndex(0), operand.RegIndex(1));
        }
        else
        {
            Ret = WarpPlatform::StringPrintf(pString, StringSize, "icb%u", operand.RegIndex(0));
        }
        break;

    case D3D10_SB_OPERAND_TYPE_LABEL:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "L%u", operand.RegIndex(0));
        break;

    case D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "PrimID");
        break;

    case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "oDepth");
        break;

    case D3D10_SB_OPERAND_TYPE_NULL:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "NULL");
        break;

    case D3D10_SB_OPERAND_TYPE_RASTERIZER:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "Rasterizer");
        break;

    case D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK:
        Ret = WarpPlatform::StringPrintf(pString, StringSize, "oMask");
        break;
    }

    return ( Ret != -1 );
}

} // namespace ShaderConv

#endif //DBG && WARP_INTERNAL
