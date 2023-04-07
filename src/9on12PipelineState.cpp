// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY SetRenderState(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_RENDERSTATE* pRenderStateArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pRenderStateArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetPipelineState().SetRenderState(*pDevice, (DWORD)pRenderStateArg->State, (DWORD)pRenderStateArg->Value);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetTextureStageState(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_TEXTURESTAGESTATE* pTextureStageState)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pTextureStageState == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        PipelineState& pipelineState = pDevice->GetPipelineState();
        DWORD dwStage = (DWORD)pTextureStageState->Stage;
        DWORD dwState = (DWORD)pTextureStageState->State;
        DWORD dwValue = (DWORD)pTextureStageState->Value;

        // Resolve sampler stage
        dwStage = MapSamplerStage9on12(dwStage);

        if (dwStage >= MAX_SAMPLERS_STAGES || dwState >= MAX_D3DTSS)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        if (D3DTSS_ADDRESS == dwState)
        {
            pipelineState.SetTextureStageState(dwStage, D3DDDITSS_ADDRESSU, dwValue);
            pipelineState.SetTextureStageState(dwStage, D3DDDITSS_ADDRESSV, dwValue);
        }
        else
        {
            pipelineState.SetTextureStageState(dwStage, dwState, dwValue);
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    void PipelineState::SetRenderState(Device& device, DWORD dwState, DWORD dwValue)
    {
        //TODO: early out for redundant state sets

        // Set the render state value  
        m_dwRenderStates[dwState] = dwValue;
        switch (dwState)
        {
            // Rasterizer states  
        case D3DRS_FILLMODE:
        case D3DRS_CULLMODE:
        case D3DRS_DEPTHBIAS:
        case D3DRS_SLOPESCALEDEPTHBIAS:
        case D3DRS_MULTISAMPLEANTIALIAS:
        case D3DRS_ANTIALIASEDLINEENABLE:
        case D3DRS_CLIPPING:
            m_pixelStage.SetRasterState(dwState, dwValue);
            break;
            // DepthStencil states  
        case D3DRS_ZENABLE:
        case D3DRS_ZWRITEENABLE:
        case D3DRS_ZFUNC:
        case D3DRS_STENCILENABLE:
        case D3DRS_STENCILFAIL:
        case D3DRS_STENCILZFAIL:
        case D3DRS_STENCILPASS:
        case D3DRS_STENCILFUNC:
        case D3DRS_STENCILREF:
        case D3DRS_STENCILMASK:
        case D3DRS_STENCILWRITEMASK:
        case D3DRS_TWOSIDEDSTENCILMODE:
        case D3DRS_CCW_STENCILFAIL:
        case D3DRS_CCW_STENCILZFAIL:
        case D3DRS_CCW_STENCILPASS:
        case D3DRS_CCW_STENCILFUNC:
            m_pixelStage.SetDepthStencilState(device, dwState, dwValue);
            break;
            // Blend states  
        case D3DRS_ALPHABLENDENABLE:
        case D3DRS_SRCBLEND:
        case D3DRS_DESTBLEND:
        case D3DRS_BLENDOP:
        case D3DRS_BLENDFACTOR:
        case D3DRS_SEPARATEALPHABLENDENABLE:
        case D3DRS_SRCBLENDALPHA:
        case D3DRS_DESTBLENDALPHA:
        case D3DRS_BLENDOPALPHA:
        case D3DRS_COLORWRITEENABLE:
        case D3DRS_COLORWRITEENABLE1:
        case D3DRS_COLORWRITEENABLE2:
        case D3DRS_COLORWRITEENABLE3:
            m_pixelStage.SetBlendState(device, dwState, dwValue);
            break;
        case D3DRS_MULTISAMPLEMASK:
            m_pixelStage.SetMultiSampleMask(dwValue);
            m_dirtyFlags.BlendState = true;
            break;
        case D3DRS_ALPHATESTENABLE:
        case D3DRS_ALPHAFUNC:
        case D3DRS_ALPHAREF:
        case D3DRS_FOGENABLE:
        case D3DRS_FOGCOLOR:
        case D3DRS_FOGTABLEMODE:
        case D3DRS_FOGSTART:
        case D3DRS_FOGEND:
        case D3DRS_FOGDENSITY:
        case D3DRS_SHADEMODE:
            m_pixelStage.SetPSExtensionState(dwState, dwValue);
            break;
        case D3DRS_WRAP0:
        case D3DRS_WRAP1:
        case D3DRS_WRAP2:
        case D3DRS_WRAP3:
        case D3DRS_WRAP4:
        case D3DRS_WRAP5:
        case D3DRS_WRAP6:
        case D3DRS_WRAP7:
            m_rasterStates.SetPSSamplerTexCoordWrap(dwState - D3DRS_WRAP0 + 0, static_cast<BYTE>(dwValue));
            break;
        case D3DRS_WRAP8:
        case D3DRS_WRAP9:
        case D3DRS_WRAP10:
        case D3DRS_WRAP11:
        case D3DRS_WRAP12:
        case D3DRS_WRAP13:
        case D3DRS_WRAP14:
        case D3DRS_WRAP15:
            m_rasterStates.SetPSSamplerTexCoordWrap(dwState - D3DRS_WRAP8 + 8, static_cast<BYTE>(dwValue));
            break;
        case D3DRS_POINTSIZE:
            if (dwValue == RESZ_CODE)
            {
                auto pSRVZero = m_pixelStage.GetSRV(0);
                auto pDSV = m_pixelStage.GetDepthStencil();
                if (pSRVZero && pDSV &&
                    pSRVZero->GetLogicalDesc().Format == pDSV->GetLogicalDesc().Format)
                {
                    if (pDSV->GetLogicalDesc().SampleDesc.Count > 1)
                    {
                        device.GetContext().ResourceResolveSubresource(
                            pSRVZero->GetUnderlyingResource(), 0,
                            pDSV->GetUnderlyingResource(), 0,
                            pSRVZero->GetLogicalDesc().Format);
                    }
                    else
                    {
                        device.GetContext().ResourceCopyRegion(
                            pSRVZero->GetUnderlyingResource(), 0, 0, 0, 0,
                            pDSV->GetUnderlyingResource(), 0, nullptr);
                    }
                }
                break;
            }
            else if (dwValue == D3DFMT_A2M1 || dwValue == D3DFMT_A2M0)
            {
                //Special values for Alpha To Coverage
                m_pixelStage.SetBlendState( device, dwState, dwValue );
            }
            // Fallthrough
        case D3DRS_POINTSIZE_MIN:
        case D3DRS_POINTSIZE_MAX:
            m_vertexStage.SetPointSize(dwState, dwValue);
            break;
        case D3DRS_POINTSPRITEENABLE:
            m_rasterStates.SetPointSpriteEnable(dwValue != FALSE);
            break;
        case D3DRS_CLIPPLANEENABLE:
            m_rasterStates.SetUserClipplanes(dwValue);
            break;
        case D3DRS_SRGBWRITEENABLE:
            m_pixelStage.SetSRGBWriteEnable(dwValue > 0);
            break;
        case D3DRS_SCISSORTESTENABLE:
            m_vertexStage.SetScissorTestEnabled(device, dwValue > 0);
            break;
        case D3DRS_ADAPTIVETESS_Y:
            m_pixelStage.SetBlendState( device, dwState, dwValue );
            break;
            //  
            // map legacy modes with one-to-one mappings to texture stage 0  
            //  
        case D3DRENDERSTATE_TEXTUREMAPBLEND:
            if (RegistryConstants::g_cBreakOnMissingDDI)
            {
                Check9on12(false);
            }
#if 0
            this->MapLegacyTextureBlend(dwValue); // Map legacy texture blend modes  
#endif
            break;
        case D3DRENDERSTATE_TEXTUREADDRESS:
            this->SetTextureStageState(0, D3DTSS_ADDRESSU, dwValue);
            this->SetTextureStageState(0, D3DTSS_ADDRESSV, dwValue);
            break;
        case D3DRENDERSTATE_TEXTUREADDRESSU:
            this->SetTextureStageState(0, D3DTSS_ADDRESSU, dwValue);
            break;
        case D3DRENDERSTATE_TEXTUREADDRESSV:
            this->SetTextureStageState(0, D3DTSS_ADDRESSV, dwValue);
            break;
        case D3DRENDERSTATE_MIPMAPLODBIAS:
            this->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, dwValue);
            break;
        case D3DRENDERSTATE_BORDERCOLOR:
            this->SetTextureStageState(0, D3DTSS_BORDERCOLOR, dwValue);
            break;
        case D3DRENDERSTATE_ANISOTROPY:
            this->SetTextureStageState(0, D3DTSS_MAXANISOTROPY, dwValue);
            break;
        case D3DRENDERSTATE_TEXTUREMAG:
        case D3DRENDERSTATE_TEXTUREMIN:
            if (RegistryConstants::g_cBreakOnMissingDDI)
            {
                Check9on12(false);
            }
#if 0
            this->MapLegacyTextureFilter(dwState, dwValue);
#endif
            break;
        case D3DRENDERSTATE_TEXTUREHANDLE:
            this->SetTextureStageState(0, D3DTSS_TEXTUREMAP, dwValue);
            break;
        case D3DRENDERSTATE_WRAPU:
            this->SetRenderState(device, D3DRENDERSTATE_WRAP0, (m_dwRenderStates[D3DRENDERSTATE_WRAP0] & ~D3DWRAP_U) | (dwValue ? D3DWRAP_U : 0));
            break;
        case D3DRENDERSTATE_WRAPV:
            this->SetRenderState(device, D3DRENDERSTATE_WRAP0, (m_dwRenderStates[D3DRENDERSTATE_WRAP0] & ~D3DWRAP_V) | (dwValue ? D3DWRAP_V : 0));
            break;
        case D3DRENDERSTATE_COLORKEYENABLE:
            m_rasterStates.SetColorKeyEnable(dwValue != FALSE);
            Check9on12(dwValue == FALSE);
            break;
        case D3DRENDERSTATE_COLORKEYBLENDENABLE:
            m_rasterStates.SetColorKeyBlendEnable(dwValue != FALSE);
            break;
        case D3DRENDERSTATE_SCENECAPTURE:
            break;
        }
    }

    void PipelineState::SetTextureStageState(DWORD dwStage, DWORD dwState, DWORD dwValue)
    {
        //TODO: early out on redundant sets

        // Set the texture stage state
        if (dwStage >= _countof(m_dwTextureStageStates))
        {
            Check9on12(false);
            return;
        }
        if (dwState >= _countof(m_dwTextureStageStates[dwStage]))
        {
            Check9on12(false);
            return;
        }
        if (dwStage >= MAX_SAMPLERS_STAGES)
        {
            Check9on12(false);
            return;
        }

        if (dwStage == DMAP_SAMPLER)
        {
            // DMAP_SAMPLER should be handled by the FF converter, the runtime still calls 
            // functions to modify this sampler on device creation, but we can safely ignore it
            return;
        }

        // check for integer bitfield overflow causing issues with some apps submitting invalid
        // sampler state values. 
        if ((dwState == D3DTSS_MAGFILTER && dwValue >= 16u) ||
            (dwState == D3DTSS_MINFILTER && dwValue >= 16u) ||
            (dwState == D3DTSS_MIPFILTER && dwValue >= 16u) ||
            (dwState == D3DTSS_ADDRESSU && dwValue >= 8u) ||
            (dwState == D3DTSS_ADDRESSV && dwValue >= 8u) ||
            (dwState == D3DTSS_ADDRESSW && dwValue >= 8u) ||
            (dwState == D3DTSS_MAXANISOTROPY && dwValue >= 256u))
        {
	        Check9on12(false);
            return;
        }

        m_dwTextureStageStates[dwStage][dwState] = dwValue;

        SamplerStateID& samplerID = m_pixelStage.GetSamplerID(dwStage);

        const UINT dirtyMask = BIT(dwStage);

        switch (dwState)
        {
        case D3DTSS_SRGBTEXTURE:
            m_pixelStage.SetSRGBTexture(dwStage, dwValue > 0);
            break;

        case D3DTSS_MAGFILTER:
            samplerID.MagFilter = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_MINFILTER:
            samplerID.MinFilter = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_MIPFILTER:
            // We disable mips using SRV fields, re-emit the SRV if this setting changes
            if (samplerID.MipFilter == D3DDDITEXF_NONE || dwValue == D3DDDITEXF_NONE)
            {
                m_dirtyFlags.Textures |= dirtyMask;
            }

            samplerID.MipFilter = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_ADDRESSU:
            samplerID.AddressU = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_ADDRESSV:
            samplerID.AddressV = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_ADDRESSW:
            samplerID.AddressW = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_MAXANISOTROPY:
            samplerID.MaxAnisotropy = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_BORDERCOLOR:
            samplerID.BorderColor = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_MIPMAPLODBIAS:
            samplerID.MipLODBias = *(FLOAT*)(&dwValue);
            // Some apps pass garbage floating points
            if (isnan(samplerID.MipLODBias))
            {
                samplerID.MipLODBias = 0;
                m_dwTextureStageStates[dwStage][dwState] = *(DWORD*)(&samplerID.MipLODBias);
            }
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DTSS_MAXMIPLEVEL:
            samplerID.MaxMipLevel = dwValue;
            m_dirtyFlags.Samplers |= dirtyMask;
            break;

        case D3DDDITSS_TEXTURECOLORKEYVAL:
            m_dirtyFlags.PSExtension3 = 1;
            break;
        }

        if (dwStage < D3DHAL_TSS_MAXSTAGES)
        {
            switch (dwState)
            {
            case D3DTSS_TEXCOORDINDEX:
                m_rasterStates.ClearTCIMappingFlags(0xf << (dwStage * 4));
                m_rasterStates.SetTCIMappingFlags((dwValue & 0xf) << (dwStage * 4));
                break;

            case D3DTSS_BUMPENVMAT00:
            case D3DTSS_BUMPENVMAT01:
            case D3DTSS_BUMPENVMAT10:
            case D3DTSS_BUMPENVMAT11:
            case D3DTSS_BUMPENVLSCALE:
            case D3DTSS_BUMPENVLOFFSET:
                m_pixelStage.SetPSExtension2State(dwStage, dwState, dwValue);
                break;
            case D3DTSS_TEXTURETRANSFORMFLAGS:
                if (dwValue & D3DTTFF_PROJECTED)
                {
                    m_rasterStates.SetProjectedTCsMask(dirtyMask);
                }
                else
                {
                    m_rasterStates.ClearProjectedTCsMask(dirtyMask);
                }
                break;

            case D3DDDITSS_DISABLETEXTURECOLORKEY:
                if (dwValue)
                {
                    m_rasterStates.SetColorKeyTSSDisable(dirtyMask);
                }
                else
                {
                    m_rasterStates.ClearColorKeyTSSDisable(dirtyMask);
                }
                break;
            }
        }

    }

    PipelineState::PipelineState(Device& device) :
        m_rasterStates(m_dirtyFlags),
        m_inputAssembly(m_dirtyFlags, m_rasterStates),
        m_vertexStage(device, m_dirtyFlags, m_rasterStates),
        m_pixelStage(m_dirtyFlags, m_rasterStates),
        m_bNeedsPipelineState(false),
        m_intzRestoreZWrite(false)
    {
        memset(&m_PSODesc, 0, sizeof(m_PSODesc));

        // Set all the PSO desc fields that should never change
        m_PSODesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        m_PSODesc.NodeMask = 0;
        m_PSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    }

    HRESULT PipelineState::Init(Device& device)
    {
        HRESULT hr = S_OK;
        hr = m_pixelStage.Init(device);
        
        CHECK_HR(hr);
        return hr;
    }

    HRESULT PipelineState::ResolveDeferredState(Device &device, OffsetArg BaseVertexStart, OffsetArg BaseIndexStart)
    {
        HRESULT hr = S_OK;
        if (!m_bNeedsPipelineState) return S_OK;

        hr = m_inputAssembly.ResolveDeferredState(device, m_PSODesc, BaseVertexStart, BaseIndexStart);
        CHECK_HR(hr);

        if (SUCCEEDED(hr))
        {
            hr = m_vertexStage.ResolveDeferredState(device, m_PSODesc);
            CHECK_HR(hr);
        }

        // PS must come after VS since it may need to be recompiled based on the VS output
        UINT textureDirtyMaskToKeep = 0;
        if (SUCCEEDED(hr))
        {
            hr = m_pixelStage.ResolveDeferredState(device, m_PSODesc, textureDirtyMaskToKeep);
            CHECK_HR(hr);
        }

        if (m_dirtyFlags.IsPSOChangeRequired())
        {
            D3D12TranslationLayer::PipelineState * pPipelineState = device.GetPipelineStateCache().GetPipelineState(m_PSODesc,
                GetVertexStage().GetCurrentD3D12VertexShader(),
                GetPixelStage().GetCurrentD3D12PixelShader(),
                GetVertexStage().GetCurrentD3D12GeometryShader());

            if (pPipelineState)
            {
                device.GetContext().SetPipelineState(pPipelineState);
            }
        }

        m_dirtyFlags.Clear();

        // If this draw call was made with byte offsetting, we bake this offset in to the bounded VB/IB.
        // We must clean this up for draws that won't expect this additional offset in the VB/IB
        
        if (BaseVertexStart.m_type == OffsetType::OFFSET_IN_BYTES)            
        {
            m_dirtyFlags.VertexBuffers |= BIT(0);
        }

        if (BaseIndexStart.m_type == OffsetType::OFFSET_IN_BYTES)
        {
            m_dirtyFlags.IndexBuffer = true;
        }

        m_dirtyFlags.Textures |= textureDirtyMaskToKeep;

        device.GetConstantsManager().BindShaderConstants();
        CHECK_HR(hr);

        return hr;
    }
};
