// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"


namespace D3D9on12
{

    _Check_return_ HRESULT APIENTRY CreateVideoProcessDevice(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEVIDEOPROCESSDEVICE *pCreateVideoProcessDevice)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr  ||  pCreateVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        VideoProcessDevice *pVideoProcessDevice = new VideoProcessDevice (pCreateVideoProcessDevice->hVideoProcess, pDevice, pCreateVideoProcessDevice);
        if (pVideoProcessDevice == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        pCreateVideoProcessDevice->hVideoProcess = VideoProcessDevice::GetHandleFromVideoProcessDevice(pVideoProcessDevice);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DestroyVideoProcessDevice(_In_ HANDLE /* hDevice */, _In_ HANDLE hVideoProcessDevice)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(hVideoProcessDevice);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        delete pVideoProcessDevice;

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY VideoProcessBeginFrame(_In_ HANDLE /* hDevice */, _In_ HANDLE hVideoProcessDevice)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(hVideoProcessDevice);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY VideoProcessEndFrame(_In_ HANDLE /* hDevice */, _Inout_ D3DDDIARG_VIDEOPROCESSENDFRAME * pEndFrame)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(pEndFrame->hVideoProcess);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetVideoProcessRenderTarget(_In_ HANDLE /* hDevice */, _In_ CONST D3DDDIARG_SETVIDEOPROCESSRENDERTARGET *pRenderTarget)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(pRenderTarget->hVideoProcess);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pVideoProcessDevice->SetRenderTarget(pRenderTarget);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY VideoProcessBlit(_In_ HANDLE /* hDevice */, _In_ CONST D3DDDIARG_VIDEOPROCESSBLT *pBlit)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(pBlit->hVideoProcess);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pVideoProcessDevice->Blit(pBlit);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    /////////////////////////////////////////////////////////////////////////////
    // DXVA-HD entry points
    /////////////////////////////////////////////////////////////////////////////

    _Check_return_ HRESULT APIENTRY DXVAHD_CreateVideoProcessor(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_DXVAHD_CREATEVIDEOPROCESSOR *pCreateVideoProcessor)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pCreateVideoProcessor == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        VideoProcessDevice *pVideoProcessDevice = new VideoProcessDevice(pDevice, pCreateVideoProcessor);
        if (pVideoProcessDevice == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        pCreateVideoProcessor->hVideoProcessor = VideoProcessDevice::GetHandleFromVideoProcessDevice(pVideoProcessDevice);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DXVAHD_SetVideoProcessorBlitState(_In_ HANDLE /*hDevice*/, _In_ CONST D3DDDIARG_DXVAHD_SETVIDEOPROCESSBLTSTATE *pBltState)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(pBltState->hVideoProcessor);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pVideoProcessDevice->SetBlitState(pBltState);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DXVAHD_GetVideoProcessorBlitState(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_DXVAHD_GETVIDEOPROCESSBLTSTATEPRIVATE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_FAIL);        // no privates in DX12.  E_FAIL is required by DXVAHDVideoProcessing tests
    }

    _Check_return_ HRESULT APIENTRY DXVAHD_SetVideoProcessorStreamState(_In_ HANDLE /*hDevice*/, _In_ CONST D3DDDIARG_DXVAHD_SETVIDEOPROCESSSTREAMSTATE *pStreamState)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(pStreamState->hVideoProcessor);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pVideoProcessDevice->SetStreamState(pStreamState);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DXVAHD_GetVideoProcessorStreamState(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_DXVAHD_GETVIDEOPROCESSSTREAMSTATEPRIVATE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_UNEXPECTED);        // no privates in DX12
    }

