// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class Device; // Forward Declaration
    class Resource;
    const HANDLE NULL_HANDLE = (HANDLE)1;

    struct ResourceCopyArgs
    {
        ResourceCopyArgs(Resource& destination, Resource& source);
        void AsBufferBlit(UINT dstOffset, D3DDDIRANGE srcRange);
        void AsTextureBlit(const POINT &dstPoint, const RECT &srcRect, UINT cubeFace = UINT_MAX);
        void AsSubresourceBlit(UINT dstSubresourceIndex, UINT srcSubresourceIndex, CONST RECT* pDstRect = nullptr, CONST RECT* pSrcRect = nullptr);
        void AsStretchBlit(UINT dstSubresourceIndex, UINT srcSubresourceIndex, CONST RECT* pDstRect = nullptr, CONST RECT* pSrcRect = nullptr);
        void AsVolumeBlit(UINT dstX, UINT dstY, UINT dstZ, const D3D12_BOX &srcBox);
        bool IsBufferBlit() const;

        ResourceCopyArgs GetSubArgs(UINT arrayLevel, UINT sourceMipLevel, UINT destinationMipLevel) const;

        // Align the source box and destination to meet Block compressed and
        // Planar requirements
        void AlignCopyDimensions();

        // ClipSourceBox to respect the src and dst resource's dimensions
        void ClipSourceBox(UINT sourceMipLevel, UINT destinationMipLevel);

        UINT m_sourceSubresourceIndex;
        UINT m_destinationSubresourceIndex;
        UINT m_cubeFace;
        UINT m_numSubresources;
        bool m_allSubresources;
        bool m_mipsAlreadyExpanded;

        UINT m_destinationX, m_destinationY, m_destinationZ;
        D3D12_BOX m_sourceBox;
        bool m_dimensionsCoverEntireResource;

        RECT m_stretchSource;
        RECT m_stretchDestination;
        bool m_isStretchCopy;

        Resource& m_destination;
        Resource& m_source;

        BOOL m_EnableAlpha;
    };

    struct LockRange
    {
        LockRange(const D3DDDIARG_LOCKASYNC &lockArgs)
        {
            // Regardless of what union member is initialized, Box should cover it since it's the biggest struct
            Box = lockArgs.Box;
        }

        LockRange(const D3DDDIARG_LOCK &lockArgs)
        {
            // Regardless of what union member is initialized, Box should cover it since it's the biggest struct
            Box = lockArgs.Box;
        }

        union
        {
            D3DDDIRANGE     Range;              // in: range being locked
            RECT            Area;               // in: area being locked
            D3DDDIBOX       Box;                // in: volume being locked
        };
    };

    static UINT64 g_globalResourceID = 0;

    static const D3D12_DSV_FLAGS g_cPossibleDepthStencilStates[] = 
    {
        D3D12_DSV_FLAG_NONE,
        D3D12_DSV_FLAG_READ_ONLY_DEPTH,
        D3D12_DSV_FLAG_READ_ONLY_STENCIL,
         (D3D12_DSV_FLAGS)COMPILETIME_OR_2FLAGS(D3D12_DSV_FLAG_READ_ONLY_DEPTH, D3D12_DSV_FLAG_READ_ONLY_STENCIL),
    };

    class Resource
    {
    public:
        Resource();

        Resource(HANDLE runtimeHandle, Device* parent);
        virtual ~Resource();

        HRESULT Init(_In_ D3DDDIARG_CREATERESOURCE2& createArgs);
        HRESULT Init(UINT width, UINT height, D3DFORMAT format, D3DDDI_RESOURCEFLAGS resourceFlags, UINT mipLevels = 1, UINT depth = 1, UINT arraySize = 1, bool DoNotCreateAsTypelessResource = false);
        HRESULT Init(unique_comptr<D3D12TranslationLayer::Resource> pResource, D3D9ON12_OPEN_RESOURCE_ARGS *pD3d9on12Args, bool DoNotCreateAsTypelessResource = false);

        HRESULT OpenResource(D3DKMT_HANDLE kmtHandle, void *pPrivateResourceDriverData, UINT privateResourceDriverDataSize, D3D9ON12_OPEN_RESOURCE_ARGS *pD3d9on12Args);

        D3D12TranslationLayer::Resource *GetUnderlyingResource() { return m_pResource.get(); }

        ID3D12Resource &GetResource();
        D3D12TranslationLayer::ResourceCreationArgs &GetTranslationLayerCreateArgs() { return m_TranslationLayerCreateArgs; }
        const D3D12_RESOURCE_DESC &GetDesc() { return m_desc; };
        DXGI_FORMAT GetViewFormat(bool sRGBEnabled);
        D3DFORMAT GetD3DFormat() { return m_dx9Format; }
        const D3D12_RESOURCE_DESC &GetLogicalDesc() { return m_logicalDesc; };
        HANDLE GetRuntimeHandle() { return m_runtimeHandle; }

        HRESULT Lock(Device &device,
            _In_ UINT subresourceIndex,
            _In_ D3DDDI_LOCKFLAGS flags,
            _In_ LockRange lock,
            _In_ bool bAsyncLock,
            _Out_ VOID* &pSurfData,
            _Out_ UINT &pitch,
            _Out_ UINT &slicePitch,
            _Out_opt_ HANDLE *phCookie = nullptr);

        void Unlock(Device &device, 
            UINT subresourceIndex,
            D3DDDI_UNLOCKFLAGS flags,
            bool bAsyncUnlock);

        HRESULT Rename(Device& device, CONST D3DDDIARG_RENAME& arg);

        HRESULT GenerateMips(D3DDDITEXTUREFILTERTYPE filterType);

        HRESULT PreBind();

        static FORCEINLINE HANDLE GetHandleFromResource(Resource* pResource) { return (pResource) ? static_cast<HANDLE>(pResource) : NULL_HANDLE; }
        static FORCEINLINE Resource* GetResourceFromHandle(HANDLE hResource) { return (hResource != NULL_HANDLE) ? static_cast<Resource*>(hResource) : nullptr; }

        HRESULT Clear(_In_ UINT SubResourceIndex, _In_ const RECT* dstRects, _In_ UINT numRects, _In_ D3DCOLOR  Color);
        void ClearAllPlanesInArraySlice(_In_ UINT ArrayIndex, _In_ const RECT* dstRects, _In_ UINT numRects, _In_ D3DCOLOR  Color);
        HRESULT ClearDepthStencil(UINT flag, _In_ const RECT* dstRects, _In_ UINT numRects, float depthValue, UINT stencilValue);

        D3D12TranslationLayer::RTV* GetRenderTargetView(UINT subresourceIndex, bool srgbEnabled = false);
        D3D12TranslationLayer::DSV* &GetDepthStencilView(D3D12_DSV_FLAGS flags);
        enum ShaderResourceViewFlags
        {
            None = 0x0,
            SRGBEnabled = 0x1,
            DisableMips = 0x2,
            NumPermutations = 4
        };
        D3D12TranslationLayer::SRV* GetShaderResourceView(DWORD flags = ShaderResourceViewFlags::None);


        // Gets a resource that has all the same contents as this resource
        // but is a "plain" DX12 resource (single sample, no CPU access, optionally 
        // not as a TypelessResource) This is helpful when we need to pass a resource 
        // into an API that only expects "plain" DX12 resources. Some examples:
        // * Binding System Memory as an SRV
        // * A resource that's the source of a DX12 internal blt present.
        virtual Resource *GetBackingPlainResource(bool bDoNotCreateAsTypelessResource = false);
        
        // Returns a resource that has the current contents of this resource
        // copied into a readback heap. Generally expected to be a short-lived
        // resource only used for reading the data on the CPU
        unique_comptr<D3D12TranslationLayer::Resource> GetStagingCopy();
        
        ShaderConv::TEXTURETYPE GetShaderResourceTextureType();
        HANDLE GetSharedGDIHandle();
        virtual HANDLE CreateSharedNTHandle(SECURITY_DESCRIPTOR *pSD);

        UINT ConvertAppSubresourceToDX12Subresource(UINT index);

        void ConvertAppSubresourceToDX12SubresourceIndices(
            _In_ UINT subresourceIndex,
            _Out_writes_(MAX_PLANES) UINT *pSubresourceIndices,
            _Out_range_(0, MAX_PLANES) UINT &pPlaneCountToMap,
            bool bMipAlreadyExpanded = false);

        UINT ConvertAppSubresourceToDX12SubresourceInternal(UINT appIndex, UINT planeIndex, bool bMipAlreadyExpanded);

        UINT GetNumSubresources() { return m_numSubresources; }
        UINT GetNumAppVisibleMips() { return m_mipMapsHiddenFromApp ? 1 : GetLogicalDesc().MipLevels; }
        bool IsCubeMap() const { return m_cubeMap; }
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT &GetSubresourceFootprint(UINT subresource);

        UINT8 GetArraySize();
        UINT GetMipLevelFromSubresourceIndex(UINT subresourceIndex);
        UINT8 GetArraySliceFromSubresourceIndex(UINT subresourceIndex);
        bool IsSystemMemory() { return m_isD3D9SystemMemoryPool; }
        bool IsAlphaChannelDisabled() { return m_DisableAlphaChannel; }
        bool NeedsSwapRBOutputChannels() { return FormatRequiresSwappedRBChannels(m_dx9Format); }
        bool IsEligibleForHardwareShadowMapping() { return m_compatOptions.m_isEligibleForHardwareShadowMapping; }
        bool IsResident() { return GetUnderlyingResource()->IsResident(); }

        static void CopyPrologue(Device& device, ResourceCopyArgs& args);
        static void CopyResource(Device& device, ResourceCopyArgs& args);
        static void CopySubResource(Device& device, ResourceCopyArgs& args);
        static void CopyToSystemMemoryResource(Device& device, ResourceCopyArgs& args);
        static void CopyMatchingMipLevels(Device& device, ResourceCopyArgs& args);

        DXGI_FORMAT DepthStencilViewFormat() { return m_dsvFormat; }

        UINT64 ID(){ return m_ID; }

        void* GetSystemMemoryBase() { return m_appLinearRepresentation.m_systemData[0].pData; }

        // ---- Triangle fan index buffer cache methods ----        
        void GetTriFanIB(_Out_ InputBuffer& convertedIB, _In_ UINT baseIndexLocation, _In_ UINT indexCount, _In_ UINT ibStride);
        bool IsTriangleFanIBCacheDirty(UINT baseIndexLocation);


        Device* GetParent() { return m_pParentDevice; }
    private:
        class BindingTracker
        {
        public:
            void Bind(UINT bindIndex) { m_boundIndices.push_back(bindIndex); }
            void Unbind(UINT bindIndex)
            {
                // Doing an inefficient O(N) delete, betting that this is generally okay
                // because a resource should generally only be bound at a couple of places
                // at any given time.
                if (m_boundIndices.size() > 5)
                {
                    INEFFICIENT_UNBINDING_ARRAY_DELETION();
                }

                for (UINT i = 0; i < m_boundIndices.size(); i++)
                {
                    if (bindIndex == m_boundIndices[i])
                    {
                        m_boundIndices.erase(m_boundIndices.begin() + i);
                        return;
                    }
                }
            }

            bool IsBound() { return m_boundIndices.size() > 0; }
            const std::vector<UINT> &GetBindingIndices() { return m_boundIndices; }
        private:
            std::vector<UINT> m_boundIndices;
        };

        BindingTracker m_SRVBindingTracker;
        BindingTracker m_RTVBindingTracker;
        BindingTracker m_DSVBindingTracker;
        BindingTracker m_VBBindingTracker;
        BindingTracker m_IBBindingTracker;
    public:
        BindingTracker &GetRTVBindingTracker() { return m_RTVBindingTracker; }
        BindingTracker &GetSRVBindingTracker() { return m_SRVBindingTracker; }
        BindingTracker &GetDSVBindingTracker() { return m_DSVBindingTracker; }
        BindingTracker &GetVBBindingTracker() { return m_VBBindingTracker; }
        BindingTracker &GetIBBindingTracker() { return m_IBBindingTracker; }

        D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDescForArraySlice(UINT8 arraySlice, UINT8 arraySize);
        UINT GetVidPnSourceId() const { return m_VidPnSourceId; }

    protected:
        virtual void CreateUnderlyingResource( D3D12TranslationLayer::ResourceCreationArgs& underlyingResourceCreateArgs, _Inout_ D3DDDIARG_CREATERESOURCE2& createArgs );
        void CreateUnderlyingResource( D3D12TranslationLayer::ResourceCreationArgs& createArgs );
        Resource* m_pBackingShaderResource;

    private:
        void UpdateTriangleFanIBCache(InputBuffer& newIBToStore, UINT baseIndexLocation);
        // Buffer used to cache conversion between triangle fan and triangle list topologies. This allows us to avoid maping(and blocking the gpu) the index buffer in each draw call.
        // This stores the TRIANGLE LIST CONVERTED INDEX BUFFER, actually enabling us to avoid the conversion as well.
        struct TriangleFanIBCache {
            TriangleFanIBCache() : 
                m_baseIndexOffsetInBytes(0), 
                m_dirtyFlag(true)
            {}

            InputBuffer m_TriangleFanConvertedTriangleListIndexBuffer;
            UINT m_baseIndexOffsetInBytes;
            bool m_dirtyFlag;
        } m_TriFanIBCache;

        void NotifyResourceChanged();
        bool IsSRGBCompatibleTexture();

        HRESULT InitInternal(
            _In_ D3DDDIARG_CREATERESOURCE2& createArgs, 
            bool onlyFirstSurfInfoInitialized, 
            unique_comptr<D3D12TranslationLayer::Resource> pAlreadyCreatedResource = {},
            bool DoNotCreateAsTypelessResource = false);

        HRESULT ClearResourceWithNoRenderTarget(_In_ UINT SubResourceIndex, _In_ const RECT* dstRects, _In_ UINT numRects, _In_ D3DCOLOR  Color);
        void CreateRenderTargetViews(DXGI_FORMAT rtvFormat, bool GenerateSRGBRTV);
        void CreateDepthStencilViews();
        void CreateVideoDecoderOutputViews();
        void CreateVideoProcessorOutputViews();

        static void InitializeSRVDescFormatAndShaderComponentMapping(_In_ D3DFORMAT d3dFormat, _Out_ D3D12_SHADER_RESOURCE_VIEW_DESC &desc, bool disableAlphaChannel);
        D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc();
        D3D12TranslationLayer::SRV * CreateShaderResourceView(ShaderResourceViewFlags flag);

        UINT64 m_ID;

        Device* m_pParentDevice;

        D3D12TranslationLayer::D3D12ResourceSuballocation m_LastRenamedResource;

        unique_unbind_resourceptr m_pResource;

        HANDLE m_runtimeHandle;

        bool m_DisableAlphaChannel = false;
        bool m_IsLockable;
        UINT m_numSubresources;
        UINT8 m_numArrayLevels;
        UINT8 m_NonOpaquePlaneCount;
        UINT64 m_totalSize;
        bool m_mipMapsHiddenFromApp;
        bool m_cubeMap;

#if DBG
        D3DDDIARG_CREATERESOURCE2 m_d3d9Desc;
#endif
        D3D12_RESOURCE_DESC m_desc;
        D3DFORMAT m_dx9Format;
        DXGI_FORMAT m_dsvFormat;
        DXGI_FORMAT m_ViewFormat;
        D3D12_RESOURCE_DESC m_logicalDesc;
        UINT m_CpuAccessFlags;
        UINT m_VidPnSourceId;

        D3D12TranslationLayer::ResourceCreationArgs m_TranslationLayerCreateArgs;

        std::vector<std::unique_ptr<D3D12TranslationLayer::RTV>> m_renderTargetViews;
        std::vector<std::unique_ptr<D3D12TranslationLayer::RTV>> m_SRGBRenderTargetViews;

        byte m_depthStencilViewSpace[sizeof(D3D12TranslationLayer::DSV) * _countof(g_cPossibleDepthStencilStates)];
        D3D12TranslationLayer::DSV* m_pDepthStencilViews[_countof(g_cPossibleDepthStencilStates)];

        std::unique_ptr<D3D12TranslationLayer::SRV> m_shaderResourceViews[ShaderResourceViewFlags::NumPermutations];

        std::vector<std::unique_ptr<D3D12TranslationLayer::VDOV>> m_videoDecoderOutputViews;
        std::vector<std::unique_ptr<D3D12TranslationLayer::VPOV>> m_videoProcessorOutputViews;

        ShaderConv::TEXTURETYPE m_srvTextureType;

        bool m_isDecodeCompressedBuffer;
        bool m_isD3D9SystemMemoryPool; //System memory resources need to be updated at bind time
        

        struct LockData
        {
            LockData() : m_LockCount(0) {}

            D3DDDI_LOCKFLAGS m_Flags;
            D3D12_BOX m_Box;
            UINT m_LockCount;
        };

        std::vector<LockData> m_lockData;

        // Information which describes how the app 
        // represents the resource in system memory
        struct AppLinearRepresentation
        {
            std::vector<D3D12_MEMCPY_DEST> m_systemData;
        };

        // Information which describes how the resource
        // will be represented as a linear buffer for each
        // subresource
        struct PhysicalLinearRepresentation
        {
            std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> m_footprints;
            std::vector<UINT64> m_rowPitces;
            std::vector<UINT> m_numRows;
        };

        AppLinearRepresentation m_appLinearRepresentation;
        PhysicalLinearRepresentation m_physicalLinearRepresentation;

        // Contains information about possible runtime extensions e.g. Hardware shadow maps.
        struct ResourceCompatibilityOptions
        {
            ResourceCompatibilityOptions() : m_isEligibleForHardwareShadowMapping(false){};

            void Init(D3DFORMAT format);

            bool m_isEligibleForHardwareShadowMapping;
        };

        ResourceCompatibilityOptions m_compatOptions;

        class CreateTextureArgHelper
        {
        public:

            CreateTextureArgHelper(UINT width, UINT height, DXGI_FORMAT format, D3DDDI_RESOURCEFLAGS resourceFlags, UINT mipLevels = 1, UINT depth = 1, UINT arraySize = 1) :
                CreateTextureArgHelper(width, height, ConvertFromDXGIFormatToD3DDDIFormat(format), resourceFlags, mipLevels, depth, arraySize) {}
                
            CreateTextureArgHelper(UINT width, UINT height, D3DDDIFORMAT format, D3DDDI_RESOURCEFLAGS resourceFlags, UINT mipLevels = 1, UINT depth = 1, UINT arraySize = 1)
            {
                // Arrays of 3D textures are not allowed
                Check9on12(!(depth > 1 && arraySize > 1));
                const UINT surfCount = mipLevels * arraySize;

                // Don't waste time calculating the height and width for other mip levels, 9on12 doesn't need that information
                m_surfaceInfo = {};
                m_surfaceInfo.Height = static_cast<UINT>(height);
                m_surfaceInfo.Width = static_cast<UINT>(width);
                m_surfaceInfo.Depth = depth;

                m_createArg = {};
                m_createArg.Flags = resourceFlags;
                m_createArg.MipLevels = mipLevels;
                m_createArg.MultisampleQuality = 0;
                m_createArg.Pool = D3DDDIPOOL_LOCALVIDMEM;
                m_createArg.MultisampleType = D3DDDIMULTISAMPLE_NONE;
                m_createArg.SurfCount = surfCount;
                m_createArg.pSurfList = &m_surfaceInfo;
                m_createArg.Format = format;
            }


            _D3DDDIARG_CREATERESOURCE2 &GetCreateArgs()
            {
                return m_createArg;
            }

        private:

            _D3DDDI_SURFACEINFO m_surfaceInfo;
            _D3DDDIARG_CREATERESOURCE2 m_createArg;
        };
    };
};
