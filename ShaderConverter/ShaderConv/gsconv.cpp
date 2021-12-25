// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Implementation for The Geometry Shader Converter
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
enum eSystemRegister2
{
    SYS_REG0,
    SYS_REG1,
    SYS_REG2,
    SYS_REG3,

    SYS_PSCALE0,
    SYS_PSCALE1,
    SYS_PSCALE2,
    SYS_PSCALE3,

    SYS_REG_SIZE,
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTranslator::TranslateGS( const CGeometryShaderDesc* pShaderDesc,
                          CCodeBlob** ppCodeBlob )
{
    HRESULT hr;

    if ( NULL == ppCodeBlob )
    {
        SHADER_CONV_ASSERT(FALSE);
        return E_INVALIDARG;
    }

    // Verify that this geometry shader descriptor is valid
    if ( !pShaderDesc->IsValid() )
    {
        SHADER_CONV_ASSERT(FALSE);
        return E_INVALIDARG;
    }

    // Create a new geometry shader context
    CGSContext* pContext = new CGSContext( m_runtimeVersion, pShaderDesc, m_pShaderAsm );
    if ( NULL == pContext )
    {
        return E_OUTOFMEMORY;
    }

    // Start the shader assembler
    m_pShaderAsm->StartShader( D3D10_SB_GEOMETRY_SHADER, 4, 0, pShaderDesc->GetShaderSettings());

    // Write the shader declarations
    hr = pContext->WriteDeclarations();
    if ( FAILED( hr ) )
    {
        __safeDelete( pContext );
        SHADER_CONV_ASSERT(FALSE);
        return hr;
    }

    // Translate instructions
    hr = pContext->TranslateInstructions();
    if ( FAILED( hr ) )
    {
        __safeDelete( pContext );
        SHADER_CONV_ASSERT(FALSE);
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
        SHADER_CONV_ASSERT(FALSE);
        return hr;
    }

    // Create the code blob
    hr = CCodeBlob::Create( m_pShaderAsm->ShaderSizeInDWORDs() * sizeof( UINT ),
                            m_pShaderAsm->GetShader(),
                            ppCodeBlob );
    if ( FAILED( hr ) )
    {
        SHADER_CONV_ASSERT(FALSE);
        return hr;
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////

CGeometryShaderDesc::CGeometryShaderDesc(
    UINT runtimeVersion,
    UINT shaderSettings,
    const VSOutputDecls& vsOutputDecls,
    const RasterStates& rasterStates ) :
    m_shaderSettings(shaderSettings)
{
    this->Clear();

    GSFLAGS flags;

    if ( D3DFILL_POINT == rasterStates.FillMode &&
         rasterStates.PrimitiveType >= D3DPT_TRIANGLELIST )
    {
        // Triangle Point fill enabled
        flags.PointFill = 1;

        if ((runtimeVersion < 9)
         && (D3DSHADE_FLAT == rasterStates.ShadeMode))
        {
            flags.FlatColorFill = 1;
        }
    }

    if ( flags.PointFill ||
         D3DPT_POINTLIST == rasterStates.PrimitiveType )
    {
        if ( rasterStates.PointSpriteEnable )
        {
            // Point sprite enabled
            flags.PointSprite = 1;
        }

        if ( flags.PointFill ||
             flags.PointSprite ||
             vsOutputDecls.PointSize ||
             rasterStates.PointSizeEnable )
        {
            // Point size enabled
            flags.PointSize = 1;
        }
    }

    if (vsOutputDecls.TexCoords 
     && !flags.PointSprite 
     && (D3DPT_POINTLIST != rasterStates.PrimitiveType))
    {
        for (UINT i = 0, mask = vsOutputDecls.TexCoords; mask; mask >>= 1, ++i)
        {
            if (mask & 1)
            {
                const BYTE texCoordWrap = rasterStates.PSSamplers[i].TexCoordWrap;
                if (texCoordWrap)
                {
                    // Copy the texture wrap value
                    m_textureWraps[i] = texCoordWrap;

                    // Enable Texture wrapping
                    flags.TextureWrap = 1;
                }
            }
        }
    }

    if (flags.Value)
    {
        // Select input/output topology
        switch (rasterStates.PrimitiveType)
        {
        default:
            NO_DEFAULT;

        case D3DPT_POINTLIST:
            flags.PrimitiveSize = 1;
            break;

        case D3DPT_LINELIST:
        case D3DPT_LINESTRIP:
            flags.PrimitiveSize = 2;
            break;

        case D3DPT_TRIANGLELIST:
        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            flags.PrimitiveSize = 3;
            break;
        }

        flags.UserClipPlanes = rasterStates.UserClipPlanes;
        flags.HasTLVertices  = rasterStates.HasTLVertices;                

        // Copy the input declaration
        m_inputDecls = vsOutputDecls;
        m_outputDecls = vsOutputDecls;

        // Update the flags
        m_flags = flags;
    }
    
    if (rasterStates.PointSpriteEnable)
    {
        m_outputDecls.AddDecl(D3DDECLUSAGE_POINTSPRITE, 0, VSOREG_PointSprite, D3DSP_WRITEMASK_ALL);
    }
}

//////////////////////////////////////////////////////////////////////////////

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
CGSContext::CGSContext( 
    UINT runtimeVersion,
    const CGeometryShaderDesc* pShaderDesc,
    CShaderAsmWrapper* pShaderAsm) 
    : IContext(runtimeVersion, pShaderAsm)
    , m_pShaderDesc(pShaderDesc)
{
    // Get the primitive size
    m_uiPrimitiveSize = pShaderDesc->GetPrimitiveSize();

    // Cache needed declarations
    const VSOutputDecls& vsInputDecls = pShaderDesc->GetInputDecls();
    for (UINT i = 0, n = vsInputDecls.GetSize(); i < n; ++i)
    {
        const UINT usage = vsInputDecls[i].Usage;
        const UINT usageIndex = vsInputDecls[i].UsageIndex;

        if (D3DDECLUSAGE_TEXCOORD == usage)
        {
            m_texcoordDecls[usageIndex] = vsInputDecls[i];
        }
        else
        if (((D3DDECLUSAGE_POSITION == usage) 
          || (D3DDECLUSAGE_POSITIONT == usage)) 
         && (0 == usageIndex))
        {
            m_positionDecl = vsInputDecls[i];
        }
        else
        if ((D3DDECLUSAGE_PSIZE == usage)
         && (0 == usageIndex))
        {
            m_psizeDecl = vsInputDecls[i];
        }
    }

    // Reset temp registers
    memset(m_tempRegs, INVALID_INDEX, sizeof(m_tempRegs));
    m_uiNumTempRegs = SYS_REG_SIZE;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CGSContext::WriteDeclarations()
{
    const bool isPointSizeEnable   = m_pShaderDesc->PointSizeEnable();

    D3D10_SB_PRIMITIVE_TOPOLOGY topology;
    D3D10_SB_PRIMITIVE primitive;

    const UINT uiPrimitiveSize = m_uiPrimitiveSize;
    switch (uiPrimitiveSize)
    {
    default:
        NO_DEFAULT;

    case 1:
        primitive = D3D10_SB_PRIMITIVE_POINT;
        topology  = D3D10_SB_PRIMITIVE_TOPOLOGY_POINTLIST;
        break;

    case 2:
        primitive = D3D10_SB_PRIMITIVE_LINE;
        topology  = D3D10_SB_PRIMITIVE_TOPOLOGY_LINESTRIP;
        break;

    case 3:
        primitive = D3D10_SB_PRIMITIVE_TRIANGLE;
        topology  = D3D10_SB_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        break;
    }

    UINT uiMaxVertices = uiPrimitiveSize;

    if (isPointSizeEnable)
    {
        uiMaxVertices *= 4;
        topology = D3D10_SB_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    }

    // Declare maximum output vertex count
    m_pShaderAsm->EmitGSMaxOutputVertexCountDecl(uiMaxVertices);

    // Declare temp registers
    const VSOutputDecls& gsOutputDecls = m_pShaderDesc->GetOutputDecls();
    const UINT uNumTempRegs = m_uiNumTempRegs + gsOutputDecls.GetSize();
    SHADER_CONV_ASSERT(uNumTempRegs <= 0xff);
    m_pShaderAsm->EmitTempsDecl(uNumTempRegs);

    // Declare extension constant buffer
    m_pShaderAsm->EmitConstantBufferDecl(
        CB_VS_EXT, __sizeof16( VSCBExtension), D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED);

    // Declare input primitive type
    m_pShaderAsm->EmitGSInputPrimitiveDecl(primitive);

    // Declare output topology
    m_pShaderAsm->EmitGSOutputTopologyDecl(topology);

    // Declare input/output registers
    for (UINT i = 0, n = gsOutputDecls.GetSize(); i < n; ++i)
    {
        const UINT usage      = gsOutputDecls[i].Usage;
        const UINT usageIndex = gsOutputDecls[i].UsageIndex;
        const UINT regIndex   = gsOutputDecls[i].RegIndex;
        const UINT writeMask  = gsOutputDecls[i].WriteMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;

        if ((D3DDECLUSAGE_POSITIONT == usage) 
         && (0 == usageIndex))
        {
            // D3D10_SB_NAME_POSITIONT is WARP only.
            //
            // // Declare input position register
            // m_pShaderAsm->EmitInputSystemInterpretedValueDecl2D(
            //    uiPrimitiveSize, regIndex, writeMask, D3D10_SB_NAME_POSITIONT );

            // // Declare output position register
            // m_pShaderAsm->EmitOutputSystemInterpretedValueDecl(
            //    regIndex, writeMask, D3D10_SB_NAME_POSITIONT);

            return E_FAIL;
        }
        else
        if ((D3DDECLUSAGE_POSITION == usage) 
         && (0 == usageIndex))
        {
            // Declare input position register
            m_pShaderAsm->EmitInputSystemInterpretedValueDecl2D(
                uiPrimitiveSize, regIndex, writeMask, D3D10_SB_NAME_POSITION);

            // Declare output position register
            m_pShaderAsm->EmitOutputSystemInterpretedValueDecl(
                regIndex, writeMask, D3D10_SB_NAME_POSITION);
        }
        else if (usage == D3DDECLUSAGE_POINTSPRITE)
        {
            // GS generates this value so don't declare an input declaration
            m_pShaderAsm->EmitOutputDecl(regIndex, writeMask);
        }
        else if (usage == D3DDECLUSAGE_CLIPDISTANCE)
        {
            // Declare input position register
            m_pShaderAsm->EmitInputSystemInterpretedValueDecl2D(
                uiPrimitiveSize, regIndex, writeMask, D3D10_SB_NAME_CLIP_DISTANCE);

            // Declare output position register
            m_pShaderAsm->EmitOutputSystemInterpretedValueDecl(
                regIndex, writeMask, D3D10_SB_NAME_CLIP_DISTANCE);
        }
        else 
        {
            m_pShaderAsm->EmitInputDecl2D(uiPrimitiveSize, regIndex, writeMask);
            
            // Do not output the pointsize register
            if (usage != D3DDECLUSAGE_PSIZE)
            {
                m_pShaderAsm->EmitOutputDecl(regIndex, writeMask);
            }
        }
    }

    return S_OK;
}


///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CGSContext::TranslateInstructions()
{
    //--

    return S_OK;
}


///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CGSContext::WriteOutputs()
{
    const float vPointScaleCW[4][2] = {
        {-1.0f,-1.0f },
        { 1.0f,-1.0f },
        {-1.0f, 1.0f },
        { 1.0f, 1.0f },
    };

    const float vPointScaleCCW[4][2] = {
        {-1.0f,-1.0f },
        {-1.0f, 1.0f },
        { 1.0f,-1.0f },
        { 1.0f, 1.0f },
    };

    const UINT uiPrimitiveSize = m_uiPrimitiveSize;
    const VSOutputDecls& gsOutputDecls = m_pShaderDesc->GetOutputDecls();
    const UINT uiInputDeclSize = gsOutputDecls.GetSize();

    const bool isPointSizeEnable     = m_pShaderDesc->PointSizeEnable();
    const bool isPointFillEnable     = m_pShaderDesc->PointFillEnable();
    const bool isFlatColorFillEnable = m_pShaderDesc->FlatColorFillEnable();
    const bool isPointSpriteEnable   = m_pShaderDesc->PointSpriteEnable();
    const bool isTextureWrapEnable   = m_pShaderDesc->TextureWrapEnable();
    const bool hasTLVertices         = m_pShaderDesc->HasTLVertices();
    const UINT userClipPlanes        = m_pShaderDesc->GetUserClipPlanes();

    if ( isPointSizeEnable )
    {
        const InputRegister posReg(m_positionDecl.RegIndex, InputRegister::Input, m_positionDecl.WriteMask);
        const InputRegister psizeReg(m_psizeDecl.RegIndex, InputRegister::Input, m_psizeDecl.WriteMask);
        const UINT psizeSwizzle  = __swizzleFromWriteMask(
            m_psizeDecl.WriteMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT );

        if ( isPointFillEnable )
        {
            // The three vertices might be orriented counter clock wise,
            // then allowing culling to reject the generated quads.
            // We need to make sure that this doesn't happen by performing
            // our own cull test here and reverting the triangles direction.

            if ( hasTLVertices )
            {
                // Calculate backface culling
                // sign = ( ( x1 - x0 ) * ( y2 - y0 ) ) -
                //        ( ( x2 - x0 ) * ( y1 - y0 ) );
                // if ( sign < 0 ) => "is front face"

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_ADD,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  this->CreateInputSrcOperand( 1, posReg, __SWIZZLE_X ),
                                  CNegate( this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_X ) ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_ADD,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                                  this->CreateInputSrcOperand( 2, posReg, __SWIZZLE_Y ),
                                  CNegate( this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_Y ) ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_Y ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_ADD,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  this->CreateInputSrcOperand( 2, posReg, __SWIZZLE_X ),
                                  CNegate( this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_X ) ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_ADD,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                                  this->CreateInputSrcOperand( 1, posReg, __SWIZZLE_Y ),
                                  CNegate( this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_Y ) ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MAD,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_Y ),
                                  CNegate( CTempOperand4( SYS_REG0, __SWIZZLE_Z ) ) ) );
            }
            else
            {
                // Calculate backface culling
                // sign = ( w0 * ( x1 * y2 - x2 * y1 ) -
                //          w1 * ( x0 * y2 - x2 * y0 ) +
                //          w2 * ( x0 * y1 - x1 * y0 ) );
                // if ( sign < 0 ) => "is front face"

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  this->CreateInputSrcOperand( 1, posReg, __SWIZZLE_X ),
                                  this->CreateInputSrcOperand( 2, posReg, __SWIZZLE_Y ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                                  this->CreateInputSrcOperand( 2, posReg, __SWIZZLE_X ),
                                  this->CreateInputSrcOperand( 1, posReg, __SWIZZLE_Y ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG1, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_X ),
                                  this->CreateInputSrcOperand( 2, posReg, __SWIZZLE_Y ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG1, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                                  this->CreateInputSrcOperand( 2, posReg, __SWIZZLE_X ),
                                  this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_Y ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG2, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_X ),
                                  this->CreateInputSrcOperand( 1, posReg, __SWIZZLE_Y ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG2, D3D10_SB_OPERAND_4_COMPONENT_MASK_Y ),
                                  this->CreateInputSrcOperand( 1, posReg, __SWIZZLE_X ),
                                  this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_Y ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_ADD,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                                  CNegate( CTempOperand4( SYS_REG0, __SWIZZLE_Y ) ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_ADD,
                                  CTempOperandDst( SYS_REG1, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                                  CTempOperand4( SYS_REG1, __SWIZZLE_X ),
                                  CNegate( CTempOperand4( SYS_REG1, __SWIZZLE_Y ) ) ) );
                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_ADD,
                                  CTempOperandDst( SYS_REG2, D3D10_SB_OPERAND_4_COMPONENT_MASK_Z ),
                                  CTempOperand4( SYS_REG2, __SWIZZLE_X ),
                                  CNegate( CTempOperand4( SYS_REG2, __SWIZZLE_Y ) ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                                  this->CreateInputSrcOperand( 1, posReg, __SWIZZLE_W ),
                                  CTempOperand4( SYS_REG1, __SWIZZLE_Z ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MAD,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_W ),
                                  this->CreateInputSrcOperand( 0, posReg, __SWIZZLE_W ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_Z ),
                                  CNegate( CTempOperand4( SYS_REG0, __SWIZZLE_W ) ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MAD,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  this->CreateInputSrcOperand( 2, posReg, __SWIZZLE_W ),
                                  CTempOperand4( SYS_REG2, __SWIZZLE_Z ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_W ) ) );
            }

            m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_LT,
                              CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                              CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                              COperand( 0.0f ) ) );

            for ( UINT i = 0; i < 4; ++i )
            {
                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MOVC,
                                  COperandDst( D3D10_SB_OPERAND_TYPE_TEMP, SYS_PSCALE0 + i, __WRITEMASK_XY ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                                  COperand( vPointScaleCW[i][0], vPointScaleCW[i][1], 0.0f, 0.0f ),
                                  COperand( vPointScaleCCW[i][0], vPointScaleCCW[i][1], 0.0f, 0.0f ) ) );
            }
        }

