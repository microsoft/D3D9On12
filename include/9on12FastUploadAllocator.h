// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Device;

    class FastUploadAllocator
    {
    public:
        struct SubBuffer
        {
            SubBuffer() :
                m_pResource(0),
                m_pMappedAddress(nullptr),
                m_offsetFromBase(0){};

            SubBuffer(D3D12TranslationLayer::Resource *pResource, void* mapped, UINT offset) :
                m_pResource(pResource), m_pMappedAddress(mapped), m_offsetFromBase(offset){};

            D3D12TranslationLayer::Resource *m_pResource;
            void* m_pMappedAddress;
            UINT m_offsetFromBase;
        };

        FastUploadAllocator(Device& device, UINT size, UINT alignment, bool bDeferDestroyDuringRealloc = false);
        ~FastUploadAllocator() = default;

        SubBuffer Allocate(UINT size);
        void Destroy();
        void ClearDeferredDestroyedResource();

    private:
        void Realloc();

        Device& m_parentDevice;
        const bool m_bDeferDestroyDuringRealloc;
        UINT m_size;
        const UINT m_alignmentRequired;
        UINT m_spaceUsed;

        unique_unbind_resourceptr m_pResource;
        void* m_pMappedAddress;

        unique_unbind_resourceptr m_pDeferredDestroyedResource;
    };
};