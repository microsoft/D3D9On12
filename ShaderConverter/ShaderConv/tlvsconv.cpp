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
CTranslator::TranslateTLVS(
    const CTLVertexShaderDesc* pShaderDesc,
    CCodeBlob** ppCodeBlob
    )
{
    HRESULT hr;

    if ((NULL == pShaderDesc)
     || (NULL == ppCodeBlob))
    {
        SHADER_CONV_ASSERT(!"CTranslator::TranslateTLVS() failed, invalid parameters\n");
        return E_INVALIDARG;
    }

    // Create a new geometry shader context
    CTLVSContext* pContext = new CTLVSContext(m_runtimeVersion, pShaderDesc, m_pShaderAsm);
    if (NULL == pContext)
    {
        return E_OUTOFMEMORY;
    }

    // Start the shader assembler
    m_pShaderAsm->StartShader(D3D10_SB_VERTEX_SHADER, 5, 0, pShaderDesc->GetShaderSettings());

    // Write the shader declarations
    hr = pContext->WriteDeclarations();
    if (FAILED(hr))
    {
        __safeDelete(pContext);
        SHADER_CONV_ASSERT(!"CTLVSContext::WriteDeclarations() failed, hr = %d\n");
        return hr;
    }

    // Translate instructions
    hr = pContext->TranslateInstructions();
    if (FAILED(hr))
    {
        __safeDelete(pContext);
        SHADER_CONV_ASSERT(!"CTLVSContext::TranslateInstructions() failed, hr = %d\n");
        return hr;
    }

    // Write the ouput registers
    pContext->WriteOutputs();

    // Delete the context
    __safeDelete( pContext );

    // End the shader assembler
    hr = m_pShaderAsm->EndShader();
    if (FAILED(hr))
    {
        SHADER_CONV_ASSERT(!"CShaderAsm::EndShader() failed, hr = %d\n");
        return hr;
    }

    // Create the code blob
    hr = CCodeBlob::Create(
        m_pShaderAsm->ShaderSizeInDWORDs() * sizeof(UINT),
        m_pShaderAsm->GetShader(),
        ppCodeBlob
        );
    if (FAILED(hr))
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
CTLVertexShaderDesc::CTLVertexShaderDesc(const VSInputDecls& vsInputDecls, UINT shaderSettings) : 
    m_vsInputDecls(MAX_VS_INPUT_REGS),
    m_shaderSettings(shaderSettings)
{
    for (UINT i = 0, n = vsInputDecls.GetSize(); i < n; ++i)
    {
        const UINT usage = vsInputDecls[i].Usage;
        switch (usage)
        {        
        case D3DDECLUSAGE_POSITIONT:
            // This should already have D3DDECLUSAGE_POSITIONT replaced with D3DDECLUSAGE_POSITION.
            SHADER_CONV_ASSERT(false);
            break;
        case D3DDECLUSAGE_POSITION:
        case D3DDECLUSAGE_COLOR:
        case D3DDECLUSAGE_TEXCOORD:
        case D3DDECLUSAGE_FOG:
        case D3DDECLUSAGE_PSIZE:
            m_vsInputDecls.AddDecl(
                usage,
                vsInputDecls[i].UsageIndex,
                vsInputDecls[i].RegIndex,
                vsInputDecls[i].IsTransformedPosition
                );

            m_vsOutputDecls.AddDecl(
                usage, 
                vsInputDecls[i].UsageIndex,
                vsInputDecls[i].RegIndex,
                D3DSP_WRITEMASK_ALL
                );
            break;
        default:
            NO_DEFAULT;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
CTLVSContext::CTLVSContext(
    UINT runtimeVersion, 
    const CTLVertexShaderDesc* pShaderDesc, 
    CShaderAsmWrapper* pShaderAsm) :
    IContext(runtimeVersion, pShaderAsm),
    m_pShaderDesc(pShaderDesc)
{
    //--
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTLVSContext::WriteDeclarations()
{
    const VSInputDecls& vsInputDecls = m_pShaderDesc->GetInputDecls();

    // Declare temp registers.  Currently transformed vertices use a hard coded shader that uses exactly 3 temp registers.
    m_pShaderAsm->EmitTempsDecl(12);

    // Declare extension constant buffer.
    m_pShaderAsm->EmitConstantBufferDecl(
        CB_VS_EXT, __sizeof16( VSCBExtension ), D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED );

    // Declare input registers
    for (UINT i = 0, n = vsInputDecls.GetSize(); i < n; ++i)
    {
        const UINT regIndex = vsInputDecls[i].RegIndex;
        m_pShaderAsm->EmitInputDecl(regIndex, D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL);
    }
        
    // Declare other output registerss
    for (UINT i = 0, n = vsInputDecls.GetSize(); i < n; ++i)
    {
        const UINT usage      = vsInputDecls[i].Usage;
        const UINT usageIndex = vsInputDecls[i].UsageIndex;
        const UINT regIndex   = vsInputDecls[i].RegIndex;

        if ((D3DDECLUSAGE_POSITIONT == usage) 
        && (0 == usageIndex))
        {
            // D3D10_SB_NAME_POSITIONT is WARP only
            //
            // Declare the output position register
            // m_pShaderAsm->EmitOutputSystemInterpretedValueDecl(
            //    0, D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL, D3D10_SB_NAME_POSITIONT);

            // All vertex declarations should already have D3DDECLUSAGE_POSITIONT replaced with D3DDECLUSAGE_POSITION
            SHADER_CONV_ASSERT(false);
            return E_FAIL;
        }
        else if ( D3DDECLUSAGE_POSITION == usage &&
             0 == usageIndex )
        {
            // Declare the position ouput register.
            m_pShaderAsm->EmitOutputSystemInterpretedValueDecl(regIndex, D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL, D3D10_SB_NAME_POSITION );
        }
        else
        {
            m_pShaderAsm->EmitOutputDecl(regIndex, D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL);
        }
    }

    return S_OK;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTLVSContext::TranslateInstructions()
{
    //--

    return S_OK;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CTLVSContext::WriteOutputs()
{
    const VSInputDecls& vsInputDecls = m_pShaderDesc->GetInputDecls();

    //
    // Write other output registers
    //

    for (UINT i = 0, n = vsInputDecls.GetSize(); i < n; ++i)
    {
        const UINT usage = vsInputDecls[i].Usage;

        bool bSaturate = false;

#if 1 // start - pixel center workaround
        if (D3DDECLUSAGE_POSITIONT == usage
            && 0 == vsInputDecls[i].UsageIndex)
        {
            // Any instance of D3DDECLUSAGE_POSITIONT should have already been replaced by D3DDECLUSAGE_POSITION at this stage.
            SHADER_CONV_ASSERT(false);
        }
        else if (D3DDECLUSAGE_POSITION == usage
            && vsInputDecls[i].IsTransformedPosition
            && 0 == vsInputDecls[i].UsageIndex)
        {
            // The vertex shader input declaration should already have D3DDECLUSAGE_POSITIONT replaced with D3DDECLUSAGE_POSITION at this stage.

            // First transform from screen space (the original input space) and clip space.

            // Xclip = ( Xscrn + 0.5 - ( VP.width/2 + VP.left ) ) * 2/VP.width * 1/Wscrn
            // Yclip = ( Yscrn + 0.5 - ( VP.height/2 + VP.top ) ) * -2/VP.height * 1/Wscrn
            // Zclip = ( Zscrn - VP.zmin ) * 1/VP.depth * 1/Wscrn
            // Wclip = 1/Wscrn

            // cb3[ScreenToClipOffset] = (  
            //     0.5 - ( VP.width/2 + VP.left ),
            //     0.5 - ( VP.height/2 + VP.top ),
            //     - VP.zmin,
            //     0
            // )

            // v0 contains the original vertex input positionT value
            // By reading v0 directly this will ignore any world/view/projection transform applied as part of fixed function shader translation.
            // s0.xyzw = post.xyzw + cb3[ScreenToClipOffset].xyzw 
            // add s0.xyzw, v0.xyzw, cb3[ScreenToClipOffset].xyzw
            m_pShaderAsm->EmitInstruction(
                CInstruction(
                    D3D10_SB_OPCODE_ADD,
                    CTempOperandDst( SREG_TMP0 ),
                    CInputOperand4(0),
                    CCBOperand2D( CB_VS_EXT, VSCBExtension::SCREENTOCLIPOFFSET )));

            // cb3[ScreenToClipScale] = (  
            //     2/VP.width,
            //     -2/VP.height,
            //     1/VP.depth,
            //     1
            // )

            // s1.xyzw = s0.xyzw * cb3[ScreenToClipScale].xyzw
            // mul s1.xyzw, s0.xyzw, cb3[ScreenToClipScale].xyzw
            m_pShaderAsm->EmitInstruction(
                CInstruction(
                    D3D10_SB_OPCODE_MUL,
                    CTempOperandDst( SREG_TMP1 ),
                    CTempOperand4( SREG_TMP0 ),
                    CCBOperand2D( CB_VS_EXT, VSCBExtension::SCREENTOCLIPSCALE )));

            // s2.x = 1 / post.w
            // rcp s2.x, v0.w
            m_pShaderAsm->EmitInstruction(
                CInstruction(
                    D3D11_SB_OPCODE_RCP,
                    CTempOperandDst( SREG_TMP2, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                    CSwizzle( CInputOperand4(0), __SWIZZLE_W )));

            // pos.xyzw = (1 / post.w) * s1.xyzw
            // mul pos.xyzw, s2.x, s1.xyzw
            m_pShaderAsm->EmitInstruction(
                CInstruction(
                    D3D10_SB_OPCODE_MUL,
                    COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, 0),
                    CSwizzle( CTempOperand4( SREG_TMP2 ), __SWIZZLE_X ),
                    CTempOperand4( SREG_TMP1 )));

            // At this point pos.w = ((post.w + 0) * 1) / post.w  = 1
            // Need an extra divide by Wscrn for pos.w only

            // pos.w = 1 / post.w
            // mov pos.w, s2.x
            m_pShaderAsm->EmitInstruction(
                CInstruction(
                    D3D10_SB_OPCODE_MOV,
                    COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, 0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W),
                    CSwizzle( CTempOperand4( SREG_TMP2 ), __SWIZZLE_X )));


            // Alternate Implemenation
            // (this was not implemented due to lack of time, but would be a cleaner solution)

            // To assign  1/Wscrn to Wclip, first force Wclip to 1 and then olny divide pos by Wscrn once.
            // Could simplify the vertex shader instructions due to no extra divide by Wscrn at the end.

            // Xclip = ( Xscrn * (2/VP.width)        + (1/VP.width - ( VP.width/2 + VP.left ) * 2/VP.width) )             * 1/Wscrn
            // Yclip = ( Yscrn * (-2/VP.height)      + (- 1/VP.height - ( VP.height/2 + VP.top ) * -2/VP.height) )        * 1/Wscrn
            // Zclip = ( Zscrn * (2/VP.depth)        + (- VP.zmin * 2/VP.depth) )                                         * 1/Wscrn
            // Wclip = ( Wscrn * 0                   + 1 )                                                                * 1/Wscrn

            // s0.xyzw = v0.xyzw * cb3[ScreenToClipScale].xyzw + cb3[ScreenToClipOffset].xyzw
            // mad s0.xyzw, v0.xyzw, cb3[ScreenToClipScale].xyzw, cb3[ScreenToClipOffset].xyzw
        }
        else
        {
#endif // end - pixel center workaround

            switch (usage)
            {
            case D3DDECLUSAGE_COLOR:
                bSaturate = true;
                break;

            case D3DDECLUSAGE_FOG:
                bSaturate = (m_runtimeVersion < 9);
                break;
            }

            const UINT regIndex = vsInputDecls[i].RegIndex;
            const CInputOperand4 inOperand(regIndex);
            const COperandDst outOperand(D3D10_SB_OPERAND_TYPE_OUTPUT, regIndex);

            m_pShaderAsm->EmitInstruction(
                CInstructionEx(D3D10_SB_OPCODE_MOV, bSaturate, outOperand, inOperand));

#if 1 // start - pixel center workaround
        }
#endif // end - pixel center workaround
    }
}

} // namespace ShaderConv
