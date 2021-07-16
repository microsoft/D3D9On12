// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY ShareFence(HANDLE hFence, _In_opt_ SECURITY_DESCRIPTOR* pSD, _Out_ HANDLE* phSharedHandle)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Fence* pFence = Fence::GetFenceFromHandle(hFence);
        if (pFence == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        *phSharedHandle = pFence->Share(pSD);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY WaitForFence(HANDLE hFence, UINT64 NewFenceValue)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Fence* pFence = Fence::GetFenceFromHandle(hFence);
        if (pFence == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pFence->Wait(NewFenceValue);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SignalFence(HANDLE hFence, UINT64 NewFenceValue)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Fence* pFence = Fence::GetFenceFromHandle(hFence);
        if (pFence == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pFence->Signal(NewFenceValue);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    void APIENTRY DestroyTrackedFence(HANDLE hFence)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Fence* pFence = Fence::GetFenceFromHandle(hFence);
        if (pFence == nullptr)
        {
            return;
        }
        
        delete pFence;
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
    }

    UINT64 APIENTRY GetCompletedFenceValue(HANDLE hFence)
    {
        UINT64 result = 0;
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Fence* pFence = Fence::GetFenceFromHandle(hFence);
        if (pFence != nullptr)
        {
            result = pFence->GetFenceValue();
        }
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
        return result;
    }

    Fence::Fence(Device* pDevice, UINT64 InitialValue, UINT Flags)
        : m_pDevice(pDevice)
    {
        D3D12TranslationLayer::FENCE_FLAGS TranslationFlags = D3D12TranslationLayer::FENCE_FLAG_SHARED;
        if (Flags & D3D9ON12USAGE_CROSSADAPTER)
        {
            TranslationFlags |= D3D12TranslationLayer::FENCE_FLAG_SHARED_CROSS_ADAPTER;
        }
        if (Flags & D3D9ON12USAGE_DEFERREDFENCEWAITS)
        {
            TranslationFlags |= D3D12TranslationLayer::FENCE_FLAG_DEFERRED_WAITS;
        }
        if ((Flags & D3D9ON12USAGE_MONITOREDFENCE) == 0)
        {
            TranslationFlags |= D3D12TranslationLayer::FENCE_FLAG_NON_MONITORED;
        }

        m_spUnderlyingFence = std::make_shared<D3D12TranslationLayer::Fence>(&m_pDevice->GetContext(), TranslationFlags, InitialValue);
    }

    Fence::Fence(Device* pDevice, HANDLE hSharedHandle)
        : m_pDevice(pDevice)
    {
        m_spUnderlyingFence = std::make_shared<D3D12TranslationLayer::Fence>(&m_pDevice->GetContext(), hSharedHandle);
    }

    UINT64 Fence::GetFenceValue()
    {
        return m_spUnderlyingFence->GetCompletedValue();
    }


    void Fence::Signal(UINT64 Value)
    {
        m_pDevice->GetContext().Signal(m_spUnderlyingFence.get(), Value);
    }

    void Fence::Wait(UINT64 Value)
    {
        m_pDevice->GetContext().Wait(m_spUnderlyingFence, Value);
    }

    HANDLE Fence::Share(_In_opt_ SECURITY_DESCRIPTOR* pSD)
    {
        DWORD dwAccess = GENERIC_ALL;
        SECURITY_ATTRIBUTES securityAttributes = ConvertSecurityDescriptorToSecurityAttributes(pSD);

        HANDLE hSharedHandle = nullptr;
        ThrowFailure(m_spUnderlyingFence->CreateSharedHandle(&securityAttributes, dwAccess, nullptr, &hSharedHandle));
        return hSharedHandle;
    }

    bool Fence::IsMonitored() const
    {
        return m_spUnderlyingFence->IsMonitored();
    }
}