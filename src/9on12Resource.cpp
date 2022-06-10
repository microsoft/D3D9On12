// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"
#include <formatdesc.hpp>
#undef min
#undef max
using std::min;
using std::max;

namespace D3D9on12
{
    _Check_return_ UINT APIENTRY QueryResourcePrivateDriverDataSize(HANDLE hDD)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDD);
        if (pDevice == nullptr)
        {
            return 0;
        }

        D3D9on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN(hDD, S_OK, D3D12TranslationLayer::SharedResourceHelpers::cPrivateResourceDriverDataSize);
    }

    _Check_return_ HRESULT APIENTRY QueryResourceInfoFromKMTHandle(HANDLE hDD, D3DKMT_HANDLE hKMResource, _Out_ D3D9ON12_RESOURCE_INFO *pResourceInfo)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDD);
        if (pDevice == nullptr || pResourceInfo == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        D3D12TranslationLayer::ResourceInfo resourceInfo;;
        pDevice->GetSharedResourceHelper().QueryResourceInfoFromKMTHandle(hKMResource, nullptr, &resourceInfo);

#define D3DUSAGE_HEAVYWEIGHT              (0x08000000L)
        // Pulled from d3d8typesp.h. Every resource from an LH driver is expected by the 
        // DX9 runtime to have this usage

        assert(!resourceInfo.m_bSynchronized); // 9on12 doesn't understand the keyed mutex
        if (resourceInfo.m_Type == D3D12TranslationLayer::ResourceInfoType::ResourceType)
        {
            // convert dx12 info to dx9 info
            ZeroMemory(pResourceInfo, sizeof(*pResourceInfo));
            pResourceInfo->bNtHandle = resourceInfo.m_bNTHandle;
            pResourceInfo->bAllocatedBy9on12 = resourceInfo.m_bAllocatedBy9on12;
            pResourceInfo->Format = ConvertToD3DFORMAT(resourceInfo.Resource.m_ResourceDesc.Format);
            pResourceInfo->Pool = D3DPOOL_DEFAULT; // TODO is this safe to assume?
            pResourceInfo->Usage = D3DUSAGE_HEAVYWEIGHT;

            if (resourceInfo.Resource.m_ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                pResourceInfo->BufferDesc.FVF = 0;
                pResourceInfo->BufferDesc.Size = (UINT)resourceInfo.Resource.m_ResourceDesc.Width;

                if (resourceInfo.Resource.m_ResourceDesc.Format == DXGI_FORMAT_R16_UINT || resourceInfo.Resource.m_ResourceDesc.Format == DXGI_FORMAT_R8_UINT)
                {
                    pResourceInfo->Type = D3DRTYPE_INDEXBUFFER;
                }
                else
                {
                    assert(resourceInfo.Resource.m_ResourceDesc.Format == DXGI_FORMAT_UNKNOWN);
                    pResourceInfo->Type = D3DRTYPE_VERTEXBUFFER;
                }
            }
            else
            {
                const bool bIs3DTexture = resourceInfo.Resource.m_ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                pResourceInfo->TextureDesc.Depth = bIs3DTexture ? resourceInfo.Resource.m_ResourceDesc.DepthOrArraySize : 1;
                pResourceInfo->TextureDesc.Levels = resourceInfo.Resource.m_ResourceDesc.MipLevels;
                pResourceInfo->Width = (UINT)resourceInfo.Resource.m_ResourceDesc.Width;
                pResourceInfo->Height = resourceInfo.Resource.m_ResourceDesc.Height;
                pResourceInfo->TextureDesc.MultiSampleQuality = resourceInfo.Resource.m_ResourceDesc.SampleDesc.Quality;

                if (resourceInfo.Resource.m_ResourceDesc.SampleDesc.Count == 1)
                {
                    pResourceInfo->TextureDesc.MultiSampleType = D3DMULTISAMPLE_NONE;
                }
                else
                {
                    pResourceInfo->TextureDesc.MultiSampleType = (D3DMULTISAMPLE_TYPE)resourceInfo.Resource.m_ResourceDesc.SampleDesc.Count;
                }

                if (resourceInfo.Resource.m_ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
                {
                    pResourceInfo->Usage |= D3DUSAGE_RENDERTARGET;
                }
                if (resourceInfo.Resource.m_ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
                {
                    pResourceInfo->Usage |= D3DUSAGE_DEPTHSTENCIL;
                }

                switch (resourceInfo.Resource.m_ResourceDesc.Dimension)
                {
                case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                    assert(false);
                    break;
                case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                    if (resourceInfo.Resource.m_ResourceDesc.DepthOrArraySize == 6)
                    {
                        pResourceInfo->Type = D3DRTYPE_CUBETEXTURE;
                    }
                    // We don't have enough information to 100% determine whether
                    // this resource is a "texture" or a "surface", but this distinction
                    // turns out not to be too meaningful, so we just take a guess
                    else if (resourceInfo.Resource.m_ResourceDesc.SampleDesc.Count > 1)
                    {
                        pResourceInfo->Type = D3DRTYPE_SURFACE;
                    }
                    else
                    {
                        pResourceInfo->Type = D3DRTYPE_TEXTURE;
                    }
                    break;
                case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                    pResourceInfo->Type = D3DRTYPE_VOLUMETEXTURE;
                    break;
                }
            }
        }
        else
        {
            ThrowFailure(E_UNEXPECTED);
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DestroyKMTHandle(
        HANDLE hDD, D3DKMT_HANDLE hKMResource)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDD);
        if (pDevice == nullptr)
        {
            return E_INVALIDARG;
        }

        pDevice->GetSharedResourceHelper().DestroyKMTHandle(hKMResource);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY CreateKMTHandle(
        HANDLE hDD, _In_ HANDLE resourceHandle, _Out_ D3DKMT_HANDLE *pHKMResource,
        _Out_writes_bytes_(dataSize) void *pResourcePrivateDriverData, _In_ UINT dataSize)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDD);
        if (pDevice == nullptr || pHKMResource == nullptr)
        {
            return E_INVALIDARG;
        }

        CComPtr<IUnknown> pResourceUnk = nullptr;
        {
            std::lock_guard WrappingLock(pDevice->m_WrappingResourceSetLock);
            if (pDevice->m_WrappingResourceSet.find(reinterpret_cast<IUnknown*>(resourceHandle)) != pDevice->m_WrappingResourceSet.end())
            {
                pResourceUnk = reinterpret_cast<IUnknown*>(resourceHandle);
            }
        }
        if (pResourceUnk)
        {
            *pHKMResource = pDevice->GetSharedResourceHelper().CreateKMTHandle(pResourceUnk);
            pDevice->GetSharedResourceHelper().InitializePrivateDriverData(D3D12TranslationLayer::DeferredDestructionType::Submission, pResourcePrivateDriverData, dataSize);
        }
        else
        {
            *pHKMResource = pDevice->GetSharedResourceHelper().CreateKMTHandle(resourceHandle);
            pDevice->GetSharedResourceHelper().InitializePrivateDriverData(D3D12TranslationLayer::DeferredDestructionType::Completion, pResourcePrivateDriverData, dataSize);
        }
        pResourceUnk.Detach();
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY CreateResourceWrappingHandle(
        HANDLE hDD, _In_ IUnknown* pD3D12Resource, _Out_ HANDLE* phWrappingHandle)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDD);
        if (pDevice == nullptr || pD3D12Resource == nullptr || phWrappingHandle == nullptr)
        {
            return E_INVALIDARG;
        }

        std::lock_guard WrappingLock(pDevice->m_WrappingResourceSetLock);
        pDevice->m_WrappingResourceSet.insert(pD3D12Resource);
        *phWrappingHandle = pD3D12Resource;
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    void APIENTRY DestroyResourceWrappingHandle(HANDLE hDD, HANDLE hWrappingHandle)
    {
        Device* pDevice = Device::GetDeviceFromHandle(hDD);
        if (pDevice == nullptr || hWrappingHandle == nullptr)
        {
            return;
        }

        std::lock_guard WrappingLock(pDevice->m_WrappingResourceSetLock);
        pDevice->m_WrappingResourceSet.erase(reinterpret_cast<IUnknown*>(hWrappingHandle));
    }

    void CleanupTranslationLayerResourceBindings(D3D12TranslationLayer::ImmediateContext &context, D3D12TranslationLayer::Resource &resource)
    {
        context.ClearInputBindings(&resource);
        context.ClearOutputBindings(&resource);
    }

    _Check_return_ HRESULT APIENTRY GetSharedGDIHandle_Private(_In_ HANDLE hDevice, HANDLE DriverResource, HANDLE *pSharedGDIHandle)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device *pDevice = Device::GetDeviceFromHandle(hDevice);
        Resource *pResource = Resource::GetResourceFromHandle(DriverResource);
        if (pDevice == nullptr || pResource == nullptr)
        {
            return E_INVALIDARG;
        }

        *pSharedGDIHandle = 0;
        *pSharedGDIHandle = pResource->GetSharedGDIHandle();
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY CreateSharedNTHandle_Private(_In_ HANDLE hDevice, HANDLE DriverResource, _In_opt_ SECURITY_DESCRIPTOR* pSD, HANDLE *pSharedNTHandle)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device *pDevice = Device::GetDeviceFromHandle(hDevice);
        Resource *pResource = Resource::GetResourceFromHandle(DriverResource);
        if (pDevice == nullptr || pResource == nullptr)
        {
            return E_INVALIDARG;
        }

        *pSharedNTHandle = 0;
        *pSharedNTHandle = pResource->CreateSharedNTHandle(pSD);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DepthFill(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_DEPTHFILL* pDepthFill)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pDepthFill == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource* pResource = Resource::GetResourceFromHandle(pDepthFill->hResource);
        if (pResource == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Check9on12(pDepthFill->SubResourceIndex == 0);
        HRESULT hr =pResource->ClearDepthStencil(D3DCLEAR_ZBUFFER, &pDepthFill->DstRect, 1, (float)pDepthFill->Depth, 0);
        CHECK_HR(hr);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY CreateResource2(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATERESOURCE2* pCreateArgs)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device *pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pCreateArgs == nullptr)
        {
            return E_INVALIDARG;
        }

        if (pCreateArgs->Format == D3DFMT_NULL)
        {
            pCreateArgs->hResource = Resource::GetHandleFromResource(nullptr);
            return S_OK;
        }

        Resource *pResource = new Resource(pCreateArgs->hResource, pDevice);
        if (pResource == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = S_OK;

        hr = pResource->Init(*pCreateArgs);

        if (SUCCEEDED(hr))
        {
            pCreateArgs->hResource = Resource::GetHandleFromResource(pResource);
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY OpenResource_Private(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_OPENRESOURCE* hOpenResource, _In_ _D3D9ON12_OPEN_RESOURCE_ARGS *pD3D9on12OpenResource)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        HRESULT hr = S_OK;
        if (pDevice == nullptr || hOpenResource == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        if (hOpenResource->PrivateDriverDataSize != D3D12TranslationLayer::SharedResourceHelpers::cPrivateResourceDriverDataSize || hOpenResource->pPrivateDriverData == nullptr || pD3D9on12OpenResource == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource* pResource = new Resource(hOpenResource->hResource, pDevice);
        if (pResource == nullptr)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pResource->OpenResource(hOpenResource->hKMResource, hOpenResource->pPrivateDriverData, hOpenResource->PrivateDriverDataSize, pD3D9on12OpenResource);

            if (SUCCEEDED(hr))
            {
                hOpenResource->hResource = Resource::GetHandleFromResource(pResource);
            }
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY OpenResource(_In_ HANDLE, _Inout_ D3DDDIARG_OPENRESOURCE*)
    {
        // Expecting OpenResource_Private to get called instead
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY DestroyResource(_In_ HANDLE hDevice, _In_ HANDLE hResource)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        Resource* pResource = Resource::GetResourceFromHandle(hResource);
        // Destroying a null resource is valid if a resource is created with D3DFMT_NULL
        if (pResource == nullptr)
        {
            return S_OK;
        }

        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = S_OK;
        delete pResource;
        
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY UpdateSubresourceUP(_In_ HANDLE /*hDevice*/, _In_ CONST D3DDDIARG_UPDATESUBRESOURCEUP* /*pUpdateArgs*/)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    _Check_return_ HRESULT APIENTRY ColorFill(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_COLORFILL* pColorFill)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);

        if (pDevice == nullptr || pColorFill == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource* pResource = Resource::GetResourceFromHandle(pColorFill->hResource);

        if (pResource == nullptr)
        {
            // ColorFill on a null resource is okay (does nothing).
            return S_OK;
        }

        UINT subresourceIndices[MAX_PLANES];
        UINT numPlanes;
        pResource->ConvertAppSubresourceToDX12SubresourceIndices(pColorFill->SubResourceIndex, subresourceIndices, numPlanes);

        if (numPlanes == 1)
        {
            pResource->Clear(subresourceIndices[0], &pColorFill->DstRect, 1, pColorFill->Color);
        }
        else
        {
            UINT arrayIndex = pResource->GetArraySliceFromSubresourceIndex(subresourceIndices[0]);
#if DBG
            for (UINT planeIndex = 1; planeIndex < numPlanes; planeIndex++) assert(arrayIndex == pResource->GetArraySliceFromSubresourceIndex(subresourceIndices[planeIndex]));
#endif

            pResource->ClearAllPlanesInArraySlice(arrayIndex, &pColorFill->DstRect, 1, pColorFill->Color);
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY Clear(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_CLEAR *pClearArg, _In_ UINT numRects, _In_ CONST RECT*pDstRect)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pClearArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        if (pClearArg->Flags & D3DCLEAR_TARGET)
        {
            for (UINT i = 0; i < MAX_RENDER_TARGETS; i++)
            {
                const BoundRenderTarget &boundRenderTarget = pDevice->GetPipelineState().GetPixelStage().GetRenderTarget(i);
                if (boundRenderTarget.m_pResource)
                {
                    if (pDevice != boundRenderTarget.m_pResource->GetParent())
                    {
                        RETURN_E_INVALIDARG_AND_CHECK();
                    }

                    boundRenderTarget.m_pResource->Clear(boundRenderTarget.m_subresource, pDstRect, numRects, static_cast<D3DCOLOR>(pClearArg->FillColor));
                }
            }
        }
        if (pClearArg->Flags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL))
        {
            Resource *pDepthStencil = pDevice->GetPipelineState().GetPixelStage().GetDepthStencil();
            Check9on12(pDepthStencil);
            if (pDevice != pDepthStencil->GetParent())
            {
                RETURN_E_INVALIDARG_AND_CHECK();
            }

            pDepthStencil->ClearDepthStencil(pClearArg->Flags, pDstRect, numRects, pClearArg->FillDepth, pClearArg->FillStencil);
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY Lock(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_LOCK *pLockArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device *pDevice = Device::GetDeviceFromHandle(hDevice);
        Resource *pResource = Resource::GetResourceFromHandle(pLockArg->hResource);
        if (pDevice == nullptr || pLockArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        if (pResource == nullptr)
        {
            return S_OK;
        }

        UINT subresourceIndicies[MAX_PLANES];

        struct {
            void *pSurfData;
            UINT Pitch;
            UINT SlicePitch;
        } lockOutput[MAX_PLANES] = {};
        UINT numPlanes = 0;
        pResource->ConvertAppSubresourceToDX12SubresourceIndices(pLockArg->SubResourceIndex, subresourceIndicies, numPlanes);
        Check9on12(numPlanes > 0);

        HRESULT hr = S_OK;
        for (UINT planeIndex = 0; planeIndex < numPlanes && SUCCEEDED(hr); planeIndex++)
        {
            hr = pResource->Lock(
                *pDevice, 
                subresourceIndicies[planeIndex], 
                pLockArg->Flags, 
                LockRange(*pLockArg), 
                false, 
                lockOutput[planeIndex].pSurfData, 
                lockOutput[planeIndex].Pitch, 
                lockOutput[planeIndex].SlicePitch);
        }

        // Set the output data to the first subresource mapped.  The driver is required to 
        // place multiple planes adjacently for dynamic and staging textures, which is the
        // only case that emulation happens for.
        pLockArg->pSurfData = lockOutput[0].pSurfData;
        pLockArg->Pitch = lockOutput[0].Pitch;
        pLockArg->SlicePitch = lockOutput[0].SlicePitch;

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY Unlock(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UNLOCK* pUnlockArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device *pDevice = Device::GetDeviceFromHandle(hDevice);
        Resource *pResource = Resource::GetResourceFromHandle(pUnlockArg->hResource);
        if (pDevice == nullptr || pUnlockArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        if (pResource == nullptr)
        {
            return S_OK;
        }

        if (pDevice != pResource->GetParent())
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        UINT subresourceIndicies[MAX_PLANES];
        UINT numPlanes = 0;
        pResource->ConvertAppSubresourceToDX12SubresourceIndices(pUnlockArg->SubResourceIndex, subresourceIndicies, numPlanes);
        for (UINT planeIndex = 0; planeIndex < numPlanes; planeIndex++)
        {
            pResource->Unlock(*pDevice, subresourceIndicies[planeIndex], pUnlockArg->Flags, false);
        }
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY GenerateMipSubLevels(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_GENERATEMIPSUBLEVELS* pArgs)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        Resource* pResource = Resource::GetResourceFromHandle(pArgs->hResource);
        if (pDevice == nullptr || pArgs == nullptr || pDevice != pResource->GetParent())
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = S_OK;
        if (pResource != nullptr)
        {
            hr = pResource->GenerateMips(pArgs->Filter);
        }
        else
        {
            hr = E_INVALIDARG;
        }

        CHECK_HR(hr);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY LockAsync(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_LOCKASYNC* pArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource *pResource = Resource::GetResourceFromHandle(pArg->hResource);

        HANDLE hCookie = nullptr;
        UINT subresourceIndicies[MAX_PLANES];
        UINT numPlanes;
        pResource->ConvertAppSubresourceToDX12SubresourceIndices(pArg->SubResourceIndex, subresourceIndicies, numPlanes);

        // Planar resources don't support dynamic usage so we should never be using Lock-Discard/Rename with a planar resource 
        // unless it's just a system memory resource
        bool bIsPlanar = numPlanes > 1;
        HRESULT hr = S_OK;
        for (UINT i = 0; i < numPlanes && SUCCEEDED(hr); i++)
        {
            hr = pResource->Lock(*pDevice,
                subresourceIndicies[i],
                ConvertLockAsyncFlags(pArg->Flags),
                LockRange(*pArg),
                true,
                pArg->pSurfData,
                pArg->Pitch,
                pArg->SlicePitch,
                bIsPlanar ? nullptr : &hCookie);
        }

        // Pass a pointer to the resource out to 9, it's kept alive by the immediate context and we can release that ref later.
        pArg->hCookie = hCookie;
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY UnlockAsync(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_UNLOCKASYNC* pArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource *pResource = Resource::GetResourceFromHandle(pArg->hResource);

        UINT subresourceIndicies[MAX_PLANES];
        UINT numPlanes;
        pResource->ConvertAppSubresourceToDX12SubresourceIndices(pArg->SubResourceIndex, subresourceIndicies, numPlanes);
        for (UINT i = 0; i < numPlanes; i++)
        {
            pResource->Unlock(*pDevice, subresourceIndicies[i], ConvertUnlockAsyncFlag(pArg->Flags), true);
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY Rename(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_RENAME* pArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pArg == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource *pResource = Resource::GetResourceFromHandle(pArg->hResource);

        HRESULT hr = pResource->Rename(*pDevice, *pArg);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    Resource::Resource() : Resource(NULL, nullptr) {}

    Resource::Resource(HANDLE runtimeHandle, Device* parent) :
        m_runtimeHandle(runtimeHandle),
        m_numSubresources(0),
        m_numArrayLevels(0),
        m_renderTargetViews(),
        m_isD3D9SystemMemoryPool(false),
        m_ID(InterlockedIncrement(&g_globalResourceID)),
        m_pBackingShaderResource(nullptr),
        m_dx9Format(D3DFMT_UNKNOWN),
        m_ViewFormat(DXGI_FORMAT_UNKNOWN),
        m_dsvFormat(DXGI_FORMAT_UNKNOWN),
        m_pParentDevice(parent),
        m_CpuAccessFlags(0),
        m_isDecodeCompressedBuffer(false)
    {
        memset(m_pDepthStencilViews, 0, sizeof(m_pDepthStencilViews));
        memset(&m_TranslationLayerCreateArgs, 0, sizeof(m_TranslationLayerCreateArgs));
    }

    Resource::~Resource()
    {
        if (m_pBackingShaderResource)
        {
            delete m_pBackingShaderResource;
        }

        // This must be called before the views are destroyed
        m_pParentDevice->GetContext().ClearInputBindings(m_pResource.get());
        m_pParentDevice->GetContext().ClearOutputBindings(m_pResource.get());

        while (GetSRVBindingTracker().GetBindingIndices().size())
        {
            ThrowFailure(m_pParentDevice->GetPipelineState().GetPixelStage().SetSRV(*m_pParentDevice, nullptr, GetSRVBindingTracker().GetBindingIndices()[0]));
        }

        const BoundRenderTarget nullRenderTarget(nullptr, 0);
        while (GetRTVBindingTracker().GetBindingIndices().size())
        {
            m_pParentDevice->GetPipelineState().GetPixelStage().SetRenderTarget(GetRTVBindingTracker().GetBindingIndices()[0], nullRenderTarget);
        }

        while (GetVBBindingTracker().GetBindingIndices().size())
        {
            m_pParentDevice->GetPipelineState().GetInputAssembly().SetVertexBuffer(
                *m_pParentDevice, 
                nullptr, 
                GetVBBindingTracker().GetBindingIndices()[0], 
                0, 
                0);
        }

        if (GetDSVBindingTracker().IsBound())
        {
            assert(GetDSVBindingTracker().GetBindingIndices().size() == 1);
            m_pParentDevice->GetPipelineState().GetPixelStage().SetDepthStencil(nullptr);
        }

        if (GetIBBindingTracker().IsBound())
        {
            assert(GetIBBindingTracker().GetBindingIndices().size() == 1);
            m_pParentDevice->GetPipelineState().GetInputAssembly().SetIndexBuffer(*m_pParentDevice, nullptr, 0);
        }

        for (UINT i = 0; i < _countof(g_cPossibleDepthStencilStates); i++)
        {
            if (m_pDepthStencilViews[i])
            {
                m_pDepthStencilViews[i]->~View();
            }
        }

        m_pParentDevice->m_lockedResourceRanges.GetLocked()->erase(this);

        m_pResource.reset(nullptr);
    }

    void Resource::CreateUnderlyingResource( D3D12TranslationLayer::ResourceCreationArgs& underlyingResourceCreateArgs, _Inout_ D3DDDIARG_CREATERESOURCE2& createArgs )
    {
        UNREFERENCED_PARAMETER( createArgs );
        CreateUnderlyingResource( underlyingResourceCreateArgs );
    }

    void Resource::CreateUnderlyingResource(D3D12TranslationLayer::ResourceCreationArgs& createArgs)
    {
        m_pResource = D3D12TranslationLayer::Resource::CreateResource(&m_pParentDevice->GetContext(), createArgs, D3D12TranslationLayer::ResourceAllocationContext::ImmediateContextThreadLongLived); // throw( bad_alloc, _com_error )
    }

    ID3D12Resource& Resource::GetResource()
    {
        return *m_pResource->GetUnderlyingResource();
    }

    DXGI_FORMAT Resource::GetViewFormat(bool sRGBEnabled) 
    { 
        return sRGBEnabled ? ConvertToSRGB(m_ViewFormat) : m_ViewFormat; 
    }

    HRESULT Resource::Init(UINT width, UINT height, D3DFORMAT format, D3DDDI_RESOURCEFLAGS resourceFlags, UINT mipLevels, UINT depth, UINT arraySize, bool DoNotCreateAsTypelessResource)
    {
        CreateTextureArgHelper createArg(width, height, ConvertD3DFormatToDDIFormat(format), resourceFlags, mipLevels, depth, arraySize);
        return InitInternal(createArg.GetCreateArgs(), true, {}, DoNotCreateAsTypelessResource);
    }

    HRESULT Resource::Init(_In_ D3DDDIARG_CREATERESOURCE2 &createArgs)
    {
        return InitInternal(createArgs, false);
    }

    UINT16 MaxMipLevels(UINT64 uiMaxDimension)
    {
        UINT16 uiRet = 0;
        while (uiMaxDimension > 0)
        {
            uiRet++;
            uiMaxDimension >>= 1;
        }
        return uiRet;
    }

    HRESULT Resource::Init(unique_comptr<D3D12TranslationLayer::Resource> pResource, D3D9ON12_OPEN_RESOURCE_ARGS *pD3d9on12Args, bool DoNotCreateAsTypelessResource)
    {
        const D3D12_RESOURCE_DESC &desc = pResource->Parent()->m_desc12;
        _D3DDDI_RESOURCEFLAGS resourceFlags = {};

        const bool isBuffer = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
        const bool is3DTexture = desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        resourceFlags.IndexBuffer = isBuffer;
        resourceFlags.RenderTarget = desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        resourceFlags.Texture = !isBuffer && (cSupportsPlanarSRVs || !CD3D11FormatHelper::YUV(desc.Format));
        resourceFlags.ZBuffer = desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        resourceFlags.IndexBuffer = isBuffer;

        CreateTextureArgHelper createArgs(
            (UINT)desc.Width, 
            desc.Height, 
            ConvertD3DFormatToDDIFormat(pD3d9on12Args->Format),
            resourceFlags,
            desc.MipLevels,
            (UINT)(is3DTexture ? desc.DepthOrArraySize : 1),
            (UINT)(is3DTexture ? 1 : desc.DepthOrArraySize));

        return InitInternal(createArgs.GetCreateArgs(), true, std::move(pResource), DoNotCreateAsTypelessResource);
    }

    HRESULT Resource::InitInternal(_In_ D3DDDIARG_CREATERESOURCE2 &createArgs, bool onlyFirstSurfInitialized, unique_comptr<D3D12TranslationLayer::Resource> pAlreadyCreatedResource, bool DoNotCreateAsTypelessResource)
    {
#if DBG
        m_d3d9Desc = createArgs;
#endif

        if (createArgs.pSurfList == nullptr) return E_INVALIDARG;

        m_compatOptions.Init((D3DFORMAT)createArgs.Format);

        m_isD3D9SystemMemoryPool = createArgs.Pool == D3DDDIPOOL_SYSTEMMEM;

        ID3D12Device& d3dDevice = m_pParentDevice->GetDevice();

        const D3DDDI_SURFACEINFO &SurfList = *createArgs.pSurfList;
        D3D12_RESOURCE_DIMENSION dimension = ConvertToD3D12Dimension(createArgs.Flags, SurfList.Width, SurfList.Height, SurfList.Depth);

        if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
            (createArgs.pSurfList[0].Width > MAX_TEXTURE_WIDTH_SIZE ||
            createArgs.pSurfList[0].Height > MAX_TEXTURE_HEIGHT_SIZE))
        {
            return E_INVALIDARG;
        }

        const bool bIsDisplayable = createArgs.Flags.Primary;
        m_dx9Format = createArgs.Format == D3DDDIFMT_UNKNOWN ? GetDefaultFormat(createArgs.Flags) : ConvertDDIFormatToAPI(createArgs.Format);
        // X8R8G8B8 is not a DISPLAYABLE format in DX12, but we can emulate it with the compatible D3DFMT_A8R8G8B8
        if (bIsDisplayable && m_dx9Format == D3DFMT_X8R8G8B8)
        {
            m_dx9Format = D3DFMT_A8R8G8B8;
            m_DisableAlphaChannel = true;
        }
        m_VidPnSourceId = bIsDisplayable ? createArgs.VidPnSourceId : D3DDDI_ID_UNINITIALIZED;

        m_ViewFormat = ConvertToDXGIFORMAT(m_dx9Format);
        m_IsLockable = IsLockable(m_dx9Format) && !createArgs.Flags.NotLockable;
        m_NonOpaquePlaneCount = (UINT8)CD3D11FormatHelper::NonOpaquePlaneCount(m_ViewFormat);
        m_cubeMap = createArgs.Flags.CubeMap;

        UINT mssSampleCount, mssQualityLevel = 0;
        if (D3DDDIMULTISAMPLE_NONE == createArgs.MultisampleType)
        {
            mssSampleCount = 1;
        }
        else if (D3DDDIMULTISAMPLE_NONMASKABLE == createArgs.MultisampleType)
        {
            mssSampleCount = 2 << createArgs.MultisampleQuality;
        }
        else
        {
            mssSampleCount = createArgs.MultisampleType;
            mssQualityLevel = createArgs.MultisampleQuality;
        }

        UINT16 numMips = static_cast<UINT16>(max(createArgs.MipLevels, 1u));
        assert(createArgs.SurfCount % numMips == 0);
        m_numArrayLevels = static_cast<UINT8>(createArgs.SurfCount / numMips);
        
        // From MSDN: To generate a mipmap automatically, set a new usage D3DUSAGE_AUTOGENMIPMAP 
        // before calling CreateTexture.Sublevel generation from this point on is completely 
        // transparent to the application.Only the top texture level is accessible to the application;
        // https://msdn.microsoft.com/en-us/library/windows/desktop/bb172340(v=vs.85).aspx

        // If autogenmipmaps is set, we're expected to allocate a full mipchain
        // but the app gets to pretend like the mip levels don't exist. This requires
        // us to convert any subresource indexes we get from the app to account for the
        // "hidden" mip levels
        m_mipMapsHiddenFromApp = createArgs.Flags.AutogenMipmap;
        if (m_mipMapsHiddenFromApp)
        {
            assert(createArgs.MipLevels == 0 || createArgs.MipLevels == 1);
            assert(NeedsShaderResourceView(createArgs.Flags, createArgs.Pool));
            numMips = MaxMipLevels(max(max(SurfList.Width, SurfList.Height), SurfList.Depth));
        }

        UINT16 depth = static_cast<UINT16>(max(SurfList.Depth, 1u));
        const UINT64 width = max(SurfList.Width, 1u);
        const UINT height = max(SurfList.Height, 1u);

        m_desc = CD3DX12_RESOURCE_DESC(
            dimension,
            0,
            width,
            height,
            dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? depth : m_numArrayLevels,
            numMips,
            m_ViewFormat,
            mssSampleCount,
            mssQualityLevel,
            ConvertToD3D12TextureLayout(dimension, createArgs.Flags2),
            ConvertToD3D12ResourceFlags(createArgs.Flags, createArgs.Flags2, createArgs.Pool));

        m_numSubresources = D3D12CalcSubresource(
            numMips - 1, 
            m_numArrayLevels - 1, 
            m_NonOpaquePlaneCount - 1, 
            numMips, 
            m_numArrayLevels) 
                + 1;

        m_lockData.resize(m_numSubresources);
        m_appLinearRepresentation.m_systemData.resize(m_numSubresources);
        m_physicalLinearRepresentation.m_footprints.resize(m_numSubresources);
        m_physicalLinearRepresentation.m_rowPitces.resize(m_numSubresources);
        m_physicalLinearRepresentation.m_numRows.resize(m_numSubresources);

        m_totalSize = 0;

        m_logicalDesc = m_desc;

        // If the format has an SRGB equivalent, make the format typeless so that we can make SRVs with both the 
        // normal format and it's SRGB variant for gamma correction
        // Note: We avoid making everything typeless because it makes it difficult to derive the original API format
        // when sharing this resource across processes. We could work around this by storing extra data in the 
        // kernel handle but for now this is the simplest approach
        const bool GenerateSRGBViews = IsSRGBCompatible(m_ViewFormat) && !DoNotCreateAsTypelessResource;
        const bool IsBothDepthStencilAndSRV = NeedsDepthStencil(createArgs.Flags, createArgs.Pool) && NeedsShaderResourceView(createArgs.Flags, createArgs.Pool);
        const bool bNeedsToBeTypeless = !GetParent()->GetAdapter().SupportsCastingTypelessResources() && (GenerateSRGBViews || IsBothDepthStencilAndSRV);

        // API requires this resource to be typeless so we better not request this to avoid typeless
        Check9on12(!IsBothDepthStencilAndSRV || !DoNotCreateAsTypelessResource);

        DXGI_FORMAT resourceFormat = bNeedsToBeTypeless ? ConvertToTypeless(m_ViewFormat) : m_ViewFormat;

        m_desc.Format = resourceFormat;

        d3dDevice.GetCopyableFootprints(
            &m_desc,
            0,
            m_numSubresources,
            0,
            &m_physicalLinearRepresentation.m_footprints[0],
            &m_physicalLinearRepresentation.m_numRows[0],
            &m_physicalLinearRepresentation.m_rowPitces[0],
            &m_totalSize);

        // We can safely skip initializing app memory information if only the first surf is 
        // initialized because this only happens as when we internally create this resource 
        // and won't be working with any app initial memory
        if (!onlyFirstSurfInitialized)
        {
            for (UINT planeIndex = 0; planeIndex < m_NonOpaquePlaneCount; planeIndex++)
            {
                for (UINT appSubresourceIndex = 0; appSubresourceIndex < createArgs.SurfCount; appSubresourceIndex++)
                {
                    UINT physicalSubresourceIndex = createArgs.SurfCount * planeIndex + appSubresourceIndex;

                    UINT pitch = createArgs.pSurfList[appSubresourceIndex].SysMemPitch;
                    UINT subresourceStartOffset = 0;

                    if (IsChromaPlane(planeIndex))
                    {
                        UINT subsampleX, subsampleY = 0;
                        CD3D11FormatHelper::GetYCbCrChromaSubsampling(m_ViewFormat, subsampleX, subsampleY);

                        UINT lumaPlaneHeight = createArgs.pSurfList[appSubresourceIndex].Height;
                        UINT chromaPlaneHeight = lumaPlaneHeight / subsampleY;
                        UINT numChromaPlanes = planeIndex - 1;

                        subresourceStartOffset = lumaPlaneHeight * pitch + numChromaPlanes * chromaPlaneHeight;
                    }

                    m_appLinearRepresentation.m_systemData[physicalSubresourceIndex].pData = (BYTE *)createArgs.pSurfList[appSubresourceIndex].pSysMem + subresourceStartOffset;
                    m_appLinearRepresentation.m_systemData[physicalSubresourceIndex].RowPitch = pitch;
                    m_appLinearRepresentation.m_systemData[physicalSubresourceIndex].SlicePitch = createArgs.pSurfList[appSubresourceIndex].SysMemSlicePitch;

                    // Because the DX9 runtime has no insight on IHV formats, it isn't aware that the 
                    // resource is block compressed and will treat this as a regular texture without correctly
                    // dividing the total number of rows by 4 like it would for the officially supported DXT formats.
                    // To work around this, IHVs pass in an arbitrary BPP for these formats (this seems to always be 8) 
                    // and internally correct the pitch reported by the runtime with a multiplier.
                    // More details in 9on12Caps.h with the caps inclusion of D3DFMT_ATI1/D3DFMT_ATI2
                    if (IsIHVFormat(createArgs.Format) && IsBlockCompressedFormat(resourceFormat))
                    {
                        C_ASSERT(BPP_FOR_IHV_BLOCK_COMPRESSED_FORMATS % 8 == 0);
                        const UINT runtimeCalculatedPitch = GetBlockWidth(resourceFormat) * BPP_FOR_IHV_BLOCK_COMPRESSED_FORMATS / 8;
                        const UINT actualPitch = GetBytesPerUnit(resourceFormat);

                        assert(actualPitch % runtimeCalculatedPitch == 0);
                        const UINT pitchMultiplier = actualPitch / runtimeCalculatedPitch;

                        m_appLinearRepresentation.m_systemData[physicalSubresourceIndex].RowPitch *= pitchMultiplier;
                    }
                }
            }
        }


        // Store information about how we represent the resource as a linear buffer
        if (IsSystemMemory())
        {
            if (dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                // What they really want is a buffer
                m_desc = CD3DX12_RESOURCE_DESC::Buffer(m_totalSize);
            }
        }

        if (createArgs.Flags.MightDrawFromLocked)
        {
            // Do nothing, resources that are both lockable and buffers will automatically
            // become upload buffers. It's perfectly valid to read an locked vertex buffer
        }

        if (createArgs.Flags.DecodeCompressedBuffer)
        {
            m_isDecodeCompressedBuffer = true;
        }

        HRESULT hr = S_OK;
        if (m_isD3D9SystemMemoryPool == false)
        {
            UINT bindFlags = {};

            if (IsDecoderSurface(createArgs.Flags))
            {
                bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_DECODER;
            }

            if (NeedsRenderTarget(createArgs.Flags, createArgs.Pool))
            {
                bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_RENDER_TARGET;
            }

            if (NeedsDepthStencil(createArgs.Flags, createArgs.Pool))
            {
                bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_DEPTH_STENCIL;
            }

            // Even if it doesn't need an SRV, mark this as an SRV so we have the option of making an SRV in case this resource is used as the 
            // source of a StretchRect
            if (NeedsShaderResourceView(createArgs.Flags, createArgs.Pool) || m_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
            {
                bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_SHADER_RESOURCE;
            }

            if (createArgs.Flags.IndexBuffer)
            {
                bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_INDEX_BUFFER;
            }

            if (createArgs.Flags.VertexBuffer)
            {
                bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_VERTEX_BUFFER;
            }

            if (m_IsLockable)
            {
                m_CpuAccessFlags |= D3D12TranslationLayer::RESOURCE_CPU_ACCESS_WRITE;
                // Note: without supporting the R2VB extension, there is no way to write
                // to D3D9 vertex or index buffers.
                if (!createArgs.Flags.WriteOnly &&
                    !createArgs.Flags.VertexBuffer &&
                    !createArgs.Flags.IndexBuffer)
                {
                    m_CpuAccessFlags |= D3D12TranslationLayer::RESOURCE_CPU_ACCESS_READ;
                }
            }

            const D3D12TranslationLayer::RESOURCE_USAGE UsageFlag = GetResourceUsage(createArgs.Flags, m_CpuAccessFlags);
            const D3D12_HEAP_TYPE HeapType = D3D12TranslationLayer::Resource::GetD3D12HeapType(UsageFlag, m_CpuAccessFlags);

            m_TranslationLayerCreateArgs.m_desc12 = m_desc;
            m_TranslationLayerCreateArgs.m_appDesc = ConvertToAppResourceDesc(m_desc,
                UsageFlag,
                (D3D12TranslationLayer::RESOURCE_CPU_ACCESS)m_CpuAccessFlags,
                (D3D12TranslationLayer::RESOURCE_BIND_FLAGS)bindFlags);
            m_TranslationLayerCreateArgs.m_heapDesc = CD3DX12_HEAP_DESC(m_pParentDevice->GetDevice().GetResourceAllocationInfo(0, 1, &m_desc),
                CD3DX12_HEAP_PROPERTIES(HeapType),
                createArgs.Flags.SharedResource ? D3D12_HEAP_FLAG_SHARED : D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT);
            m_TranslationLayerCreateArgs.m_bBoundForStreamOut = false;
            m_TranslationLayerCreateArgs.m_isPlacedTexture = 0;
            m_TranslationLayerCreateArgs.m_OffsetToStreamOutputSuffix = 0;
            m_TranslationLayerCreateArgs.m_bManageResidency = true;
            m_TranslationLayerCreateArgs.m_bIsD3D9on12Resource = true;
            if (bIsDisplayable)
            {
                m_TranslationLayerCreateArgs.m_heapDesc.Flags |= D3D12_HEAP_FLAG_ALLOW_DISPLAY;
                m_TranslationLayerCreateArgs.m_bManageResidency = false;
            }
            if (createArgs.Flags2.CrossAdapter)
            {
                assert(createArgs.Flags.SharedResource);
                m_TranslationLayerCreateArgs.m_heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE, D3D12_MEMORY_POOL_L0);
                m_TranslationLayerCreateArgs.m_heapDesc.Flags |= D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER;
                m_TranslationLayerCreateArgs.m_desc12.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                m_TranslationLayerCreateArgs.m_desc12.Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
            }

            m_TranslationLayerCreateArgs.m_flags11.BindFlags = ConvertTranslationLayerBindFlagsToD3D11BindFlags(bindFlags);
            if (createArgs.Flags.SharedResource)
            {
                // The DX9 runtime only works with GDI handles but DX12 defaults to returning NT handles,
                // therefore we need to mark this flag so that 9on12 can open handles from other 9on12 devices
                m_TranslationLayerCreateArgs.m_flags11.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
                if (createArgs.Flags2.IsDisplayable || createArgs.Flags2.CrossAdapter)
                {
                    // TODO: This info really needs to be passed to 9on12 via side-channel rather than us guessing it
                    m_TranslationLayerCreateArgs.m_flags11.MiscFlags |= D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
                }
                if (createArgs.Flags2.IsDisplayable)
                {
                    m_TranslationLayerCreateArgs.m_bTriggerDeferredWaits = true;
                }
            }

            if (createArgs.Flags.DecodeRenderTarget)
            {
                m_TranslationLayerCreateArgs.m_desc12.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
            }

            if (m_dx9Format == D3DFMT_YV12)
            {
                m_TranslationLayerCreateArgs.m_FormatEmulation = D3D12TranslationLayer::FormatEmulation::YV12;
            }

            if (pAlreadyCreatedResource)
            {
                m_pResource = std::move(pAlreadyCreatedResource);
            }
            else
            {
                CreateUnderlyingResource(m_TranslationLayerCreateArgs, createArgs);
            }

            if (SUCCEEDED(hr))
            {
                // DXGI swapchain resources aren't created as typeless resources so we can't
                // make a view with the SRGB variant of the resource format
                if (NeedsRenderTarget(createArgs.Flags, createArgs.Pool))
                {
                    CreateRenderTargetViews(m_ViewFormat, GenerateSRGBViews);
                }

                if (NeedsDepthStencil(createArgs.Flags, createArgs.Pool))
                {
                    CreateDepthStencilViews();
                }

                if (NeedsShaderResourceView(createArgs.Flags, createArgs.Pool))
                {
                    m_shaderResourceViews[ShaderResourceViewFlags::None] = std::unique_ptr<D3D12TranslationLayer::SRV>(
                        CreateShaderResourceView(ShaderResourceViewFlags::None));
                    m_srvTextureType = ConvertShaderResourceDimensionToTextureType(m_shaderResourceViews[ShaderResourceViewFlags::None]->GetDesc12().ViewDimension);

                    if (GenerateSRGBViews)
                    {
                        m_shaderResourceViews[ShaderResourceViewFlags::SRGBEnabled] = std::unique_ptr<D3D12TranslationLayer::SRV>(
                            CreateShaderResourceView(ShaderResourceViewFlags::SRGBEnabled));
                    }
                }

                if (NeedsVideoDecoderOutputView(createArgs.Flags, createArgs.Pool))
                {
                    CreateVideoDecoderOutputViews();
                }

                if (NeedsVideoProcessorOutputView(createArgs.Flags, createArgs.Pool))
                {
                    CreateVideoProcessorOutputViews();
                }
            }
        }
        else
        {
            m_pResource = nullptr;
        }

        return hr;
    }

    HRESULT Resource::OpenResource(D3DKMT_HANDLE kmtHandle, void *pPrivateResourceDriverData, UINT privateResourceDriverDataSize, D3D9ON12_OPEN_RESOURCE_ARGS *pD3d9on12Args)
    {
        HRESULT hr = S_OK;

        D3D12TranslationLayer::ResourceInfo resourceInfo;
        GetParent()->GetSharedResourceHelper().QueryResourceInfoFromKMTHandle(kmtHandle, nullptr, &resourceInfo);

        Check9on12(resourceInfo.m_Type == D3D12TranslationLayer::ResourceInfoType::ResourceType);
        D3D12TranslationLayer::ResourceCreationArgs CreateArgs = GetCreateArgsFromExistingResource(&GetParent()->GetDevice(), resourceInfo.Resource.m_ResourceDesc);

        auto spResource = GetParent()->GetSharedResourceHelper().OpenResourceFromKmtHandle(
            CreateArgs,
            kmtHandle,
            pPrivateResourceDriverData,
            privateResourceDriverDataSize,
            D3D12_RESOURCE_STATE_COMMON);
        Init(std::move(spResource), pD3d9on12Args, true);

        return hr;
    }

    void Resource::ClearAllPlanesInArraySlice(_In_ UINT arrayIndex, _In_ const RECT* dstRects, _In_ UINT numRects, _In_ D3DCOLOR  Color)
    {
        if (m_videoDecoderOutputViews.size() > 0)
        {
            Check9on12(m_videoDecoderOutputViews[arrayIndex] != nullptr);
            DirectX::XMFLOAT4 col = ARGBToUNORMFloat(Color);

            m_pParentDevice->GetContext().ClearVideoDecoderOutputView(m_videoDecoderOutputViews[arrayIndex].get(), (float*)&col, numRects, dstRects);
        }
        else if (m_videoProcessorOutputViews.size() > 0)
        {
            Check9on12(m_videoProcessorOutputViews[arrayIndex] != nullptr);
            DirectX::XMFLOAT4 col = ARGBToUNORMFloat(Color);

            m_pParentDevice->GetContext().ClearVideoProcessorOutputView(m_videoProcessorOutputViews[arrayIndex].get(), (float*)&col, numRects, dstRects);
        }
        else
        {
            // Not supporting clears used on planar resources that aren't marked for video yet
            Check9on12(m_renderTargetViews.size() == 0);
        }
    }

    HRESULT Resource::Clear(_In_ UINT SubResourceIndex, _In_ const RECT *dstRects, _In_ UINT numRects, _In_ D3DCOLOR  Color)
    {
        DirectX::XMFLOAT4 col = ARGBToUNORMFloat(Color);
        if (NeedsSwapRBOutputChannels())
        {
            std::swap(col.x, col.z);
        }
        if (m_renderTargetViews.size() > 0)
        {
            Check9on12(m_renderTargetViews[SubResourceIndex] != nullptr);

            // From MSDN: D3DRS_SRGBWRITEENABLE is honored while performing a clear of the render target
            bool srgbEnabled = m_pParentDevice->GetPipelineState().GetPixelStage().GetSRGBWriteEnable();
            m_pParentDevice->GetContext().ClearRenderTargetView(GetRenderTargetView(SubResourceIndex, srgbEnabled), (float*)&col, numRects, dstRects);
        }
        
        else
        {
            m_pParentDevice->GetContext().ClearResourceWithNoRenderTarget(GetUnderlyingResource(), (float*)&col, numRects, dstRects, SubResourceIndex, SubResourceIndex, m_ViewFormat);
        }

        return S_OK;
    }

    HRESULT Resource::ClearDepthStencil(UINT flag, _In_ const RECT *dstRects, _In_ UINT numRects, float depthValue, UINT stencilValue)
    {
        Check9on12(flag & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL));
        Check9on12(m_pDepthStencilViews[D3D12_DSV_FLAG_NONE] != nullptr);

        D3D12_CLEAR_FLAGS clearFlag = {};
        if (flag & D3DCLEAR_ZBUFFER)
        {
            clearFlag |= D3D12_CLEAR_FLAG_DEPTH;
        }

        if (flag & D3DCLEAR_STENCIL)
        {
            clearFlag |= D3D12_CLEAR_FLAG_STENCIL;
        }

        m_pParentDevice->GetContext().ClearDepthStencilView(m_pDepthStencilViews[D3D12_DSV_FLAG_NONE], clearFlag, depthValue, static_cast<UINT8>(stencilValue), numRects, dstRects);

        return S_OK;
    }

    UINT Resource::ConvertAppSubresourceToDX12Subresource(UINT appIndex)
    {
        if (m_NonOpaquePlaneCount > 1)
        {
            // This should never get called in cases where the resource could be planar.
            // ConvertAppSubresourceToDX12SubresourceIndices should be used instead and the 
            // caller should be modified to handle planar subresources
            Check9on12(false);
            return (UINT)-1;
        }

        return ConvertAppSubresourceToDX12SubresourceInternal(appIndex, 0, false);
    }

    void Resource::ConvertAppSubresourceToDX12SubresourceIndices(
        _In_ UINT appIndex,
        _Out_writes_(MAX_PLANES) UINT *pSubresourceIndices,
        _Out_range_(0, MAX_PLANES) UINT &planeCountToMap,
        bool bMipAlreadyExpanded)
    {
        planeCountToMap = m_NonOpaquePlaneCount;
        Check9on12(planeCountToMap <= MAX_PLANES);
        ASSUME(planeCountToMap <= MAX_PLANES);

        for (UINT i = 0; i < planeCountToMap; i++)
        {
            pSubresourceIndices[i] = ConvertAppSubresourceToDX12SubresourceInternal(appIndex, i, bMipAlreadyExpanded);
        }
    }

    UINT Resource::ConvertAppSubresourceToDX12SubresourceInternal(UINT appIndex, UINT planeIndex, bool bMipAlreadyExpanded)
    {
        // Calculate subresource for plane 0, then use D3D12CalcSubresource to account for plane Index
        UINT subresource = appIndex;
        if (m_mipMapsHiddenFromApp && !bMipAlreadyExpanded)
        {
            subresource *= m_logicalDesc.MipLevels;
        }

        return D3D12CalcSubresource(
            GetMipLevelFromSubresourceIndex(subresource), 
            GetArraySliceFromSubresourceIndex(subresource), 
            planeIndex, 
            m_logicalDesc.MipLevels, 
            GetArraySize());
    }


    HRESULT Resource::Lock(Device &device,
        _In_ UINT subresourceIndex,
        _In_ D3DDDI_LOCKFLAGS flags,
        _In_ LockRange lockRange,
        _In_ bool bAsyncLock,
        _Out_ VOID* &pSurfData,
        _Out_ UINT &pitch,
        _Out_ UINT &slicePitch,
        _Out_opt_ HANDLE *phCookie)
    {
        Check9on12(m_IsLockable);
        //TODO: When they do a lock discard combined with a 'MightDrawFromLocked' flag
        //      we might have to version the buffer.
        if (flags.MightDrawFromLocked && flags.Discard)
        {
            PrintDebugMessage("Warning: Lock with flags 'MightDrawFromLocked' and 'Discard'. This is not implemented properly");
        }

        if (flags.NotifyOnly)
        {
            Check9on12(IsSystemMemory());
            return S_OK;
        }

        if (m_lockData[subresourceIndex].m_LockCount == 0)
        {
            m_lockData[subresourceIndex].m_Flags = flags;
        }
        else
        {
            // DX9 has a strange pattern where it allows recursive locks, but
            // recursive locks are expected to ignore what the runtime passed
            // in and stick with the properties set by the first lock call
            flags = m_lockData[subresourceIndex].m_Flags;
        }

        Check9on12(!IsSystemMemory()); // System memory should only use notify only

        D3D12TranslationLayer::MAP_TYPE mapType = D3D12TranslationLayer::MAP_TYPE_READ;

        // In the async case, we need to make sure that we don't touch m_pResource's identity,
        // as this could be changing out from under us. However, m_pResource is safe to query
        // information from. Instead of using m_pResource's identity, use m_LastRenamedResource.
        D3D12TranslationLayer::SafeRenameResourceCookie pCookieProtector(device.GetContext());
        if (bAsyncLock)
        {
            // TODO: (11369816) Not supporting lock async with non-buffers or CPU readable resources
            if ((!flags.Discard && !flags.NoOverwrite) ||
                (m_CpuAccessFlags & D3D12TranslationLayer::RESOURCE_CPU_ACCESS_READ) ||
                m_logicalDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                // Not supporting LockAsync operations that would require synchronization
                return E_NOTIMPL;
            }

            // Taken from the dx9 runtime:
            // D3DLOCK_DISCARD is allowed only on level 0 (the top mip level). DISCARD in this case will discard the entire mipmap.
            // Additionally, since we're only supporting buffers at the moment, only subresource 0 should make it this far.
            Check9on12(subresourceIndex == 0);
            if (flags.Discard)
            {
                Check9on12(phCookie != nullptr);

                // Only rename on the first lock, recursive locks should just use the last renamed resource
                if (m_lockData[subresourceIndex].m_LockCount == 0)
                {
                    pCookieProtector.Reset(device.GetContext().CreateRenameCookie(m_pResource.get(), D3D12TranslationLayer::ResourceAllocationContext::FreeThread));
                    *phCookie = pCookieProtector.Get();

                    m_LastRenamedResource = pCookieProtector.Get()->GetIdentity()->m_suballocation;
                }
            }
            else if (m_LastRenamedResource.GetResource() == nullptr)
            {
                return E_NOTIMPL; // Don't fall through to reading m_pResource
            }
            else
            {
                Check9on12(flags.NoOverwrite == true);
            }
        }

        bool bSubrangeLocked = IsLockingSubrange<D3DDDI_LOCKFLAGS>(flags);
        D3D12_BOX *pBox = nullptr;
        if (bSubrangeLocked)
        {
            pBox = &m_lockData[subresourceIndex].m_Box;
            if (flags.RangeValid)
            {
                *pBox = CD3DX12_BOX(lockRange.Range.Offset, lockRange.Range.Size + lockRange.Range.Offset);
            }
            else if (flags.AreaValid)
            {
                *pBox = CD3DX12_BOX(lockRange.Area.left, lockRange.Area.top, lockRange.Area.right, lockRange.Area.bottom);
            }
            else if (flags.BoxValid)
            {
                *pBox = CD3DX12_BOX(lockRange.Box.Left, lockRange.Box.Top, lockRange.Box.Front, lockRange.Box.Right, lockRange.Box.Bottom, lockRange.Box.Back);
            }
            // Validation of ranges/rects/boxes is debug-only in 9, so apps can pass whatever they want and get away with it.
            // Sanitize it here.
            pBox->left = min(pBox->left, (UINT)m_logicalDesc.Width);
            pBox->right = min(pBox->right, (UINT)m_logicalDesc.Width);
            pBox->top = min(pBox->top, m_logicalDesc.Height);
            pBox->bottom = min(pBox->bottom, m_logicalDesc.Height);
            // The front/back should be 0/1 for non-volumes anyway, which is in the range of 0->ArraySize.
            pBox->front = min(pBox->front, (UINT)m_logicalDesc.DepthOrArraySize);
            pBox->back = min(pBox->back, (UINT)m_logicalDesc.DepthOrArraySize);
        }

        mapType = GetMapTypeFlag(flags.ReadOnly, flags.WriteOnly, flags.Discard, flags.NoOverwrite, m_isDecodeCompressedBuffer, m_pResource->AppDesc()->CPUAccessFlags());

        if (RegistryConstants::g_cLockDiscardOptimization)
        {
            if (mapType == D3D12TranslationLayer::MAP_TYPE_WRITE_DISCARD)
            {
                // Range not valid means the whole resource is mapped
                if(!flags.RangeValid)
                {
                    lockRange.Range.Offset = 0;
                    assert(this->m_totalSize <= UINT_MAX); // For casting below
                    lockRange.Range.Size = (UINT) this->m_totalSize;
                }

                // Check if buffer is in the map
                // If it is, get a list of previously mapped ranges

                auto lockedResourceRanges = device.m_lockedResourceRanges.GetLocked();
                auto it = lockedResourceRanges->find(this);
                if (it != lockedResourceRanges->end()) {

                    // it->second has the mapped ranges vector corresponding to it->first == this

                    bool overlapsWithPreviouslyMappedRanges = false;
                    bool mergedWithExistingMappedRange = false;

                    UINT currentRangeStart = lockRange.Range.Offset;
                    UINT currentRangeEnd = lockRange.Range.Offset + lockRange.Range.Size;

                    // Compare current range with previously mapped ranges
                    for (auto& mappedRange : it->second)
                    {
                        UINT mappedRangeStart = mappedRange.Range.Offset;
                        UINT mappedRangeEnd = mappedRange.Range.Offset + mappedRange.Range.Size;

                        if (((currentRangeStart >= mappedRangeStart) && (currentRangeStart < mappedRangeEnd)) ||
                            ((currentRangeEnd >= mappedRangeStart) && (currentRangeEnd < mappedRangeEnd)))
                        {
                            overlapsWithPreviouslyMappedRanges = true;
                            break;
                        }

                        // If the current range is a neighbor of an existing mapped range, merge it with the neighbor
                        if (currentRangeStart == mappedRangeEnd)
                        {
                            mappedRange.Range.Size += lockRange.Range.Size;

                            mergedWithExistingMappedRange = true;
                            break;
                        }
                        if (currentRangeEnd == mappedRangeStart)
                        {
                            mappedRange.Range.Offset = lockRange.Range.Offset;
                            mappedRange.Range.Size += lockRange.Range.Size;

                            mergedWithExistingMappedRange = true;
                            break;
                        }
                    }

                    // If the newly mapped range doesn't intersect previously mapped ranges - add it to the list and change flag to NO_OVERWRITE
                    if (!overlapsWithPreviouslyMappedRanges)
                    {
                        mapType = D3D12TranslationLayer::MAP_TYPE_WRITE_NOOVERWRITE;
                    }
                    else // Else clear the list of ranges, add the current range, and keep the DISCARD flag
                    {
                        it->second.clear();
                    }

                    if (!mergedWithExistingMappedRange)
                    {
                        it->second.push_back(lockRange);
                    }
                }
                else
                {
                    std::vector<LockRange> lockedRange = { lockRange };
                    lockedResourceRanges->insert({ this, lockedRange });
                }
            }
        }

        if (bAsyncLock)
        {
            CD3DX12_RANGE ReadRange(0, 0);
            void* pMappedData;
            m_LastRenamedResource.Map(0, &ReadRange, &pMappedData);

            assert(subresourceIndex == 0); // Means pointer doesn't need to be offset beyond suballocation.
            assert(m_logicalDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);
            pSurfData = (byte*)pMappedData + (pBox ? pBox->left : 0u);
            pitch = m_pResource->GetSubresourcePlacement(subresourceIndex).Footprint.RowPitch;
            slicePitch = static_cast<UINT>(m_pResource->DepthPitch(subresourceIndex));
        }
        else
        {
            D3D12TranslationLayer::MappedSubresource mappedData = {};
            bool bResourceMapped = device.GetContext().Map(m_pResource.get(), subresourceIndex, mapType, flags.DoNotWait, pBox, &mappedData);
            if (!bResourceMapped)
            {
                return D3DERR_WASSTILLDRAWING;
            }
            pSurfData = (byte*)mappedData.pData;
            pitch = static_cast<UINT>(mappedData.RowPitch);
            slicePitch = static_cast<UINT>(mappedData.DepthPitch);
        }

        //We had better of got some data to return
        Check9on12(pSurfData);
         
        if (mapType != D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_READ)
        {
            NotifyResourceChanged();
        }

        // No more exceptions
        m_lockData[subresourceIndex].m_LockCount++;
        pCookieProtector.Detach();
        return S_OK;
    }

    void Resource::Unlock(Device &device, UINT subresourceIndex, D3DDDI_UNLOCKFLAGS flags, bool bAsyncUnlock)
    { 
        if (flags.NotifyOnly)
        {
            Check9on12(IsSystemMemory());
            return;
        }
        Check9on12(m_lockData[subresourceIndex].m_LockCount > 0);
        m_lockData[subresourceIndex].m_LockCount--;

        const D3DDDI_LOCKFLAGS &lockFlags = m_lockData[subresourceIndex].m_Flags;
        bool bLockingSubrange = IsLockingSubrange<D3DDDI_LOCKFLAGS>(lockFlags);
        const D3D12_BOX *pReadWriteBox = (bLockingSubrange) ? &m_lockData[subresourceIndex].m_Box : nullptr;
        
        // In the async case, we need to make sure that we don't touch m_pResource's identity,
        // as this could be changing out from under us. However, m_pResource is safe to query
        // information from. Instead of using m_pResource's identity, use m_LastRenamedResource.
        D3D12TranslationLayer::Resource *pResourceToUnmap = m_pResource.get();
        if (bAsyncUnlock && m_LastRenamedResource.GetResource())
        {
            D3D12_RANGE WrittenRange = CD3DX12_RANGE(0u, static_cast<SIZE_T>(m_desc.Width));
            m_LastRenamedResource.Unmap(0, &WrittenRange);
        }
        else
        {
            D3D12TranslationLayer::MAP_TYPE mapType = GetMapTypeFlag(lockFlags.ReadOnly, lockFlags.WriteOnly, lockFlags.Discard, lockFlags.NoOverwrite, m_isDecodeCompressedBuffer, pResourceToUnmap->AppDesc()->CPUAccessFlags());
            device.GetContext().Unmap(pResourceToUnmap, subresourceIndex, mapType, pReadWriteBox);
        }
    }

    HRESULT Resource::PreBind()
    {
        return S_OK;
    }

    void Resource::GetTriFanIB(_Out_ InputBuffer& convertedIB, _In_ UINT indexOffsetInBytes, _In_ UINT indexCount, _In_ UINT ibStride)
    {
        if (IsTriangleFanIBCacheDirty(indexOffsetInBytes))
        {
            D3D12TranslationLayer::Resource *pMappableIndexBuffer = GetUnderlyingResource();

            // If the index buffer doesn't have CPU read access, allocate a staging resource and copy the index buffer
            // to the staging resource
            unique_comptr<D3D12TranslationLayer::Resource> pReadbackBuffer;
            if ((GetUnderlyingResource()->AppDesc()->CPUAccessFlags() & D3D12TranslationLayer::RESOURCE_CPU_ACCESS_READ) == 0)
            {
                pReadbackBuffer = std::move(GetStagingCopy());
                pMappableIndexBuffer = pReadbackBuffer.get();
            }

            // Read from GPU and set a flag to unmap this after the draw operations
            D3D12TranslationLayer::MappedSubresource MappedResult;
            m_pParentDevice->GetContext().Map(pMappableIndexBuffer, 0, D3D12TranslationLayer::MAP_TYPE_READ, false, nullptr, &MappedResult);
            const void* pSrcIndexBuffer = (void *)MappedResult.pData;

            D3D9on12::InputAssembly::CreateTriangleListIBFromTriangleFanIB(*m_pParentDevice, pSrcIndexBuffer, ibStride, indexOffsetInBytes, indexCount, convertedIB);
            UpdateTriangleFanIBCache(convertedIB, indexOffsetInBytes);

            m_pParentDevice->GetContext().Unmap(pMappableIndexBuffer, 0, D3D12TranslationLayer::MAP_TYPE_READ, nullptr);
        }
        else
        {
            // We can use the previously stored converted index buffer since the resource hasn't changed since the last conversion.
            convertedIB = m_TriFanIBCache.m_TriangleFanConvertedTriangleListIndexBuffer;
        }
    }

    bool Resource::IsTriangleFanIBCacheDirty(UINT baseIndexLocation)
    {
        return  (m_TriFanIBCache.m_dirtyFlag) ||
            (baseIndexLocation != m_TriFanIBCache.m_baseIndexOffsetInBytes);
    }

    void Resource::UpdateTriangleFanIBCache(InputBuffer& newIBToStore, UINT indexOffsetInBytes)
    {
        m_TriFanIBCache.m_TriangleFanConvertedTriangleListIndexBuffer = newIBToStore;
        m_TriFanIBCache.m_baseIndexOffsetInBytes = indexOffsetInBytes;
        m_TriFanIBCache.m_dirtyFlag = false;
    }

    void Resource::NotifyResourceChanged()
    {
        m_TriFanIBCache.m_baseIndexOffsetInBytes = 0;
        m_TriFanIBCache.m_dirtyFlag = true;
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT &Resource::GetSubresourceFootprint(UINT subresource)
    {
        Check9on12(subresource < m_physicalLinearRepresentation.m_footprints.size());
        return m_physicalLinearRepresentation.m_footprints[subresource];
    }

    HRESULT Resource::GenerateMips(D3DDDITEXTUREFILTERTYPE filterType)
    {
        Check9on12(GetShaderResourceView() != nullptr);

        D3D12_FILTER_TYPE filterD3D12 = ConvertD3D9GenMipsFilterType(filterType);
        m_pParentDevice->GetContext().GenMips(GetShaderResourceView(), filterD3D12);

        return S_OK;
    }

    ShaderConv::TEXTURETYPE Resource::GetShaderResourceTextureType() {
        Check9on12(GetShaderResourceView() != nullptr);

        return m_srvTextureType;
    }

    unique_comptr<D3D12TranslationLayer::Resource> Resource::GetStagingCopy()
    {
        D3D12TranslationLayer::ResourceCreationArgs args = *GetUnderlyingResource()->Parent();
        args.m_heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        D3D12TranslationLayer::AppResourceDesc &origAppDesc = args.m_appDesc;
        args.m_appDesc = D3D12TranslationLayer::AppResourceDesc(
            origAppDesc.SubresourcesPerPlane(),
            origAppDesc.NonOpaquePlaneCount(),
            origAppDesc.Subresources(),
            origAppDesc.MipLevels(),
            origAppDesc.ArraySize(),
            origAppDesc.Depth(),
            origAppDesc.Width(),
            origAppDesc.Height(),
            origAppDesc.Format(),
            origAppDesc.Samples(),
            origAppDesc.Quality(),
            D3D12TranslationLayer::RESOURCE_USAGE_STAGING,
            D3D12TranslationLayer::RESOURCE_CPU_ACCESS_READ,
            (D3D12TranslationLayer::RESOURCE_BIND_FLAGS)0,
            origAppDesc.ResourceDimension());
            

        auto pResource = D3D12TranslationLayer::Resource::CreateResource(
            &m_pParentDevice->GetContext(), args, D3D12TranslationLayer::ResourceAllocationContext::ImmediateContextThreadTemporary);

        m_pParentDevice->GetContext().ResourceCopy(pResource.get(), GetUnderlyingResource());
        return std::move(pResource);
    }

    Resource *Resource::GetBackingPlainResource(bool bDoNotCreateAsTypelessResource)
    {
        const bool isVolumeTexture = (m_logicalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        if (!m_pBackingShaderResource)
        {
            HANDLE tempHandle = {};
            m_pBackingShaderResource = new Resource(tempHandle, m_pParentDevice);

            UINT depth = isVolumeTexture ? m_logicalDesc.DepthOrArraySize : 1;
            UINT arraySize = isVolumeTexture ? 1 : m_logicalDesc.DepthOrArraySize;

            D3DDDI_RESOURCEFLAGS flags = {};
            flags.Texture = 1;
            D3DFORMAT format = m_dx9Format;
            if (NeedsSwapRBOutputChannels())
            {
                // Make the dest the underlying format so that it'll trigger
                // StretchCopy's logic for correcting channels
                format = GetFormatWithSwappedRBChannels(m_dx9Format);
            }
            m_pBackingShaderResource->Init((UINT)m_logicalDesc.Width, (UINT)m_logicalDesc.Height, format, flags, m_logicalDesc.MipLevels, depth, arraySize, bDoNotCreateAsTypelessResource);
        }

        ResourceCopyArgs copyArgs(*m_pBackingShaderResource, *this);
        if (isVolumeTexture)
        {
            D3D12_BOX srcBox = BoxFromDesc(m_logicalDesc);
            copyArgs.AsVolumeBlit(0, 0, 0, srcBox);
            CopyResource(*m_pParentDevice, copyArgs);
        }
        else
        {
            RECT dstRect = RectThatCoversEntireResource(m_pBackingShaderResource->GetDesc());
            copyArgs.AsTextureBlit({}, dstRect);
            CopyResource(*m_pParentDevice, copyArgs);
        }

        return m_pBackingShaderResource;
    }

    bool Resource::IsSRGBCompatibleTexture()
    {
        DXGI_FORMAT format = GetLogicalDesc().Format;
        return IsTypelessFormat(format) && IsSRGBCompatible(format);
    }

    D3D12TranslationLayer::SRV* Resource::GetShaderResourceView(DWORD flags)
    {
        D3D12TranslationLayer::SRV* pSRV = nullptr;

        // Apps can set SRGB mode on a texture which can't be SRGB so just return the normal view.
        if ((flags & ShaderResourceViewFlags::SRGBEnabled) && IsSRGBCompatibleTexture())
        {
            flags &= ~ShaderResourceViewFlags::SRGBEnabled;
        }

        // If there's only 1 mip, there's no need to create a special SRV for disabling mips
        if ((flags & ShaderResourceViewFlags::DisableMips) && m_logicalDesc.MipLevels == 1)
        {
            flags &= ~ShaderResourceViewFlags::DisableMips;
        }

        pSRV = m_shaderResourceViews[flags].get();
        if (!pSRV && (m_pResource->AppDesc()->BindFlags() & D3D12TranslationLayer::RESOURCE_BIND_SHADER_RESOURCE))
        {
            m_shaderResourceViews[flags] = std::unique_ptr<D3D12TranslationLayer::SRV>(CreateShaderResourceView((ShaderResourceViewFlags)flags));
            pSRV = m_shaderResourceViews[flags].get();
        }

        return pSRV;
    }

    D3D12TranslationLayer::RTV* Resource::GetRenderTargetView(UINT subresourceIndex, bool srgbEnabled)
    {
        if ((m_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) == 0)
        {
            Check9on12(m_renderTargetViews.size() == 0);
            Check9on12(m_SRGBRenderTargetViews.size() == 0);
            return nullptr;
        }
        else
        {
            // Ignore srgbEnabled if the format isn't compatible. From MSDN:
            // "Only the formats that pass the CheckDeviceFormat with the usage flag D3DUSAGE_QUERY_SRGBWRITE 
            // can be linearized. The render state D3DRS_SRGBWRITEENABLE is ignored for the rest."
            // https://msdn.microsoft.com/en-us/library/windows/desktop/bb173460(v=vs.85).aspx
            if (srgbEnabled && IsSRGBCompatible(m_desc.Format))
            {
                Check9on12(subresourceIndex < m_SRGBRenderTargetViews.size());
                return m_SRGBRenderTargetViews[subresourceIndex].get();
            }
            else
            {
                Check9on12(subresourceIndex < m_renderTargetViews.size());
                return m_renderTargetViews[subresourceIndex].get();
            }
        }
    }

    void Resource::CreateRenderTargetViews(DXGI_FORMAT rtvFormat, bool GenerateSRGBRTV)
    {
        Check9on12(m_logicalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
        const bool srgbCompatible = IsSRGBCompatible(rtvFormat);
        GenerateSRGBRTV = GenerateSRGBRTV && srgbCompatible;

        m_renderTargetViews = std::vector<std::unique_ptr<D3D12TranslationLayer::RTV>>(m_numSubresources);
        if (GenerateSRGBRTV)
        {
            m_SRGBRenderTargetViews = std::vector<std::unique_ptr<D3D12TranslationLayer::RTV>>(m_numSubresources);
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

        UINT subresourceIndex = 0;
        for (UINT arraySlice = 0; arraySlice < GetArraySize(); arraySlice++)
        {
            for (UINT mipLevel = 0; mipLevel < m_logicalDesc.MipLevels; mipLevel++)
            {
                if (m_logicalDesc.DepthOrArraySize > 1)
                {
                    if (m_logicalDesc.SampleDesc.Count > 1)
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                        rtvDesc.Texture2DMSArray.ArraySize = 1;
                        rtvDesc.Texture2DMSArray.FirstArraySlice = arraySlice;
                    }
                    else
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                        rtvDesc.Texture2DArray.MipSlice = mipLevel;
                        rtvDesc.Texture2DArray.FirstArraySlice = arraySlice;
                        rtvDesc.Texture2DArray.ArraySize = 1;
                        rtvDesc.Texture2DArray.PlaneSlice = 0;
                    }
                }
                else
                {
                    if (m_logicalDesc.SampleDesc.Count > 1)
                    {
                        assert(m_logicalDesc.MipLevels == 1);
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                        rtvDesc.Texture2D.MipSlice = mipLevel;
                        rtvDesc.Texture2D.PlaneSlice = 0;
                    }
                }

                Check9on12(subresourceIndex < m_renderTargetViews.size());

                rtvDesc.Format = rtvFormat;
                m_renderTargetViews[subresourceIndex] = std::unique_ptr<D3D12TranslationLayer::RTV>(
                    new D3D12TranslationLayer::RTV(&m_pParentDevice->GetContext(), rtvDesc, *m_pResource));

                if (GenerateSRGBRTV)
                {
                    rtvDesc.Format = ConvertToSRGB(rtvFormat);
                    m_SRGBRenderTargetViews[subresourceIndex] = std::unique_ptr<D3D12TranslationLayer::RTV>(
                        new D3D12TranslationLayer::RTV(&m_pParentDevice->GetContext(), rtvDesc, *m_pResource));
                }
                subresourceIndex++;
            }
        }
    }

    void Resource::CreateVideoDecoderOutputViews()
    {
        Check9on12(m_logicalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
        Check9on12(m_logicalDesc.MipLevels == 1);

        m_videoDecoderOutputViews = std::vector<std::unique_ptr<D3D12TranslationLayer::VDOV>>(GetArraySize());

        D3D12TranslationLayer::VIDEO_DECODER_OUTPUT_VIEW_DESC_INTERNAL viewDesc = {};
        viewDesc.Format = m_desc.Format;

        for (UINT arraySlice = 0; arraySlice < GetArraySize(); arraySlice++)
        {
            viewDesc.ArraySlice= arraySlice;
            m_videoDecoderOutputViews[arraySlice] = std::unique_ptr<D3D12TranslationLayer::VDOV>(
                new D3D12TranslationLayer::VDOV(&m_pParentDevice->GetContext(), viewDesc, *m_pResource));
        }
    }

    void Resource::CreateVideoProcessorOutputViews()
    {
        Check9on12(m_logicalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
        Check9on12(m_logicalDesc.MipLevels == 1);

        m_videoProcessorOutputViews = std::vector<std::unique_ptr<D3D12TranslationLayer::VPOV>>(GetArraySize());

        D3D12TranslationLayer::VIDEO_PROCESSOR_OUTPUT_VIEW_DESC_INTERNAL viewDesc = {};
        viewDesc.Format = m_desc.Format;
        viewDesc.MipSlice = 0;
        viewDesc.ArraySize = GetArraySize();

        for (UINT arraySlice = 0; arraySlice < GetArraySize(); arraySlice++)
        {
            viewDesc.FirstArraySlice = arraySlice;
            m_videoProcessorOutputViews[arraySlice] = std::unique_ptr<D3D12TranslationLayer::VPOV>(
                new D3D12TranslationLayer::VPOV(&m_pParentDevice->GetContext(), viewDesc, *m_pResource));
        }
    }

    D3D12TranslationLayer::DSV* & Resource::GetDepthStencilView(D3D12_DSV_FLAGS flags)
    {
        Check9on12(flags < _countof(g_cPossibleDepthStencilStates));
        // If we don't have a stencil plane, we can ignore DSV_STENCIL_READ_ONLY
        const UINT DSVIndex = FormatSupportsStencil(m_dsvFormat) ? flags : (flags & D3D12_DSV_FLAG_READ_ONLY_DEPTH);
        return m_pDepthStencilViews[DSVIndex];
    }

    void Resource::CreateDepthStencilViews()
    {
        m_dsvFormat = m_ViewFormat;

        for (size_t i = 0; i < _countof(g_cPossibleDepthStencilStates); i++)
        {
            D3D12_DSV_FLAGS flag = g_cPossibleDepthStencilStates[i];

            if ((flag & D3D12_DSV_FLAG_READ_ONLY_STENCIL) && FormatSupportsStencil(m_ViewFormat) == false)
            {
                continue;
            }
            Check9on12(m_numArrayLevels == 1); // Not supporting DSV cubemaps yet
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = m_ViewFormat;
            dsvDesc.Flags = flag;
            switch (m_logicalDesc.Dimension)
            {
                case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                {
                    if (m_logicalDesc.SampleDesc.Count > 1)
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                        dsvDesc.Texture2D.MipSlice = 0;
                    }
                    break;
                }
                case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                case D3D12_RESOURCE_DIMENSION_UNKNOWN:
                case D3D12_RESOURCE_DIMENSION_BUFFER:
                default:
                {
                    Check9on12(FALSE);
                    break;
                }
            }

            m_pDepthStencilViews[i] = new (m_depthStencilViewSpace + i * sizeof(D3D12TranslationLayer::DSV)) D3D12TranslationLayer::DSV(&m_pParentDevice->GetContext(), dsvDesc, *m_pResource);
        }
    }

    void Resource::InitializeSRVDescFormatAndShaderComponentMapping(_In_ D3DFORMAT d3dFormat, _Out_ D3D12_SHADER_RESOURCE_VIEW_DESC &desc, bool DisableAlphaChannel)
    {
        desc.Format = ConvertToDXGIFORMAT(d3dFormat);
        // If they intend to bind the Depth Stencil as a SRV, we need to change the format
        if (IsDepthStencilFormat(desc.Format))
        {
            desc.Format = ConvertDepthFormatToCompanionSRVFormat(desc.Format);
        }

        desc.Shader4ComponentMapping = ConvertFormatToShaderResourceViewComponentMapping(d3dFormat, DisableAlphaChannel);
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC Resource::GetShaderResourceViewDescForArraySlice(UINT8 arraySlice, UINT8 arraySize)
    {
        Check9on12(GetArraySize() >= arraySlice + arraySize);
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

        InitializeSRVDescFormatAndShaderComponentMapping(m_dx9Format, srvDesc, IsAlphaChannelDisabled());

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = 0;
        srvDesc.Texture2DArray.MipLevels = m_logicalDesc.MipLevels;
        srvDesc.Texture2DArray.FirstArraySlice = arraySlice;
        srvDesc.Texture2DArray.ArraySize = arraySize;
        srvDesc.Texture2DArray.PlaneSlice = 0;
        srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;

        return srvDesc;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC Resource::GetShaderResourceViewDesc()
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        InitializeSRVDescFormatAndShaderComponentMapping(m_dx9Format, srvDesc, IsAlphaChannelDisabled());

        switch (m_logicalDesc.Dimension)
        {
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        {
            if (m_numArrayLevels == 6)
            {
                assert(m_logicalDesc.SampleDesc.Count <= 1); // D3D12 doesn't support MSAA TextureCubes
                                                             // We need to explicitly define a descriptor to say that this a texture cube 
                                                             // and not a texture array
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                srvDesc.TextureCube.MostDetailedMip = 0;
                srvDesc.TextureCube.MipLevels = m_logicalDesc.MipLevels;
                srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                if (m_logicalDesc.SampleDesc.Count > 1)
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MipLevels = m_logicalDesc.MipLevels;
                    srvDesc.Texture2D.MostDetailedMip = 0;
                    srvDesc.Texture2D.PlaneSlice = 0;
                    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                }

            }
            break;
        }
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            srvDesc.Texture3D.MipLevels = m_logicalDesc.MipLevels;
            srvDesc.Texture3D.MostDetailedMip = 0;
            srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        case D3D12_RESOURCE_DIMENSION_BUFFER:
        case D3D12_RESOURCE_DIMENSION_UNKNOWN:
        default:
        {
            // DX9 does not support creating SRVs for these resource types
            Check9on12(FALSE);
            break;
        }
        }

        return srvDesc;
    }

    D3D12TranslationLayer::SRV * Resource::CreateShaderResourceView(ShaderResourceViewFlags flag)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = GetShaderResourceViewDesc();
        if ((flag & ShaderResourceViewFlags::SRGBEnabled) && IsSRGBCompatible(srvDesc.Format))
        {
            srvDesc.Format = ConvertToSRGB(srvDesc.Format);
        }

        if ((flag & ShaderResourceViewFlags::DisableMips))
        {
            switch (srvDesc.ViewDimension)
            {
            case D3D12_SRV_DIMENSION_BUFFER:
                // Do nothing
                break;
            case D3D12_SRV_DIMENSION_TEXTURE2D:
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.MipLevels = 1;
                break;
            case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
                srvDesc.Texture2DArray.MostDetailedMip = 0;
                srvDesc.Texture2DArray.MipLevels = 1;
                break;
            case D3D12_SRV_DIMENSION_TEXTURE3D:
                srvDesc.Texture3D.MostDetailedMip = 0;
                srvDesc.Texture3D.MipLevels = 1;
                break;
            case D3D12_SRV_DIMENSION_TEXTURECUBE:
                srvDesc.TextureCube.MostDetailedMip = 0;
                srvDesc.TextureCube.MipLevels = 1;
                break;
            default:
                Check9on12(E_FAIL); // Unexpected dimension
            }
        }

        return new D3D12TranslationLayer::SRV(&m_pParentDevice->GetContext(), srvDesc, *m_pResource);
    }

    UINT8 Resource::GetArraySize()
    {
        if (m_logicalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            return 1;
        }
        else
        {
            return m_numArrayLevels;
        }
    }

    HANDLE Resource::GetSharedGDIHandle()
    {
        HANDLE sharedHandle = 0;
        // Try not to throw exceptions in a common case.
        if ((GetUnderlyingResource()->Parent()->IsGDIStyleHandleShared()))
        {
            m_pParentDevice->GetContext().GetSharedGDIHandle(GetUnderlyingResource(), &sharedHandle);
        }

        return sharedHandle;
    }

    HANDLE Resource::CreateSharedNTHandle(SECURITY_DESCRIPTOR *pSD)
    {
        HANDLE sharedHandle = 0;
        D3D12TranslationLayer::Resource *pUnderlyingResource = GetUnderlyingResource();
        SECURITY_ATTRIBUTES securityAttributes = ConvertSecurityDescriptorToSecurityAttributes(pSD);
        m_pParentDevice->GetContext().CreateSharedNTHandle(pUnderlyingResource, &sharedHandle, &securityAttributes);
        return sharedHandle;
    }


    UINT Resource::GetMipLevelFromSubresourceIndex(UINT subresourceIndex)
    {
        UINT mipSlice, arraySlice, planeSlice;
        D3D12DecomposeSubresource(subresourceIndex, m_logicalDesc.MipLevels, GetArraySize(), mipSlice, arraySlice, planeSlice);

        Check9on12(mipSlice < MAX_MIPS);
        return mipSlice;
    }

    UINT8 Resource::GetArraySliceFromSubresourceIndex(UINT subresourceIndex)
    {
        UINT8 arraySlice;
        UINT mipSlice, planeSlice;
        D3D12DecomposeSubresource(subresourceIndex, m_logicalDesc.MipLevels, GetArraySize(), mipSlice, arraySlice, planeSlice);
        return arraySlice;
    }

    void Resource::ResourceCompatibilityOptions::Init(D3DFORMAT format)
    {
        switch (format)
        {
        case D3DFMT_D16:
        case D3DFMT_D24X8:
        case D3DFMT_D24S8:
            m_isEligibleForHardwareShadowMapping = true;
            break;
        default:
            break;
        }
    }

    HRESULT Resource::Rename(Device& device, CONST D3DDDIARG_RENAME& arg)
    {
        HRESULT hr = S_OK;
        Check9on12(arg.SubResourceIndex == 0);

        device.GetContext().Rename(m_pResource.get(), reinterpret_cast<D3D12TranslationLayer::Resource*>(arg.hCookie));
        device.GetContext().DeleteRenameCookie(reinterpret_cast<D3D12TranslationLayer::Resource*>(arg.hCookie));

        // Discard resources should never be RTVs or DSVs
        Check9on12(GetRTVBindingTracker().GetBindingIndices().size() == 0 && GetDSVBindingTracker().GetBindingIndices().size() == 0);

        // The rotation mechanism used by rename ensures SRVs are updated before they're used, and future draws will reference the new resource.

        NotifyResourceChanged();

        return hr;
    }

    _Check_return_ HRESULT APIENTRY QueryResourceResidency(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_QUERYRESOURCERESIDENCY *pQueryResidencyArgs)
    {
        D3D9on12_DDI_ENTRYPOINT_START(FALSE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = S_OK;
        for (UINT i = 0; i < pQueryResidencyArgs->NumResources; i++)
        {
            Resource *pResource = Resource::GetResourceFromHandle(pQueryResidencyArgs->pHandleList[i]);

            // Runtime validates that system memory resources never get queried by the driver
            Check9on12(!pResource->IsSystemMemory());

            if (!pResource->IsResident())
            {
                // Optimistically assuming it's still in main memory (i.e. not disk, though this is a possiblility)
                // This is okay since this DDI is a guess at best
                hr = S_RESIDENT_IN_SHARED_MEMORY;
                break;
            }
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    void CleanupResourceBindingsAndRelease::operator()(D3D12TranslationLayer::Resource* pResource)
    {
        CleanupTranslationLayerResourceBindings(*pResource->m_pParent, *pResource);
        pResource->Release();
    }

    void APIENTRY TransitionResource(HANDLE hDD, HANDLE hResource, UINT State)
    {
        Device* pDevice = Device::GetDeviceFromHandle(hDD);
        if (pDevice == nullptr || hResource == nullptr)
        {
            return;
        }

        Resource* pResource = Resource::GetResourceFromHandle(hResource);
        pDevice->GetContext().GetResourceStateManager().TransitionResource(
            pResource->GetUnderlyingResource(), (D3D12_RESOURCE_STATES)State,
            D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS,
            D3D12TranslationLayer::SubresourceTransitionFlags::NoBindingTransitions |
                D3D12TranslationLayer::SubresourceTransitionFlags::StateMatchExact |
                D3D12TranslationLayer::SubresourceTransitionFlags::NotUsedInCommandListIfNoStateChange);
        pDevice->GetContext().GetResourceStateManager().ApplyAllResourceTransitions();
    }

    void APIENTRY SetCurrentResourceState(HANDLE hDD, HANDLE hResource, UINT State)
    {
        Device* pDevice = Device::GetDeviceFromHandle(hDD);
        if (pDevice == nullptr || hResource == nullptr)
        {
            return;
        }

        Resource* pResource = Resource::GetResourceFromHandle(hResource);

        D3D12TranslationLayer::CCurrentResourceState::ExclusiveState ExclusiveState = {};
        ExclusiveState.FenceValue = pDevice->GetContext().GetCommandListID(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS);
        ExclusiveState.CommandListType = D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS;
        ExclusiveState.State = (D3D12_RESOURCE_STATES)State;
        pResource->GetUnderlyingResource()->GetIdentity()->m_currentState.SetExclusiveResourceState(ExclusiveState);
    }
};
