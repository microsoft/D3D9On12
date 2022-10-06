// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    void APIENTRY GetPrivateDDITable(D3D9ON12_PRIVATE_DDI_TABLE *pPrivateDDITable)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        static const D3D9ON12_PRIVATE_DDI_TABLE cPrivateDDITable =
        {
            OpenAdapter_Private,

            GetSharedGDIHandle_Private,
            CreateSharedNTHandle_Private,
            GetDeviceExecutionState_Private,
            KmdPresent,

            CreateFence,
            OpenFence,
            ShareFence,
            WaitForFence,
            SignalFence,

            GetCompletedFenceValue,
            DestroyTrackedFence,

            QueryResourcePrivateDriverDataSize,
            OpenResource_Private,

            CreateKMTHandle,
            QueryResourceInfoFromKMTHandle,
            DestroyKMTHandle,

            CreateResourceWrappingHandle,
            DestroyResourceWrappingHandle,

            GetD3D12Device,
            TransitionResource,
            SetCurrentResourceState,

            SetMaximumFrameLatency,
            IsMaximumFrameLatencyReached,
            GetD3D12Resource,
            AddResourceWaitsToQueue,
            AddDeferredWaitsToResource
        };

        memcpy(pPrivateDDITable, &cPrivateDDITable, sizeof(D3D9ON12_PRIVATE_DDI_TABLE));
        D3D9on12_DDI_ENTRYPOINT_END_AND_REPORT_HR((HANDLE)0, S_OK);
    }

    HRESULT APIENTRY GetPrivateDDITableVersioned(_Inout_updates_bytes_(ddiTableSize) void* pPrivateDDITableVersioned, UINT ddiTableSize)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        if (ddiTableSize > sizeof(D3D9ON12_PRIVATE_DDI_TABLE_VERSIONED))
        {
            return E_INVALIDARG;
        }

        static const D3D9ON12_PRIVATE_DDI_TABLE_VERSIONED cPrivateDDITable =
        {
            OpenAdapter_Private,

            GetSharedGDIHandle_Private,
            CreateSharedNTHandle_Private,
            GetDeviceExecutionState_Private,
            KmdPresent,

            CreateFence,
            OpenFence,
            ShareFence,
            WaitForFence,
            SignalFence,

            GetCompletedFenceValue,
            DestroyTrackedFence,

            QueryResourcePrivateDriverDataSize,
            OpenResource_Private,

            CreateKMTHandle,
            QueryResourceInfoFromKMTHandle,
            DestroyKMTHandle,

            CreateResourceWrappingHandle,
            DestroyResourceWrappingHandle,

            GetD3D12Device,
            TransitionResource,
            SetCurrentResourceState,

            SetMaximumFrameLatency,
            IsMaximumFrameLatencyReached,
            GetD3D12Resource,
            AddResourceWaitsToQueue,
            AddDeferredWaitsToResource,

            // interface version 2
            CloseAndSubmitGraphicsCommandListForPresent,
            PreExecuteCommandList,
            PostExecuteCommandList
        };
        memcpy(pPrivateDDITableVersioned, &cPrivateDDITable, ddiTableSize);
        
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY GetDeviceExecutionState_Private(_In_ HANDLE hDevice)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        HRESULT hr = pDevice->GetContext().GetDeviceState();
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY CreateFence(HANDLE hDevice, UINT64 InitialValue, UINT Flags, _Out_ HANDLE* phFence)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        *phFence = Fence::GetHandleFromFence(new Fence(pDevice, InitialValue, Flags));
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY OpenFence(HANDLE hDevice, HANDLE hSharedFence, _Out_ BOOL* pbMonitored, _Out_ HANDLE* phFence)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Fence* pOpenedFence = new Fence(pDevice, hSharedFence);
        *phFence = Fence::GetHandleFromFence(pOpenedFence);
        *pbMonitored = pOpenedFence->IsMonitored();
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetDisplayMode(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETDISPLAYMODE*)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        // Unsure if we need to do anything here, DXGI already seems to do the right thing

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY ValidateDevice(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_VALIDATETEXTURESTAGESTATE* pValidateTextureStageState)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pValidateTextureStageState == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        // TODO: Verify that DX12 hardware is good enough for this assumption
        pValidateTextureStageState->NumPasses = 1;

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY Flush1(_In_ HANDLE hDevice, UINT FlushFlags)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = pDevice->FlushWork(false, FlushFlags);
        CHECK_HR(hr);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY DestroyDevice(_In_ HANDLE hDevice)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = pDevice->Destroy();
        CHECK_HR(hr);

        SAFE_DELETE(pDevice);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }


    void APIENTRY AcquireResource(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SYNCTOKEN* pToken)
    {
        HRESULT hr = S_OK;

        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);

        pDevice->FlushWork(false);

        D3DDDICB_SYNCTOKEN syncTokenCB = {};
        syncTokenCB.BroadcastContextCount = 0;
        syncTokenCB.hSyncToken = pToken->hSyncToken;
        hr = pDevice->GetRuntimeCallbacks().pfnAcquireResourceCb(pDevice->GetRuntimeHandle(), &syncTokenCB);

        D3D9on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    void APIENTRY ReleaseResource(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SYNCTOKEN *pToken)
    {
        HRESULT hr = S_OK;

        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);

        pDevice->FlushWork(false);

        D3DDDICB_SYNCTOKEN syncTokenCB = {};
        syncTokenCB.BroadcastContextCount = 0;
        syncTokenCB.hSyncToken = pToken->hSyncToken;
        hr = pDevice->GetRuntimeCallbacks().pfnReleaseResourceCb(pDevice->GetRuntimeHandle(), &syncTokenCB);

        D3D9on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    HRESULT APIENTRY GetD3D12Device(HANDLE hDevice, REFIID riid, void** ppv)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (hDevice == nullptr || ppv == nullptr)
        {
            return E_INVALIDARG;
        }

        HRESULT hr = pDevice->m_pDevice->QueryInterface(riid, ppv);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT APIENTRY GetD3D12Resource(HANDLE hResource, REFIID riid, void** ppv)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        HRESULT hr = S_OK;

        Resource* pResource = Resource::GetResourceFromHandle(hResource);
        if (pResource == nullptr || ppv == nullptr)
        {
            hr = E_INVALIDARG;
        }

        if (SUCCEEDED(hr))
        {
            auto* pTranslationLayerResource = pResource->GetUnderlyingResource();

            if (   pTranslationLayerResource != nullptr
                && pTranslationLayerResource->GetUnderlyingResource() != nullptr)
            {
                hr = pTranslationLayerResource->GetUnderlyingResource()->QueryInterface(riid, ppv);
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT APIENTRY AddResourceWaitsToQueue(HANDLE hResource, ID3D12CommandQueue* pCommmandQueue)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        Resource* p9on12Resource = Resource::GetResourceFromHandle(hResource);
        auto* pTranslationLayerResource = p9on12Resource->GetUnderlyingResource();
        D3DX12Residency::ManagedObject* pResidencyHandle = pTranslationLayerResource->GetResidencyHandle();

        Device* pDevice = p9on12Resource->GetParent();
        auto& ImmCtx = pDevice->GetContext();

        if (ImmCtx.IsResidencyManagementEnabled() && pResidencyHandle)
        {
            // Pin the resource while it is checked out to the caller.
            pResidencyHandle->Pin();

            // Ensure that the resource is resident after the waits on the callers queue are satisfied.
            std::unique_ptr<D3DX12Residency::ResidencySet> pResidencySet = std::unique_ptr<D3DX12Residency::ResidencySet>(
                ImmCtx.GetResidencyManager().CreateResidencySet());

            pResidencySet->Open();
            pResidencySet->Insert(pResidencyHandle);
            pResidencySet->Close();

            ImmCtx.GetResidencyManager().SubmitCommandQueueCommand(pCommmandQueue, []() {}, pResidencySet.get());
            
            // Add a deferred wait for the residency operation.  This operation is signaled on the callers queue.
            // This handles the case where caller decides to return the resource without scheduling dependent work
            // that they provide fence/value pair for.
            pTranslationLayerResource->AddFenceForUnwrapResidency(pCommmandQueue);
        }

        for (UINT i = 0; i < (UINT)D3D12TranslationLayer::COMMAND_LIST_TYPE::MAX_VALID; ++i)
        {
            auto* pCommandListManager = ImmCtx.GetCommandListManager(static_cast<D3D12TranslationLayer::COMMAND_LIST_TYPE>(i));
            if (pCommandListManager)
            {
                UINT64 WaitValue = pTranslationLayerResource->m_LastUsedCommandListID[i];

                if (WaitValue > pCommandListManager->GetCompletedFenceValue())
                {
                    auto* pFence = pCommandListManager->GetFence();
                    pCommmandQueue->Wait(pFence->Get(), WaitValue);
                    pCommandListManager->SetNeedSubmitFence();
                }
            }
        }

        
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    HRESULT APIENTRY AddDeferredWaitsToResource(HANDLE hResource, UINT NumSync, UINT64* pSignalValues, ID3D12Fence** ppFences)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        Resource* p9on12Resource = Resource::GetResourceFromHandle(hResource);
        auto* pTranslationLayerResource = p9on12Resource->GetUnderlyingResource();
        D3DX12Residency::ManagedObject* pResidencyHandle = pTranslationLayerResource->GetResidencyHandle();

        Device* pDevice = p9on12Resource->GetParent();
        auto& ImmCtx = pDevice->GetContext();

        std::vector<D3D12TranslationLayer::DeferredWait> DeferredWaits;
        DeferredWaits.reserve(NumSync); // throw( bad_alloc )

        for (UINT i = 0; i < NumSync; i++)
        {
            const D3D12TranslationLayer::DeferredWait deferredWait = 
            {
                std::make_shared<D3D12TranslationLayer::Fence>(&ImmCtx, ppFences[i]),
                pSignalValues[i]
            };
            DeferredWaits.push_back(deferredWait);
        }

        pTranslationLayerResource->AddDeferredWaits(DeferredWaits);

        if (ImmCtx.IsResidencyManagementEnabled() && pResidencyHandle)
        {
            // Transition from an explicit pin to a pin until these waits are satisfied.
            pResidencyHandle->AddPinWaits(NumSync, pSignalValues, ppFences);
            pResidencyHandle->UnPin();
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetTexture(_In_ HANDLE hDevice, _In_ UINT stage, _In_ HANDLE hTexture)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        Resource* pResource = Resource::GetResourceFromHandle(hTexture);


        if (stage == DMAP_SAMPLER || stage == D3DDMAPSAMPLER)
        {
            if (pResource)
            {
                PrintDebugMessage("Skipping SetTexture for a DMAP_SAMPLER or D3DDMAPSAMPLER");
            }
            return S_OK;
        }


        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        stage = MapSamplerStage9on12(stage);
        HRESULT hr = pDevice->GetPipelineState().GetPixelStage().SetSRV(*pDevice, pResource, stage);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    void APIENTRY SetMaximumFrameLatency(HANDLE hDevice, UINT MaxFrameLatency)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        pDevice->GetContext().m_MaxFrameLatencyHelper.SetMaximumFrameLatency(MaxFrameLatency);
        D3D9on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    BOOL APIENTRY IsMaximumFrameLatencyReached(HANDLE hDevice)
    {
        BOOL result = FALSE;
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        result = pDevice->GetContext().m_MaxFrameLatencyHelper.IsMaximumFrameLatencyReached();
        D3D9on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
        return result;
    }

    HRESULT Device::FlushWork(bool WaitOnCompletion, UINT /*FlushFlags*/)
    {
        if (WaitOnCompletion)
        {
            GetContext().WaitForCompletion(D3D12TranslationLayer::COMMAND_LIST_TYPE_ALL_MASK);
        }
        else
        {
            GetContext().Flush(D3D12TranslationLayer::COMMAND_LIST_TYPE_ALL_MASK);
        }
        return S_OK;
    }

    HRESULT Device::Init(ID3D12Device *pDevice, ID3D12CommandQueue *pCommandQueue)
    {
        HRESULT hr = S_OK;
        
        m_pDevice = pDevice;
        m_pCommandQueue = pCommandQueue;

        D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureDataD3D12 = {};
        D3D12TranslationLayer::TranslationLayerCallbacks Callbacks{};

        if (SUCCEEDED(hr))
        {
            hr = GetDevice().CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureDataD3D12, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS));
            CHECK_HR(hr);
        }

        if (SUCCEEDED(hr))
        {
            D3D12TranslationLayer::ImmediateContext::CreationArgs args = {};
            args.RequiresBufferOutOfBoundsHandling = false;
            args.CreatesAndDestroysAreMultithreaded = !RegistryConstants::g_cSingleThread;
            args.RenamingIsMultithreaded = args.CreatesAndDestroysAreMultithreaded;
            args.UseThreadpoolForPSOCreates = false;
            args.UseRoundTripPSOs = false;
            args.UseResidencyManagement = !RegistryConstants::g_cDisableMemoryManagement;
            args.DisableGPUTimeout = false;
            args.AdjustYUY2BlitCoords = m_Adapter.RequiresYUY2BlitWorkaround();
#ifdef __D3D9On12CreatorID_INTERFACE_DEFINED__
            args.CreatorID = __uuidof(D3D9On12CreatorID);
#endif
            args.MaxAllocatedUploadHeapSpacePerCommandList =
                min(RegistryConstants::g_cMaxAllocatedUploadHeapSpacePerCommandList,
                    g_AppCompatInfo.MaxAllocatedUploadHeapSpacePerCommandList);

            args.MaxSRVHeapSize = min(RegistryConstants::g_cMaxSRVHeapSize, g_AppCompatInfo.MaxSRVHeapSize);

            if (!RegistryConstants::g_cSingleThread)
            {
                m_lockedResourceRanges.InitLock();
            }
            
            m_pImmediateContext.emplace(
                0,
                FeatureDataD3D12,
                &GetDevice(),
                &GetCommandQueue(),
                Callbacks,
                0,
                args);

            D3D12TranslationLayer::SharedResourceHelpers::CreationFlags sharingFlags = {};
            m_pSharedResourceHelpers.emplace(*m_pImmediateContext, sharingFlags);

            m_pImmediateContext->SetNumScissorRects(1);
            m_pImmediateContext->SetNumViewports(1);

            hr = m_constantsManager.Init();
            CHECK_HR(hr);
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pipelineState.Init(*this);
            CHECK_HR(hr);
        }

        if (SUCCEEDED(hr))
        {
            m_Adapter.DeviceCreated(this);
        }
        return hr;
    }

    Device::Device( Adapter& Adapter, _Inout_ D3DDDIARG_CREATEDEVICE& CreateDeviceArgs ) :
        m_Adapter( Adapter ),
        m_runtimeHandle( CreateDeviceArgs.hDevice ),
        m_pUMDPresentArgs( nullptr ),
        m_pipelineStateCache( *this ),
        m_pipelineState( *this ),
        m_NodeMask( 0 ),
        m_d3d9APIVersion( CreateDeviceArgs.Version ),
        m_constantsManager( *this ),
        m_systemMemoryAllocator( *this, 32 * 1024 * 1024, 4, /*bDeferDestroyDuringRealloc*/ true ),
        m_pVideoDevice( nullptr )
    {
        memcpy( (void*)&m_Callbacks, CreateDeviceArgs.pCallbacks, sizeof( m_Callbacks ) );
        memset( m_streamFrequency, 0, sizeof( m_streamFrequency ) );

        HRESULT hr = Init( m_Adapter.GetDevice(), m_Adapter.GetCommandQueue() );

        ThrowFailure( hr );

        CreateDeviceArgs.hDevice = Device::GetHandleFromDevice( this );// in:  Runtime handle/out: Driver handle
        CreateDeviceArgs.pAllocationList = NULL;
        CreateDeviceArgs.pPatchLocationList = NULL;
        CreateDeviceArgs.CommandBuffer = 0;

        memcpy( CreateDeviceArgs.pDeviceFuncs, &g_9on12DeviceFuntions, sizeof( *CreateDeviceArgs.pDeviceFuncs ) );  // out: Driver function table

        // Marking these null informs the runtime that we don't want to participate in driver threading
        if (RegistryConstants::g_cSingleThread)
        {
            CreateDeviceArgs.pDeviceFuncs->pfnLockAsync = nullptr;
            CreateDeviceArgs.pDeviceFuncs->pfnUnlockAsync = nullptr;
            CreateDeviceArgs.pDeviceFuncs->pfnRename = nullptr;
        }
    }

    Device::~Device()
    {
        if (m_pVideoDevice)
        {
            m_pVideoDevice->~VideoDevice();
        }
    }

    HRESULT Device::Destroy()
    {
        m_constantsManager.Destroy();
        m_systemMemoryAllocator.Destroy();
        m_Adapter.DeviceDestroyed(this);

        return S_OK;
    }

    HRESULT Device::ResolveDeferredState(OffsetArg BaseVertexStart, OffsetArg BaseIndexStart)
    {
        //First resolve the pipeline state
        HRESULT hr = m_pipelineState.ResolveDeferredState(*this, BaseVertexStart, BaseIndexStart);

        return hr;
    }

    WeakHash Device::HashStreamFrequencyData(WeakHash inputHash)
    {
        return HashData(m_streamFrequency, sizeof(m_streamFrequency), inputHash);
    }

    void Device::EnsureVideoDevice()
    {
        HRESULT hr = S_OK;
        if (!m_pVideoDevice)
        {
            try
            {
                m_pVideoDevice = new (m_pVideoDeviceSpace)VideoDevice(this);
            }
            catch (_com_error& hrEx)
            {
                hr = hrEx.Error();
            }
            catch (std::bad_alloc&)
            {
                hr = E_OUTOFMEMORY;
            }
            ThrowFailure(hr);
        }
    }

    VideoDevice* Device::GetVideoDevice()
    {
        return m_pVideoDevice;
    }

    void Device::LogCreateVideoDevice( HRESULT hr )
    {
        //Do nothing
        UNREFERENCED_PARAMETER( hr );
    }

    void Device::SetDrawingPreTransformedVerts(bool preTransformedVerts)
    {
        if (preTransformedVerts != m_drawingPreTransformedVerts)
        {
            GetPipelineState().GetPixelStage().SetPreTransformedVertexMode(preTransformedVerts);
        }
    }
};