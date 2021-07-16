// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Header Definitions for The Shader Binary Extension class
*
****************************************************************************/

#pragma once
#include <cmath>

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
#define __SWIZZLE1( x ) \
    ( ( (x) << 7 ) | ( (x) << 5 ) | ( (x) << 3 ) | ( (x) << 1 ) | 0x1 )

#define __SWIZZLE2( x, y ) \
    ( ( (y) << 7 ) | ( (y) << 5 ) | ( (y) << 3 ) | ( (x) << 1 ) | 0x1 )

#define __SWIZZLE3( x, y, z ) \
    ( ( (z) << 7 ) | ( (z) << 5 ) | ( (y) << 3 ) | ( (x) << 1 ) | 0x1 )

#define __SWIZZLE4( x, y, z, w ) \
    ( ( (w) << 7 ) | ( (z) << 5 ) | ( (y) << 3 ) | ( (x) << 1 ) | 0x1 )

#define __SWIZZLE_ALL \
    __SWIZZLE4(D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Y, \
               D3D10_SB_4_COMPONENT_Z, \
               D3D10_SB_4_COMPONENT_W)

#define __SWIZZLEN( i, value ) \
    ( ( (value) << ( 1 + 2 * (i) ) ) | 0x1 )

#define __UNSWIZZLEN( i, mask ) \
    ( ( (mask) >> ( 1 + 2 * (i) ) ) & 0x3 )

#define __SWIZZLE_X \
    __SWIZZLE1(D3D10_SB_4_COMPONENT_X)

#define __SWIZZLE_Y \
    __SWIZZLE1(D3D10_SB_4_COMPONENT_Y)

#define __SWIZZLE_Z \
    __SWIZZLE1(D3D10_SB_4_COMPONENT_Z)

#define __SWIZZLE_W \
    __SWIZZLE1(D3D10_SB_4_COMPONENT_W)

#define __SWIZZLE_XY \
    __SWIZZLE2(D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Y)

#define __SWIZZLE_YZ \
    __SWIZZLE2(D3D10_SB_4_COMPONENT_Y, \
               D3D10_SB_4_COMPONENT_Z)

#define __SWIZZLE_ZY \
    __SWIZZLE2(D3D10_SB_4_COMPONENT_Z, \
               D3D10_SB_4_COMPONENT_Y)

#define __SWIZZLE_ZW \
    __SWIZZLE2(D3D10_SB_4_COMPONENT_Z, \
               D3D10_SB_4_COMPONENT_W)

#define __SWIZZLE_WX \
    __SWIZZLE2(D3D10_SB_4_COMPONENT_W, \
               D3D10_SB_4_COMPONENT_X)

#define __SWIZZLE_XYZ \
    __SWIZZLE3(D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Y, \
               D3D10_SB_4_COMPONENT_Z)

#define __SWIZZLE_XXY \
    __SWIZZLE3(D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Y)

#define __SWIZZLE_XZX \
    __SWIZZLE3(D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Z, \
               D3D10_SB_4_COMPONENT_X)

#define __SWIZZLE_XXZ \
    __SWIZZLE3(D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Z)

#define __SWIZZLE_ZXY \
    __SWIZZLE3(D3D10_SB_4_COMPONENT_Z, \
               D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Y)

#define __SWIZZLE_YZX \
    __SWIZZLE3(D3D10_SB_4_COMPONENT_Y, \
               D3D10_SB_4_COMPONENT_Z, \
               D3D10_SB_4_COMPONENT_X)

#define __SWIZZLE_XXXY \
    __SWIZZLE4(D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Y)

#define __SWIZZLE_XXYX \
    __SWIZZLE4(D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_X, \
               D3D10_SB_4_COMPONENT_Y, \
               D3D10_SB_4_COMPONENT_X)

#define __WRITEMASK_XY \
    ( D3D10_SB_OPERAND_4_COMPONENT_MASK_X | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_Y )

#define __WRITEMASK_XZ \
    ( D3D10_SB_OPERAND_4_COMPONENT_MASK_X | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_Z )

#define __WRITEMASK_XW \
    ( D3D10_SB_OPERAND_4_COMPONENT_MASK_X | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_W )

#define __WRITEMASK_YZ \
    ( D3D10_SB_OPERAND_4_COMPONENT_MASK_Y | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_Z )

