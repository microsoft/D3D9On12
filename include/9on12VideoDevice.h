// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Device; // Forward declaration

    class VideoDevice
    {
    public:
        VideoDevice(Device *pParent);
        ~VideoDevice();

        void GetCaps(_Inout_ CONST D3DDDIARG_GETCAPS* pGetCaps);

        static D3D12TranslationLayer::VideoDevice* UnderlyingVideoDevice(VideoDevice *pVideoDevice)
        {
            assert(pVideoDevice);
            return pVideoDevice->m_pUnderlyingVideoDevice;
        }

        UINT GetMaxInputStreams() { return m_vpMaxInputStreams; }

        FLOAT GetFilterRangeMultiplier(D3D12_VIDEO_PROCESS_FILTER filter) { return m_filterRanges[filter].Multiplier; }

        bool IsAutoProcessingSupported() { return m_pUnderlyingVideoProcessEnum->IsAutoProcessingSupported(); }

        bool IsAlphaBlendProcessingSupported() { return m_fAlphaBlending; }

    protected:
        template<typename T>
        void Preamble(_In_ CONST D3DDDIARG_GETCAPS* pGetCaps, bool expression);
        void CacheVideoProcessInfo(_In_ D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS &args);

        UINT GetFilteredFormatsCount(_In_ const std::vector<DXGI_FORMAT> &vpFormats);

        BYTE m_pUnderlyingSpace[sizeof(D3D12TranslationLayer::VideoDevice)];
        D3D12TranslationLayer::VideoDevice *m_pUnderlyingVideoDevice = nullptr;
        BYTE m_pUnderlyingSpaceVPEnum[sizeof(D3D12TranslationLayer::VideoProcessEnum)];
        D3D12TranslationLayer::VideoProcessEnum *m_pUnderlyingVideoProcessEnum = nullptr;
        Device *m_pParentDevice = nullptr;
        DXVAHDDDI_VPDEVCAPS m_vpDXVAHDDevCaps;
        std::vector<DXVAHDDDI_VPCAPS> m_vpDXVAHDCaps;
        const UINT m_maxVideoProcessors = 3;  // Progressive, Bob, and Adaptive
        D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS m_currentVideoProcessArgs = {};
        static const UINT MAX_VIDEO_PROCESSOR_FILTERS = D3D12_VIDEO_PROCESS_MAX_FILTERS;
        DXVAHDDDI_FILTER_RANGE_DATA m_filterRanges[MAX_VIDEO_PROCESSOR_FILTERS] = {};
        UINT m_vpMaxInputStreams = 0;
        bool m_fAlphaBlending = false;
    };
};
