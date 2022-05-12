// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Device;
    class PixelShader;
    struct D3D12PixelShader;

    class PixelStage
    {
    public:
        PixelStage(PipelineStateDirtyFlags& pipelineStateDirtyFlags, RasterStatesWrapper& rasterStates);

        HRESULT Init(Device& device);

        UINT GetNumBoundRenderTargets() { return m_numBoundRenderTargets; }
        const BoundRenderTarget &GetRenderTarget(UINT renderTargetIndex) { return m_pRenderTargets[renderTargetIndex]; }
        void SetDepthStencil(Resource *pDepthStencil);
        Resource *GetDepthStencil() { return m_pDepthStencil; }
        void SetMultiSampleMask(DWORD value) { m_multiSampleMask = value; }

        SamplerStateID& GetSamplerID(DWORD dwStage){ return m_samplerStateIDs[dwStage]; }
        Resource* GetSRV(DWORD dwStage) { return m_shaderResources[dwStage]; }

        void SetRenderTarget(UINT renderTargetIndex, BoundRenderTarget boundRenderTarget);
        void UpdateWInfo(_In_ CONST D3DDDIARG_WINFO& winInfo); // this is used for setting fog values
        void SetPixelShader(PixelShader* pShader);
        void SetRasterState(DWORD dwState, DWORD dwValue);
        void SetDepthStencilState(Device& device, DWORD dwState, DWORD dwValue);
        void SetBlendState(Device& device, DWORD dwState, DWORD dwValue);
        void SetPSExtensionState(DWORD dwState, DWORD dwValue);
        void SetPSExtension2State(DWORD dwStage, DWORD dwState, DWORD dwValue);
        void SetPreTransformedVertexMode(bool drawingPretransformedVerts);

        HRESULT ResolveDeferredState(Device &device, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, UINT& textureDirtyMaskToKeep);

        HRESULT SetSRV(Device& device, Resource* resource, UINT dx9TextureIndex);
        void SetSRGBTexture(UINT index, bool srgbEnabled);
        void SetSRGBWriteEnable(bool srgbEnabled);
        bool GetSRGBWriteEnable() { return m_SRGBWriteEnabled; }

        RasterizerStateID& GetRasterizerStateID() { return m_rasterizerStateID; }
        DepthStencilStateID& GetDepthStencilStateID() { return m_depthStencilStateID; }
        D3D12PixelShader* GetCurrentD3D12PixelShader() { return m_pCurrentD3D12PixelShader; }

        PixelShader* GetCurrentD3D9PixelShader() { return m_pCurrentPS; }
        void MarkSRVIndicesDirty(const std::vector<UINT> &indices);

    private: // Types
        struct SamplerCache
        {
            D3D12TranslationLayer::Sampler* GetSampler( Device& device, SamplerStateID& id );
        private:
            //Required for stl map
            struct Hasher {
                size_t operator()( SamplerStateID const& other ) const { return size_t( HashData( &other, sizeof( other ) ).m_data ); }
            };


            typedef std::unordered_map < SamplerStateID, std::unique_ptr<D3D12TranslationLayer::Sampler>, Hasher > MapType;

            MapType m_map;
        };

        struct ComputedRasterStates {
            UINT m_alphaTestEnable;
            UINT m_alphaFunc;
        };

    private: // Methods
        void ResolveRenderTargets(Device &device, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, bool bDSVBound);
        Resource* FindFirstValidBoundWritableResource();
        inline void SetAlphaToCoverageEnabled( const bool enabled );

    private: // Members
        DepthStencilStateID         m_depthStencilStateID;
        RasterizerStateID           m_rasterizerStateID;
        BlendStateID                m_blendStateID;
        UINT                        m_blendFactor;
        UINT                        m_multiSampleMask;
        RTVBitMask                  m_alphaDisabledMask;
        UINT                        m_rtvShaderComponentMapping[MAX_RENDER_TARGETS];
        SamplerStateID              m_samplerStateIDs[MAX_SAMPLERS_STAGES];

        Resource*                   m_shaderResources[MAX_SAMPLERS_STAGES];
        bool                        m_SRGBTexture[MAX_SAMPLERS_STAGES];
        bool                        m_SRGBWriteEnabled;
        bool                        m_hideDepthStencil;

        /* Extensions to handle 9 APIs */
        ShaderConv::PSCBExtension           m_PSExtension;
        ShaderConv::PSCBExtension2          m_PSExtension2;

        PipelineStateDirtyFlags& m_dirtyFlags;
        RasterStatesWrapper& m_rasterStates;
        ComputedRasterStates m_computedRasterStates;

        PixelShader* m_pCurrentPS;
        D3D12PixelShader* m_pCurrentD3D12PixelShader;

        UINT m_rtSetMask;
        _Field_range_(0, MAX_RENDER_TARGETS) DWORD m_numBoundRenderTargets;
        BoundRenderTarget m_pRenderTargets[MAX_RENDER_TARGETS];

        Resource *m_pDepthStencil;
        D3D12_DSV_FLAGS m_dsvRWType;

        SamplerCache m_samplerCache;
    };
};