    _Check_return_ HRESULT APIENTRY DXVAHD_VideoProcessBlitHD(_In_ HANDLE /*hDevice*/, _In_ CONST D3DDDIARG_DXVAHD_VIDEOPROCESSBLTHD* pBlitHD)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(pBlitHD->hVideoProcessor);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        pVideoProcessDevice->BlitHD(pBlitHD);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DXVAHD_DestroyVideoProcessor(_In_ HANDLE /* hDevice */, _In_ HANDLE hVideoProcessDevice)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        VideoProcessDevice *pVideoProcessDevice = VideoProcessDevice::GetVideoProcessDeviceFromHandle(hVideoProcessDevice);
        if (pVideoProcessDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        delete pVideoProcessDevice;
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // VideoProcessDevice
    ////////////////////////////////////////////////////////////////////////////////////////////

    _Use_decl_annotations_
    VideoProcessDevice::VideoProcessDevice(HANDLE /*runtimeHandle*/, Device *pParent, const D3DDDIARG_CREATEVIDEOPROCESSDEVICE *pCreateVideoProcessDevice) :
        m_pParentDevice(pParent)
    {
        m_pParentDevice->EnsureVideoDevice();
        m_MaxInputStreams = pCreateVideoProcessDevice->MaxSubStreams + 1;
        m_inputArguments.ResetStreams(m_MaxInputStreams);       // throw(bad_alloc)

        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS DeinterlaceMode = D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB;
        if (   pCreateVideoProcessDevice->pVideoProcGuid != nullptr
            && *pCreateVideoProcessDevice->pVideoProcGuid == DXVADDI_VideoProcD3D9On12CustomDeinterlaceDevice)
        {
            DeinterlaceMode |= D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM;
        }

        m_pUnderlyingVideoProcess = new (m_pUnderlyingSpace) D3D12TranslationLayer::VideoProcess(&m_pParentDevice->GetContext(), DeinterlaceMode);
    }

    _Use_decl_annotations_
    VideoProcessDevice::VideoProcessDevice(Device *pParent, const D3DDDIARG_DXVAHD_CREATEVIDEOPROCESSOR  * pCreateVideoProcessDevice) :
        m_pParentDevice(pParent)
    {
        m_pParentDevice->EnsureVideoDevice();
        VideoDevice *pVideoDevice = m_pParentDevice->GetVideoDevice();
        m_MaxInputStreams = pVideoDevice->GetMaxInputStreams();
        m_inputArguments.ResetStreams(m_MaxInputStreams);       // throw(bad_alloc)
        m_dxvaHDStreamStates.resize(m_MaxInputStreams);

        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS DeinterlaceMode = D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB;
        if (   pCreateVideoProcessDevice->pVPGuid != nullptr
            && *pCreateVideoProcessDevice->pVPGuid == DXVADDI_VideoProcD3D9On12CustomDeinterlaceDevice)
        {
            DeinterlaceMode |= D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM;
        }

        m_pUnderlyingVideoProcess = new (m_pUnderlyingSpace) D3D12TranslationLayer::VideoProcess(&m_pParentDevice->GetContext(), DeinterlaceMode);
    }

    VideoProcessDevice::~VideoProcessDevice()
    {
        if (m_pUnderlyingVideoProcess)
        {
            m_pUnderlyingVideoProcess->~VideoProcess();
        }
    }

    _Use_decl_annotations_
    void VideoProcessDevice::SetRenderTarget(CONST D3DDDIARG_SETVIDEOPROCESSRENDERTARGET *pRenderTarget)
    {
        SetViewInfo(pRenderTarget->hRenderTarget, pRenderTarget->SubResourceIndex, &m_outputArguments.CurrentFrame[0]);
        Resource *pResource = Resource::GetResourceFromHandle(pRenderTarget->hRenderTarget);
        m_inUseResources.target = pResource->GetUnderlyingResource();
    }

    _Use_decl_annotations_
    void VideoProcessDevice::SetViewInfo(HANDLE hResource, UINT subresourceIndex, D3D12TranslationLayer::VideoProcessView *pView)
    {
        Resource *pResource = Resource::GetResourceFromHandle(hResource);
        pView->pResource = pResource->GetUnderlyingResource();

        D3D12TranslationLayer::AppResourceDesc* pAppDesc = pResource->GetUnderlyingResource()->AppDesc();

        // Method used for both input and output views, but VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL.
        D3D12TranslationLayer::VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL viewDesc = { pAppDesc->Format(), 0, subresourceIndex }; 

        const UINT8 MipLevels = pAppDesc->MipLevels();
        const UINT16 ArraySize = pAppDesc->ArraySize();
        const UINT8 PlaneCount = (pResource->GetUnderlyingResource()->SubresourceMultiplier() * pAppDesc->NonOpaquePlaneCount());

        pView->SubresourceSubset = D3D12TranslationLayer::CViewSubresourceSubset(viewDesc, MipLevels, ArraySize, PlaneCount);
    }

    _Use_decl_annotations_
    void VideoProcessDevice::SetFilter(D3D12_VIDEO_PROCESS_FILTER filter, D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pStreamDesc, D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1 *pStreamArguments, INT level)
    {
        pStreamDesc->FilterFlags |= (D3D12_VIDEO_PROCESS_FILTER_FLAGS)(1 << (UINT)filter);
        
        // Conversion between DXVA and D3D12 Multiplier/StepSize semantics.
        // Need to re-normalize the DXVA range expressed as an absolute range with the fractional step to the D3D12 semantics as an expanded scaled range with unitary integer steps
            // DXVA semantics
            // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dumddi/ns-d3dumddi-_dxvaddi_valuerange
            // The range is expressed in absolute terms in Minimum, Maximum, Default. The allowed precision step between that range is defined in StepSize.
            // For example, a hue filter might have an actual range of [–180.0 ... +180.0] with a step size of 0.25. 
            // The device would report the following range and multiplier:
            // Minimum: –180
            // Maximum : +180
            // StepSize : 0.25
            // 
            // D3D12 semantics
            // https://docs.microsoft.com/zh-cn/windows-hardware/drivers/ddi/d3d12umddi/ns-d3d12umddi-d3d12ddi_video_process_filter_range_0020
            // The multiplier enables the filter range to have a fractional step value. For example, a hue filter might have an actual range of [–180.0 ... +180.0] with a step size of 0.25. 
            // The device would report the following range and multiplier:
            // Minimum: –720
            // Maximum : +720
            // Multiplier : 0.25
            // In this case, a filter value of 2 would be interpreted by the device as 0.50, which is 2 × 0.25.
            // The device should use a multiplier that can be represented exactly as a base - 2 fraction.

        FLOAT rangesMultiplier = m_pParentDevice->GetVideoDevice()->GetFilterRangeMultiplier(filter);
        pStreamArguments->FilterLevels[filter] = static_cast<INT>(floor(level / rangesMultiplier));
    }

    _Use_decl_annotations_
    void VideoProcessDevice::Blit(CONST D3DDDIARG_VIDEOPROCESSBLT *pBlit)
    {
        assert(m_outputArguments.CurrentFrame[0].pResource);

        // output target rectangle
        static_assert(sizeof(m_outputArguments.D3D12OutputStreamArguments.TargetRectangle) == sizeof(pBlit->TargetRect), "Rects should match");
        if (pBlit->DestFlags.TargetRectChanged)
        {
            m_outputArguments.D3D12OutputStreamArguments.TargetRectangle = *reinterpret_cast<const D3D12_RECT*>(&pBlit->TargetRect);
            m_outputArguments.EnableTargetRect = true;
        }

        // output background color + output alpha fill mode
        if (pBlit->DestFlags.BackgroundChanged)
        {
            VideoTranslate::VideoColor(&pBlit->BackgroundColor, m_outputArguments.D3D12OutputStreamDesc.BackgroundColor);
            m_outputArguments.BackgroundColorYCbCr = true;      // bg color is always specified as YUV color, according to MSDN
            m_outputArguments.BackgroundColorSet = true;
        }

        if (pBlit->DestFlags.AlphaChanged)
        {
            const DXVA2_Fixed32 opaqueAlpha = DXVA2_Fixed32OpaqueAlpha();
            if (pBlit->Alpha.ll == opaqueAlpha.ll)
            {
                m_outputArguments.D3D12OutputStreamDesc.AlphaFillMode = D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_OPAQUE;
            }
            else
            {
                // unclear what the interaction between the Alpha & the Alpha in the background color are in this case:
                // https://msdn.microsoft.com/en-us/library/windows/desktop/hh447648(v=vs.85).aspx
                // We assume that the explicit alpha overrides whatever was set in the alpha for the BG color above.
                m_outputArguments.D3D12OutputStreamDesc.AlphaFillMode = D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_BACKGROUND;
                float alpha = VideoTranslate::FromFixed32<float>(pBlit->Alpha);
                m_outputArguments.D3D12OutputStreamDesc.BackgroundColor[3] = alpha;
            }
        }

        // output constriction: can't allow size != target && size != 0
        if (((UINT)pBlit->ConstrictionSize.cx != RectWidth(pBlit->TargetRect) || (UINT)pBlit->ConstrictionSize.cy != RectHeight(pBlit->TargetRect)) && 
             (pBlit->ConstrictionSize.cx != 0 || pBlit->ConstrictionSize.cy != 0))
        {
            ThrowFailure(E_INVALIDARG);        // constriction not allowed in DX12
        }

        // output stereo
        m_outputArguments.D3D12OutputStreamDesc.EnableStereo = FALSE;

        // output color space
        // Note that dxva2 does not set the color data changed if all DestFlags are 0s. Unfortunately, we still need to map the 0s (DXVA2_NominalRange_Unknown,  
        // DXVA2_VideoTransferMatrix_Unknown and DXVA2_VideoTransFunc_Unknown) to a valid DXGI value.
        if (pBlit->DestFlags.ColorDataChanged  ||  m_colorSpaceNeverSet)
        {
            ID3D12Resource *pTexture = m_outputArguments.CurrentFrame[0].pResource->GetUnderlyingResource();
            const D3D12_RESOURCE_DESC &desc = pTexture->GetDesc();
            m_outputArguments.D3D12OutputStreamDesc.ColorSpace = VideoTranslate::ColorSpaceType(desc.Format, desc.Height, pBlit->DestFormat);
            m_outputArguments.ColorSpaceSet = true;
        }

        // The DXVA2 runtime method CVideoProcessorDevice::VideoProcessBlt() method doesn't set DXVA2_SampleFlag_PlanarAlpha_Changed
        // if PlanarAlpha is not != than the previous method call's PlanarAlpha value. Since the initial value of PlanarAlpha is zero
        // and DXVA2_Fixed32TransparentAlpha is also 0, when setting alpha transparent for the first process call,
        // the flag never gets triggered even when we want to blend 
        // m_outputArguments.D3D12OutputStreamDesc.AlphaFillMode = D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_OPAQUE in the composite rect
        // with transparent for the sample rect in the first video process call.
        // Let's keep track of the first initialization of each stream in std::vector<bool> m_IsStreamPlanarAlphaBlendInitialized
        if (m_IsStreamPlanarAlphaBlendInitialized.size() < pBlit->NumSrcSurfaces)
        {
            m_IsStreamPlanarAlphaBlendInitialized.resize(pBlit->NumSrcSurfaces, false);
        }

        // input streams
        for (UINT streamIndex = 0; streamIndex < pBlit->NumSrcSurfaces; streamIndex++)
        {
            DXVADDI_VIDEOSAMPLE *pSource = &pBlit->pSrcSurfaces[streamIndex];
            auto *pStreamInfo = &m_inputArguments.StreamInfo[streamIndex];
            D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1 *pStreamArguments = &m_inputArguments.D3D12InputStreamArguments[streamIndex];
            D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pStreamDesc = &m_inputArguments.D3D12InputStreamDesc[streamIndex];

            // stream resource
            SetViewInfo(pSource->SrcResource, pSource->SrcSubResourceIndex, &pStreamInfo->ResourceSet[0].CurrentFrame);

            // stream src and dst rects
            static_assert(sizeof(D3D12_RECT) == sizeof(RECT), "Rects should match");
            if (pSource->SampleFlags.SrcRectChanged)
            {
                pStreamArguments->Transform.SourceRectangle = *reinterpret_cast<const D3D12_RECT*>(&pSource->SrcRect);
                D3D12_VIDEO_SIZE_RANGE& SourceSizeRange = pStreamDesc->SourceSizeRange;
                SourceSizeRange.MaxWidth = static_cast<UINT>(pSource->SrcRect.right - pSource->SrcRect.left);
                SourceSizeRange.MinWidth = SourceSizeRange.MaxWidth;
                SourceSizeRange.MaxHeight = static_cast<UINT>(pSource->SrcRect.bottom - pSource->SrcRect.top);
                SourceSizeRange.MinHeight = SourceSizeRange.MaxHeight;
                pStreamInfo->EnableSourceRect = TRUE;
            }
            if (pSource->SampleFlags.DstRectChanged)
            {
                pStreamArguments->Transform.DestinationRectangle = *reinterpret_cast<const D3D12_RECT*>(&pSource->DstRect);
                D3D12_VIDEO_SIZE_RANGE& DestinationSizeRange = pStreamDesc->DestinationSizeRange;
                DestinationSizeRange.MaxWidth = static_cast<UINT>(pSource->DstRect.right - pSource->DstRect.left);
                DestinationSizeRange.MinWidth = DestinationSizeRange.MaxWidth;
                DestinationSizeRange.MaxHeight = static_cast<UINT>(pSource->DstRect.bottom - pSource->DstRect.top);
                DestinationSizeRange.MinHeight = DestinationSizeRange.MaxHeight;
                pStreamInfo->EnableDestinationRect = TRUE;
            }

            // stream frame format
            pStreamArguments->FieldType = VideoTranslate::VideoFieldType(pSource->SampleFormat.SampleFormat);

            // stream alpha 
            if ((pSource->SampleFlags.PlanarAlphaChanged || !m_IsStreamPlanarAlphaBlendInitialized[streamIndex]) && m_pParentDevice->GetVideoDevice()->IsAlphaBlendProcessingSupported())
            {
                const DXVA2_Fixed32 opaqueAlpha = DXVA2_Fixed32OpaqueAlpha();
                if (pSource->PlanarAlpha.ll != opaqueAlpha.ll)
                {
                    pStreamArguments->AlphaBlending.Alpha = VideoTranslate::FromFixed32<float>(pSource->PlanarAlpha);
                    pStreamArguments->AlphaBlending.Enable = TRUE;
                    pStreamDesc->EnableAlphaBlending = TRUE;
                }
                m_IsStreamPlanarAlphaBlendInitialized[streamIndex] = true;
            }

            // input color space
            if (pSource->SampleFlags.ColorDataChanged  ||  m_colorSpaceNeverSet)
            {
                ID3D12Resource *pSourceTexture = pStreamInfo->ResourceSet[0].CurrentFrame.pResource->GetUnderlyingResource();
                const D3D12_RESOURCE_DESC &sourceDesc = pSourceTexture->GetDesc();
                pStreamDesc->ColorSpace = VideoTranslate::ColorSpaceType(sourceDesc.Format, sourceDesc.Height, pSource->SampleFormat);
                pStreamInfo->ColorSpaceSet = TRUE;
            }
            m_colorSpaceNeverSet = false;

            // DXVA doesn't have any way to enable/disable autoprocessing. We will enable it automatically in 12 if autoprocessing is supported in the 12 driver.
            pStreamDesc->EnableAutoProcessing = m_pParentDevice->GetVideoDevice()->IsAutoProcessingSupported();

            // stream filters
            // TODO: note that DX12 just has proc amp control on the input streams, and DXVA has on the destination. The
            // workaround here is to set in every input, which is not correct in many cases (it is the same when there is 1 stream covering the whole output). 
            // Should we ignore instead? Should we modify DX12?
            SetFilter(D3D12_VIDEO_PROCESS_FILTER_BRIGHTNESS, pStreamDesc, pStreamArguments, VideoTranslate::FromFixed32<INT>(pBlit->ProcAmpValues.Brightness));
            SetFilter(D3D12_VIDEO_PROCESS_FILTER_CONTRAST, pStreamDesc, pStreamArguments, VideoTranslate::FromFixed32<INT>(pBlit->ProcAmpValues.Contrast));
            SetFilter(D3D12_VIDEO_PROCESS_FILTER_HUE, pStreamDesc, pStreamArguments, VideoTranslate::FromFixed32<INT>(pBlit->ProcAmpValues.Hue));
            SetFilter(D3D12_VIDEO_PROCESS_FILTER_SATURATION, pStreamDesc, pStreamArguments, VideoTranslate::FromFixed32<INT>(pBlit->ProcAmpValues.Saturation));
        }

        m_pUnderlyingVideoProcess->ProcessFrames(&m_inputArguments, pBlit->NumSrcSurfaces, &m_outputArguments);
        m_inUseResources = decltype(m_inUseResources){};
    }

    _Use_decl_annotations_
    void VideoProcessDevice::SetBlitState(CONST D3DDDIARG_DXVAHD_SETVIDEOPROCESSBLTSTATE *pBltState)
    {
        switch (pBltState->State)
        {
        case DXVAHDDDI_BLT_STATE_TARGET_RECT:
        {
            assert(pBltState->DataSize == sizeof(DXVAHDDDI_BLT_STATE_TARGET_RECT_DATA));
            const DXVAHDDDI_BLT_STATE_TARGET_RECT_DATA *pRectData = reinterpret_cast<const DXVAHDDDI_BLT_STATE_TARGET_RECT_DATA *>(pBltState->pData);
            m_outputArguments.D3D12OutputStreamArguments.TargetRectangle = pRectData->TargetRect;
            m_outputArguments.EnableTargetRect = pRectData->Enable;
        }
        break;

        case DXVAHDDDI_BLT_STATE_BACKGROUND_COLOR:
        {
            assert(pBltState->DataSize == sizeof(DXVAHDDDI_BLT_STATE_BACKGROUND_COLOR_DATA));
            const DXVAHDDDI_BLT_STATE_BACKGROUND_COLOR_DATA *pColorData = reinterpret_cast<const DXVAHDDDI_BLT_STATE_BACKGROUND_COLOR_DATA *>(pBltState->pData);
            VideoTranslate::VideoColor(&pColorData->BackgroundColor, m_outputArguments.D3D12OutputStreamDesc.BackgroundColor);
            m_outputArguments.BackgroundColorYCbCr = pColorData->YCbCr;
            m_outputArguments.BackgroundColorSet = true;
        }
        break;

        case DXVAHDDDI_BLT_STATE_OUTPUT_COLOR_SPACE:
        {
            assert(pBltState->DataSize == sizeof(DXVAHDDDI_BLT_STATE_OUTPUT_COLOR_SPACE_DATA));
            const DXVAHDDDI_BLT_STATE_OUTPUT_COLOR_SPACE_DATA *pColorSpaceData = reinterpret_cast<const DXVAHDDDI_BLT_STATE_OUTPUT_COLOR_SPACE_DATA *>(pBltState->pData);
            m_dxvaHDColorSpaceData = *pColorSpaceData;
            m_outputArguments.ColorSpaceSet = true;
        }
        break;

        case DXVAHDDDI_BLT_STATE_ALPHA_FILL:
        {
            assert(pBltState->DataSize == sizeof(DXVAHDDDI_BLT_STATE_ALPHA_FILL_DATA));
            const DXVAHDDDI_BLT_STATE_ALPHA_FILL_DATA *pAlphaFillData = reinterpret_cast<const DXVAHDDDI_BLT_STATE_ALPHA_FILL_DATA *>(pBltState->pData);
            switch (pAlphaFillData->Mode)
            {
            case DXVAHDDDI_ALPHA_FILL_MODE_OPAQUE:
                m_outputArguments.D3D12OutputStreamDesc.AlphaFillMode = D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_OPAQUE;
                break;
            case DXVAHDDDI_ALPHA_FILL_MODE_BACKGROUND:
                m_outputArguments.D3D12OutputStreamDesc.AlphaFillMode = D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_BACKGROUND;
                break;
            case DXVAHDDDI_ALPHA_FILL_MODE_DESTINATION:
                m_outputArguments.D3D12OutputStreamDesc.AlphaFillMode = D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_DESTINATION;
                break;
            case DXVAHDDDI_ALPHA_FILL_MODE_SOURCE_STREAM:
                m_outputArguments.D3D12OutputStreamDesc.AlphaFillMode = D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_SOURCE_STREAM;
                m_outputArguments.D3D12OutputStreamDesc.AlphaFillModeSourceStreamIndex = pAlphaFillData->StreamNumber;
                break;
            default:
                ThrowFailure(E_UNEXPECTED);
                break;
            }
        }
        break;

        case DXVAHDDDI_BLT_STATE_PRIVATE:
            ThrowFailure(E_UNEXPECTED);            // no privates in DX12.
            break;

        case DXVAHDDDI_BLT_STATE_CONSTRICTION:
        {
            assert(pBltState->DataSize == sizeof(DXVAHDDDI_BLT_STATE_CONSTRICTION_DATA));
            const DXVAHDDDI_BLT_STATE_CONSTRICTION_DATA *pConstrictionData = reinterpret_cast<const DXVAHDDDI_BLT_STATE_CONSTRICTION_DATA *>(pBltState->pData);
            if (pConstrictionData->Enable  &&  (pConstrictionData->Size.cx != 0 || pConstrictionData->Size.cy != 0))
            {
                ThrowFailure(E_INVALIDARG);        // constriction not allowed in DX12
            }
        }
        break;

        default:
            ThrowFailure(E_UNEXPECTED);
            break;
        }
    }

    _Use_decl_annotations_
    void VideoProcessDevice::SetStreamState(CONST D3DDDIARG_DXVAHD_SETVIDEOPROCESSSTREAMSTATE *pStreamState)
    {
        assert(pStreamState->StreamNumber < m_MaxInputStreams);
        if (pStreamState->State == DXVAHDDDI_STREAM_STATE_PRIVATE)
        {
            ThrowFailure(E_FAIL); // no privates in DX12.  E_FAIL is required by DXVAHDVideoProcessing tests
        }

        std::vector<BYTE>& streamStateData = m_dxvaHDStreamStates[pStreamState->StreamNumber][pStreamState->State];
        streamStateData.resize(pStreamState->DataSize);
        memcpy(streamStateData.data(), pStreamState->pData, pStreamState->DataSize);
    }


    _Use_decl_annotations_
    void VideoProcessDevice::SetStreamState(UINT streamIndex, D3D12TranslationLayer::VideoProcessView *pStreamView, DXVAHDDDI_STREAM_STATE state, const std::vector<BYTE> &streamStateData)
    {
        D3D12TranslationLayer::VIDEO_PROCESS_STREAM_INFO *pStreamInfo = &m_inputArguments.StreamInfo[streamIndex];
        D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1 *pStreamArguments = &m_inputArguments.D3D12InputStreamArguments[streamIndex];
        D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pStreamDesc = &m_inputArguments.D3D12InputStreamDesc[streamIndex];

        switch (state)
        {
        case DXVAHDDDI_STREAM_STATE_FRAME_FORMAT:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_FRAME_FORMAT_DATA));
            const DXVAHDDDI_STREAM_STATE_FRAME_FORMAT_DATA *pData = reinterpret_cast<const DXVAHDDDI_STREAM_STATE_FRAME_FORMAT_DATA *>(streamStateData.data());
            pStreamArguments->FieldType = VideoTranslate::VideoFieldType(pData->FrameFormat);
        }
        break;