        for ( UINT primitive = 0; primitive < uiPrimitiveSize; ++primitive )
        {
            if ( gsOutputDecls.PointSize )
            {
                // max r0.x, i_pts.swizzle, cb3[pointsize].y
                // min r0.x, r0.x, cb3[pointsize].z

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MAX,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  this->CreateInputSrcOperand( primitive, psizeReg, psizeSwizzle ),
                                  CCBOperand2D( CB_VS_EXT,
                                                VSCBExtension::POINTSIZE,
                                                __SWIZZLE_Y ) ) );

                m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_MIN,
                              CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                              CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                              CCBOperand2D( CB_VS_EXT,
                                            VSCBExtension::POINTSIZE,
                                            __SWIZZLE_Z ) ) );
            }
            else
            {
                // mov r0.x, cb3[pointsize].x

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MOV,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  CCBOperand2D( CB_VS_EXT,
                                                VSCBExtension::POINTSIZE,
                                                __SWIZZLE_X ) ) );
            }

            if ( hasTLVertices )
            {
                // mul r0.xy, r0.x, 0.5f
                m_pShaderAsm->EmitInstruction(
                CInstruction( D3D10_SB_OPCODE_MUL,
                              CTempOperandDst( SYS_REG0, __WRITEMASK_XY ),
                              CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                              COperand( 0.5f ) ) );
            }
            else
            {
                // Convert the point size from screen to clip space
                // The divide by 2 is consumed inside cb3[viewportscale].
                // mul r0.x, r0.x, t_pos.w
                // mul r0.xy, r0.x, cb3[viewportscale].xy

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG0, D3D10_SB_OPERAND_4_COMPONENT_MASK_X ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                                  this->CreateInputSrcOperand( primitive, posReg, __SWIZZLE_W ) ) );

                m_pShaderAsm->EmitInstruction(
                    CInstruction( D3D10_SB_OPCODE_MUL,
                                  CTempOperandDst( SYS_REG0, __WRITEMASK_XY ),
                                  CTempOperand4( SYS_REG0, __SWIZZLE_X ),
                                  CCBOperand2D( CB_VS_EXT, VSCBExtension::VIEWPORTSCALE, __SWIZZLE_XY ) ) );
            }

            for ( UINT v = 0; v < 4; ++v )
            {
                BYTE tempRegs = m_uiNumTempRegs;
                const BYTE tempRegPos = tempRegs++;

                if ( isPointFillEnable )
                {
                    // mad o_pos.xy, r0.xy, pscale[v], i_pos.xy
                    m_pShaderAsm->EmitInstruction(
                        CInstruction( D3D10_SB_OPCODE_MAD,
                                      this->CreateInputDstOperand( tempRegPos, primitive, posReg.Reg(), __WRITEMASK_XY ),
                                      CTempOperand4( SYS_REG0, __SWIZZLE_XY ),
                                      CTempOperand4( SYS_PSCALE0 + v, __SWIZZLE_XY ),
                                      COperand2DEx( D3D10_SB_OPERAND_TYPE_INPUT, primitive, posReg.Reg(), __SWIZZLE_XY ) ) );
                }
                else
                {
                    // mad o_pos.xy, r0.xy, scalecw, i_pos.xy
                    m_pShaderAsm->EmitInstruction(
                        CInstruction( D3D10_SB_OPCODE_MAD,
                                      this->CreateInputDstOperand( tempRegPos, primitive, posReg.Reg(), __WRITEMASK_XY ),
                                      CTempOperand4( SYS_REG0, __SWIZZLE_XY ),
                                      COperand( vPointScaleCW[v][0], vPointScaleCW[v][1], 0.0f, 0.0f ),
                                      COperand2DEx( D3D10_SB_OPERAND_TYPE_INPUT, primitive, posReg.Reg(), __SWIZZLE_XY ) ) );
                }

                // mov o_pos.zw, t_pos
                m_pShaderAsm->EmitInstruction(
                        CInstruction( D3D10_SB_OPCODE_MOV,
                                      this->CreateInputDstOperand( tempRegPos, primitive, posReg.Reg(), __WRITEMASK_ZW ),
                                      COperand2DEx( D3D10_SB_OPERAND_TYPE_INPUT, primitive, posReg.Reg() ) ) );

                if ( userClipPlanes )
                {
                    // Output user-defined clipplane distances
                    this->WriteClipplanes( userClipPlanes, this->CreateInputSrcOperand( primitive, posReg ) );
                }

                // Write to output registers
                for ( UINT i = 0; i < uiInputDeclSize; ++i )
                {
                    const UINT usage     = gsOutputDecls[i].Usage;
                    const InputRegister regIndex(gsOutputDecls[i].RegIndex, InputRegister::Input, gsOutputDecls[i].WriteMask);
                    const UINT writeMask = gsOutputDecls[i].WriteMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;

                    if (isFlatColorFillEnable
                     && (D3DDECLUSAGE_COLOR == usage))
                    {
                        // Copy primitive 0 color data
                        m_pShaderAsm->EmitInstruction(
                            CInstruction( D3D10_SB_OPCODE_MOV,
                                          COperandDst( D3D10_SB_OPERAND_TYPE_OUTPUT, regIndex.Reg(), writeMask ),
                                          this->CreateInputSrcOperand( 0, regIndex ) ) );
                    }
                    else if (usage == D3DDECLUSAGE_POINTSPRITE)
                    {
                        UNREFERENCED_PARAMETER(isPointSpriteEnable);
                        SHADER_CONV_ASSERT(isPointSpriteEnable);
                        const float vTexCoords[4][2] = {
                            { 0.0f, 0.0f },
                            { 1.0f, 0.0f },
                            { 0.0f, 1.0f },
                            { 1.0f, 1.0f },
                        };

                        // Ouput pointsprite texture coordinates
                        m_pShaderAsm->EmitInstruction(
                            CInstruction(D3D10_SB_OPCODE_MOV,
                                COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, VSOREG_PointSprite, D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL),
                                COperand(vTexCoords[v][0], vTexCoords[v][1], 0.0f, 1.0f)));
                    }
                    else if (usage == D3DDECLUSAGE_CLIPDISTANCE)
                    {
                        // Do nothing, already handled by WriteClipPlanes
                    }
                    else // Do not output the pointsize register
                    if (usage != D3DDECLUSAGE_PSIZE)
                    {
                        m_pShaderAsm->EmitInstruction(
                            CInstruction( D3D10_SB_OPCODE_MOV,
                                          COperandDst( D3D10_SB_OPERAND_TYPE_OUTPUT, regIndex.Reg(), writeMask ),
                                          this->CreateInputSrcOperand( primitive, regIndex ) ) );
                    }
                }

                m_pShaderAsm->Emit( D3D10_SB_OPCODE_EMIT );
            }

            m_pShaderAsm->Emit( D3D10_SB_OPCODE_CUT );
        }
    }
    else
    {
        const InputRegister posReg(m_positionDecl.RegIndex, InputRegister::Input, m_positionDecl.WriteMask);

        for ( UINT primitive = 0; primitive < uiPrimitiveSize; ++primitive )
        {
            if ( userClipPlanes )
            {
                // Output user-defined clipplane distances
                this->WriteClipplanes( userClipPlanes, this->CreateInputSrcOperand( primitive, posReg ) );
            }

            // Write to output registers
            for ( UINT i = 0; i < uiInputDeclSize; ++i )
            {
                const UINT usage      = gsOutputDecls[i].Usage;
                const UINT usageIndex = gsOutputDecls[i].UsageIndex;
                const InputRegister regIndex(gsOutputDecls[i].RegIndex, InputRegister::Input, gsOutputDecls[i].WriteMask);
                const UINT writeMask  = gsOutputDecls[i].WriteMask << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;

                if ((D3DDECLUSAGE_TEXCOORD == usage)
                 && isTextureWrapEnable
                 && (primitive > 0))
                {
                    this->EmitTexWrap( primitive, usageIndex, regIndex);
                }
                else
                if (usage != D3DDECLUSAGE_PSIZE)
                {
                    m_pShaderAsm->EmitInstruction(
                        CInstruction( D3D10_SB_OPCODE_MOV,
                                      COperandDst( D3D10_SB_OPERAND_TYPE_OUTPUT, regIndex.Reg(), writeMask ),
                                      this->CreateInputSrcOperand( primitive, regIndex ) ) );
                }
            }

            m_pShaderAsm->Emit( D3D10_SB_OPCODE_EMIT );
        }
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
void
CGSContext::EmitTexWrap(
    UINT primitive,
    UINT usageIndex,
    const InputRegister &regIndex)
{
    UINT swizzle = 0;
    UINT origWriteMask = regIndex.WriteMask() << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;
    UINT writeMask = 0;

    const UINT textureWrap = m_pShaderDesc->GetTextureWrap( usageIndex );

    if ( origWriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_X )
    {
        if ( textureWrap & D3DWRAPCOORD_0 )
        {
            writeMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
            swizzle |= __SWIZZLEN( 0, D3D10_SB_4_COMPONENT_X );
        }
    }

    if ( origWriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Y )
    {
        if ( textureWrap & D3DWRAPCOORD_1 )
        {
            writeMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;
            swizzle |= __SWIZZLEN( 1, D3D10_SB_4_COMPONENT_Y );
        }
    }

    if ( origWriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Z )
    {
        if ( textureWrap & D3DWRAPCOORD_2 )
        {
            writeMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_Z;
            swizzle |= __SWIZZLEN( 2, D3D10_SB_4_COMPONENT_Z );
        }
    }

    if ( origWriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_W )
    {
        if ( textureWrap & D3DWRAPCOORD_3 )
        {
            writeMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_W;
            swizzle |= __SWIZZLEN( 3, D3D10_SB_4_COMPONENT_W );
        }
    }

    // mov  out, t[p]
    // add  r0.writemask, t[p].swizzle, -t[0].swizzle
    // add  r1.writemask, r0.swizzle, vec4(1.0f)
    // add  r2.writemask, r0.swizzle, vec4(-1.0f)
    // lt   r3.writemask, r0.swizzle, vec4(0.0f)
    // movc r1.writemask, r3.swizzle, r1.swizzle, r2.swizzle
    // lt   r3.writemask, abs( r0.swizzle ), abs( r1.swizzle )
    // movc r0.writemask, r3.swizzle, r0.swizzle, r1.swizzle
    // add  out.writemask, r0.swizzle, t[0].swizzle

    if (writeMask)
    {
        m_pShaderAsm->EmitInstruction(
            CInstruction(D3D10_SB_OPCODE_ADD,
                CTempOperandDst(SYS_REG0, writeMask),
                this->CreateInputSrcOperand(primitive, regIndex, swizzle),
                CNegate(this->CreateInputSrcOperand(0, regIndex, swizzle))));

        m_pShaderAsm->EmitInstruction(
            CInstruction(D3D10_SB_OPCODE_ADD,
                CTempOperandDst(SYS_REG1, writeMask),
                CTempOperand4(SYS_REG0, swizzle),
                COperand(1.0f)));

        m_pShaderAsm->EmitInstruction(
            CInstruction(D3D10_SB_OPCODE_ADD,
                CTempOperandDst(SYS_REG2, writeMask),
                CTempOperand4(SYS_REG0, swizzle),
                COperand(-1.0f)));

        m_pShaderAsm->EmitInstruction(
            CInstruction(D3D10_SB_OPCODE_LT,
                CTempOperandDst(SYS_REG3, writeMask),
                CTempOperand4(SYS_REG0, swizzle),
                COperand(0.0f)));

        m_pShaderAsm->EmitInstruction(
            CInstruction(D3D10_SB_OPCODE_MOVC,
                CTempOperandDst(SYS_REG1, writeMask),
                CTempOperand4(SYS_REG3, swizzle),
                CTempOperand4(SYS_REG1, swizzle),
                CTempOperand4(SYS_REG2, swizzle)));

        m_pShaderAsm->EmitInstruction(
            CInstruction(D3D10_SB_OPCODE_LT,
                CTempOperandDst(SYS_REG3, writeMask),
                CAbs(CTempOperand4(SYS_REG0, swizzle)),
                CAbs(CTempOperand4(SYS_REG1, swizzle))));

        m_pShaderAsm->EmitInstruction(
            CInstruction(D3D10_SB_OPCODE_MOVC,
                CTempOperandDst(SYS_REG0, writeMask),
                CTempOperand4(SYS_REG3, swizzle),
                CTempOperand4(SYS_REG0, swizzle),
                CTempOperand4(SYS_REG1, swizzle)));

        m_pShaderAsm->EmitInstruction(
            CInstruction(D3D10_SB_OPCODE_ADD,
                COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, regIndex.Reg(), writeMask),
                CTempOperand4(SYS_REG0, swizzle),
                this->CreateInputSrcOperand(0, regIndex, swizzle)));
    }

    if ( origWriteMask != writeMask )
    {
        // Need to copy the remaining components
        const UINT writeMaskDiff = writeMask ^ origWriteMask;
        m_pShaderAsm->EmitInstruction(
            CInstruction( D3D10_SB_OPCODE_MOV,
                          COperandDst( D3D10_SB_OPERAND_TYPE_OUTPUT, regIndex.Reg(), writeMaskDiff ),
                          this->CreateInputSrcOperand( primitive, regIndex ) ) );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
COperandBase
CGSContext::CreateInputSrcOperand( UINT primitive, const InputRegister &inputReg, UINT swizzle )
{
    if (!swizzle)
    {
        swizzle = __swizzleFromWriteMask(inputReg.WriteMask() << D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT);
    }

    if ( m_tempRegs[primitive][inputReg.Reg()] != INVALID_INDEX )
    {
        return CTempOperand4( m_tempRegs[primitive][inputReg.Reg()], swizzle );
    }
    else
    {
        return COperand2DEx( D3D10_SB_OPERAND_TYPE_INPUT, primitive, inputReg.Reg(), swizzle );
    }
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
COperandBase
CGSContext::CreateInputDstOperand( BYTE tempRegIndex, UINT primitive, UINT regIndex, UINT writeMask )
{
    if ( INVALID_INDEX == m_tempRegs[primitive][regIndex] )
    {
        m_tempRegs[primitive][regIndex] = tempRegIndex;
    }

    return CTempOperandDst( m_tempRegs[primitive][regIndex], writeMask );
}

} // namespace ShaderConv