#define __WRITEMASK_ZW \
    ( D3D10_SB_OPERAND_4_COMPONENT_MASK_Z | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_W )

#define __WRITEMASK_YZW \
    ( D3D10_SB_OPERAND_4_COMPONENT_MASK_Y | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_Z | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_W )

#define __WRITEMASK_XYZ \
    ( D3D10_SB_OPERAND_4_COMPONENT_MASK_X | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_Y | \
      D3D10_SB_OPERAND_4_COMPONENT_MASK_Z )

namespace ShaderConv
{

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
inline
void
SetSwizzle( COperandBase& operand, UINT swizzle)
{
    if ( swizzle )
    {
        if ( D3D10_SB_OPERAND_TYPE_IMMEDIATE32 == operand.OperandType() )
        {
            float fTmp[4];
            for ( UINT i = 0; i < 4; ++i )
            {
                const UINT comp = __UNSWIZZLEN( i, swizzle );
                fTmp[i] = operand.m_Valuef[comp];
            }

            for ( UINT i = 0; i < 4; ++i )
            {
                operand.m_Valuef[i] = fTmp[i];
            }
        }
        else
        {
            operand.m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
            for ( UINT i = 0; i < 4; ++i )
            {
                operand.m_Swizzle[i] = (BYTE)__UNSWIZZLEN( i, swizzle );
            }
        }
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
inline
void
SetAsSingleComponent(COperandBase& operand, D3D10_SB_4_COMPONENT_NAME component)
{
    if (operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE32)
    {
        operand.m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
        operand.m_Value[0] = operand.m_Value[component];
    }
    else
    {
        operand.m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    }
    operand.m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE;
    operand.m_ComponentName = component;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
inline
void
SetRelAddress( COperandBase& operand,
               UINT index,
               D3D10_SB_OPERAND_TYPE relRegType,
               UINT relRegIndex,
               D3D10_SB_4_COMPONENT_NAME relComponentName )
{
    operand.m_IndexType[index]                = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
    operand.m_Index[index].m_RelRegType        = relRegType;
    operand.m_Index[index].m_RelIndex        = relRegIndex;
    operand.m_Index[index].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    operand.m_Index[index].m_ComponentName    = relComponentName;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class COperand2DEx: public COperand2D
{
public:
    COperand2DEx( D3D10_SB_OPERAND_TYPE type, UINT regIndex0, UINT regIndex1 ) :
      COperand2D( type, regIndex0, regIndex1 ) {}

    COperand2DEx( D3D10_SB_OPERAND_TYPE type, UINT regIndex0, UINT regIndex1, UINT swizzle ) :
        COperand2D( type, regIndex0, regIndex1 )
    {
        SetSwizzle( *this, swizzle );
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CCBOperand2D: public COperand2DEx
{
public:
    CCBOperand2D( UINT regIndex0, UINT regIndex1 ) :
      COperand2DEx( D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER, regIndex0, regIndex1 ) {}

    CCBOperand2D( UINT regIndex0, UINT regIndex1, UINT swizzle ) :
      COperand2DEx( D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER, regIndex0, regIndex1, swizzle ) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CTempOperand2D: public COperand2DEx
{
public:
    CTempOperand2D( UINT regIndex0, UINT regIndex1 ) :
        COperand2DEx( D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP, regIndex0, regIndex1 ) {}

    CTempOperand2D( UINT regIndex0, UINT regIndex1, UINT swizzle ) :
        COperand2DEx( D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP, regIndex0, regIndex1, swizzle ) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class COperand1Ex : public COperand4
{
public:
    COperand1Ex(D3D10_SB_OPERAND_TYPE type, UINT regIndex, D3D10_SB_4_COMPONENT_NAME component) :
        COperand4(type, regIndex, component) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CTempOperand1 : public COperand1Ex
{
public:
    CTempOperand1(UINT regIndex, D3D10_SB_4_COMPONENT_NAME component) :
        COperand1Ex(D3D10_SB_OPERAND_TYPE_TEMP, regIndex, component) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class COperand4Ex : public COperand4
{
public:

    COperand4Ex(D3D10_SB_OPERAND_TYPE type, UINT regIndex) :
        COperand4(type, regIndex) {}

    COperand4Ex(D3D10_SB_OPERAND_TYPE type, UINT regIndex, UINT swizzle) :
        COperand4(type, regIndex)
    {
        SetSwizzle(*this, swizzle);
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CTempOperandDst : public COperandDst
{
public:

    CTempOperandDst( UINT regIndex, UINT writeMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL ) :
        COperandDst( D3D10_SB_OPERAND_TYPE_TEMP, regIndex, writeMask ) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CTempOperandDst2D : public COperandDst
{
public:

    CTempOperandDst2D( UINT regIndex0, UINT regIndex1, UINT writeMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL ) :
        COperandDst( D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP, regIndex0, regIndex1, writeMask ) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CTempOperand4 : public COperand4Ex
{
public:

    CTempOperand4( UINT regIndex ) :
        COperand4Ex( D3D10_SB_OPERAND_TYPE_TEMP, regIndex ) {}

    CTempOperand4( UINT regIndex, UINT swizzle ) :
        COperand4Ex( D3D10_SB_OPERAND_TYPE_TEMP, regIndex, swizzle ) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CInputOperand4 : public COperand4Ex
{
public:

    CInputOperand4( UINT regIndex ) :
        COperand4Ex( D3D10_SB_OPERAND_TYPE_INPUT, regIndex ) {}

    CInputOperand4( UINT regIndex, UINT swizzle ) :
        COperand4Ex( D3D10_SB_OPERAND_TYPE_INPUT, regIndex, swizzle ) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CResOperand4 : public COperand4Ex
{
public:

    CResOperand4( UINT regIndex ) :
        COperand4Ex( D3D10_SB_OPERAND_TYPE_RESOURCE, regIndex ) {}

    CResOperand4( UINT regIndex, UINT swizzle ) :
        COperand4Ex( D3D10_SB_OPERAND_TYPE_RESOURCE, regIndex, swizzle ) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CWriteMask : public COperandBase
{
public:

    CWriteMask( const COperandBase& operand, UINT writeMask ) : COperandBase( operand )
    {
        m_WriteMask = writeMask;
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CSwizzle : public COperandBase
{
public:

    CSwizzle( const COperandBase& operand, UINT swizzle ) : COperandBase( operand )
    {
        SetSwizzle( *this, swizzle );
    }
};

class CSingleComponent : public COperandBase
{
public:

    CSingleComponent(const COperandBase& operand, D3D10_SB_4_COMPONENT_NAME component) : COperandBase(operand)
    {
        SetAsSingleComponent(*this, component);
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CNegate : public COperandBase
{
public:

    CNegate( const COperandBase& operand ) : COperandBase( operand )
    {
        if ( D3D10_SB_OPERAND_TYPE_IMMEDIATE32 == m_Type )
        {
            for ( UINT i = 0; i < 4; ++i )
            {
                m_Valuef[i] = -m_Valuef[i];
            }
        }
        else
        {
            const D3D10_SB_OPERAND_MODIFIER modifier = m_Modifier;
            switch ( modifier )
            {
            case D3D10_SB_OPERAND_MODIFIER_NONE:
                this->SetModifier( D3D10_SB_OPERAND_MODIFIER_NEG );
                break;

            case D3D10_SB_OPERAND_MODIFIER_NEG:
                this->SetModifier( D3D10_SB_OPERAND_MODIFIER_NONE );
                break;

            case D3D10_SB_OPERAND_MODIFIER_ABS:
                this->SetModifier( D3D10_SB_OPERAND_MODIFIER_ABSNEG );
                break;

            case D3D10_SB_OPERAND_MODIFIER_ABSNEG:
                this->SetModifier( D3D10_SB_OPERAND_MODIFIER_ABS );
                break;
            }
        }
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CAbs : public COperandBase
{
public:

    CAbs( const COperandBase& operand ) : COperandBase( operand )
    {
        if ( D3D10_SB_OPERAND_TYPE_IMMEDIATE32 == m_Type )
        {
            for ( UINT i = 0; i < 4; ++i )
            {
                m_Valuef[i] = fabs( m_Valuef[i] );
            }
        }
        else
        {
            const D3D10_SB_OPERAND_MODIFIER modifier = m_Modifier;
            switch ( modifier )
            {
            case D3D10_SB_OPERAND_MODIFIER_NONE:
            case D3D10_SB_OPERAND_MODIFIER_NEG:
            case D3D10_SB_OPERAND_MODIFIER_ABSNEG:
                this->SetModifier( D3D10_SB_OPERAND_MODIFIER_ABS );
                break;
            }
        }
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CInstructionEx : public CInstruction
{
public:

    CInstructionEx(D3D10_SB_OPCODE_TYPE opCode) :
      CInstruction(opCode) {}

    CInstructionEx( D3D10_SB_OPCODE_TYPE opCode,
                    bool bSaturate,
                    const COperandBase& dstOperand,
                    const COperandBase& srcOperand0 ) :
        CInstruction( opCode, dstOperand, srcOperand0 )
    {
        m_bSaturate = bSaturate;
    }

    CInstructionEx( D3D10_SB_OPCODE_TYPE OpCode,
                    D3D10_SB_INSTRUCTION_TEST_BOOLEAN Test,
                    const COperandBase& Operand0,
                    const COperandBase& Operand1 )
    {
        this->Clear();
        m_OpCode = OpCode;
        m_NumOperands = 2;
        m_bExtended = FALSE;
        m_Test = Test;
        m_Operands[0] = Operand0;
        m_Operands[1] = Operand1;
    }

    CInstructionEx( D3D10_SB_OPCODE_TYPE opCode,
                    bool bSaturate,
                    const COperandBase& dstOperand,
                    const COperandBase& srcOperand0,
                    const COperandBase& srcOperand1 ) :
        CInstruction( opCode, dstOperand, srcOperand0, srcOperand1 )
    {
        m_bSaturate = bSaturate;
    }

    CInstructionEx( D3D10_SB_OPCODE_TYPE opCode,
                    bool bSaturate,
                    const COperandBase& dstOperand,
                    const COperandBase& srcOperand0,
                    const COperandBase& srcOperand1,
                    const COperandBase& srcOperand2 ) :
        CInstruction( opCode, dstOperand, srcOperand0, srcOperand1, srcOperand2 )
    {
        m_bSaturate = bSaturate;
    }

    CInstructionEx( D3D10_SB_OPCODE_TYPE opCode, const COperandBase& dstOperand )
    {
        this->Clear();
        m_OpCode      = opCode;
        m_NumOperands = 1;
        m_bExtended   = FALSE;
        m_Operands[0] = dstOperand;
    }

    void SetOperand(UINT index, const COperandBase& operand)
    {
        m_Operands[index] = operand;
        if (++index > m_NumOperands)
        {
            m_NumOperands = index;
        }
    }

    void SetSaturate(bool bSaturate)
    {
        m_bSaturate = bSaturate;
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CCallInstruction : public CInstruction
{
public:

    CCallInstruction( UINT uiLabel )
    {
        m_Operands[0] = COperand( uiLabel );
        m_Operands[0].m_Type = D3D10_SB_OPERAND_TYPE_LABEL;
        m_Operands[0].m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
        m_Operands[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_Operands[0].m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Operands[0].m_Index[0].m_RegIndex = uiLabel;
        m_NumOperands = 1;
        m_OpCode = D3D10_SB_OPCODE_CALL;
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CLabelInstruction : public CInstruction
{
public:

    CLabelInstruction( UINT uiLabel )
    {
        m_Operands[0] = COperand( uiLabel );
        m_Operands[0].m_Type = D3D10_SB_OPERAND_TYPE_LABEL;
        m_Operands[0].m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
        m_Operands[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_Operands[0].m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Operands[0].m_Index[0].m_RegIndex = uiLabel;
        m_NumOperands = 1;
        m_OpCode = D3D10_SB_OPCODE_LABEL;
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CLoopInstruction : public CInstruction
{
public:

    CLoopInstruction()
    {
        m_NumOperands = 0;
        m_OpCode = D3D10_SB_OPCODE_LOOP;
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CEndLoopInstruction : public CInstruction
{
public:

    CEndLoopInstruction()
    {
        m_NumOperands = 0;
        m_OpCode = D3D10_SB_OPCODE_ENDLOOP;
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CRetInstruction : public CInstruction
{
public:

    CRetInstruction()
    {
        m_NumOperands = 0;
        m_OpCode = D3D10_SB_OPCODE_RET;
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CBreakInstruction : public CInstruction
{
public:

    CBreakInstruction()
    {
        m_NumOperands = 0;
        m_OpCode = D3D10_SB_OPCODE_BREAK;
    }
};

} // namespace ShaderConv