// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12::Test 
{
    class FastUploadAllocatorMock : public FastUploadAllocator
    {
    public:
        FastUploadAllocatorMock(Device& device) : FastUploadAllocator(device, 0, 0, false) {}
        FastUploadAllocatorMock(Device& device, UINT size, UINT alignment, bool bDeferDestroyDuringRealloc = false) :
            FastUploadAllocator(device, size, alignment, bDeferDestroyDuringRealloc) {}
    };

    class FastUploadAllocatorFactoryFake : public FastUploadAllocatorFactory
    {
    public:
        FastUploadAllocator Create(Device& device, UINT size, UINT alignment, bool bDeferDestroyDuringRealloc = false) 
        {
            return FastUploadAllocator(device, size, alignment, bDeferDestroyDuringRealloc);
        }
    };
}