// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Device; // Forward declaration

    typedef std::map<DXVAHDDDI_STREAM_STATE, std::vector<BYTE>> DXVAHDStreamState;

    class VideoProcessDevice
    {
    public:
        VideoProcessDevice(HANDLE runtimeHandle, _In_ Device *pParent, _In_ const D3DDDIARG_CREATEVIDEOPROCESSDEVICE *pCreateVideoProcessDevice);
        VideoProcessDevice(Device *pParent, _In_ const D3DDDIARG_DXVAHD_CREATEVIDEOPROCESSOR  *pCreateVideoProcessDevice);
        ~VideoProcessDevice();

        Device* GetParent() { return m_pParentDevice; }

        static FORCEINLINE HANDLE GetHandleFromVideoProcessDevice(VideoProcessDevice* pVideoProcessDevice) { return (pVideoProcessDevice) ? static_cast<HANDLE>(pVideoProcessDevice) : NULL_HANDLE; }
        static FORCEINLINE VideoProcessDevice* GetVideoProcessDeviceFromHandle(HANDLE hVideoProcessDevice) { return (hVideoProcessDevice != NULL_HANDLE) ? static_cast<VideoProcessDevice*>(hVideoProcessDevice) : nullptr; }

        void SetRenderTarget(_In_ CONST D3DDDIARG_SETVIDEOPROCESSRENDERTARGET *pRenderTarget);
        void Blit(_In_ CONST D3DDDIARG_VIDEOPROCESSBLT *pVideoProcessBlt);
        void SetBlitState(_In_ CONST D3DDDIARG_DXVAHD_SETVIDEOPROCESSBLTSTATE *pBltState);
        void SetStreamState(_In_ CONST D3DDDIARG_DXVAHD_SETVIDEOPROCESSSTREAMSTATE *pStreamState);
        void BlitHD(_In_ CONST D3DDDIARG_DXVAHD_VIDEOPROCESSBLTHD *pBlitHD);

    protected:
        void SetFilter(D3D12_VIDEO_PROCESS_FILTER filter, _In_ D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pStreamDesc, _In_ D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1 *pStreamArguments, INT level);
        void SetViewInfo(HANDLE hResource, UINT subresourceIndex, _Out_ D3D12TranslationLayer::VideoProcessView *pView);
        void FillReferenceSet(_In_ D3D12TranslationLayer::VIDEO_PROCESS_STREAM_INFO *pStreamInfo, UINT view, _In_ const DXVAHDDDI_SURFACE &InputSurface, UINT numPastFrames, _In_ const DXVAHDDDI_SURFACE *pPastSurfaces, UINT numFutureFrames, _In_ const DXVAHDDDI_SURFACE* pFutureSurfaces);
        void SetStreamState(UINT streamIndex, D3D12TranslationLayer::VideoProcessView *pStreamView, DXVAHDDDI_STREAM_STATE state, _In_ const std::vector<BYTE> &streamStateData);

        BYTE m_pUnderlyingSpace[sizeof(D3D12TranslationLayer::VideoProcess)];
        Device *m_pParentDevice = nullptr;
        D3D12TranslationLayer::VideoProcess *m_pUnderlyingVideoProcess = nullptr;
        UINT m_MaxInputStreams = 0;
        D3D12TranslationLayer::VIDEO_PROCESS_INPUT_ARGUMENTS m_inputArguments = {};
        D3D12TranslationLayer::VIDEO_PROCESS_OUTPUT_ARGUMENTS m_outputArguments = {};
        D3D12TranslationLayer::VideoProcessView m_targetView;
        bool m_colorSpaceNeverSet = true;
        std::vector<bool> m_IsStreamPlanarAlphaBlendInitialized;
        struct {
            unique_comptr<D3D12TranslationLayer::Resource> target;
        } m_inUseResources;

        DXVAHDDDI_BLT_STATE_OUTPUT_COLOR_SPACE_DATA m_dxvaHDColorSpaceData = {};
        std::vector<DXVAHDStreamState> m_dxvaHDStreamStates;
    };
};
