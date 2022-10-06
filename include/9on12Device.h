// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "SharedResourceHelpers.hpp"

namespace D3D9on12
{
    struct Context
    {
        HANDLE m_runtimeHandle;
    };

    class VideoDevice;

    class Device
    {
    public:
        Device( Adapter& Adapter, _Inout_ D3DDDIARG_CREATEDEVICE& CreateDeviceArgs );
        ~Device();

        static void ReportError(HRESULT /*hr*/) {};

        HRESULT Init(ID3D12Device *pDevice, ID3D12CommandQueue *pCommandQueue);

        HRESULT Destroy();
        HRESULT FlushWork(bool WaitOnCompletion, UINT FlushFlags = 0);
        HRESULT Present(CONST D3DDDIARG_PRESENT1& PresentArgs, D3DKMT_PRESENT *pKMTArgs);
        HRESULT CloseAndSubmitGraphicsCommandListForPresent(BOOL commandsAdded, _In_reads_(numSrcSurfaces) const D3DDDIARG_PRESENTSURFACE* pSrcSurfaces, UINT numSrcSurfaces, _In_opt_ HANDLE hDestResource, _In_ D3DKMT_PRESENT* pKMTPresent);

        void MarkStateAsDirty();

        static FORCEINLINE HANDLE GetHandleFromDevice(Device* pDevice){ return static_cast<HANDLE>(pDevice); }
        static FORCEINLINE Device* GetDeviceFromHandle(HANDLE hDevice){ return static_cast<Device*>(hDevice); }
        std::shared_ptr<D3D12TranslationLayer::SwapChainManager> GetSwapChainManager();

        ConstantsManager &GetConstantsManager() { return m_constantsManager; }

        UINT32 GetNodeMask() { return m_NodeMask; }

        Adapter& GetAdapter() { return m_Adapter; }
        HANDLE GetRuntimeHandle() { return m_runtimeHandle; }
        Context& GetRuntimeContext() { return m_runtimeContext; }


        PipelineState& GetPipelineState() { return m_pipelineState; }
        PipelineStateCache& GetPipelineStateCache() { return m_pipelineStateCache; }

        UINT GetD3D9ApiVersion() { return m_d3d9APIVersion; }

        void SetStreamFrequency(_In_range_(0, MAX_VERTEX_STREAMS - 1 ) UINT streamIndex, UINT divider)
        {
            Check9on12(streamIndex < MAX_VERTEX_STREAMS);
            if (m_streamFrequency[streamIndex] != divider){ m_pipelineState.MarkInputLayoutAsDirty(); }
            m_streamFrequency[streamIndex] = divider;
        }

        UINT GetStreamFrequency(_In_range_(0, MAX_VERTEX_STREAMS - 1) UINT streamIndex)
        {
            Check9on12(streamIndex < MAX_VERTEX_STREAMS);
            return m_streamFrequency[streamIndex];
        }

        UINT* GetPointerToStreamFrequencies() { return m_streamFrequency; }

        WeakHash HashStreamFrequencyData(WeakHash inputHash);

        HRESULT ResolveDeferredState(OffsetArg BaseVertexStart, OffsetArg BaseIndexStart);

        D3DDDI_DEVICECALLBACKS& GetRuntimeCallbacks() { return m_Callbacks; }

        ID3D12CommandQueue& GetCommandQueue(){ return *m_pCommandQueue; }
        ID3D12Device &GetDevice() { return *m_pDevice; }
        D3D_FEATURE_LEVEL GetFeatureLevel() { return GetContext().FeatureLevel();}
        
        CComPtr<ID3D12Device> m_pDevice;
        CComPtr<ID3D12CommandQueue> m_pCommandQueue;

        D3D12TranslationLayer::ImmediateContext& GetContext() { return *m_pImmediateContext; }
        
        D3D12TranslationLayer::SharedResourceHelpers& GetSharedResourceHelper() { return *m_pSharedResourceHelpers; }
        D3D12TranslationLayer::BlitHelper& GetBlitHelper() { return GetContext().m_BlitHelper; }

