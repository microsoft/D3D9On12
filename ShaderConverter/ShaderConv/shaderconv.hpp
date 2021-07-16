// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Header Definitions for The Shader Converter objects
*
****************************************************************************/

#pragma once

#include "ShaderConvInternal.h"

namespace ShaderConv
{

#define __safeRelease( p )      if ( p ) { p->Release(); p = NULL; }

#define __safeDelete( p )       if ( p ) { delete p; p = NULL; }

#define __safeDeleteArray( p )  if ( p ) { delete[] p; p = NULL; }

#define __mincountof( min, reg ) \
    if ( (reg) < min ) { min = (reg); }

#define __maxcountof( max, reg ) \
    if ( (reg) > max ) { max = (reg); }

#define __sizeofn( type, bytes ) \
    ( ( sizeof( type ) + ( (bytes) - 1 ) ) / (bytes) )

#define __sizeof16( type )    __sizeofn( type, 16 )

#define __IS_VS( version ) \
    ( ( (version) & 0xFFFF0000 ) == 0xFFFE0000 )

#define __IS_PS( version ) \
    ( ( (version) & 0xFFFF0000 ) == 0xFFFF0000 )

#define __D3DSPR_TEXTURE1X  20

#define __D3DI_TYPE_TOKEN( regType ) \
    ( ( ( (regType) << D3DSP_REGTYPE_SHIFT  ) & D3DSP_REGTYPE_MASK ) | \
      ( ( (regType) << D3DSP_REGTYPE_SHIFT2 ) & D3DSP_REGTYPE_MASK2 ) )

#define __D3DI_SRC_TOKEN( regType, index, swizzle, mod ) \
    __D3DI_TYPE_TOKEN( regType ) | (index) | (swizzle) | (mod) | 0x80000000

#define __D3DI_DST_TOKEN( regType, index, writeMask ) \
    __D3DI_TYPE_TOKEN( regType ) | (index) | (writeMask) | 0x80000000

#define D3DSP_WRITEMASK_SHIFT 16
#define D3DSP_WRITEMASK_MASK  0x000f0000

inline UINT
__getInstrLength( DWORD dwInstr, DWORD dwVersion )
{
    const D3DSHADER_INSTRUCTION_OPCODE_TYPE opCode =
            (D3DSHADER_INSTRUCTION_OPCODE_TYPE)( dwInstr & D3DSI_OPCODE_MASK );

    if ( D3DSIO_COMMENT == opCode )
    {
        return ( ( (UINT)dwInstr & D3DSI_COMMENTSIZE_MASK ) >> D3DSI_COMMENTSIZE_SHIFT ) + 1;
    }
    else
    if ( D3DSIO_DEF == opCode ||
         D3DSIO_DEFI == opCode )
    {
        return 6;
    }
    else
    if ( D3DSIO_DEFB == opCode )
    {
        return 3;
    }
    else
    if ( __IS_PS( dwVersion ) ||
         dwVersion >= D3DVS_VERSION(2,0) )
    {
        return D3DSI_GETINSTLENGTH( dwInstr ) + 1;
    }
    else
    {
        switch ( opCode )
        {
        case D3DSIO_DCL :
            return 3;

        case D3DSIO_DEF :
            return 6;

        // 0 source instructions
        case D3DSIO_END :
        case D3DSIO_NOP :
            return 1;

        // 1 source instructions
        case D3DSIO_EXP :
        case D3DSIO_EXPP:
        case D3DSIO_FRC :
        case D3DSIO_LIT :
        case D3DSIO_LOG :
        case D3DSIO_LOGP:
        case D3DSIO_MOV :
        case D3DSIO_RCP :
        case D3DSIO_RSQ :
            return 3;

        // 2 source instructions
        case D3DSIO_ADD  :
        case D3DSIO_DP3  :
        case D3DSIO_DP4  :
        case D3DSIO_DST  :
        case D3DSIO_M4x4 :
        case D3DSIO_M4x3 :
        case D3DSIO_M3x4 :
        case D3DSIO_M3x3 :
        case D3DSIO_M3x2 :
        case D3DSIO_MAX  :
        case D3DSIO_MIN  :
        case D3DSIO_MUL  :
        case D3DSIO_SGE  :
        case D3DSIO_SLT  :
            return 4;

        // 3 source instructions
        case D3DSIO_MAD  :
            return 4;

        default:
            //WarpError("Invalid instruction opCode.");
            return 1;
        }
    }
}

inline BYTE
__getConstRegSize( UINT opCode )
{
    switch ( opCode )
    {
    case D3DSIO_M3x2:
        return 2;

    case D3DSIO_M3x3:
    case D3DSIO_M4x3:
        return 3;

    case D3DSIO_M3x4:
    case D3DSIO_M4x4:
        return 4;
    }

    return 1;
}

inline D3D10_SB_RESOURCE_DIMENSION
__toResourceDimension10( TEXTURETYPE textureType )
{
    switch ( textureType )
    {
    default:
    case TEXTURETYPE_UNKNOWN:
        // Default to Tex2D in cases where we don't have enough information to make a better decision.
        // Better to have a valid shader which isn't exactly what was asked for, than an invalid shader.
    case TEXTURETYPE_2D:
        return D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D;

    case TEXTURETYPE_CUBE:
        return D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE;

    case TEXTURETYPE_VOLUME:
        return D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D;
    }
}

inline TEXTURETYPE
__toTextureType( D3DSAMPLER_TEXTURE_TYPE samplerType )
{
    switch ( samplerType )
    {
    default:
    case D3DSTT_UNKNOWN:
        return TEXTURETYPE_UNKNOWN;

    case D3DSTT_2D:
        return TEXTURETYPE_2D;

    case D3DSTT_CUBE:
        return TEXTURETYPE_CUBE;

    case D3DSTT_VOLUME:
        return TEXTURETYPE_VOLUME;
    }
}

inline UINT
__getComponentsNeeded(TEXTURETYPE samplerType, bool UsingHardwareShadowMapping)
{
    switch (samplerType)
    {
    default:
    case TEXTURETYPE_UNKNOWN:
        return 4;

    case TEXTURETYPE_2D:
        // When using hardware shadow mapping, a third channel is used as the comparison value for SampleCmp
        if (UsingHardwareShadowMapping)
        {
            return 3;
        }
        else
        {
            return 2;
        }

    case TEXTURETYPE_CUBE:
    case TEXTURETYPE_VOLUME:
        return 3;
    }
}

inline UINT __getWriteMaskFromTextureType(TEXTURETYPE samplerType)
{
    switch (samplerType)
    {
    default:
    case TEXTURETYPE_UNKNOWN:
        return D3D10_SB_OPERAND_4_COMPONENT_MASK_X | D3D10_SB_OPERAND_4_COMPONENT_MASK_Y | D3D10_SB_OPERAND_4_COMPONENT_MASK_Z | D3D10_SB_OPERAND_4_COMPONENT_MASK_W;

    case TEXTURETYPE_2D:
        return D3D10_SB_OPERAND_4_COMPONENT_MASK_X | D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;

    case TEXTURETYPE_CUBE:
    case TEXTURETYPE_VOLUME:
        return D3D10_SB_OPERAND_4_COMPONENT_MASK_X | D3D10_SB_OPERAND_4_COMPONENT_MASK_Y | D3D10_SB_OPERAND_4_COMPONENT_MASK_Z;
    }
}

inline UINT
__getWriteMask( DWORD dwToken )
{
    UINT writeMask = 0;

    if ( dwToken & D3DSP_WRITEMASK_0 )
    {
        writeMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
    }

    if ( dwToken & D3DSP_WRITEMASK_1 )
    {
        writeMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;
    }

    if ( dwToken & D3DSP_WRITEMASK_2 )
    {
        writeMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_Z;
    }

    if ( dwToken & D3DSP_WRITEMASK_3 )
    {
        writeMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_W;
    }

    return writeMask;
}

inline UINT
__getSwizzle( DWORD dwToken, UINT numComponents )
{
    UINT value   = 0;
    UINT swizzle = 0;

    const DWORD dwSwizzleMask = dwToken & D3DVS_SWIZZLE_MASK;
    for ( UINT i = 0; i < numComponents; ++i )
    {
        value = ( ( dwSwizzleMask >> ( D3DVS_SWIZZLE_SHIFT + 2 * i ) ) & 0x3 );
        swizzle |= __SWIZZLEN( i, value );
    }

    for ( UINT i = numComponents; i < 4; ++i )
    {
       swizzle |= __SWIZZLEN( i, value );
    }

    return swizzle;
}

inline UINT
__swizzleFromWriteMask( UINT writeMask )
{
    UINT swizzle  = 0;
    UINT position = 0;
    UINT lastcomp = 0;

    if ( writeMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_X )
    {
        swizzle |= __SWIZZLEN( position++, D3D10_SB_4_COMPONENT_X );
        lastcomp = D3D10_SB_4_COMPONENT_X;
    }

    if ( writeMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Y )
    {
        swizzle |= __SWIZZLEN( position++, D3D10_SB_4_COMPONENT_Y );
        lastcomp = D3D10_SB_4_COMPONENT_Y;
    }

    if ( writeMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Z )
    {
        swizzle |= __SWIZZLEN( position++, D3D10_SB_4_COMPONENT_Z );
        lastcomp = D3D10_SB_4_COMPONENT_Z;
    }

    if ( writeMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_W )
    {
        swizzle |= __SWIZZLEN( position++, D3D10_SB_4_COMPONENT_W );
        lastcomp = D3D10_SB_4_COMPONENT_W;
    }

    while ( position < 4 )
    {
        swizzle |= __SWIZZLEN( position++, lastcomp );
    }

    return swizzle;
}

enum eVSOuputRegister
{
    VSOREG_Position     = 0,
    VSOREG_Color0       = 1,
    VSOREG_Color1       = 2,
    VSOREG_TexCoord0    = 3,
    VSOREG_TexCoord1    = 4,
    VSOREG_TexCoord2    = 5,
    VSOREG_TexCoord3    = 6,
    VSOREG_TexCoord4    = 7,
    VSOREG_TexCoord5    = 8,
    VSOREG_TexCoord6    = 9,
    VSOREG_TexCoord7    = 10,
    VSOREG_FogPSize     = 11,    
    VSOREG_PointSprite  = 12,    

