// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    VideoDevice::VideoDevice(Device *pParent) :
        m_pParentDevice(pParent)
    {
        m_pUnderlyingVideoDevice = new (m_pUnderlyingSpace)D3D12TranslationLayer::VideoDevice(&m_pParentDevice->GetContext());

        m_pUnderlyingVideoProcessEnum = new (m_pUnderlyingSpaceVPEnum)D3D12TranslationLayer::VideoProcessEnum(&m_pParentDevice->GetContext());
        m_pUnderlyingVideoProcessEnum->Initialize();

        // Cache max input streams info
        {
            D3D12_FEATURE_DATA_VIDEO_PROCESS_MAX_INPUT_STREAMS vpMaxInputStreams = {};
            m_pUnderlyingVideoProcessEnum->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_MAX_INPUT_STREAMS, &vpMaxInputStreams, sizeof(vpMaxInputStreams));
            m_vpMaxInputStreams = vpMaxInputStreams.MaxInputStreams;
        }

        m_vpDXVAHDCaps.reserve(m_maxVideoProcessors);
    }

    VideoDevice::~VideoDevice()
    {
        if (m_pUnderlyingVideoDevice)
        {
            m_pUnderlyingVideoDevice->~VideoDevice();
        }
        if (m_pUnderlyingVideoProcessEnum)
        {
            m_pUnderlyingVideoProcessEnum->~VideoProcessEnum();
        }
    }

    UINT FilterId(UINT ProcAmpCap)
    {
        UINT filter = 0;
        while (ProcAmpCap)
        {
            if (ProcAmpCap & 0x1)
            {
                return filter;
            }
            ++filter;
            ProcAmpCap >>= 1;
        }
        ThrowFailure(E_INVALIDARG);
        return 0;
    }

    _Use_decl_annotations_
    UINT VideoDevice::GetFilteredFormatsCount(const std::vector<DXGI_FORMAT> &vpFormats)
    {
        FORMATOP rgFormatOps[Adapter::MAX_SURFACE_FORMATS];
        UINT NumFormatOps = m_pParentDevice->GetAdapter().GetFormatData(rgFormatOps, _countof(rgFormatOps));

        UINT count = 0;
        for(UINT i = 0; i < (UINT)vpFormats.size(); i++)
        {
            for (UINT j = 0; j < NumFormatOps; j++)
            {
                if (rgFormatOps[j].Format == ConvertFromDXGIFormatToD3DDDIFormat(vpFormats[i]))
                {
                    count++;
                    break;
                }
            }
        }

        return count;
    };

    _Use_decl_annotations_
    void VideoDevice::GetCaps(CONST D3DDDIARG_GETCAPS* pGetCaps)
    {
        // VP output format order communicates preference.  Media pipeline attempts to create a swapchain with the first format
        // in the list, followed by the next, etc.  Move valid swapchain formats to the beginning of the list while preserving
        // the order of the remaining formats.
        auto pfnIsBackBufferSortRank = [](D3DDDIFORMAT fmt) 
        { 
            switch (fmt)
            {
                case D3DDDIFMT_A8R8G8B8:
                    return 0;
                case D3DDDIFMT_X8R8G8B8:
                    return 1;
            }

            return 100; // High default rank to allow more to be added above.
        };

        auto pfnGetFormats = [&](const std::vector<DXGI_FORMAT> &vpFormats, const std::function<UINT (D3DDDIFORMAT)>& fnSortRank = {})
        {
            FORMATOP rgFormatOps[Adapter::MAX_SURFACE_FORMATS];
            UINT NumFormatOps = m_pParentDevice->GetAdapter().GetFormatData(rgFormatOps, _countof(rgFormatOps));

            D3DDDIFORMAT *pD3DFormats = (D3DDDIFORMAT *)pGetCaps->pData;
            UINT count = pGetCaps->DataSize / sizeof(D3DDDIFORMAT);

            for (UINT unFilteredIndex = 0, filteredIndex = 0; unFilteredIndex < (UINT)vpFormats.size() && filteredIndex < count; unFilteredIndex++)
            {
                D3DDDIFORMAT d3dVpFmt = ConvertFromDXGIFormatToD3DDDIFormat(vpFormats[unFilteredIndex]); 

                for (UINT adapterFormatIndex = 0; adapterFormatIndex < NumFormatOps; adapterFormatIndex++)
                {
                    if (rgFormatOps[adapterFormatIndex].Format == d3dVpFmt)
                    {
                        pD3DFormats[filteredIndex++] = d3dVpFmt;
                        break;
                    }
                }
            }

            if (fnSortRank)
            {
                std::stable_sort(pD3DFormats, pD3DFormats + count, [fnSortRank](D3DDDIFORMAT fmt1, D3DDDIFORMAT fmt2){ return fnSortRank(fmt1) < fnSortRank(fmt2);});
            }
        };

        switch (pGetCaps->Type)
        {
            case D3DDDICAPS_GETDECODEGUIDCOUNT:
            {
                Check9on12(pGetCaps->DataSize == sizeof(UINT));
                UINT *pCount = (UINT *)pGetCaps->pData;
                m_pUnderlyingVideoDevice->GetVideoDecoderProfileCount(pCount);
            }
            break;

            case D3DDDICAPS_GETDECODEGUIDS:
            {
                Check9on12(pGetCaps->DataSize % sizeof(GUID) == 0);
                UINT count = pGetCaps->DataSize / sizeof(GUID);
                GUID *pGuids = (GUID *)pGetCaps->pData;
                for (DWORD i = 0; i < count; i++)
                {
                    m_pUnderlyingVideoDevice->GetVideoDecoderProfile(i, &pGuids[i]);
                }
            }
            break;

            case D3DDDICAPS_GETDECODERTFORMATCOUNT:
            {
                Check9on12(pGetCaps->DataSize == sizeof(UINT));
                GUID *pDecodeProfile = (GUID *)pGetCaps->pInfo;
                UINT *pCount = (UINT *)pGetCaps->pData;
                m_pUnderlyingVideoDevice->GetVideoDecoderFormatCount(pDecodeProfile, pCount);
            }
            break;

            case D3DDDICAPS_GETDECODERTFORMATS:
            {
                Check9on12(pGetCaps->DataSize % sizeof(D3DDDIFORMAT) == 0);
                GUID *pDecodeProfile = (GUID *)pGetCaps->pInfo;
                UINT count = pGetCaps->DataSize / sizeof(D3DDDIFORMAT);
                D3DDDIFORMAT *pD3DFormats = (D3DDDIFORMAT *)pGetCaps->pData;
                for (DWORD i = 0; i < count; i++)
                {
                    DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
                    m_pUnderlyingVideoDevice->GetVideoDecoderFormat(pDecodeProfile, i, &dxgiFormat);
                    pD3DFormats[i] = ConvertFromDXGIFormatToD3DDDIFormat(dxgiFormat);
                }
            }
            break;

            case D3DDDICAPS_GETDECODECONFIGURATIONCOUNT:
            {
                Check9on12(pGetCaps->DataSize == sizeof(UINT));
                DXVADDI_DECODEINPUT *pDecodeInput = (DXVADDI_DECODEINPUT *)pGetCaps->pInfo;
                UINT *pCount = (UINT *)pGetCaps->pData;
                D3D12TranslationLayer::VIDEO_DECODE_DESC desc = {};
                VideoTranslate::VideoDecodeDesc(pDecodeInput, &desc);
                m_pUnderlyingVideoDevice->GetVideoDecoderConfigCount(&desc, pCount);
            }
            break;

            case D3DDDICAPS_GETDECODECONFIGURATIONS:
            {
                Check9on12(pGetCaps->DataSize % sizeof(DXVADDI_CONFIGPICTUREDECODE) == 0);
                DXVADDI_DECODEINPUT *pDecodeInput = (DXVADDI_DECODEINPUT *)pGetCaps->pInfo;
                UINT count = pGetCaps->DataSize / sizeof(DXVADDI_CONFIGPICTUREDECODE);
                D3D12TranslationLayer::VIDEO_DECODE_DESC desc = {};
                VideoTranslate::VideoDecodeDesc(pDecodeInput, &desc);
                DXVADDI_CONFIGPICTUREDECODE *pDXVAConfigs = (DXVADDI_CONFIGPICTUREDECODE *)pGetCaps->pData;
                for (DWORD i = 0; i < count; i++)
                {
                    D3D12TranslationLayer::VIDEO_DECODE_CONFIG config = {};
                    m_pUnderlyingVideoDevice->GetVideoDecoderConfig(&desc, i, &config);
                    VideoTranslate::VideoDecodeConfig(&desc, &config, &pDXVAConfigs[i]);
                }
            }
            break;

            case D3DDDICAPS_GETDECODECOMPRESSEDBUFFERINFOCOUNT:
            {
                Check9on12(pGetCaps->DataSize == sizeof(UINT));
                DXVADDI_DECODEINPUT *pDecodeInput = (DXVADDI_DECODEINPUT *)pGetCaps->pInfo;
                UINT *pCount = (UINT *)pGetCaps->pData;
                D3D12TranslationLayer::VIDEO_DECODE_DESC desc = {};
                VideoTranslate::VideoDecodeDesc(pDecodeInput, &desc);
                m_pUnderlyingVideoDevice->GetVideoDecoderBufferTypeCount(&desc, pCount);
            }
            break;

            case D3DDDICAPS_GETDECODECOMPRESSEDBUFFERINFO:
            {
                Check9on12(pGetCaps->DataSize % sizeof(DXVADDI_DECODEBUFFERINFO) == 0);
                DXVADDI_DECODEINPUT *pDecodeInput = (DXVADDI_DECODEINPUT *)pGetCaps->pInfo;
                UINT count = pGetCaps->DataSize / sizeof(DXVADDI_DECODEBUFFERINFO);
                D3D12TranslationLayer::VIDEO_DECODE_DESC desc = {};
                VideoTranslate::VideoDecodeDesc(pDecodeInput, &desc);
                DXVADDI_DECODEBUFFERINFO *pDXVABufferInfo = (DXVADDI_DECODEBUFFERINFO *)pGetCaps->pData;
                for (DWORD i = 0; i < count; i++)
                {
                    UINT size;
                    D3D12TranslationLayer::VIDEO_DECODE_BUFFER_TYPE type;
                    m_pUnderlyingVideoDevice->GetVideoDecoderBufferInfo(&desc, i, &type, &size);
                    VideoTranslate::VideoDecodeBufferInfo(type, size, &pDXVABufferInfo[i]);
                }
            }
            break;

            case D3DDDICAPS_GETVIDEOPROCESSORRTFORMATCOUNT:
            case D3DDDICAPS_GETVIDEOPROCESSORRTSUBSTREAMFORMATCOUNT:
            {
                Preamble<DXVADDI_VIDEOPROCESSORINPUT>(pGetCaps, pGetCaps->DataSize == sizeof(UINT));
                *reinterpret_cast<UINT*>(pGetCaps->pData) = GetFilteredFormatsCount(m_pUnderlyingVideoProcessEnum->GetVPOutputFormats());
            }
            break;

            case D3DDDICAPS_GETVIDEOPROCESSORRTFORMATS:
            case D3DDDICAPS_GETVIDEOPROCESSORRTSUBSTREAMFORMATS:
            {
                Preamble<DXVADDI_VIDEOPROCESSORINPUT>(pGetCaps, pGetCaps->DataSize % sizeof(D3DDDIFORMAT) == 0);
                pfnGetFormats(m_pUnderlyingVideoProcessEnum->GetVPOutputFormats(), pfnIsBackBufferSortRank);
            }
            break;

            case D3DDDICAPS_GETVIDEOPROCESSORDEVICEGUIDCOUNT:
            {
                Preamble<DXVADDI_VIDEODESC>(pGetCaps, pGetCaps->DataSize == sizeof(UINT));
                *reinterpret_cast<UINT*>(pGetCaps->pData) = (UINT)m_vpDXVAHDCaps.size();
            }
            break;

            case D3DDDICAPS_GETVIDEOPROCESSORDEVICEGUIDS:
            {
                Preamble<DXVADDI_VIDEODESC>(pGetCaps, pGetCaps->DataSize % sizeof(GUID) == 0);

                UINT count = min(pGetCaps->DataSize / (UINT)sizeof(GUID), (UINT)m_vpDXVAHDCaps.size());
                GUID *pGuid = (GUID *)pGetCaps->pData;
                for (UINT i = 0; i < count; i++)
                {
                    pGuid[i] = m_vpDXVAHDCaps[i].VPGuid;
                }
            }
            break;

            case D3DDDICAPS_GETVIDEOPROCESSORCAPS:
            {
                Preamble<DXVADDI_VIDEOPROCESSORINPUT>(pGetCaps, pGetCaps->DataSize == sizeof(DXVADDI_VIDEOPROCESSORCAPS));

                DXVADDI_VIDEOPROCESSORINPUT *pVideoProcessorInput = static_cast<DXVADDI_VIDEOPROCESSORINPUT *>(pGetCaps->pInfo);
                DXVADDI_VIDEOPROCESSORCAPS *pVPCaps = (DXVADDI_VIDEOPROCESSORCAPS *)pGetCaps->pData;

                size_t dxvaCapsIndex = 0;
                for(;dxvaCapsIndex < m_vpDXVAHDCaps.size(); ++dxvaCapsIndex)
                {
                    if (*pVideoProcessorInput->pVideoProcGuid == m_vpDXVAHDCaps[dxvaCapsIndex].VPGuid)
                    {
                        break;
                    }
                }

                if (dxvaCapsIndex >= m_vpDXVAHDCaps.size())
                {
                    assert(m_vpDXVAHDCaps.size() >= 1);

                    // The requested DXVA guid was not found.  9on12 originally shipped without checking the incomming
                    // guid.  It's not know if failing at this point would introduce an app compat problem as a number of
                    // vendor defined GUIDs exist.
                    dxvaCapsIndex = 0;
                }

                pVPCaps->InputPool = m_vpDXVAHDDevCaps.InputPool;
                pVPCaps->NumForwardRefSamples = m_vpDXVAHDCaps[dxvaCapsIndex].FutureFrames;
                pVPCaps->NumBackwardRefSamples = m_vpDXVAHDCaps[dxvaCapsIndex].PastFrames;
                pVPCaps->OutputFormat = pVideoProcessorInput->RenderTargetFormat;
                pVPCaps->DeinterlaceTechnology |= (m_vpDXVAHDCaps[dxvaCapsIndex].ProcessorCaps & DXVAHDDDI_PROCESSOR_CAPS_DEINTERLACE_BOB) ? DXVADDI_DEINTERLACETECH_BOBLINEREPLICATE : 0;
                pVPCaps->DeinterlaceTechnology |= (m_vpDXVAHDCaps[dxvaCapsIndex].ProcessorCaps & DXVAHDDDI_PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE) ? DXVADDI_DEINTERLACETECH_FIELDADAPTIVE : 0;
                pVPCaps->ProcAmpControlCaps = m_vpDXVAHDDevCaps.FilterCaps & ~(DXVAHDDDI_FILTER_CAPS_NOISE_REDUCTION | DXVAHDDDI_FILTER_CAPS_EDGE_ENHANCEMENT | DXVAHDDDI_FILTER_CAPS_ANAMORPHIC_SCALING);

                // Minimal set of VideoProcessorOperations for dxva Caps
                pVPCaps->VideoProcessorOperations |= DXVADDI_VIDEOPROCESS_YUV2RGB | DXVADDI_VIDEOPROCESS_STRETCHX | DXVADDI_VIDEOPROCESS_STRETCHY | DXVADDI_VIDEOPROCESS_SUBRECTS;

                pVPCaps->VideoProcessorOperations |= (m_vpDXVAHDDevCaps.FeatureCaps & DXVAHDDDI_FEATURE_CAPS_ALPHA_FILL) ? DXVADDI_VIDEOPROCESS_PLANARALPHA : 0;
                pVPCaps->VideoProcessorOperations |= (m_vpDXVAHDDevCaps.DeviceCaps & DXVAHDDDI_DEVICE_CAPS_LINEAR_SPACE) ? DXVADDI_VIDEOPROCESS_LINEARSCALING : 0;
                pVPCaps->VideoProcessorOperations |=  (m_vpDXVAHDDevCaps.MaxInputStreams > 1) ? (DXVADDI_VIDEOPROCESS_SUBSTREAMS | DXVADDI_VIDEOPROCESS_SUBSTREAMSEXTENDED) : 0;
                pVPCaps->VideoProcessorOperations |= m_fAlphaBlending ? DXVADDI_VIDEOPROCESS_ALPHABLEND : 0;

                pVPCaps->NoiseFilterTechnology = 0;
                pVPCaps->DetailFilterTechnology = 0;
            }
            break;

            case D3DDDICAPS_GETPROCAMPRANGE:
            {
                // TODO: should we return E_NOTIMPL? since there's a mismatch between setting the PROCAMP values in input (DX12) x output (DXVA)
                // Should we add instead output control to DX12?
                Preamble<DXVADDI_QUERYPROCAMPINPUT>(pGetCaps, pGetCaps->DataSize == sizeof(DXVADDI_VALUERANGE));
                DXVADDI_QUERYPROCAMPINPUT *pInput = (DXVADDI_QUERYPROCAMPINPUT *)pGetCaps->pInfo;
                DXVADDI_VALUERANGE *pValue = (DXVADDI_VALUERANGE *)pGetCaps->pData;
                UINT filter = FilterId(pInput->ProcAmpCap);
                pValue->MinValue = VideoTranslate::ToFixed32<INT>(m_filterRanges[filter].Minimum);
                pValue->MaxValue = VideoTranslate::ToFixed32<INT>(m_filterRanges[filter].Maximum);
                pValue->DefaultValue = VideoTranslate::ToFixed32<INT>(m_filterRanges[filter].Default);
                pValue->StepSize = VideoTranslate::ToFixed32<FLOAT>(m_filterRanges[filter].Multiplier);
            }
            break;

            case D3DDDICAPS_DXVAHD_GETVPOUTPUTFORMATS:
            {
                Preamble<DXVAHDDDI_DEVICE_DESC>(pGetCaps, pGetCaps->DataSize % sizeof(D3DDDIFORMAT) == 0);
                pfnGetFormats(m_pUnderlyingVideoProcessEnum->GetVPOutputFormats(), pfnIsBackBufferSortRank);
            }
            break;

            case D3DDDICAPS_DXVAHD_GETVPINPUTFORMATS:
            {
                Preamble<DXVAHDDDI_DEVICE_DESC>(pGetCaps, pGetCaps->DataSize % sizeof(D3DDDIFORMAT) == 0);
                pfnGetFormats(m_pUnderlyingVideoProcessEnum->GetVPInputFormats());
            }
            break;

            case D3DDDICAPS_DXVAHD_GETVPDEVCAPS:
            {
                Preamble<DXVAHDDDI_DEVICE_DESC>(pGetCaps, pGetCaps->DataSize == sizeof(DXVAHDDDI_VPDEVCAPS));
                DXVAHDDDI_VPDEVCAPS *pVPDevCaps = (DXVAHDDDI_VPDEVCAPS *)pGetCaps->pData;
                *pVPDevCaps = m_vpDXVAHDDevCaps;
            }
            break;

            case D3DDDICAPS_DXVAHD_GETVPCAPS:
            {
                Preamble<DXVAHDDDI_DEVICE_DESC>(pGetCaps, pGetCaps->DataSize % sizeof(DXVAHDDDI_VPCAPS) == 0);
                DXVAHDDDI_VPCAPS *pVPCaps = (DXVAHDDDI_VPCAPS *)pGetCaps->pData;
                UINT count = pGetCaps->DataSize / sizeof(DXVAHDDDI_VPCAPS);
                for (UINT i = 0; i < count; i++)
                {
                    pVPCaps[i] = m_vpDXVAHDCaps[i];
                }
            }
            break;

            case D3DDDICAPS_DXVAHD_GETVPCUSTOMRATES:
            {
                Check9on12(pGetCaps->DataSize % sizeof(DXVAHDDDI_CUSTOM_RATE_DATA) == 0);
#pragma warning( suppress :22107)  
                // The input structure has no SAL annotation and cannot be fixed easily, so suppress instead  
                ZeroMemory(pGetCaps->pData, pGetCaps->DataSize);
            }
            break;

            case D3DDDICAPS_DXVAHD_GETVPFILTERRANGE:
            {
                Check9on12(pGetCaps->DataSize == sizeof(DXVAHDDDI_FILTER_RANGE_DATA));
#pragma warning( suppress :22107)  
                // The input structure has no SAL annotation and cannot be fixed easily, so suppress instead  
                ZeroMemory(pGetCaps->pData, pGetCaps->DataSize);
                DXVAHDDDI_FILTER_RANGE_DATA *pFilterRange = (DXVAHDDDI_FILTER_RANGE_DATA *)pGetCaps->pData;
                DXVAHDDDI_FILTER filter = *(DXVAHDDDI_FILTER  *)pGetCaps->pInfo;
                if ((UINT)filter > DXVAHDDDI_FILTER_ANAMORPHIC_SCALING || (UINT)filter > MAX_VIDEO_PROCESSOR_FILTERS)
                {
                    ThrowFailure(E_INVALIDARG);
                }
                *pFilterRange = m_filterRanges[filter];
            }
            break;

            case D3DDDICAPS_GETEXTENSIONGUIDCOUNT:
            {
                Check9on12(pGetCaps->DataSize % sizeof(UINT) == 0);
#pragma warning( suppress :22107)  
                // The input structure has no SAL annotation and cannot be fixed easily, so suppress instead  
                ZeroMemory(pGetCaps->pData, pGetCaps->DataSize);
            }
            break;

            case D3DDDICAPS_GETEXTENSIONGUIDS:
            {
                Check9on12(pGetCaps->DataSize % sizeof(GUID) == 0);
#pragma warning( suppress :22107)  
                // The input structure has no SAL annotation and cannot be fixed easily, so suppress instead  
                ZeroMemory(pGetCaps->pData, pGetCaps->DataSize);
            }
            break;

            case D3DDDICAPS_FILTERPROPERTYRANGE:        // invalid filters in DX12
            case D3DDDICAPS_GETEXTENSIONCAPS:           // no extension guids were returned, so invalid to call extension caps.
            default:
                ZeroMemory(pGetCaps->pData, pGetCaps->DataSize);
                ThrowFailure(E_UNEXPECTED);
                break;
        }
    }

    template<typename T>
    _Use_decl_annotations_
    void VideoDevice::Preamble(CONST D3DDDIARG_GETCAPS* pGetCaps, bool expression)
    {
        Check9on12(expression);
#pragma warning( suppress :22107)  
        // The input structure has no SAL annotation and cannot be fixed easily, so suppress instead  
        ZeroMemory(pGetCaps->pData, pGetCaps->DataSize);

        D3DDDIFORMAT rtFormat;
        T *pInfo = (T *)pGetCaps->pInfo;
        D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS args;
        VideoTranslate::VideoProcessDesc(pInfo, m_vpMaxInputStreams, args, rtFormat);
        CacheVideoProcessInfo(args);
    }

    _Use_decl_annotations_
    void VideoDevice::CacheVideoProcessInfo(D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS &args)
    {
        if (m_currentVideoProcessArgs.InputFieldType != args.InputFieldType ||
            m_currentVideoProcessArgs.InputWidth != args.InputWidth ||
            m_currentVideoProcessArgs.InputHeight != args.InputHeight ||
            m_currentVideoProcessArgs.OutputWidth != args.OutputWidth ||
            m_currentVideoProcessArgs.OutputHeight != args.OutputHeight)
        {
            m_pUnderlyingVideoProcessEnum->CacheVideoProcessInfo(args);

            std::vector<D3D12TranslationLayer::VIDEO_PROCESS_SUPPORT> vpSupportTuples = m_pUnderlyingVideoProcessEnum->GetVPCapsSupportTuples();

            DXVAHDDDI_VPCAPS vpDXVAHDCaps = {};
            ZeroMemory(&m_vpDXVAHDDevCaps, sizeof(m_vpDXVAHDDevCaps));
            m_fAlphaBlending = false;

            for (auto& tuple : vpSupportTuples)
            {
                // translate to DXVA & DXVA HD CAPS
                VideoTranslate::AddVideoProcessCaps(tuple.dx12Support, m_vpDXVAHDDevCaps, m_filterRanges, _countof(m_filterRanges), tuple.colorConversionCaps, m_fAlphaBlending);
            }

            D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS deinterlaceFlags = m_pUnderlyingVideoProcessEnum->GetDeinterlaceSupport();

            m_vpDXVAHDCaps.resize(1);
            m_vpDXVAHDCaps[0] = vpDXVAHDCaps;
            m_vpDXVAHDCaps[0].VPGuid = DXVADDI_VideoProcProgressiveDevice;
            
            auto referenceInfo = m_pUnderlyingVideoProcessEnum->UpdateReferenceInfo(D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_NONE);
            m_vpDXVAHDCaps[0].PastFrames = referenceInfo.pastFrames;
            m_vpDXVAHDCaps[0].FutureFrames = referenceInfo.futureFrames;
            m_vpDXVAHDCaps[0].ProcessorCaps |= referenceInfo.frameRateConversionSupported ? DXVAHDDDI_PROCESSOR_CAPS_FRAME_RATE_CONVERSION : 0;

            if (deinterlaceFlags & D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB)
            {
                m_vpDXVAHDCaps.resize(2);
                m_vpDXVAHDCaps[1] = vpDXVAHDCaps;
                m_vpDXVAHDCaps[1].VPGuid = DXVADDI_VideoProcBobDevice;

                referenceInfo = m_pUnderlyingVideoProcessEnum->UpdateReferenceInfo(D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB);
                m_vpDXVAHDCaps[1].PastFrames = referenceInfo.pastFrames;
                m_vpDXVAHDCaps[1].FutureFrames = referenceInfo.futureFrames;
                m_vpDXVAHDCaps[1].ProcessorCaps |= referenceInfo.frameRateConversionSupported ? DXVAHDDDI_PROCESSOR_CAPS_FRAME_RATE_CONVERSION : 0;
                m_vpDXVAHDCaps[1].ProcessorCaps |= DXVAHDDDI_PROCESSOR_CAPS_DEINTERLACE_BOB;

                if (deinterlaceFlags & D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM)
                {
                    m_vpDXVAHDCaps.resize(3);
                    m_vpDXVAHDCaps[2] = vpDXVAHDCaps;
                    m_vpDXVAHDCaps[2].VPGuid = DXVADDI_VideoProcD3D9On12CustomDeinterlaceDevice;

                    referenceInfo = m_pUnderlyingVideoProcessEnum->UpdateReferenceInfo(deinterlaceFlags);
                    m_vpDXVAHDCaps[2].PastFrames = referenceInfo.pastFrames;
                    m_vpDXVAHDCaps[2].FutureFrames = referenceInfo.futureFrames;
                    m_vpDXVAHDCaps[2].ProcessorCaps |= referenceInfo.frameRateConversionSupported ? DXVAHDDDI_PROCESSOR_CAPS_FRAME_RATE_CONVERSION : 0;
                    m_vpDXVAHDCaps[2].ProcessorCaps |= DXVAHDDDI_PROCESSOR_CAPS_DEINTERLACE_BOB | DXVAHDDDI_PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE;
                }
            }

            m_vpDXVAHDDevCaps.InputPool = D3DDDIPOOL_VIDEOMEMORY;
            m_vpDXVAHDDevCaps.InputFormatCount = GetFilteredFormatsCount(m_pUnderlyingVideoProcessEnum->GetVPInputFormats());
            m_vpDXVAHDDevCaps.OutputFormatCount = GetFilteredFormatsCount(m_pUnderlyingVideoProcessEnum->GetVPOutputFormats());
            m_vpDXVAHDDevCaps.VideoProcessorCount = (UINT)m_vpDXVAHDCaps.size();

            // translate max input streams info
            VideoTranslate::AddVideoProcessCaps(m_vpMaxInputStreams, m_vpDXVAHDDevCaps);

            m_currentVideoProcessArgs = args;
        }
    }

};
