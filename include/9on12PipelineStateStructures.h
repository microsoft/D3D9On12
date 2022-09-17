// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    struct PipelineStateDirtyFlags
    {
        union
        {
            UINT64 MiscFlags;
            struct
            {
                UINT VSExtension : 1;
                UINT PSExtension : 1;
                UINT PSExtension2 : 1;
                UINT PSExtension3 : 1;
                UINT VertexBuffers : 1;
                UINT IndexBuffer : 1;
                UINT Samplers : MAX_SAMPLERS_STAGES;
                UINT Textures : MAX_SAMPLERS_STAGES;
            };
        };

        union
        {
            UINT64 PSOFlags;
            struct
            {
                UINT RenderTargets : MAX_RENDER_TARGETS;
                UINT DepthStencil : 1;
                UINT Viewport : 1;
                UINT Topology : 1;
                UINT InputLayout : 1;
                UINT RasterizerState : 1;
                UINT DepthStencilState : 1;
                UINT BlendState : 1;
                UINT PointSize : 1;
                UINT IndexedStream : 1;
                UINT VertexShader : 1;
                UINT PixelShader : 1;
            };
        };


        PipelineStateDirtyFlags()
        {
            Clear();
            C_ASSERT(sizeof(PipelineStateDirtyFlags) == 2 * sizeof(UINT64));
        }

        void Clear()
        {
            memset(&MiscFlags, 0, sizeof(MiscFlags));
            memset(&PSOFlags, 0, sizeof(MiscFlags));
        }

        bool IsPSOChangeRequired()
        {
            return PSOFlags != 0;
        }
    };

    class Resource;

    struct BoundRenderTarget
    {
        BoundRenderTarget() : m_pResource(nullptr), m_subresource(0) {}

        BoundRenderTarget(Resource *pResource, UINT subresource) :
            m_pResource(pResource), m_subresource(subresource) {}

        Resource *m_pResource;
        UINT m_subresource;
    };

    struct RasterizerStateID
    {
        union
        {
            UINT Flags;
            struct
            {
                UINT TriangleMode : 1;
                UINT FillMode : 2;
                UINT CullMode : 2;
                UINT MultiSampleRenderTarget : 1;
                UINT MultiSampleAntiAlias : 1;
                UINT AntialiasedLineEnable : 1;
                UINT DepthClipEnable : 1;
                UINT DrawingPreTransformedVertices : 1;
            };
        };
        FLOAT DepthBias;
        FLOAT SlopeScaleDepthBias;
        RasterizerStateID() : Flags(0),
            DepthBias(0.0f),
            SlopeScaleDepthBias(0.0f)
        {
            C_ASSERT(sizeof(RasterizerStateID) == sizeof(Flags) +
                sizeof(DepthBias) +
                sizeof(SlopeScaleDepthBias));
        }
        bool operator==(const RasterizerStateID& rhs) const
        {
            return (this->Flags == rhs.Flags)
                && (this->DepthBias == rhs.DepthBias)
                && (this->SlopeScaleDepthBias == rhs.SlopeScaleDepthBias);
        }
        bool operator!=(const RasterizerStateID& rhs) const
        {
            return !(*this == rhs);
        }
        operator UINT() const
        {
            return this->Flags;
        }

        bool RequiresForcedSampleCount() const
        {
            return MultiSampleRenderTarget && !MultiSampleAntiAlias;
        }
    };

    struct DepthStencilStateID
    {
        union
        {
            UINT Flags1;
            struct
            {
                UINT ZEnable : 1;
                UINT ZWriteEnable : 1;
                UINT ZFunc : 3;
                UINT StencilEnable : 1;
                UINT TwoSidedStencil : 1;
                UINT StencilFail : 3;
                UINT StencilZFail : 3;
                UINT StencilPass : 3;
                UINT StencilFunc : 3;
                UINT CCWStencilFail : 3;
                UINT CCWStencilZFail : 3;
                UINT CCWStencilPass : 3;
                UINT CCWStencilFunc : 3;
            };
        };
        union
        {
            UINT Flags2;
            struct
            {
                UINT StencilMask : 8;
                UINT StencilWriteMask : 8;
                UINT StencilRef : 8;
            };
        };
        DepthStencilStateID() : Flags1(0), Flags2(0)
        {
            C_ASSERT(sizeof(DepthStencilStateID) == sizeof(Flags1) + sizeof(Flags2));
        }
        bool operator==(const DepthStencilStateID& rhs) const
        {
            return (this->Flags1 == rhs.Flags1)
                && (this->Flags2 == rhs.Flags2);
        }
        bool operator!=(const DepthStencilStateID& rhs) const
        {
            return !(*this == rhs);
        }
        operator UINT() const
        {
            return this->Flags1;
        }

        bool DepthStencilViewBindingOptional()
        {
            return !ZEnable && !StencilEnable;
        }
    };

    struct BlendStateID
    {
        union
        {
            UINT Flags;
            struct
            {
                UINT AlphaBlendEnable : 1;
                UINT AlphaToCoverageEnable : 1;
                UINT SrcBlend : 5;
                UINT DstBlend : 5;
                UINT BlendOp : 3;
                UINT SeparateAlphaBlend : 1;
                UINT SrcBlendAlpha : 5;
                UINT DstBlendAlpha : 5;
                UINT BlendOpAlpha : 3;
            };
        };
        union
        {
            UINT ColorWriteFullMask;
            struct
            {
                BYTE ColorWriteMasks[MAX_RENDER_TARGETS];
            };
            struct
            {
                BYTE ColorWriteMask0;
                BYTE ColorWriteMask1;
                BYTE ColorWriteMask2;
                BYTE ColorWriteMask3;
            };
        };
        BlendStateID() : Flags(0), ColorWriteFullMask(0)
        {
            C_ASSERT(sizeof(BlendStateID) == sizeof(Flags) + sizeof(ColorWriteFullMask));
        }
        bool operator==(const BlendStateID& rhs) const
        {
            return (this->ColorWriteFullMask == rhs.ColorWriteFullMask)
                && (this->Flags == rhs.Flags);
        }
        bool operator!=(const BlendStateID& rhs) const
        {
            return !(*this == rhs);
        }
        operator UINT() const
        {
            return this->Flags;
        }
    };

    struct SamplerStateID
    {
        union
        {
            UINT Flags;
            struct
            {
                UINT MagFilter : 4;
                UINT MinFilter : 4;
                UINT MipFilter : 4;
                UINT AddressU : 3;
                UINT AddressV : 3;
                UINT AddressW : 3;
                UINT MaxAnisotropy : 8;
                UINT UseHardwareShadowMapping : 1;
                UINT SwapRBBorderColors : 1;
            };
        };
        UINT  BorderColor;
        UINT  MaxMipLevel;
        FLOAT MipLODBias;

        SamplerStateID() : Flags(0),
            BorderColor(0),
            MaxMipLevel(0),
            MipLODBias(0.0f)
        {
            C_ASSERT(sizeof(SamplerStateID) == sizeof(Flags) +
                sizeof(BorderColor) +
                sizeof(MaxMipLevel) +
                sizeof(MipLODBias));
        }
        bool operator==(const SamplerStateID& rhs) const
        {
            return (this->Flags == rhs.Flags)
                && (this->BorderColor == rhs.BorderColor)
                && (this->MaxMipLevel == rhs.MaxMipLevel)
                && (this->MipLODBias == rhs.MipLODBias);
        }
        bool operator!=(const SamplerStateID& rhs) const
        {
            return !(*this == rhs);
        }
        operator UINT() const
        {
            return this->Flags;
        }
    };

    static D3D12_RASTERIZER_DESC ConvertRasterizerState(RasterizerStateID rasterizerID, bool bDepthEnabledAndBound, DXGI_FORMAT dsvFormat)
    {
        D3D12_RASTERIZER_DESC rasterizerDesc = {};

        rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        if (rasterizerID.TriangleMode)
        {
            switch (rasterizerID.FillMode)
            {
            case D3DFILL_WIREFRAME:
                rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
                break;
            default:
            case D3DFILL_SOLID:
                rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
                break;
            }
            switch (rasterizerID.CullMode)
            {
            default:
                Check9on12(false);
                // Fall through to a reasonable default - hit by Tree of Savior
            case D3DCULL_NONE:
                rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
                break;
            case D3DCULL_CW:
                rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
                break;
            case D3DCULL_CCW:
                rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
                break;
            }
        }
        else
        {
            rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
            rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        }
        rasterizerDesc.FrontCounterClockwise = FALSE;

        if (bDepthEnabledAndBound)
        {
            if (rasterizerID.TriangleMode)
            {
                const UINT uiDepthBiasFactor = DepthBiasFactorFromDSVFormat(dsvFormat);
                rasterizerDesc.DepthBias = static_cast<INT32>(rasterizerID.DepthBias * uiDepthBiasFactor);
                rasterizerDesc.SlopeScaledDepthBias = rasterizerID.SlopeScaleDepthBias;
            }
            else
            {
                rasterizerDesc.DepthBias = 0;
                rasterizerDesc.SlopeScaledDepthBias = 0.0f;
            }
        }
        rasterizerDesc.DepthBiasClamp = 0.0f;

        if (rasterizerID.DrawingPreTransformedVertices && !bDepthEnabledAndBound)
        {
            rasterizerDesc.DepthClipEnable = false;
        }
        else
        {
            rasterizerDesc.DepthClipEnable = rasterizerID.DepthClipEnable ? TRUE : FALSE;
        }

        rasterizerDesc.AntialiasedLineEnable = rasterizerID.AntialiasedLineEnable ? TRUE : FALSE;

        rasterizerDesc.MultisampleEnable = (rasterizerID.MultiSampleRenderTarget
            && rasterizerID.MultiSampleAntiAlias) ? TRUE : FALSE;

        // In DX10.1+ MultisampleEnabled has a different meaning from DX9. In DX9 "MultiSampleEnabled == false"
        // means only sample from center of the pixel. In DX10.1+ "MultiSampleEnabled == false" means use
        // alpha line anti-aliasing. We can emulate DX9 by setting the ForcedSampleCount = 1.
        // If the D3D9 app disabled depth before turning off anti-aliasing, we'll respect it and use ForcedSampleCount = 1.
        // But if the app left depth enabled, we have to choose between turning off depth or leaving on anti-aliasing.
        // The safer thing is to just leave on anti-aliasing.
        if (rasterizerID.RequiresForcedSampleCount() && !bDepthEnabledAndBound)
        {
            rasterizerDesc.ForcedSampleCount = 1;
        }

        return rasterizerDesc;
    }

    static void PatchBlendForDisaledAlpha(D3D12_BLEND &blend)
    {
        // Only the destination blend is disabled. Because the "source" alpha for the blend
        // operation is calculated in the shader, it's not affected by whether the render target
        // has alpha disabled because this is only applied after the color is written out to the RTV
        switch (blend)
        {
        case D3D12_BLEND_DEST_ALPHA:
            blend =  D3D12_BLEND_ONE;
            break;
        case D3D12_BLEND_INV_DEST_ALPHA:
            blend = D3D12_BLEND_ZERO;
            break;
        }
    }


    static void PatchRenderTargetBlendDescForDisaledAlpha(D3D12_RENDER_TARGET_BLEND_DESC &desc)
    {
        PatchBlendForDisaledAlpha(desc.SrcBlend);
        PatchBlendForDisaledAlpha(desc.DestBlend);
    }

    static D3D12_BLEND_DESC ConvertBlendState(BlendStateID blendID, const RTVBitMask alphaDisabledMask, _In_reads_(MAX_RENDER_TARGETS) UINT *pRTVShaderComponentMappings)
    {
        D3D12_BLEND_DESC blendDesc = {};

        blendDesc.AlphaToCoverageEnable = blendID.AlphaToCoverageEnable ? TRUE : FALSE;
        blendDesc.IndependentBlendEnable = TRUE;
        blendDesc.RenderTarget[0].BlendEnable = blendID.AlphaBlendEnable ? TRUE : FALSE;
        if (blendID.AlphaBlendEnable)
        {
            UINT srcBlend = blendID.SrcBlend;
            UINT dstBlend = blendID.DstBlend;
            if (D3DBLEND_BOTHSRCALPHA == srcBlend)
            {
                srcBlend = D3DBLEND_SRCALPHA;
                dstBlend = D3DBLEND_INVSRCALPHA;
            }
            else if (D3DBLEND_BOTHINVSRCALPHA == srcBlend)
            {
                srcBlend = D3DBLEND_INVSRCALPHA;
                dstBlend = D3DBLEND_SRCALPHA;
            }

            blendDesc.RenderTarget[0].SrcBlend = ConvertToD3D12Blend(static_cast<D3DBLEND>(srcBlend));
            blendDesc.RenderTarget[0].DestBlend = ConvertToD3D12Blend(static_cast<D3DBLEND>(dstBlend));
            blendDesc.RenderTarget[0].BlendOp = ConvertToD3D12BlendOp(static_cast<D3DBLENDOP>(blendID.BlendOp));
            if (blendID.SeparateAlphaBlend)
            {
                blendDesc.RenderTarget[0].SrcBlendAlpha = ConvertToD3D12AlphaBlend(static_cast<D3DBLEND>(blendID.SrcBlendAlpha));
                blendDesc.RenderTarget[0].DestBlendAlpha = ConvertToD3D12AlphaBlend(static_cast<D3DBLEND>(blendID.DstBlendAlpha));
                blendDesc.RenderTarget[0].BlendOpAlpha = ConvertToD3D12BlendOp(static_cast<D3DBLENDOP>(blendID.BlendOpAlpha));
            }
            else
            {
                blendDesc.RenderTarget[0].SrcBlendAlpha = ConvertToD3D12AlphaBlend(static_cast<D3DBLEND>(srcBlend));
                blendDesc.RenderTarget[0].DestBlendAlpha = ConvertToD3D12AlphaBlend(static_cast<D3DBLEND>(dstBlend));

                blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
            }
        }

        for (UINT i = 0; i < MAX_RENDER_TARGETS; ++i)
        {
            if (i > 0)
            {
                blendDesc.RenderTarget[i] = blendDesc.RenderTarget[0];
            }

            UINT8 renderTargetWriteMask = blendID.ColorWriteMasks[i] & COLOR_CHANNEL_MASK;
            blendDesc.RenderTarget[i].RenderTargetWriteMask = SwizzleRenderTargetMask(
                renderTargetWriteMask,
                pRTVShaderComponentMappings[i]);
            if (alphaDisabledMask[i])
            {
                PatchRenderTargetBlendDescForDisaledAlpha(blendDesc.RenderTarget[i]);
            }

        }

        return blendDesc;
    }

    static D3D12_DEPTH_STENCIL_DESC ConvertDepthStencilState(DepthStencilStateID depthStencilID, bool bDsvBound)
    {
        D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

        depthStencilDesc.DepthEnable = (bDsvBound && depthStencilID.ZEnable) ? TRUE : FALSE;
        depthStencilDesc.DepthWriteMask =
            depthStencilID.ZWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.DepthFunc = ConvertToD3D12ComparisonFunc(static_cast<D3DCMPFUNC>(depthStencilID.ZFunc + 1));
        depthStencilDesc.StencilEnable = depthStencilID.StencilEnable ? TRUE : FALSE;
        depthStencilDesc.StencilReadMask = depthStencilID.StencilMask;
        depthStencilDesc.StencilWriteMask = depthStencilID.StencilWriteMask;
        depthStencilDesc.FrontFace.StencilFailOp = ConvertToD3D12StencilOp(static_cast<D3DSTENCILOP>(depthStencilID.StencilFail + 1));
        depthStencilDesc.FrontFace.StencilDepthFailOp = ConvertToD3D12StencilOp(static_cast<D3DSTENCILOP>(depthStencilID.StencilZFail + 1));
        depthStencilDesc.FrontFace.StencilPassOp = ConvertToD3D12StencilOp(static_cast<D3DSTENCILOP>(depthStencilID.StencilPass + 1));
        depthStencilDesc.FrontFace.StencilFunc = ConvertToD3D12ComparisonFunc(static_cast<D3DCMPFUNC>(depthStencilID.StencilFunc + 1));
        if (depthStencilID.TwoSidedStencil)
        {
            depthStencilDesc.BackFace.StencilFailOp = ConvertToD3D12StencilOp(static_cast<D3DSTENCILOP>(depthStencilID.CCWStencilFail + 1));
            depthStencilDesc.BackFace.StencilDepthFailOp = ConvertToD3D12StencilOp(static_cast<D3DSTENCILOP>(depthStencilID.CCWStencilZFail + 1));
            depthStencilDesc.BackFace.StencilPassOp = ConvertToD3D12StencilOp(static_cast<D3DSTENCILOP>(depthStencilID.CCWStencilPass + 1));
            depthStencilDesc.BackFace.StencilFunc = ConvertToD3D12ComparisonFunc(static_cast<D3DCMPFUNC>(depthStencilID.CCWStencilFunc + 1));
        }
        else
        {
            depthStencilDesc.BackFace = depthStencilDesc.FrontFace;
        }

        return depthStencilDesc;
    }

    static D3D12_SAMPLER_DESC ConvertSampler(SamplerStateID samplerID)
    {
        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.MinLOD = static_cast<float>(samplerID.MaxMipLevel);
        samplerDesc.MaxLOD = FLT_MAX;

        samplerDesc.Filter = ConvertToD3D12SamplerFilter(
            static_cast<D3DTEXTUREFILTERTYPE>(samplerID.MagFilter),
            static_cast<D3DTEXTUREFILTERTYPE>(samplerID.MinFilter),
            static_cast<D3DTEXTUREFILTERTYPE>(samplerID.MipFilter),
            samplerID.UseHardwareShadowMapping);

        samplerDesc.AddressU = ConvertToD3D12TextureAddress(static_cast<D3DTEXTUREADDRESS>(samplerID.AddressU));
        samplerDesc.AddressV = ConvertToD3D12TextureAddress(static_cast<D3DTEXTUREADDRESS>(samplerID.AddressV));
        samplerDesc.AddressW = ConvertToD3D12TextureAddress(static_cast<D3DTEXTUREADDRESS>(samplerID.AddressW));
        samplerDesc.MipLODBias = samplerID.MipLODBias;
        samplerDesc.MaxAnisotropy = samplerID.MaxAnisotropy;
        
        samplerDesc.ComparisonFunc = (samplerID.UseHardwareShadowMapping) ? D3D12_COMPARISON_FUNC_LESS_EQUAL : D3D12_COMPARISON_FUNC_NEVER;
       
        ConvertToRGBA(samplerID.BorderColor, samplerDesc.BorderColor);
        if (samplerID.SwapRBBorderColors)
        {
            std::swap(samplerDesc.BorderColor[0], samplerDesc.BorderColor[2]);
        }

        return samplerDesc;
    }

    static DWORD MapSamplerStage9on12(DWORD dwStage)
    {
        if (D3DDMAPSAMPLER == dwStage)
        {
            return DMAP_SAMPLER;
        }
        else
        {
            if ((dwStage >= D3DVERTEXTEXTURESAMPLER0) && (dwStage < (D3DVERTEXTEXTURESAMPLER0 + D3DHAL_SAMPLER_MAXVERTEXSAMP)))
            {
                return VERTEX_SAMPLER0 + (dwStage - D3DVERTEXTEXTURESAMPLER0);
            }
        }

        return dwStage;
    }

    class RasterStatesWrapper : private ShaderConv::RasterStates
    {

    public:
        RasterStatesWrapper(PipelineStateDirtyFlags &dirtyFlags) : m_dirtyFlags(dirtyFlags) {}

        const ShaderConv::RasterStates &GetRasterState()
        {
            return *dynamic_cast<ShaderConv::RasterStates *>(this);
        }

        void SetUserClipplanes(UINT inUserClipPlanes)
        {
            if (!IsEquals(inUserClipPlanes, UserClipPlanes))
            {
                UserClipPlanes = inUserClipPlanes;
                MarkVSDirty();
            }
        }

        void SetFillMode(_D3DFILLMODE inFillMode)
        {
            if (!IsEquals((_D3DFILLMODE)FillMode, inFillMode))
            {
                FillMode = inFillMode;
                MarkPSDirty();
            }
        }

        void SetShadeMode(_D3DSHADEMODE inShadeMode)
        {
            if (!IsEquals((_D3DSHADEMODE)ShadeMode, inShadeMode))
            {
                ShadeMode = inShadeMode;
                MarkPSDirty();
                MarkGSDirty();
            }
        }

        void SetPrimitiveType(D3DPRIMITIVETYPE inPrimitiveType)
        {
            if (!IsEquals(inPrimitiveType, inPrimitiveType))
            {
                PrimitiveType = inPrimitiveType;
                MarkGSDirty();
                MarkPSDirty();
            }
        }

        void SetAlphaFunc(_D3DCMPFUNC inAlphaFunc)
        {
            if (!IsEquals((_D3DCMPFUNC)AlphaFunc, inAlphaFunc))
            {
                AlphaFunc = inAlphaFunc;
                MarkPSDirty();
            }
        }

        void SetFogEnable(bool inFogEnable)
        {
            if (!IsEquals((bool)FogEnable, inFogEnable))
            {
                FogEnable = inFogEnable;
                MarkPSDirty();
            }
        }

        void SetFogTableMode(UINT inFogTableMode)
        {
            if (!IsEquals(FogTableMode, inFogTableMode))
            {
                FogTableMode = inFogTableMode;
                MarkPSDirty();
            }
        }

        void SetWFogEnable(bool inWFogEnable)
        {
            if (!IsEquals((bool)WFogEnable, inWFogEnable))
            {
                WFogEnable = inWFogEnable;
                MarkPSDirty();
            }
        }

        void SetAlphaTestEnable(bool inAlphaTestEnable)
        {
            if (!IsEquals((bool)AlphaTestEnable, inAlphaTestEnable))
            {
                AlphaTestEnable = inAlphaTestEnable;
                MarkPSDirty();
            }
        }

        void SetPointSizeEnable(bool inPointSizeEnable)
        {
            if (!IsEquals((bool)PointSizeEnable, inPointSizeEnable))
            {
                PointSizeEnable = inPointSizeEnable;
                MarkGSDirty();
            }
        }

        void SetPointSpriteEnable(bool inPointSpriteEnable)
        {
            if (!IsEquals((bool)PointSpriteEnable, inPointSpriteEnable))
            {
                PointSpriteEnable = inPointSpriteEnable;
                MarkGSDirty();
            }
        }

        void SetColorKeyEnable(bool inColorKeyEnable)
        {
            if (!IsEquals((bool)ColorKeyEnable, inColorKeyEnable))
            {
                ColorKeyEnable = inColorKeyEnable;
                MarkPSDirty();
            }
        }

        void SetColorKeyBlendEnable(bool inColorKeyBlendEnable)
        {
            if (!IsEquals((bool)ColorKeyBlendEnable, inColorKeyBlendEnable))
            {
                ColorKeyBlendEnable = inColorKeyBlendEnable;
                MarkPSDirty();
            }
        }

        void SetHasTLVertices(bool inHasTLVertices)
        {
            if (!IsEquals((bool)HasTLVertices, inHasTLVertices))
            {
                HasTLVertices = inHasTLVertices;
                MarkPSDirty();
            }
        }

        void SetPSSamplerTextureType(_In_range_(0, D3DHAL_SAMPLER_MAXSAMP - 1) UINT samplerIndex, ShaderConv::TEXTURETYPE inTextureType)
        {
            Check9on12(samplerIndex < ARRAYSIZE(PSSamplers));
            if (!IsEquals((ShaderConv::TEXTURETYPE)PSSamplers[samplerIndex].TextureType, inTextureType))
            {
                PSSamplers[samplerIndex].TextureType = inTextureType;
                MarkPSDirty();
            }
        }

        void SetPSSamplerTexCoordWrap(_In_range_(0, D3DHAL_SAMPLER_MAXSAMP - 1) UINT samplerIndex, BYTE inTexCoordWrap)
        {
            Check9on12(samplerIndex < ARRAYSIZE(PSSamplers));
            if (!IsEquals(PSSamplers[samplerIndex].TexCoordWrap, inTexCoordWrap))
            {
                PSSamplers[samplerIndex].TexCoordWrap = inTexCoordWrap;
                MarkPSDirty();
            }
        }

        void ClearTCIMappingFlags(UINT mask)
        {
            if (TCIMapping & mask)
            {
                TCIMapping &= ~mask;
                MarkPSDirty();
            }
        }

        void SetTCIMappingFlags(UINT mask)
        {
            if ((TCIMapping | mask) != TCIMapping)
            {
                TCIMapping |= mask;
                MarkPSDirty();
            }
        }

        void SetProjectedTCsMask(UINT mask)
        {
            if ((ProjectedTCsMask | mask) != ProjectedTCsMask)
            {
                ProjectedTCsMask |= mask;
                MarkPSDirty();
            }
        }

        void ClearProjectedTCsMask(UINT mask)
        {
            if (ProjectedTCsMask & mask)
            {
                ProjectedTCsMask &= ~mask;
                MarkPSDirty();
            }
        }

        void SetColorKeyTSSDisable(UINT mask)
        {
            if ((ColorKeyTSSDisable | mask) != ColorKeyTSSDisable)
            {
                ColorKeyTSSDisable |= mask;
                MarkPSDirty();
            }
        }

        void ClearColorKeyTSSDisable(UINT mask)
        {
            if (ColorKeyTSSDisable & mask)
            {
                ColorKeyTSSDisable &= ~mask;
                MarkPSDirty();
            }
        }

        bool IsUsingPointFill()
        {
            return FillMode == D3DFILL_POINT && PrimitiveType >= D3DPT_TRIANGLELIST;
        }
        
        bool IsUsingPointSpritesOrSizes(const ShaderConv::VSOutputDecls &vsOutputDecls)
        {
            return (PointSizeEnable || PointSpriteEnable || vsOutputDecls.PointSize) && PrimitiveType == D3DPT_POINTLIST;
        }

        BYTE GetPSTexCoordWrap(UINT regIndex)
        {
            return PSSamplers[regIndex].TexCoordWrap;
        }

        D3DSHADEMODE GetShadingMode()
        {
            return (D3DSHADEMODE)ShadeMode;
        }

        void SetHardwareShadowMappingFlag(D3D12TranslationLayer::EShaderStage shaderType, UINT textureIndex, bool enabled)
        {
            Check9on12(textureIndex <= ShaderConv::MAX_PS_SAMPLER_REGS);
            Check9on12(shaderType == D3D12TranslationLayer::e_PS || shaderType == D3D12TranslationLayer::e_VS);

           if (shaderType == D3D12TranslationLayer::e_PS)
           {
               if (enabled)
               {
                   HardwareShadowMappingRequiredPS |= BIT(textureIndex);
               }
               else
               {
                   HardwareShadowMappingRequiredPS &= ~BIT(textureIndex);
               }
               MarkPSDirty();
           }
           else
           {
               if (enabled)
               {
                   HardwareShadowMappingRequiredVS |= BIT(textureIndex);
               }
               else
               {
                   HardwareShadowMappingRequiredVS &= ~BIT(textureIndex);
               }
               MarkVSDirty();
           }
        }

        void SetSwapRBOutputChannelsFlag(UINT outputIndex, bool enabled)
        {
            if (enabled)
            {
                SwapRBOnOutputMask |= BIT(outputIndex);
            }
            else
            {
                SwapRBOnOutputMask &= ~BIT(outputIndex);
            }
            MarkPSDirty();
        }

    private:
        PipelineStateDirtyFlags &m_dirtyFlags;
        void MarkPSDirty()
        {
            m_dirtyFlags.PixelShader = true;
        }

        void MarkVSDirty()
        {
            m_dirtyFlags.VertexShader = true;
        }

        void MarkGSDirty()
        {
        }

        template<typename T>
        FORCEINLINE bool IsEquals(T objA, T objB)
        {
            return objA == objB;
        }
    };

    class VSCBExntensionWrapper : private ShaderConv::VSCBExtension
    {
        const UINT POINT_SIZE_INDEX = 0;
        const UINT MIN_POINT_SIZE_INDEX = 1;
        const UINT MAX_POINT_SIZE_INDEX = 2;
    public:
        VSCBExntensionWrapper(PipelineStateDirtyFlags &dirtyFlags) : m_dirtyFlags(dirtyFlags) {
            memset((void *)&GetVSCBExtension(), 0, sizeof(GetVSCBExtension()));
        }

        const ShaderConv::VSCBExtension &GetUnderlyingData() { return *this; }

        void SetViewportScale(float xScale, float yScale)
        {
            AssertFloatNotInfOrNan(xScale);
            AssertFloatNotInfOrNan(yScale);
            if (xScale != vViewPortScale[0] || yScale != vViewPortScale[1])
            {
                vViewPortScale[0] = xScale;
                vViewPortScale[1] = yScale;

                MarkVSExtensionDirty();
            }
        }
        
        void SetMaxPointSize(float maxPointSize)
        {
            AssertFloatNotInfOrNan(maxPointSize);
            if (vPointSize[MAX_POINT_SIZE_INDEX] != maxPointSize)
            {
                vPointSize[MAX_POINT_SIZE_INDEX] = maxPointSize;
                MarkVSExtensionDirty();
                MarkPointSizeDirty();
            }
        }

        void SetMinPointSize(float minPointSize)
        {
            AssertFloatNotInfOrNan(minPointSize);
            if (vPointSize[MIN_POINT_SIZE_INDEX] != minPointSize)
            {
                vPointSize[MIN_POINT_SIZE_INDEX] = minPointSize;
                MarkVSExtensionDirty();
                MarkPointSizeDirty();
            }
        }

        void SetPointSize(float pointSize)
        {
            AssertFloatNotInfOrNan(pointSize);
            if (vPointSize[POINT_SIZE_INDEX] != pointSize)
            {
                vPointSize[POINT_SIZE_INDEX] = pointSize;
                MarkVSExtensionDirty();
                MarkPointSizeDirty();
            }
        }

        void SetClipPlane(_In_range_(0, ShaderConv::MAX_CLIPLANES - 1) UINT clipPlaneIndex, _In_reads_(4) const float *pClipPlane)
        {
            for (UINT i = 0; i < 4; i++) AssertFloatNotInfOrNan(pClipPlane[i]);

            if (memcmp(pClipPlane, vClipPlanes[clipPlaneIndex], sizeof(vClipPlanes[clipPlaneIndex])) != 0)
            {
                memcpy(vClipPlanes[clipPlaneIndex], pClipPlane, sizeof(vClipPlanes[clipPlaneIndex]));
                MarkVSExtensionDirty();
            }
        }

        void SetScreenToClipOffset(float xOffset, float yOffset, float zOffset, float wOffset = 0.0f)
        {
            AssertFloatNotInfOrNan(xOffset);
            AssertFloatNotInfOrNan(yOffset);
            AssertFloatNotInfOrNan(zOffset);
            AssertFloatNotInfOrNan(wOffset);

            if (xOffset != vScreenToClipOffset[0] || yOffset != vScreenToClipOffset[1] || zOffset != vScreenToClipOffset[2] || wOffset != vScreenToClipOffset[3])
            {
                vScreenToClipOffset[0] = xOffset;
                vScreenToClipOffset[1] = yOffset;
                vScreenToClipOffset[2] = zOffset;
                vScreenToClipOffset[3] = wOffset;
                MarkVSExtensionDirty();
            }
        }

        void SetScreenToClipScale(float xScale, float yScale, float zScale, float wScale = 1.0f)
        {
            AssertFloatNotInfOrNan(xScale);
            AssertFloatNotInfOrNan(yScale);
            AssertFloatNotInfOrNan(zScale);
            AssertFloatNotInfOrNan(wScale);

            if (xScale != vScreenToClipScale[0] || yScale != vScreenToClipScale[1] || zScale != vScreenToClipScale[2] || wScale != vScreenToClipScale[3])
            {
                vScreenToClipScale[0] = xScale;
                vScreenToClipScale[1] = yScale;
                vScreenToClipScale[2] = zScale;
                vScreenToClipScale[3] = wScale;
                MarkVSExtensionDirty();
            }
        }

        void ResolvePointSize()
        {
            const float pointSizeMin = vPointSize[MIN_POINT_SIZE_INDEX];
            const float pointSizeMax = vPointSize[MAX_POINT_SIZE_INDEX];

            // Clamp the point size  
            const float pointSize = min(max(GetPointSize(), pointSizeMin), pointSizeMax);

            // Update the vertex extension  
            SetPointSize(pointSize);
        }

        float GetPointSize()
        {
            return vPointSize[POINT_SIZE_INDEX];
        }

        const ShaderConv::VSCBExtension &GetVSCBExtension()
        {
            return *dynamic_cast<ShaderConv::VSCBExtension *>(this);
        }
            
    private:
        PipelineStateDirtyFlags &m_dirtyFlags;
        void MarkVSExtensionDirty()
        {
            m_dirtyFlags.VSExtension = true;
        }

        void MarkPointSizeDirty()
        {
            m_dirtyFlags.PointSize = true;
        }
    };
};
