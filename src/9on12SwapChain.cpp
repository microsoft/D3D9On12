// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"
#include "SwapChainHelper.hpp"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY Present1(_In_ HANDLE hDevice, _In_ D3DDDIARG_PRESENT1* pPresentArgs)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);

        pDevice->m_pUMDPresentArgs = pPresentArgs;

        // The runtime and 9on12's KmdPresent will handle filling this out with meaningful data,
        // we need to go through the callback because it's the only way to get access to the runtime's
        // D3DKMT_PRESENT data
        D3DDDICB_PRESENT callbackArgs = {};
        pDevice->GetRuntimeCallbacks().pfnPresentCb(pDevice->GetRuntimeHandle(), &callbackArgs);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY KmdPresent(_In_ HANDLE hDevice, D3DKMT_PRESENT *pKMTArgs)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pDevice->m_pUMDPresentArgs == nullptr || pKMTArgs == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        
        HRESULT hr = pDevice->Present(*pDevice->m_pUMDPresentArgs, pKMTArgs);

        // This pointer becomes invalid after present since it's owned by the runtime
        pDevice->m_pUMDPresentArgs = nullptr;
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY CloseAndSubmitGraphicsCommandListForPresent(
        _In_ HANDLE hDevice,
        BOOL commandsAdded,
        _In_reads_(numSrcSurfaces) const D3DDDIARG_PRESENTSURFACE* pSrcSurfaces,
        UINT numSrcSurfaces,
        _In_opt_ HANDLE hDestResource,
        _In_ D3DKMT_PRESENT* pKMTPresent)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = pDevice->CloseAndSubmitGraphicsCommandListForPresent(commandsAdded, pSrcSurfaces, numSrcSurfaces, hDestResource, pKMTPresent);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT Device::Present(CONST D3DDDIARG_PRESENT1& PresentArgs, D3DKMT_PRESENT* pKMTArgs)
    {
        Check9on12(PresentArgs.SrcResources == 1);

        Resource* pSource = Resource::GetResourceFromHandle(PresentArgs.phSrcResources[0].hResource);
        Resource* pDest = Resource::GetResourceFromHandle(PresentArgs.hDstResource);
        if (pSource)
        {
            const D3DDDIARG_PRESENTSURFACE* presentSurfaces = PresentArgs.phSrcResources;
            bool bPresentWithSystemMemory = pSource->IsSystemMemory();
            bool sourceIsCompatibleWithPresent = !bPresentWithSystemMemory && !pSource->NeedsSwapRBOutputChannels();

            if (pDest)
            {
                // We can't use a typeless or msaa resources when doing a blt-model present.
                // We handle this by copying it over to a backing resource with a 
                // single-count and fully qualified format.
                sourceIsCompatibleWithPresent = sourceIsCompatibleWithPresent && IsCompatibleAsSourceForBltPresent(pSource->GetDesc());
            }
            else
            {
                sourceIsCompatibleWithPresent = sourceIsCompatibleWithPresent && !IsTypelessFormat(pSource->GetDesc().Format);
            }
            
            if (!sourceIsCompatibleWithPresent)
            {
                pSource = pSource->GetBackingPlainResource( true );
                // Need to reference the new resource in present surface 0
                m_d3d9PresentSurfaces.clear();
                for (UINT i = 0; i < PresentArgs.SrcResources; i++)
                {
                    m_d3d9PresentSurfaces.push_back(PresentArgs.phSrcResources[i]);
                }
                m_d3d9PresentSurfaces[0].hResource = Resource::GetHandleFromResource(pSource);
                presentSurfaces = m_d3d9PresentSurfaces.data();
            }

            HRESULT hr = E_FAIL;
            if (GetAdapter().m_bSupportsNewPresent)
            {          
                try
                {
                    auto forwardToPresent9On12CB = [adapter = GetAdapter(), runtimeDeviceHandle = GetRuntimeHandle(), pArgs = PresentArgs, presentSurfaces]
                    (D3D12TranslationLayer::PresentCBArgs& args) {
                        D3D9ON12_PRESENTCB_ARGS cbArgs = {};
                        cbArgs.flipInterval = args.flipInterval;
                        cbArgs.pSrcSurfaces = presentSurfaces;
                        cbArgs.numSrcSurfaces = args.numSrcSurfaces;
                        cbArgs.hDestResource = pArgs.hDstResource;
                        cbArgs.pGraphicsCommandList = args.pGraphicsCommandList;
                        cbArgs.pGraphicsCommandQueue = args.pGraphicsCommandQueue;
                        cbArgs.pKMTPresent = args.pKMTPresent;
                        cbArgs.vidPnSourceId = args.vidPnSourceId;
                        return (*adapter.m_privateCallbacks.pfnPresentCB)(runtimeDeviceHandle, &cbArgs);
                    };

                    // Convert present Surfaces into d3d12TranslationLayer format
                    m_d3d12tlPresentSurfaces.clear();
                    for (UINT i = 0; i < PresentArgs.SrcResources; i++)
                    {
                        auto presentSurface = presentSurfaces[i];
                        auto resource = Resource::GetResourceFromHandle(presentSurface.hResource);
                        D3D12TranslationLayer::PresentSurface surface = D3D12TranslationLayer::PresentSurface(resource->GetUnderlyingResource(), presentSurface.SubResourceIndex);
                        m_d3d12tlPresentSurfaces.push_back(surface);
                    }

                    GetContext().Present(
                        m_d3d12tlPresentSurfaces.data(),
                        PresentArgs.SrcResources,
                        pDest ? pDest->GetUnderlyingResource() : nullptr,
                        PresentArgs.FlipInterval,
                        pSource->GetVidPnSourceId(),
                        pKMTArgs,
                        false,
                        forwardToPresent9On12CB);

                    hr = S_OK;
                }
                catch (_com_error& hrEx)
                {
                    hr = hrEx.Error();
                }
                catch (std::bad_alloc&)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                // Fallback to old present path
                auto swapChainManager = GetSwapChainManager();
                D3D12TranslationLayer::Resource* pSrcTranslationLayerResource = pSource->GetUnderlyingResource();

                auto pSwapChain = swapChainManager->GetSwapChainForWindow(pKMTArgs->hWindow, *pSrcTranslationLayerResource);
                auto swapChainHelper = D3D12TranslationLayer::SwapChainHelper(pSwapChain);
                GetContext().m_MaxFrameLatencyHelper.WaitForMaximumFrameLatency();

                hr = swapChainHelper.StandardPresent(GetContext(), pKMTArgs, *pSrcTranslationLayerResource);
            }
            
            m_lockedResourceRanges.GetLocked()->clear();

            return hr;
        }
        else
        {
            return E_INVALIDARG;
        }
    }

    std::shared_ptr<D3D12TranslationLayer::SwapChainManager> Device::GetSwapChainManager()
    {
        std::lock_guard lock( m_SwapChainManagerMutex );
        if (!m_SwapChainManager)
        {
            m_SwapChainManager = std::make_shared<D3D12TranslationLayer::SwapChainManager>( GetContext() );
        }
        return m_SwapChainManager;
    }

    HRESULT Device::CloseAndSubmitGraphicsCommandListForPresent(
        BOOL commandsAdded,
        _In_reads_(numSrcSurfaces) const D3DDDIARG_PRESENTSURFACE* pSrcSurfaces,
        UINT numSrcSurfaces,
        _In_opt_ HANDLE hDestResource,
        _In_ D3DKMT_PRESENT* pKMTPresent)
    {
        D3D12TranslationLayer::ImmediateContext& immediateContext = GetContext();
        m_d3d12tlPresentSurfaces.clear();
        for (UINT i = 0; i < numSrcSurfaces; i++)
        {
            auto presentSurface = pSrcSurfaces[i];
            auto resource = Resource::GetResourceFromHandle(presentSurface.hResource);
            D3D12TranslationLayer::PresentSurface surface = D3D12TranslationLayer::PresentSurface(resource->GetUnderlyingResource(), presentSurface.SubResourceIndex);
            m_d3d12tlPresentSurfaces.push_back(surface);
        }

        D3D12TranslationLayer::Resource* pD3d12tlDestResource = nullptr;
        if (hDestResource != NULL)
        {
            pD3d12tlDestResource = Resource::GetResourceFromHandle(hDestResource)->GetUnderlyingResource();
        }
        return immediateContext.CloseAndSubmitGraphicsCommandListForPresent(
            commandsAdded,
            m_d3d12tlPresentSurfaces.data(),
            numSrcSurfaces,
            pD3d12tlDestResource,
            pKMTPresent);
    }
};