    VSOREG_ClipDist0    = 13,
    VSOREG_ClipDist1    = 14,

    VSOREG_SIZE,
};

enum ePSInputRegister
{
    PSIREG_V0    = 0,
    PSIREG_V1    = 1,
    PSIREG_V2    = 2,
    PSIREG_V3    = 3,
    PSIREG_V4    = 4,
    PSIREG_V5    = 5,
    PSIREG_V6    = 6,
    PSIREG_V7    = 7,
    PSIREG_V8    = 8,
    PSIREG_V9    = 9,
    PSIREG_V10   = 10,
    PSIREG_V11   = 11,
    PSIREG_V12   = 12,

    PSIREG_VPos  = 13,
    PSIREG_VFace = 14,

    PSIREG_SIZE,
};

inline bool IsImplicitFogCalculationNeeded(const RasterStates& rasterStates, UINT version)
{
    return version < D3DPS_VERSION(3, 0) && rasterStates.FogEnable;
}

// Wraps CShaderASM so that extra commands can be patched in at EmitInstruction.
// Allows for specifying special shader behavior like enforcing "anything time 0 
// always equals 0"
class CShaderAsmWrapper : public CShaderAsm
{
public:
    CShaderAsmWrapper(){}

    void StartShader(D3D10_SB_TOKENIZED_PROGRAM_TYPE ShaderType, UINT vermajor, UINT verminor, UINT ShaderSettings)
    {
        m_ShaderFlags = ShaderSettings;
        m_InstructionsEmitted = m_ExtraInstructionsEmitted = 0;
        CShaderAsm::StartShader(ShaderType, vermajor, verminor);
    }

