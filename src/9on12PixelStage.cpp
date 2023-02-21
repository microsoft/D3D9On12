// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY UpdateWindowInfo(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_WINFO* pWInfo)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pWInfo == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetPipelineState().GetPixelStage().UpdateWInfo(*pWInfo);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetPixelShader(_In_ HANDLE hDevice, _In_ HANDLE hShader)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        PixelShader* pShader = (PixelShader*)Shader::GetShaderFromHandle(hShader);

        if (pShader)
        {
            pDevice->GetPipelineState().GetPixelStage().SetPixelShader(pShader);
        }
        else
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetRenderTarget(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETRENDERTARGET* pSetRenderTargetArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);

        if (pDevice == nullptr || pSetRenderTargetArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource *pResource = Resource::GetResourceFromHandle(pSetRenderTargetArg->hRenderTarget);

        const UINT subresource = pResource ? pResource->ConvertAppSubresourceToDX12Subresource(pSetRenderTargetArg->SubResourceIndex) : 0;

        pDevice->GetPipelineState().GetPixelStage().SetRenderTarget(pSetRenderTargetArg->RenderTargetIndex, BoundRenderTarget(pResource, subresource));
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetDepthStencil(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETDEPTHSTENCIL* pSetDepthStencilArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);

        if (pDevice == nullptr || pSetDepthStencilArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource *pResource = Resource::GetResourceFromHandle(pSetDepthStencilArg->hZBuffer);
        pDevice->GetPipelineState().GetPixelStage().SetDepthStencil(pResource);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    HRESULT PixelStage::SetSRV(Device& device, Resource* pResource, UINT dx9TextureIndex)
    {
        UNREFERENCED_PARAMETER(device);

        DWORD shaderRegister;
        D3D12TranslationLayer::EShaderStage shaderType;
        ConvertDX9TextureIndexToShaderStageAndRegisterIndex(dx9TextureIndex, shaderType, shaderRegister);

        HRESULT hr = S_OK;
        if (pResource)
        {
            const bool bResourceUsesHardwareShadowMapping = pResource->IsEligibleForHardwareShadowMapping();
            if (bResourceUsesHardwareShadowMapping && pResource->GetLogicalDesc().MipLevels > 1)
            {
                // 9on12 can't handle hardware shadow map cases where a Depth Buffer with multiple mips is passed in.
                // This is because hardware shadow mapping requires the sample_c command. However the sample_c command 
                // does not allow for the shader to specify an explicit LOD to read from if the shader uses the TEXLDL 
                // DXBC instruction
                ThrowFailure(E_FAIL);
            }

            if ((bool)m_samplerStateIDs[dx9TextureIndex].UseHardwareShadowMapping != bResourceUsesHardwareShadowMapping)
            {
                // The app is binding a special depth resource as a Texture this means that they will probably want to use
                // 'Hardware Shadow Mapping'. this means that we will need to edit the shader code so that it changes samples
                // of this texture to 'SampleCmp'
                m_rasterStates.SetHardwareShadowMappingFlag(shaderType, shaderRegister, bResourceUsesHardwareShadowMapping);
                m_samplerStateIDs[dx9TextureIndex].UseHardwareShadowMapping = bResourceUsesHardwareShadowMapping;

                m_dirtyFlags.Samplers |= BIT(dx9TextureIndex);
            }

            if ((bool)m_samplerStateIDs[dx9TextureIndex].SwapRBBorderColors != pResource->NeedsSwapRBOutputChannels())
            {
                m_samplerStateIDs[dx9TextureIndex].SwapRBBorderColors = pResource->NeedsSwapRBOutputChannels();
                m_dirtyFlags.Samplers |= BIT(dx9TextureIndex);
            }

            if (pResource->IsSystemMemory())
            {
                SYSTEM_MEMORY_RESOURCE_BOUND_AS_SHADER_RESOURCE_WARNING();
                pResource = pResource->GetBackingPlainResource();
            }

            hr = pResource->PreBind();
        }

        if(m_shaderResources[dx9TextureIndex] != pResource)
        {
            if (m_shaderResources[dx9TextureIndex])
            {
                m_shaderResources[dx9TextureIndex]->GetSRVBindingTracker().Unbind(dx9TextureIndex);
            }
            if (pResource)
            {
                pResource->GetSRVBindingTracker().Bind(dx9TextureIndex);
            }

            m_shaderResources[dx9TextureIndex] = pResource;
            // If a null resource is bound, arbitrarily call it a texture2D. We can't go with TEXTURETYPE_UNKNOWN
            // because the shader converter needs a valid texture type for the shader declaration even if the resource is null
            const ShaderConv::TEXTURETYPE srvTextureType = pResource ? pResource->GetShaderResourceTextureType() : ShaderConv::TEXTURETYPE_2D;
            if (shaderType == D3D12TranslationLayer::e_PS)
            {
                m_rasterStates.SetPSSamplerTextureType(shaderRegister, srvTextureType);
            }
            m_dirtyFlags.Textures |= BIT(dx9TextureIndex);
        }

        return hr;
    }

    void PixelStage::SetSRGBTexture(UINT index, bool srgbEnabled)
    {
        if (m_SRGBTexture[index] != srgbEnabled)
        {
            m_SRGBTexture[index] = srgbEnabled;
            m_dirtyFlags.Textures |= BIT(index);
        }
    }

    void PixelStage::SetSRGBWriteEnable(bool srgbEnabled)
    {
        if (m_SRGBWriteEnabled != srgbEnabled)
        {
            m_SRGBWriteEnabled = srgbEnabled;
            m_dirtyFlags.RenderTargets = true;
        }
    }

    PixelStage::PixelStage(PipelineStateDirtyFlags& pipelineStateDirtyFlags, RasterStatesWrapper& rasterStates) :
        m_dirtyFlags(pipelineStateDirtyFlags),
        m_rasterStates(rasterStates),
        m_pCurrentPS(nullptr),
        m_pCurrentD3D12PixelShader(nullptr),
        m_blendFactor(0),
        m_multiSampleMask(MAXUINT),
        m_SRGBWriteEnabled(false),
        m_dsvRWType(D3D12_DSV_FLAG_NONE),
        m_numBoundRenderTargets(0),
        m_rtSetMask(0),
        m_pDepthStencil(nullptr),
        m_hideDepthStencil(false)
    {
        memset(m_pRenderTargets, 0, sizeof(m_pRenderTargets));
        memset(m_shaderResources, 0, sizeof(m_shaderResources));
        memset(m_SRGBTexture, 0, sizeof(m_SRGBTexture));
        for (auto &componentMapping : m_rtvShaderComponentMapping)
        {
            componentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        }
    }

    HRESULT PixelStage::Init(Device& /*device*/)
    {
        SetRasterState(D3DRS_CLIPPING, 1); //d3d9 defaults this raster state to 1
        return S_OK;
    }

    void PixelStage::MarkSRVIndicesDirty(const std::vector<UINT> &indices)
    {
        for (UINT index : indices)
        {
            m_dirtyFlags.Textures |= BIT(index);
        }
    }


    void PixelStage::SetDepthStencil(Resource *pDepthStencil)
    { 
        const bool oldDepthStencilIsNull = m_pDepthStencil == nullptr;
        const bool newDepthStencilIsNull = pDepthStencil == nullptr;
        // Depth stencil state sets DepthEnable based on whether a depth stencil is valid or 
        // not, so update it if the DSV is changing to or from a null DSV
        if (oldDepthStencilIsNull != newDepthStencilIsNull)
        {
            m_dirtyFlags.DepthStencilState = true;
        }

        if (m_pDepthStencil != pDepthStencil)
        {
            // If binding a depth stencil that's already bound as an SRV, we need to make sure
            // we mark the SRV dirty flag so that appropriate logic is done later to determine if
            // we should hide the SRV binding (simultaneous DSV & SRV bindings are allowed in DX9 
            // but restricted in DX12 to only when depth write is off). We also need to do the same
            // if we're unbinding a depth stencil that may have caused an SRV to be hidden.
            if (m_pDepthStencil)
            {
                m_pDepthStencil->GetDSVBindingTracker().Unbind(0);
                MarkSRVIndicesDirty(m_pDepthStencil->GetSRVBindingTracker().GetBindingIndices());
            }

            if (pDepthStencil)
            {
                pDepthStencil->GetDSVBindingTracker().Bind(0);
                MarkSRVIndicesDirty(pDepthStencil->GetSRVBindingTracker().GetBindingIndices());
            }

            m_pDepthStencil = pDepthStencil;
            m_dirtyFlags.DepthStencil = true;
            m_dirtyFlags.RenderTargets = true;
        }
    }

    void PixelStage::UpdateWInfo(_In_ CONST D3DDDIARG_WINFO& winInfo)
    {
        m_rasterStates.SetWFogEnable(
            ((1.0f != winInfo.WNear) || (1.0f != winInfo.WFar)) ? 1 : 0);

        m_dirtyFlags.RasterizerState = true;
    }

    void PixelStage::SetPixelShader(PixelShader* pShader)
    {
        if (m_pCurrentPS != pShader)
        {
            m_pCurrentPS = pShader;
            m_dirtyFlags.PixelShader = true;
        }
    }

    void PixelStage::SetRenderTarget(UINT renderTargetIndex, BoundRenderTarget boundRenderTarget)
    {
        (boundRenderTarget.m_pResource == nullptr) ? m_rtSetMask &= ~BIT(renderTargetIndex) :
            m_rtSetMask |= BIT(renderTargetIndex);

        Resource *pPrevResource = m_pRenderTargets[renderTargetIndex].m_pResource;
        Resource *pNewResource = boundRenderTarget.m_pResource;

        if (pPrevResource != pNewResource)
        {
            // DX9 allows Render Targets and SRVs to be bound simultaneously as long as the app
            // doesn't read from the SRV. RTV/SRV simultaneous binding is not allowed in 12 
            // and so we need to mark the SRV dirty flags when this happens so that we can 
            // later choose to "hide" the SRV bindings in this case
            if (pPrevResource)
            {
                pPrevResource->GetRTVBindingTracker().Unbind(renderTargetIndex);
                MarkSRVIndicesDirty(pPrevResource->GetSRVBindingTracker().GetBindingIndices());
            }

            if (pNewResource)
            {
                pNewResource->GetRTVBindingTracker().Bind(renderTargetIndex);
                MarkSRVIndicesDirty(pNewResource->GetSRVBindingTracker().GetBindingIndices());

                bool bDisableAlpha = pNewResource->IsAlphaChannelDisabled();
                UINT rtvShaderComponentMapping = ConvertFormatToShaderResourceViewComponentMapping(pNewResource->GetD3DFormat());
                if (m_rtvShaderComponentMapping[renderTargetIndex] != rtvShaderComponentMapping)
                {
                    m_rtvShaderComponentMapping[renderTargetIndex] = rtvShaderComponentMapping;
                    m_dirtyFlags.BlendState = true;
                }

                if (bDisableAlpha != m_alphaDisabledMask[renderTargetIndex])
                {
                    m_alphaDisabledMask[renderTargetIndex] = bDisableAlpha;
                    m_dirtyFlags.BlendState = true;
                }

                m_rasterStates.SetSwapRBOutputChannelsFlag(renderTargetIndex, pNewResource->NeedsSwapRBOutputChannels());
            }
        }

        m_pRenderTargets[renderTargetIndex] = boundRenderTarget;

        m_numBoundRenderTargets = 0;
        if (m_rtSetMask)
        {
            BitScanReverse(&m_numBoundRenderTargets, m_rtSetMask);
            m_numBoundRenderTargets += 1; // + 1 because of zero index
        }

        m_dirtyFlags.RenderTargets = true;
    }

    void PixelStage::SetRasterState(DWORD dwState, DWORD dwValue)
    {
        switch (dwState)
        {
            // Rasterizer states  
        case D3DRS_FILLMODE:
            if (dwValue < 1 || dwValue > 3) dwValue = D3DFILL_SOLID;
            m_rasterStates.SetFillMode(static_cast<D3DFILLMODE>(dwValue));
            m_rasterizerStateID.FillMode = dwValue;
            m_dirtyFlags.RasterizerState = true;
            break;
        case D3DRS_CULLMODE:
            if (dwValue < 1 || dwValue > 3) dwValue = D3DCULL_NONE;
            m_rasterizerStateID.CullMode = dwValue;
            m_dirtyFlags.RasterizerState = true;
            break;
        case D3DRS_DEPTHBIAS:
            m_rasterizerStateID.DepthBias = *(FLOAT*)(&dwValue);
            m_dirtyFlags.RasterizerState = true;
            break;
        case D3DRS_SLOPESCALEDEPTHBIAS:
            m_rasterizerStateID.SlopeScaleDepthBias = *(FLOAT*)(&dwValue);
            m_dirtyFlags.RasterizerState = true;
            break;
        case D3DRS_MULTISAMPLEANTIALIAS:
            m_rasterizerStateID.MultiSampleAntiAlias = dwValue;
            m_dirtyFlags.RasterizerState = true;
            break;
        case D3DRS_ANTIALIASEDLINEENABLE:
            m_rasterizerStateID.AntialiasedLineEnable = dwValue;
            m_dirtyFlags.RasterizerState = true;
            break;
        case D3DRS_CLIPPING:
            m_rasterizerStateID.DepthClipEnable = dwValue;
            m_dirtyFlags.RasterizerState = true;
            break;
        default:
            Check9on12(false);
        }
    }

    void PixelStage::SetDepthStencilState(Device& device, DWORD dwState, DWORD dwValue)
    {
        auto& context = device.GetContext();

        switch (dwState)
        {
            // DepthStencil states  
        case D3DRS_ZENABLE:
            m_depthStencilStateID.ZEnable = dwValue;
            m_dirtyFlags.DepthStencilState = true;
            m_dirtyFlags.RasterizerState = true;
            break;
        case D3DRS_ZWRITEENABLE:
        {
            m_depthStencilStateID.ZWriteEnable = dwValue;
            D3D12_DSV_FLAGS oldState = m_dsvRWType;
            if (m_depthStencilStateID.ZWriteEnable)
            {
                m_dsvRWType &= ~D3D12_DSV_FLAG_READ_ONLY_DEPTH;
            }
            else
            {
                m_dsvRWType |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
            }
            if (m_dsvRWType != oldState)
            {
                m_dirtyFlags.RenderTargets = true;

                // Hide or unhide the SRVs of the current depth stencil as appropriate
                if (m_pDepthStencil)
                {
                    MarkSRVIndicesDirty(m_pDepthStencil->GetSRVBindingTracker().GetBindingIndices());
                }
            }

            m_dirtyFlags.DepthStencilState = true;
            break;
        }
        case D3DRS_ZFUNC:
            m_depthStencilStateID.ZFunc = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_STENCILENABLE:
            m_depthStencilStateID.StencilEnable = dwValue;
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_STENCILFAIL:
            m_depthStencilStateID.StencilFail = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_STENCILZFAIL:
            m_depthStencilStateID.StencilZFail = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_STENCILPASS:
            m_depthStencilStateID.StencilPass = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_STENCILFUNC:
            m_depthStencilStateID.StencilFunc = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_STENCILREF:
            m_depthStencilStateID.StencilRef = static_cast<UINT8>(dwValue);
            m_dirtyFlags.DepthStencilState = true;
            context.OMSetStencilRef(m_depthStencilStateID.StencilRef);
            break;
        case D3DRS_STENCILMASK:
            m_depthStencilStateID.StencilMask = static_cast<UINT8>(dwValue);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_STENCILWRITEMASK:
            {
                m_depthStencilStateID.StencilWriteMask = static_cast<UINT8>(dwValue);
                D3D12_DSV_FLAGS oldState = m_dsvRWType;
                if (m_depthStencilStateID.StencilWriteMask == 0)
                {
                    m_dsvRWType |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
                }
                else
                {
                    m_dsvRWType &= ~D3D12_DSV_FLAG_READ_ONLY_STENCIL;
                }
                if (m_dsvRWType != oldState)
                {
                    m_dirtyFlags.RenderTargets = true;
                }
                m_dirtyFlags.DepthStencilState = true;
                break;
            }
        case D3DRS_TWOSIDEDSTENCILMODE:
            m_depthStencilStateID.TwoSidedStencil = dwValue;
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_CCW_STENCILFAIL:
            m_depthStencilStateID.CCWStencilFail = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_CCW_STENCILZFAIL:
            m_depthStencilStateID.CCWStencilZFail = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_CCW_STENCILPASS:
            m_depthStencilStateID.CCWStencilPass = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        case D3DRS_CCW_STENCILFUNC:
            m_depthStencilStateID.CCWStencilFunc = (dwValue - 1);
            m_dirtyFlags.DepthStencilState = true;
            break;
        default:
            Check9on12(false);
        }
    }

    void PixelStage::SetBlendState(Device& device, DWORD dwState, DWORD dwValue)
    {
        auto& context = device.GetContext();

        switch (dwState)
        {
            // Blend states  
        case D3DRS_ALPHABLENDENABLE:
            m_blendStateID.AlphaBlendEnable = dwValue;
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_SRCBLEND:
            m_blendStateID.SrcBlend = dwValue;
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_DESTBLEND:
            m_blendStateID.DstBlend = dwValue;
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_BLENDOP:
            m_blendStateID.BlendOp = dwValue;
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_BLENDFACTOR:
            m_blendFactor = dwValue;
            m_dirtyFlags.BlendState = true;
            {
                DirectX::XMFLOAT4 factor = ARGBToUNORMFloat(m_blendFactor);
                context.OMSetBlendFactor((float*)&factor);
            }
            break;
        case D3DRS_SEPARATEALPHABLENDENABLE:
            m_blendStateID.SeparateAlphaBlend = dwValue;
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_SRCBLENDALPHA:
            m_blendStateID.SrcBlendAlpha = dwValue;
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_DESTBLENDALPHA:
            m_blendStateID.DstBlendAlpha = dwValue;
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_BLENDOPALPHA:
            assert(dwValue <= D3DBLENDOP_MAX);
            // Assuming the app is asking for "default" behavior
            if (dwValue == 0)
            {
                dwValue = D3DBLENDOP_ADD;
            }

            m_blendStateID.BlendOpAlpha = dwValue;
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_COLORWRITEENABLE:
            m_blendStateID.ColorWriteMask0 = static_cast<BYTE>(dwValue);
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_COLORWRITEENABLE1:
            m_blendStateID.ColorWriteMask1 = static_cast<BYTE>(dwValue);
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_COLORWRITEENABLE2:
            m_blendStateID.ColorWriteMask2 = static_cast<BYTE>(dwValue);
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_COLORWRITEENABLE3:
            m_blendStateID.ColorWriteMask3 = static_cast<BYTE>(dwValue);
            m_dirtyFlags.BlendState = true;
            break;
            //Special flags for Alpha To Coverage
        case D3DRS_POINTSIZE:
            if (dwValue == D3DFMT_A2M0)
            {
                SetAlphaToCoverageEnabled( false );
            }
            else if (dwValue == D3DFMT_A2M1)
            {
                SetAlphaToCoverageEnabled( true );
            }
            break;
        case D3DRS_ADAPTIVETESS_Y:
            if (dwValue == 0)
            {
                SetAlphaToCoverageEnabled( false );
            }
            else if (dwValue == D3DFMT_ATOC)
            {
                SetAlphaToCoverageEnabled( true );
            }
            break;
        default:
            Check9on12(false);
        }
    }

    void PixelStage::SetPSExtensionState(DWORD dwState, DWORD dwValue)
    {
        switch (dwState)
        {
        case D3DRS_ALPHATESTENABLE:
            m_rasterStates.SetAlphaTestEnable(dwValue == TRUE);
            break;
        case D3DRS_ALPHAFUNC:
            m_rasterStates.SetAlphaFunc(static_cast<D3DCMPFUNC>(dwValue));
            break;
        case D3DRS_ALPHAREF:
            m_PSExtension.fAlphaRef = ConvertToFloat(dwValue);
            m_dirtyFlags.PSExtension = true;
            break;
        case D3DRS_FOGENABLE:
            m_rasterStates.SetFogEnable(dwValue == TRUE);
            break;
        case D3DRS_FOGCOLOR:
            ConvertToRGB(dwValue, m_PSExtension.vFogColor);
            m_dirtyFlags.PSExtension = true;
            break;
        case D3DRS_FOGTABLEMODE:
            m_rasterStates.SetFogTableMode(dwValue);
            m_dirtyFlags.PSExtension = true; // Needed to compute m_PSExtension.fFogDistInv  
            break;
        case D3DRS_FOGSTART:
            m_PSExtension.fFogStart = *(FLOAT*)(&dwValue);
            AssertFloatNotInfOrNan(m_PSExtension.fFogStart);
            m_dirtyFlags.PSExtension = true;
            break;
        case D3DRS_FOGEND:
            m_PSExtension.fFogEnd = *(FLOAT*)(&dwValue);
            AssertFloatNotInfOrNan(m_PSExtension.fFogEnd);
            m_dirtyFlags.PSExtension = true;
            break;
        case D3DRS_FOGDENSITY:
            m_PSExtension.fFogDensity = *(FLOAT*)(&dwValue);
            AssertFloatNotInfOrNan(m_PSExtension.fFogDensity);
            m_dirtyFlags.PSExtension = true;
            break;
        case D3DRS_SHADEMODE:
            m_rasterStates.SetShadeMode(static_cast<D3DSHADEMODE>(dwValue));
            break;
        default:
            Check9on12(false);
        }
    }

    void PixelStage::SetPSExtension2State(DWORD dwStage, DWORD dwState, DWORD dwValue)
    {
        switch (dwState)
        {
        case D3DTSS_BUMPENVMAT00:
        case D3DTSS_BUMPENVMAT01:
        case D3DTSS_BUMPENVMAT10:
        case D3DTSS_BUMPENVMAT11:
            m_PSExtension2.vBumpEnvMat[dwStage][dwState - D3DTSS_BUMPENVMAT00] = *(FLOAT*)(&dwValue);
            AssertFloatNotInfOrNan(m_PSExtension2.vBumpEnvMat[dwStage][dwState - D3DTSS_BUMPENVMAT00]);
            m_dirtyFlags.PSExtension2 = 1;
            break;

        case D3DTSS_BUMPENVLSCALE:
        case D3DTSS_BUMPENVLOFFSET:
            m_PSExtension2.vBumpEnvL[dwStage][dwState - D3DTSS_BUMPENVLSCALE] = *(FLOAT*)(&dwValue);
            AssertFloatNotInfOrNan(m_PSExtension2.vBumpEnvL[dwStage][dwState - D3DTSS_BUMPENVLSCALE]);
            m_dirtyFlags.PSExtension2 = 1;
            break;
        default:
            Check9on12(false);
        }
    }

    void PixelStage::SetPreTransformedVertexMode(bool drawingPretransformedVerts)
    {
        if (static_cast<UINT>(drawingPretransformedVerts) != m_rasterizerStateID.DrawingPreTransformedVertices)
        {
            m_rasterizerStateID.DrawingPreTransformedVertices = drawingPretransformedVerts;
            m_dirtyFlags.RasterizerState = true;
        }
    }

    void PixelStage::ResolveRenderTargets(Device &device, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, bool bDSVBound)
    {
        psoDesc.NumRenderTargets = m_numBoundRenderTargets;

        D3D12TranslationLayer::RTV* ppRTVs[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        D3D12TranslationLayer::DSV* pDSV = nullptr;

        if (bDSVBound)
        {
            pDSV = m_pDepthStencil->GetDepthStencilView(m_dsvRWType);
            Check9on12(pDSV != nullptr);
        }

        Check9on12(psoDesc.NumRenderTargets <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
        for (UINT i = 0; i < psoDesc.NumRenderTargets && i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        {
            if (m_pRenderTargets[i].m_pResource)
            {
                ppRTVs[i] = m_pRenderTargets[i].m_pResource->GetRenderTargetView(m_pRenderTargets[i].m_subresource, m_SRGBWriteEnabled);
                psoDesc.RTVFormats[i] = m_pRenderTargets[i].m_pResource ? m_pRenderTargets[i].m_pResource->GetViewFormat(m_SRGBWriteEnabled) :
                    DXGI_FORMAT_UNKNOWN;
            }
        }

        for (UINT i = psoDesc.NumRenderTargets; i <D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        {
            psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        }

        UINT renderTargetsToSet = m_numBoundRenderTargets;
        if (m_numBoundRenderTargets == 0)
        {
            // Unbind all
            renderTargetsToSet = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
        }

        device.GetContext().OMSetRenderTargets(ppRTVs, renderTargetsToSet, pDSV);
    }

    Resource* PixelStage::FindFirstValidBoundWritableResource()
    {
        // Find the first valid writable resource to determine the sample count/quality
        Resource *pValidWritableResource = nullptr;
        if (m_rtSetMask)
        {
            DWORD index = 0;
            BitScanReverse(&index, m_rtSetMask);
            pValidWritableResource = m_pRenderTargets[index].m_pResource;
        }
        else
        {
            pValidWritableResource = m_pDepthStencil;
        }
        return pValidWritableResource;
    }

    void PixelStage::SetAlphaToCoverageEnabled( const bool enabled )
    {
        m_blendStateID.AlphaToCoverageEnable = enabled ? 1 : 0;
        m_dirtyFlags.BlendState = true;
    }

    HRESULT PixelStage::ResolveDeferredState(Device &device, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, UINT& textureDirtyMaskToKeep)
    {
        HRESULT hr = S_OK;

        auto& context = device.GetContext();

        // Need to check if multisampling is enabled first for the logic that determines if 
        // the depth stencil should be hid
        if (m_dirtyFlags.RenderTargets)
        {
            Resource *pValidWritableResource = FindFirstValidBoundWritableResource();
            Check9on12(pValidWritableResource != nullptr);

            psoDesc.SampleDesc = pValidWritableResource->GetDesc().SampleDesc;
            bool multisampleEnabled = psoDesc.SampleDesc.Count > 1;
            if (multisampleEnabled != (m_rasterizerStateID.MultiSampleRenderTarget != 0))
            {
                m_rasterizerStateID.MultiSampleRenderTarget = multisampleEnabled;
                m_dirtyFlags.RasterizerState = true;
            }
        }

        // ForcedSampleCount requires that there cannot be a bound depth stencil view. CS:GO causes a case
        // where a ForcedSampleCount is required AND a depth buffer is bound BUT because depth and stencil 
        // are disabled, we can work around this by temporarily hiding the depth stencil and rebinding it
        // once ForcedSampleCount is off
        // However, Tree of Savior binds a render target which is a different dimension from their still-bound
        // depth buffer. So, for simplicity, we'll just hide the depth buffer any time it's not needed.
        const bool needToHideDepthStencil = GetDepthStencilStateID().DepthStencilViewBindingOptional();
        if (needToHideDepthStencil != m_hideDepthStencil)
        {
            m_hideDepthStencil = needToHideDepthStencil;
            m_dirtyFlags.DepthStencil = true;
            m_dirtyFlags.RenderTargets = true;
        }

        bool bDSVBound = (m_pDepthStencil != nullptr) && !m_hideDepthStencil;

        if (m_dirtyFlags.PSExtension)
        {
            if (m_PSExtension.fFogEnd - m_PSExtension.fFogStart != 0.0f)
            {
                m_PSExtension.fFogDistInv = 1.0f / (m_PSExtension.fFogEnd - m_PSExtension.fFogStart);
            }
            else
            {
                m_PSExtension.fFogDistInv = 0.0f;
            }
            AssertFloatNotInfOrNan(m_PSExtension.fFogDistInv);

            device.GetConstantsManager().UpdatePixelShaderExtension(
                ShaderConv::CB_PS_EXT,
                &m_PSExtension,
                sizeof(m_PSExtension));
        }

        if (m_dirtyFlags.PSExtension2)
        {
            device.GetConstantsManager().UpdatePixelShaderExtension(
                ShaderConv::CB_PS_EXT2,
                &m_PSExtension2,
                sizeof(m_PSExtension2));
        }

        if (m_dirtyFlags.Samplers)
        {
            for (UINT i = 0; i < MAX_SAMPLERS_STAGES; i++)
            {
                if (m_dirtyFlags.Samplers & BIT(i))
                {
                    DWORD shaderRegister;
                    D3D12TranslationLayer::EShaderStage shaderStage;

                    D3D12TranslationLayer::Sampler* sampler = m_samplerCache.GetSampler(device, m_samplerStateIDs[i]);
                    ConvertDX9TextureIndexToShaderStageAndRegisterIndex(i, shaderStage, shaderRegister);

                    if (shaderStage == D3D12TranslationLayer::e_PS)
                    {
                        context.SetSamplers<D3D12TranslationLayer::e_PS>(shaderRegister, 1, &sampler);
                    }
                    else
                    {
                        context.SetSamplers<D3D12TranslationLayer::e_VS>(shaderRegister, 1, &sampler);
                    }

                }
            }
        }

        if (m_dirtyFlags.DepthStencil)
        {
            if (bDSVBound)
            {
                psoDesc.DSVFormat = m_pDepthStencil->DepthStencilViewFormat();
            }
            else
            {
                psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
            }
        }

        // If Alpha to coverage is enabled, we must disable Alpha Testing and set AlphaFunc to D3DCMP_ALWAYS.
        // If it is disabled, we need to make sure that we revert the computed raster states to the values requested by the app.
        // Setting AlphaTestEnable or AlphaFunc states should disable the PixelShader flag, so we need to do this before the
        // PixelShader block below.
        if (m_dirtyFlags.BlendState || m_dirtyFlags.PixelShader)
        {
            const auto& rasterStates = m_rasterStates.GetRasterState();
            if (rasterStates.AlphaTestEnable && m_blendStateID.AlphaToCoverageEnable)
            {
                m_computedRasterStates.m_alphaTestEnable = false;
                m_computedRasterStates.m_alphaFunc = D3DCMP_ALWAYS;

                m_dirtyFlags.PixelShader = true;
            }
            else if(m_computedRasterStates.m_alphaTestEnable != rasterStates.AlphaTestEnable
                     || m_computedRasterStates.m_alphaFunc != rasterStates.AlphaFunc )
            {
                // Restore the values requested by the app
                m_computedRasterStates.m_alphaTestEnable = rasterStates.AlphaTestEnable;
                m_computedRasterStates.m_alphaFunc = rasterStates.AlphaFunc;

                m_dirtyFlags.PixelShader = true;
            }
        }

        if (m_dirtyFlags.PixelShader)
        {
            Check9on12(m_pCurrentPS);
            D3D12VertexShader* pVS = device.GetPipelineState().GetVertexStage().GetCurrentD3D12VertexShader();
            D3D12GeometryShader* pGS = device.GetPipelineState().GetVertexStage().GetCurrentD3D12GeometryShader();

            if (pVS != nullptr)
            {
                ShaderConv::RasterStates mergedRasterStates = m_rasterStates.GetRasterState();  // copy by value
                mergedRasterStates.AlphaTestEnable = m_computedRasterStates.m_alphaTestEnable;
                mergedRasterStates.AlphaFunc = m_computedRasterStates.m_alphaFunc;

                m_pCurrentD3D12PixelShader = &m_pCurrentPS->GetD3D12Shader(
                    mergedRasterStates,
                    pGS ? pGS->m_gsOutputDecls : pVS->m_vsOutputDecls,
                    pGS ? (D3D12Shader &)*pGS  : (D3D12Shader &)*pVS);

                psoDesc.PS = m_pCurrentD3D12PixelShader->GetUnderlying()->GetByteCode();
                if (psoDesc.PS.pShaderBytecode == nullptr)
                {
                    hr = E_FAIL;
                    CHECK_HR(hr);
                }

                for (UINT i = 0; i < ARRAYSIZE(m_pCurrentD3D12PixelShader->m_inlineConsts); i++)
                {
                    for (auto &data : m_pCurrentD3D12PixelShader->m_inlineConsts[i])
                    {
                        device.GetConstantsManager().GetPixelShaderConstants().GetConstantBufferData((ShaderConv::eConstantBuffers)i).SetData((Float4*)&data.Value, data.RegIndex / 4, 1);
                    }
                }
            }
            else
            {
                hr = E_FAIL;
                CHECK_HR(hr);
            }
        }

        textureDirtyMaskToKeep = 0;
        if (m_dirtyFlags.Textures)
        {
            UINT textureMask = m_dirtyFlags.Textures;
            for (UINT i = 0; textureMask > 0; textureMask = textureMask >> 1, ++i)
            {
                if (textureMask & BIT(0))
                {
                    DWORD shaderRegister;
                    D3D12TranslationLayer::EShaderStage shaderStage;
                    ConvertDX9TextureIndexToShaderStageAndRegisterIndex(i, shaderStage, shaderRegister);

                    D3D12TranslationLayer::SRV* pSRV = nullptr;
                    Resource* pResource = m_shaderResources[i];
                    if (pResource)
                    {   
                        // Certain apps expect the driver to automatically unbind resources as SRVs when they are bound as RTVs
                        // Examples inclue Alan Wake and Valkyrie Chronicles
                        bool HideSRV = pResource->GetDSVBindingTracker().IsBound() && GetDepthStencilStateID().ZWriteEnable;

                        // resources with INTZ surface format, set ZWriteEnable to false instead of hiding SRV
                        if (HideSRV && (pResource->GetD3DFormat() == D3DFMT_INTZ))
                        {
                            HideSRV = false;
                            SetDepthStencilState(device, D3DRS_ZWRITEENABLE, 0);
                            device.GetPipelineState().SetIntzRestoreZWrite(true);
                        }

                        if (!HideSRV && pResource->GetRTVBindingTracker().IsBound())
                        {
                            HideSRV = true;

                            // Some apps want to be able to read from the currently bound render target.
                            // Examples include Tree of Savior
                            D3D12TranslationLayer::TDeclVector* pDecls = nullptr;
                            if (shaderStage == D3D12TranslationLayer::e_PS && m_pCurrentD3D12PixelShader)
                            {
                                pDecls = &m_pCurrentD3D12PixelShader->GetUnderlying()->m_ResourceDecls;
                            }
                            else if (shaderStage == D3D12TranslationLayer::e_VS)
                            {
                                auto pVS = device.GetPipelineState().GetVertexStage().GetCurrentD3D12VertexShader();
                                if (pVS)
                                {
                                    pDecls = &pVS->GetUnderlying()->m_ResourceDecls;
                                }
                            }
                            if (pDecls)
                            {
                                if (shaderRegister < pDecls->size() && (*pDecls)[shaderRegister] != D3D12TranslationLayer::RESOURCE_DIMENSION::UNKNOWN)
                                {
                                    pResource = pResource->GetBackingPlainResource();
                                    HideSRV = false;
                                }
                            }
                            textureDirtyMaskToKeep |= BIT(i); // Set this dirty bit again to check if we should do this copy next time
                        }

                        if (HideSRV)
                        {
                            pSRV = nullptr;
                        }
                        else
                        {
                            DWORD flags = Resource::ShaderResourceViewFlags::None;
                            if (GetSamplerID(i).MipFilter == D3DDDITEXF_NONE)
                            {
                                flags |= Resource::ShaderResourceViewFlags::DisableMips;
                            }
                            if (m_SRGBTexture[i])
                            {
                                flags |= Resource::ShaderResourceViewFlags::SRGBEnabled;
                            }
                            pSRV = pResource->GetShaderResourceView(flags);
                        }
                    }

                    if (shaderStage == D3D12TranslationLayer::e_PS)
                    {
                        context.SetShaderResources<D3D12TranslationLayer::e_PS>(shaderRegister, 1, &pSRV);
                    }
                    else
                    {
                        context.SetShaderResources<D3D12TranslationLayer::e_VS>(shaderRegister, 1, &pSRV);
                    }
                }
            }
        }

        if (m_dirtyFlags.DepthStencilState)
        {
            psoDesc.DepthStencilState = ConvertDepthStencilState(m_depthStencilStateID, bDSVBound);
        }

        if (m_dirtyFlags.RenderTargets)
        {
            ResolveRenderTargets(device, psoDesc, bDSVBound);
        }

        if (m_dirtyFlags.BlendState)
        {
            psoDesc.BlendState = ConvertBlendState(m_blendStateID, m_alphaDisabledMask, m_rtvShaderComponentMapping);

            UINT sampleMask = m_multiSampleMask;
            // Some apps such as Halo set a sample mask of 0. Ignore the app setting unless
            // MSAA is actaully enabled.
            if (psoDesc.SampleDesc.Count == 1 && m_multiSampleMask == 0)
            {
                sampleMask = MAXUINT;
            }
            psoDesc.SampleMask = sampleMask;
        }

        if (m_dirtyFlags.RasterizerState)
        {
            psoDesc.RasterizerState = ConvertRasterizerState(m_rasterizerStateID,  bDSVBound && psoDesc.DepthStencilState.DepthEnable, psoDesc.DSVFormat);
        }

        return hr;
    }

    D3D12TranslationLayer::Sampler* PixelStage::SamplerCache::GetSampler(Device& device, SamplerStateID& id)
    {
        auto result = m_map.find(id);
        if (result == m_map.end())
        {
            D3D12_SAMPLER_DESC createSamplerDesc = ConvertSampler(id, device.m_Options19.AnisoFilterWithPointMipSupported);

            m_map[id] = std::unique_ptr<D3D12TranslationLayer::Sampler>(new D3D12TranslationLayer::Sampler(&device.GetContext(), createSamplerDesc));

            return m_map[id].get();
        }
        else
        {
            return result->second.get();
        }
    }

};