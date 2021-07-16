// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY SetStreamSource(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETSTREAMSOURCE *pStreamSourceArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pStreamSourceArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource *pResource = Resource::GetResourceFromHandle(pStreamSourceArg->hVertexBuffer);
        InputAssembly& ia = pDevice->GetPipelineState().GetInputAssembly();
        HRESULT hr = ia.SetVertexBuffer(*pDevice, pResource, pStreamSourceArg->Stream, pStreamSourceArg->Offset, pStreamSourceArg->Stride);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY SetStreamSourceUM(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETSTREAMSOURCEUM* pStreamSourceArg, _In_ CONST VOID* pData)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pStreamSourceArg == nullptr || pData == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        InputAssembly& ia = pDevice->GetPipelineState().GetInputAssembly();
        ia.SetVertexBufferUM(*pDevice, pStreamSourceArg->Stream, pStreamSourceArg->Stride, pData);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetIndices(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETINDICES* pSetIndicesArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pSetIndicesArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource *pResource = Resource::GetResourceFromHandle(pSetIndicesArg->hIndexBuffer);
        HRESULT hr = pDevice->GetPipelineState().GetInputAssembly().SetIndexBuffer(*pDevice, pResource, pSetIndicesArg->Stride);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY SetIndicesUM(_In_ HANDLE hDevice, _In_ UINT indexBufferStride, _In_ CONST VOID*pIndices)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = pDevice->GetPipelineState().GetInputAssembly().SetIndexBufferUM(*pDevice, indexBufferStride, pIndices);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    InputAssembly::InputAssembly(PipelineStateDirtyFlags& pipelineStateDirtyFlags, RasterStatesWrapper& rasterStates) :
        m_dirtyFlags(pipelineStateDirtyFlags),
        m_rasterStates(rasterStates),
        m_hasTLVertices(false),
        m_numBoundVBs(0),
        m_topology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED),
        m_pInputLayout(nullptr)
    {
        m_IndexBufferStack.push(InputBuffer());
    };

    void InputBuffer::InitWithUploadData(Device& /*device*/, UINT strideInBytes, CONST VOID* pDataToUpload)
    {
        *this = InputBuffer(); // Initialize defaults
        m_isSystemMemory = (pDataToUpload != nullptr);
        m_pDeferredSystemMemoryData = pDataToUpload;
        m_bufferStride = strideInBytes;
    }

    void InputBuffer::InitWithResource(Device& /*device*/, Resource* pAppResource, UINT64 bufferSize, UINT64 offset, UINT strideInBytes)
    {
        *this = InputBuffer(); // Initialize defaults
        m_sizeInBytes = static_cast<UINT>(bufferSize);
        m_bufferStride = strideInBytes;
        m_bufferOffset = (UINT)offset;
        m_pAppProvidedResource = pAppResource;

        if (pAppResource && pAppResource->IsSystemMemory())
        {
            // System memory resources must be versioned and uploaded before each draw
            m_isSystemMemory = true;
            m_pDeferredSystemMemoryData = pAppResource->GetSystemMemoryBase();
        }
        else
        {
            m_isSystemMemory = false;
        }
    }

    void InputBuffer::InitAsPersistentTriangleFan(Device & device, UINT64 bufferSize, UINT strideInBytes)
    {
        *this = InputBuffer(); // Initialize defaults

        //Check for overflow
        assert(bufferSize <= UINT_MAX);

        D3D12TranslationLayer::ResourceCreationArgs args = GetCreateUploadBufferArgs(&device.GetDevice(), (size_t)bufferSize, 0);
        m_pInternallyProvidedResource.reset(
            D3D12TranslationLayer::Resource::CreateResource(&device.GetContext(), args, D3D12TranslationLayer::ResourceAllocationContext::ImmediateContextThreadLongLived).release(),
            CleanupResourceBindingsAndRelease{});
        m_tempGPUBuffer.m_pResource = m_pInternallyProvidedResource.get();

        D3D12TranslationLayer::MappedSubresource MappedResult;
        device.GetContext().Map(m_tempGPUBuffer.m_pResource, 0, D3D12TranslationLayer::MAP_TYPE_WRITE, false, nullptr, &MappedResult);

        m_tempGPUBuffer.m_pMappedAddress = (byte*)MappedResult.pData;
        m_tempGPUBuffer.m_offsetFromBase = 0;

        m_pDeferredSystemMemoryData = m_tempGPUBuffer.m_pMappedAddress;
        m_isTriangleFanIndexBuffer = true;
        m_isSystemMemory = true;
        m_bufferStride = strideInBytes;
        m_sizeInBytes = (UINT)bufferSize;
        m_bufferOffset = 0;
    }

    D3D12TranslationLayer::Resource * InputBuffer::GetUnderlyingResource() 
    {
        if (m_isSystemMemory)
        {
            // Can be null if this VB is not referenced by the input layout and we choose
            // not to upload
            return m_tempGPUBuffer.m_pResource;
        }
        else if (m_pAppProvidedResource)
        {
            Check9on12(m_pInternallyProvidedResource == nullptr);
            return m_pAppProvidedResource->GetUnderlyingResource();
        }
        else
        {
            Check9on12(m_pInternallyProvidedResource == nullptr &&
                       m_pAppProvidedResource == nullptr &&
                       m_tempGPUBuffer.m_pResource == nullptr);
            return nullptr;
        }
    }

    UINT InputBuffer::GetOffsetInBytesD3D12()
    {         
        if (m_isSystemMemory)
        {
            return static_cast<UINT>(m_tempGPUBuffer.m_offsetFromBase);
        }
        else
        {
            return m_bufferOffset;
        }
    }

    HRESULT InputBuffer::Upload(Device& device, OffsetArg baseVertexIndex, UINT vertexCount)
    {
        Check9on12(m_isTriangleFanIndexBuffer == false);
        Check9on12(m_isSystemMemory);
        
        UINT32 drawOffset = 0;

        switch (baseVertexIndex.m_type)
        {
            case OffsetType::OFFSET_IN_BYTES:
                drawOffset = baseVertexIndex.GetOffsetInBytes();
                break;
            case OffsetType::OFFSET_IN_VERTICES:
                drawOffset = baseVertexIndex.GetOffsetInVertices() * m_bufferStride;
                break;
            case OffsetType::OFFSET_IN_INDICES:
                drawOffset = baseVertexIndex.GetOffsetInIndices() * m_bufferStride;
                break;
            default:
                Check9on12(false);
                break;
        }


        const UINT32 drawSize = vertexCount * m_bufferStride;

        m_tempGPUBuffer = device.GetSystemMemoryAllocator().Allocate(drawOffset + drawSize);

        memcpy((byte*)m_tempGPUBuffer.m_pMappedAddress + drawOffset, (byte*)GetSystemMemoryBase() + drawOffset, drawSize);

        return S_OK;
    }

    HRESULT InputAssembly::SetIndexBuffer(Device &device, Resource *pResource, UINT stride)
    {
        Resource *pPrevResource = CurrentIndexBuffer().GetAppResource();
        if (pPrevResource != pResource || stride != CurrentIndexBuffer().GetStrideInBytes())
        {
            if (pPrevResource)
            {
                pPrevResource->GetIBBindingTracker().Unbind(0);
            }
            if (pResource)
            {
                pResource->GetIBBindingTracker().Bind(0);
            }

            UINT64 bufferSize = pResource ? pResource->GetDesc().Width : 0;
            CurrentIndexBuffer().InitWithResource(device, pResource, bufferSize, 0, stride);
            m_dirtyFlags.IndexBuffer = true;
        }

        return S_OK;
    }

    HRESULT InputAssembly::SetIndexBufferUM(Device &device, UINT indexBufferStride, CONST VOID*pIndices)
    {
        Resource *pPrevResource = CurrentIndexBuffer().GetAppResource();
        if (pPrevResource)
        {
            pPrevResource->GetIBBindingTracker().Unbind(0);
        }

        CurrentIndexBuffer().InitWithUploadData(device, indexBufferStride, pIndices);
        m_dirtyFlags.IndexBuffer = true;

        return S_OK;
    }

    void InputAssembly::CreateTriangleFanIndexBuffer(_In_ Device &device, _Out_ InputBuffer& targetBuffer, _In_ UINT& indexCount)
    {
        typedef UINT32 TriFanUINT;        
        const TriFanUINT bufferSize = indexCount * sizeof(TriFanUINT);

        targetBuffer.InitAsPersistentTriangleFan(device, bufferSize, sizeof(TriFanUINT));

        // TODO: Using a UINT32 to handle all cases, but if primitive count is smaller, we can use UINT16s as an optimization
        typedef UINT32 TriFanUINT;
        TriFanUINT *pFanIndexBuffer = (TriFanUINT *)targetBuffer.GetTriangleFanMemory();

        for (UINT i = 0; i < indexCount; i += 3)
        {
            pFanIndexBuffer[i + 0] = i / 3 + 1;
            pFanIndexBuffer[i + 1] = i / 3 + 2;
            pFanIndexBuffer[i + 2] = 0;
        }
    }

    void InputAssembly::PushNewIndexBuffer(_In_ InputBuffer& targetBuffer)
    {
        m_IndexBufferStack.push(targetBuffer);
        m_dirtyFlags.IndexBuffer = true;
    }
    
    // The output variable (convertedBuffer) is initialized from scratch using the bufferSize argument(persistent allocation, deleted in resource).    
    void InputAssembly::CreateTriangleListIBFromTriangleFanIB(_In_ Device &device, _In_ CONST void* pTriFanIndexBuffer, _In_ UINT indexBufferStride, _In_ UINT indexOffsetInBytes, _In_ UINT indexCount, _Out_ InputBuffer& convertedBuffer)
    {
        void * pInputBuffer = (void*)(((BYTE*)pTriFanIndexBuffer) + indexOffsetInBytes);

        assert(pInputBuffer != nullptr);

        // TODO: Using a UINT32 to handle all cases, but if primitive count is smaller, we can use UINT16s as an optimization
        typedef UINT32 TriFanUINT;

        const TriFanUINT bufferSize = indexCount * sizeof(TriFanUINT);
        
        convertedBuffer.InitAsPersistentTriangleFan(device, bufferSize, sizeof(TriFanUINT));

        TriFanUINT *pFanIndexBuffer = (TriFanUINT *)convertedBuffer.GetTriangleFanMemory();

        for (UINT i = 0; i < indexCount; i += 3)
        {
            if (indexBufferStride == 2)
            {
                const UINT16 *pIndexBuffer16 = static_cast<const UINT16 *>(pInputBuffer);
                pFanIndexBuffer[i + 0] = static_cast<TriFanUINT>(pIndexBuffer16[i / 3 + 1]);
                pFanIndexBuffer[i + 1] = static_cast<TriFanUINT>(pIndexBuffer16[i / 3 + 2]);
                pFanIndexBuffer[i + 2] = static_cast<TriFanUINT>(pIndexBuffer16[0]);
            }
            else
            {
                Check9on12(indexBufferStride == 4);
                const UINT32 *pIndexBuffer32 = static_cast<const UINT32 *>(pInputBuffer);
                pFanIndexBuffer[i + 0] = static_cast<TriFanUINT>(pIndexBuffer32[i / 3 + 1]);
                pFanIndexBuffer[i + 1] = static_cast<TriFanUINT>(pIndexBuffer32[i / 3 + 2]);
                pFanIndexBuffer[i + 2] = static_cast<TriFanUINT>(pIndexBuffer32[0]);
            }
        }
    }    

    void InputAssembly::RestorePreviousIB(Device& /*device*/)
    {
        m_IndexBufferStack.pop();

        Check9on12(m_IndexBufferStack.size() >= 1);
        m_dirtyFlags.IndexBuffer = true;
    }

    HRESULT InputAssembly::ResolveDeferredState(Device &device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& /*psoDesc*/, OffsetArg BaseVertexStart, OffsetArg BaseIndexStart)
    {
        // The vb/ib must be re-emitted if we need to bake in the draw's byte offset into the vb/ib due to DrawPrimitive2/DrawPrimitiveIndexed2
        if (BaseVertexStart.m_type == OffsetType::OFFSET_IN_BYTES)
        {
            m_dirtyFlags.VertexBuffers |= BIT(0);
        }

        if (BaseIndexStart.m_type == OffsetType::OFFSET_IN_BYTES)
        {
            m_dirtyFlags.IndexBuffer = true;
        }

        HRESULT hr = S_OK;
        if (m_dirtyFlags.Topology)
        {
            device.GetContext().IaSetTopology(m_topology);
        }
        if (m_dirtyFlags.VertexBuffers)
        {
            ResolveVertexBuffers(device, BaseVertexStart);
        }
        if (m_dirtyFlags.IndexBuffer)
        {
            InputBuffer& ib = CurrentIndexBuffer();
            DXGI_FORMAT ibFormat = ib.GetStrideInBytes() == 0 ? DXGI_FORMAT_UNKNOWN : ConvertStrideToIndexBufferFormat(ib.GetStrideInBytes());


            // To support multithreading we must support the case when the offset passed by argument is in bytes instead of indices. Since Dx12 does not support draw offsets in bytes
            // we add the aditional offset here, and then call draw* with zero offset.
            
            // Please note that if the offset is not expressed in bytes, then aditionalOffset is zero.

            INT aditionalOffset = (BaseIndexStart.m_type == OffsetType::OFFSET_IN_BYTES) ? BaseIndexStart.GetOffsetInBytes() : 0;
            device.GetContext().IaSetIndexBuffer(ib.GetUnderlyingResource(), ibFormat, ib.GetOffsetInBytesD3D12() + aditionalOffset);
        }
        return hr;
    }

    void InputAssembly::SetVertexDeclaration(InputLayout *pInputLayout)
    { 
        if (m_pInputLayout != pInputLayout)
        {
            m_pInputLayout = pInputLayout;
            m_dirtyFlags.InputLayout = true;
            if (pInputLayout)
            {
                if (m_hasTLVertices != pInputLayout->VerticesArePreTransformed())
                {
                    m_hasTLVertices = pInputLayout->VerticesArePreTransformed();
                    m_dirtyFlags.VertexShader = true;
                }
            }

        }
    }

    void InputAssembly::ResolveVertexBuffers(Device& device, OffsetArg BaseVertexStart)
    {
        D3D12TranslationLayer::Resource * vbs[MAX_VERTEX_STREAMS] = {};
        UINT offsets[MAX_VERTEX_STREAMS] = {};
        UINT strides[MAX_VERTEX_STREAMS] = {};
        
        // To support multithreading we must support the case when the offset passed by argument is in bytes instead of vertices. Since Dx12 does not support draw offsets in bytes
        // we add the aditional offset here, and then call draw* with zero offset.
        // ie. The functions DrawIndexedPrimitive2 and DrawPrimitive2 have its offset argument expressed in bytes and it only applies to the stream# 0.
        // This means that in order to support that behaviour when we receive the offset in bytes, we'll assume that it only applies to the first stream
        // and all the other streams have zero aditional offset.

        // Please note that if the offset is not expressed in bytes, then aditionalOffset is zero and the following code is equivalent to a simple loop over the streams.

        INT aditionalOffset = (BaseVertexStart.m_type == OffsetType::OFFSET_IN_BYTES) ? BaseVertexStart.GetOffsetInBytes() : 0;
        
        size_t vbIndex = 0;
        
        vbs[vbIndex] = m_inputStreams[vbIndex].GetUnderlyingResource();
        strides[vbIndex] = m_inputStreams[vbIndex].GetStrideInBytes();
        offsets[vbIndex] = m_inputStreams[vbIndex].GetOffsetInBytesD3D12() + aditionalOffset;

        for (vbIndex = 1; vbIndex < MAX_VERTEX_STREAMS; vbIndex++)
        {
            vbs[vbIndex] = m_inputStreams[vbIndex].GetUnderlyingResource();
            strides[vbIndex] = m_inputStreams[vbIndex].GetStrideInBytes();
            offsets[vbIndex] = m_inputStreams[vbIndex].GetOffsetInBytesD3D12();
        }

        if (m_numBoundVBs == 0)
        {
            // clear out all streams
            m_numBoundVBs = MAX_VERTEX_STREAMS;
        }

        device.GetContext().IaSetVertexBuffers(0, m_numBoundVBs, vbs, strides, offsets);
    }

    bool InputAssembly::InputBufferNeedsUpload(UINT streamIndex)
    {
        Check9on12(streamIndex < MAX_VERTEX_STREAMS);
        return m_inputStreams[streamIndex].IsSystemMemory() && (m_pInputLayout->GetStreamMask() & BIT(streamIndex)) != 0;
    }

    void InputBuffer::ResetUploadedData()
    {
        if (IsSystemMemory())
        {
            m_tempGPUBuffer = {};
        }
    }


    void InputAssembly::ResetUploadBufferData()
    {
        for (UINT index = 0; index < MAX_VERTEX_STREAMS; index++)
        {
            m_inputStreams[index].ResetUploadedData();
        }
        CurrentIndexBuffer().ResetUploadedData();
    }


    HRESULT InputAssembly::UploadDeferredInputBufferData(Device& device, OffsetArg baseVertexIndex, UINT vertexCount, OffsetArg baseIndexLocation, UINT indexCount)
    {
        HRESULT hr = S_OK;

        auto& inputStream = m_inputStreams[0];

        if (InputBufferNeedsUpload(0))
        {
            hr = inputStream.Upload(device, baseVertexIndex, vertexCount);

            m_dirtyFlags.VertexBuffers |= BIT(0);
        }
        
        // If the offset was set in bytes, it only referenced the stream number zero. We can assume the other streams have zero offset.
        const OffsetArg zeroOffset = OffsetArg::AsOffsetInBytes(0);
        const OffsetArg &realOffset = (baseVertexIndex.m_type == OffsetType::OFFSET_IN_BYTES) ? zeroOffset : baseVertexIndex;

        if (SUCCEEDED(hr))
        {
            for (UINT index = 1; index < MAX_VERTEX_STREAMS; index++)
            {
                if (InputBufferNeedsUpload(index))
                {
                    hr = m_inputStreams[index].Upload(device, realOffset, vertexCount);

                    m_dirtyFlags.VertexBuffers |= BIT(index);
                }

                if (FAILED(hr))
                {
                    break;
                }

                index++;
            }
        }

        if (SUCCEEDED(hr) && indexCount > 0)
        {
            auto& currentIB = CurrentIndexBuffer();

            if (currentIB.IsSystemMemory())
            {
                // Triangle fans are uploaded on creation
                if (currentIB.IsTriangleFan() == false)
                {
                    hr = currentIB.Upload(device, baseIndexLocation, indexCount);
                }

                m_dirtyFlags.IndexBuffer = true;
            }
        }
        
        CHECK_HR(hr);
        return hr;
    }

    void InputAssembly::SetPrimitiveTopology(Device &device, D3DPRIMITIVETYPE primitiveType)
    {
        D3D12_PRIMITIVE_TOPOLOGY newTopology = ConvertToD3D12Topology(primitiveType);

        RasterizerStateID& rasterizerStateID = device.GetPipelineState().GetPixelStage().GetRasterizerStateID();

        const UINT uiTriangleMode = (primitiveType >= D3DPT_TRIANGLELIST) ? 1 : 0;
        if (rasterizerStateID.TriangleMode != uiTriangleMode)
        {
            // Update rasterizer state ID
            rasterizerStateID.TriangleMode = uiTriangleMode;
            m_dirtyFlags.RasterizerState = 1;
        }

        if (m_topology != newTopology)
        {
            m_topology = newTopology;
            device.GetPipelineState().CurrentPSODesc().PrimitiveTopologyType = ConvertToD3D12TopologyType(m_topology);
            device.GetContext().IaSetTopology(m_topology);
        }
        m_rasterStates.SetPrimitiveType(primitiveType);
    }

    void InputAssembly::SetVertexBufferUM(Device& device, UINT vbSlot, UINT stride, _In_ CONST VOID* pData)
    {
        Resource* pPrevResource = m_inputStreams[vbSlot].GetAppResource();
        if (pPrevResource)
        {
            pPrevResource->GetVBBindingTracker().Unbind(vbSlot);
        }

        m_inputStreams[vbSlot].InitWithUploadData(device, stride, pData);

        m_numBoundVBs = max(m_numBoundVBs, vbSlot + 1);

        m_dirtyFlags.VertexBuffers = true;
    }

    HRESULT InputAssembly::SetVertexBuffer(Device &device, Resource *pResource, UINT vbSlot, UINT offset, UINT stride)
    {
        HRESULT hr = S_OK;
        const UINT64 bufferSize = pResource ? pResource->GetDesc().Width : 0;

        InputBuffer &inputBuffer = m_inputStreams[vbSlot];
        Resource* pPrevResource = m_inputStreams[vbSlot].GetAppResource();
        if (pPrevResource != pResource || m_inputStreams[vbSlot].GetStrideInBytes() != stride || inputBuffer.GetOffsetInBytesAPI() != offset)
        {
            if (pPrevResource)
            {
                pPrevResource->GetVBBindingTracker().Unbind(vbSlot);
            }

            if (pResource)
            {
                pResource->GetVBBindingTracker().Bind(vbSlot);
                m_numBoundVBs = max(m_numBoundVBs, vbSlot + 1);
            }
            // If they're unbinding this VB
            else
            {
                // If they're removing the last VB, 
                if (m_numBoundVBs - 1 == vbSlot)
                {
                    do
                    {
                        m_numBoundVBs--;

                    } while (m_numBoundVBs > 0 && m_inputStreams[m_numBoundVBs - 1].GetUnderlyingResource() == nullptr);
                }
            }

            m_inputStreams[vbSlot].InitWithResource(device, pResource, bufferSize, offset, stride);
            m_dirtyFlags.VertexBuffers = true;
        }

        CHECK_HR(hr);
        return hr;
    }
};