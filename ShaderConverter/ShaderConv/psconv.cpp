// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Implementation for The Pixel Shader Converter
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
CPixelShaderDesc::Create( CPixelShaderDesc** ppPixelShaderDesc )
{
    SHADER_CONV_ASSERT( ppPixelShaderDesc );

    CPixelShaderDesc* const pPixelShaderDesc = new CPixelShaderDesc();
    if ( NULL == pPixelShaderDesc )
    {
        SHADER_CONV_ASSERT(FALSE);
        return E_OUTOFMEMORY;
    }

    pPixelShaderDesc->AddRef();
    *ppPixelShaderDesc = pPixelShaderDesc;

    return S_OK;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CPixelShaderDesc::UpdateInputDecls( 
    const VSOutputDecls& vsOuputDecls,
    const ShaderConv::RasterStates& rasterStates,
    VSOutputDecls* pMergedDecls
    )
{
    SHADER_CONV_ASSERT(pMergedDecls);

    // Obtain the current TCI mapping (ignored for PS3.0)
    const UINT uiTCIMapping = m_version < D3DPS_VERSION(3,0) ?
        rasterStates.TCIMapping : TCIMASK_PASSTHRU;

    // Check if point sprite processing is enabled
    const bool bPointSpriteEnabled = rasterStates.PointSpriteEnable 
                                  && ((D3DPT_POINTLIST == rasterStates.PrimitiveType) 
                                   || (D3DFILL_POINT == rasterStates.FillMode));

    BYTE positionRegister = (BYTE)vsOuputDecls.FindRegisterIndex(D3DDECLUSAGE_POSITION, 0);
    SHADER_CONV_ASSERT(positionRegister != VSOutputDecls::INVALID_INDEX);
    SetPositionRegister(positionRegister);

    // Copy the current input declarion
    *pMergedDecls = m_inputDecls;

    const UINT nNumMergedDecls = pMergedDecls->GetSize();
    const UINT nNumInputDecls  = vsOuputDecls.GetSize();

    for (UINT i = 0; i < nNumMergedDecls; ++i)
    {
        const UINT usage = (*pMergedDecls)[i].Usage;
        UINT usageIndex = (*pMergedDecls)[i].UsageIndex;

        // Skip VPOS and VFACE input registers
        // since they are not produced by the vertex shader
        if (usage <= D3DDECLUSAGE_SAMPLE)
        {
            if (D3DDECLUSAGE_TEXCOORD == usage)
            {
                if (bPointSpriteEnabled 
                 && ((m_version < D3DPS_VERSION(2,0)) 
                  || (0 == usageIndex)))
                {
                    // Pointsprite generated texture coordinates map to VSOREG_TexCoord0
                    const BYTE regIndexIn = (*pMergedDecls)[i].RegIndex;
                    (*pMergedDecls)[i].RegIndex  = VSOREG_PointSprite;
                    (*pMergedDecls)[i].WriteMask = D3DSP_WRITEMASK_ALL >> __D3DSP_WRITEMASK_SHIFT;
                    SHADER_CONV_ASSERT(regIndexIn < _countof(m_inputRegs.v));
                    m_inputRegs.v[regIndexIn] = InputRegister(VSOREG_PointSprite, InputRegister::Input, (*pMergedDecls)[i].WriteMask);
                    continue;
                }

                if (uiTCIMapping != TCIMASK_PASSTHRU)
                {
                    // Compute the new usage index from TCI mask
                    usageIndex = (uiTCIMapping >> (4 * usageIndex)) & 0xf;
                }
            }

            const VSOutputDecl *pOutputDecl = vsOuputDecls.FindOutputDecl(usage, usageIndex);
            if (pOutputDecl)
            {
                const BYTE regIndexIn = (*pMergedDecls)[i].RegIndex;
                (*pMergedDecls)[i].RegIndex = pOutputDecl->RegIndex;
                (*pMergedDecls)[i].WriteMask = pOutputDecl->WriteMask;
                SHADER_CONV_ASSERT(regIndexIn < _countof(m_inputRegs.v));
                m_inputRegs.v[regIndexIn] = InputRegister(pOutputDecl->RegIndex, InputRegister::Input, (*pMergedDecls)[i].WriteMask);
            }
            else
            {
                // The input register could not be found, 
                // clear its writemask and input register
                (*pMergedDecls)[i].WriteMask = 0;
                const BYTE regIndexIn = (*pMergedDecls)[i].RegIndex;                
                SHADER_CONV_ASSERT(regIndexIn < _countof(m_inputRegs.v));
                m_inputRegs.v[regIndexIn] = InputRegister(INVALID_INDEX, InputRegister::Undeclared, (*pMergedDecls)[i].WriteMask);
                continue;
            }
        }
        else if(D3DDECLUSAGE_VPOS == usage && usageIndex == 0)
        {
            // Patch VPOS to use the position register declared by the Vertex Shader
            (*pMergedDecls)[i].RegIndex = positionRegister;
        }
    }

    // Resolve pixel fog extension
    if (rasterStates.FogEnable)
    {
        // Check if the input fog or specular is available
        if (vsOuputDecls.Fog 
        ||  (vsOuputDecls.Colors & 0x2))
        {
            UINT uiFogInputDcl      = VSOutputDecls::INVALID_INDEX;
            UINT uiSpecularInputDcl = VSOutputDecls::INVALID_INDEX;

            for (UINT j = 0; j < nNumInputDecls; ++j)
            {
                const VSOutputDecl ouputDecl = vsOuputDecls[j];
                if (D3DDECLUSAGE_FOG == ouputDecl.Usage)
                {                  
                    uiFogInputDcl = j;
                }
                else
                if ((D3DDECLUSAGE_COLOR == ouputDecl.Usage) 
                 && (1 == ouputDecl.UsageIndex))
                {                  
                    uiSpecularInputDcl = j;
                }
            }

            if (uiFogInputDcl != VSOutputDecls::INVALID_INDEX)
            {
                pMergedDecls->AddDecl(vsOuputDecls[uiFogInputDcl]);
            }
            else
            {
                SHADER_CONV_ASSERT(uiSpecularInputDcl != VSOutputDecls::INVALID_INDEX);
                pMergedDecls->AddDecl(vsOuputDecls[uiSpecularInputDcl]);
            }
        }
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTranslator::AnalyzePS( const void* pSrcBytes,
                        UINT cbCodeSize,
                        UINT shaderSettings,
                        const RasterStates& rasterStates,
                        CPixelShaderDesc** ppShaderDesc )
{
    HRESULT hr;

    if ( NULL == pSrcBytes ||
         NULL == ppShaderDesc )
    {
        SHADER_CONV_ASSERT(!"CTranslator::AnalyzePS() failed, invalid parameters\n");
        return E_INVALIDARG;
    }

    // Get the shader version
    const DWORD* const pdwCodeBytes = reinterpret_cast<const DWORD*>( pSrcBytes );
    const DWORD dwVersion = *pdwCodeBytes;

    // Validate supported shader versions
    switch ( dwVersion )
    {
    case D3DPS_VERSION(1,0):
    case D3DPS_VERSION(1,1):
    case D3DPS_VERSION(1,2):
    case D3DPS_VERSION(1,3):
    case D3DPS_VERSION(1,4):
    case D3DPS_VERSION(2,0):
    case D3DPS_VERSION(2,1):
    case D3DPS_VERSION(3,0):
        break;

    default:
        SHADER_CONV_ASSERT(!"CTranslator::AnalyzePS() failed, shader version not supported: 0x%x\n");
        return E_INVALIDARG;
    }

#if DBG && WARP_INTERNAL
    // Disassemble input the shader code to the debug output
    hr = Disasm20( pdwCodeBytes, 0 );
    if ( FAILED( hr ) )
    {
        SHADER_CONV_ASSERT(!"Disasm20() failed, hr = %d\n");
        return hr;
    }

#endif

    VSOutputDecls inputDecls;
    InputRegs      inputRegs;
    OutputRegs      outputRegs;
    PSUsageFlags  usageFlags;

    UINT minUsedConstantsF = 0xFFFFFFFF;
    UINT minUsedConstantsI = 0xFFFFFFFF;
    UINT minUsedConstantsB = 0xFFFFFFFF;

    UINT maxUsedConstantsF = 0;
    UINT maxUsedConstantsI = 0;
    UINT maxUsedConstantsB = 0;

    bool hasRelativeAddressF = false;

    BYTE ubNumTempRegs = SREG_SIZE;
    BYTE uiNumLoopRegs = 0;

    // Determine if inline shader constants is supported (only available to Dx9 and above)
    const bool bInlineConstsEnabled = (m_runtimeVersion >= 9);

    // Create the shader descriptor
    CPixelShaderDesc* pShaderDesc;
    hr = CPixelShaderDesc::Create( &pShaderDesc );
    if ( FAILED( hr ) )
    {
        SHADER_CONV_ASSERT(FALSE);
        return hr;
    }

    // TODO: This looks like it's allocating sizeof(DWORD) times to many arrays
    // Allocate instructions buffer
    DWORD* pdwInstrs = new DWORD[cbCodeSize];
    if ( NULL == pdwInstrs )
    {
        hr = E_OUTOFMEMORY;
        goto L_ERROR;
    }

    DWORD* pdwCurInstr = pdwInstrs;
    *pdwCurInstr++ = dwVersion;

    //
    // Parse the shader to find the input and output registers
    //

    const DWORD* pdwCurToken = pdwCodeBytes + 1;
    while ( *pdwCurToken != D3DPS_END() )
    {
        if ( static_cast<UINT>( reinterpret_cast<const BYTE*>( pdwCurToken ) -
                                reinterpret_cast<const BYTE*>( pdwCodeBytes ) ) > cbCodeSize )
        {
            SHADER_CONV_ASSERT(!"CTranslator::AnalyzePS() failed, invalid shader\n");
            hr = E_FAIL;
            goto L_ERROR;
        }

        const DWORD dwInstr = *pdwCurToken;
        const UINT uiLength = __getInstrLength( dwInstr, dwVersion );
        const D3DSHADER_INSTRUCTION_OPCODE_TYPE opCode =
            (D3DSHADER_INSTRUCTION_OPCODE_TYPE)( dwInstr & D3DSI_OPCODE_MASK );

        switch ( opCode )
        {
        case D3DSIO_COMMENT:
        case D3DSIO_NOP:
            pdwCurToken += uiLength;
            continue;

        case D3DSIO_DCL:
            {
                const DWORD dwDclDesc = pdwCurToken[1];
                const DWORD dwDclReg  = pdwCurToken[2];
                const DWORD dwRegNum  = D3DSI_GETREGNUM( dwDclReg );
                const D3DSHADER_PARAM_REGISTER_TYPE regType = D3DSI_GETREGTYPE( dwDclReg );

                switch ( regType )
                {
                case D3DSPR_INPUT:
                    if ( dwVersion >= D3DPS_VERSION(3,0) )
                    {
                        // Generic input registers declaration
                        const UINT usage       = (D3DDECLUSAGE)D3DSI_GETUSAGE( dwDclDesc );
                        const UINT usageIndex  = D3DSI_GETUSAGEINDEX( dwDclDesc );
                        const UINT writeMask   = D3DSI_GETWRITEMASK( dwDclReg );
                        const bool bDoCentroid = ( ( D3DSPDM_MSAMPCENTROID & dwDclReg ) ? true : false ) || ( D3DDECLUSAGE_COLOR == usage );
                        inputDecls.AddDecl( usage, usageIndex, dwRegNum, writeMask, bDoCentroid );
                    }
                    else
                    {
                        // Input color register declaration
                        inputDecls.AddDecl( D3DDECLUSAGE_COLOR, dwRegNum, VSOREG_Color0 + dwRegNum, D3DSP_WRITEMASK_ALL );
                    }
                    break;

                case D3DSPR_TEXTURE:
                    {
                        // Input texture coordinates register declaration
                        const bool bDoCentroid = ( D3DSPDM_MSAMPCENTROID & dwDclReg ) ? true : false;
                        inputDecls.AddDecl( D3DDECLUSAGE_TEXCOORD, dwRegNum, VSOREG_TexCoord0 + dwRegNum, D3DSP_WRITEMASK_ALL, bDoCentroid );
                    }
                    break;

                case D3DSPR_SAMPLER:
                    {
                        // Texture sampler declaration
                        SHADER_CONV_ASSERT( dwRegNum < MAX_PS_SAMPLER_REGS );
                        inputRegs.s[dwRegNum] = (BYTE)__toTextureType( (D3DSAMPLER_TEXTURE_TYPE)( dwDclDesc & D3DSP_TEXTURETYPE_MASK ) );
                    }
                    break;

                case D3DSPR_MISCTYPE:
                    // Miscaleneous input registers declaration
                    switch ( dwRegNum )
                    {
                    case D3DSMO_POSITION:
                        inputDecls.AddDecl( D3DDECLUSAGE_VPOS, 0, PSIREG_VPos, D3DSP_WRITEMASK_ALL);
                        inputRegs.v[PSIREG_VPos] = InputRegister(ubNumTempRegs++, InputRegister::Temp, inputDecls[inputDecls.GetSize() - 1].WriteMask);
                        break;

                    case D3DSMO_FACE:
                        inputDecls.AddDecl( D3DDECLUSAGE_VFACE, 0, PSIREG_VFace, D3DSP_WRITEMASK_0 );
                        inputRegs.v[PSIREG_VFace] = InputRegister(ubNumTempRegs++, InputRegister::Temp, inputDecls[inputDecls.GetSize() - 1].WriteMask);
                        break;

                    default:
                        SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, invalid register number: %d\n");
                        hr = E_FAIL;
                        goto L_ERROR;
                    }
                    break;

                default:
                    SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, invalid register type: %d\n");
                    hr = E_FAIL;
                    goto L_ERROR;
                }
                pdwCurToken += uiLength;
            }
            continue;

        case D3DSIO_DEF:
            {
                const DWORD dwRegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS( pdwCurToken[1] );
                hr = pShaderDesc->AddInlineConstsF( dwRegNum, reinterpret_cast<const FLOAT*>( &pdwCurToken[2] ) );
                if ( FAILED( hr ) )
                {
                    SHADER_CONV_ASSERT(!"CShaderDesc::AddInlineConstsF() failed, hr = %d\n");
                    goto L_ERROR;
                }
                pdwCurToken += uiLength;
            }
            continue;

        case D3DSIO_DEFI:
            {
                const DWORD dwRegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS( pdwCurToken[1] );
                hr = pShaderDesc->AddInlineConstsI( dwRegNum, reinterpret_cast<const INT*>( &pdwCurToken[2] ) );
                if ( FAILED( hr ) )
                {
                    SHADER_CONV_ASSERT(!"CShaderDesc::AddInlineConstsI() failed, hr = %d\n");
                    goto L_ERROR;
                }
                pdwCurToken += uiLength;
            }
            continue;

        case D3DSIO_DEFB:
            {
                const DWORD dwRegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS( pdwCurToken[1] );
                hr = pShaderDesc->AddInlineConstsB( dwRegNum, pdwCurToken[2] );
                if ( FAILED( hr ) )
                {
                    SHADER_CONV_ASSERT(!"CShaderDesc::AddInlineConstsB() failed, hr = %d\n");
                    goto L_ERROR;
                }
                pdwCurToken += uiLength;
            }
            continue;
        }

        *pdwCurInstr++ = *pdwCurToken++;

        // Check if the intruction has parameters
        if ( *pdwCurToken & ( 1L << 31 ) )
        {
            // Parse destination parameter
            switch ( opCode )
            {
            case D3DSIO_REP:
            case D3DSIO_LOOP:

                // Allocate loop registers
                ++uiNumLoopRegs;

            case D3DSIO_CALL:
            case D3DSIO_CALLNZ:
            case D3DSIO_IF:
            case D3DSIO_IFC:
            case D3DSIO_BREAK:
            case D3DSIO_BREAKC:
            case D3DSIO_BREAKP:
            case D3DSIO_LABEL:
            case D3DSIO_TEXKILL:
                // No destination parameter
                break;

            case D3DSIO_BEM:
            case D3DSIO_TEXBEM:
            case D3DSIO_TEXBEML:
            case D3DSIO_TEX:
            case D3DSIO_TEXREG2AR:
            case D3DSIO_TEXREG2GB:
            case D3DSIO_TEXREG2RGB:
            case D3DSIO_TEXM3x2TEX:
            case D3DSIO_TEXM3x3TEX:
            case D3DSIO_TEXM3x3SPEC:
            case D3DSIO_TEXM3x3VSPEC:
            case D3DSIO_TEXM3x2DEPTH:
            case D3DSIO_TEXDP3TEX:
            case D3DSIO_TEXDEPTH:

                switch ( opCode )
                {
                case D3DSIO_TEXM3x2DEPTH:
                case D3DSIO_TEXDEPTH:
                    // These instructions implicitly write out to the depth register
                    outputRegs.oDepth = ubNumTempRegs++;
                    break;

                case D3DSIO_BEM:
                case D3DSIO_TEXBEM:
                case D3DSIO_TEXBEML:
                    // These instructions implicitely use bump environment materials
                    usageFlags.BumpEnvMat = 1;
                    if ( D3DSIO_BEM == opCode )
                    {
                        break;
                    }

                default:
                    if ( dwVersion < D3DPS_VERSION(2,0) )
                    {
                        // These instructions implicitly use texture resources & samplers
                        const DWORD dwRegNum = D3DSI_GETREGNUM( pdwCurToken[0] );
                        SHADER_CONV_ASSERT( dwRegNum < MAX_SAMPLER_REGS );
                        inputRegs.s[dwRegNum] = TEXTURETYPE_UNKNOWN;
                    }
                    break;
                }
                __fallthrough;

            default:

                const DWORD dwDstToken = *pdwCurToken++;
                *pdwCurInstr++ = dwDstToken;

                const DWORD dwRegNum = D3DSI_GETREGNUM( dwDstToken );
                const D3DSHADER_PARAM_REGISTER_TYPE regType = D3DSI_GETREGTYPE( dwDstToken );

                switch ( regType )
                {
                case D3DSPR_SAMPLER:
                    // Already processed in D3DSIO_DCL.
                    SHADER_CONV_ASSERT( inputRegs.s[dwRegNum] != INVALID_INDEX );
                    break;

                case D3DSPR_COLOROUT:
                    SHADER_CONV_ASSERT( dwRegNum < MAX_PS_COLOROUT_REGS );
                    if ( INVALID_INDEX == outputRegs.oC[dwRegNum] )
                    {
                        outputRegs.oC[dwRegNum] = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_DEPTHOUT:
                    if ( INVALID_INDEX == outputRegs.oDepth )
                    {
                        outputRegs.oDepth = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_TEMP:
                    SHADER_CONV_ASSERT( dwRegNum < MAX_PS_TEMP_REGS );
                    if ( INVALID_INDEX == inputRegs.r[dwRegNum] )
                    {
                        inputRegs.r[dwRegNum] = ubNumTempRegs++;
                    }

                    if ( 0 == dwRegNum &&
                         dwVersion < D3DPS_VERSION(2,0) &&
                         INVALID_INDEX == outputRegs.oC[0] )
                    {
                        // In PS 1.x, r0 is also used as the ouput color register
                        outputRegs.oC[0] = inputRegs.r[0];
                    }
                    break;

                case D3DSPR_PREDICATE:
                    if ( INVALID_INDEX == inputRegs.p0 )
                    {
                        inputRegs.p0 = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_TEXTURE:

                    switch ( opCode )
                    {
                    case D3DSIO_TEXM3x2PAD:
                    case D3DSIO_TEXM3x3PAD:
                        // These are speudo instructions use
                        // a system register for destination.
                        break;

                    default:
                        SHADER_CONV_ASSERT( dwVersion <= D3DPS_VERSION(1,3) );
                        SHADER_CONV_ASSERT( dwRegNum < MAX_PS1X_TEXTURE_REGS );
                        inputRegs._t[dwRegNum] = ubNumTempRegs++;
                        break;
                    }

                    switch ( opCode )
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
                        // ps_1_x doesn't support dcl's, need to add it explicitly
                        if ( 0 == ( inputDecls.TexCoords & ( 1 << dwRegNum ) ) )
                        {
                            // Input texture coordinates register declaration
                            inputDecls.AddDecl( D3DDECLUSAGE_TEXCOORD, dwRegNum, VSOREG_TexCoord0 + dwRegNum, D3DSP_WRITEMASK_ALL );

                            // Enable texture coordinates custom projection
                            usageFlags.ProjectedTCsMask |= ( 1 << dwRegNum );
                        }
                        break;
                    }
                    break;

                default:
                    SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, invalid register type: %d\n");
                    hr = E_FAIL;
                    goto L_ERROR;
                }
            }

            // Skip the destination write predicate
            if ( dwInstr & D3DSHADER_INSTRUCTION_PREDICATED )
            {
                *pdwCurInstr++ = *pdwCurToken++;
            }

            // Parse source parameters
            while ( *pdwCurToken & ( 1L << 31 ) )
            {
                const DWORD dwSrcToken = *pdwCurToken++;
                *pdwCurInstr++ = dwSrcToken;

                const DWORD dwRegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS( dwSrcToken );
                const D3DSHADER_PARAM_REGISTER_TYPE regType = D3DSI_GETREGTYPE_RESOLVING_CONSTANTS( dwSrcToken );
                const BOOL hasRelativeAddress = ( D3DSI_GETADDRESSMODE( dwSrcToken ) & D3DSHADER_ADDRMODE_RELATIVE );

                switch ( regType )
                {
                case D3DSPR_SAMPLER:
                    // Already processed in D3DSIO_DCL.
                    SHADER_CONV_ASSERT( dwRegNum < MAX_PS_SAMPLER_REGS );
                    SHADER_CONV_ASSERT( inputRegs.s[dwRegNum] != INVALID_INDEX );
                    break;

                case D3DSPR_TEMP:
                    SHADER_CONV_ASSERT( dwRegNum < MAX_PS_TEMP_REGS );
                    if ( INVALID_INDEX == inputRegs.r[dwRegNum] )
                    {
                        inputRegs.r[dwRegNum] = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_PREDICATE:
                    if ( INVALID_INDEX == inputRegs.p0 )
                    {
                        inputRegs.p0 = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_LABEL:
                case D3DSPR_MISCTYPE:
                    break;

                case D3DSPR_INPUT:
                    if ( dwVersion >= D3DPS_VERSION(3,0) )
                    {
                        if ( hasRelativeAddress )
                        {
                            usageFlags.IndexableInputs = 1;
                        }
                    }
                    else
                    if ( dwVersion < D3DPS_VERSION(2,0) )
                    {
                        // ps_1_x doesn't support dcl's, add it manually
                        if ( 0 == ( inputDecls.Colors & ( 1 << dwRegNum ) ) )
                        {
                            // Input color register declaration
                            inputDecls.AddDecl( D3DDECLUSAGE_COLOR, dwRegNum, VSOREG_Color0 + dwRegNum, D3DSP_WRITEMASK_ALL );
                        }
                    }
                    break;

                case D3DSPR_TEXTURE:
                    if ( dwVersion <= D3DPS_VERSION(1,3) &&
                         D3DSIO_TEXKILL != opCode )
                    {
                        // Should have been already defined in previous destination registers.
                        SHADER_CONV_ASSERT( dwRegNum < MAX_PS1X_TEXTURE_REGS );
                        SHADER_CONV_ASSERT( inputRegs._t[dwRegNum] != INVALID_INDEX );
                    }
                    else
                    if ( dwVersion < D3DPS_VERSION(2,0) )
                    {
                        // ps_1_4 doesn't support dcl's, need to add it explicitly.
                        if ( 0 == ( inputDecls.TexCoords & ( 1 << dwRegNum ) ) )
                        {
                            // Input texture coordinates register declaration
                            inputDecls.AddDecl( D3DDECLUSAGE_TEXCOORD, dwRegNum, VSOREG_TexCoord0 + dwRegNum, D3DSP_WRITEMASK_ALL );
                        }
                    }
                    break;

                case D3DSPR_CONST:
                    if ( hasRelativeAddress )
                    {
                        hasRelativeAddressF = true;
                    }
                    for ( UINT i = dwRegNum, n = i + __getConstRegSize( opCode ); i < n; ++i )
                    {
                        if ( hasRelativeAddressF ||
                             !bInlineConstsEnabled ||
                             !pShaderDesc->FindInlineConstant( CB_FLOAT, i * 4 ) )
                        {
                            __mincountof( minUsedConstantsF, i );
                            __maxcountof( maxUsedConstantsF, i + 1 );
                        }
                    }
                    break;

                case D3DSPR_CONSTINT:
                    if ( !bInlineConstsEnabled ||
                         !pShaderDesc->FindInlineConstant( CB_INT, dwRegNum * 4 ) )
                    {
                        __mincountof( minUsedConstantsI, dwRegNum );
                        __maxcountof( maxUsedConstantsI, dwRegNum + 1 );
                    }
                    break;

                case D3DSPR_CONSTBOOL:
                    if ( !bInlineConstsEnabled ||
                         !pShaderDesc->FindInlineConstant( CB_BOOL, dwRegNum ) )
                    {
                        __mincountof( minUsedConstantsB, dwRegNum );
                        __maxcountof( maxUsedConstantsB, dwRegNum + 1 );
                    }
                    break;

                case D3DSPR_LOOP:
                    if ( INVALID_INDEX == inputRegs.aL )
                    {
                       inputRegs.aL = ubNumTempRegs++;
                    }
                    break;

                default:
                    SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, invalid register type: %d\n");
                    hr = E_FAIL;
                    goto L_ERROR;
                }
            }
        }
    }

    if (IsImplicitFogCalculationNeeded(rasterStates, m_runtimeVersion) && rasterStates.FogTableMode != D3DFOG_NONE)
    {
        inputDecls.AddDecl(D3DDECLUSAGE_POSITION, 0, INVALID_INDEX, D3DSP_WRITEMASK_ALL);
    }

    *pdwCurInstr++ = D3DPS_END();

    for (UINT i = 0; i < MAX_PS_COLOROUT_REGS; i++)
    {
        if (outputRegs.oC[i] != INVALID_INDEX)
        {
            pShaderDesc->AddOutputRegisters(i);
        }
    }

    if (outputRegs.oDepth != INVALID_INDEX)
    {
        pShaderDesc->AddOutputDepthRegister();
    }


    // Copy shader instructions
#if DBG
    hr = pShaderDesc->CopyInstructions( pSrcBytes, cbCodeSize );
#else
    hr = pShaderDesc->CopyInstructions( pdwInstrs, (UINT)( pdwCurInstr - pdwInstrs ) * sizeof( DWORD ) );
#endif
    if ( FAILED( hr ) )
    {
        SHADER_CONV_ASSERT(!"CShaderDesc::CopyInstructions() failed, hr = %d\n");
        goto L_ERROR;
    }

    if (pdwInstrs) { delete[] pdwInstrs; }

    // Update the shader description
    pShaderDesc->SetVersion( dwVersion );
    pShaderDesc->SetUsageFlags( usageFlags );
    pShaderDesc->SetInputDecls( inputDecls );
    pShaderDesc->SetInputRegs( inputRegs );
    pShaderDesc->SetOutputRegs( outputRegs );
    pShaderDesc->SetMinUsedConsts( CB_FLOAT, minUsedConstantsF * 4 );
    pShaderDesc->SetMinUsedConsts( CB_INT, minUsedConstantsI * 4 );
    pShaderDesc->SetMinUsedConsts( CB_BOOL, minUsedConstantsB );
    pShaderDesc->SetMaxUsedConsts( CB_FLOAT, maxUsedConstantsF * 4 );
    pShaderDesc->SetMaxUsedConsts( CB_INT, maxUsedConstantsI * 4 );
    pShaderDesc->SetMaxUsedConsts( CB_BOOL, maxUsedConstantsB );
    pShaderDesc->SetRelAddrConsts( CB_FLOAT, hasRelativeAddressF );
    pShaderDesc->SetNumTempRegs( ubNumTempRegs );
    pShaderDesc->SetNumLoopRegs( uiNumLoopRegs );
    pShaderDesc->SetShaderSettings(shaderSettings);

    // Set the returned shader description
    *ppShaderDesc = pShaderDesc;

    return S_OK;

L_ERROR:

    if ( pdwInstrs )
    {
        delete[] pdwInstrs;
    }

    __safeRelease( pShaderDesc );

    return hr;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTranslator::TranslatePS( const CPixelShaderDesc* pShaderDesc,
                          const RasterStates& rasterStates,
                          const ShaderConv::VSOutputDecls& inputDecls,
                          CCodeBlob** ppCodeBlob )
{
    HRESULT hr;

    if ( NULL == pShaderDesc ||
         NULL == ppCodeBlob )
    {
        SHADER_CONV_ASSERT(!"CTranslator::TranslatePS() failed, invalid parameters\n");
        return E_INVALIDARG;
    }

#if DBG && WARP_INTERNAL

    // Disassemble input the shader code to the debug output
    hr = Disasm20( pShaderDesc->GetInstructions(), 0 );
    if ( FAILED( hr ) )
    {
        SHADER_CONV_ASSERT(!"Disasm20() failed, hr = %d\n");
        return hr;
    }

#endif

    // Create a new pixel shader context
    CPSContext* pContext = new CPSContext( m_runtimeVersion, pShaderDesc, rasterStates, inputDecls, m_pShaderAsm );
    if ( NULL == pContext )
    {
        return E_OUTOFMEMORY;
    }

    // Start the shader assembler
    // Note: Upgraded from SM 4.0 -> SM 5.0 because rcp is only supported on SM 5.0
    m_pShaderAsm->StartShader(D3D10_SB_PIXEL_SHADER, 5, 0, pShaderDesc->GetShaderSettings());

    // Write the shader declarations
    hr = pContext->WriteDeclarations();
    if ( FAILED( hr ) )
    {
        __safeDelete( pContext );
        SHADER_CONV_ASSERT(!"CPSContext::WriteDeclarations() failed, hr = %d\n");
        return hr;
    }

    // Translate instructions
    hr = pContext->TranslateInstructions();
    if ( FAILED( hr ) )
    {
        __safeDelete( pContext );
        SHADER_CONV_ASSERT(!"CPSContext::TranslateInstructions() failed, hr = %d\n");
        return hr;
    }

    // Write the ouput registers
    pContext->WriteOutputs();

    // Delete the context
    __safeDelete( pContext );

    // End the shader assembler
    hr = m_pShaderAsm->EndShader();
    if ( FAILED( hr ) )
    {
        SHADER_CONV_ASSERT(!"CShaderAsm::EndShader() failed, hr = %d\n");
        return hr;
    }

        // Create the code blob
    hr = CCodeBlob::Create( m_pShaderAsm->ShaderSizeInDWORDs() * sizeof( UINT ),
                            m_pShaderAsm->GetShader(),
                            ppCodeBlob );
    if ( FAILED( hr ) )
    {
        SHADER_CONV_ASSERT(!"CCodeBlob::Create() failed, hr = %d\n");
        return hr;
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
CPSContext::CPSContext( UINT runtimeVersion,
                        const CPixelShaderDesc* pShaderDesc,
                        const RasterStates& rasterStates,
                        const ShaderConv::VSOutputDecls& inputDecls,
                        CShaderAsmWrapper* pShaderAsm) :
    CContext( runtimeVersion, pShaderDesc, rasterStates, pShaderAsm ),
    m_inputDecls( inputDecls ),
    m_usageFlags( pShaderDesc->GetUsageFlags() ),
    m_pInputFog( NULL ),
    m_pInputSpecular( NULL ),
    m_positionRegister(pShaderDesc->GetPositionRegister())
{
    //--
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CPSContext::WriteDeclarations()
{
    // Set the next loop register
    BYTE uiTotalTempRegs = m_pShaderDesc->GetNumTempRegs();

    // Store the loop registers base index
    m_nextLoopRegister = uiTotalTempRegs;

    // Add the loop registers count
    uiTotalTempRegs += m_pShaderDesc->GetNumLoopRegs();

    AllocateTempRegistersForUndeclaredInputs(m_inputDecls, uiTotalTempRegs);

    // Declare temp registers
    m_pShaderAsm->EmitTempsDecl( uiTotalTempRegs );

    // Declare floats constant buffers
    UINT nFloatConstRegs = m_pShaderDesc->GetMaxUsedConsts( ShaderConv::CB_FLOAT );
    if ( nFloatConstRegs )
    {
        if ( m_pShaderDesc->HasRelAddrConsts( ShaderConv::CB_FLOAT ) )
        {
            nFloatConstRegs = 0; // ie any size
        }

        m_pShaderAsm->EmitConstantBufferDecl(
            CB_FLOAT, nFloatConstRegs / 4, D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
    }

    // Declare intergers constant buffers
    const UINT nIntConstRegs = m_pShaderDesc->GetMaxUsedConsts( CB_INT );
    if ( nIntConstRegs )
    {
        m_pShaderAsm->EmitConstantBufferDecl(
            CB_INT, nIntConstRegs / 4, D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
    }

    // Declare booleans constant buffers
    const UINT nBoolConstRegs = m_pShaderDesc->GetMaxUsedConsts( CB_BOOL );
    if ( nBoolConstRegs )
    {
        m_pShaderAsm->EmitConstantBufferDecl(
            CB_BOOL, ( nBoolConstRegs + 3) / 4, D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
    }

    // Declare extension constant buffer
    if ( IsImplicitFogCalculationNeeded() ||
         m_rasterStates.AlphaTestEnable )
    {
        m_pShaderAsm->EmitConstantBufferDecl(
            CB_PS_EXT, __sizeof16( PSCBExtension ), D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
    }

    // Declare extension2 constant buffer
    if ( m_usageFlags.BumpEnvMat )
    {
        m_pShaderAsm->EmitConstantBufferDecl(
            CB_PS_EXT2, __sizeof16( PSCBExtension2 ), D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
    }

    // Declare extension3 constant buffer
    if ( m_rasterStates.ColorKeyEnable ||
         m_rasterStates.ColorKeyBlendEnable )
    {
        m_pShaderAsm->EmitConstantBufferDecl(
            CB_PS_EXT3, __sizeof16( PSCBExtension3 ), D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
    }

    // Declare pixel shader samplers
    for ( UINT i = 0; i < MAX_PS_SAMPLER_REGS; ++i )
    {
        TEXTURETYPE textureType = (TEXTURETYPE)m_inputRegs.s[i];
        if ( textureType != INVALID_INDEX )
        {
            if ( TEXTURETYPE_UNKNOWN == textureType )
            {
                textureType = (TEXTURETYPE)m_rasterStates.PSSamplers[i].TextureType;
            }

            // if reading from a specialized depth texture implicitly use a comparison 
            // sample for 'Hardware Shadow Maps'
            if (TextureUsesHardwareShadowMapping(i))
            {
                m_pShaderAsm->EmitSamplerDecl(i, D3D10_SB_SAMPLER_MODE_COMPARISON);
            }
            else
            {
                m_pShaderAsm->EmitSamplerDecl(i, D3D10_SB_SAMPLER_MODE_DEFAULT);
            }

            m_pShaderAsm->EmitResourceDecl( __toResourceDimension10( textureType ),
                                            i,
                                            D3D10_SB_RETURN_TYPE_FLOAT,
                                            D3D10_SB_RETURN_TYPE_FLOAT,
                                            D3D10_SB_RETURN_TYPE_FLOAT,
                                            D3D10_SB_RETURN_TYPE_FLOAT );
        }
    }

    BYTE declaredRegisterMasks[16] = { 0 };

    bool bDeclInputPosition = false;
    // Declare input registers
    for (UINT i = 0, n = m_inputDecls.GetSize(); i < n; ++i)
    {
        // Check if the input register is available        
        if (m_inputDecls[i].WriteMask)
        {
            const UINT usage      = m_inputDecls[i].Usage;
            const UINT usageIndex = m_inputDecls[i].UsageIndex;        
            const UINT regIndex   = m_inputDecls[i].RegIndex;
            const UINT writeMask  = m_inputDecls[i].WriteMask;
            const bool bCentroid  = ((m_inputDecls.CentroidMask >> i) & 0x1) ? true : false;

            if ((D3DDECLUSAGE_POSITION == usage) ||
                (D3DDECLUSAGE_VPOS == usage && 0 == usageIndex))
            {
                bDeclInputPosition = true;
            }
            else
            if ( D3DDECLUSAGE_VFACE == usage &&
                 0 == usageIndex )
            {
                m_pShaderAsm->EmitPSInputSystemGeneratedValueDecl(
                    PSIREG_VFace, writeMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT, D3D10_SB_INTERPOLATION_CONSTANT, D3D10_SB_NAME_IS_FRONT_FACE );
            }
            else
            {
                // Ensure that there is no duplicate declarations
                __assume(regIndex < _countof(declaredRegisterMasks));
                if (0 ==(declaredRegisterMasks[regIndex] & writeMask))
                {                    
                    declaredRegisterMasks[regIndex] |= writeMask;

                    D3D10_SB_INTERPOLATION_MODE interpolation;

                    if ((D3DDECLUSAGE_COLOR == usage) 
                     && (0 == usageIndex)
                     && (D3DSHADE_FLAT == m_rasterStates.ShadeMode))
                    {
                        interpolation = D3D10_SB_INTERPOLATION_CONSTANT;
                    }
                    else
                    if (bCentroid)
                    {
                        interpolation = D3D10_SB_INTERPOLATION_LINEAR_CENTROID;
                    }
                    else
                    {
                        interpolation = D3D10_SB_INTERPOLATION_LINEAR;
                    }

                    if (D3DDECLUSAGE_FOG == usage)
                    {
                        m_pInputFog = &m_inputDecls[i];
                    }
                    else
                    if ((D3DDECLUSAGE_COLOR == usage) 
                     && (1 == usageIndex))
                    {
                        m_pInputSpecular = &m_inputDecls[i];
                    }

                    m_pShaderAsm->EmitPSInputDecl(regIndex, writeMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT, interpolation);
                }
            }
        }
    }

    if (m_usageFlags.IndexableInputs)
    {
        // Declare input registers indexing range
        for (UINT baseReg = 0; baseReg < MAX_PS_INPUT_REGS; ++baseReg)
        {
            UINT regCount = 0;
            while (baseReg + regCount < MAX_PS_INPUT_REGS && declaredRegisterMasks[baseReg + regCount] != 0)
            {
                ++regCount;
            }
            if (regCount > 0)
            {
                m_pShaderAsm->EmitInputIndexingRangeDecl(
                    baseReg, regCount, D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL);
            }
            baseReg += regCount;
        }
    }

    if (bDeclInputPosition)
    {
        m_pShaderAsm->EmitPSInputSystemInterpretedValueDecl(
            m_positionRegister,
            D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL,
            D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE,
            D3D10_SB_NAME_POSITION);
    }

    // Declare color ouput registers
    for ( UINT i = 0; i < MAX_PS_COLOROUT_REGS; ++i )
    {
        if ( m_outputRegs.oC[i] != INVALID_INDEX )
        {
            m_pShaderAsm->EmitOutputDecl( i, D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL );
        }
    }

    // Declare depth ouput registers
    if ( m_outputRegs.oDepth != INVALID_INDEX )
    {
        m_pShaderAsm->EmitODepthDecl();
    }

    InitializeTempRegistersForUndeclaredInputs(m_inputDecls, uiTotalTempRegs);

    if ( m_inputRegs.v[PSIREG_VPos].Reg() != INVALID_INDEX && m_version >= D3DPS_VERSION(3, 0))
    {
        // Convert the position register value to integral float
        // round_ni t#, v#

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ROUND_NI,
                          CTempOperandDst( m_inputRegs.v[PSIREG_VPos].Reg(), __WRITEMASK_XY ),
                          CInputOperand4(m_positionRegister, __SWIZZLE_XY ) ) );
    }

    if ( m_inputRegs.v[PSIREG_VFace].Reg() != INVALID_INDEX )
    {
        // Convert the face register boolean value to float
        // movc t#, v#, 1.0f, -1.0f

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOVC,
                          CTempOperandDst( m_inputRegs.v[PSIREG_VFace].Reg() ),
                          CInputOperand4( PSIREG_VFace, __SWIZZLE_X ),
                          COperand( 1.0f ),
                          COperand(-1.0f ) ) );
    }

    return S_OK;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CPSContext::WriteOutputs()
{
    if ( m_bOutputWritten )
    {
        return;
    }

    if ( m_version < D3DPS_VERSION(2,0) )
    {
        // In PS 1.x, clamp the output color to [0..1]
        for ( UINT i = 0; i < MAX_PS_COLOROUT_REGS; ++i )
        {
            const UINT regIndex = m_outputRegs.oC[i];
            if ( regIndex != INVALID_INDEX )
            {
                m_pShaderAsm->EmitInstruction(
                    CInstructionEx( D3D10_SB_OPCODE_MOV,
                                    true,
                                    CTempOperandDst( regIndex ),
                                    CTempOperand4( regIndex ) ) );
            }
        }
    }

    //
    // Apply Fixed function extensions
    //

    if ( m_rasterStates.AlphaTestEnable &&
         D3DCMP_ALWAYS != m_rasterStates.AlphaFunc )
    {
        // Do alpha test
        this->ComputeAlphaTest();
    }

    if ( IsImplicitFogCalculationNeeded() )
    {
        // Do fog blend
        this->ComputeFogBlend();
    }

    //
    // Output color registers
    //

    for ( UINT i = 0; i < MAX_PS_COLOROUT_REGS; ++i )
    {
        const UINT regIndex = m_outputRegs.oC[i];
        if ( regIndex != INVALID_INDEX )
        {
            UINT swizzle = (m_rasterStates.SwapRBOnOutputMask & (1 << i))
                ? __SWIZZLE4(D3D10_SB_4_COMPONENT_Z,
                             D3D10_SB_4_COMPONENT_Y,
                             D3D10_SB_4_COMPONENT_X,
                             D3D10_SB_4_COMPONENT_W)
                : __SWIZZLE_ALL;
            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_MOV,
                              COperandDst( D3D10_SB_OPERAND_TYPE_OUTPUT, i ),
                              CTempOperand4( regIndex, swizzle ) ) );
        }
    }

    //
    // Output depth register
    //

    if ( m_outputRegs.oDepth != INVALID_INDEX )
    {
        auto depthOperand = CTempOperand4(m_outputRegs.oDepth);
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          COperandDst( D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH ),
                          CSingleComponent(depthOperand, D3D10_SB_4_COMPONENT_X) ) );
    }

    m_bOutputWritten = true;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CPSContext::ComputeAlphaTest()
{
    if ( D3DCMP_NEVER == m_rasterStates.AlphaFunc )
    {
        // discard_z 0.0f
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_DISCARD,
                          COperand( 0.0f ),
                          D3D10_SB_INSTRUCTION_TEST_ZERO ) );
    }
    else
    {
        const UINT colorIndex = m_outputRegs.oC[0];
        if ( colorIndex != INVALID_INDEX )
        {
            // <comp> s0.x, tC[0].w, cb3.alpharef.w
            // discard_z s0.x

            D3DSHADER_COMPARISON compare;

            switch ( m_rasterStates.AlphaFunc )
            {
            default:
                NO_DEFAULT;

            case D3DCMP_LESS:
                compare = D3DSPC_LT;
                break;

            case D3DCMP_EQUAL:
                compare = D3DSPC_EQ;
                break;

            case D3DCMP_LESSEQUAL:
                compare = D3DSPC_LE;
                break;

            case D3DCMP_GREATER:
                compare = D3DSPC_GT;
                break;

            case D3DCMP_NOTEQUAL:
                compare = D3DSPC_NE;
                break;

            case D3DCMP_GREATEREQUAL:
                compare = D3DSPC_GE;
                break;
            }

            // Generate compare instructions
            this->Compare( compare,
                           CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                           CTempOperand4( colorIndex, __SWIZZLE_W ),
                           CCBOperand2D( CB_PS_EXT,
                                         PSCBExtension::ALPHATEST_W,
                                         __SWIZZLE_W ) );

            // Execute the alpha test
            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_DISCARD,
                              CTempOperand1( SREG_TMP0, D3D10_SB_4_COMPONENT_X),
                              D3D10_SB_INSTRUCTION_TEST_ZERO ) );
        }
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CPSContext::ComputeFogBlend()
{
    assert(IsImplicitFogCalculationNeeded());
    const UINT colorIndex = m_outputRegs.oC[0];
    if ( colorIndex != INVALID_INDEX )
    {
        if ( m_rasterStates.FogTableMode != D3DFOG_NONE )
        {
            // Execute pixel fog
            this->ComputePixelFog();
        }
        else
        {
            if ( m_pInputFog )
            {
                // mov_sat s0.w, fog.mask
                m_pShaderAsm->EmitInstruction(
                    CInstructionEx( D3D10_SB_OPCODE_MOV,
                                    true,
                                    CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                                    CInputOperand4( m_pInputFog->RegIndex, __swizzleFromWriteMask( m_pInputFog->WriteMask ) ) ) );
            }
            else
            if ( m_pInputSpecular )
            {
                // mov_sat s0.w, specular.w
                m_pShaderAsm->EmitInstruction(
                    CInstructionEx( D3D10_SB_OPCODE_MOV,
                                    true,
                                    CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                                    CInputOperand4( m_pInputSpecular->RegIndex, __SWIZZLE_W ) ) );
            }
            else
            {
                // No input fog is available, set the fog factor to Zero
                // mov_sat s0.w, 0.0f
                m_pShaderAsm->EmitInstruction(
                    CInstructionEx( D3D10_SB_OPCODE_MOV,
                                    true,
                                    CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                                    COperand( 0.0f ) ) );
            }
        }

        // add s0.xyz, tC[0], -cb3.fogcolor.xyz
        // mad tC[0].xyz, s0, s0.w, cb3.fogcolor.xyz

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ADD,
                          CTempOperandDst( SREG_TMP0, __WRITEMASK_XYZ ),
                          CTempOperand4( colorIndex ),
                          CNegate( CCBOperand2D( CB_PS_EXT, PSCBExtension::FOGCOLOR_XYZ, __SWIZZLE_XYZ ) ) ) );

         m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MAD,
                          CTempOperandDst( colorIndex, __WRITEMASK_XYZ ),
                          CTempOperand4( SREG_TMP0 ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                          CCBOperand2D( CB_PS_EXT, PSCBExtension::FOGCOLOR_XYZ, __SWIZZLE_XYZ ) ) );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CPSContext::ComputePixelFog()
{
    assert(IsImplicitFogCalculationNeeded());
    
    // Get the fog index value
    const COperandBase srcFogIndex = ( m_rasterStates.WFogEnable ) ?
        CInputOperand4(m_positionRegister, __SWIZZLE_W) :
        CInputOperand4(m_positionRegister, __SWIZZLE_Z);

    switch ( m_rasterStates.FogTableMode )
    {
    default:
        NO_DEFAULT;

    case D3DFOG_LINEAR:

        // ge s0.x, fogIndex, cb3.fogEnd
        // if s0.x
        //   mov s0.w, vec4(0.0f)
        // else
        //   ge s0.x, cb3.fogStart, fogIndex
        //   if s0.x
        //     mov s0.w, vec4(1.0f)
        //   else
        //     add s0.w, cb3.fogEnd, -fogIndex
        //     mul s0.w, s0.w, cb3.fogDistInv
        //   endif
        // endif

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_GE,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          srcFogIndex,
                          CCBOperand2D( CB_PS_EXT, PSCBExtension::FOGEND_Y, __SWIZZLE_Y ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_IF,
                          CTempOperand1( SREG_TMP0, D3D10_SB_4_COMPONENT_X),
                          D3D10_SB_INSTRUCTION_TEST_NONZERO ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          COperand( 0.0f ) ) );

        m_pShaderAsm->EmitInstruction( CInstruction( D3D10_SB_OPCODE_ELSE ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_GE,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CCBOperand2D( CB_PS_EXT, PSCBExtension::FOGSTART_X, __SWIZZLE_X ),
                          srcFogIndex ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_IF,
                          CTempOperand1( SREG_TMP0, D3D10_SB_4_COMPONENT_X),
                          D3D10_SB_INSTRUCTION_TEST_NONZERO ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          COperand( 1.0f ) ) );

        m_pShaderAsm->EmitInstruction( CInstruction( D3D10_SB_OPCODE_ELSE ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_ADD,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          CCBOperand2D( CB_PS_EXT, PSCBExtension::FOGEND_Y, __SWIZZLE_Y ),
                          CNegate( srcFogIndex ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MUL,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_W ),
                          CCBOperand2D( CB_PS_EXT, PSCBExtension::FOGDISTINV_Z, __SWIZZLE_Z ) ) );

        m_pShaderAsm->EmitInstruction( CInstruction( D3D10_SB_OPCODE_ENDIF ) );
        m_pShaderAsm->EmitInstruction( CInstruction( D3D10_SB_OPCODE_ENDIF ) );

        break;

    case D3DFOG_EXP:

        // in D3D10, EXP(x) instruction is actually EXP2( x ) = 2^x.
        // EXP( x ) = EXP2( x * LOG2( e ) )
        // mul s0.x, fogIndex, cb3.fogDensity
        // mul s0.x, s0.x, vec4(-1.44269504088896f)
        // exp s0.w, s0.x

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MUL,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          srcFogIndex,
                          CCBOperand2D( CB_PS_EXT, PSCBExtension::FOGDENSITY_W, __SWIZZLE_W ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MUL,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                          COperand( -1.44269504088896f ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_EXP,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

        break;

    case D3DFOG_EXP2:

        // in D3D10, EXP(x) instruction is actually EXP2( x ) = 2^x.
        // EXP( x ) = EXP2( x * LOG2( e ) )
        // mul s0.x, fogIndex, cb3.fogDensity
        // mul s0.x, s0.x, s0.x
        // mul s0.x, s0.x, vec4(-1.44269504088896f)
        // exp s0.w, s0.x

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MUL,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          srcFogIndex,
                          CCBOperand2D( CB_PS_EXT, PSCBExtension::FOGDENSITY_W, __SWIZZLE_W ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MUL,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MUL,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_X ),
                          COperand( -1.44269504088896f ) ) );

        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_EXP,
                          CTempOperandDst( SREG_TMP0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                          CTempOperand4( SREG_TMP0, __SWIZZLE_X ) ) );

        break;
    }
}

} // namespace ShaderConv