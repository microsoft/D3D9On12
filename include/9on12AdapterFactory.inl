// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

namespace D3D9on12
{
    template <class AdapterType>
    AdapterType *AdapterFactory<AdapterType>::CreateAdapter(D3DDDIARG_OPENADAPTER &OpenAdapter, LUID *pAdapterLUID, D3D9ON12_CREATE_DEVICE_ARGS *pArgs)
    {
        AdapterType *adapter = new AdapterType(OpenAdapter);

        HRESULT hr = S_OK;
        try
        {
            if (pAdapterLUID == nullptr)
                ThrowFailure(E_FAIL);
            {
                CComPtr<IUnknown> pAdapter;

                {
                    CComPtr<IDXCoreAdapterFactory> pFactory;
                    if (SUCCEEDED(DXCoreCreateAdapterFactory(IID_PPV_ARGS(&pFactory))))
                    {
                        (void)pFactory->GetAdapterByLuid(*pAdapterLUID, IID_PPV_ARGS(&pAdapter));
                    }

                    if (pAdapter)
                    {
                        CComQIPtr<IDXCoreAdapter> pDXCoreAdapter = pAdapter;
                        ThrowFailure(pDXCoreAdapter->GetProperty(DXCoreAdapterProperty::HardwareID, &adapter->m_HWIDs));
                        ThrowFailure(pDXCoreAdapter->GetProperty(DXCoreAdapterProperty::DriverVersion, &adapter->m_DriverVersion));
                    }
                }
                if (!pAdapter)
                {
                    CComPtr<IDXGIFactory4> pFactory;
                    ThrowFailure(CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory)));
                    ThrowFailure(pFactory->EnumAdapterByLuid(*pAdapterLUID, IID_PPV_ARGS(&pAdapter)));

                    CComQIPtr<IDXGIAdapter> pDXGIAdapter = pAdapter;
                    DXGI_ADAPTER_DESC AdapterDesc;
                    ThrowFailure(pDXGIAdapter->GetDesc(&AdapterDesc));
                    adapter->m_HWIDs = {AdapterDesc.VendorId, AdapterDesc.DeviceId, AdapterDesc.SubSysId, AdapterDesc.Revision};

                    LARGE_INTEGER DriverVersion;
                    ThrowFailure(pDXGIAdapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &DriverVersion));
                    adapter->m_DriverVersion = DriverVersion.QuadPart;
                }

                if (pArgs && pArgs->pD3D12Device)
                {
                    ThrowFailure(pArgs->pD3D12Device->QueryInterface(&adapter->m_pD3D12Device));

                    // Ensure the app provided D3D12 runtime matches up with the adapter the 9on12 device is being created ondapterLuid));
                    ThrowFailure((memcmp(&adapter->m_pD3D12Device->GetAdapterLuid(), pAdapterLUID, sizeof(*pAdapterLUID)) == 0) ? S_OK : E_FAIL);
                }
                else
                {
                    if (RegistryConstants::g_cUseDebugLayer)
                    {
                        adapter->InitDebugLayer();
                    }

                    hr = D3D12CreateDevice(pAdapter, MinSupportedFeatureLevel, IID_PPV_ARGS(&adapter->m_pD3D12Device));
                    ThrowFailure(hr);
                }

                D3D12_FEATURE_DATA_D3D12_OPTIONS3 d3d12Options3;
                hr = adapter->m_pD3D12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &d3d12Options3, sizeof(d3d12Options3));
                ThrowFailure(hr);

                // TODO: 12281030 Once QC updates the BSP to have this on by default, just throw when
                // CastingFullyTypedFormatSupported is not supported. Allows for major improvement in
                // simplicity/efficiency in 9on12's present path
                adapter->m_bSupportsCastingTypelessResources = d3d12Options3.CastingFullyTypedFormatSupported;

                if (pArgs && pArgs->NumQueues > 0)
                {
                    // If a queue is provided, they better have provided the device they created it with
                    Check9on12(pArgs->pD3D12Device);
                    Check9on12(pArgs->NumQueues == 1);

                    ThrowFailure(pArgs->ppD3D12Queues[0]->QueryInterface(&adapter->m_pD3D12CommandQueue));
                    ThrowFailure(adapter->m_pD3D12CommandQueue->GetDesc().Type != D3D12_COMMAND_LIST_TYPE_DIRECT ? E_FAIL : S_OK);
                }
                else
                {
                    D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
                    commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                    commandQueueDesc.NodeMask = 0;
                    commandQueueDesc.Priority = 0;
                    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    ThrowFailure(adapter->m_pD3D12Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&adapter->m_pD3D12CommandQueue)));
                }
            }
        }
        catch (_com_error &hrEx)
        {
            hr = hrEx.Error();
        }
        catch (std::bad_alloc &)
        {
            hr = E_OUTOFMEMORY;
        }
        adapter->LogAdapterCreated(pAdapterLUID, hr);
        ThrowFailure(hr);

        OpenAdapter.hAdapter = Adapter::GetHandleFromAdapter(adapter);
        memcpy(OpenAdapter.pAdapterFuncs, &g_9on12AdapterFunctions, sizeof(*OpenAdapter.pAdapterFuncs)); // out: Driver function table
        OpenAdapter.DriverVersion = min<UINT>(OpenAdapter.Version, D3D_UMD_INTERFACE_VERSION);           // out: D3D UMD interface version

        return adapter;
    }
};