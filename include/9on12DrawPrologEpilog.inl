namespace D3D9on12
{
    static HRESULT DrawProlog(Device& device, OffsetArg BaseVertexStart, UINT vertexCount, OffsetArg baseIndexLocation, UINT indexCount, D3DPRIMITIVETYPE primitiveType, UINT& instancesToDraw, bool& skipDraw)
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
        instancesToDraw = (streamFrequence & D3DSTREAMSOURCE_INDEXEDDATA) ? streamFrequence & ~D3DSTREAMSOURCE_INDEXEDDATA : 1;

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
};