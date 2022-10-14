// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Adapter
    {
    public:
        Adapter( _Inout_ D3DDDIARG_OPENADAPTER& OpenAdapter, LUID* pAdapterLUID, D3D9ON12_CREATE_DEVICE_ARGS* pArgs );

        ~Adapter();

        HRESULT Destroy();

        static FORCEINLINE HANDLE GetHandleFromAdapter(Adapter* pAdapter){ return static_cast<HANDLE>(pAdapter); }
        static FORCEINLINE Adapter* GetAdapterFromHandle(HANDLE hAdapter){ return static_cast<Adapter*>(hAdapter); }

        CONST D3DDDI_ADAPTERCALLBACKS& GetRuntimeCallbacks() { return m_AdapterCallbacks; }
        
        void GetVideoCaps(_Inout_ CONST D3DDDIARG_GETCAPS* pGetCaps);
        void DeviceCreated(_In_ Device *pDevice) { m_pDevice = pDevice; }
        void DeviceDestroyed(_In_ Device *pDevice) { UNREFERENCED_PARAMETER(pDevice);  assert(pDevice == m_pDevice);  m_pDevice = nullptr; }

        static const UINT MAX_SURFACE_FORMATS = 48;
        UINT GetFormatData(FORMATOP* pFormatOPs, UINT uiNumFormats = 0);

        ID3D12Device *GetDevice() { return m_pD3D12Device; }
        ID3D12CommandQueue *GetCommandQueue() { return m_pD3D12CommandQueue; }

        bool SupportsCastingTypelessResources() { return m_bSupportsCastingTypelessResources; }
        bool RequiresYUY2BlitWorkaround() const;

    protected:
        virtual void LogAdapterCreated( LUID *pluid, HRESULT hr );

    private:
        void InitDebugLayer();

        CONST D3DDDI_ADAPTERCALLBACKS& m_AdapterCallbacks;
        bool m_bSupportsCastingTypelessResources;
        Device *m_pDevice;      // weak-ref

        CComPtr<ID3D12Device> m_pD3D12Device;
        CComPtr<ID3D12CommandQueue> m_pD3D12CommandQueue;
        DXCoreHardwareID m_HWIDs;
        uint64_t m_DriverVersion;

    public:
        const D3D9ON12_PRIVATE_CALLBACKS m_privateCallbacks;

        const bool m_bSupportsNewPresent;
        const bool m_bSupportsShaderSigning;
    };
};