    void EmitInstruction(const CInstruction& inst)
    {
        CInstruction patchedInstruction = inst;
        if (InstructionRequiresMultiplicationPatch(patchedInstruction))
        {
            COperandBase multiplicandSrc0;
            COperandBase multiplicandSrc1;

            UINT multiplicandIndex0 = 1, multiplicandIndex1 = 2;
            switch (patchedInstruction.OpCode())
            {
            case D3D10_SB_OPCODE_DP2:
            case D3D10_SB_OPCODE_DP3:
            case D3D10_SB_OPCODE_DP4:
            case D3D10_SB_OPCODE_MUL:
            case D3D10_SB_OPCODE_MAD:
                break;
            default:
                assert(false);
            }

            PatchMultiplicandArgs(
                patchedInstruction.m_Operands[multiplicandIndex0],
                patchedInstruction.m_Operands[multiplicandIndex1]);
        }
        EmitInstructionInternal(patchedInstruction);
    }

    bool IsMultiplicationPatchingEnabled()
    {
        return (m_ShaderFlags & ShaderSettings::AnythingTimes0Equals0);
    }

    UINT GetTotalInstructionsEmitted() const 
    {
        return m_InstructionsEmitted;
    }

    UINT GetTotalExtraInstructionsEmitted() const
    {
        return m_ExtraInstructionsEmitted;
    }

protected:
    void EmitExtraInstructionInternal(const CInstruction& inst)
    {
        m_ExtraInstructionsEmitted++;
        EmitInstructionInternal(inst);
    }

