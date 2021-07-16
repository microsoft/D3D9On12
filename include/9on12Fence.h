// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Fence
    {
    public:
        Fence(Device* pDevice, UINT64 InitialValue, UINT Flags);
        Fence(Device* pDevice, HANDLE hSharedHandle);
        void Signal(UINT64 Value);
        void Wait(UINT64 Value);
        
        UINT64 GetFenceValue();

        HANDLE Share(_In_opt_ SECURITY_DESCRIPTOR* pSD);

        static FORCEINLINE HANDLE GetHandleFromFence(Fence* pFence){ return static_cast<HANDLE>(pFence); }
        static FORCEINLINE Fence* GetFenceFromHandle(HANDLE hFence){ return static_cast<Fence*>(hFence); }

        bool IsMonitored() const;

    private:
        Device* const m_pDevice;
        std::shared_ptr<D3D12TranslationLayer::Fence> m_spUnderlyingFence;
    };
}