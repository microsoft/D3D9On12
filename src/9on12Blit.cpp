// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{    

    ResourceCopyArgs::ResourceCopyArgs(Resource& destination, Resource& source) :
        m_source(source),
        m_sourceSubresourceIndex(0),
        m_destinationSubresourceIndex(0),
        m_cubeFace(0),
        m_numSubresources(0),
        m_allSubresources(false),
        m_mipsAlreadyExpanded(false),
        m_dimensionsCoverEntireResource(false),
        m_isStretchCopy(false),
        m_destination(destination),
        m_stretchSource{},
        m_stretchDestination{},
        m_destinationX(0), m_destinationY(0), m_destinationZ(0),
        m_sourceBox{},
        m_EnableAlpha(FALSE){}

    void ResourceCopyArgs::AlignCopyDimensions()
    {
        //Adjust the rectScaled coordinates for DXTn (4x4) alignment.
        if (IsBlockCompressedFormat(m_source.GetLogicalDesc().Format))
        {
            m_destinationX &= ~3;
            m_destinationY &= ~3;

            m_sourceBox.left &= ~3;
            m_sourceBox.top &= ~3;
            m_sourceBox.right = (m_sourceBox.right + 3) & (~3);
            m_sourceBox.bottom = (m_sourceBox.bottom + 3) & (~3);
        }
        else if (IsYUVFormat(m_source.GetLogicalDesc().Format))  // Adjust the rectScaled coordinates for YUV (2x1) alignment.
        {
            m_destinationX &= ~1;

            m_sourceBox.left &= ~1;
            m_sourceBox.right = (m_sourceBox.right + 1) & ~1;
        }
    }

    void ResourceCopyArgs::ClipSourceBox(UINT sourceMipLevel, UINT destinationMipLevel)
    {
        ClipBox(m_sourceBox, m_source.GetSubresourceFootprint(sourceMipLevel));
        ClipBox(m_sourceBox, m_destination.GetSubresourceFootprint(destinationMipLevel));
        
        const D3D12_SUBRESOURCE_FOOTPRINT &srcFootprint = m_source.GetSubresourceFootprint(sourceMipLevel).Footprint;
        m_sourceBox.back = min(m_sourceBox.back, srcFootprint.Depth);
        m_sourceBox.right = min(m_sourceBox.right, srcFootprint.Width);
        m_sourceBox.bottom = min(m_sourceBox.bottom, srcFootprint.Height);

        if (m_sourceBox.left >= m_sourceBox.right)
        {
            m_sourceBox.left = m_sourceBox.right - 1;
            Check9on12(m_sourceBox.left <= m_sourceBox.right); // Check for overflow
        }

        if (m_sourceBox.top >= m_sourceBox.bottom)
        {
            m_sourceBox.top = m_sourceBox.bottom - 1;
            Check9on12(m_sourceBox.top <= m_sourceBox.bottom); // Check for overflow
        }

        if (m_sourceBox.front >= m_sourceBox.back)
        {
            m_sourceBox.front = m_sourceBox.back - 1;
            Check9on12(m_sourceBox.front <= m_sourceBox.back); // Check for overflow
        }

        // Corner case that gets hit when the depth slice has been cut down
        // to the last depth slice, need to make sure the destinationZ is clamped
        // to boundaries also
        const UINT dstDepth = m_destination.GetSubresourceFootprint(destinationMipLevel).Footprint.Depth;
        Check9on12(m_destinationZ <= dstDepth);
        if (m_destinationZ == dstDepth)
        {
            m_destinationZ = dstDepth - 1;
        }

        Check9on12(BoxDepth(m_sourceBox) <= m_source.GetSubresourceFootprint(sourceMipLevel).Footprint.Depth);
        Check9on12(BoxHeight(m_sourceBox) <= m_source.GetSubresourceFootprint(sourceMipLevel).Footprint.Height);
        Check9on12(BoxWidth(m_sourceBox) <= m_source.GetSubresourceFootprint(sourceMipLevel).Footprint.Width);

        Check9on12(m_destinationZ + BoxDepth(m_sourceBox) <= m_destination.GetSubresourceFootprint(destinationMipLevel).Footprint.Depth);
        Check9on12(m_destinationY + BoxHeight(m_sourceBox) <= m_destination.GetSubresourceFootprint(destinationMipLevel).Footprint.Height);
        Check9on12(m_destinationX + BoxWidth(m_sourceBox) <= m_destination.GetSubresourceFootprint(destinationMipLevel).Footprint.Width);
    }

    bool ResourceCopyArgs::IsBufferBlit() const
    { 
        return m_destination.GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && 
            m_source.GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
    }

    void ResourceCopyArgs::AsBufferBlit(UINT dstOffset, D3DDDIRANGE srcRange)
    {
        Check9on12(srcRange.Size > 0);
        m_allSubresources = true;
        m_numSubresources = 1;
        m_destinationX = dstOffset;
        m_sourceBox.left = srcRange.Offset;
        m_sourceBox.right = srcRange.Offset + srcRange.Size;
        m_sourceBox.top = 0;
        m_sourceBox.bottom = 1;
        m_sourceBox.front = 0;
        m_sourceBox.back = 1;

        if (m_destination.GetDesc().Width == m_source.GetDesc().Width &&
            (m_sourceBox.right - m_sourceBox.left) == m_destination.GetDesc().Width)
        {
            m_dimensionsCoverEntireResource = true;
        }
    }

    void ResourceCopyArgs::AsTextureBlit(const POINT &dstPoint, const RECT &srcRect, UINT cubeFace)
    {
        m_allSubresources = true;
        m_destinationSubresourceIndex = 0;
        m_sourceSubresourceIndex = 0;
        m_cubeFace = cubeFace;
        m_numSubresources = max(m_destination.GetNumSubresources(), m_source.GetNumSubresources());

        m_destinationX = dstPoint.x;
        m_destinationY = dstPoint.y;
        m_sourceBox.left = srcRect.left;
        m_sourceBox.right = srcRect.right;
        m_sourceBox.top = srcRect.top;
        m_sourceBox.bottom = srcRect.bottom;
        m_sourceBox.front = 0;
        m_sourceBox.back = 1;

        if ((m_sourceBox.right - m_sourceBox.left) == m_destination.GetLogicalDesc().Width &&
            (m_sourceBox.bottom - m_sourceBox.top) == m_destination.GetLogicalDesc().Height)
        {
            m_dimensionsCoverEntireResource = true;
        }
        else
        {
            AlignCopyDimensions();
        }
    }

    void ResourceCopyArgs::AsVolumeBlit(UINT dstX, UINT dstY, UINT dstZ, const D3D12_BOX &srcBox)
    {
        m_allSubresources = true;
        m_destinationSubresourceIndex = 0;
        m_sourceSubresourceIndex = 0;
        m_numSubresources = max(m_destination.GetNumSubresources(), m_source.GetNumSubresources());

        m_destinationX = dstX;
        m_destinationY = dstY;
        m_destinationZ = dstZ;
        m_sourceBox = srcBox;

        if ((m_sourceBox.right - m_sourceBox.left) == m_destination.GetLogicalDesc().Width &&
            (m_sourceBox.bottom - m_sourceBox.top) == m_destination.GetLogicalDesc().Height &&
            (m_sourceBox.back - m_sourceBox.front) == m_destination.GetLogicalDesc().DepthOrArraySize)
        {
            m_dimensionsCoverEntireResource = true;
        }
        else
        {
            AlignCopyDimensions();
        }
    }

    void ResourceCopyArgs::AsSubresourceBlit(UINT dstSubresourceIndex, UINT srcSubresourceIndex, CONST RECT* pDstRect, CONST RECT* pSrcRect)
    {
        m_allSubresources = false;

        m_destinationSubresourceIndex = dstSubresourceIndex;
        m_sourceSubresourceIndex = srcSubresourceIndex;
        m_numSubresources = 1;

        if (pDstRect)
        {
            Check9on12(pSrcRect);
        }
        if (pSrcRect)
        {
            Check9on12(pDstRect);
        }

        if (pDstRect == nullptr || pSrcRect == nullptr)
        {
            m_dimensionsCoverEntireResource = true;
        }
        else
        {
            const D3D12_RESOURCE_DESC& srcDesc = m_source.GetLogicalDesc();
            const D3D12_RESOURCE_DESC& dstDesc = m_destination.GetLogicalDesc();

            POINT dstRectSize = { (pDstRect->right - pDstRect->left), (pDstRect->bottom - pDstRect->top) };
            POINT srcRectSize = { (pSrcRect->right - pSrcRect->left), (pSrcRect->bottom - pSrcRect->top) };

            if (dstRectSize.x != srcRectSize.x || dstRectSize.y != srcRectSize.y)
            {
                // Stretch blit
                AsStretchBlit(dstSubresourceIndex, srcSubresourceIndex, pDstRect, pSrcRect);
            }
            else
            {
                if (srcDesc.Width == RectWidth(*pSrcRect) && srcDesc.Height == RectHeight(*pSrcRect) &&
                    dstDesc.Width == RectWidth(*pDstRect) && dstDesc.Height == RectHeight(*pDstRect))
                {
                    m_dimensionsCoverEntireResource = true;
                }
                else
                {
                    m_destinationX = pDstRect->left;
                    m_destinationY = pDstRect->top;

                    m_sourceBox.left = pSrcRect->left;
                    m_sourceBox.right = pSrcRect->right;
                    m_sourceBox.top = pSrcRect->top;
                    m_sourceBox.bottom = pSrcRect->bottom;
                    m_sourceBox.back = 1;
                }

                AlignCopyDimensions();
            }
        }
    }

    void ResourceCopyArgs::AsStretchBlit(UINT dstSubresourceIndex, UINT srcSubresourceIndex, CONST RECT* pDstRect, CONST RECT* pSrcRect)
    {
        m_allSubresources = false;

        m_destinationSubresourceIndex = dstSubresourceIndex;
        m_sourceSubresourceIndex = srcSubresourceIndex;
        m_numSubresources = 1;

        m_isStretchCopy = true;

        if (pDstRect)
        {
            m_stretchDestination = *pDstRect;
        }
        else
        {
            m_stretchDestination = RectThatCoversEntireResource(m_destination.GetLogicalDesc());
        }

        if (pSrcRect)
        {
            m_stretchSource = *pSrcRect;
        }
        else
        {
            m_stretchSource = RectThatCoversEntireResource(m_source.GetLogicalDesc());
        }
    }

    ResourceCopyArgs ResourceCopyArgs::GetSubArgs(UINT arrayLevel, UINT sourceMipLevel, UINT destinationMipLevel) const
    {
        ResourceCopyArgs subArgs = ResourceCopyArgs(m_destination, m_source);
        subArgs.m_sourceBox = m_sourceBox;
        subArgs.m_dimensionsCoverEntireResource = m_dimensionsCoverEntireResource;

        if (m_dimensionsCoverEntireResource == false)
        {
            subArgs.m_destinationX = m_destinationX >> destinationMipLevel;
            subArgs.m_destinationY = m_destinationY >> destinationMipLevel;
            subArgs.m_destinationZ = m_destinationZ >> destinationMipLevel;

            if (sourceMipLevel > 0)
            {
                ScaleBoxDown(subArgs.m_sourceBox, sourceMipLevel);
                subArgs.ClipSourceBox(sourceMipLevel, destinationMipLevel);
                // Mip levels dimensions are rounded down for each mip level division, but the copy rects right/bottom points 
                // are rounded up each time they are divided. We need to clamp to handle when the copy rects
                // go out of bounds.
            }

            subArgs.AlignCopyDimensions();
        }

        //TODO: depth planes?
        subArgs.m_numSubresources = 1;
        subArgs.m_destinationSubresourceIndex = D3D12CalcSubresource(destinationMipLevel, arrayLevel, 0, m_destination.GetLogicalDesc().MipLevels, m_destination.GetArraySize());
        subArgs.m_sourceSubresourceIndex = D3D12CalcSubresource(sourceMipLevel, arrayLevel, 0, m_source.GetLogicalDesc().MipLevels, m_source.GetArraySize());
        subArgs.m_mipsAlreadyExpanded = true;

        return subArgs;
    }

    void Resource::CopyPrologue(Device& /*device*/, ResourceCopyArgs& /*args*/)
    {
    }

    void Resource::CopyToSystemMemoryResource(Device& device, ResourceCopyArgs& args)
    {
        D3D12TranslationLayer::ImmediateContext& context = device.GetContext();
        Resource& source = args.m_source;
        Resource& destination = args.m_destination;

        COPY_TO_SYSTEM_MEMORY_WARNING();

        Check9on12(destination.IsSystemMemory());

        UINT sourceSubresourceIndex = args.m_sourceSubresourceIndex;
        UINT destinationSubresourceIndex = args.m_destinationSubresourceIndex;

        unique_comptr<D3D12TranslationLayer::Resource> scopedMappableResource;
        D3D12TranslationLayer::Resource *pMappableResource = source.GetUnderlyingResource();
        if ((pMappableResource->AppDesc()->CPUAccessFlags() & D3D12TranslationLayer::RESOURCE_CPU_ACCESS_READ) == 0)
        {
            scopedMappableResource = std::move(source.GetStagingCopy());
            pMappableResource = scopedMappableResource.get();
        }

        // Can't do a GPU copy to an upload heap so we have to memcpy it all over
        for (UINT i = 0; i < args.m_numSubresources; i++)
        {
            D3D12_MEMCPY_DEST appDstData = destination.m_appLinearRepresentation.m_systemData[destinationSubresourceIndex];

            D3D12TranslationLayer::MappedSubresource srcData = {};
            context.Map(pMappableResource, sourceSubresourceIndex, D3D12TranslationLayer::MAP_TYPE_READ, false , nullptr, &srcData);

            D3D12_SUBRESOURCE_DATA srcSubresourceData = {};
            srcSubresourceData.pData = srcData.pData;
            srcSubresourceData.RowPitch = srcData.RowPitch;
            srcSubresourceData.SlicePitch = srcData.DepthPitch;

            SIZE_T BytesPerRow = appDstData.RowPitch;
            UINT NumRows = source.m_physicalLinearRepresentation.m_numRows[sourceSubresourceIndex];
            if (!args.m_dimensionsCoverEntireResource)
            {
                const DXGI_FORMAT SrcFormat = args.m_source.GetLogicalDesc().Format;
                UINT8 BytesPerPixel = (SrcFormat == DXGI_FORMAT_UNKNOWN) ? 1 : GetBytesPerUnit(SrcFormat);

                // Offset destination
                reinterpret_cast<BYTE*&>(appDstData.pData) +=
                    (args.m_destinationZ * appDstData.SlicePitch) +
                    (args.m_destinationY * appDstData.RowPitch) +
                    (args.m_destinationX * BytesPerPixel);

                // Offset source
                reinterpret_cast<BYTE*&>(srcData.pData) +=
                    (args.m_sourceBox.front * srcData.DepthPitch) +
                    (args.m_sourceBox.top * srcData.RowPitch) +
                    (args.m_sourceBox.left * BytesPerPixel);

                // Adjust how much data is copied
                BytesPerRow = BytesPerPixel * (args.m_sourceBox.right - args.m_sourceBox.left);
                NumRows = args.m_sourceBox.bottom - args.m_sourceBox.top;
            }

            MemcpySubresource(&appDstData, &srcSubresourceData, BytesPerRow, NumRows, 1);

            context.Unmap(pMappableResource, sourceSubresourceIndex, D3D12TranslationLayer::MAP_TYPE_READ, nullptr);

            sourceSubresourceIndex++;
            destinationSubresourceIndex++;
        }
    }  

    void Resource::CopyResource(Device& device, ResourceCopyArgs& args)
    {
        // DX9 allows for uploading from a src resource with different mip levels than the dst resource.
        // In this case, only the matching mip levels from the src are copied over
        if (args.m_allSubresources && args.m_source.m_numSubresources != args.m_destination.m_numSubresources)
        {
            Resource::CopyMatchingMipLevels(device, args);
        }
        else
        {
            D3D12TranslationLayer::ImmediateContext& context = device.GetContext();
            Resource& source = args.m_source;
            Resource& destination = args.m_destination;

            // We generally shouldn't expect to be copying to a system memory resource but this gets hit in the UpdateSurface HLK test
            if (args.m_destination.m_isD3D9SystemMemoryPool)
            {
                Resource::CopyPrologue(device, args);
                Check9on12(args.m_source.m_isD3D9SystemMemoryPool == false);
                CopyToSystemMemoryResource(device, args);
            }
            else
            {
                const bool mismatchedFormats = source.GetD3DFormat() != destination.GetD3DFormat();
                if (args.m_dimensionsCoverEntireResource && args.m_source.IsSystemMemory() == false && !mismatchedFormats)
                {
                    Resource::CopyPrologue(device, args);

                    const bool needsMSAAResolve = args.m_source.GetDesc().SampleDesc.Count > 1 && args.m_destination.GetDesc().SampleDesc.Count == 1;
                    if (needsMSAAResolve)
                    {
                        Check9on12(args.m_numSubresources == 1);
                        context.ResourceResolveSubresource(args.m_destination.GetUnderlyingResource(),
                            args.m_destinationSubresourceIndex,
                            args.m_source.GetUnderlyingResource(),
                            args.m_sourceSubresourceIndex,
                            args.m_destination.GetViewFormat(false));
                    }
                    else
                    {
                        context.ResourceCopy(destination.GetUnderlyingResource(), source.GetUnderlyingResource());
                    }
                }
                else
                {
                    if (args.IsBufferBlit() && args.m_source.IsSystemMemory() == false)
                    {
                        Resource::CopyPrologue(device, args);
                        context.ResourceCopyRegion(destination.GetUnderlyingResource(), //Dest
                            0, // Dest Subresource
                            args.m_destinationX, // Dest X
                            0, // Dest Y
                            0, // Dest Z
                            source.GetUnderlyingResource(), // Source
                            0, // Source Subresource
                            &args.m_sourceBox); //SourceBox

                    }
                    else
                    {
                        UINT arrayStart = args.m_cubeFace == UINT_MAX ? 0 : args.m_cubeFace;
                        UINT arrayEnd = args.m_cubeFace == UINT_MAX ? destination.GetArraySize() : args.m_cubeFace + 1;
                        for (UINT arraySlice = arrayStart; arraySlice < arrayEnd; arraySlice++)
                        {
                            for (UINT mipSlice = 0; mipSlice < args.m_destination.GetLogicalDesc().MipLevels; mipSlice++)
                            {
                                //If they give us a copy rect/box we need to calculate
                                //that box for every mip sublevel.
                                ResourceCopyArgs arg = args.GetSubArgs(arraySlice, mipSlice, mipSlice);
                                CopySubResource(device, arg);
                            }
                        }
                    }
                }
            }
        }
        args.m_destination.NotifyResourceChanged();
    }

    static inline D3D12_RECT ConvertBoxToRect(const D3D12_BOX& box)
    {
        Check9on12(box.front == 0 && box.back == 1);

        D3D12_RECT rect =
        {
            static_cast<LONG>(box.left), 
            static_cast<LONG>(box.top), 
            static_cast<LONG>(box.right), 
            static_cast<LONG>(box.bottom)
        };

        return rect;
    }

    void Resource::CopySubResource(Device& device, ResourceCopyArgs& args)
    {
        D3D12TranslationLayer::ImmediateContext& context = device.GetContext();
        Resource& source = args.m_source;
        Resource& destination = args.m_destination;


        // This method expects D3D9's understanding of subresource indices in its input arguments, where all the 
        // planes of a of a texture are a single subresource.
        // It handles mapping that understanding to D3D12's where each plane is it's own subresource.
        // This method only handles cases where we are converting from a multi-plane source resource to a single 
        // plane destination resource (e.g. NV12->BGRA) or a source and destination with the same number of planes 
        // (same format, e.g. NV12->NV12, BGRA->BGRA etc.)

        UINT srcSubresources[MAX_PLANES];
        UINT dstSubresources[MAX_PLANES];
        UINT numDstPlanes, numSrcPlanes;

        source.ConvertAppSubresourceToDX12SubresourceIndices(args.m_sourceSubresourceIndex, srcSubresources, numSrcPlanes, args.m_mipsAlreadyExpanded);
        destination.ConvertAppSubresourceToDX12SubresourceIndices(args.m_destinationSubresourceIndex, dstSubresources, numDstPlanes, args.m_mipsAlreadyExpanded);

        for (UINT planeIndex = 0; planeIndex < numSrcPlanes; planeIndex++)
        {
            UINT sourceSubresourceIndex = srcSubresources[planeIndex];
            UINT destinationSubresourceIndex = dstSubresources[0];

            UINT destinationX = args.m_destinationX;
            UINT destinationY = args.m_destinationY;
            UINT destinationZ = args.m_destinationZ;

            D3D12_BOX sourceBox = args.m_sourceBox;

            if (IsChromaPlane(planeIndex))
            {
                UINT subsampleX, subsampleY;
                CD3D11FormatHelper::GetYCbCrChromaSubsampling(source.GetViewFormat(false), subsampleX, subsampleY);

                // YUV is 2D.
                D3D12_RECT sourceRect = ConvertBoxToRect(args.m_sourceBox);

                sourceRect = ConvertLumaRectToChromaRect(sourceRect, subsampleX, subsampleY);

                sourceBox.left = sourceRect.left;
                sourceBox.top = sourceRect.top;
                sourceBox.right = sourceRect.right;
                sourceBox.bottom = sourceRect.bottom;

                if (numDstPlanes == numSrcPlanes)
                {
                    destinationSubresourceIndex = dstSubresources[planeIndex];

                    RECT destinationRect;
                    destinationRect.left = destinationX;
                    destinationRect.top = destinationY;
                    destinationRect.right = destinationX + (args.m_sourceBox.right - args.m_sourceBox.left);
                    destinationRect.bottom = destinationY + (args.m_sourceBox.bottom - args.m_sourceBox.top);

                    destinationRect = ConvertLumaRectToChromaRect(destinationRect, subsampleX, subsampleY);

                    destinationX = destinationRect.left;
                    destinationY = destinationRect.top;
                }
            }

            Resource::CopyPrologue(device, args);

            // We generally shouldn't expect to be copying to a system memory resource but this gets hit in the UpdateSurface HLK test
            if (destination.m_isD3D9SystemMemoryPool)
            {
                Check9on12(args.m_isStretchCopy == false);
                Check9on12(source.m_isD3D9SystemMemoryPool == false);
                auto SysmemCopyArgs = args;
                SysmemCopyArgs.m_sourceSubresourceIndex = sourceSubresourceIndex;
                SysmemCopyArgs.m_destinationSubresourceIndex = destinationSubresourceIndex;
                CopyToSystemMemoryResource(device, SysmemCopyArgs);
            }
            else if (source.IsSystemMemory())
            {
                Check9on12(args.m_isStretchCopy == false);
                D3D12_BOX destBox = CD3DX12_BOX(destinationX, destinationY, destinationZ,
                    destinationX + BoxWidth(sourceBox),
                    destinationY + BoxHeight(sourceBox),
                    destinationZ + BoxDepth(sourceBox));

                const DXGI_FORMAT SrcFormat = source.GetLogicalDesc().Format;
                UINT8 BytesPerPixel = (SrcFormat == DXGI_FORMAT_UNKNOWN) ? 1 : GetBytesPerUnit(SrcFormat);
                UINT SrcX = sourceBox.left;
                UINT SrcY = sourceBox.top;
                UINT SrcZ = sourceBox.front;
                if (IsBlockCompressedFormat(SrcFormat))
                {
                    const UINT COMPRESSED_BLOCK_SIZE = 4;
                    Check9on12(IsAligned(SrcX, COMPRESSED_BLOCK_SIZE) && IsAligned(SrcY, COMPRESSED_BLOCK_SIZE));
                        SrcX /= COMPRESSED_BLOCK_SIZE;
                    SrcY /= COMPRESSED_BLOCK_SIZE;
                }

                const D3D12_MEMCPY_DEST &SrcDataInfo = source.m_appLinearRepresentation.m_systemData[sourceSubresourceIndex];

                // ResourceUpdateSubresourceUP doesn't take in a src box so we need to do the pointer offset math here
                void *pSrcData = (BYTE *)SrcDataInfo.pData
                    + BytesPerPixel * SrcX
                    + SrcDataInfo.RowPitch * SrcY
                    + SrcDataInfo.SlicePitch * SrcZ;

                D3D11_SUBRESOURCE_DATA SubresourceDesc =
                {
                    pSrcData,
                    static_cast<UINT>(source.m_appLinearRepresentation.m_systemData[sourceSubresourceIndex].RowPitch),
                    static_cast<UINT>(source.m_appLinearRepresentation.m_systemData[sourceSubresourceIndex].SlicePitch)
                };
                UINT8 MipLevel, PlaneSlice;
                UINT16 ArraySlice;
                D3D12TranslationLayer::DecomposeSubresourceIdxExtended(destinationSubresourceIndex,
                    destination.GetUnderlyingResource()->AppDesc()->MipLevels(),
                    destination.GetUnderlyingResource()->AppDesc()->ArraySize(), MipLevel, ArraySlice, PlaneSlice);

                D3D12TranslationLayer::ImmediateContext::UpdateSubresourcesFlags Flags = D3D12TranslationLayer::ImmediateContext::UpdateSubresourcesFlags::ScenarioImmediateContext;
                if (source.NeedsSwapRBOutputChannels() != destination.NeedsSwapRBOutputChannels())
                {
                    assert(ConvertToTypeless(source.GetViewFormat(false)) == DXGI_FORMAT_R10G10B10A2_TYPELESS);
                    Flags |= D3D12TranslationLayer::ImmediateContext::UpdateSubresourcesFlags::ChannelSwapR10G10B10A2;
                }

                context.UpdateSubresources(destination.GetUnderlyingResource(),
                    D3D12TranslationLayer::CSubresourceSubset(1, 1, destination.GetUnderlyingResource()->SubresourceMultiplier(), MipLevel, ArraySlice, PlaneSlice),
                    &SubresourceDesc,
                    (args.m_dimensionsCoverEntireResource) ? nullptr : &destBox,
                    Flags);
            }
            else
            {
                const DXGI_FORMAT srcFormat = source.GetViewFormat(false);
                const DXGI_FORMAT dstFormat = destination.GetViewFormat(false);
                const bool mismatchingSampleCount = source.GetDesc().SampleDesc.Count != destination.GetDesc().SampleDesc.Count;
                const bool needsMSAAResolve = source.GetDesc().SampleDesc.Count > 1 && destination.GetDesc().SampleDesc.Count == 1;
                // ResolveSubresource can't handle stretching or sub rects, use a draw to copy instead in these cases
                const bool mismatchedFormats = srcFormat != DXGI_FORMAT_UNKNOWN && dstFormat != DXGI_FORMAT_UNKNOWN && (ConvertToTypeless(srcFormat) != ConvertToTypeless(dstFormat));
                const bool channelSwap = source.NeedsSwapRBOutputChannels() != destination.NeedsSwapRBOutputChannels();
                const bool canUseResolveSubresource = needsMSAAResolve && !mismatchedFormats && !args.m_isStretchCopy && args.m_dimensionsCoverEntireResource;
                const bool needsStretchCopy = mismatchedFormats || args.m_isStretchCopy || (mismatchingSampleCount && !canUseResolveSubresource) || channelSwap;

                if (canUseResolveSubresource)
                {
                    context.ResourceResolveSubresource(destination.GetUnderlyingResource(),
                        destinationSubresourceIndex,
                        source.GetUnderlyingResource(),
                        sourceSubresourceIndex,
                        destination.GetViewFormat(false));
                }
                else if (needsStretchCopy)
                {
                    auto SetupRects = [&source, &destination, sourceSubresourceIndex, destinationSubresourceIndex](RECT& stretchSource, RECT& stretchDestination)
                    {
                        if (IsRectEmpty(stretchSource))
                        {
                            stretchSource = RectThatCoversEntireResource(source.GetSubresourceFootprint(sourceSubresourceIndex));
                        }

                        if (IsRectEmpty(stretchDestination))
                        {
                            stretchDestination = RectThatCoversEntireResource(destination.GetSubresourceFootprint(destinationSubresourceIndex));
                        }

                        ClipCopyRect(stretchDestination, stretchSource, static_cast<INT>(destination.GetDesc().Width), static_cast<INT>(destination.GetDesc().Height));
                        ClipCopyRect(stretchSource, stretchDestination, static_cast<INT>(source.GetDesc().Width), static_cast<INT>(source.GetDesc().Height));
                    };

                    if (!args.m_isStretchCopy)
                    {
                        RECT dstRect = RectThatCoversEntireResource(destination.GetSubresourceFootprint(destinationSubresourceIndex));
                        dstRect.left = destinationX;
                        dstRect.top = destinationY;
                        dstRect.right = dstRect.left + BoxWidth(sourceBox);
                        dstRect.bottom = dstRect.top + BoxHeight(sourceBox);

                        RECT srcRect = {};
                        srcRect.left = sourceBox.left;
                        srcRect.right = sourceBox.right;
                        srcRect.top = sourceBox.top;
                        srcRect.bottom = sourceBox.bottom;

                        SetupRects(srcRect, dstRect);
                        device.GetBlitHelper().Blit(
                            source.GetUnderlyingResource(),
                            srcSubresources, numSrcPlanes,
                            srcRect,
                            destination.GetUnderlyingResource(),
                            dstSubresources, numDstPlanes,
                            dstRect,
                            false,
                            channelSwap);
                    }
                    else
                    {
                        SetupRects(args.m_stretchSource, args.m_stretchDestination);
                        device.GetBlitHelper().Blit(
                            source.GetUnderlyingResource(),
                            srcSubresources, numSrcPlanes,
                            args.m_stretchSource,
                            destination.GetUnderlyingResource(),
                            dstSubresources, numDstPlanes,
                            args.m_stretchDestination,
                            args.m_EnableAlpha,
                            channelSwap);
                    }

                    break; // BlitHelper handles planer blits in a single calls.
                }
                else
                {
                    const D3D12_BOX* pBox = nullptr;
                    if (args.m_dimensionsCoverEntireResource == false)
                    {
                        pBox = &sourceBox;
                    }

                    context.ResourceCopyRegion(destination.GetUnderlyingResource(),
                        destinationSubresourceIndex,
                        destinationX, destinationY, destinationZ,
                        source.GetUnderlyingResource(),
                        sourceSubresourceIndex,
                        pBox);
                }
            }
        }
    }

    HRESULT Device::DoBlit(_In_ CONST D3DDDIARG_BLT* pBltArgs, BOOL EnableAlpha)
    {
        Resource* pSource = Resource::GetResourceFromHandle(pBltArgs->hSrcResource);
        Resource* pDestination = Resource::GetResourceFromHandle(pBltArgs->hDstResource);

        if (pSource == nullptr || pDestination == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        //TODO: Implement Dest ColorKey blit
        Check9on12(pBltArgs->ColorKey == 0 || pBltArgs->Flags.DstColorKey==0);

        // Copy within the same resource
        if (pSource == pDestination)
        {
            Check9on12(pBltArgs->DstSubResourceIndex != pBltArgs->SrcSubResourceIndex);
        }

        {
            DXGI_FORMAT srcFormat = pSource->GetViewFormat(false);
            DXGI_FORMAT dstFormat = pDestination->GetViewFormat(false);

            // Stretch copy pipeline path does not support color space conversion
            if (IsYUVFormat(srcFormat) != IsYUVFormat(dstFormat))
            {
                if (IsYUVFormat(dstFormat))
                {
                    RETURN_E_INVALIDARG_AND_CHECK();
                }
            }
        }

        {
            ResourceCopyArgs resourceCopyArg = ResourceCopyArgs(*pDestination, *pSource);

            D3D12TranslationLayer::BlitColorKey blitColorKey;
            DirectX::XMFLOAT4 colorKey = ARGBToUNORMFloat(pBltArgs->ColorKey);
            memcpy(blitColorKey.ColorKey, &colorKey, sizeof(float) * 4);
            blitColorKey.Type = pBltArgs->Flags.SrcColorKey ? D3D12TranslationLayer::COLORKEY_SRC :
                                pBltArgs->Flags.DstColorKey ? D3D12TranslationLayer::COLORKEY_DEST :
                                D3D12TranslationLayer::COLORKEY_NONE;

            resourceCopyArg.m_EnableAlpha = EnableAlpha;
            resourceCopyArg.AsSubresourceBlit(pBltArgs->DstSubResourceIndex, pBltArgs->SrcSubResourceIndex, &pBltArgs->DstRect, &pBltArgs->SrcRect);

            Resource::CopySubResource(*this, resourceCopyArg);
        }
        return S_OK;
    }

    _Check_return_ HRESULT APIENTRY Blit(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_BLT* pBltArgs)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pBltArgs == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = pDevice->DoBlit(pBltArgs, false);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    // In D3D9 the app can copy between two textures that have the same dimensions, but different mip
    // counts. When this happens we copy over all of the mip levels that match (missing the larger ones
    // which exist in one texture).
    void Resource::CopyMatchingMipLevels(Device& device, ResourceCopyArgs& args)
    {
        Resource& destination = args.m_destination;
        Resource& source = args.m_source;

        Check9on12(destination.GetArraySize() == source.GetArraySize());

        UINT srcStartMip = 0;
        UINT dstStartMip = 0;

        auto SkipLargerMips = [](UINT& leftMip, Resource& leftResource, Resource& rightResource)
        {
            UINT numLeftMips = leftResource.GetNumAppVisibleMips();
            for (; leftMip < numLeftMips; ++leftMip)
            {
                if (leftResource.GetLogicalDesc().Width >> leftMip <= rightResource.GetLogicalDesc().Width ||
                    leftResource.GetLogicalDesc().Height >> leftMip <= rightResource.GetLogicalDesc().Height)
                {
                    break;
                }
            }
        };
        SkipLargerMips(srcStartMip, source, destination);
        SkipLargerMips(dstStartMip, destination, source);
        Check9on12(max(source.m_logicalDesc.Width >> srcStartMip, 1ull) == max(destination.m_logicalDesc.Width >> dstStartMip, 1ull));
        Check9on12(max(source.m_logicalDesc.Height >> srcStartMip, 1u) == max(destination.m_logicalDesc.Height >> dstStartMip, 1u));

        UINT mipLevels = min(source.m_logicalDesc.MipLevels - srcStartMip, destination.m_logicalDesc.MipLevels - dstStartMip);

        UINT arrayStart = args.m_cubeFace == UINT_MAX ? 0 : args.m_cubeFace;
        UINT arrayEnd = args.m_cubeFace == UINT_MAX ? destination.GetArraySize() : args.m_cubeFace + 1;
        for (UINT arraySlice = arrayStart; arraySlice < arrayEnd; arraySlice++)
        {
            for (UINT mipSlice = 0; mipSlice < mipLevels; mipSlice++)
            {
                UINT srcMip = srcStartMip + mipSlice;
                UINT dstMip = dstStartMip + mipSlice;

                ResourceCopyArgs arg = args.GetSubArgs(arraySlice, srcMip, dstMip);
                Resource::CopySubResource(device, arg);
            }
        }
    }

    _Check_return_ HRESULT APIENTRY VolumeBlit1(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_VOLUMEBLT1* pBltArgs)
    {
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pBltArgs == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource* pSource = Resource::GetResourceFromHandle(pBltArgs->hSrcResource);
        Resource* pDestination = Resource::GetResourceFromHandle(pBltArgs->hDstResource);

        ResourceCopyArgs args = ResourceCopyArgs(*pDestination, *pSource);
        D3D12_BOX box = {};
        box.left = pBltArgs->SrcBox.Left;
        box.top = pBltArgs->SrcBox.Top;
        box.front = pBltArgs->SrcBox.Front;
        box.right = pBltArgs->SrcBox.Right;
        box.bottom = pBltArgs->SrcBox.Bottom;
        box.back = pBltArgs->SrcBox.Back;

        args.AsVolumeBlit(pBltArgs->DstX, pBltArgs->DstY, pBltArgs->DstZ, box);

        Resource::CopyResource(*pDevice, args);
        return S_OK;
    }

     _Check_return_ HRESULT APIENTRY BufferBlit1(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_BUFFERBLT1* pBltArgs)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pBltArgs == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource* pSource = Resource::GetResourceFromHandle(pBltArgs->hSrcResource);
        Resource* pDestination = Resource::GetResourceFromHandle(pBltArgs->hDstResource);

        if (pSource == nullptr || pDestination == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = S_OK;

        ResourceCopyArgs args = ResourceCopyArgs(*pDestination, *pSource);
        args.AsBufferBlit(pBltArgs->Offset, pBltArgs->SrcRange);

        Resource::CopyResource(*pDevice, args);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY TextureBlit1(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_TEXBLT1* pBltArgs)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pBltArgs == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Resource* pSource = Resource::GetResourceFromHandle(pBltArgs->hSrcResource);
        Resource* pDestination = Resource::GetResourceFromHandle(pBltArgs->hDstResource);

        if (pSource == nullptr || pDestination == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = S_OK;
        // Copy within the same resource
        if (pSource == pDestination)
        {
            Check9on12(false);

            hr = E_NOTIMPL;
        }

        ResourceCopyArgs args = ResourceCopyArgs(*pDestination, *pSource);
        args.AsTextureBlit(pBltArgs->DstPoint, pBltArgs->SrcRect, pBltArgs->CubeMapFace);

        Resource::CopyResource(*pDevice, args);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }
};
