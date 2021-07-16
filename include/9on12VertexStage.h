// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Device;
    class VertexShader;
    struct D3D12VertexShader;

    class VertexStage
    {
    public:
        VertexStage(Device& device, PipelineStateDirtyFlags& pipelineStateDirtyFlags, RasterStatesWrapper& rasterStates);

        void SetVertexShader(VertexShader* pShader);

        void SetClipPlane(_In_ CONST D3DDDIARG_SETCLIPPLANE& setClipPlane);

        HRESULT ResolveDeferredState(Device &device, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

        void SetPointSize(DWORD dwState, DWORD dwValue);

        void SetScissorRect(Device& device, _In_ RECT scissorRect);
        void ApplyScissorRect(Device& device);
        void SetScissorTestEnabled(Device& device, bool scissorTestEnabled);
        void SetViewPort(Device& device, _In_ CONST D3DDDIARG_VIEWPORTINFO& viewPortInfo);
        void SetZRange(_In_ CONST D3DDDIARG_ZRANGE& zRange);

        void ResolveViewPort(Device& device);

        D3D12VertexShader* GetCurrentD3D12VertexShader() { return m_pCurrentD3D12VS; }
        D3D12GeometryShader* GetCurrentD3D12GeometryShader() { return m_pCurrentD3D12GS; }

        VertexShader *GetCurrentD3D9VertexShader() { return m_pCurrentVS; }
    private:
        bool NeedsGeometryShader(const ShaderConv::VSOutputDecls &vsOutputDecls);

        bool m_scissorTestEnabled;
        RECT m_scissorRect;

        VertexShader* m_pCurrentVS;
        D3D12VertexShader* m_pCurrentD3D12VS;
        D3D12GeometryShader* m_pCurrentD3D12GS;

        VertexShader m_tlShaderCache;
        GeometryShader m_geometryShaderCache;

        D3D12_VIEWPORT m_currentViewPort;

        /* Extensions to handle 9 APIs */
        VSCBExntensionWrapper   m_VSExtension;

        PipelineStateDirtyFlags& m_dirtyFlags;
        RasterStatesWrapper& m_rasterStates;
    };
};