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
    inline _Check_return_ HRESULT APIENTRY DrawPrimitive(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DRAWPRIMITIVE* pDrawPrimitiveArg, _In_opt_ CONST UINT* pFlagBuffer)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        Device* pDevice = Device::GetDeviceFromHandle(hDevice);

        if (pDevice == nullptr || pDrawPrimitiveArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

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

        const size_t bufferSize = dwIndicesSize * pData->PrimitiveCount * pData->NumVertices;
        
        UINT const stream0Stride = pDevice->GetPipelineState().GetInputAssembly().GetStream0Stride();
        Check9on12(stream0Stride > 0);

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
