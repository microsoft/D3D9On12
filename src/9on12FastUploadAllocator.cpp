// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    FastUploadAllocator::FastUploadAllocator(Device& device, UINT size, UINT alignment, bool bDeferDestroyDuringRealloc) :
        m_parentDevice(device),
        m_bDeferDestroyDuringRealloc(bDeferDestroyDuringRealloc),
        m_size(size),
        m_alignmentRequired(alignment),
        m_spaceUsed(size),
        m_pMappedAddress(nullptr)
    {
    }

    void FastUploadAllocator::ClearDeferredDestroyedResource()
    {
        m_pDeferredDestroyedResource.reset(nullptr);
    }

    void FastUploadAllocator::Destroy()
    {
        m_pResource.reset(nullptr);
    }

    auto FastUploadAllocator::Allocate(UINT size) -> SubBuffer
    {
        SubBuffer bufferOut = {};
        const UINT alignedSize = Align(size, m_alignmentRequired);

        if (m_spaceUsed + alignedSize > m_size)
        {
            if (size > m_size)
            {
                assert(size < MAXUINT32);
                m_size = CeilToClosestPowerOfTwo((UINT32)size);
            }

            Realloc();
        }

        bufferOut.m_pResource = m_pResource.get();
        bufferOut.m_pMappedAddress = (byte*)m_pMappedAddress + m_spaceUsed;

        auto offsetFromBase = (byte*)bufferOut.m_pMappedAddress - (byte*)m_pMappedAddress;
        assert(offsetFromBase >= 0 && offsetFromBase <= UINT_MAX);
        bufferOut.m_offsetFromBase = static_cast<UINT>(offsetFromBase);

        m_spaceUsed += alignedSize;

        assert(IsAligned(bufferOut.m_offsetFromBase, m_alignmentRequired));
        return bufferOut;
    }

    void FastUploadAllocator::Realloc()
    {
        if (m_bDeferDestroyDuringRealloc)
        {
            m_pDeferredDestroyedResource = std::move(m_pResource);
        }

        D3D12TranslationLayer::ResourceCreationArgs args = GetCreateUploadBufferArgs(&m_parentDevice.GetDevice(), m_size, m_alignmentRequired);
        m_pResource = D3D12TranslationLayer::Resource::CreateResource(&m_parentDevice.GetContext(), args, D3D12TranslationLayer::ResourceAllocationContext::ImmediateContextThreadTemporary);

        D3D12TranslationLayer::MappedSubresource MappedResult;
        m_parentDevice.GetContext().Map(m_pResource.get(), 0, D3D12TranslationLayer::MAP_TYPE_WRITE_DISCARD, false, nullptr, &MappedResult);
        m_pMappedAddress = MappedResult.pData;
        m_spaceUsed = 0;
    }

};