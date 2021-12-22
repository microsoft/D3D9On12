// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    void VerifyPipelineState(const D3D12TranslationLayer::GRAPHICS_PIPELINE_STATE_DESC &desc)
    {
        if (desc.RasterizerState.ForcedSampleCount > 0)
        {
            Check9on12(desc.DSVFormat == DXGI_FORMAT_UNKNOWN);
        }
    }

    D3D12TranslationLayer::PipelineState * PipelineStateCache::GetPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc, D3D12VertexShader* pVS, D3D12PixelShader* pPS, D3D12GeometryShader* pGS)
    {
        Trim();

        PipelineStateKey key(psoDesc, pVS, pPS, pGS);

        UINT64 timestamp = m_device.GetContext().GetCommandListID(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS);

        auto it = m_cache.m_map.find(key);

        if (it != m_cache.m_map.end())
        {
            std::shared_ptr<PipelineStateCacheEntry>& cacheEntry = it->second;
            assert(cacheEntry.get() == cacheEntry->m_accessOrderPos->get());
            m_cache.m_accessOrder.erase(cacheEntry->m_accessOrderPos);
            m_cache.m_accessOrder.emplace_front(cacheEntry);
            cacheEntry->m_accessOrderPos = m_cache.m_accessOrder.begin();
            cacheEntry->m_timestamp = timestamp;
            return cacheEntry->m_pPipelineState.get();
        }

        std::shared_ptr<PipelineStateCacheEntry> cacheEntry = std::make_shared<PipelineStateCacheEntry>(key);
        m_cache.m_accessOrder.emplace_front(cacheEntry);
        cacheEntry->m_accessOrderPos = m_cache.m_accessOrder.begin();
        cacheEntry->m_timestamp = timestamp;

        m_cache.m_map[key] = cacheEntry;
        assert(m_cache.m_map.size() == m_cache.m_accessOrder.size());

        D3D12TranslationLayer::GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pVertexShader = pVS->GetUnderlying();
        desc.pGeometryShader = (pGS) ? pGS->GetUnderlying() : nullptr;
        desc.pPixelShader = pPS->GetUnderlying();
        desc.BlendState = psoDesc.BlendState;
        desc.SampleMask = psoDesc.SampleMask;
        desc.RasterizerState = psoDesc.RasterizerState;
        desc.DepthStencilState = psoDesc.DepthStencilState;
        desc.InputLayout = psoDesc.InputLayout;
        desc.IBStripCutValue = psoDesc.IBStripCutValue;
        desc.PrimitiveTopologyType = psoDesc.PrimitiveTopologyType;
        desc.NumRenderTargets = psoDesc.NumRenderTargets;
        desc.DSVFormat = psoDesc.DSVFormat;
        desc.SampleDesc = psoDesc.SampleDesc;
        desc.NodeMask = psoDesc.NodeMask;
        memcpy(desc.RTVFormats, psoDesc.RTVFormats, sizeof(DXGI_FORMAT) * desc.NumRenderTargets);

        VerifyPipelineState(desc);

        // No cached PSO exists, time to create one
        cacheEntry->m_pPipelineState.reset(new D3D12TranslationLayer::PipelineState(&m_device.GetContext(), desc)); // throw( bad_alloc, _com_error )

        Check9on12(pPS->GetD3D9ParentShader());
        Check9on12(pVS->GetD3D9ParentShader());
        AddUses(*pPS->GetD3D9ParentShader(), *pVS->GetD3D9ParentShader(), key);

        return cacheEntry->m_pPipelineState.get();
    }

    void PipelineStateCache::AddUses(Shader &ps, Shader &vs, PipelineStateKey key)
    {
        vs.AddPSO(key);
        ps.AddPSO(key);
    }

    void PipelineStateCache::Trim()
    {
        DWORD psoCacheTrimLimitSize = min(RegistryConstants::g_cPSOCacheTrimLimitSize, g_AppCompatInfo.PSOCacheTrimLimitSize);

        if (psoCacheTrimLimitSize == MAXDWORD)
            return;

        DWORD psoCacheTrimLimitAge = min(RegistryConstants::g_cPSOCacheTrimLimitAge, g_AppCompatInfo.PSOCacheTrimLimitAge);

        if (psoCacheTrimLimitAge == MAXDWORD)
            return;

        UINT64 timestamp = m_device.GetContext().GetCommandListID(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS);

        while (m_cache.m_map.size() > (size_t)psoCacheTrimLimitSize)
        {
            if (m_cache.m_accessOrder.empty())
                break;

            UINT64 age = timestamp - m_cache.m_accessOrder.back()->m_timestamp;

            if (age > (UINT64)psoCacheTrimLimitAge)
            {
                std::shared_ptr<PipelineStateCacheEntry> cacheEntry = m_cache.m_accessOrder.back();
                m_cache.m_accessOrder.pop_back();

                cacheEntry->m_key.m_desc.m_pPS->GetD3D9ParentShader()->RemovePSO(cacheEntry->m_key);
                cacheEntry->m_key.m_desc.m_pVS->GetD3D9ParentShader()->RemovePSO(cacheEntry->m_key);
                m_cache.m_map.erase(cacheEntry->m_key);
            }
            else
            {
                break;
            }
        }
    }

    UINT8 PipelineStateKey::D3D9on12PipelineStateDesc::CompressedData::CompressDepthFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_UNKNOWN:
            return 0;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return 1;
        case DXGI_FORMAT_D32_FLOAT:
            return 2;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return 3;
        case DXGI_FORMAT_D16_UNORM:
            return 4;
        }

        Check9on12(false);
        return 0;
    }

    UINT8 PipelineStateKey::D3D9on12PipelineStateDesc::CompressedData::CompressRenderTargetFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_UNKNOWN :
            return 0;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return 1;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return 2;
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            return 3;
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 4;
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            return 5;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return 6;
        case DXGI_FORMAT_R16G16_UNORM:
            return 7;
        case DXGI_FORMAT_R16G16_FLOAT:
            return 8;
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            return 9;
        case DXGI_FORMAT_R16_FLOAT:
            return 10;
        case DXGI_FORMAT_R32_FLOAT:
            return 11;
        case DXGI_FORMAT_R32G32_FLOAT:
            return 12;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return 13;
        case DXGI_FORMAT_R8G8_SNORM:
            return 14;
        case DXGI_FORMAT_R8G8B8A8_SNORM:
            return 15;
        case DXGI_FORMAT_R16G16_SNORM:
            return 16;
        case DXGI_FORMAT_R16G16B16A16_SNORM:
            return 17;
        case DXGI_FORMAT_R8_UNORM:
            return 18;
        case DXGI_FORMAT_R16_UNORM:
            return 19;
        case DXGI_FORMAT_A8_UNORM:
            return 20;
        case DXGI_FORMAT_R8G8_UNORM:
            return 21;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return 22;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return 23;
        case DXGI_FORMAT_B5G6R5_UNORM:
            return 24;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return 25;
        }

        Check9on12(false);
        return 0;
    }

    PipelineStateKey::D3D9on12PipelineStateDesc::CompressedData::CompressedData(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
    {
        memset(this, 0, sizeof(*this));
         
        Check9on12(desc.SampleDesc.Count <= std::pow(2.0, (double)m_SampleCountBits)); 
        Check9on12(desc.SampleDesc.Quality <= std::pow(2.0, (double)m_SampleCountBits));
        Check9on12(desc.RasterizerState.ForcedSampleCount <= std::pow(2.0, (double)m_SampleCountBits));

        //Note: the subtraction from some variables is due to the fact that their enums start a 1 not 0..
        PrimitiveTopologyType   = desc.PrimitiveTopologyType;
        NumRenderTargets        = desc.NumRenderTargets;
        RenderTargetFormat0     = CompressRenderTargetFormat(desc.RTVFormats[0]);
        RenderTargetFormat1     = CompressRenderTargetFormat(desc.RTVFormats[1]);
        RenderTargetFormat2     = CompressRenderTargetFormat(desc.RTVFormats[2]);
        RenderTargetFormat3     = CompressRenderTargetFormat(desc.RTVFormats[3]);
        DepthStencilFormat      = CompressDepthFormat(desc.DSVFormat);
        SampleCount             = desc.SampleDesc.Count;
        SampleQuality           = desc.SampleDesc.Quality;
        FillMode                = (desc.RasterizerState.FillMode - D3D12_FILL_MODE_WIREFRAME);
        CullMode                = (desc.RasterizerState.CullMode - D3D12_CULL_MODE_NONE);
        FrontCounterClockwise   = desc.RasterizerState.FrontCounterClockwise;
        DepthClipEnable         = desc.RasterizerState.DepthClipEnable;
        MultisampleEnable       = desc.RasterizerState.MultisampleEnable;
        AntialiasedLineEnable   = desc.RasterizerState.AntialiasedLineEnable;
        ForcedSampleCount       = desc.RasterizerState.ForcedSampleCount;
        DepthEnable             = desc.DepthStencilState.DepthEnable;
        DepthWriteMask          = desc.DepthStencilState.DepthWriteMask;
        DepthFunc               = (desc.DepthStencilState.DepthFunc - D3D12_COMPARISON_FUNC_NEVER);
        StencilEnable           = desc.DepthStencilState.StencilEnable;
        FrontStencilFailOp      = desc.DepthStencilState.FrontFace.StencilFailOp - D3D12_STENCIL_OP_KEEP;
        FrontStencilDepthFailOp = desc.DepthStencilState.FrontFace.StencilDepthFailOp - D3D12_STENCIL_OP_KEEP;
        FrontStencilPassOp      = desc.DepthStencilState.FrontFace.StencilPassOp - D3D12_STENCIL_OP_KEEP;
        FrontStencilFunc        = desc.DepthStencilState.FrontFace.StencilFunc - D3D12_COMPARISON_FUNC_NEVER;
        BackStencilFailOp       = desc.DepthStencilState.BackFace.StencilFailOp - D3D12_STENCIL_OP_KEEP;
        BackStencilDepthFailOp  = desc.DepthStencilState.BackFace.StencilDepthFailOp - D3D12_STENCIL_OP_KEEP;
        BackStencilPassOp       = desc.DepthStencilState.BackFace.StencilPassOp - D3D12_STENCIL_OP_KEEP;
        BackStencilFunc         = desc.DepthStencilState.BackFace.StencilFunc - D3D12_COMPARISON_FUNC_NEVER;

#define InitBlendDesc(index) \
        BlendDescBlendEnable##index             = desc.BlendState.RenderTarget[index].BlendEnable; \
        BlendDescLogicOpEnable##index           = desc.BlendState.RenderTarget[index].LogicOpEnable; \
        BlendDescSrcBlend##index                = desc.BlendState.RenderTarget[index].SrcBlend - D3D12_BLEND_ZERO; \
        BlendDescDestBlend##index               = desc.BlendState.RenderTarget[index].DestBlend - D3D12_BLEND_ZERO; \
        BlendDescBlendOp##index                 = desc.BlendState.RenderTarget[index].BlendOp - D3D12_BLEND_OP_ADD; \
        BlendDescSrcBlendAlpha##index           = desc.BlendState.RenderTarget[index].SrcBlendAlpha - D3D12_BLEND_ZERO; \
        BlendDescDestBlendAlpha##index          = desc.BlendState.RenderTarget[index].DestBlendAlpha - D3D12_BLEND_ZERO; \
        BlendDescBlendOpAlpha##index            = desc.BlendState.RenderTarget[index].BlendOpAlpha - D3D12_BLEND_OP_ADD; \
        BlendDescLogicOp##index                 = desc.BlendState.RenderTarget[index].LogicOp; \
        BlendDescRenderTargetWriteMask##index   = desc.BlendState.RenderTarget[index].RenderTargetWriteMask;

        InitBlendDesc(0);
        InitBlendDesc(1);
        InitBlendDesc(2);
        InitBlendDesc(3);

    }

};