    void EmitInstructionInternal(const CInstruction& inst) 
    { 
        m_InstructionsEmitted++;
        CShaderAsm::EmitInstruction(inst);
    }


    UINT m_ShaderFlags;

    UINT m_InstructionsEmitted;
    UINT m_ExtraInstructionsEmitted;

    bool InstructionRequiresMultiplicationPatch(const CInstruction& instruction)
    {
        return IsMultiplicationPatchingEnabled() && OpTypeImpliesFloatMultiplication(instruction.OpCode());
    }

    void PatchMultiplicandArgs(
        _In_ COperandBase &src0,
        _In_ COperandBase &src1);

    // Returns true for any optype that would imply that the hardware needs 
    // to do any sort of float multiplication
    static bool OpTypeImpliesFloatMultiplication(D3D10_SB_OPCODE_TYPE opType)
    {
        switch (opType)
        {
        case D3D10_SB_OPCODE_DP2:
        case D3D10_SB_OPCODE_DP3:
        case D3D10_SB_OPCODE_DP4:
        case D3D10_SB_OPCODE_MUL:
        case D3D10_SB_OPCODE_MAD:
            return true;
        default:
            return false;
        }
    }
};

class CTranslator : public ITranslator
{
public:

    static HRESULT Create( UINT runtimeVersion, ITranslator** ppTranslator );

    HRESULT AnalyzeVS( const void* pSrcBytes,
                       UINT cbCodeSize,
                       UINT shaderSettings,
                       const RasterStates& rasterStates,
                       const VSInputDecls *pReferenceInputDecls,
                       CVertexShaderDesc** ppShaderDesc );

    HRESULT AnalyzePS( const void* pSrcBytes,
                       UINT cbCodeSize,
                       UINT shaderSettings,
                       const RasterStates& rasterStates,
                       CPixelShaderDesc** ppShaderDesc );

    HRESULT TranslateVS( const CVertexShaderDesc* pShaderDesc,
                         const RasterStates& rasterStates,
                         CCodeBlob** ppCodeBlob );

    HRESULT TranslateTLVS( const CTLVertexShaderDesc* pShaderDesc,
                           CCodeBlob** ppCodeBlob );    

    HRESULT TranslatePS( const CPixelShaderDesc* pShaderDesc,
                         const RasterStates& rasterStates,
                         const ShaderConv::VSOutputDecls& inputDecls,
                         CCodeBlob** ppCodeBlob );

    HRESULT TranslateGS( const CGeometryShaderDesc* pShaderDesc,
                         CCodeBlob** ppCodeBlob );

    TranslationData GetTranslationData()
    {
        return{ m_pShaderAsm->GetTotalInstructionsEmitted(), m_pShaderAsm->GetTotalExtraInstructionsEmitted() };
    }
private:
    void DeclareClipplaneRegisters(
        VSOutputDecls &outputDecls,
        UINT activeClipPlanesMask);

    CTranslator( UINT runtimeVersion );
    virtual ~CTranslator();

    HRESULT Initialize();

    UINT        m_runtimeVersion;
    CShaderAsmWrapper* m_pShaderAsm;
};

} // namespace ShaderConv
