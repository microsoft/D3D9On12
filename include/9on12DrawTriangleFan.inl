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
        InputBuffer& currentIndexBuffer = inputAssembly.CurrentIndexBuffer();
        void* pSrcIndexBuffer = nullptr;
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

};