        case DXVAHDDDI_STREAM_STATE_INPUT_COLOR_SPACE:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_INPUT_COLOR_SPACE_DATA));
            const DXVAHDDDI_STREAM_STATE_INPUT_COLOR_SPACE_DATA *pData = reinterpret_cast<const DXVAHDDDI_STREAM_STATE_INPUT_COLOR_SPACE_DATA *>(streamStateData.data());
            const D3D12_RESOURCE_DESC &desc = pStreamView->pResource->GetUnderlyingResource()->GetDesc();
            pStreamDesc->ColorSpace = VideoTranslate::ColorSpaceType<DXVAHDDDI_STREAM_STATE_INPUT_COLOR_SPACE_DATA>(desc.Format, *pData);
            pStreamInfo->ColorSpaceSet = TRUE;
        }
        break;

        case DXVAHDDDI_STREAM_STATE_OUTPUT_RATE:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_OUTPUT_RATE_DATA));
            // ignore OutputIndex/Half/Custom parameters
        }
        break;

        case DXVAHDDDI_STREAM_STATE_SOURCE_RECT:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_SOURCE_RECT_DATA));
            const DXVAHDDDI_STREAM_STATE_SOURCE_RECT_DATA *pData = reinterpret_cast<const DXVAHDDDI_STREAM_STATE_SOURCE_RECT_DATA *>(streamStateData.data());
            D3D12_VIDEO_SIZE_RANGE& SourceSizeRange = pStreamDesc->SourceSizeRange;
            D3D12_RECT& SourceRectangle = pStreamArguments->Transform.SourceRectangle;
            SourceRectangle = *reinterpret_cast<const D3D12_RECT*>(&pData->SourceRect);
            SourceSizeRange.MaxWidth = static_cast<UINT>(SourceRectangle.right - SourceRectangle.left);
            SourceSizeRange.MinWidth = SourceSizeRange.MaxWidth;
            SourceSizeRange.MaxHeight = static_cast<UINT>(SourceRectangle.bottom - SourceRectangle.top);
            SourceSizeRange.MinHeight = SourceSizeRange.MaxHeight;
            pStreamInfo->EnableSourceRect = pData->Enable;
        }
        break;

        case DXVAHDDDI_STREAM_STATE_DESTINATION_RECT:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_DESTINATION_RECT_DATA));
            const DXVAHDDDI_STREAM_STATE_DESTINATION_RECT_DATA *pData = reinterpret_cast<const DXVAHDDDI_STREAM_STATE_DESTINATION_RECT_DATA *>(streamStateData.data());
            D3D12_VIDEO_SIZE_RANGE& DestinationSizeRange = pStreamDesc->DestinationSizeRange;
            D3D12_RECT& DestinationRectangle = pStreamArguments->Transform.DestinationRectangle;
            DestinationRectangle = *reinterpret_cast<const D3D12_RECT*>(&pData->DestinationRect);
            DestinationSizeRange.MaxWidth = static_cast<UINT>(DestinationRectangle.right - DestinationRectangle.left);
            DestinationSizeRange.MinWidth = DestinationSizeRange.MaxWidth;
            DestinationSizeRange.MaxHeight = static_cast<UINT>(DestinationRectangle.bottom - DestinationRectangle.top);
            DestinationSizeRange.MinHeight = DestinationSizeRange.MaxHeight;
            pStreamInfo->EnableDestinationRect = pData->Enable;
        }
        break;

        case DXVAHDDDI_STREAM_STATE_ALPHA:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_ALPHA_DATA));
            const DXVAHDDDI_STREAM_STATE_ALPHA_DATA *pData = reinterpret_cast<const DXVAHDDDI_STREAM_STATE_ALPHA_DATA *>(streamStateData.data());
            pStreamArguments->AlphaBlending.Alpha = pData->Alpha;
            pStreamArguments->AlphaBlending.Enable = pData->Enable;
            pStreamDesc->EnableAlphaBlending = pData->Enable;
        }
        break;

        case DXVAHDDDI_STREAM_STATE_LUMA_KEY:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_LUMA_KEY_DATA));
            const DXVAHDDDI_STREAM_STATE_LUMA_KEY_DATA *pData = reinterpret_cast<const DXVAHDDDI_STREAM_STATE_LUMA_KEY_DATA *>(streamStateData.data());
            pStreamDesc->LumaKey.Lower = pData->Lower;
            pStreamDesc->LumaKey.Upper = pData->Upper;
            pStreamDesc->LumaKey.Enable = pData->Enable;
        }
        break;

        case DXVAHDDDI_STREAM_STATE_ASPECT_RATIO:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_ASPECT_RATIO_DATA));
            const DXVAHDDDI_STREAM_STATE_ASPECT_RATIO_DATA *pData = reinterpret_cast<const DXVAHDDDI_STREAM_STATE_ASPECT_RATIO_DATA *>(streamStateData.data());
            if (pData->Enable)
            {
                pStreamDesc->SourceAspectRatio.Numerator = pData->SourceAspectRatio.Numerator;
                pStreamDesc->SourceAspectRatio.Denominator = pData->SourceAspectRatio.Denominator;
                pStreamDesc->DestinationAspectRatio.Numerator = pData->DestinationAspectRatio.Numerator;
                pStreamDesc->DestinationAspectRatio.Denominator = pData->DestinationAspectRatio.Denominator;
            }
            else
            {
                pStreamDesc->SourceAspectRatio.Numerator = 1;
                pStreamDesc->SourceAspectRatio.Denominator = 1;
                pStreamDesc->DestinationAspectRatio.Numerator = 1;
                pStreamDesc->DestinationAspectRatio.Denominator = 1;
            }
        }
        break;

        case DXVAHDDDI_STREAM_STATE_FILTER_BRIGHTNESS:
        case DXVAHDDDI_STREAM_STATE_FILTER_CONTRAST:
        case DXVAHDDDI_STREAM_STATE_FILTER_HUE:
        case DXVAHDDDI_STREAM_STATE_FILTER_SATURATION:
        case DXVAHDDDI_STREAM_STATE_FILTER_NOISE_REDUCTION:
        case DXVAHDDDI_STREAM_STATE_FILTER_EDGE_ENHANCEMENT:
        case DXVAHDDDI_STREAM_STATE_FILTER_ANAMORPHIC_SCALING:
        {
            assert(streamStateData.size() == sizeof(DXVAHDDDI_STREAM_STATE_FILTER_DATA));
            const DXVAHDDDI_STREAM_STATE_FILTER_DATA *pData = reinterpret_cast<const DXVAHDDDI_STREAM_STATE_FILTER_DATA *>(streamStateData.data());

            D3D12_VIDEO_PROCESS_FILTER_FLAGS Filter12Flag = VideoTranslate::VideoProcessFilterFlag(state);
            if (pData->Enable)
            {
                D3D12_VIDEO_PROCESS_FILTER Filter12 = VideoTranslate::VideoProcessFilter(state);
                pStreamDesc->FilterFlags |= Filter12Flag;
                pStreamArguments->FilterLevels[Filter12] = pData->Level;
            }
            else
            {
                pStreamDesc->FilterFlags &= ~Filter12Flag;
            }
        }
        break;

        case DXVAHDDDI_STREAM_STATE_PALETTE:
            // ignore
            break;

        default:
            ThrowFailure(E_UNEXPECTED);
            break;
        }
    }

    _Use_decl_annotations_
    void VideoProcessDevice::FillReferenceSet(D3D12TranslationLayer::VIDEO_PROCESS_STREAM_INFO *pStreamInfo, UINT view, const DXVAHDDDI_SURFACE &InputSurface, UINT numPastFrames, const DXVAHDDDI_SURFACE *pPastSurfaces, UINT numFutureFrames, const DXVAHDDDI_SURFACE* pFutureSurfaces)
    {
        SetViewInfo(InputSurface.hResource, InputSurface.SubResourceIndex, &pStreamInfo->ResourceSet[view].CurrentFrame);
        if (numPastFrames)
        {
            pStreamInfo->ResourceSet[view].PastFrames.resize(numPastFrames);
            for (DWORD i = 0; i < numPastFrames; i++)
            {
                SetViewInfo(pPastSurfaces[i].hResource, pPastSurfaces[i].SubResourceIndex, &pStreamInfo->ResourceSet[view].PastFrames[i]);
            }
        }

        if (numFutureFrames)
        {
            pStreamInfo->ResourceSet[view].FutureFrames.resize(numFutureFrames);
            for (DWORD i = 0; i < numFutureFrames; i++)
            {
                SetViewInfo(pFutureSurfaces[i].hResource, pFutureSurfaces[i].SubResourceIndex, &pStreamInfo->ResourceSet[view].FutureFrames[i]);
            }
        }
    }

    _Use_decl_annotations_
    void VideoProcessDevice::BlitHD(CONST D3DDDIARG_DXVAHD_VIDEOPROCESSBLTHD *pBlit)
    {
        SetViewInfo(pBlit->OutputSurface.hResource, pBlit->OutputSurface.SubResourceIndex, &m_outputArguments.CurrentFrame[0]);

        if (m_outputArguments.ColorSpaceSet)
        {
            ID3D12Resource *pTexture = m_outputArguments.CurrentFrame[0].pResource->GetUnderlyingResource();
            const D3D12_RESOURCE_DESC &desc = pTexture->GetDesc();
            m_outputArguments.D3D12OutputStreamDesc.ColorSpace = VideoTranslate::ColorSpaceType<DXVAHDDDI_BLT_STATE_OUTPUT_COLOR_SPACE_DATA>(desc.Format, m_dxvaHDColorSpaceData);
        }

        UINT curStream = 0;
        for (UINT streamIndex = 0; streamIndex < pBlit->StreamCount; streamIndex++)
        {
            CONST DXVAHDDDI_STREAM_DATA *pStreamData = &pBlit->pStreams[streamIndex];
            D3D12TranslationLayer::VIDEO_PROCESS_STREAM_INFO *pStreamInfo = &m_inputArguments.StreamInfo[curStream];
            D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pStreamDesc = &m_inputArguments.D3D12InputStreamDesc[streamIndex];

            // stream resource
            if (pStreamData->Enable)
            {
                FillReferenceSet(pStreamInfo, 0, pStreamData->InputSurface, pStreamData->PastFrames, pStreamData->pPastSurfaces, pStreamData->FutureFrames, pStreamData->pFutureSurfaces);

                for (const auto &iter : m_dxvaHDStreamStates[streamIndex])
                {
                    SetStreamState(curStream, &pStreamInfo->ResourceSet[0].CurrentFrame, iter.first, iter.second);
                }
                pStreamInfo->OutputIndex = pStreamData->OutputIndex;
                pStreamInfo->InputFrameOrField = pStreamData->InputFrameOrField;
                ++curStream;
            }

            // DXVA-HD doesn't have any way to enable/disable autoprocessing. We will enable it automatically in 12 if autoprocessing is supported in the 12 driver.
            pStreamDesc->EnableAutoProcessing = m_pParentDevice->GetVideoDevice()->IsAutoProcessingSupported();
        }

        if (curStream == 0)
        {
            ThrowFailure(E_INVALIDARG);
        }

        m_pUnderlyingVideoProcess->ProcessFrames(&m_inputArguments, curStream, &m_outputArguments);
    }
};
