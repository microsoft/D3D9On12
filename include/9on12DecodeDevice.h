// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Device; // Forward declaration

    class DecodeDevice
    {
    public:
        DecodeDevice(HANDLE runtimeHandle, _In_ Device *pParent, _In_ const D3DDDIARG_CREATEDECODEDEVICE *pCreateDecodeDevice);
        ~DecodeDevice();

        HANDLE GetRuntimeHandle() { return m_runtimeHandle; }
        Device* GetParent() { return m_pParentDevice; }

        static FORCEINLINE HANDLE GetHandleFromDecodeDevice(DecodeDevice* pDecodeDevice) { return (pDecodeDevice) ? static_cast<HANDLE>(pDecodeDevice) : NULL_HANDLE; }
        static FORCEINLINE DecodeDevice* GetDecodeDeviceFromHandle(HANDLE hDecodeDevice) { return (hDecodeDevice != NULL_HANDLE) ? static_cast<DecodeDevice*>(hDecodeDevice) : nullptr; }
        static void GetCaps(_Inout_ CONST D3DDDIARG_GETCAPS* pGetCaps);

        void BeginFrame(_In_ D3DDDIARG_DECODEBEGINFRAME *pBeginFrame);
        void EndFrame(_Inout_ D3DDDIARG_DECODEENDFRAME *pEndFrame);
        void Execute(_In_ CONST D3DDDIARG_DECODEEXECUTE *pExecute);
        void SetDecodeRenderTarget(_In_ CONST D3DDDIARG_SETDECODERENDERTARGET *pRenderTarget);
        HRESULT ExecuteExtension(_In_ CONST D3DDDIARG_DECODEEXTENSIONEXECUTE* pExtensionExecute);

    protected:
        D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE GetInterlaceType(DXVADDI_SAMPLEFORMAT format);

        BYTE m_pUnderlyingSpace[sizeof(D3D12TranslationLayer::VideoDecode)];
        Device *m_pParentDevice = nullptr;
        HANDLE m_runtimeHandle = NULL_HANDLE;
        D3D12TranslationLayer::VideoDecode *m_pUnderlyingVideoDecode = nullptr;
        GUID m_decodeProfile = GUID_NULL;
        D3D12TranslationLayer::VIDEO_DECODE_INPUT_STREAM_ARGUMENTS m_inputArguments = {};
        D3D12TranslationLayer::VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS m_outputArguments = {};
        UINT m_frameNestCount = 0;
        struct {
            unique_comptr<D3D12TranslationLayer::Resource> frameArguments[D3D12_VIDEO_DECODE_MAX_ARGUMENTS];
            unique_comptr<D3D12TranslationLayer::Resource> compressedBitstream;
            unique_comptr<D3D12TranslationLayer::Resource> output;
        } m_inUseResources;
    };
};
