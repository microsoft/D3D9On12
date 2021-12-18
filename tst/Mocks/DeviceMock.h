// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once


namespace D3D9on12::Test
{
    class DeviceMock : public Device 
    {
    public:
        DeviceMock(Adapter& Adapter, D3DDDIARG_CREATEDEVICE& CreateDeviceArgs,
            ConstantsManagerFactory& constantsManagerFactory,
            FastUploadAllocatorFactory& fastUploadAllocatorFactory,
            PipelineStateFactory& pipelineStateFactory,
            PipelineStateCacheFactory& pipelineStateCacheFactory)
            : Device(Adapter, CreateDeviceArgs, constantsManagerFactory, fastUploadAllocatorFactory, pipelineStateFactory, pipelineStateCacheFactory) {}

        MOCK_METHOD(HRESULT, DrawTriangleFan, (_In_ OffsetArg baseVertex, _In_ UINT primitiveCount), (override));
        MOCK_METHOD(HRESULT, DrawWireframeTriangleFanWithEdgeFlags, (_In_ OffsetArg baseVertex, _In_ UINT primitiveCount, _In_ UINT edgeFlags), (override));
        MOCK_METHOD(HRESULT, DrawTriangleFanIndexed, (_In_ OffsetArg baseVertex, _In_ UINT vertexCount, _In_ OffsetArg baseIndex, _In_ UINT primitiveCount), (override));

    
    };


    class DeviceMockBuilder
    {
    public:
        DeviceMockBuilder& Adapter(Adapter& adapter) { m_adapter.emplace(adapter); return *this; }
        DeviceMockBuilder& CreateArgs(D3DDDIARG_CREATEDEVICE createArgs) { m_createArgs.emplace(createArgs); return *this; }
        DeviceMockBuilder& ConstantsManagerFactory(ConstantsManagerFactory& factory) { m_constantsManagerFactory = &factory; return *this; }
        DeviceMockBuilder& FastUploadAllocatorFactory(FastUploadAllocatorFactory& factory) { m_allocatorFactory = &factory; return *this; }
        DeviceMockBuilder& PipelineStateFactory(PipelineStateFactory& factory) { m_pipelineStateFactory = &factory; return *this; }
        DeviceMockBuilder& PipelineStateFactory(PipelineStateCacheFactory& factory) { m_pipelineStateCacheFactory = &factory; return *this; }
        DeviceMock Build() {
            return DeviceMock(
                m_adapter.value_or(adapterPlaceholder),
                m_createArgs.value_or(createDeviceArgsPlaceholder),
                m_constantsManagerFactory ? *m_constantsManagerFactory : constantsManagerFactoryPlaceholder,
                m_allocatorFactory ? *m_allocatorFactory : fastUploadAllocatorFactoryPlaceholder,
                m_pipelineStateFactory ? *m_pipelineStateFactory : pipelineStateFactoryPlaceholder,
                m_pipelineStateCacheFactory ? *m_pipelineStateCacheFactory : pipelineStateCacheFactoryPlaceholder);
        }

    private:
        std::optional<D3D9on12::Adapter> m_adapter;
        std::optional<D3DDDIARG_CREATEDEVICE> m_createArgs;
        D3D9on12::ConstantsManagerFactory* m_constantsManagerFactory;
        D3D9on12::FastUploadAllocatorFactory* m_allocatorFactory;
        D3D9on12::PipelineStateFactory* m_pipelineStateFactory;
        D3D9on12::PipelineStateCacheFactory* m_pipelineStateCacheFactory;

    private:
        D3DDDIARG_CREATEDEVICE createDeviceArgsPlaceholder;
        AdapterMock adapterPlaceholder;
        ConstantsManagerFactoryFake constantsManagerFactoryPlaceholder;
        FastUploadAllocatorFactoryFake fastUploadAllocatorFactoryPlaceholder;
        PipelineStateFactoryFake pipelineStateFactoryPlaceholder;
        PipelineStateCacheFactoryFake pipelineStateCacheFactoryPlaceholder;
    };
}
