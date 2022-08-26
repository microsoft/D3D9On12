// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Implementation for The translator context
*
****************************************************************************/

#include "pch.h"
#include "context.hpp"

namespace ShaderConv
{

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CContext::TranslateInstructions()
{
    const UINT cbCodeSize = m_pShaderDesc->GetCodeSize();
    const DWORD* const pdwCodeBytes = m_pShaderDesc->GetInstructions();
    const DWORD* pdwCurToken = pdwCodeBytes + 1;
    while( *pdwCurToken != D3DPS_END() )
    {
        if ( static_cast<UINT>( reinterpret_cast<const BYTE*>( pdwCurToken ) -
                                reinterpret_cast<const BYTE*>( pdwCodeBytes ) ) > cbCodeSize )
        {
            SHADER_CONV_ASSERT(FALSE);
            return E_FAIL;
        }

        CInstr instr( *pdwCurToken, this );

        const D3DSHADER_INSTRUCTION_OPCODE_TYPE opCode = instr.GetOpCode();

        switch ( opCode )
        {
        case D3DSIO_NOP:
        case D3DSIO_COMMENT:
        case D3DSIO_DCL:
            pdwCurToken += instr.GetLength();
            continue;

        case D3DSIO_DEF:
        case D3DSIO_DEFI:
        case D3DSIO_DEFB:
            pdwCurToken += instr.GetLength();
            continue;
        }

        //
        // Decode src and dst params
        //

        ++pdwCurToken;

        //
        // Some tokens have no parameters at all (like endrep)
        // so there is nothing to decode
        //
        if ( ( *pdwCurToken & ( 1L << 31 ) ) )
        {
            switch ( opCode )
            {
            case D3DSIO_CALL:
            case D3DSIO_REP:
            case D3DSIO_LOOP:
            case D3DSIO_CALLNZ:
            case D3DSIO_IF:
            case D3DSIO_IFC:
            case D3DSIO_BREAK:
            case D3DSIO_BREAKC:
            case D3DSIO_BREAKP:
            case D3DSIO_LABEL:
            case D3DSIO_TEXKILL:
                // No dst param, only src params.
                break;

            default:
                pdwCurToken = instr.SetDstToken( pdwCurToken );
                break;
            }

            if ( instr.HasPredicate() )
            {
                pdwCurToken = instr.SetPredicate( pdwCurToken );
            }

            while ( *pdwCurToken & ( 1L << 31 ) )
            {
                pdwCurToken = instr.AddSrcToken( pdwCurToken );
            }
        }

        switch ( opCode )
        {
        case D3DSIO_MOV:
            this->Translate_MOV( instr );
            break;

        case D3DSIO_ADD:
            this->Translate_ADD( instr );
            break;

        case D3DSIO_SUB:
            this->Translate_SUB( instr );
            break;

        case D3DSIO_MAD:
            this->Translate_MAD( instr );
            break;

         case D3DSIO_MUL:
            this->Translate_MUL( instr );
            break;

        case D3DSIO_RCP:
            this->Translate_RCP( instr );
            break;

        case D3DSIO_RSQ:
            this->Translate_RSQ( instr );
            break;

        case D3DSIO_DP3:
            this->Translate_DP3( instr );
            break;

        case D3DSIO_DP4:
            this->Translate_DP4( instr );
            break;

        case D3DSIO_MIN:
            this->Translate_MIN( instr );
            break;

        case D3DSIO_MAX:
            this->Translate_MAX( instr );
            break;

        case D3DSIO_SLT:
            this->Translate_SLT( instr );
            break;

        case D3DSIO_SGE:
            this->Translate_SGE( instr );
            break;

        case D3DSIO_EXP:
            this->Translate_EXP( instr );
            break;

        case D3DSIO_LOG:
            this->Translate_LOG( instr );
            break;

        case D3DSIO_LIT:
            this->Translate_LIT( instr );
            break;

        case D3DSIO_DST:

            this->Translate_DST( instr );
            break;

        case D3DSIO_LRP:
            this->Translate_LRP( instr );
            break;

        case D3DSIO_FRC:
            this->Translate_FRC( instr );
            break;

        case D3DSIO_M4x4:
            this->Translate_M4x4( instr );
            break;

        case D3DSIO_M4x3:
            this->Translate_M4x3( instr );
            break;

        case D3DSIO_M3x4:
            this->Translate_M3x4( instr );
            break;

        case D3DSIO_M3x3:
            this->Translate_M3x3( instr );
            break;

        case D3DSIO_M3x2:
            this->Translate_M3x2( instr );
            break;

        case D3DSIO_CALL:
            this->Translate_CALL( instr );
            break;

        case D3DSIO_CALLNZ:
            this->Translate_CALLNZ( instr );
            break;

        case D3DSIO_LOOP:
            this->Translate_LOOP( instr );
            break;

        case D3DSIO_RET:
            this->Translate_RET( instr );
            break;

        case D3DSIO_ENDLOOP:
            this->Translate_ENDLOOP( instr );
            break;

        case D3DSIO_LABEL:
            this->Translate_LABEL( instr );
            break;

        case D3DSIO_POW:
            this->Translate_POW( instr );
            break;

        case D3DSIO_CRS:
            this->Translate_CRS( instr );
            break;

        case D3DSIO_SGN:
            this->Translate_SGN( instr );
            break;

        case D3DSIO_ABS:
            this->Translate_ABS( instr );
            break;

        case D3DSIO_NRM:
            this->Translate_NRM( instr );
            break;

        case D3DSIO_SINCOS:
            this->Translate_SINCOS( instr );
            break;

        case D3DSIO_REP:
            this->Translate_REP( instr );
            break;

        case D3DSIO_ENDREP:
            this->Translate_ENDREP( instr );
            break;

        case D3DSIO_IF:
            this->Translate_IF( instr );
            break;

        case D3DSIO_IFC:
            this->Translate_IFC( instr );
            break;

        case D3DSIO_ELSE:
            this->Translate_ELSE( instr );
            break;

        case D3DSIO_ENDIF:
            this->Translate_ENDIF( instr );
            break;

        case D3DSIO_BREAK:
            pdwCurToken = this->Translate_BREAK( instr, pdwCurToken );
            break;

        case D3DSIO_BREAKC:
            this->Translate_BREAKC( instr );
            break;

        case D3DSIO_MOVA:
            this->Translate_MOVA( instr );
            break;

        case D3DSIO_TEXCOORD:
            this->Translate_TEXCOORD( instr );
            break;

        case D3DSIO_TEXKILL:
            this->Translate_TEXKILL( instr );
            break;

        case D3DSIO_TEX:
            this->Translate_TEX( instr );
            break;

        case D3DSIO_TEXLDD:
            this->Translate_TEXLDD( instr );
            break;

        case D3DSIO_TEXBEM:
            this->Translate_TEXBEM( instr );
            break;

        case D3DSIO_TEXBEML:
            this->Translate_TEXBEML( instr );
            break;

        case D3DSIO_TEXREG2AR:
            this->Translate_TEXREG2AR( instr );
            break;

        case D3DSIO_TEXREG2GB:
            this->Translate_TEXREG2GB( instr );
            break;

        case D3DSIO_TEXREG2RGB:
            this->Translate_TEXREG2RGB( instr );
            break;

        case D3DSIO_TEXM3x2PAD:
            this->Translate_TEXM3x2PAD( instr );
            break;

        case D3DSIO_TEXM3x2TEX:
            this->Translate_TEXM3x2TEX( instr );
            break;

        case D3DSIO_TEXM3x2DEPTH:
            this->Translate_TEXM3x2DEPTH( instr );
            break;

        case D3DSIO_TEXM3x3PAD:
            this->Translate_TEXM3x3PAD( instr );
            break;

        case D3DSIO_TEXM3x3:
            this->Translate_TEXM3x3( instr );
            break;

        case D3DSIO_TEXM3x3TEX:
            this->Translate_TEXM3x3TEX( instr );
            break;

        case D3DSIO_TEXM3x3SPEC:
            this->Translate_TEXM3x3SPEC( instr );
            break;

        case D3DSIO_TEXM3x3VSPEC:
            this->Translate_TEXM3x3VSPEC( instr );
            break;

        case D3DSIO_CND:
            this->Translate_CND( instr );
            break;

        case D3DSIO_TEXDP3TEX:
            this->Translate_TEXDP3TEX( instr );
            break;

        case D3DSIO_TEXDP3:
            this->Translate_TEXDP3( instr );
            break;

        case D3DSIO_TEXDEPTH:
            this->Translate_TEXDEPTH( instr );
            break;

        case D3DSIO_CMP:
            this->Translate_CMP( instr );
            break;

        case D3DSIO_BEM:
            this->Translate_BEM( instr );
            break;

        case D3DSIO_DP2ADD:
            this->Translate_DP2ADD( instr );
            break;

        case D3DSIO_DSX:
            this->Translate_DSX( instr );
            break;

        case D3DSIO_DSY:
            this->Translate_DSY( instr );
            break;

         case D3DSIO_BREAKP:
            this->Translate_BREAKP( instr );
            break;

        case D3DSIO_PHASE:
            this->Translate_PHASE( instr );
            break;

        case D3DSIO_SETP:
            this->Translate_SETP( instr );
            break;

        case D3DSIO_EXPP:
            this->Translate_EXPP( instr );
            break;

        case D3DSIO_LOGP:
            this->Translate_LOGP( instr );
            break;

        case D3DSIO_TEXLDL:
            this->Translate_TEXLDL( instr );
            break;

         default:
            SHADER_CONV_ASSERT(!"Invalid instruction opcode.");
            return E_INVALIDARG;
        }

        m_bLastInstrBreak = opCode == D3DSIO_BREAK;

        //
        // Handle predicated instructions
        //

        if ( instr.HasPredicate() )
        {
            // Clear the predicate flag
            instr.ClearPredicateFlag();

            // If D3DSPSM_NOT is set, flip the order of dwSrcToken and SaveReg
            const COperandBase operands[2] = { instr.CreateDstOperand(),
                                               CTempOperand4( SREG_PRED ) };
            const DWORD dwPredicate = instr.GetPredicate();
            const UINT srcSelect = ( dwPredicate & D3DSPSM_NOT ) ? 2 : 1;

            m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MOVC,
                                  operands[0],
                                  this->EmitPredOperand( instr ),
                                  operands[srcSelect & 1 ],
                                  operands[srcSelect >> 1] ) );
        }
    }

    SHADER_CONV_ASSERT( 0 == m_controlFlowDepth );

    return S_OK;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_MOV( const CInstr& instr )
{
    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    const DWORD dwRegType = D3DSI_GETREGTYPE_RESOLVING_CONSTANTS( instr.GetDstToken() );
    if ( D3DSPR_ADDR == dwRegType )
    {
        // round_ni s0, src0
        // ftoi dest, s0

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ROUND_NI,
                          CTempOperandDst( SREG_TMP0 ),
                          src0 ) );

        this->EmitDstInstruction( instr.GetModifiers(),
                                  D3D10_SB_OPCODE_FTOI,
                                  dest,
                                  CTempOperand4( SREG_TMP0 ) );
    }
    else
    {
        // mov dest, src0

        this->EmitDstInstruction( instr.GetModifiers(),
                                  D3D10_SB_OPCODE_MOV,
                                  dest,
                                  src0 );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_ADD( const CInstr& instr )
{
    // add dest, src0, src1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_ADD,
                              dest,
                              src0,
                              src1 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_SUB( const CInstr& instr )
{
    // add dest, src0, -src1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_ADD,
                              dest,
                              src0,
                              CNegate( src1 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_MAD( const CInstr& instr )
{
    // mad dest, src0, src1, src2

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );
    const COperandBase src2 = this->EmitSrcOperand( instr, 2 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MAD,
                              dest,
                              src0,
                              src1,
                              src2 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_MUL( const CInstr& instr )
{
    // mul dest, src0, src1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MUL,
                              dest,
                              src0,
                              src1 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_RSQ( const CInstr& instr )
{
    // rsq dest, src0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_RSQ,
                              dest,
                              CAbs( src0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_DP3( const CInstr& instr )
{
    // dp3 dest, src0, src1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0, 0, 3 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1, 0, 3 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_DP3,
                              dest,
                              src0,
                              src1 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_DP4( const CInstr& instr )
{
    // dp4 dest, src0, src1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_DP4,
                              dest,
                              src0,
                              src1 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_MIN( const CInstr& instr )
{
    // lt s0, src0, src1
    // movc dest, s0, src0, src1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_LT,
                      CTempOperandDst( SREG_TMP0 ),
                      src0,
                      src1 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOVC,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              src0,
                              src1 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_MAX( const CInstr& instr )
{
    // ge s0, src0, src1
    // movc dest, s0, src0, src1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_GE,
                      CTempOperandDst( SREG_TMP0 ),
                      src0,
                      src1 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOVC,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              src0,
                              src1 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_SLT( const CInstr& instr )
{
    // lt s0, src0, src1
    // movc dest, s0, vec4(1.0f), vec4(0.0f)

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_LT,
                      CTempOperandDst( SREG_TMP0 ),
                      src0,
                      src1 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOVC,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              COperand( 1.0f ),
                              COperand( 0.0f ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_EXP( const CInstr& instr )
{
    // exp dest, src0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_EXP,
                              dest,
                              src0 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_EXPP( const CInstr& instr )
{
    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    SHADER_CONV_ASSERT( __IS_VS( m_version ) );
    if ( m_version >= D3DVS_VERSION(2,0) )
    {
        // exp s0.w, src0
        // and dest, s0.w, vec4(0xffffff00)

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_EXP,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          src0 ) );

        this->EmitDstInstruction( instr.GetModifiers(),
                                  D3D10_SB_OPCODE_AND,
                                  dest,
                                  CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                                  COperand( 0xffffff00 ) );
    }
    else
    {
        // round_ni s0.y, src0
        // exp s0.x, s0.y
        // add s0.y, src0, -s0.y
        // exp s0.z, src0
        // and s0.z, s0.z, vec4(0xffffff00)
        // mov s0.w, vec4(1.0f)
        // mov dest, s0

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ROUND_NI,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                          src0 ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_EXP,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ADD,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                          src0,
                          CNegate( CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_EXP,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                          src0 ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_AND,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_Z ),
                          COperand( 0xffffff00 ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          COperand( 1.0f ) ) );

        this->EmitDstInstruction( instr.GetModifiers(),
                                  D3D10_SB_OPCODE_MOV,
                                  dest,
                                  CTempOperand4( SREG_TMP0 ) );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_FRC( const CInstr& instr )
{
    // frc dest, src0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_FRC,
                              dest,
                              src0 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_MOVA( const CInstr& instr )
{
    // round_ne s0, src0
    // ftoi dest, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ROUND_NE,
                          CTempOperandDst( SREG_TMP0 ),
                          src0 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_FTOI,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_LIT( const CInstr& instr )
{
    // min s0.x, src0.w, vec4(127.9961f)
    // max s0.x, s0.x, vec4(-127.9961f)
    // max s0.yz, src0.xxy, vec4(0.0f)
    // log s0.z, s0.z
    // mul s0.z, s0.z, s0.x
    // exp s0.z, s0.z
    // mov s0.xw, vec4(1.0f, 0.0f, 0.0f, 1.0f)
    // mov dest,  s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MIN,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CSwizzle( src0, __SWIZZLE_W ),
                      COperand( 127.9961f ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAX,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                      COperand( -127.9961f ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAX,
                      CTempOperandDst( SREG_TMP0, __WRITEMASK_YZ ),
                      CSwizzle( src0, __SWIZZLE_XXY ),
                      COperand( 0.0f ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_LOG,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Z ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Z ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_EXP,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Z ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV,
                      CTempOperandDst( SREG_TMP0, __WRITEMASK_XW ),
                      COperand( 1.0f, 0.0f, 0.0f, 1.0f ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_DST( const CInstr& instr )
{
    // mov s0.x, vec4(1.0f)
    // mul s0.y, src0.y, src1.y
    // mov s0.z, src0.z
    // mov s0.w, src1.w
    // mov dest,  s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      COperand( 1.0f ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CSwizzle( src0, __SWIZZLE_Y ),
                      CSwizzle( src1, __SWIZZLE_Y ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      CSwizzle( src0, __SWIZZLE_Z ) ) );


    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                      CSwizzle( src1, __SWIZZLE_W ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_LRP( const CInstr& instr )
{
    // add s0,  src1, -src2
    // mad dest, src0, s0, src2

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );
    const COperandBase src2 = this->EmitSrcOperand( instr, 2 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0 ),
                      src1,
                      CNegate( src2 ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MAD,
                              dest,
                              src0,
                              CTempOperand4( SREG_TMP0 ),
                              src2 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_M4x4( const CInstr& instr )
{
    // dp4 s0.x, src0, src1[0]
    // dp4 s0.y, src0, src1[1]
    // dp4 s0.z, src0, src1[2]
    // dp4 s0.w, src0, src1[3]
    // mov dest, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP4,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 0 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP4,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 1 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP4,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 2 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP4,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 3 ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_M4x3( const CInstr& instr )
{
    // dp4 s0.x, src0, src1[0]
    // dp4 s0.y, src0, src1[1]
    // dp4 s0.z, src0, src1[2]
    // mov dest.xyz, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP4,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 0 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP4,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 1 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP4,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 2 ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              CWriteMask( dest, __WRITEMASK_XYZ ),
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_M3x4( const CInstr& instr )
{
    // dp3 s0.x, src0, src1[0]
    // dp3 s0.y, src0, src1[1]
    // dp3 s0.z, src0, src1[2]
    // dp3 s0.w, src0, src1[3]
    // mov dest, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 0 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 1 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 2 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 3 ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_M3x3( const CInstr& instr )
{
    // dp3 s0.x, src0, src1[0]
    // dp3 s0.y, src0, src1[1]
    // dp3 s0.z, src0, src1[2]
    // mov dest.xyz, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 0 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 1 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 2 ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              CWriteMask( dest, __WRITEMASK_XYZ ),
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_M3x2( const CInstr& instr )
{
    // dp3 s0.x, src0, src1[0]
    // dp3 s0.y, src0, src1[1]
    // mov dest.xy, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 0 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      src0,
                      this->EmitSrcOperand( instr, 1, 1 ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              CWriteMask( dest, __WRITEMASK_XY ),
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_POW( const CInstr& instr )
{
    // log s0,  abs( src0 )
    // mul s0,  s0, src1
    // exp dest, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_LOG,
                      CTempOperandDst( SREG_TMP0 ),
                      CAbs( src0 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP0 ),
                      src1 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_EXP,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_CRS( const CInstr& instr )
{
    // mul s0.xy, src0.yz, src1.zy
    // add s0.x, s0.x, -s0.y
    // mul s0.yz, src0.xzx, src1.xxz
    // add s0.y, s0.y, -s0.z
    // mul s0.zw, src0.xxxy, src1.xxyx
    // add s0.z, s0.z, -s0.w
    // mov dest, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, __WRITEMASK_XY ),
                      CSwizzle( src0, __SWIZZLE_YZ ),
                      CSwizzle( src1, __SWIZZLE_ZY ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                      CNegate( CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, __WRITEMASK_YZ ),
                      CSwizzle( src0, __SWIZZLE_XZX ),
                      CSwizzle( src1, __SWIZZLE_XXZ ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Y ),
                      CNegate( CTempOperand4( SREG_TMP0, __SWIZZLE_Z ) ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, __WRITEMASK_ZW ),
                      CSwizzle( src0, __SWIZZLE_XXXY ),
                      CSwizzle( src1, __SWIZZLE_XXYX ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Z ),
                      CNegate( CTempOperand4( SREG_TMP0, __SWIZZLE_W ) ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_SGN( const CInstr& instr )
{
    // eq   s0,  src0, vec4(0.0f)
    // movc s1,  s0,   vec4(0.0f), vec4(1.0f)
    // lt   s0,  src0, vec4(0.0f)
    // movc dest, s0,  vec4(-1.0f), s1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_EQ,
                      CTempOperandDst( SREG_TMP0 ),
                      src0,
                      COperand( 0.0f ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOVC,
                      CTempOperandDst( SREG_TMP1 ),
                      CTempOperand4( SREG_TMP0 ),
                      COperand( 0.0f ),
                      COperand( 1.0f ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_LT,
                      CTempOperandDst( SREG_TMP0 ),
                      src0,
                      COperand( 0.0f ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOVC,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              COperand( -1.0f ),
                              CTempOperand4( SREG_TMP1 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_ABS( const CInstr& instr )
{
    // mov dest, abs( src0 )

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CAbs( src0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_NRM( const CInstr& instr )
{
    // dp3  s0.x, src0, src0
    // rsq  s0.y, s0.x
    // movc s0.z, s0.x, s0.y, vec4(FLT_MAX)
    // mul  dest, src0, s0.z

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0,
                      src0 ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_RSQ,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction(   D3D10_SB_OPCODE_MOVC,
                        CTempOperandDst(SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z),
                        CTempOperand4(SREG_TMP0, __SWIZZLE_X),
                        CTempOperand4(SREG_TMP0, __SWIZZLE_Y),
                        COperand( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MUL,
                              dest,
                              src0,
                              CTempOperand4( SREG_TMP0, __SWIZZLE_Z ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_SINCOS( const CInstr& instr )
{
    // sincos s0.x, s0.y, src0
    // mov dest,  s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_SINCOS,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_CMP( const CInstr& instr )
{
    // ge s0 src0 vec4(0.0f)
    // movc dest, s0, src1, src2

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );
    const COperandBase src2 = this->EmitSrcOperand( instr, 2 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_GE,
                      CTempOperandDst( SREG_TMP0 ),
                      src0,
                      COperand( 0.0f ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOVC,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              src1,
                              src2 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_DP2ADD( const CInstr& instr )
{
    // dp2 s0, src0, src1
    // add dest, s0, src2

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0, 0, 2 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1, 0, 2 );
    const COperandBase src2 = this->EmitSrcOperand( instr, 2 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP2,
                      CTempOperandDst( SREG_TMP0 ),
                      src0,
                      src1 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_ADD,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              src2 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_RCP( const CInstr& instr )
{
    // div  dest, vec4(1.0f), src0
    // movc dest, src0, dest, vec4(FLT_MAX)

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_DIV,
                              dest,
                              COperand( 1.0f ),
                              src0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOVC,
                              dest,
                              src0,
                              dest,
                              COperand( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_CND( const CInstr& instr )
{
    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version < D3DPS_VERSION(2,0) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );
    const COperandBase src2 = this->EmitSrcOperand( instr, 2 );

    if ( m_version > D3DPS_VERSION(1,3) )
    {
        // lt s0, vec4(0.5f), src0
        // movc dest, s0, src1, src2

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_LT,
                          CTempOperandDst( SREG_TMP0 ),
                          COperand( 0.5f ),
                          src0 ) );

        this->EmitDstInstruction( instr.GetModifiers(),
                                  D3D10_SB_OPCODE_MOVC,
                                  dest,
                                  CTempOperand4( SREG_TMP0 ),
                                  src1,
                                  src2 );
    }
    else
    {
        if ( instr.HasCoissue() )
        {
            // mov dest, src1

            this->EmitDstInstruction( instr.GetModifiers(),
                                      D3D10_SB_OPCODE_MOV,
                                      dest,
                                      src1 );
        }
        else
        {
            // lt s0.x, vec4(0.5f), src0.a
            // movc dest, s0.x, src1, src2

            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_LT,
                              CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                              COperand( 0.5f ),
                              CSwizzle( src0, __SWIZZLE_W ) ) );

            this->EmitDstInstruction( instr.GetModifiers(),
                                      D3D10_SB_OPCODE_MOVC,
                                      dest,
                                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                                      src1,
                                      src2 );
        }
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_SGE( const CInstr& instr )
{
    // ge s0, src0, src1
    // movc dest, s0, vec4(1.0f), vec4(0.0f)

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_GE,
                      CTempOperandDst( SREG_TMP0 ),
                      src0,
                      src1 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOVC,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              COperand( 1.0f ),
                              COperand( 0.0f ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_LOG( const CInstr& instr )
{
    // log dest, abs( src0 )

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_LOG,
                              dest,
                              CAbs( src0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_LOGP( const CInstr& instr )
{
    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    SHADER_CONV_ASSERT( __IS_VS( m_version ) );
    if ( m_version >= D3DVS_VERSION(2,0) )
    {
        // log s0.w, abs( src0 )
        // and dest, s0.w, vec4(0xffffff00)

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_LOG,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          CAbs( src0 ) ) );

        this->EmitDstInstruction( instr.GetModifiers(),
                                  D3D10_SB_OPCODE_AND,
                                  dest,
                                  CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                                  COperand( 0xffffff00 ) );
    }
    else
    {
        // mov  s0.w, abs(src0)
        // ushr s0.x, s0.w, vec4(23)
        // iadd s0.x, s0.x, vec4(-127)
        // itof s0.x, s0.x
        // and  s0.y, s0.w, vec4(0x7fffff)
        // or   s0.y, s0.y, vec4(0x3f800000)
        // log  s0.z, s0.w
        // and  s0.z, s0.z, vec4(0xffffff00)
        // eq   s0.w, s0.w, vec4(0.0f)
        // movc s0.xz, s0.w, vec4(0xFF7FFFFF), s0.xxz
        // movc s0.y, s0.w, vec4(1.0f), s0.y
        // mov  s0.w, vec4(1.0f)
        // mov  dest, s0

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          CAbs( src0 ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_USHR,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                          COperand( 23 ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_IADD,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                          COperand( -127 ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ITOF,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_AND,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                          COperand( 0x7fffff ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_OR,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_Y ),
                          COperand( 0x3f800000 ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_LOG,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_W ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_AND,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_Z ),
                          COperand( 0xffffff00 ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_EQ,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                          COperand( 0.0f ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOVC,
                          CTempOperandDst( SREG_TMP0, __WRITEMASK_XZ ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                          COperand( 0xFF7FFFFF ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_XXZ ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOVC,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                          COperand( 1.0f ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          COperand( 1.0f ) ) );

        this->EmitDstInstruction( instr.GetModifiers(),
                                  D3D10_SB_OPCODE_MOV,
                                  dest,
                                  CTempOperand4( SREG_TMP0 ) );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_DSX( const CInstr& instr )
{
    // dsx dest, src0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D11_SB_OPCODE_DERIV_RTX_COARSE,
                              dest,
                              src0 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_DSY( const CInstr& instr )
{
    // dsy dest, src0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D11_SB_OPCODE_DERIV_RTY_COARSE,
                              dest,
                              src0 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_PHASE( const CInstr& /*instr*/ )
{
    // Phase blocks are not needed by this compiler.
    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( D3DPS_VERSION(1,4) == m_version );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_LABEL( const CInstr& instr )
{
    // label #n

    m_pShaderAsm->EmitInstruction(
        CLabelInstruction( D3DSI_GETREGNUM( instr.GetSrcToken( 0 ) ) ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_REP( const CInstr& instr )
{
    // mov rep.x, src0.x
    // loop
    // breakc_z rep.x

    ++m_controlFlowDepth;
    ++m_loopNestingDepth;

    this->AllocateLoopRegister();

    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV,
                      CTempOperandDst( this->GetLoopRegister(), D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CSwizzle( src0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction( CLoopInstruction() );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_BREAKC,
                      CTempOperand1( this->GetLoopRegister(), D3D10_SB_4_COMPONENT_X),
                      D3D10_SB_INSTRUCTION_TEST_ZERO ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_ENDREP( const CInstr& /*instr*/ )
{
    // iadd rep.x, rep.x, vec4(-1)
    // endloop

    // The loop counter isn't necessary if there was an unconditional break in the loop
    if (!m_bLastInstrBreak)
    {
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_IADD,
                          CTempOperandDst( this->GetLoopRegister(),
                                           D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( this->GetLoopRegister(), __SWIZZLE_X ),
                          COperand( -1 ) ) );
    }

    m_pShaderAsm->EmitInstruction( CEndLoopInstruction() );

    --m_loopNestingDepth;
    --m_controlFlowDepth;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_LOOP( const CInstr& instr )
{
    // mov rep, src1
    // loop
    // breakc_z rep.x
    // ? mov aL, rep.y

    ++m_controlFlowDepth;
    ++m_loopNestingDepth;

    this->AllocateLoopRegister();

    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV,
                      CTempOperandDst( this->GetLoopRegister() ),
                      src1 ) );

    m_pShaderAsm->EmitInstruction( CLoopInstruction() );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_BREAKC,
                      CTempOperand1( this->GetLoopRegister(), D3D10_SB_4_COMPONENT_X),
                      D3D10_SB_INSTRUCTION_TEST_ZERO ) );

    if ( m_inputRegs.aL != INVALID_INDEX )
    {
        // Update the loop counter register value
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          CTempOperandDst( m_inputRegs.aL ),
                          CTempOperand4( this->GetLoopRegister(), __SWIZZLE_Y ) ) );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_ENDLOOP( const CInstr& /*instr*/ )
{
    // iadd rep.x, rep.x, vec4(-1)
    // iadd rep.y, rep.y, rep.z
    // endloop
    // ? mov aL, rep.y

    // The loop counter isn't necessary if there was an unconditional break in the loop
    if (!m_bLastInstrBreak)
    {
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_IADD,
                          CTempOperandDst( this->GetLoopRegister(),
                                           D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( this->GetLoopRegister(), __SWIZZLE_X ),
                          COperand( -1 ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_IADD,
                          CTempOperandDst( this->GetLoopRegister(),
                                           D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                          CTempOperand4( this->GetLoopRegister(), __SWIZZLE_Y ),
                          CTempOperand4( this->GetLoopRegister(), __SWIZZLE_Z ) ) );
    }

    m_pShaderAsm->EmitInstruction( CEndLoopInstruction() );

    --m_loopNestingDepth;
    --m_controlFlowDepth;

    if ( m_loopNestingDepth != 0xff &&
         m_inputRegs.aL != INVALID_INDEX )
    {
        // Restore the loop counter register value
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          CTempOperandDst( m_inputRegs.aL ),
                          CTempOperand4( this->GetLoopRegister(), __SWIZZLE_Y ) ) );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_CALL( const CInstr& instr )
{
    // call

    m_pShaderAsm->EmitInstruction(
        CCallInstruction( D3DSI_GETREGNUM( instr.GetSrcToken( 0 ) ) ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_CALLNZ( const CInstr& instr )
{
    // call_[z|nz] src0.c[0], label

    SHADER_CONV_ASSERT( !instr.HasPredicate() );

    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1, 0, 1 );
    const DWORD dwSrcToken1 = instr.GetSrcToken( 1 );

    m_pShaderAsm->EmitInstruction(
        CInstructionEx( D3D10_SB_OPCODE_CALLC,
                        ( dwSrcToken1 & D3DSPSM_NOT ) ?
                            D3D10_SB_INSTRUCTION_TEST_ZERO :
                            D3D10_SB_INSTRUCTION_TEST_NONZERO,
                        CSingleComponent(src1, (D3D10_SB_4_COMPONENT_NAME)src1.SwizzleComponent(0)),
                        src0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_RET( const CInstr& /*instr*/ )
{
    // ret

    // If we are not inside a nested call,
    // ensure that the ouput registers are written before exiting the shader

    if ( 0 == m_controlFlowDepth )
    {
        this->WriteOutputs();
    }

    m_pShaderAsm->EmitInstruction( CRetInstruction() );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_IF( const CInstr& instr )
{
    // if_[z|nz] src0.c[0]

    SHADER_CONV_ASSERT( !instr.HasPredicate() );

    ++m_controlFlowDepth;

    const COperandBase src0 = this->EmitSrcOperand( instr, 0, 0, 1 );
    const DWORD dwSrcToken  = instr.GetSrcToken( 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_IF, 
                      CSingleComponent(src0, (D3D10_SB_4_COMPONENT_NAME)src0.SwizzleComponent(0)),
                      ( dwSrcToken & D3DSPSM_NOT ) ?
                        D3D10_SB_INSTRUCTION_TEST_ZERO :
                        D3D10_SB_INSTRUCTION_TEST_NONZERO ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_IFC( const CInstr& instr )
{
    // <comp> s0.x, src0, src1
    // if_nz s0.x,

    ++m_controlFlowDepth;

    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const D3DSHADER_COMPARISON compare = D3DSI_GETCOMPARISON( instr.GetToken() );

    this->Compare( compare,
                   CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                   src0,
                   src1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_IF,
                      CTempOperand1( SREG_TMP0, D3D10_SB_4_COMPONENT_X),
                      D3D10_SB_INSTRUCTION_TEST_NONZERO ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_ELSE( const CInstr& /*instr*/ )
{
    // else

    m_pShaderAsm->EmitInstruction( CInstruction( D3D10_SB_OPCODE_ELSE ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_ENDIF( const CInstr& /*instr*/ )
{
    // endif

    m_pShaderAsm->EmitInstruction( CInstruction( D3D10_SB_OPCODE_ENDIF ) );
    SHADER_CONV_ASSERT( m_controlFlowDepth );
    --m_controlFlowDepth;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
const DWORD*
CContext::Translate_BREAK( const CInstr& /*instr*/, const DWORD* pdwCurToken )
{
    // break

    m_pShaderAsm->EmitInstruction( CBreakInstruction() );

    // D3D9 allows computation instructions to follow a break instruction
    // inside the conditional branch statement even though it is impossible
    // for those instructions to ever execute
    // The D3D10 shader assembler does not allow this
    // so skip all computational instructions that follow this instruction

    for ( bool bContinue = true; bContinue; )
    {
        const DWORD dwInstr = *pdwCurToken++;
        D3DSHADER_INSTRUCTION_OPCODE_TYPE opCode = (D3DSHADER_INSTRUCTION_OPCODE_TYPE)( dwInstr & D3DSI_OPCODE_MASK );
        switch ( opCode )
        {
        case D3DSIO_COMMENT:
            pdwCurToken += ( dwInstr & D3DSI_COMMENTSIZE_MASK ) >> D3DSI_COMMENTSIZE_SHIFT;
            break;

        case D3DSIO_DCL:
            pdwCurToken += 2;
            break;

        case D3DSIO_RET:
        case D3DSIO_ENDLOOP:
        case D3DSIO_ENDREP:
        case D3DSIO_ELSE:
        case D3DSIO_ENDIF:
        case D3DSIO_END:
            bContinue = false;
            --pdwCurToken;
            break;

        default:
            while ( *pdwCurToken & ( 1L << 31 ) )
            {
                ++pdwCurToken;
            }
        }
    }

    return pdwCurToken;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_BREAKC( const CInstr& instr )
{
    // <lt|eq|ge|ne> s0, src0, src1
    // breakc_nz s0.x

    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const D3DSHADER_COMPARISON compare = D3DSI_GETCOMPARISON( instr.GetToken() );

    this->Compare( compare,
                   CTempOperandDst( SREG_TMP0 ),
                   src0,
                   src1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_BREAKC,
                      CTempOperand1( SREG_TMP0, D3D10_SB_4_COMPONENT_X),
                      D3D10_SB_INSTRUCTION_TEST_NONZERO ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_BREAKP( const CInstr& instr )
{
    // break_[z|nz] src0.c[0]

    const COperandBase src0 = this->EmitSrcOperand( instr, 0, 0, 1 );

    // If D3DSPSM_NOT is set, then flip the order of dwSrcToken
    const DWORD dwSrcToken = instr.GetSrcToken( 0 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_BREAKC,
                      CSingleComponent(src0, (D3D10_SB_4_COMPONENT_NAME)src0.SwizzleComponent(0)),
                      ( dwSrcToken & D3DSPSM_NOT ) ?
                        D3D10_SB_INSTRUCTION_TEST_ZERO :
                        D3D10_SB_INSTRUCTION_TEST_NONZERO ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXKILL( const CInstr& instr )
{
    // lt s0.mask, src0.xyzw, vec4(0.0f)
    // ? { or s0.i, s0.i, s0.j }
    // discard_nz s0.i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );

    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    const UINT writeMask = ( m_version >= D3DPS_VERSION(2,0) ) ?
                                __getWriteMask( instr.GetSrcToken( 0 ) ) : __WRITEMASK_XYZ;

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_LT,
                      CTempOperandDst( SREG_TMP0, writeMask ),
                      CSwizzle( src0, __SWIZZLE_ALL ),
                      COperand( 0.0f ) ) );

    D3D10_SB_4_COMPONENT_NAME component = D3D10_SB_4_COMPONENT_X;
    UINT selectMask = 0;

    if ( writeMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_W )
    {
        selectMask |= ( 1 << 3 );
        component = D3D10_SB_4_COMPONENT_W;
    }

    if ( writeMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Z )
    {
        selectMask |= ( 1 << 2 );
        component = D3D10_SB_4_COMPONENT_Z;
    }

    if ( writeMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Y )
    {
        selectMask |= ( 1 << 1 );
        component = D3D10_SB_4_COMPONENT_Y;
    }

    if ( writeMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_X )
    {
        selectMask |= ( 1 << 0 );
        component = D3D10_SB_4_COMPONENT_X;
    }

    const UINT compWriteMask = ( 1 << component ) << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;

    for ( int i = 0 ; i < 4; ++i )
    {
        if ( i != component &&
             selectMask & ( 1 << i ) )
        {
            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_OR,
                              CTempOperandDst( SREG_TMP0, compWriteMask ),
                              CTempOperand4( SREG_TMP0, __SWIZZLE1(component) ),
                              CTempOperand4( SREG_TMP0, __SWIZZLE1(i) ) ) );
        }
    }

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DISCARD,
                      CTempOperand1( SREG_TMP0, component ),
                      D3D10_SB_INSTRUCTION_TEST_NONZERO ) );
}


void CContext::EmitSampleCmp(const CInstr& instr, const COperandBase dest, const DWORD dwStage, const COperandBase src0)
{
    SHADER_CONV_ASSERT(__IS_PS(m_version));

    COperandBase depth = COperand4(src0.m_Type, src0.RegIndex(), D3D10_SB_4_COMPONENT_Z);
    if (src0.m_Type == D3D10_SB_OPERAND_TYPE_INPUT)
    {
        if ((m_inputRegs.v[src0.RegIndex()].WriteMask() & D3D10_SB_COMPONENT_MASK_Z) == 0)
        {
            depth = COperand(0.0f);
        }
    }

    this->EmitDstInstruction(instr.GetModifiers(),
        D3D10_SB_OPCODE_SAMPLE_C,
        dwStage,
        dest,//dest
        src0,//texcoord
        CResOperand4(dwStage, __SWIZZLE_X),//texture
        COperand(D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage),//sampler
        depth);
}
///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEX( const CInstr& instr )
{
    SHADER_CONV_ASSERT( __IS_PS( m_version ) );

    const COperandBase dest = instr.CreateDstOperand();

    if ( m_version < D3DPS_VERSION(2,0) )
    {
        // tex instruction
        // sample dest, src0, t#i, s#i

        const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );

        const COperandBase src0 = this->EmitSrcOperand(instr, 0, 0);

        // if reading from a specialized depth texture implicitly use a comparison sample for 'Hardware Shadow Maps'
        if (TextureUsesHardwareShadowMapping(dwStage))
        {
            EmitSampleCmp(instr, dest, dwStage, src0);
        }
        else
        {
            this->EmitDstInstruction( instr.GetModifiers(),
                                      D3D10_SB_OPCODE_SAMPLE,
                                      dwStage,
                                      dest,
                                      src0,
                                      CResOperand4( dwStage ),
                                      COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
        }
    }
    else
    {
        const DWORD dwInstr = instr.GetToken();
        const DWORD dwSrcToken1 = instr.GetSrcToken( 1 );
        const DWORD dwStage = D3DSI_GETREGNUM( dwSrcToken1 );
        CONST UINT maxComponentsNeeded = __getComponentsNeeded((TEXTURETYPE)m_rasterStates.PSSamplers[dwStage].TextureType, (m_rasterStates.HardwareShadowMappingRequiredPS & (1 << dwStage)) != 0);
        
        const COperandBase src0 = this->EmitSrcOperand(instr, 0);

        // Use the maskedSrc0 for the actual texture coordinate, masks off unused channels to ensure that the shader validator
        // doesn't complain about using an undeclared color channel
        const COperandBase maskedSrc0 = this->EmitSrcOperand(instr, 0, 0, maxComponentsNeeded);
        const UINT resSwizzle = __getSwizzle( dwSrcToken1, 4 );

        if ( dwInstr & D3DSI_TEXLD_PROJECT )
        {
            // texldp instruction: divide texture coordinates by 4th element
            // div    s0,  src0, src0.w
            // sample dest, s0, t#i, s#i

            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_DIV,
                              CTempOperandDst( SREG_TMP0 ),
                              maskedSrc0,
                              CSwizzle( src0, __SWIZZLE_W ) ) );

            if (TextureUsesHardwareShadowMapping(dwStage))
            {
                EmitSampleCmp(instr, dest, dwStage, CTempOperand4(SREG_TMP0));
            }
            else
            {
                this->EmitDstInstruction( instr.GetModifiers(),
                                          D3D10_SB_OPCODE_SAMPLE,
                                          dwStage,
                                          dest,
                                          CTempOperand4( SREG_TMP0 ),
                                          CResOperand4( dwStage, resSwizzle ),
                                          COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
            }
        }
        else
        if ( dwInstr & D3DSI_TEXLD_BIAS )
        {
            if (TextureUsesHardwareShadowMapping(dwStage))
            {
                EmitSampleCmp(instr, dest, dwStage, maskedSrc0);
            }
            else 
            {
                // texldb instruction:/ bias lod with 4th element of texture coordinates
                // sample_b dest, src0, t#i, s#i, src0.w

                this->EmitDstInstruction( instr.GetModifiers(),
                                          D3D10_SB_OPCODE_SAMPLE_B,
                                          dwStage,
                                          dest,
                                          maskedSrc0,
                                          CResOperand4( dwStage, resSwizzle ),
                                          COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ),
                                          CSingleComponent( src0, D3D10_SB_4_COMPONENT_W ) );
            }
        }
        else
        {
            // texld instruction

            if (TextureUsesHardwareShadowMapping(dwStage))
            {
                EmitSampleCmp(instr, dest, dwStage, maskedSrc0);
            }
            else
            {
                // sample dest, src0, t#i, s#i

                this->EmitDstInstruction( instr.GetModifiers(),
                                          D3D10_SB_OPCODE_SAMPLE,
                                          dwStage,
                                          dest,
                                          maskedSrc0,
                                          CResOperand4( dwStage, resSwizzle ),
                                          COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
            };
        }
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXLDD( const CInstr& instr )
{
    // sample_d dest, src0, t#i, s#i, src2, src3

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src2 = this->EmitSrcOperand( instr, 2 );
    const COperandBase src3 = this->EmitSrcOperand( instr, 3 );

    const DWORD dwSrcToken1 = instr.GetSrcToken( 1 );
    const DWORD dwStage = D3DSI_GETREGNUM( dwSrcToken1 );
    const UINT resSwizzle = __getSwizzle( dwSrcToken1, 4 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE_D,
                              dwStage,
                              dest,
                              src0,
                              CResOperand4( dwStage, resSwizzle ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ),
                              src2,
                              src3 );
}

bool CContext::TextureUsesHardwareShadowMapping(DWORD stage) const
{
    bool bUsesHardwareShadowMapping = false;
    if (__IS_PS(m_version))
    {
        bUsesHardwareShadowMapping = m_rasterStates.HardwareShadowMappingRequiredPS & (1 << stage);
    }
    else if (__IS_VS(m_version))
    {
        if (m_rasterStates.HardwareShadowMappingRequiredVS & (1 << stage))
        {
            // Not expecting to need support for Hardware Shadow Mapping for VS
            SHADER_CONV_ASSERT(false);
        }
    }
    return bUsesHardwareShadowMapping;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXLDL( const CInstr& instr )
{
    // sample_l dest, src0, t#i, s#i, src0.w

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    const DWORD dwSrcToken1 = instr.GetSrcToken( 1 );
    const DWORD dwStage = D3DSI_GETREGNUM( dwSrcToken1 );
    const UINT resSwizzle = __getSwizzle( dwSrcToken1, 4 );

    if (TextureUsesHardwareShadowMapping(dwStage))
    {
        EmitSampleCmp(instr, dest, dwStage, src0);
    }
    else
    {
        this->EmitDstInstruction(instr.GetModifiers(),
            D3D10_SB_OPCODE_SAMPLE_L,
            dwStage,
            dest,
            src0,
            CResOperand4(dwStage, resSwizzle),
            COperand(D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage),
            CSingleComponent(src0, D3D10_SB_4_COMPONENT_W));
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXREG2AR( const CInstr& instr )
{
    // mov s0, src0.wx
    // sample dest, s0, t#i, s#i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_MOV,
                              CTempOperandDst( SREG_TMP0 ),
                              CSwizzle( src0, __SWIZZLE_WX ) ) );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );
    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXREG2GB( const CInstr& instr )
{
    // mov s0, src0.yz
    // sample dest, s0, t#i, s#i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_MOV,
                              CTempOperandDst( SREG_TMP0 ),
                              CSwizzle( src0,__SWIZZLE_YZ ) ) );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );
    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXREG2RGB( const CInstr& instr )
{
    // mov s0, src0.xyz
    // sample dest, s0, t#i, s#i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );

    m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_MOV,
                              CTempOperandDst( SREG_TMP0 ),
                              CSwizzle( src0, __SWIZZLE_XYZ ) ) );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXM3x2PAD( const CInstr& instr )
{
    // dp3 s0.x, src0, src1

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0,
                      src1 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXM3x2TEX( const CInstr& instr )
{
    // dp3 s0.y, src0, src1
    // sample dest, s0, t#i, s#i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      src0,
                      src1 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXM3x2DEPTH( const CInstr& instr )
{
    // dp3 s0.y, src0, src1
    // ne s0.z, s0.y, vec4(0.0f)
    // movc s0.xy, s0.z, s0.xy, vec4(1.0f)
    // div oDepth, s0.x, s0.y

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( D3DPS_VERSION(1,3) == m_version );

    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      src0,
                      src1 ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_NE,
                      CTempOperandDst( SREG_TMP0,D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Y ),
                      COperand( 0.0f ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOVC,
                      CTempOperandDst( SREG_TMP0,__WRITEMASK_XY ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Z ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_XY ),
                      COperand( 1.0f ) ) );

    Modifiers modifiers = instr.GetModifiers();
    modifiers._SATURATE = 1;

    this->EmitDstInstruction( modifiers,
                              D3D10_SB_OPCODE_DIV,
                              CTempOperandDst( m_outputRegs.oDepth ),
                              CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                              CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXM3x3PAD( const CInstr& instr )
{
    // dp3 s0.[i], src0, src1

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const UINT writeMask = ( m_toggleTEXM3x3PAD ) ? D3D10_SB_OPERAND_4_COMPONENT_MASK_Y :
                                                    D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, writeMask ),
                      src0,
                      src1 ) );

    // Toggle the bit for the next instruction
    m_toggleTEXM3x3PAD ^= 1;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXM3x3( const CInstr& instr )
{
    // dp3 s0.z, src0, src1
    // mov s0.w, vec4(1.0f)
    // mov dest, s0

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version >= D3DPS_VERSION(1,2) &&
                m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      src0,
                      src1 ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV,
                      CTempOperandDst( SREG_TMP0,D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                      COperand( 1.0f ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXM3x3TEX( const CInstr& instr )
{
    // dp3 s0.z, src0, src1
    // sample dest, s0, t#i, s#i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      src0,
                      src1 ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXM3x3SPEC( const CInstr& instr )
{
    // RF = 2.0f * N.E * N - E * N.N

    // dp3 s0.z, src0, src1
    // dp3 s1.w, s0, s0
    // mul s1, src2, s1.w
    // dp3 s0.w, s0, src2
    // mul s0, s0, s0.w
    // mad s0, vec4(2.0f), s0, -s1
    // sample dest, s0, t#i, s#i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );
    const COperandBase src2 = this->EmitSrcOperand( instr, 2 );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      src0,
                      src1 ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP1, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                      CTempOperand4( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP0 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP1 ),
                      src2,
                      CTempOperand4( SREG_TMP1, __SWIZZLE_W ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                      CTempOperand4( SREG_TMP0 ),
                      src2 ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_W ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP0 ),
                      COperand( 2.0f ),
                      CTempOperand4( SREG_TMP0 ),
                      CNegate( CTempOperand4( SREG_TMP1 ) ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

DWORD
GetTextureRegisterIndex([[maybe_unused]] UINT version, DWORD dwRegNum)
{
    SHADER_CONV_ASSERT(__IS_PS(version));
    DWORD dwRegIndex = dwRegNum + VSOREG_TexCoord0;
    return dwRegIndex;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXM3x3VSPEC( const CInstr& instr )
{
    // RF = 2.0f * N.E * N - E * N.N

    // dp3 s0.z, src0, src1
    // mov s1.x, TC[stage-2].w
    // mov s1.y, TC[stage-1].w
    // mov s1.z, TC[stage].w
    // dp3 s0.w, s0, s1
    // dp3 s1.w, s0, s0
    // mul s1, s1, s1.w
    // mul s0, s0, s0.w
    // mad s0, vec4(2.0f), s0, -s1
    // sample dest, s0, t#i, s#i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );
    SHADER_CONV_ASSERT( dwStage >= 2 );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      src0,
                      src1 ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction(D3D10_SB_OPCODE_MOV,
                     CTempOperandDst(SREG_TMP1, D3D10_SB_OPERAND_4_COMPONENT_MASK_X),
                     this->EmitInputOperand(GetTextureRegisterIndex(m_version, dwStage-2), __SWIZZLE_W)));

    m_pShaderAsm->EmitInstruction(
        CInstruction(D3D10_SB_OPCODE_MOV,
                     CTempOperandDst(SREG_TMP1, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y),
                     this->EmitInputOperand(GetTextureRegisterIndex(m_version, dwStage-1), __SWIZZLE_W)));

    m_pShaderAsm->EmitInstruction(
        CInstruction(D3D10_SB_OPCODE_MOV,
                     CTempOperandDst(SREG_TMP1, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z),
                     this->EmitInputOperand(GetTextureRegisterIndex(m_version, dwStage), __SWIZZLE_W)));

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                      CTempOperand4( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP1 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP1, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                      CTempOperand4( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP0 ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP1 ),
                      CTempOperand4( SREG_TMP1 ),
                      CTempOperand4( SREG_TMP1, __SWIZZLE_W ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_W ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP0 ),
                      COperand( 2.0f ),
                      CTempOperand4( SREG_TMP0 ),
                      CNegate( CTempOperand4( SREG_TMP1 ) ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXCOORD( const CInstr& instr )
{
    // mov[_sat] dest, src0

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version < D3DPS_VERSION(2,0) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );    

    // Retrieve the input write mask
    UINT writeMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL;
    {
        const UINT regIndex = src0.RegIndex();    
        const VSOutputDecls& inputDecls = reinterpret_cast<CPSContext*>(this)->GetInputDecls();
        for (UINT i = 0, n = inputDecls.GetSize(); i < n; ++i)
        {
            if ((inputDecls[i].WriteMask != 0)
             && (regIndex == inputDecls[i].RegIndex))
            {
                writeMask = inputDecls[i].WriteMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;
                break;
            }
        }
    }

    Modifiers modifiers = instr.GetModifiers();

    if ( m_version <= D3DPS_VERSION(1,3) )
    {
        modifiers._SATURATE = 1;
    }

    // Move source texture coordinate value to destination register
    this->EmitDstInstruction( modifiers,
                              D3D10_SB_OPCODE_MOV,
                              CWriteMask( dest, writeMask ),
                              src0 );

    //
    // Set the default value for missing components
    //

    switch ( writeMask )
    {
    case D3D10_SB_OPERAND_4_COMPONENT_MASK_X:
        writeMask = __WRITEMASK_YZW;
        break;

    case D3D10_SB_OPERAND_4_COMPONENT_MASK_X |
         D3D10_SB_OPERAND_4_COMPONENT_MASK_Y :
        writeMask = __WRITEMASK_ZW;
        break;

    case D3D10_SB_OPERAND_4_COMPONENT_MASK_X |
         D3D10_SB_OPERAND_4_COMPONENT_MASK_Y |
         D3D10_SB_OPERAND_4_COMPONENT_MASK_Z:
        writeMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_W;
        break;
    }

    if ( writeMask != D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL)
    {
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          CWriteMask( dest, writeMask ),
                          COperand( 0.0f, 0.0f, 0.0f, 1.0f ) ) );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXDEPTH( const CInstr& instr )
{
    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( D3DPS_VERSION(1,4) == m_version );

    // ne s0.z, dst.y, vec4(0.0f)
    // movc s0.xy, s0.z, dst.xy, vec4(1.0f)
    // div oDepth, s0.x, s0.y

    const DWORD dwRegNum = D3DSI_GETREGNUM( instr.GetDstToken() );

    SHADER_CONV_ASSERT( dwRegNum < MAX_TEMP_REGS );
    const DWORD dwRegIndex = m_inputRegs.r[dwRegNum];

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_NE,
                      CTempOperandDst( SREG_TMP0,D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                      CTempOperand4( dwRegIndex, __SWIZZLE_Y ),
                      COperand( 0.0f ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOVC,
                      CTempOperandDst( SREG_TMP0, __WRITEMASK_XY ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Z ),
                      CTempOperand4( dwRegIndex, __SWIZZLE_XY ),
                      COperand( 1.0f ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_DIV,
                              CTempOperandDst( m_outputRegs.oDepth ),
                              CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                              CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXDP3( const CInstr& instr )
{
    // dp3 dest, src0, src1

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version >= D3DPS_VERSION(1,2) &&
                m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_DP3,
                              dest,
                              src0,
                              src1 );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXDP3TEX( const CInstr& instr )
{
    // dp3 s0.x, src0, src1
    // mov s0.yz vec4(0.0f)
    // sample dest, s0, t#i, s#i

    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version >= D3DPS_VERSION(1,2) &&
                m_version <= D3DPS_VERSION(1,3) );

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_DP3,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      src0,
                      src1 ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV,
                      CTempOperandDst( SREG_TMP0, __WRITEMASK_YZ ),
                      COperand( 0.0f ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_BEM( const CInstr& instr )
{
    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( D3DPS_VERSION(1,4) == m_version );

    // mul s0.x, cb3[i].x, src1.x
    // mad s0.x, cb3[i].z, src1.y, s0.x
    // add s0.x, s0.x, src0.x
    // mul s0.y, cb3[i].y, src1.x
    // mad s0.y, cb3[i].w, src1.y, s0.y
    // add s0.y, s0.y, src0.y
    // mov dest, s0

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const DWORD dwDstReg = D3DSI_GETREGNUM( instr.GetDstToken() );
    const UINT i = PSCBExtension2::BUMPENVMAT0 + dwDstReg;

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_X ),
                      CSwizzle( src1, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_Z ),
                      CSwizzle( src1, __SWIZZLE_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                      CSwizzle( src0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_Y ),
                      CSwizzle( src1, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_W ),
                      CSwizzle( src1, __SWIZZLE_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Y ),
                      CSwizzle( src0, __SWIZZLE_Y ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MOV,
                              dest,
                              CTempOperand4( SREG_TMP0 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXBEM( const CInstr& instr )
{
    SHADER_CONV_ASSERT( __IS_PS( m_version ) );
    SHADER_CONV_ASSERT( m_version <= D3DPS_VERSION(1,3) );

    // mul s0.x, c3[i].x, src0.x
    // mad s0.x, c3[i].y, src0.y, s0.x
    // add s0.x, s0.x, TC[i].x
    // mul s0.y, c3[i].z, src0.x
    // mad s0.y, c3[i].w, src0.y, s0.y
    // add s0.y, s0.y, TC[i].y
    // sample dest, s0, t#i, s#i

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );
    const UINT i = PSCBExtension2::BUMPENVMAT0 + dwStage;

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_X ),
                      CSwizzle( src1, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_Y ),
                      CSwizzle( src1, __SWIZZLE_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                      CSwizzle( src0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_Z ),
                      CSwizzle( src1, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_W ),
                      CSwizzle( src1, __SWIZZLE_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Y ),
                      CSwizzle( src0, __SWIZZLE_Y ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_SAMPLE,
                              dwStage,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CResOperand4( dwStage ),
                              COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_TEXBEML( const CInstr& instr )
{
    // mul s0.x, c3[i].x, src0.x
    // mad s0.x, c3[i].y, src0.y, s0.x
    // add s0.x, s0.x, TC[i].x
    // mul s0.y, c3[i].z, src0.x
    // mad s0.y, c3[i].w, src0.y, s0.y
    // add s0.y, s0.y, TC[i].y
    // sample s0, s0, t#i, s#i
    // mad s1.x, c3[j].x, src0.z, c3[j].y
    // mul dest, s0, s1.x

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const DWORD dwStage = D3DSI_GETREGNUM( instr.GetDstToken() );
    const UINT i = PSCBExtension2::BUMPENVMAT0 + dwStage;
    const UINT j = PSCBExtension2::BUMPENVL0 + dwStage;

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_X ),
                      CSwizzle( src1, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_Y ),
                      CSwizzle( src1, __SWIZZLE_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                      CSwizzle( src0, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MUL,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_Z ),
                      CSwizzle( src1, __SWIZZLE_X ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CCBOperand2D( CB_PS_EXT2, i, __SWIZZLE_W ),
                      CSwizzle( src1, __SWIZZLE_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Y ) ) );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_ADD,
                      CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                      CTempOperand4( SREG_TMP0, __SWIZZLE_Y ),
                      CSwizzle( src0, __SWIZZLE_Y ) ) );

    COperandBase resOperand = CResOperand4( dwStage );

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_SAMPLE,
                      CTempOperandDst( SREG_TMP0 ),
                      CTempOperand4( SREG_TMP0 ),
                      resOperand,
                      COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwStage ) ) );

    UINT samplerSwizzle = (m_rasterStates.SamplerSwizzleMask >> (dwStage * SAMPLER_SWIZZLE_BITS)) & SAMPLER_SWIZZLE_MASK;

    if (samplerSwizzle)
    {
        this->EmitSamplerSwizzle(CTempOperandDst(SREG_TMP0), CTempOperand4(SREG_TMP0), (SAMPLER_SWIZZLE)samplerSwizzle);
    }

    if (m_rasterStates.ColorKeyEnable
     || m_rasterStates.ColorKeyBlendEnable)
    {
        this->EmitColorKeyModifier(dwStage, CTempOperandDst(SREG_TMP0), CTempOperand4(SREG_TMP0));
    }

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MAD,
                      CTempOperandDst( SREG_TMP1, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                      CCBOperand2D( CB_PS_EXT2, j, __SWIZZLE_X ),
                      CSwizzle( src0, __SWIZZLE_Z ),
                      CCBOperand2D( CB_PS_EXT2, j, __SWIZZLE_Y ) ) );

    this->EmitDstInstruction( instr.GetModifiers(),
                              D3D10_SB_OPCODE_MUL,
                              dest,
                              CTempOperand4( SREG_TMP0 ),
                              CTempOperand4( SREG_TMP1, __SWIZZLE_X ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Translate_SETP( const CInstr& instr )
{
    // <comp> dest, src0, src1

    const COperandBase dest = instr.CreateDstOperand();
    const COperandBase src0 = this->EmitSrcOperand( instr, 0 );
    const COperandBase src1 = this->EmitSrcOperand( instr, 1 );

    const D3DSHADER_COMPARISON compare = D3DSI_GETCOMPARISON( instr.GetToken() );

    this->Compare( compare, dest, src0, src1 );
}

//////////////////////////////////////////////////////////////////////////////

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::Compare( D3DSHADER_COMPARISON comp,
                   const COperandBase& dstOperand,
                   const COperandBase& srcOperand0,
                   const COperandBase& srcOperand1 ) const
{
    D3D10_SB_OPCODE_TYPE opcode;
    bool bSwapOperands = false;

    switch ( comp )
    {
    default:
        NO_DEFAULT;

    case D3DSPC_GT:
        opcode = D3D10_SB_OPCODE_LT;
        bSwapOperands = true;
        break;

    case D3DSPC_EQ:
        opcode = D3D10_SB_OPCODE_EQ;
        break;

    case D3DSPC_GE:
        opcode = D3D10_SB_OPCODE_GE;
        break;

    case D3DSPC_LT:
        opcode = D3D10_SB_OPCODE_LT;
        break;

    case D3DSPC_NE:
        opcode = D3D10_SB_OPCODE_NE;
        break;

    case D3DSPC_LE:
        opcode = D3D10_SB_OPCODE_GE;
        bSwapOperands = true;
        break;
    }

    m_pShaderAsm->EmitInstruction(
        CInstruction( opcode,
                      dstOperand,
                      bSwapOperands ? srcOperand1 : srcOperand0,
                      bSwapOperands ? srcOperand0 : srcOperand1 ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::EmitShiftModifier( UINT shiftMask,
                             bool bSaturate,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand )
{
    float fValue;

    switch ( shiftMask )
    {
    default:
        NO_DEFAULT;

    case 0x1:   // x2
        fValue = 2.0f;
        break;

    case 0x2:   // x4
        fValue = 4.0f;
        break;

    case 0x3:   // x8
        fValue = 8.0f;
        break;

    case 0xF:   // d2
        fValue = 0.5f;
        break;

    case 0xE:   // d4
        fValue = 0.25f;
        break;

    case 0xD:   // d8
        fValue = 0.125f;
        break;
    }

     m_pShaderAsm->EmitInstruction(
            CInstructionEx( D3D10_SB_OPCODE_MUL,
                            bSaturate,
                            dstOperand,
                            srcOperand,
                            COperand( fValue ) ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::EmitSamplerSwizzle( const COperandBase& dstOperand,
                              const COperandBase& srcOperand,
                              SAMPLER_SWIZZLE swizzle )
{
    // Converts data sampled from a D3D11 texture to D3D9 format:
    //
    //  mov dest, src.rrra  ; for luminance formats
    //  mov dest, src.raaa  ; for single channel formats
    //  mov dest, src.rgaa  ; for two channel formats

    COperandBase swizzledSrc(srcOperand);

    switch (swizzle)
    {
        case SAMPLER_SWIZZLE_RRRA:
            // Luminance texture format: swizzle (r, 0, 0, 1) -> (r, r, r, 1)
            // This is because shim uses D3D11 R8/R16 formats to emulate D3D9 L8/L16 types.
            swizzledSrc.m_Swizzle[1] = swizzledSrc.m_Swizzle[0];
            swizzledSrc.m_Swizzle[2] = swizzledSrc.m_Swizzle[0];
            break;
            
        case SAMPLER_SWIZZLE_RAAA:
            // Single channel texture format: swizzle (r, 0, 0, 1) -> (r, 1, 1, 1)
            // This is because D3D11 DDI returns 0 in unused g/b channels, but D3D9 expects 1.
            // We have no good way to generate a 1 value in patched shader code, but can simply
            // copy this over from the alpha component, which is always 1 for these formats.
            swizzledSrc.m_Swizzle[1] = swizzledSrc.m_Swizzle[3];
            swizzledSrc.m_Swizzle[2] = swizzledSrc.m_Swizzle[3];
            break;
            
        case SAMPLER_SWIZZLE_RGAA:
            // Two channel texture format: swizzle (r, g, 0, 1) -> (r, g, 1, 1)
            swizzledSrc.m_Swizzle[2] = swizzledSrc.m_Swizzle[3];
            break;
        
        default:
            SHADER_CONV_ASSERT(false);
    }

    m_pShaderAsm->EmitInstruction(
        CInstruction( D3D10_SB_OPCODE_MOV, dstOperand, swizzledSrc ) );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::EmitColorKeyModifier( UINT stage,
                                const COperandBase& dstOperand,
                                const COperandBase& srcOperand )
{
    SHADER_CONV_ASSERT(m_rasterStates.ColorKeyEnable || m_rasterStates.ColorKeyBlendEnable);

    if (0 == (m_rasterStates.ColorKeyTSSDisable & (1 << stage)))
    {
        // Apply colorkey

        // eq m0, src, colorkey[i]
        // and m0.x, m0.x, m0.y
        // and m0.x, m0.x, m0.z
        // and m0.x, m0.x, m0.w
        // (1) discard_nz s0.i
        // (2) movc dst, m0, zero, src

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_EQ,
                            CTempOperandDst( SREG_MOD0 ),
                            srcOperand,
                            CCBOperand2D( CB_PS_EXT3, PSCBExtension3::COLORKEY0 + stage ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_AND,
                            CTempOperandDst( SREG_MOD0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                            CTempOperand4( SREG_MOD0, __SWIZZLE_X ),
                            CTempOperand4( SREG_MOD0, __SWIZZLE_Y ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_AND,
                            CTempOperandDst( SREG_MOD0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                            CTempOperand4( SREG_MOD0, __SWIZZLE_X ),
                            CTempOperand4( SREG_MOD0, __SWIZZLE_Z ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_AND,
                            CTempOperandDst( SREG_MOD0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                            CTempOperand4( SREG_MOD0, __SWIZZLE_X ),
                            CTempOperand4( SREG_MOD0, __SWIZZLE_W ) ) );

        if (m_rasterStates.ColorKeyEnable)
        {
            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_DISCARD,
                                CTempOperand1( SREG_MOD0, D3D10_SB_4_COMPONENT_X),
                                D3D10_SB_INSTRUCTION_TEST_NONZERO ) );

            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_MOV, dstOperand, srcOperand ) );
        }
        else
        {
            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_MOVC,
                                dstOperand,
                                CTempOperand4( SREG_MOD0, __SWIZZLE_X ),
                                COperand( 0.0f ),
                                srcOperand ) );
        }
    }


    else
    {
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV, dstOperand, srcOperand ) );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CContext::EmitDstInstruction( const Modifiers& modifiers,
                              D3D10_SB_OPCODE_TYPE opCode,
                              UINT stage,
                              UINT NumSrcOperands,
                              const COperandBase* pDst,
                              ...)
{
    CInstructionEx instruction(opCode);

    va_list args;
    va_start(args, pDst);

    for (UINT i = 0; i < NumSrcOperands; ++i)
    {
        const COperandBase* const pOperand = va_arg(args, const COperandBase*);
        instruction.SetOperand(i + 1, *pOperand);
    }

    va_end(args);

    const bool bSaturate = modifiers._SATURATE ? true : false;

    const bool isSampleInstruction = (opCode == D3D10_SB_OPCODE_SAMPLE) ||
                                     (opCode == D3D10_SB_OPCODE_SAMPLE_B) ||
                                     (opCode == D3D10_SB_OPCODE_SAMPLE_D) ||
                                     (opCode == D3D10_SB_OPCODE_SAMPLE_L);

    const UINT samplerSwizzle = (m_rasterStates.SamplerSwizzleMask >> (stage * SAMPLER_SWIZZLE_BITS)) & SAMPLER_SWIZZLE_MASK;

    const bool needsSamplerSwizzle = isSampleInstruction && (samplerSwizzle != 0);

    if (modifiers._SHIFT
     || needsSamplerSwizzle
     || m_rasterStates.ColorKeyEnable
     || m_rasterStates.ColorKeyBlendEnable)
    {
        CTempOperandDst dstTmp0(SREG_TMP0);
        CTempOperand4   srcTmp0(SREG_TMP0);

        instruction.SetOperand(0, dstTmp0);
        m_pShaderAsm->EmitInstruction(instruction);

        if (needsSamplerSwizzle)
        {
            this->EmitSamplerSwizzle(dstTmp0, srcTmp0, (SAMPLER_SWIZZLE)samplerSwizzle);
        }

        if (m_rasterStates.ColorKeyEnable
         || m_rasterStates.ColorKeyBlendEnable)
        {
            this->EmitColorKeyModifier(stage, dstTmp0, srcTmp0);
        }

        if (modifiers._SHIFT)
        {
            this->EmitShiftModifier(modifiers._SHIFT, bSaturate, *pDst, srcTmp0);
        }
        else
        {
            m_pShaderAsm->EmitInstruction(
                CInstructionEx(D3D10_SB_OPCODE_MOV, bSaturate, *pDst, srcTmp0));
        }
    }
    else
    {
        instruction.SetOperand(0, *pDst);
        instruction.SetSaturate(bSaturate);
        m_pShaderAsm->EmitInstruction(instruction);
    }
}


UINT MaskSwizzleWithWriteMask(UINT swizzle, UINT writeMask)
{
    UINT newSwizzle = 0;
    for (UINT i = 0; i < 4; ++i)
    {
        const UINT comp = __UNSWIZZLEN(i, swizzle);
        if (D3D10_SB_OPERAND_4_COMPONENT_MASK(comp) & writeMask)
        {
            newSwizzle |= __SWIZZLEN(i, comp);
        }
    }
    return newSwizzle;
}


DWORD
GetInputRegisterIndex(UINT version, DWORD dwRegNum)
{
    DWORD dwRegIndex = dwRegNum;
    if (__IS_PS(version) && version < D3DPS_VERSION(3, 0))
    {
        dwRegIndex += VSOREG_Color0;
    }

    return dwRegIndex;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
COperandBase
CContext::EmitSrcOperand( const CInstr& instr,
                          DWORD dwIndex,
                          DWORD dwOffset,
                          DWORD dwSwizzleCount)
{
    const DWORD dwToken    = instr.GetSrcToken( dwIndex );
    const DWORD dwModifier = ( dwToken & D3DSP_SRCMOD_MASK );
    const DWORD dwRegType  = D3DSI_GETREGTYPE_RESOLVING_CONSTANTS( dwToken );

    if (dwRegType == D3DSPR_TEXTURE)
    {
        [[maybe_unused]] const DWORD dwRegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS(dwToken) + dwOffset;
        SHADER_CONV_ASSERT(dwRegNum < MAX_PS_TEXCOORD_REGS);
    }

    UINT swizzle     = __getSwizzle( dwToken, dwSwizzleCount);

    COperandBase srcOperand = instr.CreateSrcOperand( dwToken, dwOffset );

    if ( D3DSPR_CONSTBOOL == dwRegType )
    {
        return srcOperand;
    }

    if ( D3D10_SB_OPERAND_TYPE_IMMEDIATE32 == srcOperand.OperandType() )
    {
        return this->EmitImmOperand( srcOperand, dwModifier, swizzle );
    }

    //
    // Apply register addressing
    //

    if ( D3DSI_GETADDRESSMODE( dwToken ) & D3DSHADER_ADDRMODE_RELATIVE )
    {
        const DWORD dwAddrToken = instr.GetSrcAddress( dwIndex );
        instr.ApplyAddressing( srcOperand, dwAddrToken, ( D3DSPR_INPUT == dwRegType ) ? 0 : 1 );
    }

    // Force a [-1,1] clamp after applying modifier (for PS 1.x constants only)
    // This overrides the the standard [-PixelShader1xMaxValue,PixelShader1xMaxValue] clamp.

    if ( __IS_PS( m_version ) &&
         m_version < D3DPS_VERSION(2,0) &&
         D3DSPR_CONST == dwRegType )
    {
        // max s2, src[i], vec4(-1.0f)
        // min m[i], s2, vec4(1.0f)

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MAX,
                          CTempOperandDst( SREG_TMP2 ),
                          srcOperand,
                          COperand( -1.0f ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MIN,
                          CTempOperandDst( SREG_MOD0 + dwIndex ),
                          CTempOperand4( SREG_TMP2 ),
                          COperand( 1.0f ) ) );

        srcOperand = CTempOperand4( SREG_MOD0 + dwIndex );
    }

    switch ( dwModifier )
    {
    case D3DSPSM_DZ:
    case D3DSPSM_DW:
        // mov  s2, src
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV, CTempOperandDst( SREG_TMP2 ), srcOperand ) );

        switch ( dwModifier )
        {
        case D3DSPSM_DZ:
            // div  m[i].xy, s2.xy, s2.z
            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_DIV,
                              CTempOperandDst( SREG_MOD0 + dwIndex, __WRITEMASK_XY ),
                              CTempOperand4( SREG_TMP2, __SWIZZLE_XY ),
                              CTempOperand4( SREG_TMP2, __SWIZZLE_Z ) ) );
            break;

        case D3DSPSM_DW:
            // div  m[i].xy, s2.xy, s2.w
            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_DIV,
                              CTempOperandDst( SREG_MOD0 + dwIndex, __WRITEMASK_XYZ ),
                              CTempOperand4( SREG_TMP2, __SWIZZLE_XYZ ),
                              CTempOperand4( SREG_TMP2, __SWIZZLE_W ) ) );
            break;
        }
        srcOperand = CTempOperand4( SREG_MOD0 + dwIndex );
        break;

    case D3DSPSM_BIASNEG:
    case D3DSPSM_BIAS:
        // add  m[i], src, vec4(-0.5f)
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ADD,
                          CTempOperandDst( SREG_MOD0 + dwIndex ),
                          srcOperand,
                          COperand( -0.5f ) ) );
        srcOperand = CTempOperand4( SREG_MOD0 + dwIndex );
        break;

    case D3DSPSM_SIGNNEG:
    case D3DSPSM_SIGN:
        // mad m[i], src, vec4(2.0f), vec4(-1.0f)
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MAD,
                          CTempOperandDst( SREG_MOD0 + dwIndex ),
                          srcOperand,
                          COperand( 2.0f ),
                          COperand( -1.0f ) ) );
        srcOperand = CTempOperand4( SREG_MOD0 + dwIndex );
        break;

    case D3DSPSM_COMP:
        // add  m[i], vec4(1.0f), -src
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ADD,
                          CTempOperandDst( SREG_MOD0 + dwIndex ),
                          COperand( 1.0f ),
                          CNegate( srcOperand ) ) );
        srcOperand = CTempOperand4( SREG_MOD0 + dwIndex );
        break;

    case D3DSPSM_X2NEG:
    case D3DSPSM_X2:
        // mul  m[i], src[i], vec4(2.0f)
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MUL,
                          CTempOperandDst( SREG_MOD0 + dwIndex ),
                          srcOperand,
                          COperand( 2.0f ) ) );
        srcOperand = CTempOperand4( SREG_MOD0 + dwIndex );
        break;
    }

    //
    // Apply D3D10 modifier if available
    //

    switch ( dwModifier )
    {
    case D3DSPSM_NEG:
    case D3DSPSM_BIASNEG:
    case D3DSPSM_SIGNNEG:
    case D3DSPSM_X2NEG:
        srcOperand.SetModifier( D3D10_SB_OPERAND_MODIFIER_NEG );
        break;

    case D3DSPSM_ABSNEG:
        srcOperand.SetModifier( D3D10_SB_OPERAND_MODIFIER_ABSNEG );
        break;

    case D3DSPSM_ABS:
        srcOperand.SetModifier( D3D10_SB_OPERAND_MODIFIER_ABS );
        break;

    default:
        srcOperand.SetModifier( D3D10_SB_OPERAND_MODIFIER_NONE );
        break;
    }

    // Apply the swizzle mask
    if (dwRegType == D3DSPR_INPUT || dwRegType == D3DSPR_TEXTURE)
    {
        const DWORD dwRegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS(dwToken) + dwOffset;
        DWORD dwRegIndex;
        if (dwRegType == D3DSPR_INPUT)
        {
            dwRegIndex = GetInputRegisterIndex(m_version, dwRegNum);
        }
        else
        {
            SHADER_CONV_ASSERT(dwRegType == D3DSPR_TEXTURE);
            dwRegIndex = GetTextureRegisterIndex(m_version, dwRegNum);
        }

        if (m_inputRegs.v[dwRegIndex].Reg() != INVALID_INDEX)
        {
            if (swizzle)
            {
                swizzle = MaskSwizzleWithWriteMask(swizzle, m_inputRegs.v[dwRegIndex].WriteMask() << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT);
            }
            else
            {
                swizzle = __swizzleFromWriteMask(m_inputRegs.v[dwRegIndex].WriteMask() << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT);
            }
        }

    }

    if ( swizzle )
    {
        SetSwizzle(srcOperand, swizzle);
    }

    return srcOperand;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
COperandBase
CContext::EmitImmOperand( COperandBase& srcOperand, DWORD dwModifier, UINT swizzle )
{
    // Force a [-1,1] clamp after applying modifier (for PS 1.x constants only)
    // This overrides the the standard [-PixelShader1xMaxValue,PixelShader1xMaxValue] clamp.

    if ( __IS_PS( m_version ) &&
         m_version < D3DPS_VERSION(2,0) )
    {
        // max s2, src[i], vec4(-1.0f)
        // min m[i], s2, vec4(1.0f)
        for ( UINT i = 0; i < 4; ++i )
        {
            srcOperand.m_Valuef[i] = max( srcOperand.m_Valuef[i], -1.0f );
            srcOperand.m_Valuef[i] = min( srcOperand.m_Valuef[i], 1.0f );
        }
    }

    switch ( dwModifier )
    {
    case D3DSPSM_DZ:
        // div  m[i].xy, s2.xy, s2.zz
        srcOperand.m_Valuef[0] /= srcOperand.m_Valuef[2];
        srcOperand.m_Valuef[1] /= srcOperand.m_Valuef[2];
        break;

    case D3DSPSM_DW:
        // div  m[i].xy, s2.xy, s2.ww
        srcOperand.m_Valuef[0] /= srcOperand.m_Valuef[3];
        srcOperand.m_Valuef[1] /= srcOperand.m_Valuef[3];
        break;

    case D3DSPSM_BIASNEG:
    case D3DSPSM_BIAS:
        // add  m[i], src, vec4(-0.5f)
        for ( UINT i = 0; i < 4; ++i )
        {
srcOperand.m_Valuef[i] -= 0.5f;
        }
        break;

    case D3DSPSM_SIGNNEG:
    case D3DSPSM_SIGN:
        // mad m[i], src, vec4(2.0f), vec4(-1.0f)
        for (UINT i = 0; i < 4; ++i)
        {
            srcOperand.m_Valuef[i] = 2.0f * srcOperand.m_Valuef[i] - 1.0f;
        }
        break;

    case D3DSPSM_COMP:
        // add  m[i], vec4(1.0f), -src
        for (UINT i = 0; i < 4; ++i)
        {
            srcOperand.m_Valuef[i] = 1.0f - srcOperand.m_Valuef[i];
        }
        break;

    case D3DSPSM_X2NEG:
    case D3DSPSM_X2:
        // mul  m[i], src[i], vec4(2.0f)
        for (UINT i = 0; i < 4; ++i)
        {
            srcOperand.m_Valuef[i] = 2.0f * srcOperand.m_Valuef[i];
        }
        break;

    case D3DSPSM_ABSNEG:
    case D3DSPSM_ABS:
        // mov  m[i], abs(src[i])
        for (UINT i = 0; i < 4; ++i)
        {
            srcOperand.m_Valuef[i] = abs(srcOperand.m_Valuef[i]);
        }
        break;
    }

    //
    // Apply negation if needed
    //

    switch (dwModifier)
    {
    case D3DSPSM_NEG:
    case D3DSPSM_BIASNEG:
    case D3DSPSM_SIGNNEG:
    case D3DSPSM_X2NEG:
    case D3DSPSM_ABSNEG:
        for (UINT i = 0; i < 4; ++i)
        {
            srcOperand.m_Valuef[i] = -srcOperand.m_Valuef[i];
        }
        break;
    }

    if (swizzle)
    {
        // Apply the swizzle mask
        SetSwizzle(srcOperand, swizzle);
    }

    return srcOperand;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
COperandBase
CContext::EmitInputOperand(DWORD dwRegIndex, UINT swizzle) const
{
    const InputRegister &inputRegister = m_inputRegs.v[dwRegIndex];
    switch (inputRegister.GetType())
    {
    case InputRegister::Temp:
        return CTempOperand4(m_inputRegs.v[dwRegIndex].Reg(), swizzle);
    case InputRegister::Input:
        return CInputOperand4(m_inputRegs.v[dwRegIndex].Reg(), swizzle);
    default:
        {
            // Use the default constant value
            SHADER_CONV_ASSERT(inputRegister.GetType() == InputRegister::Undeclared);
            dwRegIndex = GetDefaultInputRegister(dwRegIndex);
            return CTempOperand4(dwRegIndex, swizzle);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
CInstr::CInstr( DWORD dwInstr, const CContext* pContext ) :
    m_version( pContext->m_version ),
    m_inputRegs( pContext->m_inputRegs ),
    m_outputRegs( pContext->m_outputRegs )
{
    SHADER_CONV_ASSERT( pContext );

    if ( D3DSI_COISSUE & dwInstr )
    {
        m_modifiers._COISSUE = 1;
    }

    if ( D3DSHADER_INSTRUCTION_PREDICATED & dwInstr )
    {
        m_modifiers._PREDICATE = 1;
    }

    m_token         = dwInstr;
    m_opCode        = (D3DSHADER_INSTRUCTION_OPCODE_TYPE)( dwInstr & D3DSI_OPCODE_MASK );
    m_length        = __getInstrLength( dwInstr, m_version );
    m_srcTokenCount = 0;
    m_pContext      = pContext;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
const DWORD*
CInstr::SetDstToken( const DWORD* pdwToken )
{
    SHADER_CONV_ASSERT( pdwToken );
    DWORD dwToken = *pdwToken++;
    DWORD dwAddress = 0;

    if ( D3DSI_GETADDRESSMODE( dwToken ) & D3DSHADER_ADDRMODE_RELATIVE )
    {
        if ( ( __IS_VS( m_version ) && m_version >= D3DVS_VERSION(2,0) ) ||
             ( __IS_PS( m_version ) && m_version >= D3DPS_VERSION(3,0) ) )
        {
            dwAddress = *pdwToken++;
        }
    }

    // Get the PS 1.x shift modifiers
    if ( __IS_PS( m_version ) &&
         m_version < D3DPS_VERSION(2,0) )
    {
        m_modifiers._SHIFT = ( dwToken & D3DSP_DSTSHIFT_MASK ) >> D3DSP_DSTSHIFT_SHIFT;
    }

    // Get the saturation modifier
    m_modifiers._SATURATE = ( dwToken & D3DSPDM_SATURATE ) ? 1 : 0;

    // Get the partial precision modifier
    if ( dwToken & D3DSPDM_PARTIALPRECISION )
    {
        m_modifiers._PPRECISION = 1;
    }

    // Get the multi sample centroid modifier
    if ( dwToken & D3DSPDM_MSAMPCENTROID )
    {
        m_modifiers._CENTROID = 1;
    }

    const DWORD dwRegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS( dwToken );
    const DWORD dwRegType = D3DSI_GETREGTYPE_RESOLVING_CONSTANTS( dwToken );

    if ( __IS_PS( m_version ) &&
         m_version < D3DPS_VERSION(2,0) &&
         D3DSPR_TEXTURE == dwRegType )
    {
        switch ( m_opCode )
        {
        case D3DSIO_TEXM3x2PAD:
        case D3DSIO_TEXM3x3PAD:
            // These instructions use a system register to keep their destination value.
            break;

        default:
            // In ps_1_x, texture registers are used as temp registers,
            // we need to change their type to our custom register type
            dwToken &= ~( D3DSP_REGTYPE_MASK | D3DSP_REGTYPE_MASK2 );
            dwToken |= __D3DI_TYPE_TOKEN( __D3DSPR_TEXTURE1X );
            break;
        }

        switch ( m_opCode )
        {
        case D3DSIO_TEXCOORD:
        case D3DSIO_TEX:
        case D3DSIO_TEXM3x2PAD:
        case D3DSIO_TEXM3x2TEX:
        case D3DSIO_TEXM3x2DEPTH:
        case D3DSIO_TEXM3x3PAD:
        case D3DSIO_TEXM3x3:
        case D3DSIO_TEXM3x3TEX:
        case D3DSIO_TEXM3x3SPEC:
        case D3DSIO_TEXM3x3VSPEC:
        case D3DSIO_TEXDP3TEX:
        case D3DSIO_TEXDP3:
        case D3DSIO_TEXBEM:
        case D3DSIO_TEXBEML:

            // In ps_1_x, these instructions read from a texcoords register,
            // we need to manually define the register

            DWORD dwModifier = D3DSPSM_NONE;
            if ( m_pContext->m_rasterStates.ProjectedTCsMask & ( 1 << dwRegNum ) )
            {
                dwModifier = D3DSPSM_DW;
            }

            DWORD dwSrcToken = __D3DI_SRC_TOKEN( D3DSPR_TEXTURE,
                                                 dwRegNum,
                                                 D3DVS_NOSWIZZLE,
                                                 dwModifier );

            m_srcTokens[ m_srcTokenCount++ ] = Token( dwSrcToken, 0 );

            break;
        }
    }

    m_dstToken = Token( dwToken, dwAddress );

    return pdwToken;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
const DWORD*
CInstr::AddSrcToken( const DWORD* pdwToken )
{
    SHADER_CONV_ASSERT( pdwToken );

    DWORD dwToken   = *pdwToken++;
    DWORD dwAddress = 0;

    if ( D3DSI_GETADDRESSMODE( dwToken ) & D3DSHADER_ADDRMODE_RELATIVE )
    {
        if ( ( __IS_VS( m_version ) && m_version >= D3DVS_VERSION(2,0) ) ||
             ( __IS_PS( m_version ) && m_version >= D3DPS_VERSION(3,0) ) )
        {
            dwAddress = *pdwToken++;
        }
    }

    const DWORD dwRegType = D3DSI_GETREGTYPE_RESOLVING_CONSTANTS( dwToken );

    if ( __IS_PS( m_version ) &&
         m_version <= D3DPS_VERSION(1,3) &&
         D3DSIO_TEXKILL != m_opCode &&
         D3DSPR_TEXTURE == dwRegType )
    {
        // In ps_1_x, the texture register are used as temp registers,
        // we need to change the type to our custom register type
        dwToken &= ~( D3DSP_REGTYPE_MASK | D3DSP_REGTYPE_MASK2 );
        dwToken |= __D3DI_TYPE_TOKEN( __D3DSPR_TEXTURE1X );
    }

    if ( m_srcTokenCount < MAX_SRC_REGISTERS )
    {
        m_srcTokens[ m_srcTokenCount++ ] = Token( dwToken, dwAddress );
    }

    return pdwToken;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
COperandBase
CInstr::CreateDstOperand() const
{
    const DWORD dwToken   = m_dstToken.Value;
    const DWORD dwRegNum  = D3DSI_GETREGNUM_RESOLVING_CONSTANTS( dwToken );
    const DWORD dwRegType = D3DSI_GETREGTYPE_RESOLVING_CONSTANTS( dwToken );

    // Compute the write mask
    UINT writeMask = __getWriteMask( dwToken );

    //
    //  Get the register index
    //

    UINT regIndex = INVALID_INDEX;


    if ( m_modifiers._PREDICATE )
    {
        regIndex = SREG_PRED;
    }
    else
    {
        switch ( dwRegType )
        {
        default:
            SHADER_CONV_ASSERT(!"Invalid register type.");

        case D3DSPR_TEMP:
            // Temporary register
            SHADER_CONV_ASSERT( dwRegNum < MAX_TEMP_REGS );
            regIndex = m_inputRegs.r[dwRegNum];
            break;

        case D3DSPR_COLOROUT:
            // PS output color register
            SHADER_CONV_ASSERT( dwRegNum < MAX_PS_COLOROUT_REGS );
            regIndex = m_outputRegs.oC[dwRegNum];
            break;

        case D3DSPR_DEPTHOUT:
            // PS output depth register
            SHADER_CONV_ASSERT( dwRegNum < MAX_PS_DEPTHOUT_REGS );
            regIndex = m_outputRegs.oDepth;
            break;

        case D3DSPR_PREDICATE:
            // Predicate register
            regIndex = m_inputRegs.p0;
            break;

        case D3DSPR_RASTOUT:
            switch ( dwRegNum )
            {
            case D3DSRO_POSITION:
                // VS output position register
                regIndex = m_outputRegs.O[VSOREG_Position];
                break;

            case D3DSRO_FOG:
                // VS output fog register
                regIndex = m_outputRegs.O[VSOREG_FogPSize];
                writeMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
                break;

            case D3DSRO_POINT_SIZE:
                // VS output point size register
                regIndex = m_outputRegs.O[VSOREG_FogPSize];
                writeMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;
                break;

            default:
                SHADER_CONV_ASSERT(!"Invalid register number.");
                break;
            }
            break;

        case D3DSPR_ATTROUT:
            // VS output color register
            SHADER_CONV_ASSERT( dwRegNum < MAX_VS_COLOR_REGS );
            regIndex = m_outputRegs.O[VSOREG_Color0 + dwRegNum];
            break;

        case D3DSPR_TEXCRDOUT:
            SHADER_CONV_ASSERT( __IS_VS( m_version ) );
            if ( m_version >= D3DVS_VERSION(3,0) )
            {
                // VS output register
                SHADER_CONV_ASSERT( dwRegNum < MAX_VS_OUTPUT_REGS );
                regIndex = m_outputRegs.O[dwRegNum];
            }
            else
            {
                // VS output texture coordinates register
                SHADER_CONV_ASSERT( dwRegNum < MAX_VS_TEXCOORD_REGS );
                regIndex = m_outputRegs.O[VSOREG_TexCoord0 + dwRegNum];
            }
            break;

        case D3DSPR_ADDR:
            // VS input address register
            regIndex = m_inputRegs.a0;
            break;

        case __D3DSPR_TEXTURE1X:
            // PS 1.x texture register
            SHADER_CONV_ASSERT( dwRegNum < MAX_PS1X_TEXTURE_REGS );
            regIndex = m_inputRegs._t[dwRegNum];
            break;
        }
    }

    if ( regIndex != INVALID_INDEX )
    {
        return CTempOperandDst( regIndex, writeMask );
    }
    else
    {
        SHADER_CONV_ASSERT( D3DSPR_TEXCRDOUT == dwRegType );
        SHADER_CONV_ASSERT( m_version >= D3DVS_VERSION(3,0) );

        COperandBase dstOperand = CTempOperandDst2D( 0, dwRegNum, writeMask );

        if ( D3DSI_GETADDRESSMODE( dwToken ) & D3DSHADER_ADDRMODE_RELATIVE )
        {
            const DWORD dwAddrToken = this->GetDstAddress();
            this->ApplyAddressing( dstOperand, dwAddrToken, 1 );
        }

        return dstOperand;
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
COperandBase
CInstr::CreateSrcOperand( DWORD dwToken, DWORD dwOffset ) const
{
    const DWORD dwRegNum  = D3DSI_GETREGNUM_RESOLVING_CONSTANTS( dwToken ) + dwOffset;
    const DWORD dwRegType = D3DSI_GETREGTYPE_RESOLVING_CONSTANTS( dwToken );

    // Determine if the operand has a relative address
    const bool hasRelativeAddress = ( D3DSI_GETADDRESSMODE( dwToken ) & D3DSHADER_ADDRMODE_RELATIVE ) ? true : false;

    // Determine if inline shader constants is supported (only available to Dx9 and above)
    const bool bInlineConstsEnabled = (m_pContext->m_runtimeVersion >= 9);

    ShaderConst shaderConst;

    switch ( dwRegType )
    {
    default:
        NO_DEFAULT;

    case D3DSPR_TEMP:
        // Temporary register
        SHADER_CONV_ASSERT( dwRegNum < MAX_TEMP_REGS );
        return CTempOperand4( m_inputRegs.r[dwRegNum] );

    case D3DSPR_INPUT:
        {
            DWORD dwRegIndex = GetInputRegisterIndex(m_version, dwRegNum);
            if (__IS_PS(m_version) || __IS_VS(m_version))
            {
                return m_pContext->EmitInputOperand(dwRegIndex);
            }
            else
            {
                return CInputOperand4(dwRegIndex);
            }
        }

    case D3DSPR_TEXTURE:
        // Input texture register
        SHADER_CONV_ASSERT(dwRegNum < MAX_PS_TEXCOORD_REGS);
        return m_pContext->EmitInputOperand(GetTextureRegisterIndex(m_version, dwRegNum));
    case D3DSPR_CONST:
        // Constant float register
        if ( !bInlineConstsEnabled ||
             hasRelativeAddress ||
             !m_pContext->m_pShaderDesc->FindInlineConstant( CB_FLOAT, dwRegNum * 4, &shaderConst ) )
        {
            return CCBOperand2D( CB_FLOAT, dwRegNum );
        }
        else
        {
            return COperand( shaderConst.fValue[0],
                             shaderConst.fValue[1],
                             shaderConst.fValue[2],
                             shaderConst.fValue[3] );
        }

    case D3DSPR_CONSTINT:
        // Constant integer register
        if ( !bInlineConstsEnabled ||
             hasRelativeAddress ||
             !m_pContext->m_pShaderDesc->FindInlineConstant( CB_INT, dwRegNum * 4, &shaderConst ) )
        {
            return CCBOperand2D( CB_INT, dwRegNum );
        }
        else
        {
            return COperand( shaderConst.iValue[0],
                             shaderConst.iValue[1],
                             shaderConst.iValue[2],
                             shaderConst.iValue[3] );
        }

    case D3DSPR_CONSTBOOL:
        // Constant boolean register
        if ( !bInlineConstsEnabled ||
             hasRelativeAddress ||
             !m_pContext->m_pShaderDesc->FindInlineConstant( CB_BOOL, dwRegNum, &shaderConst ) )
        {
            // Bool constant registers have 1 dimension,
            // need to recompute the index and appy the correct swizzle
            const UINT regIndex = dwRegNum >> 2;
            const UINT componentIndex = dwRegNum & 3;

            return CSwizzle( CCBOperand2D( CB_BOOL, regIndex ), __SWIZZLE1( componentIndex )  );
        }
        else
        {
            return COperand( shaderConst.bValue );
        }

    case D3DSPR_SAMPLER:
        // Sampler input register
        return COperand( D3D10_SB_OPERAND_TYPE_SAMPLER, dwRegNum );

    case D3DSPR_LABEL:
        // label register
        return COperand( D3D10_SB_OPERAND_TYPE_LABEL, dwRegNum );

    case D3DSPR_PREDICATE:
        // Predicate register
        return CTempOperand4( m_inputRegs.p0 );

    case D3DSPR_LOOP:
        // Loop counter register
        return CTempOperand4( m_inputRegs.aL );

    case __D3DSPR_TEXTURE1X:
        // PS 1.x texture register
        SHADER_CONV_ASSERT( dwRegNum < MAX_PS1X_TEXTURE_REGS );
        return CTempOperand4( m_inputRegs._t[dwRegNum] );

    case D3DSPR_MISCTYPE:
        {
            UINT regIndex;
            switch ( dwRegNum )
            {
            default:
                NO_DEFAULT;

            case D3DSMO_POSITION:
                regIndex = m_inputRegs.v[PSIREG_VPos].Reg();
                break;

            case D3DSMO_FACE:
                regIndex = m_inputRegs.v[PSIREG_VFace].Reg();
                break;
            }
            return CTempOperand4( regIndex );
        }
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CInstr::ApplyAddressing( COperandBase& operand, DWORD dwAddrToken, UINT relIndex ) const
{
    if ( dwAddrToken )
    {
        UINT regIndex;

        const DWORD dwAddrRegType = D3DSI_GETREGTYPE_RESOLVING_CONSTANTS( dwAddrToken );
        switch ( dwAddrRegType )
        {
        default:
            NO_DEFAULT;

        case D3DSPR_ADDR:
            regIndex = m_inputRegs.a0;
            break;

        case D3DSPR_LOOP:
            regIndex = m_inputRegs.aL;
            break;
        }

        D3D10_SB_4_COMPONENT_NAME componentName;

        const DWORD dwSwizzle = dwAddrToken & D3DVS_SWIZZLE_MASK;
        switch ( ( dwSwizzle >> D3DVS_SWIZZLE_SHIFT ) & 0x3 )
        {
        default:
            NO_DEFAULT;

        case 0:
            componentName = D3D10_SB_4_COMPONENT_X;
            break;

        case 1:
            componentName = D3D10_SB_4_COMPONENT_Y;
            break;

        case 2:
            componentName = D3D10_SB_4_COMPONENT_Z;
            break;

        case 3:
            componentName = D3D10_SB_4_COMPONENT_W;
            break;
        }

        SHADER_CONV_ASSERT( regIndex != INVALID_INDEX );
        SetRelAddress( operand,
                       relIndex,
                       D3D10_SB_OPERAND_TYPE_TEMP,
                       regIndex,
                       componentName );
    }
    else
    {
        SHADER_CONV_ASSERT( m_inputRegs.a0 != INVALID_INDEX );
        SetRelAddress( operand,
                       relIndex,
                       D3D10_SB_OPERAND_TYPE_TEMP,
                       m_inputRegs.a0,
                       D3D10_SB_4_COMPONENT_X );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
IContext::WriteClipplanes( UINT activeClipPlanesMask, const COperandBase& position )
{
    // dp4 oC.[i], pos, cb3[clipplane[0..i]]
    for ( UINT i = 0, j = 0; i < MAX_CLIPLANES; ++i )
    {
        const UINT clipRegIndex = ( j < 4 ) ? VSOREG_ClipDist0 : VSOREG_ClipDist1;
        if ( activeClipPlanesMask & ( 1 << i ) )
        {
            m_pShaderAsm->EmitInstruction(
                CInstruction(
                    D3D10_SB_OPCODE_DP4,
                    COperandDst( D3D10_SB_OPERAND_TYPE_OUTPUT,
                                 clipRegIndex,
                                 D3D10_SB_OPERAND_4_COMPONENT_MASK_X << ( j % 4 ) ),
                    position,
                    CCBOperand2D( CB_VS_EXT, VSCBExtension::CLIPPLANE0 + i ) ) );
            ++j;
        }
    }
}

bool OperandCanBeInfinityOrNan(const COperandBase &op)
{
    bool operandCanBeInfinity = true;
    if (op.OperandType() == D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER)
    {
        // EXT constants created directly by 9on12 are verified to be floats that aren't nan or inf
        UINT regIndex0 = op.OperandIndex(0)->m_RegIndex;
        const bool bIsVSExt = regIndex0 == CB_VS_EXT;
        const bool bIsPSExt = regIndex0 == CB_PS_EXT || regIndex0 == CB_PS_EXT2 || regIndex0 == CB_PS_EXT3;
        operandCanBeInfinity = !bIsVSExt && !bIsPSExt;
    }

    return operandCanBeInfinity;
}

void CShaderAsmWrapper::PatchMultiplicandArgs(
    _In_ COperandBase &src0,
    _In_ COperandBase &src1)
{
    if (IsMultiplicationPatchingEnabled() &&

        // Anything times itself can never result in a 0 * nan or 0 * inf situation
        src0 != src1 &&

        // If either of the sources are a non-zero, float literal, then there's no 
        // need for a check since the literal can't be inf/nan.
        !(src0.OperandType() == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 && src0.m_Valuef[0] != 0.0f) &&
        !(src1.OperandType() == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 && src1.m_Valuef[0] != 0.0f))
    {
        // Assuming neither src's are safe as multiplicands, the emitted asm is:
        //
        // movc temp0, src0, src1, 0.0f
        // movc temp1, src1, src0, 0.0f
        //
        // This ensures that temp0 * temp1 == src0 * src1 except for cases when
        // either may have inf/nan. In these cases, the movc ensures that if the 
        // other multiplicand has 0, then the nan/inf will be overwritten with 0.0f
        if (OperandCanBeInfinityOrNan(src1))
        {
            EmitExtraInstructionInternal(
                CInstruction(
                    D3D10_SB_OPCODE_MOVC,
                    CTempOperandDst(SREG_MUL0),
                    src0,
                    src1,
                    COperand(0.0f)));

            src1 = CTempOperand4(SREG_MUL0);
        }

        if (OperandCanBeInfinityOrNan(src0))
        {
            EmitExtraInstructionInternal(
                CInstruction(
                    D3D10_SB_OPCODE_MOVC,
                    CTempOperandDst(SREG_MUL1),
                    src1,
                    src0,
                    COperand(0.0f)));
            src0 = CTempOperand4(SREG_MUL1);
        }
    }
}
} // namespace ShaderConv