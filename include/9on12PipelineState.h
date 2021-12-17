// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Device; // Forward declaration
    class ShaderCache;

    class PipelineState
    {
    public:
        PipelineState(Device& device, VertexStageFactory& vertexStageFactory);

        HRESULT Init(Device& device);

        HRESULT ResolveDeferredState(Device &device, OffsetArg BaseVertexStart, OffsetArg BaseIndexStart);

        void SetRenderState(Device& device, DWORD dwState, DWORD dwValue);
        void SetTextureStageState(DWORD dwStage, DWORD dwState, DWORD dwValue);

        InputAssembly& GetInputAssembly() { return m_inputAssembly; }
        VertexStage& GetVertexStage() { return m_vertexStage; }
        PixelStage& GetPixelStage() { return m_pixelStage; }

        void MarkPipelineStateNeeded() { m_bNeedsPipelineState = true; }
        void MarkInputLayoutAsDirty() { m_dirtyFlags.InputLayout = true; }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC& CurrentPSODesc() { return m_PSODesc; }
    private:
        PipelineStateDirtyFlags     m_dirtyFlags;

        InputAssembly m_inputAssembly;
        VertexStage m_vertexStage;
        PixelStage m_pixelStage;

        // This is used to avoid creating a PSO before we have enough state
        // (this can happen when ResolveDeferredState is called due to a Map()
        // before some important SetRenderState is called). This is somewhat hacky
        // and we should likely build on this to have more complex checks that 
        // don't set/create the PSO unless we know we absolutely need it
        BOOL m_bNeedsPipelineState;

        RasterStatesWrapper m_rasterStates;

        union
        {
            DWORD m_dwTextureStageStates[MAX_SAMPLERS_STAGES][MAX_D3DTSS];
            FLOAT m_fTextureStageStates[MAX_SAMPLERS_STAGES][MAX_D3DTSS];
        };

        union
        {
            DWORD m_dwRenderStates[D3DHAL_MAX_RSTATES];
            FLOAT m_fRenderStates[D3DHAL_MAX_RSTATES];
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PSODesc;
    };

    class PipelineStateFactory
    {
    public:
        virtual PipelineState Create(Device& device) = 0;
    };

    class PipelineStateFactoryImpl : public PipelineStateFactory
    {
    public:
        PipelineState Create(Device& device) override
        {
            VertexStageFactoryImpl vertexStageFactory;
            return PipelineState(device, vertexStageFactory);
        };
    };
};
