// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY CreateDecodeDevice(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEDECODEDEVICE *pCreateDecodeDevice)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr  || pCreateDecodeDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        DecodeDevice *pDecodeDevice = new DecodeDevice(pCreateDecodeDevice->hDecode, pDevice, pCreateDecodeDevice);
        if (pDecodeDevice == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        pCreateDecodeDevice->hDecode = DecodeDevice::GetHandleFromDecodeDevice(pDecodeDevice);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DestroyDecodeDevice(_In_ HANDLE /*hDevice*/, _In_ HANDLE hDecodeDevice)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        DecodeDevice *pDecodeDevice = DecodeDevice::GetDecodeDeviceFromHandle(hDecodeDevice);

        if (pDecodeDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        delete pDecodeDevice;

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DecodeBeginFrame(_In_ HANDLE /*hDevice*/, _In_ D3DDDIARG_DECODEBEGINFRAME *pBeginFrame)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        DecodeDevice *pDecodeDevice = DecodeDevice::GetDecodeDeviceFromHandle(pBeginFrame->hDecode);
        if (pDecodeDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pDecodeDevice->BeginFrame(pBeginFrame);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DecodeEndFrame(_In_ HANDLE /*hDevice*/, _Inout_ D3DDDIARG_DECODEENDFRAME *pEndFrame)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        DecodeDevice *pDecodeDevice = DecodeDevice::GetDecodeDeviceFromHandle(pEndFrame->hDecode);
        if (pDecodeDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pDecodeDevice->EndFrame(pEndFrame);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DecodeExecute(_In_ HANDLE /*hDevice*/, _In_ CONST D3DDDIARG_DECODEEXECUTE *pExecute)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        DecodeDevice *pDecodeDevice = DecodeDevice::GetDecodeDeviceFromHandle(pExecute->hDecode);
        if (pDecodeDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pDecodeDevice->Execute(pExecute);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DecodeExtensionExecuter(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_DECODEEXTENSIONEXECUTE* pExtensionExecute)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = E_NOTIMPL;
        if (pExtensionExecute->Function == DXVA_STATUS_REPORTING_FUNCTION)
        {
            DecodeDevice *pDecodeDevice = DecodeDevice::GetDecodeDeviceFromHandle(pExtensionExecute->hDecode);
            if (pDecodeDevice == nullptr)
            {
                RETURN_E_INVALIDARG_AND_CHECK();
            }
            hr = pDecodeDevice->ExecuteExtension(pExtensionExecute);
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY SetDecodeRenderTarget(_In_ HANDLE /* hDevice */, _In_ CONST D3DDDIARG_SETDECODERENDERTARGET *pRenderTarget)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        DecodeDevice *pDecodeDevice = DecodeDevice::GetDecodeDeviceFromHandle(pRenderTarget->hDecode);
        if (pDecodeDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pDecodeDevice->SetDecodeRenderTarget(pRenderTarget);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Use_decl_annotations_
    DecodeDevice::DecodeDevice(HANDLE runtimeHandle, Device *pParent, const D3DDDIARG_CREATEDECODEDEVICE *pCreateDecodeDevice) :
        m_runtimeHandle(runtimeHandle),
        m_pParentDevice(pParent)
    {
        D3D12TranslationLayer::VideoDecodeCreationArgs createArgs = {};

        if (pCreateDecodeDevice->pConfig->guidConfigMBcontrolEncryption != DXVA_NoEncrypt ||
            pCreateDecodeDevice->pConfig->guidConfigResidDiffEncryption != DXVA_NoEncrypt ||
            pCreateDecodeDevice->pConfig->ConfigBitstreamRaw == 0 ||
            pCreateDecodeDevice->pConfig->ConfigResidDiffHost != 0 ||
            pCreateDecodeDevice->pConfig->ConfigSpatialResid8 != 0 ||
            pCreateDecodeDevice->pConfig->ConfigResid8Subtraction != 0 ||
            pCreateDecodeDevice->pConfig->ConfigSpatialHost8or9Clipping != 0 ||
            pCreateDecodeDevice->pConfig->ConfigSpatialResidInterleaved != 0 ||
            pCreateDecodeDevice->pConfig->ConfigIntraResidUnsigned != 0 ||
            pCreateDecodeDevice->pConfig->Config4GroupedCoefs != 0)
        {
            ThrowFailure(E_INVALIDARG);
        }

        createArgs.Desc.DecodeProfile = *pCreateDecodeDevice->pGuid;
        createArgs.Desc.Width = pCreateDecodeDevice->VideoDesc.SampleWidth;
        createArgs.Desc.Height = pCreateDecodeDevice->VideoDesc.SampleHeight;
        createArgs.Desc.DecodeFormat = ConvertFromDDIToDXGIFORMAT(pCreateDecodeDevice->VideoDesc.Format);
        createArgs.Config.ConfigDecoderSpecific = pCreateDecodeDevice->pConfig->ConfigDecoderSpecific;
        createArgs.Config.InterlaceType = VideoTranslate::GetInterlaceType((DXVADDI_SAMPLEFORMAT)pCreateDecodeDevice->VideoDesc.SampleFormat.SampleFormat);

        m_pParentDevice->EnsureVideoDevice();

        m_pUnderlyingVideoDecode = new (m_pUnderlyingSpace) D3D12TranslationLayer::VideoDecode(&m_pParentDevice->GetContext(), createArgs);

        m_decodeProfile = createArgs.Desc.DecodeProfile;
    }

    DecodeDevice::~DecodeDevice()
    {
        if (m_pUnderlyingVideoDecode)
        {
            m_pUnderlyingVideoDecode->~VideoDecode();
        }
    }

    _Use_decl_annotations_
    void DecodeDevice::SetDecodeRenderTarget(CONST D3DDDIARG_SETDECODERENDERTARGET *pRenderTarget)
    {
        Resource *pResource = Resource::GetResourceFromHandle(pRenderTarget->hRenderTarget);

        D3D12TranslationLayer::AppResourceDesc* pAppDesc = pResource->GetUnderlyingResource()->AppDesc();
        D3D12TranslationLayer::VIDEO_DECODER_OUTPUT_VIEW_DESC_INTERNAL viewDesc = { pAppDesc->Format(), pRenderTarget->SubResourceIndex };

        const UINT8 MipLevels = pAppDesc->MipLevels();
        const UINT16 ArraySize = pAppDesc->ArraySize();
        const UINT8 PlaneCount = (pResource->GetUnderlyingResource()->SubresourceMultiplier() * pAppDesc->NonOpaquePlaneCount());

        m_outputArguments.SubresourceSubset = D3D12TranslationLayer::CViewSubresourceSubset(viewDesc, MipLevels, ArraySize, PlaneCount);
        m_outputArguments.pOutputTexture2D = pResource->GetUnderlyingResource();
        m_inUseResources.output = pResource->GetUnderlyingResource();
    }

    _Use_decl_annotations_
    HRESULT DecodeDevice::ExecuteExtension(CONST D3DDDIARG_DECODEEXTENSIONEXECUTE* pExtensionExecute)
    {
        if (pExtensionExecute->Function == DXVA_STATUS_REPORTING_FUNCTION)
        {
            return m_pUnderlyingVideoDecode->GetDecodingStatus(pExtensionExecute->pPrivateOutput->pData, pExtensionExecute->pPrivateOutput->DataSize);
        }

        return E_NOTIMPL;
    }

    _Use_decl_annotations_
    void DecodeDevice::BeginFrame(D3DDDIARG_DECODEBEGINFRAME * /*pBeginFrame*/)
    {
        if (m_frameNestCount != 0)
        {
            // we do not allow nested BeginFrames in 9on12. The post processing flag for VC1 is always off.
            ThrowFailure(E_INVALIDARG);
        }
        ++m_frameNestCount;
    }

    _Use_decl_annotations_
    void DecodeDevice::Execute(CONST D3DDDIARG_DECODEEXECUTE *pExecute)
    {
        for (DWORD i = 0; i < pExecute->NumCompBuffers; i++)
        {
            DXVADDI_DECODEBUFFERDESC *pBufferDesc = &pExecute->pCompressedBuffers[i];

            if (m_inputArguments.FrameArgumentsCount >= D3D12_VIDEO_DECODE_MAX_ARGUMENTS)
            {
                ThrowFailure(E_INVALIDARG);
            }
            __analysis_assume(m_inputArguments.FrameArgumentsCount < D3D12_VIDEO_DECODE_MAX_ARGUMENTS);

            switch (pBufferDesc->CompressedBufferType)
            {
                case D3DDDIFMT_PICTUREPARAMSDATA:
                case D3DDDIFMT_SLICECONTROLDATA:
                case D3DDDIFMT_INVERSEQUANTIZATIONDATA:
                {
                    Resource* pResource = Resource::GetResourceFromHandle(pBufferDesc->hBuffer);
                    D3D12TranslationLayer::MappedSubresource mappedSubresource;

                    m_pParentDevice->GetContext().Map(pResource->GetUnderlyingResource(), 0, D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_READ, false, nullptr, &mappedSubresource);
                    m_inputArguments.FrameArguments[m_inputArguments.FrameArgumentsCount].Type = VideoTranslate::VideoDecodeArgumentType(pBufferDesc->CompressedBufferType);
                    m_inputArguments.FrameArguments[m_inputArguments.FrameArgumentsCount].pData = mappedSubresource.pData;
                    m_inputArguments.FrameArguments[m_inputArguments.FrameArgumentsCount].Size = pBufferDesc->DataSize;
                    m_inUseResources.frameArguments[m_inputArguments.FrameArgumentsCount] = pResource->GetUnderlyingResource();
                    ++m_inputArguments.FrameArgumentsCount;
                }
                break;

                case D3DDDIFMT_BITSTREAMDATA:
                {
                    // TODO: assume it all fits for now, no bad slice chopping (see work items 5743248/5743254)
                    Resource* pResource = Resource::GetResourceFromHandle(pBufferDesc->hBuffer);
                    D3D12TranslationLayer::Resource *pTranslationResource = pResource->GetUnderlyingResource();
                    m_inputArguments.CompressedBitstream.pBuffer = pTranslationResource;
                    m_inputArguments.CompressedBitstream.Offset = pBufferDesc->DataOffset;
                    m_inputArguments.CompressedBitstream.Size = pBufferDesc->DataSize;
                    m_inUseResources.compressedBitstream = pResource->GetUnderlyingResource();
                }
                break;

            default:
                ThrowFailure(E_INVALIDARG);
                break;
            }
        }

    }

    _Use_decl_annotations_
        void DecodeDevice::EndFrame(D3DDDIARG_DECODEENDFRAME * /*pEndFrame*/)
    {
        --m_frameNestCount;

        m_pUnderlyingVideoDecode->DecodeFrame(&m_inputArguments, &m_outputArguments);

        // now unmap frame arguments
        for (UINT i = 0; i < m_inputArguments.FrameArgumentsCount; i++)
        {
            m_pParentDevice->GetContext().Unmap(m_inUseResources.frameArguments[i].get(), 0, D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_READ, nullptr);
        }
        m_inUseResources = decltype(m_inUseResources){};
        ZeroMemory(&m_inputArguments, sizeof(m_inputArguments));
        ZeroMemory(&m_outputArguments, sizeof(m_outputArguments));
    }
};
