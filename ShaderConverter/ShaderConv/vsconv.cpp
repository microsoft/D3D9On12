// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Implementation for The Vertex Shader Converter
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
CVertexShaderDesc::Create( CVertexShaderDesc** ppVertexShaderDesc )
{
    SHADER_CONV_ASSERT( ppVertexShaderDesc );

    CVertexShaderDesc* const pVertexShaderDesc = new CVertexShaderDesc();
    if ( NULL == pVertexShaderDesc )
    {
        SHADER_CONV_ASSERT(!"CVertexShaderDesc() allocation failed, out of memory\n" );
        return E_OUTOFMEMORY;
    }

    pVertexShaderDesc->AddRef();
    *ppVertexShaderDesc = pVertexShaderDesc;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTranslator::AnalyzeVS( const void* pSrcBytes,
                        UINT cbCodeSize,
                        UINT shaderSettings,
                        const RasterStates& rasterStates,
                        const VSInputDecls *pReferenceInputDecls,
                        CVertexShaderDesc** ppShaderDesc )
{
    HRESULT hr;

    if ( NULL == pSrcBytes ||
         NULL == ppShaderDesc ||
         0 == cbCodeSize )
    {
        SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, invalid parameters\n");
        return E_INVALIDARG;
    }

    // Get the shader version
    const DWORD* const pdwCodeBytes = reinterpret_cast<const DWORD*>( pSrcBytes );
    const DWORD dwVersion = *pdwCodeBytes;

    switch ( dwVersion )
    {
    case D3DVS_VERSION(1,0):
    case D3DVS_VERSION(1,1):
    case D3DVS_VERSION(1,2):
    case D3DVS_VERSION(1,3):
    case D3DVS_VERSION(1,4):
    case D3DVS_VERSION(2,0):
    case D3DVS_VERSION(2,1):
    case D3DVS_VERSION(3,0):
        break;

    default:
        SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, shader version not supported: 0x%x\n");
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

    VSInputDecls  inputDecls(MAX_VS_INPUT_REGS);
    VSOutputDecls outputDecls;
    InputRegs     inputRegs;
    OutputRegs    outputRegs;
    VSUsageFlags  usageFlags;

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
    CVertexShaderDesc* pShaderDesc;
    hr = CVertexShaderDesc::Create( &pShaderDesc );
    if ( FAILED( hr ) )
    {
        SHADER_CONV_ASSERT(!"CVertexShaderDesc::Create() failed, hr = %d\n");
        return hr;
    }

    // Allocate instructions buffer
    DWORD* pdwInstrs = new DWORD [cbCodeSize];
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
    while ( *pdwCurToken != D3DVS_END() )
    {
        if ( static_cast<UINT>( reinterpret_cast<const BYTE*>( pdwCurToken ) -
                                reinterpret_cast<const BYTE*>( pdwCodeBytes ) ) > cbCodeSize )
        {
            SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, invalid shader\n");
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
                    {
                        // Input registers declaration
                        const UINT usage      = D3DSI_GETUSAGE( dwDclDesc );
                        const UINT usageIndex = D3DSI_GETUSAGEINDEX( dwDclDesc );
                        const VSInputDecl *pDecl = pReferenceInputDecls->FindInputDecl(usage, usageIndex);
                        if (pDecl)
                        {
                            inputDecls.AddDecl(usage, usageIndex, dwRegNum, pDecl->IsTransformedPosition, pDecl->InputConversion);
                        }
                        else
                        {
                            inputDecls.AddDecl(usage, usageIndex, dwRegNum);
                        }
                    }
                    break;

                case D3DSPR_TEXCRDOUT:
                    {
                        // Output registers declaration
                        const UINT usage      = D3DSI_GETUSAGE( dwDclDesc );
                        const UINT usageIndex = D3DSI_GETUSAGEINDEX( dwDclDesc );
                        const UINT writeMask  = D3DSI_GETWRITEMASK( dwDclReg );
                        outputDecls.AddDecl( usage, usageIndex, dwRegNum, writeMask );

                        // Assign output register
                        SHADER_CONV_ASSERT( dwRegNum < MAX_VS_OUTPUT_REGS );
                        if ( INVALID_INDEX == outputRegs.O[dwRegNum] )
                        {                            
                            outputRegs.O[dwRegNum] = ubNumTempRegs++;
                        }
                    }
                    break;

                case D3DSPR_SAMPLER:
                    {
                        // Texture sampler declaration
                        SHADER_CONV_ASSERT( dwRegNum < MAX_VS_SAMPLER_REGS );
                        inputRegs.s[dwRegNum] =
                            (BYTE)__toTextureType( (D3DSAMPLER_TEXTURE_TYPE)( dwDclDesc & D3DSP_TEXTURETYPE_MASK ) );
                    }
                    break;

                default:
                    SHADER_CONV_ASSERT(!"CShaderDesc::CTranslator::AnalyzeVS() failed, invalid dcl instruction type: %d\n");
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

            default:

                // Parse destination parameter
                const DWORD dwDstToken = *pdwCurToken++;
                *pdwCurInstr++ = dwDstToken;

                const DWORD dwRegNum = D3DSI_GETREGNUM( dwDstToken );
                const D3DSHADER_PARAM_REGISTER_TYPE regType = D3DSI_GETREGTYPE( dwDstToken );
                const BOOL hasRelativeAddress = D3DSI_GETADDRESSMODE( dwDstToken ) & D3DSHADER_ADDRMODE_RELATIVE;

                switch ( regType )
                {
                case D3DSPR_RASTOUT:
                    switch ( dwRegNum )
                    {
                    case D3DSRO_POSITION:
                        if ( !outputDecls.Position )
                        {
                            outputDecls.AddDecl( D3DDECLUSAGE_POSITION, 0, VSOREG_Position, D3DSP_WRITEMASK_ALL );
                        }

                        if ( INVALID_INDEX == outputRegs.O[VSOREG_Position] )
                        {
                            outputRegs.O[VSOREG_Position] = ubNumTempRegs++;
                        }
                        break;

                    case D3DSRO_FOG:
                        if ( !outputDecls.Fog )
                        {
                            outputDecls.AddDecl( D3DDECLUSAGE_FOG, 0, VSOREG_FogPSize, D3DSP_WRITEMASK_0 );
                        }

                        if ( INVALID_INDEX == outputRegs.O[VSOREG_FogPSize] )
                        {
                            outputRegs.O[VSOREG_FogPSize] = ubNumTempRegs++;
                        }
                        break;

                    case D3DSRO_POINT_SIZE:
                        if ( !outputDecls.PointSize )
                        {
                            outputDecls.AddDecl( D3DDECLUSAGE_PSIZE, 0, VSOREG_FogPSize, D3DSP_WRITEMASK_1 );
                        }

                        if ( INVALID_INDEX == outputRegs.O[VSOREG_FogPSize] )
                        {
                            outputRegs.O[VSOREG_FogPSize] = ubNumTempRegs++;
                        }
                        break;

                    default:
                        SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, invalid register number: %d\n");
                        hr = E_FAIL;
                        goto L_ERROR;
                    }
                    break;

                case D3DSPR_ATTROUT:
                    {
                        const UINT regIndex = VSOREG_Color0 + dwRegNum;
                        if ( 0 == ( outputDecls.Colors & ( 1 << dwRegNum ) ) )
                        {
                            outputDecls.AddDecl( D3DDECLUSAGE_COLOR, dwRegNum, regIndex, D3DSP_WRITEMASK_ALL );
                        }

                        SHADER_CONV_ASSERT( dwRegNum < MAX_VS_COLOR_REGS );
                        if ( INVALID_INDEX == outputRegs.O[regIndex] )
                        {
                            outputRegs.O[regIndex] = ubNumTempRegs++;
                        }
                    }
                    break;

                case D3DSPR_TEXCRDOUT:
                    {
                        if ( dwVersion >= D3DVS_VERSION(3,0) )
                        {
                            // In VS 3.0, this register type is considered the default output register
                            // and can support addressing via the aL register
                            if ( hasRelativeAddress )
                            {
                                usageFlags.OutputRegsAddressing = 1;
                            }
                        }
                        else
                        {
                            const UINT regIndex = VSOREG_TexCoord0 + dwRegNum;

                            const DWORD dwWriteMask = D3DSI_GETWRITEMASK( dwDstToken );
                            outputDecls.AddDecl( D3DDECLUSAGE_TEXCOORD, dwRegNum, regIndex, dwWriteMask );

                            SHADER_CONV_ASSERT( dwRegNum < MAX_VS_TEXCOORD_REGS );
                            if ( INVALID_INDEX == outputRegs.O[regIndex] )
                            {
                                outputRegs.O[regIndex] = ubNumTempRegs++;
                            }
                        }
                    }
                    break;

                case D3DSPR_TEMP:
                    SHADER_CONV_ASSERT( dwRegNum < MAX_VS_TEMP_REGS );
                    if ( INVALID_INDEX == inputRegs.r[dwRegNum] )
                    {
                        inputRegs.r[dwRegNum] = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_ADDR:
                    if ( INVALID_INDEX == inputRegs.a0 )
                    {
                        inputRegs.a0 = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_PREDICATE:
                    if ( INVALID_INDEX == inputRegs.p0 )
                    {
                        inputRegs.p0 = ubNumTempRegs++;
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
                const BOOL hasRelativeAddress = D3DSI_GETADDRESSMODE( dwSrcToken ) & D3DSHADER_ADDRMODE_RELATIVE;

                switch ( regType )
                {
                case D3DSPR_SAMPLER:
                case D3DSPR_INPUT:
                     // Already processed in D3DSIO_DCL.
                    break;

                case D3DSPR_TEMP:
                    SHADER_CONV_ASSERT( dwRegNum < MAX_VS_TEMP_REGS );
                    if ( INVALID_INDEX == inputRegs.r[dwRegNum] )
                    {
                        inputRegs.r[dwRegNum] = ubNumTempRegs++;
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

                case D3DSPR_ADDR:
                    if ( INVALID_INDEX == inputRegs.a0 )
                    {
                       inputRegs.a0 = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_LOOP:
                    if ( INVALID_INDEX == inputRegs.aL )
                    {
                       inputRegs.aL = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_PREDICATE:
                    if ( INVALID_INDEX == inputRegs.p0 )
                    {
                        inputRegs.p0 = ubNumTempRegs++;
                    }
                    break;

                case D3DSPR_LABEL:
                    //--
                    break;

                default:
                    SHADER_CONV_ASSERT(!"CTranslator::AnalyzeVS() failed, invalid register type: %d\n");
                    hr = E_FAIL;
                    goto L_ERROR;
                }
            }
        }
    }

    *pdwCurInstr++ = D3DVS_END();

    if ( usageFlags.OutputRegsAddressing )
    {
        // Clear all allocated output registers
        for ( UINT i = 0; i < MAX_VS_OUTPUT_REGS; ++i )
        {
            outputRegs.O[i] = INVALID_INDEX;
        }
    }

    for (UINT i = 0, n = inputDecls.GetSize(); i < n; ++i)
    {
        const BYTE regIndex = inputDecls[i].RegIndex;
        const VSInputDecl *pDecl = pReferenceInputDecls->FindInputDecl(inputDecls[i].Usage, inputDecls[i].UsageIndex);
        if (pDecl)
        {
            if (inputDecls[i].InputConversion != VSInputDecl::None)
            {
                inputRegs.v[regIndex] = InputRegister(ubNumTempRegs++, InputRegister::Temp);
            }
            else
            {
                inputRegs.v[regIndex] = InputRegister(regIndex, InputRegister::Input);
            }
        }
    }

    // Check if user planes clipping is enabled
    if (rasterStates.UserClipPlanes)
    {
        // Declare clipping output registers
        this->DeclareClipplaneRegisters(outputDecls, rasterStates.UserClipPlanes);
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
    pShaderDesc->SetOutputDecls( outputDecls );
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
    pShaderDesc->SetShaderSettings( shaderSettings );

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
void
CTranslator::DeclareClipplaneRegisters(
    VSOutputDecls &outputDecls,
    UINT activeClipPlanesMask)
{
    UINT numClipPlanes = 0;

    // Calculate the number of active clip planes
    for (; activeClipPlanesMask; activeClipPlanesMask >>= 1)
    {
        if (activeClipPlanesMask & 1)
        {
            ++numClipPlanes;
        }
    }

    UINT writeMask = 0;

    // Build the register write mask
    switch (numClipPlanes % 4)
    {
    case 0: writeMask |= D3DSP_WRITEMASK_3;
    case 3: writeMask |= D3DSP_WRITEMASK_2;
    case 2: writeMask |= D3DSP_WRITEMASK_1;
    case 1: writeMask |= D3DSP_WRITEMASK_0;
    }

    if (numClipPlanes > 4)
    {
        outputDecls.AddDecl(D3DDECLUSAGE_CLIPDISTANCE, 0, VSOREG_ClipDist0, D3DSP_WRITEMASK_ALL);
        outputDecls.AddDecl(D3DDECLUSAGE_CLIPDISTANCE, 1, VSOREG_ClipDist1, writeMask);
    }
    else
    {
        outputDecls.AddDecl(D3DDECLUSAGE_CLIPDISTANCE, 0, VSOREG_ClipDist0, writeMask);
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTranslator::TranslateVS( const CVertexShaderDesc* pShaderDesc,
                          const RasterStates& rasterStates,
                          CCodeBlob** ppCodeBlob )
{
    HRESULT hr;

    if ( NULL == pShaderDesc ||
         NULL == ppCodeBlob )
    {
        SHADER_CONV_ASSERT(!"CTranslator::TranslateVS() failed, invalid parameters\n");
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

     // Create a new vertex shader context
    CVSContext* pContext = new CVSContext( m_runtimeVersion, pShaderDesc, rasterStates, m_pShaderAsm );
    if ( NULL == pContext )
    {
        return E_OUTOFMEMORY;
    }

    // Start the shader assembler
    // Note: Upgraded from SM 4.0 -> SM 5.0 because rcp is only supported on SM 5.0
    m_pShaderAsm->StartShader( D3D10_SB_VERTEX_SHADER, 5, 0, pShaderDesc->GetShaderSettings() );

    // Write the shader declarations
    hr = pContext->WriteDeclarations();
    if ( FAILED( hr ) )
    {
        __safeDelete( pContext );
        SHADER_CONV_ASSERT(!"CVSContext::WriteDeclarations() failed, hr = %d\n");
        return hr;
    }

    // Translate instructions
    hr = pContext->TranslateInstructions();
    if ( FAILED( hr ) )
    {
        __safeDelete( pContext );
        SHADER_CONV_ASSERT(!"CVSContext::TranslateInstructions() failed, hr = %d\n");
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
CVSContext::CVSContext( UINT runtimeVersion,
                        const CVertexShaderDesc* pShaderDesc,
                        const RasterStates& rasterStates,
                        CShaderAsmWrapper* pShaderAsm) :
    CContext( runtimeVersion, pShaderDesc, rasterStates, pShaderAsm ),
    m_usageFlags( pShaderDesc->GetUsageFlags() ),
    m_inputDecls( pShaderDesc->GetInputDecls() ),
    m_outputDecls( pShaderDesc->GetOutputDecls() )
{
    //--
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CVSContext::WriteDeclarations()
{
    BYTE ubNumTempRegs = m_pShaderDesc->GetNumTempRegs();

    // Set the next loop register
    m_nextLoopRegister = ubNumTempRegs;

    // Calculate the total number of allocated temp registers
    BYTE uiTotalTempRegs = ubNumTempRegs  + m_pShaderDesc->GetNumLoopRegs();

    AllocateTempRegistersForUndeclaredInputs(m_inputDecls, uiTotalTempRegs);

    // Declare temp registers
    m_pShaderAsm->EmitTempsDecl( uiTotalTempRegs );

    // Declare indexable temp registers
    if ( m_usageFlags.OutputRegsAddressing )
    {
        m_pShaderAsm->EmitIndexableTempDecl( 0, MAX_VS_OUTPUT_REGS, 4 );
    }

    const UINT nFloatConstRegs = m_pShaderDesc->GetMaxUsedConsts( ShaderConv::CB_FLOAT );
    const UINT nIntConstRegs = m_pShaderDesc->GetMaxUsedConsts( CB_INT );
    const UINT nBoolConstRegs = m_pShaderDesc->GetMaxUsedConsts( CB_BOOL );

    // Declare floats constant buffers
    if ( nFloatConstRegs )
    {
        if ( m_pShaderDesc->HasRelAddrConsts( ShaderConv::CB_FLOAT ) )
        {
            // Qualcomm has problem with 0 size, so set maximum possbile size.
            // Also, while bool/int has separate limits on SM2.x/3.x, conservatively
            // substract those from float limit as well.
            //
            // nFloatConstRegs = 0; // ie any size
            m_pShaderAsm->EmitConstantBufferDecl(
                CB_FLOAT, 
                MAX_VS_CONSTANTSF - ((nIntConstRegs / 4) + (nBoolConstRegs / 4) + __sizeof16( VSCBExtension )),
                D3D10_SB_CONSTANT_BUFFER_DYNAMIC_INDEXED );
        }
        else
        {
            m_pShaderAsm->EmitConstantBufferDecl(
                CB_FLOAT, nFloatConstRegs / 4, D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
        }
    }

    // Declare intergers constant buffers
    if ( nIntConstRegs )
    {
        m_pShaderAsm->EmitConstantBufferDecl(
            CB_INT, nIntConstRegs / 4, D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
    }

    // Declare booleans constant buffers
    if ( nBoolConstRegs )
    {
        m_pShaderAsm->EmitConstantBufferDecl(
            CB_BOOL, ( nBoolConstRegs + 3 ) / 4, D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );
    }

    // Declare extension constant buffer
    m_pShaderAsm->EmitConstantBufferDecl(
        CB_VS_EXT, __sizeof16( VSCBExtension ), D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );

    // Declare vertex shader samplers
    for ( UINT i = 0; i < MAX_VS_SAMPLER_REGS; ++i )
    {
        TEXTURETYPE textureType = (TEXTURETYPE)m_inputRegs.s[i];
        if ( textureType != INVALID_INDEX )
        {
            // if reading from a specialized depth texture implicitly use a comparison 
            // sample for 'Hardware Shadow Maps'
            if (m_rasterStates.HardwareShadowMappingRequiredVS & (1 << i))
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

    // Declare input registers
    for ( UINT i = 0, n = m_inputDecls.GetSize(); i < n; ++i )
    {
        const UINT regIndex = m_inputDecls[i].RegIndex;
        if (m_inputRegs.v[regIndex].WriteMask() != 0)
        {
            m_pShaderAsm->EmitInputDecl(regIndex, D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL);
        }
    }

    // Declare output registers
    for ( UINT i = 0, n = m_outputDecls.GetSize(); i < n; ++i )
    {
        const UINT usage      = m_outputDecls[i].Usage;
        const UINT usageIndex = m_outputDecls[i].UsageIndex;
        const UINT regIndex   = m_outputDecls[i].RegIndex;
        const UINT writeMask  = m_outputDecls[i].WriteMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;

        if ( D3DDECLUSAGE_POSITION == usage &&
             0 == usageIndex )
        {
            // Declare the position ouput register
            m_pShaderAsm->EmitOutputSystemInterpretedValueDecl( regIndex, writeMask, D3D10_SB_NAME_POSITION );
        }
        else if (D3DDECLUSAGE_CLIPDISTANCE == usage)
        {
            m_pShaderAsm->EmitOutputSystemInterpretedValueDecl(regIndex, writeMask, D3D10_SB_NAME_CLIP_DISTANCE);
        }
        else
        {
            m_pShaderAsm->EmitOutputDecl( regIndex, writeMask );
        }
    }

    InitializeTempRegistersForUndeclaredInputs(m_inputDecls, uiTotalTempRegs);

    // In DX9, the input assembler would automatically convert int inputs to
    // float since shaders didn't support ints. In DX10+, this has been removed
    // so we need to emulate the input assembler conversion with itof
    for (UINT i = 0, n = m_inputDecls.GetSize(); i < n; ++i)
    {
        if (m_inputDecls[i].InputConversion != VSInputDecl::None)
        {
            const UINT regIndex = m_inputDecls[i].RegIndex;
            SHADER_CONV_ASSERT(m_inputRegs.v[regIndex].Reg() != INVALID_INDEX);

            switch (m_inputDecls[i].InputConversion)
            {
            case VSInputDecl::NeedsIntToFloatConversion:
            case VSInputDecl::UDEC3:
            {
                m_pShaderAsm->EmitInstruction(
                    CInstruction(D3D10_SB_OPCODE_ITOF,
                    CTempOperandDst(m_inputRegs.v[regIndex].Reg()),
                    CInputOperand4(regIndex)));
                if (m_inputDecls[i].InputConversion == VSInputDecl::UDEC3)
                {
                    // UDEC3 should have 1 for the alpha regardless of what 2 bits were in the input stream
                    m_pShaderAsm->EmitInstruction(
                        CInstruction(D3D10_SB_OPCODE_MOV,
                        CTempOperandDst(m_inputRegs.v[regIndex].Reg(), D3D10_SB_OPERAND_4_COMPONENT_MASK_W),
                        COperand(1.0f)));
                }
            } break;
            case VSInputDecl::DEC3N:
            {
                // First, sign extend to get a proper integer instead of a UINT
                m_pShaderAsm->EmitInstruction(
                    CInstruction(D3D11_SB_OPCODE_IBFE,  // integer bitfield extract
                    CTempOperandDst(SREG_TMP0),
                    COperand(10, 10, 10, 0),            // width of 10 for each 10bit value, and write 0 into w
                    COperand(0, 0, 0, 0),               // offset of 0
                    CInputOperand4(regIndex)));

                // Next, convert to float
                m_pShaderAsm->EmitInstruction(
                    CInstruction(D3D10_SB_OPCODE_ITOF,
                    CTempOperandDst(SREG_TMP1),
                    CTempOperand4(SREG_TMP0)));

                // Divide by 2^9-1
                m_pShaderAsm->EmitInstruction(
                    CInstruction(D3D10_SB_OPCODE_DIV,
                    CTempOperandDst(SREG_TMP2),
                    CTempOperand4(SREG_TMP1),
                    COperand(511.0f, 511.0f, 511.0f, 1.0f)));

                // The 10bit int was in [-512, 511] range, so clamp this value into [-1, 1]
                // and make sure there's a 1 instead of a 0 in w.
                m_pShaderAsm->EmitInstruction(
                    CInstruction(D3D10_SB_OPCODE_MAX,
                    CTempOperandDst(m_inputRegs.v[regIndex].Reg()),
                    CTempOperand4(SREG_TMP2),
                    COperand(-1.0f, -1.0f, -1.0f, 1.0f)));
            } break;
            }
        }

    }

    return S_OK;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CVSContext::WriteOutputs()
{
    if ( m_bOutputWritten )
    {
        return;
    }

    // Write to output registers
    for ( UINT i = 0, n = m_outputDecls.GetSize(); i < n; ++i )
    {
        const UINT usage      = m_outputDecls[i].Usage;
        const UINT usageIndex = m_outputDecls[i].UsageIndex;
        const UINT regIndex   = m_outputDecls[i].RegIndex;
        const UINT writeMask  = m_outputDecls[i].WriteMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;

        COperandBase dstOperand;
        COperandBase srcOperand;

        if ( m_outputRegs.O[regIndex] != INVALID_INDEX )
        {
            dstOperand = CTempOperandDst( m_outputRegs.O[regIndex] );
            srcOperand = CTempOperand4( m_outputRegs.O[regIndex] );
        }
        else
        {
            dstOperand = CTempOperandDst2D( 0, regIndex );
            srcOperand = CTempOperand2D( 0, regIndex );
        }

        const COperandDst outputDst( D3D10_SB_OPERAND_TYPE_OUTPUT, regIndex, writeMask );

        bool bSaturate = false;
        bool bMovToOutputRegister = true;

        switch ( usage )
        {
#if 1 // start - pixel center workaround
        case D3DDECLUSAGE_POSITION:
            if ( 0 == usageIndex )
            {
                // If there is a clip plane, wsd e need to do the math before the half pixel offset,
                // otherwise the half pixel offset could potentially move the pixel from one side of 
                // the plane to the other.
                if (m_rasterStates.UserClipPlanes
                    && (D3DDECLUSAGE_POSITION == usage)
                    && (0 == usageIndex))
                {
                    // Output user-defined clipplane distances
                    this->WriteClipplanes(m_rasterStates.UserClipPlanes, srcOperand);
                }

                // Need to scale the XY coordinates by w/vp to math the D3D9 rasterization rule.
                // This is because D3D10 has a half-pixel offset to render target pixel centers
                // D3D9 screen space to D3D10 clipspace transformation:
                //
                // Xclip = ( Xscrn + 0.5 - ( VP.width/2 + VP.left ) ) * 2/VP.width * 1/Wscrn
                // Yclip = ( Yscrn + 0.5 - ( VP.height/2 + VP.top ) ) * -2/VP.height * 1/Wscrn
                // Zclip = ( Zscrn - VP.zmin ) * 2/VP.depth * 1/Wscrn
                // Wclip = 1/Wscrn
                //
                // Taking out the 0.5 factor out of the equation give the following equation:
                // Xclip = Xclip( D3D9 ) + Wclip/VP.width
                // Yclip = Yclip( D3D9 ) - Wclip/VP.height
                //
                // mad pos.xy, cb3[viewportscale].xy, pos.w, pos.xy
                m_pShaderAsm->EmitInstruction(
                    CInstruction(D3D10_SB_OPCODE_MAD,
                        CWriteMask( dstOperand, __WRITEMASK_XY ),
                        CCBOperand2D( CB_VS_EXT, VSCBExtension::VIEWPORTSCALE, __SWIZZLE_XY ),
                        CSwizzle( srcOperand, __SWIZZLE_W ),
                        CSwizzle( srcOperand, __SWIZZLE_XY ) ) );
            }
            break;
#endif // end - pixel center workaround
        case D3DDECLUSAGE_COLOR:
            bSaturate = ( m_version < D3DVS_VERSION(3,0) );
            break;

        case D3DDECLUSAGE_FOG:
            bSaturate = ( m_runtimeVersion < 9 );
            break;
        case D3DDECLUSAGE_CLIPDISTANCE:
            // Handled by WriteClipPlanes, required to be done before half pixel offset
            bMovToOutputRegister = false;
            break;
        }
        
        if (bMovToOutputRegister)
        {
            m_pShaderAsm->EmitInstruction(
                CInstructionEx(D3D10_SB_OPCODE_MOV, bSaturate, outputDst, srcOperand));
        }
    }

    m_bOutputWritten = true;
}

} // namespace ShaderConv
