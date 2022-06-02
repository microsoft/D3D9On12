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

    HRESULT Device::Present(CONST D3DDDIARG_PRESENT1& PresentArgs, D3DKMT_PRESENT* pKMTArgs)
    {
        Check9on12(PresentArgs.SrcResources == 1);

        Resource* pSource = Resource::GetResourceFromHandle(PresentArgs.phSrcResources[0].hResource);
        if (pSource)
        {
            bool bPresentWithSystemMemory = pSource->IsSystemMemory();
            bool sourceIsCompatibleWithPresent = !bPresentWithSystemMemory && !pSource->NeedsSwapRBOutputChannels();
            sourceIsCompatibleWithPresent = sourceIsCompatibleWithPresent && !IsTypelessFormat( pSource->GetDesc().Format );

            if (!sourceIsCompatibleWithPresent)
            {
                pSource = pSource->GetBackingPlainResource( true );
            }
            auto swapChainManager = GetSwapChainManager();
            D3D12TranslationLayer::Resource* pSrcTranslationLayerResource = pSource->GetUnderlyingResource();

            auto pSwapChain = swapChainManager->GetSwapChainForWindow( pKMTArgs->hWindow, *pSrcTranslationLayerResource );
            auto swapChainHelper = D3D12TranslationLayer::SwapChainHelper( pSwapChain );
            GetContext().m_MaxFrameLatencyHelper.WaitForMaximumFrameLatency();

            auto ret = swapChainHelper.StandardPresent( GetContext(), pKMTArgs, *pSrcTranslationLayerResource );

            m_lockedResourceRanges.GetLocked()->clear();

            return ret;
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
};