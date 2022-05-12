// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// D3D9On12 draw inlines
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace D3D9on12
{
    static HRESULT DrawProlog(Device& device, OffsetArg BaseVertexStart, UINT vertexCount, OffsetArg baseIndexLocation, UINT indexCount, D3DPRIMITIVETYPE primitiveType, UINT& instancesToDraw, bool &skipDraw)
    {

        if ((device.GetPipelineState().GetPixelStage().GetNumBoundRenderTargets() == 0 && device.GetPipelineState().GetPixelStage().GetDepthStencil() == nullptr) ||
            ConvertToD3D12Topology(primitiveType) == (D3D_PRIMITIVE_TOPOLOGY)-1 ||
            vertexCount == 0)
        {
            skipDraw = true;
            return S_OK;
        }
        skipDraw = false;

        InputAssembly& ia = device.GetPipelineState().GetInputAssembly();
        HRESULT hr = ia.UploadDeferredInputBufferData(device, BaseVertexStart, vertexCount, baseIndexLocation, indexCount);

        CHECK_HR(hr);
        if (SUCCEEDED(hr))
        {
            ia.SetPrimitiveTopology(device, primitiveType);
            device.GetPipelineState().MarkPipelineStateNeeded();

            hr = device.ResolveDeferredState(BaseVertexStart, baseIndexLocation);
            CHECK_HR(hr);
        }

        // The instance count is always in stream 0
        UINT streamFrequence = device.GetStreamFrequency(0);

        //This is how D3D9 conveyed instance count
        instancesToDraw = (streamFrequence & D3DSTREAMSOURCE_INDEXEDDATA) ? streamFrequence &~D3DSTREAMSOURCE_INDEXEDDATA : 1;

        return hr;
    }

    static HRESULT DrawEpilogue(Device& device)
    {
        // Data uploaded for a draw is only valid for that draw, and should be cleaned up immediately after to prevent
        // successive draws from reading stale/deleted data
        device.GetSystemMemoryAllocator().ClearDeferredDestroyedResource();
        device.GetPipelineState().GetInputAssembly().ResetUploadBufferData();
        return S_OK;
    }


    inline _Check_return_ HRESULT APIENTRY DrawPrimitive(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWPRIMITIVE* pDrawPrimitiveArg, _In_opt_ CONST UINT* pFlagBuffer)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        Device* pDevice = Device::GetDeviceFromHandle(hDevice);

        if (pDevice == nullptr || pDrawPrimitiveArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->SetDrawingPreTransformedVerts(false);
        OffsetArg baseVertexOffset = OffsetArg::AsOffsetInVertices(pDrawPrimitiveArg->VStart);
        OffsetArg baseIndexOffset = OffsetArg::AsOffsetInIndices(0);

        if (IsTriangleFan(pDrawPrimitiveArg->PrimitiveType))
        {
            if (pFlagBuffer)
            {
                return pDevice->DrawWireframeTriangleFanWithEdgeFlags(baseVertexOffset, pDrawPrimitiveArg->PrimitiveCount, *pFlagBuffer);
            }
            return pDevice->DrawTriangleFan(baseVertexOffset, pDrawPrimitiveArg->PrimitiveCount);
        }

        UINT instanceCount = 1;
        const UINT verticesToDraw = CalcVertexCount(pDrawPrimitiveArg->PrimitiveType, pDrawPrimitiveArg->PrimitiveCount);
        const UINT vertexCount = verticesToDraw;
        const UINT indexCount = 0;    

        bool skipDraw = false;
        HRESULT hr = D3D9on12::DrawProlog(*pDevice, baseVertexOffset, vertexCount, baseIndexOffset, indexCount, pDrawPrimitiveArg->PrimitiveType, instanceCount, skipDraw);
        CHECK_HR(hr);

        if (skipDraw)
        {
            return hr;
        }

        if (SUCCEEDED(hr))
        {
            pDevice->GetContext().DrawInstanced(
                CalcVertexCount(pDrawPrimitiveArg->PrimitiveType, pDrawPrimitiveArg->PrimitiveCount),
                instanceCount,
                baseVertexOffset.GetOffsetInVertices(),
                0);

        }

        hr = DrawEpilogue(*pDevice);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    inline _Check_return_ HRESULT APIENTRY DrawPrimitive2(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWPRIMITIVE2* pDrawPrimitiveArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pDrawPrimitiveArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        UINT const stream0Stride = pDevice->GetPipelineState().GetInputAssembly().GetStream0Stride();
        Check9on12(stream0Stride > 0);

        pDevice->SetDrawingPreTransformedVerts(true);
        OffsetArg baseVertexOffset = OffsetArg::AsOffsetInBytes(pDrawPrimitiveArg->FirstVertexOffset);
        OffsetArg baseIndexOffset = OffsetArg::AsOffsetInIndices(0);

        if (IsTriangleFan(pDrawPrimitiveArg->PrimitiveType))
        {
            return pDevice->DrawTriangleFan(baseVertexOffset, pDrawPrimitiveArg->PrimitiveCount);
        }

        UINT instanceCount = 1;
        const UINT vertexCount = CalcVertexCount(pDrawPrimitiveArg->PrimitiveType, pDrawPrimitiveArg->PrimitiveCount);

        bool skipDraw;
        HRESULT hr = DrawProlog(*pDevice, baseVertexOffset, vertexCount, baseIndexOffset, 0, pDrawPrimitiveArg->PrimitiveType, instanceCount, skipDraw);
        CHECK_HR(hr);
        if (skipDraw)
        {
            return hr;
        }

        if (SUCCEEDED(hr))
        {
            pDevice->GetContext().DrawInstanced(vertexCount,
                instanceCount,
                0,// offset will be added in the vertex buffer resolve
                0);
        }

        hr = DrawEpilogue(*pDevice);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    inline _Check_return_ HRESULT APIENTRY DrawIndexedPrimitive2(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWINDEXEDPRIMITIVE2* pData, _In_ UINT dwIndicesSize, _In_ CONST VOID* pIndexBuffer, _In_opt_ CONST UINT* pFlagBuffer)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        if (pFlagBuffer != nullptr)
        {
            Check9on12(false);
            return E_NOTIMPL;
        }
        
        UINT const stream0Stride = pDevice->GetPipelineState().GetInputAssembly().GetStream0Stride();
        Check9on12(stream0Stride > 0);

        pDevice->SetDrawingPreTransformedVerts(true);
        bool bNegativeVertexOffset = pData->BaseVertexOffset < 0;
        OffsetArg baseVertexOffset = OffsetArg::AsOffsetInBytes(pData->BaseVertexOffset);
        OffsetArg drawOffset = OffsetArg::AsOffsetInVertices(0);

        // This is a corner case that gets hit when an app opts into software vertex processing. In these cases, the runtime
        // is always generating a new vertex buffer on-the-fly. As an optimization, if a MinIndex is passed into the API, the 
        // D3D9 runtime generates a smaller vertex buffer that skips over the vertices below the MinIndex. This 
        // leads to the current index buffer indices being incorrectly offset. Rather than generate a new index buffer, the runtime 
        // passes a negative vertex offset to make sure indices are correctly offset to the right vertices.
        if (bNegativeVertexOffset)
        {
            Check9on12((pData->BaseVertexOffset % (INT)stream0Stride == 0));
            Check9on12(pDevice->GetPipelineState().GetInputAssembly().GetNumBoundStreams() == 1);
            baseVertexOffset = OffsetArg::AsOffsetInBytes(0);
            drawOffset = OffsetArg::AsOffsetInVertices(pData->BaseVertexOffset / (INT)stream0Stride);
        }
        OffsetArg baseIndexOffset = OffsetArg::AsOffsetInBytes(pData->StartIndexOffset);

        HRESULT hr = pDevice->GetPipelineState().GetInputAssembly().SetIndexBufferUM(*pDevice, dwIndicesSize, pIndexBuffer);
        CHECK_HR(hr);
        
        if (SUCCEEDED(hr))
        {
            if (pData->PrimitiveType == D3DPT_TRIANGLEFAN)
            {
                return pDevice->DrawTriangleFanIndexed(baseVertexOffset, pData->NumVertices, baseIndexOffset, pData->PrimitiveCount);
            }

            UINT instanceCount = 1;
            const UINT indexCount = CalcVertexCount(pData->PrimitiveType, pData->PrimitiveCount);
            bool skipDraw;
            hr = DrawProlog(*pDevice, baseVertexOffset, pData->NumVertices, baseIndexOffset, indexCount, pData->PrimitiveType, instanceCount, skipDraw);
            CHECK_HR(hr);
            if (skipDraw)
            {
                return hr;
            }

            if (SUCCEEDED(hr))
            {
                pDevice->GetContext().DrawIndexedInstanced(indexCount,
                    instanceCount,
                    0,// offset will be added in the index buffer resolve,
                    drawOffset.GetOffsetInVertices(),
                    0);
            }
        }

        hr = DrawEpilogue(*pDevice);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    inline HRESULT Device::DrawTriangleFan(_In_ OffsetArg baseVertex, _In_ UINT primitiveCount)
    {
        // Update index count
        UINT indexCount = CalcVertexCount(D3DPT_TRIANGLELIST, primitiveCount);

        if (m_triangleFanIndexBufferCache.isTriFanCacheInitialized)
        {
            if (primitiveCount > m_triangleFanIndexBufferCache.highestPrimitiveCountTriFan)
            {
                // We have to reallocate a new m_highestPCIndexBufferTriFan for the new primitiveCount, 
                // release the old one and update the cache variables

                GetContext().ClearInputBindings(m_triangleFanIndexBufferCache.highestPCIndexBufferTriFan.GetUnderlyingResource());

                m_triangleFanIndexBufferCache.highestPrimitiveCountTriFan = primitiveCount;

                D3D9on12::InputAssembly::CreateTriangleFanIndexBuffer(*this, m_triangleFanIndexBufferCache.highestPCIndexBufferTriFan, //InputBuffer& targetBuffer
                    indexCount // _In_ UINT& indexCount
                    );
            }
            // Otherwise it has to be primitiveCount <= m_highestPrimitiveCountTriFan
            // If current primitiveCount is less or equal than m_highestPrimitiveCountTriFan 
            // then we can use the IB stored in m_highestPCIndexBufferTriFan and the indexCount will autoadjust from primitiveCount
            // Do nothing, by default we'll load m_highestPCIndexBufferTriFan into the index buffers stack
            // The indexCount will autoadjust with the primitiveCount received and we can use a subset of the currently stored indexbuffer                  
        }
        else
        {
            // Initialization logic

            m_triangleFanIndexBufferCache.highestPrimitiveCountTriFan = primitiveCount;

            D3D9on12::InputAssembly::CreateTriangleFanIndexBuffer(*this, m_triangleFanIndexBufferCache.highestPCIndexBufferTriFan, //InputBuffer& targetBuffer
                indexCount // _In_ UINT& indexCount
                );

            m_triangleFanIndexBufferCache.isTriFanCacheInitialized = true;
        }

        GetPipelineState().GetInputAssembly().PushNewIndexBuffer(m_triangleFanIndexBufferCache.highestPCIndexBufferTriFan);

        //Draw logic

        UINT instanceCount = 1;

        bool skipDraw;
        OffsetArg baseIndexOffset = OffsetArg::AsOffsetInIndices(0);
        HRESULT hr = DrawProlog(*this, baseVertex, CalcVertexCount(D3DPT_TRIANGLEFAN, primitiveCount), baseIndexOffset, indexCount, D3DPT_TRIANGLEFAN, instanceCount, skipDraw);
        CHECK_HR(hr);

        if (!skipDraw && SUCCEEDED(hr))
        {
            INT baseVertexVal = 0;
            switch (baseVertex.m_type)
            {
            case OffsetType::OFFSET_IN_BYTES:
                baseVertexVal = 0;// offset will be added in the vertex buffer resolve
                break;
            case OffsetType::OFFSET_IN_VERTICES:
                baseVertexVal = baseVertex.GetOffsetInVertices();
                break;
            default:
                Check9on12(false);
                break;
            }

            GetContext().DrawIndexedInstanced(indexCount,
                instanceCount,
                baseIndexOffset.GetOffsetInIndices(),
                baseVertexVal,
                0);
        }

        if (SUCCEEDED(hr))
        {
            GetPipelineState().GetInputAssembly().RestorePreviousIB(*this);
            hr = DrawEpilogue(*this);
        }

        return hr;
    }

    inline HRESULT Device::DrawWireframeTriangleFanWithEdgeFlags(_In_ OffsetArg baseVertex, _In_ UINT primitiveCount, _In_ UINT edgeFlags)
    {
        UINT maxIndexCount = primitiveCount * 6; // 3 edges per triangle, 2 indices per edge
        InputBuffer indexBuffer;
        indexBuffer.InitAsPersistentTriangleFan(*this, maxIndexCount * sizeof(UINT), sizeof(UINT));
        auto pIndices = reinterpret_cast<UINT*>(indexBuffer.GetTriangleFanMemory());

        UINT indexCount = 0;
        for (UINT primitive = 0; primitive < primitiveCount; ++primitive)
        {
            UINT primitiveEdgeFlags = edgeFlags >> primitive;
            if (primitive == 0 && (primitiveEdgeFlags & 1))
            {
                pIndices[indexCount++] = 0;
                pIndices[indexCount++] = 1;
            }
            if (primitiveEdgeFlags & 2)
            {
                pIndices[indexCount++] = primitive + 1;
                pIndices[indexCount++] = primitive + 2;
            }
            if (primitive == primitiveCount - 1 && (primitiveEdgeFlags & 4))
            {
                pIndices[indexCount++] = primitive + 2;
                pIndices[indexCount++] = 0;
            }
        }

        GetPipelineState().GetInputAssembly().PushNewIndexBuffer(indexBuffer);

        //Draw logic

        UINT instanceCount = 1;

        bool skipDraw;
        OffsetArg baseIndexOffset = OffsetArg::AsOffsetInIndices(0);
        HRESULT hr = DrawProlog(*this, baseVertex, maxIndexCount, baseIndexOffset, indexCount, D3DPT_LINELIST, instanceCount, skipDraw);
        CHECK_HR(hr);

        if (!skipDraw && SUCCEEDED(hr))
        {
            INT baseVertexVal = 0;
            switch (baseVertex.m_type)
            {
            case OffsetType::OFFSET_IN_BYTES:
                baseVertexVal = 0;// offset will be added in the vertex buffer resolve
                break;
            case OffsetType::OFFSET_IN_VERTICES:
                baseVertexVal = baseVertex.GetOffsetInVertices();
                break;
            default:
                Check9on12(false);
                break;
            }

            GetContext().DrawIndexedInstanced(indexCount,
                instanceCount,
                baseIndexOffset.GetOffsetInIndices(),
                baseVertexVal,
                0);
        }

        if (SUCCEEDED(hr))
        {
            GetPipelineState().GetInputAssembly().RestorePreviousIB(*this);
            hr = DrawEpilogue(*this);
        }

        return hr;
    }

    inline HRESULT Device::DrawTriangleFanIndexed(_In_ OffsetArg baseVertex, _In_ UINT vertexCount, _In_ OffsetArg baseIndexLocation, _In_ UINT primitiveCount)
    {
        //Update index count
        UINT indexCount = CalcVertexCount(D3DPT_TRIANGLELIST, primitiveCount);

        D3D9on12::InputAssembly& inputAssembly = GetPipelineState().GetInputAssembly();
        InputBuffer &currentIndexBuffer = inputAssembly.CurrentIndexBuffer();
        void *pSrcIndexBuffer = nullptr;
        UINT stride = currentIndexBuffer.GetStrideInBytes();
        InputBuffer convertedIB;

        UINT indexOffsetInBytes;
        if (baseIndexLocation.m_type == OffsetType::OFFSET_IN_INDICES)
        {
            indexOffsetInBytes = baseIndexLocation.GetOffsetInIndices() * stride;
        }
        else
        {
            indexOffsetInBytes = baseIndexLocation.GetOffsetInBytes();
        }

        if (currentIndexBuffer.IsSystemMemory())
        {
            pSrcIndexBuffer = currentIndexBuffer.GetSystemMemoryBase();
            stride = currentIndexBuffer.GetStrideInBytes();

            D3D9on12::InputAssembly::CreateTriangleListIBFromTriangleFanIB(*this, pSrcIndexBuffer, stride, indexOffsetInBytes, indexCount, convertedIB);
        }
        else
        {
            // We would like to avoid accessing the GPU index buffer whenever possible because doing so causes a WaitOnCompletion(true) to be called.
            // There's a cached version of the last converted to triangle list index buffer read from this resource stored in the resource
            D3D9on12::Resource* appRes = currentIndexBuffer.GetAppResource();
            Check9on12(appRes);

            appRes->GetTriFanIB(convertedIB, indexOffsetInBytes, indexCount, currentIndexBuffer.GetStrideInBytes());
        }

        // Set the converted or cached index buffer in the stack
        inputAssembly.PushNewIndexBuffer(convertedIB);

        // Draw logic
        UINT instanceCount = 1;
        bool skipDraw;
        HRESULT hr = DrawProlog(*this, baseVertex, vertexCount, baseIndexLocation, indexCount, D3DPT_TRIANGLEFAN, instanceCount, skipDraw);
        CHECK_HR(hr);

        if (!skipDraw && SUCCEEDED(hr))
        {
            UINT const stream0Stride = GetPipelineState().GetInputAssembly().GetStream0Stride();
            Check9on12(stream0Stride > 0);

            UINT baseVertexVal = 0;
            switch (baseVertex.m_type)
            {
            case OffsetType::OFFSET_IN_BYTES:
                baseVertexVal = 0;// offset will be added in the vertex buffer resolve
                break;
            case OffsetType::OFFSET_IN_VERTICES:
                baseVertexVal = baseVertex.GetOffsetInVertices();
                break;
            default:
                Check9on12(false);
                break;
            }

            GetContext().DrawIndexedInstanced(indexCount,
                instanceCount,
                0,
                baseVertexVal,
                0);
        }

        if (SUCCEEDED(hr))
        {
            GetPipelineState().GetInputAssembly().RestorePreviousIB(*this);

            hr = DrawEpilogue(*this);
        }

        return hr;
    }

    inline _Check_return_ HRESULT APIENTRY DrawIndexedPrimitive(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWINDEXEDPRIMITIVE* pDrawPrimitiveArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pDrawPrimitiveArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        UINT instanceCount = 1;

        UINT IndexCountPerInstance = CalcVertexCount(pDrawPrimitiveArg->PrimitiveType, pDrawPrimitiveArg->PrimitiveCount);
        UINT StartInstanceLocation = 0;
        const UINT vertexCount = pDrawPrimitiveArg->MinIndex + pDrawPrimitiveArg->NumVertices;

        pDevice->SetDrawingPreTransformedVerts(false);
        OffsetArg baseIndexOffset = OffsetArg::AsOffsetInIndices(pDrawPrimitiveArg->StartIndex);
        OffsetArg baseVertexOffset = OffsetArg::AsOffsetInVertices(pDrawPrimitiveArg->BaseVertexIndex);

        HRESULT hr = S_OK;
        if (pDrawPrimitiveArg->PrimitiveType == D3DPT_TRIANGLEFAN)
        {
            hr = pDevice->DrawTriangleFanIndexed(baseVertexOffset, vertexCount, baseIndexOffset, pDrawPrimitiveArg->PrimitiveCount);
        }
        else
        {
            bool skipDraw;
            hr = DrawProlog(*pDevice, baseVertexOffset, vertexCount, baseIndexOffset, IndexCountPerInstance, pDrawPrimitiveArg->PrimitiveType, instanceCount, skipDraw);
            CHECK_HR(hr);

            if (skipDraw)
            {
                return hr;
            }

            if (SUCCEEDED(hr))
            {
                pDevice->GetContext().DrawIndexedInstanced(
                    IndexCountPerInstance,
                    instanceCount,
                    baseIndexOffset.GetOffsetInIndices(),
                    baseVertexOffset.GetOffsetInVertices(),
                    StartInstanceLocation);
            }
        }

        hr = DrawEpilogue(*pDevice);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }
};
