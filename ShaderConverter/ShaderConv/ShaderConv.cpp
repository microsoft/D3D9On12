// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Implementation for the Shader Converter objects
*
****************************************************************************/

#include "pch.h"
#include "shaderconv.hpp"

namespace ShaderConv
{

void ShaderConverterAPI::CleanUpConvertedShader(ByteCode& byteCode)
{
    free(byteCode.m_pByteCode);
}

ShaderConverterAPI::~ShaderConverterAPI()
{
    if (m_pTranslator)
    {
        m_pTranslator->Release();
    }
}

HRESULT AllocTemporarySpace(ByteCode& code, size_t size)
{
    code.m_pByteCode = malloc(size);

    if (code.m_pByteCode)
    {
        code.m_byteCodeSize = size;
        return S_OK;
    }

    code.m_byteCodeSize = 0;
    return E_OUTOFMEMORY;
}

HRESULT ShaderConverterAPI::ConvertTLShader(ConvertTLShaderArgs& args)
{
    HRESULT hr = S_OK;

    if (m_pTranslator == nullptr)
    {
        hr = ShaderConv::CreateTranslator(args.apiVersion, &m_pTranslator);
    }
    if (SUCCEEDED(hr) && m_pTranslator)
    {
        const CTLVertexShaderDesc desc(args.vsInputDecl, args.shaderSettings);
        CCodeBlob* pCodeBlob;

        hr = m_pTranslator->TranslateTLVS(&desc, &pCodeBlob);
        if (SUCCEEDED(hr) && pCodeBlob)
        {
            hr = AllocTemporarySpace(args.convertedByteCode, pCodeBlob->GetBufferSize());

            if (SUCCEEDED(hr))
            {
                memcpy(args.convertedByteCode.m_pByteCode, pCodeBlob->GetBufferPointer(), pCodeBlob->GetBufferSize());

                args.vsOutputDecl = desc.GetOutputDecls();
            }
            else
            {
                Check(false);
            }
        }

        auto translationData = m_pTranslator->GetTranslationData();
        args.totalInstructionsEmitted = translationData.NumInstructionsEmitted;
        args.totalExtraInstructionsEmitted = translationData.NumExtraInstructionsEmitted;
    }

    return hr;
}

void AddAllAddedSystemSemantics(const VSOutputDecls &pixelShaderInput, std::vector<VSOutputDecl> &addedSystemSemantics)
{
    // This informs 9on12 that it can't just re-use the vertex-shader's output signature for 
    // the pixel shader's input signature when the pixel shader has added an extra system value input. 
    auto pDecl = pixelShaderInput.FindOutputDecl(D3DDECLUSAGE_VFACE, 0);
    if (pDecl)
    {
        addedSystemSemantics.push_back(*pDecl);
    }
}

HRESULT ShaderConverterAPI::ConvertShader(ConvertShaderArgs& args)
{
    HRESULT hr = S_OK;

    if (args.legacyByteCode.m_pByteCode == nullptr)
    {
        hr = E_INVALIDARG;
    }

    VSOutputDecls* pPSin = args.pPsInputDecl;
    VSInputDecls* pVSin = args.pVsInputDecl;
    VSOutputDecls* pVSout = args.pVsOutputDecl;
    const RasterStates& rasterState = args.rasterStates;
    CCodeBlob* pBlob = nullptr;

    if (m_pTranslator == nullptr)
    {
        hr = ShaderConv::CreateTranslator(args.apiVersion, &m_pTranslator);
    }
    if (SUCCEEDED(hr) && m_pTranslator)
    {
        if (args.type == ConvertShaderArgs::SHADER_TYPE::SHADER_TYPE_VERTEX)//VS
        {
            CVertexShaderDesc* pDesc = nullptr;
            hr = m_pTranslator->AnalyzeVS(args.legacyByteCode.m_pByteCode, UINT(args.legacyByteCode.m_byteCodeSize), args.shaderSettings, args.rasterStates, args.pVsInputDecl, &pDesc);
            if (SUCCEEDED(hr) && pDesc)
            {
                // If the shader uses dynamic indexing, we have no way of knowing how large the CB is
                if (pDesc->HasRelAddrConsts(ShaderConv::CB_FLOAT))
                {
                    args.maxFloatConstsUsed = MAX_VS_CONSTANTSF * 4;
                }
                else
                {
                    args.maxFloatConstsUsed = pDesc->GetMaxUsedConsts(eConstantBuffers::CB_FLOAT);
                }

                Check(!pDesc->HasRelAddrConsts(ShaderConv::CB_INT));
                Check(!pDesc->HasRelAddrConsts(ShaderConv::CB_BOOL));
                args.maxIntConstsUsed = pDesc->GetMaxUsedConsts(eConstantBuffers::CB_INT);
                args.maxBoolConstsUsed = pDesc->GetMaxUsedConsts(eConstantBuffers::CB_BOOL);

                *pVSin = pDesc->GetInputDecls();
                memcpy(pVSout, &pDesc->GetOutputDecls(), sizeof(*pVSout));

                hr = m_pTranslator->TranslateVS(pDesc, rasterState, &pBlob);

                for (UINT i = 0; i < ARRAYSIZE(args.m_inlineConsts); i++)
                {
                    args.m_inlineConsts[i] = std::move(pDesc->MoveInlineConstants((eConstantBuffers)i));
                }

                pDesc->Release();
            }
        }
        else//PS
        {
            if (pPSin)
            {
                CPixelShaderDesc* pDesc = nullptr;
                hr = m_pTranslator->AnalyzePS(args.legacyByteCode.m_pByteCode, UINT(args.legacyByteCode.m_byteCodeSize), args.shaderSettings, args.rasterStates, &pDesc);

                if (SUCCEEDED(hr) && pDesc)
                {
                    if (pDesc->HasRelAddrConsts(ShaderConv::CB_FLOAT))
                    {
                        args.maxFloatConstsUsed = MAX_PS_CONSTANTSF * 4;
                    }
                    else
                    {
                        args.maxFloatConstsUsed = pDesc->GetMaxUsedConsts(eConstantBuffers::CB_FLOAT);
                    }

                    Check(!pDesc->HasRelAddrConsts(ShaderConv::CB_INT));
                    Check(!pDesc->HasRelAddrConsts(ShaderConv::CB_BOOL));
                    args.maxIntConstsUsed = pDesc->GetMaxUsedConsts(eConstantBuffers::CB_INT);
                    args.maxBoolConstsUsed = pDesc->GetMaxUsedConsts(eConstantBuffers::CB_BOOL);

                    VSOutputDecls updatedInputDecls;
                    pDesc->UpdateInputDecls(*pPSin, rasterState, &updatedInputDecls);
                    hr = m_pTranslator->TranslatePS(pDesc, rasterState, updatedInputDecls, &pBlob);
                    
                    AddAllAddedSystemSemantics(updatedInputDecls, args.AddedSystemSemantics);

                    for (UINT i = 0; i < ARRAYSIZE(args.m_inlineConsts); i++)
                    {
                        args.m_inlineConsts[i] = std::move(pDesc->MoveInlineConstants((eConstantBuffers)i));
                    }
                    args.outputRegistersMask = pDesc->GetOutputRegistersMask();

                    pDesc->Release();
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
        auto translationData = m_pTranslator->GetTranslationData();
        args.totalInstructionsEmitted = translationData.NumInstructionsEmitted;
        args.totalExtraInstructionsEmitted = translationData.NumExtraInstructionsEmitted;

        if (SUCCEEDED(hr) && pBlob)
        {
            // DXBC needs to be 4 byte aligned for dxilconv
            UINT padding = 0;
            UINT bufferSizeMod4 = pBlob->GetBufferSize() % 4;
            if (bufferSizeMod4 != 0)
            {
                padding = 4 - bufferSizeMod4;
            }
            hr = AllocTemporarySpace(args.convertedByteCode, pBlob->GetBufferSize());
            if (SUCCEEDED(hr))
            {
                memcpy(args.convertedByteCode.m_pByteCode, pBlob->GetBufferPointer(), pBlob->GetBufferSize());
                if (padding > 0)
                {
                    ZeroMemory((BYTE*)args.convertedByteCode.m_pByteCode + pBlob->GetBufferSize(), padding);
                }
            }
            else
            {
                Check(false);
            }
        }
    }

    if (pBlob)
    {
        pBlob->Release();
    }

    return hr;
}

HRESULT ShaderConverterAPI::CreateGeometryShader(CreateGeometryShaderArgs& args)
{
    HRESULT hr = S_OK;

    CGeometryShaderDesc geoDesc = CGeometryShaderDesc(args.m_ApiVersion, args.m_ShaderSettings, args.m_VsOutputDecls, args.m_RasterStates);

    CCodeBlob* pCodeBlob = {};

    // Translate the shader code from HLSL 2.0 to 4.0
    hr = m_pTranslator->TranslateGS(&geoDesc, &pCodeBlob);

    if (SUCCEEDED(hr))
    {
        // DXBC needs to be 4 byte aligned for dxilconv
        UINT padding = 0;
        UINT bufferSizeMod4 = pCodeBlob->GetBufferSize() % 4;
        if (bufferSizeMod4 != 0)
        {
            padding = 4 - bufferSizeMod4;
        }
        hr = AllocTemporarySpace(args.m_GSByteCode, pCodeBlob->GetBufferSize());

        if (SUCCEEDED(hr))
        {
            memcpy(args.m_GSByteCode.m_pByteCode, pCodeBlob->GetBufferPointer(), pCodeBlob->GetBufferSize());
            if (padding > 0)
            {
                ZeroMemory((BYTE*)args.m_GSByteCode.m_pByteCode + pCodeBlob->GetBufferSize(), padding);
            }
            *args.m_pGsOutputDecls = geoDesc.GetOutputDecls();
        }
        else
        {
            Check(false);
        }
    }

    return hr;
}

LONG
CObject::AddRef() const
{
    return ::InterlockedIncrement( &m_lRefCount );
}

LONG
CObject::Release() const
{
    SHADER_CONV_ASSERT( m_lRefCount > 0 );
    const LONG lRefCount = ::InterlockedDecrement( &m_lRefCount );
    if ( 0 == lRefCount )
    {
        delete this;
    }
    return lRefCount;
}

//////////////////////////////////////////////////////////////////////////////

TEXTURETYPE 
ToTextureType(D3D10DDIRESOURCE_TYPE resourceType)
{
    ShaderConv::TEXTURETYPE textureType;

    switch (resourceType)
    {
    case D3D10DDIRESOURCE_TEXTURE2D:
        textureType = TEXTURETYPE_2D;
        break;

    case D3D10DDIRESOURCE_TEXTURE3D:
        textureType = TEXTURETYPE_VOLUME;
        break;

    case D3D10DDIRESOURCE_TEXTURECUBE:
        textureType = TEXTURETYPE_CUBE;
        break;

    default:
        textureType = TEXTURETYPE_UNKNOWN;
        break;
    }

    return textureType;
}

//////////////////////////////////////////////////////////////////////////////

HRESULT
CCodeBlob::Create( size_t cbSize, const void* pBuffer, CCodeBlob** ppCodeBlob )
{
    SHADER_CONV_ASSERT( pBuffer && ppCodeBlob );

    CCodeBlob* const pCodeBlog = new CCodeBlob();
    if ( NULL == pCodeBlog )
    {
        SHADER_CONV_ASSERT(!"CCodeBlob() allocation failed, out of memory\n" );
        return E_OUTOFMEMORY;
    }

    pCodeBlog->AddRef();

    // Allocate necessary memory for the buffer
    BYTE* const pBits = new BYTE[cbSize];

    if ( NULL == pBits )
    {
        pCodeBlog->Release();
        SHADER_CONV_ASSERT(!"WarpPlatform::AllocateMemory() failed, out of memory\n" );
        return E_OUTOFMEMORY;
    }

    // Copy the buffer to destination
    memcpy( pBits, pBuffer, cbSize );

    // Set the blob properties
    pCodeBlog->m_cbSize = cbSize;
    pCodeBlog->m_pBits = pBits;

    // Return the blob
    *ppCodeBlob = pCodeBlog;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////

HRESULT
CShaderDesc::CopyInstructions( const void* pInstrs, UINT cbSize )
{
    SHADER_CONV_ASSERT( pInstrs && cbSize );

    m_pdwInstrs = new DWORD[cbSize];

    if ( NULL == m_pdwInstrs )
    {
        return E_OUTOFMEMORY;
    }

    memcpy( m_pdwInstrs, pInstrs, cbSize );

    m_cbCodeSize = cbSize;

    return S_OK;
}

bool
CShaderDesc::FindInlineConstant( eConstantBuffers type, UINT regIndex, ShaderConst* pShaderConst ) const
{
    for ( size_t i = 0, n = m_inlineConsts[type].size(); i < n; ++i )
    {
        const ShaderConst& shaderConst = m_inlineConsts[type][i];
        if ( shaderConst.RegIndex == regIndex )
        {
            if ( pShaderConst )
            {
                *pShaderConst = shaderConst;
            }

            return true;
        }
    }

    return false;
}

} // namespace ShaderConv
