// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY SetScissorRect(_In_ HANDLE hDevice, _In_ CONST RECT* pRect)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pRect == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetPipelineState().GetVertexStage().SetScissorRect(*pDevice, *pRect);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetClipPlane(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETCLIPPLANE* pSetClipPlane)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pSetClipPlane == nullptr || pSetClipPlane->Index >= MAX_USER_CLIPPLANES)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetPipelineState().GetVertexStage().SetClipPlane(*pSetClipPlane);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetVertexShaderFunc(_In_ HANDLE hDevice, _In_ HANDLE hShader)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        VertexShader* pShader = (VertexShader*)Shader::GetShaderFromHandle(hShader);

        if (pShader)
        {
            pDevice->GetPipelineState().GetVertexStage().SetVertexShader(pShader);
        }
        else
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetViewport(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_VIEWPORTINFO* pViewPortInfo)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pViewPortInfo == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        PipelineState& pipeline = pDevice->GetPipelineState();
        pipeline.GetVertexStage().SetViewPort(*pDevice, *pViewPortInfo);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetZRange(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_ZRANGE* pZRange)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pZRange == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        PipelineState& pipeline = pDevice->GetPipelineState();
        pipeline.GetVertexStage().SetZRange(*pZRange);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    VertexStage::VertexStage(Device& device, PipelineStateDirtyFlags& pipelineStateDirtyFlags, RasterStatesWrapper& rasterStates) :
        m_dirtyFlags(pipelineStateDirtyFlags),
        m_VSExtension(pipelineStateDirtyFlags),
        m_rasterStates(rasterStates),
        m_pCurrentVS(nullptr),
        m_pCurrentD3D12VS(nullptr),
        m_pCurrentD3D12GS(nullptr),
        m_tlShaderCache(device),
        m_scissorRect{},
        m_currentViewPort{},
        m_geometryShaderCache(device)
    {}

    void VertexStage::SetScissorRect(Device& device, _In_ RECT scissorRect)
    {
        m_scissorRect = scissorRect;
        ApplyScissorRect(device);
    }

    void VertexStage::ApplyScissorRect(Device & device)
    {
        if (m_scissorTestEnabled)
        {
            device.GetContext().SetScissorRect(0, &m_scissorRect);
            device.GetContext().SetScissorRectEnable(true);
        }
        else
        {
            device.GetContext().SetScissorRectEnable(false);
        }
    }

    void VertexStage::SetScissorTestEnabled(Device& device, bool scissorTestEnabled)
    {
        if (m_scissorTestEnabled != scissorTestEnabled)
        {
            m_scissorTestEnabled = scissorTestEnabled;
            ApplyScissorRect(device);
        }
    }

    void VertexStage::SetClipPlane(_In_ CONST D3DDDIARG_SETCLIPPLANE& setClipPlane)
    {
        m_VSExtension.SetClipPlane(setClipPlane.Index, setClipPlane.Plane);
    }

    void VertexStage::SetVertexShader(VertexShader* pShader)
    {
        if (m_pCurrentVS != pShader)
        {
            m_pCurrentVS = pShader;
            m_dirtyFlags.VertexShader = true;
        }
    }

    void VertexStage::SetPointSize(DWORD dwState, DWORD dwValue)
    {
        switch (dwState)
        {
        case D3DRS_POINTSIZE:
            m_VSExtension.SetPointSize(*(FLOAT*)(&dwValue));
            break;
        case D3DRS_POINTSIZE_MIN:
            m_VSExtension.SetMinPointSize(max<float>(*(FLOAT*)(&dwValue), MIN_POINT_SIZE));
            break;
        case D3DRS_POINTSIZE_MAX:
            m_VSExtension.SetMaxPointSize(min<float>(*(FLOAT*)(&dwValue), MAX_POINT_SIZE));
            break;
        default:
            Check9on12(false);
        }
    }

    void VertexStage::SetViewPort(Device& /*device*/, _In_ CONST D3DDDIARG_VIEWPORTINFO& viewPortInfo)
    {
        m_currentViewPort.TopLeftX = (float)viewPortInfo.X;
        m_currentViewPort.TopLeftY = (float)viewPortInfo.Y;
        m_currentViewPort.Width = (float)viewPortInfo.Width;
        m_currentViewPort.Height = (float)viewPortInfo.Height;

        m_dirtyFlags.Viewport = true;
    }

    void VertexStage::SetZRange(_In_ CONST D3DDDIARG_ZRANGE& zRange)
    {
        m_currentViewPort.MaxDepth = zRange.MaxZ;
        m_currentViewPort.MinDepth = zRange.MinZ;

        m_dirtyFlags.Viewport = true;
    }

    // The viewport must be deferred because they can set the z range independently
    void VertexStage::ResolveViewPort(Device &device)
    {
        // Update the VS extension
        m_VSExtension.SetViewportScale(
            1.0f / m_currentViewPort.Width,
            -1.0f / m_currentViewPort.Height);

        m_VSExtension.SetScreenToClipOffset(
            0.5f - (m_currentViewPort.Width / 2.0f + m_currentViewPort.TopLeftX),
            0.5f - (m_currentViewPort.Height / 2.0f + m_currentViewPort.TopLeftY),
            -m_currentViewPort.MinDepth);

        m_VSExtension.SetScreenToClipScale(
            2.0f / m_currentViewPort.Width,
            -2.0f / m_currentViewPort.Height,
            1.0f / (m_currentViewPort.MaxDepth - m_currentViewPort.MinDepth));

        device.GetContext().SetViewport(0, &m_currentViewPort);

        m_dirtyFlags.VSExtension = true;
    }

    bool VertexStage::NeedsGeometryShader(const ShaderConv::VSOutputDecls &vsOutputDecls)
    {
        bool needsGeometryShader = m_rasterStates.IsUsingPointFill() || m_rasterStates.IsUsingPointSpritesOrSizes(vsOutputDecls);

        if (!needsGeometryShader)
        {
            UINT texCoordMask = vsOutputDecls.TexCoords;
            for (UINT i = 0; i < D3DHAL_SAMPLER_MAXSAMP && texCoordMask; i++, texCoordMask = texCoordMask >> 1)
            {
                if (texCoordMask & 1)
                {
                    if (m_rasterStates.GetPSTexCoordWrap(i))
                    {
                        needsGeometryShader = true;
                        break;
                    }
                }
            }
        }

        return needsGeometryShader;
    }

    HRESULT VertexStage::ResolveDeferredState(Device &device, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
    {
        HRESULT hr = S_OK;

        auto& ia = device.GetPipelineState().GetInputAssembly();
        auto& inputLayout = ia.GetInputLayout();

        // Must come before VSExtension
        if (m_dirtyFlags.Viewport)
        {
            // TODO: how to deal with MRT?
            ResolveViewPort(device);
        }

        if (m_dirtyFlags.PointSize)
        {
            m_VSExtension.ResolvePointSize();
            float pointSize = m_VSExtension.GetPointSize();

            // Check if PointSize should be enabled  
            m_rasterStates.SetPointSizeEnable((pointSize > 1.0f) ? 1 : 0);
        }

        if (m_dirtyFlags.VSExtension)
        {
            device.GetConstantsManager().UpdateVertexShaderExtension(m_VSExtension.GetVSCBExtension());
        }

        if (m_dirtyFlags.VertexShader || m_dirtyFlags.InputLayout)
        {
            if (inputLayout.VerticesArePreTransformed())
            {
                m_pCurrentD3D12VS = &m_tlShaderCache.GetD3D12ShaderForTL(inputLayout, m_rasterStates.GetRasterState());

                psoDesc.VS = m_pCurrentD3D12VS->GetUnderlying()->GetByteCode();
                psoDesc.InputLayout.pInputElementDescs = &m_pCurrentD3D12VS->m_inputElementDescs[0];
                psoDesc.InputLayout.NumElements = static_cast<UINT>(m_pCurrentD3D12VS->m_inputElementDescs.size());
            }
            else
            {
                m_pCurrentD3D12VS = &m_pCurrentVS->GetD3D12Shader(m_rasterStates.GetRasterState(), inputLayout);

                psoDesc.VS = m_pCurrentD3D12VS->GetUnderlying()->GetByteCode();

                psoDesc.InputLayout.pInputElementDescs = &m_pCurrentD3D12VS->m_inputElementDescs[0];
                psoDesc.InputLayout.NumElements = static_cast<UINT>(m_pCurrentD3D12VS->m_inputElementDescs.size());

                for (UINT i = 0; i < ARRAYSIZE(m_pCurrentD3D12VS->m_inlineConsts); i++)
                {
                    for (auto &data : m_pCurrentD3D12VS->m_inlineConsts[i])
                    {
                        device.GetConstantsManager().GetVertexShaderConstants().GetConstantBufferData((ShaderConv::eConstantBuffers)i).SetData((Float4*)&data.Value, data.RegIndex / 4, 1);
                    }
                }

                if (psoDesc.VS.pShaderBytecode == nullptr)
                {
                    hr = E_FAIL;
                    CHECK_HR(hr);
                }
            }

            m_dirtyFlags.PixelShader = true; // Different VSOutput means the PS may need to be recompiled
        }

        D3D12VertexShader* pShader = GetCurrentD3D12VertexShader();
        if (NeedsGeometryShader(pShader->m_vsOutputDecls))
        {
            D3D12GeometryShader& geo = m_geometryShaderCache.GetD3D12Shader(*m_pCurrentD3D12VS, m_rasterStates.GetRasterState());

            psoDesc.GS = geo.GetUnderlying()->GetByteCode();

            if (psoDesc.GS.pShaderBytecode == nullptr)
            {
                hr = E_FAIL;
                CHECK_HR(hr);
            }
            else
            {
                m_pCurrentD3D12GS = &geo;

                device.GetConstantsManager().UpdateGeometryShaderExtension(m_VSExtension.GetVSCBExtension());
            }
        }
        else
        {
            psoDesc.GS = {};
            m_pCurrentD3D12GS = nullptr;
        }

        return hr;
    }

};