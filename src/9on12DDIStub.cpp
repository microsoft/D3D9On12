// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{

    /* Depricated DDI which will probably never have to be fully implemented */
    _Check_return_ HRESULT APIENTRY DrawRectPatch(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWRECTPATCH*, _In_ CONST D3DDDIRECTPATCH_INFO*, _In_ CONST FLOAT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY DrawTriPatch(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWTRIPATCH*, _In_ CONST D3DDDITRIPATCH_INFO*, _In_ CONST FLOAT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY SetState(_In_ HANDLE hDevice, _In_ D3DDDIARG_STATESET*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY MultiplyTransform(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_MULTIPLYTRANSFORM*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetTransform(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETTRANSFORM*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetMaterial(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETMATERIAL*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetLight(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETLIGHT*, _In_ CONST D3DDDI_LIGHT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY CreateLight(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_CREATELIGHT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DestroyLight(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DESTROYLIGHT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    
    _Check_return_ HRESULT APIENTRY GetInfo(_In_ HANDLE hDevice, _In_ UINT, _Out_writes_bytes_(DevInfoSize)VOID*, _In_ UINT DevInfoSize)
    {
        UNREFERENCED_PARAMETER(DevInfoSize);

        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY ResolveSharedResource(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_RESOLVESHAREDRESOURCE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }


    /* DDI which still need to be implemented */
    _Check_return_ HRESULT APIENTRY SetPriority(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPRIORITY*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY UpdatePalette(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UPDATEPALETTE*, _In_ CONST PALETTEENTRY*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY SetPalette(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPALETTE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY SetConvolutionKernelMono(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETCONVOLUTIONKERNELMONO*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY ComposeRects(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_COMPOSERECTS*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY CreateExtensionDevice(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEEXTENSIONDEVICE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY DestroyExtensionDevice(_In_ HANDLE hDevice, _In_ HANDLE /*hExtension*/)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY ExtensionExecute(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_EXTENSIONEXECUTE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY CreateOverlay(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEOVERLAY*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY UpdateOverlay(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UPDATEOVERLAY*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY FlipOverlay(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_FLIPOVERLAY*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY GetOverlayColorControls(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_GETOVERLAYCOLORCONTROLS*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY SetOverlayColorControls(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETOVERLAYCOLORCONTROLS*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY DestroyOverlay(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DESTROYOVERLAY*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY GetCaptureAllocationHandle(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_GETCAPTUREALLOCATIONHANDLE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY CaptureToSystem(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_CAPTURETOSYSMEM*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

     _Check_return_ HRESULT APIENTRY CreateAuthenticatedChannel(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEAUTHENTICATEDCHANNEL*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY AuthenticatedChannelKeyExchange(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_AUTHENTICATEDCHANNELKEYEXCHANGE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY QueryAuthenticatedChannel(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_QUERYAUTHENTICATEDCHANNEL*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY ConfigureAuthenticatedChannel(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_CONFIGUREAUTHENTICATEDCHANNEL*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY DestroyAuthenticatedChannel(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DESTROYAUTHENTICATEDCHANNEL*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY CreateCrytoSession(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATECRYPTOSESSION*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY CrytoSessionKeyExchange(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CRYPTOSESSIONKEYEXCHANGE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY DestroyCryptoSession(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DESTROYCRYPTOSESSION*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY EncryptionBlit(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_ENCRYPTIONBLT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY GetPitch(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_GETPITCH*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY StartSessionKeyReresh(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_STARTSESSIONKEYREFRESH*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY FinishSessionKeyRefresh(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_FINISHSESSIONKEYREFRESH*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY GetEncryptionBlitKey(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_GETENCRYPTIONBLTKEY*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY DecryptionBlit(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DECRYPTIONBLT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY Discard(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DISCARD*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY OfferResources(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_OFFERRESOURCES*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY ReclaimResources(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_RECLAIMRESOURCES*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY CheckDirectFlipSupport(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CHECKDIRECTFLIPSUPPORT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY CheckMultiplaneOverlaySupport(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CHECKMULTIPLANEOVERLAYSUPPORT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY PresentMultiplaneOverlay(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_PRESENTMULTIPLANEOVERLAY*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY CheckPresentDurationSupport(_In_ HANDLE hDevice, _In_ D3DDDIARG_CHECKPRESENTDURATIONSUPPORT*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice); 
        if (pDevice == nullptr)
        { 
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }
};