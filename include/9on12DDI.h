// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    void APIENTRY GetPrivateDDITable(D3D9ON12_PRIVATE_DDI_TABLE *pPrivateDDITable);

    /* Private DDI for 9on12 */
    _Check_return_ HRESULT APIENTRY OpenAdapter_Private(_Inout_ D3DDDIARG_OPENADAPTER* pOpenAdapter, _In_ LUID *pLUID, _In_opt_ D3D9ON12_CREATE_DEVICE_ARGS* pArgs);

    _Check_return_ HRESULT APIENTRY GetSharedGDIHandle_Private(_In_ HANDLE hDevice, HANDLE DriverResource, HANDLE *pSharedHandle);
    _Check_return_ HRESULT APIENTRY CreateSharedNTHandle_Private(_In_ HANDLE hDevice, HANDLE DriverResource, SECURITY_DESCRIPTOR *pSD, HANDLE *pSharedHandle);
    _Check_return_ HRESULT APIENTRY GetDeviceExecutionState_Private(_In_ HANDLE hDevice);
    _Check_return_ HRESULT APIENTRY KmdPresent(_In_ HANDLE hDevice, D3DKMT_PRESENT *pKMTArgs);

    _Check_return_ UINT APIENTRY QueryResourcePrivateDriverDataSize(HANDLE hDD);
    _Check_return_ HRESULT APIENTRY OpenResource_Private(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_OPENRESOURCE* hOpenResource, _In_ _D3D9ON12_OPEN_RESOURCE_ARGS *pD3D9on12OpenResource);

    _Check_return_ HRESULT APIENTRY CreateKMTHandle(HANDLE hDD, _In_ HANDLE resourceHandle, _Out_ D3DKMT_HANDLE *pHKMResource, _Out_writes_bytes_(dataSize) void *pResourcePrivateDriverData, _In_ UINT dataSize);
    _Check_return_ HRESULT APIENTRY QueryResourceInfoFromKMTHandle(HANDLE hDD, D3DKMT_HANDLE hKMResource, _Out_ D3D9ON12_RESOURCE_INFO *pResourceInfo);
    _Check_return_ HRESULT APIENTRY DestroyKMTHandle(HANDLE hDD, D3DKMT_HANDLE hKMResource);

    _Check_return_ HRESULT APIENTRY CreateFence(HANDLE hDD, UINT64 InitialValue, UINT Flags, __out HANDLE* phFence);
    _Check_return_ HRESULT APIENTRY OpenFence(HANDLE hDD, HANDLE hSharedHandle, __out BOOL* pbMonitored, __out HANDLE* phFence);
    _Check_return_ HRESULT APIENTRY ShareFence(HANDLE hFence, __in_opt SECURITY_DESCRIPTOR*, __out HANDLE*);
    _Check_return_ HRESULT APIENTRY WaitForFence(HANDLE hFence, UINT64 NewFenceValue);
    _Check_return_ HRESULT APIENTRY SignalFence(HANDLE hFence, UINT64 NewFenceValue);
    void APIENTRY DestroyTrackedFence(HANDLE hFence);
    UINT64 APIENTRY GetCompletedFenceValue(HANDLE hFence);

    _Check_return_ HRESULT APIENTRY CreateResourceWrappingHandle(HANDLE hDD, _In_ IUnknown* pD3D12Resource, _Out_ HANDLE* phWrappingHandle);
    void APIENTRY DestroyResourceWrappingHandle(HANDLE hDD, HANDLE hWrappingHandle);
    _Check_return_ HRESULT APIENTRY GetD3D12Device(HANDLE hDD, REFIID riid, void** ppv);
    _Check_return_ HRESULT APIENTRY GetD3D12Resource(HANDLE hResource, REFIID riid, void** ppv);
    _Check_return_ HRESULT APIENTRY AddResourceWaitsToQueue(HANDLE hResource, ID3D12CommandQueue* pCommmandQueue);
    _Check_return_ HRESULT APIENTRY AddDeferredWaitsToResource(HANDLE hResource, UINT NumSync, UINT64* pSignalValues, ID3D12Fence** ppFences);
    void APIENTRY SetCurrentResourceState(HANDLE hDD, HANDLE hResource, UINT State);
    void APIENTRY TransitionResource(HANDLE hDD, HANDLE hResource, UINT State);

    void APIENTRY SetMaximumFrameLatency(HANDLE hDD, UINT MaxFrameLatency);
    BOOL APIENTRY IsMaximumFrameLatencyReached(HANDLE hDD);
    
    /* Adapter forward declarations */
    _Check_return_ HRESULT APIENTRY GetCaps(_In_ HANDLE hAdapter, _Inout_ CONST D3DDDIARG_GETCAPS* pGetCaps);
    _Check_return_ HRESULT APIENTRY CreateDevice(_In_ HANDLE hAdapter, _Inout_ D3DDDIARG_CREATEDEVICE* pCreateDevice);
    _Check_return_ HRESULT APIENTRY CloseAdapter(_In_ HANDLE hAdapter);
    _Check_return_ HRESULT APIENTRY OpenAdapter(_Inout_ D3DDDIARG_OPENADAPTER* pOpenAdapter);

    /* Device forward declarations */
    _Check_return_ HRESULT APIENTRY SetRenderState(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_RENDERSTATE*);
    _Check_return_ HRESULT APIENTRY UpdateWindowInfo(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_WINFO*);
    _Check_return_ HRESULT APIENTRY ValidateDevice(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_VALIDATETEXTURESTAGESTATE*);
    _Check_return_ HRESULT APIENTRY SetTextureStageState(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_TEXTURESTAGESTATE*);
    _Check_return_ HRESULT APIENTRY SetTexture(_In_ HANDLE hDevice, _In_ UINT, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY SetPixelShader(_In_ HANDLE hDevice, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY SetPixleShaderConstant(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPIXELSHADERCONST*, _In_ CONST FLOAT*);
    _Check_return_ HRESULT APIENTRY SetStreamSourceUM(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETSTREAMSOURCEUM*, _In_ CONST VOID*);
    _Check_return_ HRESULT APIENTRY SetIndices(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETINDICES*);
    _Check_return_ HRESULT APIENTRY SetIndicesUM(_In_ HANDLE hDevice, _In_ UINT, _In_ CONST VOID*);    
    _Check_return_ HRESULT APIENTRY DrawPrimitive(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWPRIMITIVE*, _In_opt_ CONST UINT*);
    _Check_return_ HRESULT APIENTRY DrawIndexedPrimitive(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWINDEXEDPRIMITIVE*);
    _Check_return_ HRESULT APIENTRY DrawRectPatch(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWRECTPATCH*, _In_ CONST D3DDDIRECTPATCH_INFO*, _In_ CONST FLOAT*);
    _Check_return_ HRESULT APIENTRY DrawTriPatch(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWTRIPATCH*, _In_ CONST D3DDDITRIPATCH_INFO*, _In_ CONST FLOAT*);
    _Check_return_ HRESULT APIENTRY DrawPrimitive2(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWPRIMITIVE2*);
    _Check_return_ HRESULT APIENTRY DrawIndexedPrimitive2(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWINDEXEDPRIMITIVE2*, _In_ UINT, _In_ CONST VOID*, _In_opt_ CONST UINT*);
    _Check_return_ HRESULT APIENTRY SetState(_In_ HANDLE hDevice, _In_ D3DDDIARG_STATESET*);
    _Check_return_ HRESULT APIENTRY SetPriority(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPRIORITY*);
    _Check_return_ HRESULT APIENTRY Clear(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_CLEAR*, _In_ UINT, _In_ CONST RECT*);
    _Check_return_ HRESULT APIENTRY UpdatePalette(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UPDATEPALETTE*, _In_ CONST PALETTEENTRY*);
    _Check_return_ HRESULT APIENTRY SetPalette(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPALETTE*);
    _Check_return_ HRESULT APIENTRY SetVertexShaderConstF(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETVERTEXSHADERCONST*, _In_ CONST VOID*);
    _Check_return_ HRESULT APIENTRY MultiplyTransform(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_MULTIPLYTRANSFORM*);
    _Check_return_ HRESULT APIENTRY SetTransform(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETTRANSFORM*);
    _Check_return_ HRESULT APIENTRY SetViewport(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_VIEWPORTINFO*);
    _Check_return_ HRESULT APIENTRY SetZRange(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_ZRANGE*);
    _Check_return_ HRESULT APIENTRY SetMaterial(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETMATERIAL*);
    _Check_return_ HRESULT APIENTRY SetLight(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETLIGHT*, _In_ CONST D3DDDI_LIGHT*);
    _Check_return_ HRESULT APIENTRY CreateLight(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_CREATELIGHT*);
    _Check_return_ HRESULT APIENTRY DestroyLight(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DESTROYLIGHT*);
    _Check_return_ HRESULT APIENTRY SetClipPlane(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETCLIPPLANE*);
    _Check_return_ HRESULT APIENTRY GetInfo(_In_ HANDLE hDevice, _In_ UINT, _Out_writes_bytes_(DevInfoSize)VOID*, _In_ UINT DevInfoSize);
    _Check_return_ HRESULT APIENTRY Lock(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_LOCK*);
    _Check_return_ HRESULT APIENTRY Unlock(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UNLOCK*);
    _Check_return_ HRESULT APIENTRY LockAsync(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_LOCKASYNC*);
    _Check_return_ HRESULT APIENTRY UnlockAsync(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UNLOCKASYNC*);
    _Check_return_ HRESULT APIENTRY Rename(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_RENAME*);
    _Check_return_ HRESULT APIENTRY DestroyResource(_In_ HANDLE hDevice, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY SetDisplayMode(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETDISPLAYMODE*);
    _Check_return_ HRESULT APIENTRY CreateVertexShaderDecl(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEVERTEXSHADERDECL*, _In_ CONST D3DDDIVERTEXELEMENT*);
    _Check_return_ HRESULT APIENTRY SetVertexShaderDecl(_In_ HANDLE hDevice, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY DeleteVertexShaderDecl(_In_ HANDLE hDevice, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY CreateVertexShaderFunc(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEVERTEXSHADERFUNC*, _In_ CONST UINT*);
    _Check_return_ HRESULT APIENTRY SetVertexShaderFunc(_In_ HANDLE hDevice, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY DeleteVertexShaderFunc(_In_ HANDLE hDevice, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY SetVertexShaderConstI(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETVERTEXSHADERCONSTI*, _In_ CONST INT*);
    _Check_return_ HRESULT APIENTRY SetVertexShaderConstB(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETVERTEXSHADERCONSTB*, _In_ CONST BOOL*);
    _Check_return_ HRESULT APIENTRY SetScissorRect(_In_ HANDLE hDevice, _In_ CONST RECT*);
    _Check_return_ HRESULT APIENTRY SetStreamSource(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETSTREAMSOURCE*);
    _Check_return_ HRESULT APIENTRY SetStreamSourceFreq(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETSTREAMSOURCEFREQ*);
    _Check_return_ HRESULT APIENTRY SetConvolutionKernelMono(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETCONVOLUTIONKERNELMONO*);
    _Check_return_ HRESULT APIENTRY ComposeRects(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_COMPOSERECTS*);
    _Check_return_ HRESULT APIENTRY Blit(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_BLT*);
    _Check_return_ HRESULT APIENTRY ColorFill(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_COLORFILL*);
    _Check_return_ HRESULT APIENTRY DepthFill(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DEPTHFILL*);
    _Check_return_ HRESULT APIENTRY CreateQuery(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEQUERY*);
    _Check_return_ HRESULT APIENTRY DestroyQuery(_In_ HANDLE hDevice, _In_ CONST HANDLE);
    _Check_return_ HRESULT APIENTRY IssueQuery(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_ISSUEQUERY*);
    _Check_return_ HRESULT APIENTRY GetQueryData(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_GETQUERYDATA*);
    _Check_return_ HRESULT APIENTRY SetRenderTarget(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETRENDERTARGET*);
    _Check_return_ HRESULT APIENTRY SetDepthStencil(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETDEPTHSTENCIL*);
    _Check_return_ HRESULT APIENTRY GenerateMipSubLevels(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_GENERATEMIPSUBLEVELS*);
    _Check_return_ HRESULT APIENTRY SetPixelShaderConstI(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPIXELSHADERCONSTI*, _In_ CONST INT*);
    _Check_return_ HRESULT APIENTRY SetPixelShaderConstB(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPIXELSHADERCONSTB*, _In_ CONST BOOL*);
    _Check_return_ HRESULT APIENTRY CreatePixelShader(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEPIXELSHADER*, _In_ CONST UINT*);
    _Check_return_ HRESULT APIENTRY DeletePixelShader(_In_ HANDLE hDevice, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY CreateDecodeDevice(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEDECODEDEVICE*);
    _Check_return_ HRESULT APIENTRY DestroyDecodeDevice(_In_ HANDLE hDevice, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY SetDecodeRenderTarget(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETDECODERENDERTARGET*);
    _Check_return_ HRESULT APIENTRY DecodeBeginFrame(_In_ HANDLE hDevice, _In_ D3DDDIARG_DECODEBEGINFRAME*);
    _Check_return_ HRESULT APIENTRY DecodeEndFrame(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_DECODEENDFRAME*);
    _Check_return_ HRESULT APIENTRY DecodeExecute(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DECODEEXECUTE*);
    _Check_return_ HRESULT APIENTRY DecodeExtensionExecuter(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_DECODEEXTENSIONEXECUTE*);
    _Check_return_ HRESULT APIENTRY CreateVideoProcessDevice(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEVIDEOPROCESSDEVICE*);
    _Check_return_ HRESULT APIENTRY DestroyVideoProcessDevice(_In_ HANDLE hDevice, _In_ HANDLE hVideoProcessor);
    _Check_return_ HRESULT APIENTRY VideoProcessBeginFrame(_In_ HANDLE hDevice, _In_ HANDLE hVideoProcess);
    _Check_return_ HRESULT APIENTRY VideoProcessEndFrame(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_VIDEOPROCESSENDFRAME*);
    _Check_return_ HRESULT APIENTRY SetVideoProcessRenderTarget(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETVIDEOPROCESSRENDERTARGET*);
    _Check_return_ HRESULT APIENTRY VideoProcessBlit(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_VIDEOPROCESSBLT*);
    _Check_return_ HRESULT APIENTRY CreateExtensionDevice(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEEXTENSIONDEVICE*);
    _Check_return_ HRESULT APIENTRY DestroyExtensionDevice(_In_ HANDLE hDevice, _In_ HANDLE hExtension);
    _Check_return_ HRESULT APIENTRY ExtensionExecute(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_EXTENSIONEXECUTE*);
    _Check_return_ HRESULT APIENTRY DestroyDevice(_In_ HANDLE hDevice);
    _Check_return_ HRESULT APIENTRY CreateOverlay(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEOVERLAY*);
    _Check_return_ HRESULT APIENTRY UpdateOverlay(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UPDATEOVERLAY*);
    _Check_return_ HRESULT APIENTRY FlipOverlay(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_FLIPOVERLAY*);
    _Check_return_ HRESULT APIENTRY GetOverlayColorControls(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_GETOVERLAYCOLORCONTROLS*);
    _Check_return_ HRESULT APIENTRY SetOverlayColorControls(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETOVERLAYCOLORCONTROLS*);
    _Check_return_ HRESULT APIENTRY DestroyOverlay(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DESTROYOVERLAY*);
    _Check_return_ HRESULT APIENTRY QueryResourceResidency(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_QUERYRESOURCERESIDENCY*);
    _Check_return_ HRESULT APIENTRY OpenResource(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_OPENRESOURCE*);
    _Check_return_ HRESULT APIENTRY GetCaptureAllocationHandle(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_GETCAPTUREALLOCATIONHANDLE*);
    _Check_return_ HRESULT APIENTRY CaptureToSystem(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_CAPTURETOSYSMEM*);
    _Check_return_ HRESULT APIENTRY DXVAHD_CreateVideoProcessor(_In_ HANDLE, _Inout_ D3DDDIARG_DXVAHD_CREATEVIDEOPROCESSOR*);
    _Check_return_ HRESULT APIENTRY DXVAHD_SetVideoProcessorBlitState(_In_ HANDLE, _In_ CONST D3DDDIARG_DXVAHD_SETVIDEOPROCESSBLTSTATE*);
    _Check_return_ HRESULT APIENTRY DXVAHD_GetVideoProcessorBlitState(_In_ HANDLE, _Inout_ D3DDDIARG_DXVAHD_GETVIDEOPROCESSBLTSTATEPRIVATE*);
    _Check_return_ HRESULT APIENTRY DXVAHD_SetVideoProcessorStreamState(_In_ HANDLE, _In_ CONST D3DDDIARG_DXVAHD_SETVIDEOPROCESSSTREAMSTATE*);
    _Check_return_ HRESULT APIENTRY DXVAHD_GetVideoProcessorStreamState(_In_ HANDLE, _Inout_ D3DDDIARG_DXVAHD_GETVIDEOPROCESSSTREAMSTATEPRIVATE*);
    _Check_return_ HRESULT APIENTRY DXVAHD_VideoProcessBlitHD(_In_ HANDLE, _In_ CONST D3DDDIARG_DXVAHD_VIDEOPROCESSBLTHD*);
    _Check_return_ HRESULT APIENTRY DXVAHD_DestroyVideoProcessor(_In_ HANDLE, _In_ HANDLE);
    _Check_return_ HRESULT APIENTRY CreateAuthenticatedChannel(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEAUTHENTICATEDCHANNEL*);
    _Check_return_ HRESULT APIENTRY AuthenticatedChannelKeyExchange(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_AUTHENTICATEDCHANNELKEYEXCHANGE*);
    _Check_return_ HRESULT APIENTRY QueryAuthenticatedChannel(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_QUERYAUTHENTICATEDCHANNEL*);
    _Check_return_ HRESULT APIENTRY ConfigureAuthenticatedChannel(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_CONFIGUREAUTHENTICATEDCHANNEL*);
    _Check_return_ HRESULT APIENTRY DestroyAuthenticatedChannel(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DESTROYAUTHENTICATEDCHANNEL*);
    _Check_return_ HRESULT APIENTRY CreateCrytoSession(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATECRYPTOSESSION*);
    _Check_return_ HRESULT APIENTRY CrytoSessionKeyExchange(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CRYPTOSESSIONKEYEXCHANGE*);
    _Check_return_ HRESULT APIENTRY DestroyCryptoSession(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DESTROYCRYPTOSESSION*);
    _Check_return_ HRESULT APIENTRY EncryptionBlit(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_ENCRYPTIONBLT*);
    _Check_return_ HRESULT APIENTRY GetPitch(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_GETPITCH*);
    _Check_return_ HRESULT APIENTRY StartSessionKeyReresh(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_STARTSESSIONKEYREFRESH*);
    _Check_return_ HRESULT APIENTRY FinishSessionKeyRefresh(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_FINISHSESSIONKEYREFRESH*);
    _Check_return_ HRESULT APIENTRY GetEncryptionBlitKey(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_GETENCRYPTIONBLTKEY*);
    _Check_return_ HRESULT APIENTRY DecryptionBlit(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DECRYPTIONBLT*);
    _Check_return_ HRESULT APIENTRY ResolveSharedResource(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_RESOLVESHAREDRESOURCE*);
    _Check_return_ HRESULT APIENTRY VolumeBlit1(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_VOLUMEBLT1*);
    _Check_return_ HRESULT APIENTRY BufferBlit1(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_BUFFERBLT1*);
    _Check_return_ HRESULT APIENTRY TextureBlit1(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_TEXBLT1*);
    _Check_return_ HRESULT APIENTRY Discard(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DISCARD*);
    _Check_return_ HRESULT APIENTRY OfferResources(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_OFFERRESOURCES*);
    _Check_return_ HRESULT APIENTRY ReclaimResources(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_RECLAIMRESOURCES*);
    _Check_return_ HRESULT APIENTRY CheckDirectFlipSupport(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CHECKDIRECTFLIPSUPPORT*);
    _Check_return_ HRESULT APIENTRY CreateResource2(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATERESOURCE2*);
    _Check_return_ HRESULT APIENTRY CheckMultiplaneOverlaySupport(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CHECKMULTIPLANEOVERLAYSUPPORT*);
    _Check_return_ HRESULT APIENTRY PresentMultiplaneOverlay(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_PRESENTMULTIPLANEOVERLAY*);
    _Check_return_ HRESULT APIENTRY Flush1(_In_ HANDLE hDevice, UINT /*D3DDDI_FLUSH_FLAGS*/ FlushFlags);
    _Check_return_ HRESULT APIENTRY UpdateSubresourceUP(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UPDATESUBRESOURCEUP*);
    _Check_return_ HRESULT APIENTRY Present1(_In_ HANDLE hDevice, _In_ D3DDDIARG_PRESENT1*);
    _Check_return_ HRESULT APIENTRY CheckPresentDurationSupport(_In_ HANDLE hDevice, _In_ D3DDDIARG_CHECKPRESENTDURATIONSUPPORT*);
    _Check_return_ HRESULT APIENTRY SetMarkerMode(_In_ HANDLE hDevice, _In_ D3DDDI_MARKERTYPE Type, /*D3DDDI_SETMARKERMODE*/ UINT Flags);
    _Check_return_ HRESULT APIENTRY SetMarker(_In_ HANDLE hDevice);
    VOID APIENTRY AcquireResource(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SYNCTOKEN*);
    VOID APIENTRY ReleaseResource(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SYNCTOKEN*);
    _Check_return_ HRESULT APIENTRY CheckCounter(_In_ HANDLE hDevice, _In_ D3DDDIQUERYTYPE, _Out_ D3DDDI_COUNTER_TYPE*, _Out_ UINT*, _Out_writes_to_opt_(*pNameLength, *pNameLength) LPSTR, _Inout_opt_ UINT* pNameLength, _Out_writes_to_opt_(*pUnitsLength, *pUnitsLength) LPSTR, _Inout_opt_ UINT* pUnitsLength, _Out_writes_to_opt_(*pDescriptionLength, *pDescriptionLength) LPSTR, _Inout_opt_ UINT* pDescriptionLength);
    VOID APIENTRY CheckCounterInfo(_In_ HANDLE hDevice, _Out_ D3DDDIARG_COUNTER_INFO*);

    static const D3DDDI_ADAPTERFUNCS g_9on12AdapterFunctions =
    {
        GetCaps,                                                     /*PFND3DDDI_GETCAPS                       pfnGetCaps;                                           */
        CreateDevice,                                                /*PFND3DDDI_CREATEDEVICE                  pfnCreateDevice;                                      */
        CloseAdapter,                                                /*PFND3DDDI_CLOSEADAPTER                  pfnCloseAdapter;                                      */
    };

   _Check_return_ HRESULT APIENTRY SetPixelShaderConstF(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPIXELSHADERCONST* pSetConst, _In_ CONST FLOAT* pFloat);

    static const D3DDDI_DEVICEFUNCS g_9on12DeviceFuntions =
    {
        SetRenderState,                         /*PFND3DDDI_SETRENDERSTATE                            pfnSetRenderState;                        */
        UpdateWindowInfo,                       /*PFND3DDDI_UPDATEWINFO                               pfnUpdateWInfo;                           */
        ValidateDevice,                         /*PFND3DDDI_VALIDATEDEVICE                            pfnValidateDevice;                        */
        SetTextureStageState,                   /*PFND3DDDI_SETTEXTURESTAGESTATE                      pfnSetTextureStageState;                  */
        SetTexture,                             /*PFND3DDDI_SETTEXTURE                                pfnSetTexture;                            */
        SetPixelShader,                         /*PFND3DDDI_SETPIXELSHADER                            pfnSetPixelShader;                        */
        SetPixelShaderConstF,                   /*PFND3DDDI_SETPIXELSHADERCONST                       pfnSetPixelShaderConst;                   */
        SetStreamSourceUM,                      /*PFND3DDDI_SETSTREAMSOURCEUM                         pfnSetStreamSourceUm;                     */
        SetIndices,                             /*PFND3DDDI_SETINDICES                                pfnSetIndices;                            */
        SetIndicesUM,                           /*PFND3DDDI_SETINDICESUM                              pfnSetIndicesUm;                          */
        DrawPrimitive,                          /*PFND3DDDI_DRAWPRIMITIVE                             pfnDrawPrimitive;                         */
        DrawIndexedPrimitive,                   /*PFND3DDDI_DRAWINDEXEDPRIMITIVE                      pfnDrawIndexedPrimitive;                  */
        DrawRectPatch,                          /*PFND3DDDI_DRAWRECTPATCH                             pfnDrawRectPatch;                         */
        DrawTriPatch,                           /*PFND3DDDI_DRAWTRIPATCH                              pfnDrawTriPatch;                          */
        DrawPrimitive2,                         /*PFND3DDDI_DRAWPRIMITIVE2                            pfnDrawPrimitive2;                        */
        DrawIndexedPrimitive2,                  /*PFND3DDDI_DRAWINDEXEDPRIMITIVE2                     pfnDrawIndexedPrimitive2;                 */
        nullptr,                                /*PFND3DDDI_VOLBLT                                    pfnVolBlt;                                */
        nullptr,                                /*PFND3DDDI_BUFBLT                                    pfnBufBlt;                                */
        nullptr,                                /*PFND3DDDI_TEXBLT                                    pfnTexBlt;                                */
        SetState,                               /*PFND3DDDI_STATESET                                  pfnStateSet;                              */
        SetPriority,                            /*PFND3DDDI_SETPRIORITY                               pfnSetPriority;                           */
        Clear,                                  /*PFND3DDDI_CLEAR                                     pfnClear;                                 */
        UpdatePalette,                          /*PFND3DDDI_UPDATEPALETTE                             pfnUpdatePalette;                         */
        SetPalette,                             /*PFND3DDDI_SETPALETTE                                pfnSetPalette;                            */
        SetVertexShaderConstF,                  /*PFND3DDDI_SETVERTEXSHADERCONST                      pfnSetVertexShaderConst;                  */
        MultiplyTransform,                      /*PFND3DDDI_MULTIPLYTRANSFORM                         pfnMultiplyTransform;                     */
        SetTransform,                           /*PFND3DDDI_SETTRANSFORM                              pfnSetTransform;                          */
        SetViewport,                            /*PFND3DDDI_SETVIEWPORT                               pfnSetViewport;                           */
        SetZRange,                              /*PFND3DDDI_SETZRANGE                                 pfnSetZRange;                             */
        SetMaterial,                            /*PFND3DDDI_SETMATERIAL                               pfnSetMaterial;                           */
        SetLight,                               /*PFND3DDDI_SETLIGHT                                  pfnSetLight;                              */
        CreateLight,                            /*PFND3DDDI_CREATELIGHT                               pfnCreateLight;                           */
        DestroyLight,                           /*PFND3DDDI_DESTROYLIGHT                              pfnDestroyLight;                          */
        SetClipPlane,                           /*PFND3DDDI_SETCLIPPLANE                              pfnSetClipPlane;                          */
        GetInfo,                                /*PFND3DDDI_GETINFO                                   pfnGetInfo;                               */
        Lock,                                   /*PFND3DDDI_LOCK                                      pfnLock;                                  */
        Unlock,                                 /*PFND3DDDI_UNLOCK                                    pfnUnlock;                                */
        nullptr,                                /*PFND3DDDI_CREATERESOURCE                            pfnCreateResource;                        */
        DestroyResource,                        /*PFND3DDDI_DESTROYRESOURCE                           pfnDestroyResource;                       */
        SetDisplayMode,                         /*PFND3DDDI_SETDISPLAYMODE                            pfnSetDisplayMode;                        */
        nullptr,                                /*PFND3DDDI_PRESENT                                   pfnPresent;                               */
        nullptr,                                /*PFND3DDDI_FLUSH                                     pfnFlush;                                 */
        CreateVertexShaderFunc,                 /*PFND3DDDI_CREATEVERTEXSHADERFUNC                    pfnCreateVertexShaderFunc;                */
        DeleteVertexShaderFunc,                 /*PFND3DDDI_DELETEVERTEXSHADERFUNC                    pfnDeleteVertexShaderFunc;                */
        SetVertexShaderFunc,                    /*PFND3DDDI_SETVERTEXSHADERFUNC                       pfnSetVertexShaderFunc;                   */
        CreateVertexShaderDecl,                 /*PFND3DDDI_CREATEVERTEXSHADERDECL                    pfnCreateVertexShaderDecl;                */
        DeleteVertexShaderDecl,                 /*PFND3DDDI_DELETEVERTEXSHADERDECL                    pfnDeleteVertexShaderDecl;                */
        SetVertexShaderDecl,                    /*PFND3DDDI_SETVERTEXSHADERDECL                       pfnSetVertexShaderDecl;                   */
        SetVertexShaderConstI,                  /*PFND3DDDI_SETVERTEXSHADERCONSTI                     pfnSetVertexShaderConstI;                 */
        SetVertexShaderConstB,                  /*PFND3DDDI_SETVERTEXSHADERCONSTB                     pfnSetVertexShaderConstB;                 */
        SetScissorRect,                         /*PFND3DDDI_SETSCISSORRECT                            pfnSetScissorRect;                        */
        SetStreamSource,                        /*PFND3DDDI_SETSTREAMSOURCE                           pfnSetStreamSource;                       */
        SetStreamSourceFreq,                    /*PFND3DDDI_SETSTREAMSOURCEFREQ                       pfnSetStreamSourceFreq;                   */
        SetConvolutionKernelMono,               /*PFND3DDDI_SETCONVOLUTIONKERNELMONO                  pfnSetConvolutionKernelMono;              */
        ComposeRects,                           /*PFND3DDDI_COMPOSERECTS                              pfnComposeRects;                          */
        Blit,                                   /*PFND3DDDI_BLT                                       pfnBlt;                                   */
        ColorFill,                              /*PFND3DDDI_COLORFILL                                 pfnColorFill;                             */
        DepthFill,                              /*PFND3DDDI_DEPTHFILL                                 pfnDepthFill;                             */
        CreateQuery,                            /*PFND3DDDI_CREATEQUERY                               pfnCreateQuery;                           */
        DestroyQuery,                           /*PFND3DDDI_DESTROYQUERY                              pfnDestroyQuery;                          */
        IssueQuery,                             /*PFND3DDDI_ISSUEQUERY                                pfnIssueQuery;                            */
        GetQueryData,                           /*PFND3DDDI_GETQUERYDATA                              pfnGetQueryData;                          */
        SetRenderTarget,                        /*PFND3DDDI_SETRENDERTARGET                           pfnSetRenderTarget;                       */
        SetDepthStencil,                        /*PFND3DDDI_SETDEPTHSTENCIL                           pfnSetDepthStencil;                       */
        GenerateMipSubLevels,                   /*PFND3DDDI_GENERATEMIPSUBLEVELS                      pfnGenerateMipSubLevels;                  */
        SetPixelShaderConstI,                   /*PFND3DDDI_SETPIXELSHADERCONSTI                      pfnSetPixelShaderConstI;                  */
        SetPixelShaderConstB,                   /*PFND3DDDI_SETPIXELSHADERCONSTB                      pfnSetPixelShaderConstB;                  */
        CreatePixelShader,                      /*PFND3DDDI_CREATEPIXELSHADER                         pfnCreatePixelShader;                     */
        DeletePixelShader,                      /*PFND3DDDI_DELETEPIXELSHADER                         pfnDeletePixelShader;                     */
        CreateDecodeDevice,                     /*PFND3DDDI_CREATEDECODEDEVICE                        pfnCreateDecodeDevice;                    */
        DestroyDecodeDevice,                    /*PFND3DDDI_DESTROYDECODEDEVICE                       pfnDestroyDecodeDevice;                   */
        SetDecodeRenderTarget,                  /*PFND3DDDI_SETDECODERENDERTARGET                     pfnSetDecodeRenderTarget;                 */
        DecodeBeginFrame,                       /*PFND3DDDI_DECODEBEGINFRAME                          pfnDecodeBeginFrame;                      */
        DecodeEndFrame,                         /*PFND3DDDI_DECODEENDFRAME                            pfnDecodeEndFrame;                        */
        DecodeExecute,                          /*PFND3DDDI_DECODEEXECUTE                             pfnDecodeExecute;                         */
        DecodeExtensionExecuter,                /*PFND3DDDI_DECODEEXTENSIONEXECUTE                    pfnDecodeExtensionExecute;                */
        CreateVideoProcessDevice,               /*PFND3DDDI_CREATEVIDEOPROCESSDEVICE                  pfnCreateVideoProcessDevice;              */
        DestroyVideoProcessDevice,              /*PFND3DDDI_DESTROYVIDEOPROCESSDEVICE                 pfnDestroyVideoProcessDevice;             */
        VideoProcessBeginFrame,                 /*PFND3DDDI_VIDEOPROCESSBEGINFRAME                    pfnVideoProcessBeginFrame;                */
        VideoProcessEndFrame,                   /*PFND3DDDI_VIDEOPROCESSENDFRAME                      pfnVideoProcessEndFrame;                  */
        SetVideoProcessRenderTarget,            /*PFND3DDDI_SETVIDEOPROCESSRENDERTARGET               pfnSetVideoProcessRenderTarget;           */
        VideoProcessBlit,                       /*PFND3DDDI_VIDEOPROCESSBLT                           pfnVideoProcessBlt;                       */
        CreateExtensionDevice,                  /*PFND3DDDI_CREATEEXTENSIONDEVICE                     pfnCreateExtensionDevice;                 */
        DestroyExtensionDevice,                 /*PFND3DDDI_DESTROYEXTENSIONDEVICE                    pfnDestroyExtensionDevice;                */
        ExtensionExecute,                       /*PFND3DDDI_EXTENSIONEXECUTE                          pfnExtensionExecute;                      */
        CreateOverlay,                          /*PFND3DDDI_CREATEOVERLAY                             pfnCreateOverlay;                         */
        UpdateOverlay,                          /*PFND3DDDI_UPDATEOVERLAY                             pfnUpdateOverlay;                         */
        FlipOverlay,                            /*PFND3DDDI_FLIPOVERLAY                               pfnFlipOverlay;                           */
        GetOverlayColorControls,                /*PFND3DDDI_GETOVERLAYCOLORCONTROLS                   pfnGetOverlayColorControls;               */
        SetOverlayColorControls,                /*PFND3DDDI_SETOVERLAYCOLORCONTROLS                   pfnSetOverlayColorControls;               */
        DestroyOverlay,                         /*PFND3DDDI_DESTROYOVERLAY                            pfnDestroyOverlay;                        */
        DestroyDevice,                          /*PFND3DDDI_DESTROYDEVICE                             pfnDestroyDevice;                         */
        QueryResourceResidency,                 /*PFND3DDDI_QUERYRESOURCERESIDENCY                    pfnQueryResourceResidency;                */
        OpenResource,                           /*PFND3DDDI_OPENRESOURCE                              pfnOpenResource;                          */
        GetCaptureAllocationHandle,             /*PFND3DDDI_GETCAPTUREALLOCATIONHANDLE                pfnGetCaptureAllocationHandle;            */
        CaptureToSystem,                        /*PFND3DDDI_CAPTURETOSYSMEM                           pfnCaptureToSysMem;                       */
        LockAsync,                              /*PFND3DDDI_LOCKASYNC                                 pfnLockAsync;                             */
        UnlockAsync,                            /*PFND3DDDI_UNLOCKASYNC                               pfnUnlockAsync;                           */
        Rename,                                 /*PFND3DDDI_RENAME                                    pfnRename;                                */
        DXVAHD_CreateVideoProcessor,            /*PFND3DDDI_DXVAHD_CREATEVIDEOPROCESSOR               pfnCreateVideoProcessor;                  */
        DXVAHD_SetVideoProcessorBlitState,      /*PFND3DDDI_DXVAHD_SETVIDEOPROCESSBLTSTATE            pfnSetVideoProcessBltState;               */
        DXVAHD_GetVideoProcessorBlitState,      /*PFND3DDDI_DXVAHD_GETVIDEOPROCESSBLTSTATEPRIVATE     pfnGetVideoProcessBltStatePrivate;        */
        DXVAHD_SetVideoProcessorStreamState,    /*PFND3DDDI_DXVAHD_SETVIDEOPROCESSSTREAMSTATE         pfnSetVideoProcessStreamState;            */
        DXVAHD_GetVideoProcessorStreamState,    /*PFND3DDDI_DXVAHD_GETVIDEOPROCESSSTREAMSTATEPRIVATE  pfnGetVideoProcessStreamStatePrivate;     */
        DXVAHD_VideoProcessBlitHD,              /*PFND3DDDI_DXVAHD_VIDEOPROCESSBLTHD                  pfnVideoProcessBltHD;                     */
        DXVAHD_DestroyVideoProcessor,           /*PFND3DDDI_DXVAHD_DESTROYVIDEOPROCESSOR              pfnDestroyVideoProcessor;                 */
        CreateAuthenticatedChannel,             /*PFND3DDDI_CREATEAUTHENTICATEDCHANNEL                pfnCreateAuthenticatedChannel;            */
        AuthenticatedChannelKeyExchange,        /*PFND3DDDI_AUTHENTICATEDCHANNELKEYEXCHANGE           pfnAuthenticatedChannelKeyExchange;       */
        QueryAuthenticatedChannel,              /*PFND3DDDI_QUERYAUTHENTICATEDCHANNEL                 pfnQueryAuthenticatedChannel;             */
        ConfigureAuthenticatedChannel,          /*PFND3DDDI_CONFIGUREAUTHENICATEDCHANNEL              pfnConfigureAuthenticatedChannel;         */
        DestroyAuthenticatedChannel,            /*PFND3DDDI_DESTROYAUTHENTICATEDCHANNEL               pfnDestroyAuthenticatedChannel;           */
        CreateCrytoSession,                     /*PFND3DDDI_CREATECRYPTOSESSION                       pfnCreateCryptoSession;                   */
        CrytoSessionKeyExchange,                /*PFND3DDDI_CRYPTOSESSIONKEYEXCHANGE                  pfnCryptoSessionKeyExchange;              */
        DestroyCryptoSession,                   /*PFND3DDDI_DESTROYCRYPTOSESSION                      pfnDestroyCryptoSession;                  */
        EncryptionBlit,                         /*PFND3DDDI_ENCRYPTIONBLT                             pfnEncryptionBlt;                         */
        GetPitch,                               /*PFND3DDDI_GETPITCH                                  pfnGetPitch;                              */
        StartSessionKeyReresh,                  /*PFND3DDDI_STARTSESSIONKEYREFRESH                    pfnStartSessionKeyRefresh;                */
        FinishSessionKeyRefresh,                /*PFND3DDDI_FINISHSESSIONKEYREFRESH                   pfnFinishSessionKeyRefresh;               */
        GetEncryptionBlitKey,                   /*PFND3DDDI_GETENCRYPTIONBLTKEY                       pfnGetEncryptionBltKey;                   */
        DecryptionBlit,                         /*PFND3DDDI_DECRYPTIONBLT                             pfnDecryptionBlt;                         */
        ResolveSharedResource,                  /*PFND3DDDI_RESOLVESHAREDRESOURCE                     pfnResolveSharedResource;                 */
        VolumeBlit1,                            /*PFND3DDDI_VOLBLT1                                   pfnVolBlt1;                               */
        BufferBlit1,                            /*PFND3DDDI_BUFBLT1                                   pfnBufBlt1;                               */
        TextureBlit1,                           /*PFND3DDDI_TEXBLT1                                   pfnTexBlt1;                               */
        Discard,                                /*PFND3DDDI_DISCARD                                   pfnDiscard;                               */
        OfferResources,                         /*PFND3DDDI_OFFERRESOURCES                            pfnOfferResources;                        */
        ReclaimResources,                       /*PFND3DDDI_RECLAIMRESOURCES                          pfnReclaimResources;                      */
        CheckDirectFlipSupport,                 /*PFND3DDDI_CHECKDIRECTFLIPSUPPORT                    pfnCheckDirectFlipSupport;                */
        CreateResource2,                        /*PFND3DDDI_CREATERESOURCE2                           pfnCreateResource2;                       */
        CheckMultiplaneOverlaySupport,          /*PFND3DDDI_CHECKMULTIPLANEOVERLAYSUPPORT             pfnCheckMultiPlaneOverlaySupport;         */
        PresentMultiplaneOverlay,               /*PFND3DDDI_PRESENTMULTIPLANEOVERLAY                  pfnPresentMultiPlaneOverlay;              */
        nullptr,                                /*                                                    pfnReserved1;                             */
        Flush1,                                 /*PFND3DDDI_FLUSH1                                    pfnFlush1;                                */
        CheckCounterInfo,                       /*PFND3DDDI_CHECKCOUNTERINFO                          pfnCheckCounterInfo;                      */
        CheckCounter,                           /*PFND3DDDI_CHECKCOUNTER                              pfnCheckCounter;                          */
        UpdateSubresourceUP,                    /*PFND3DDDI_UPDATESUBRESOURCEUP                       pfnUpdateSubresourceUP;                   */
        Present1,                               /*PFND3DDDI_PRESENT1                                  pfnPresent1;                              */
        CheckPresentDurationSupport,            /*PFND3DDDI_CHECKPRESENTDURATIONSUPPORT               pfnCheckPresentDurationSupport;           */
        SetMarker,                              /*PFND3DDDI_SETMARKER                                 pfnSetMarker;                             */
        SetMarkerMode,                          /*PFND3DDDI_SETMARKERMODE                             pfnSetMarkerMode;                         */
        /* Opt out of having the DX9 runtime manage residency by leaving TrimResidencySet null, the translation layer does this internally      */
        nullptr,                                /*PFND3DDDI_TRIMRESIDENCYSET                          pfnTrimResidencySet;                      */
        AcquireResource,                        /*PFND3DDDI_SYNCTOKEN                                 pfnAcquireResource                        */
        ReleaseResource,                        /*PFND3DDDI_SYNCTOKEN                                 pfnReleaseResource                        */
    };
};