        HRESULT DrawTriangleFan(_In_ OffsetArg baseVertex, _In_ UINT primitiveCount);
        HRESULT DrawWireframeTriangleFanWithEdgeFlags(_In_ OffsetArg baseVertex, _In_ UINT primitiveCount, _In_ UINT edgeFlags);
        HRESULT DrawTriangleFanIndexed(_In_ OffsetArg baseVertex, _In_ UINT vertexCount, _In_ OffsetArg baseIndex, _In_ UINT primitiveCount);

        FastUploadAllocator& GetSystemMemoryAllocator() { return m_systemMemoryAllocator; }
        DataLogger &GetDataLogger() { return m_dataLogger; }
        template<D3D12TranslationLayer::EShaderStage ShaderStage>
        void SetConstantBuffer(UINT shaderRegister, D3D12TranslationLayer::Resource *pResource, UINT offsetInBytes)
        {
            const UINT CONSTANT_BUFFER_ELEMENT_SIZE = sizeof(FLOAT) * 4;
            UINT firstConstant = offsetInBytes / CONSTANT_BUFFER_ELEMENT_SIZE;
            UINT maxConstantBufferElements = ConstantsManager::g_cMaxConstantBufferSize / CONSTANT_BUFFER_ELEMENT_SIZE;
            GetContext().SetConstantBuffers<ShaderStage>(shaderRegister, 1, &pResource, &firstConstant, &maxConstantBufferElements);
        }

        void EnsureVideoDevice();
        VideoDevice *GetVideoDevice();
        HRESULT DoBlit(_In_ CONST D3DDDIARG_BLT* pBltArgs, BOOL EnableAlpha);

        D3DDDIARG_PRESENT1* m_pUMDPresentArgs;

        std::mutex m_WrappingResourceSetLock;
        std::set<IUnknown*> m_WrappingResourceSet;

        ShaderConv::ShaderConverterAPI m_ShaderConvAPI;

        ShaderDedupe<VertexShader> m_VSDedupe;
        ShaderDedupe<PixelShader> m_PSDedupe;

        D3D12TranslationLayer::COptLockedContainer<std::unordered_map<Resource*, std::vector<LockRange>>> m_lockedResourceRanges;
        void SetDrawingPreTransformedVerts(bool preTransformedVerts);

    protected:
        virtual void LogCreateVideoDevice( HRESULT hr );

    private:

        struct  TriangleFanIndexBufferCache
        {
            
            InputBuffer highestPCIndexBufferTriFan;
            UINT highestPrimitiveCountTriFan;
            bool isTriFanCacheInitialized;

            TriangleFanIndexBufferCache() : highestPrimitiveCountTriFan(0), isTriFanCacheInitialized(false)
            {}
        } ;

        // Make sure this is the last thing that gets called by the destructor
        std::optional<D3D12TranslationLayer::ImmediateContext> m_pImmediateContext;
        std::optional<D3D12TranslationLayer::SharedResourceHelpers> m_pSharedResourceHelpers;

        std::mutex m_SwapChainManagerMutex;
        std::shared_ptr<D3D12TranslationLayer::SwapChainManager> m_SwapChainManager;

        CComPtr<ID3DBlob> m_pSerializedLayout;
    
        Adapter& m_Adapter;
        HANDLE m_runtimeHandle;

        Context m_runtimeContext;

        D3DDDI_DEVICECALLBACKS m_Callbacks;

        ConstantsManager m_constantsManager;

        PipelineStateCache m_pipelineStateCache;
        PipelineState m_pipelineState;

        UINT32 m_NodeMask;

        UINT m_streamFrequency[MAX_VERTEX_STREAMS];

        UINT m_d3d9APIVersion;

        FastUploadAllocator m_systemMemoryAllocator;

        TriangleFanIndexBufferCache m_triangleFanIndexBufferCache;
        DataLogger m_dataLogger;

        BYTE m_pVideoDeviceSpace[sizeof(VideoDevice)];
        VideoDevice *m_pVideoDevice;

        bool m_drawingPreTransformedVerts = false;

        //This should be cleared before each use. we're just saving the allocation
        std::vector<D3D12TranslationLayer::PresentSurface> m_d3d12tlPresentSurfaces;
        //This should be cleared before each use. we're just saving the allocation
        std::vector<D3DDDIARG_PRESENTSURFACE> m_d3d9PresentSurfaces;
    };
};
