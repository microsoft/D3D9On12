// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#define DIRECT3D_VERSION 0x0900

#undef WIN32_LEAN_AND_MEAN
#include <windows.h> // Yes, with MINMAX
#include <D3D12TranslationLayerDependencyIncludes.h>
#include <D3D12TranslationLayerIncludes.h>

#include <d3d12tokenizedprogramformat.hpp>
#include <d3d9types.h>
#include <d3d9.h>
#include <dxva.h>
#include <dxva2api.h>

#include <d3dhal.h>
#include <d3dumddi.h>

#include <ShaderConv.h>
#include <d3dcompiler.h>
#include <BlobContainer.h>

#include <d3dkmthk.h>
#include <DirectXMath.h>
#include <d3d9on12DDI.h>

//SDK Headers
#include <atlstr.h>

//STL
#include <utility>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <memory>
#include <cstddef>
#include <functional>
#include <new>
#include <bitset>
#include <algorithm>
#include <tuple>
#include <string>
#include <stack>
#include <deque>


#define BIT( x ) ( 1 << (x) )
#if DBG

#define Check9on12( a ) \
    __pragma(warning(suppress:4127)) /* conditional is constant due to constant macro parameter(s) */ \
    if((a) == FALSE) {DebugBreak();} \

#define CHECK_HR( hr ) Check9on12(SUCCEEDED(hr));
#else
#define Check9on12( a ) (void)(UNREFERENCED_PARAMETER(a))
#define CHECK_HR( hr ) (UNREFERENCED_PARAMETER(hr))
#endif

#define SAFE_DELETE( a ) if( (a) != nullptr ) { delete(a); }
#define D3DFMT_AYUV         ((D3DFORMAT)MAKEFOURCC('A', 'Y', 'U', 'V'))  

//TODO: Find out why WARP uses this
// Dx7- texture stage state
#define  D3DTSS_ADDRESS 12

namespace D3D9on12
{
    enum
    {
        MAX_ACTIVE_LIGHTS = 8,
        MAX_USER_CLIPPLANES = 8,
        MAX_VERTEX_BLEND_MATRICES = 4,
        MAX_WORLD_MATRICES = 12,
        MAX_VERTEX_BLEND_MATRIX_INDEX = 0,
        MAX_VERTEX_STREAMS = 16,
        VERTEX_STREAMS_MASK = ((1 << MAX_VERTEX_STREAMS) - 1),
        INDEX_STREAM_MASK = (1 << MAX_VERTEX_STREAMS),
        MAX_TEXTURE_STAGES = D3DHAL_SAMPLER_MAXSAMP,
        MAX_RENDER_TARGETS = D3D_MAX_SIMULTANEOUS_RENDERTARGETS,
        MAX_TEXTURE_WIDTH_SIZE = (1 << 14),
        MAX_TEXTURE_HEIGHT_SIZE = (1 << 14),
        MAX_VOLUME_EXTENT = (1 << 14),
        MAX_POINT_SIZE = (1 << 13),
        MIN_POINT_SIZE = 1,
        MAX_TEXTURE_REPEAT = (1 << 13),
        MAX_TEXTURE_ASPECT_RATIO = (1 << 13),
        MAX_VS_CONSTANTSF = 256,
        MAX_VS_CONSTANTSI = 16,
        MAX_VS_CONSTANTSB = 16,
        MAX_PS_CONSTANTSF = 224,
        MAX_PS_CONSTANTSI = 16,
        MAX_PS_CONSTANTSB = 16,
        PIXEL_SAMPLER0 = 0,
        MAX_PIXEL_SAMPLERS = D3DHAL_SAMPLER_MAXSAMP,
        DMAP_SAMPLER = MAX_PIXEL_SAMPLERS,
        VERTEX_SAMPLER0 = DMAP_SAMPLER + 1,
        MAX_VERTEX_SAMPLERS = D3DHAL_SAMPLER_MAXVERTEXSAMP,
        MAX_SAMPLERS_STAGES = VERTEX_SAMPLER0 + MAX_VERTEX_SAMPLERS,
        MAX_D3DTSS = 35,
        PIXEL_SAMPLERS_MASK = ((1 << MAX_PIXEL_SAMPLERS) - 1),
        VERTEX_SAMPLERS_MASK = (((1 << MAX_VERTEX_SAMPLERS) - 1) << VERTEX_SAMPLER0),

        MAX_INPUT_ELEMENTS = 32,

        // Used to denote that an Index buffer is bound and it's not immediately clear 
        // how many vertices will be needed in the VB
        UNKNOWN_VERTEX_COUNT = UINT_MAX,

        // First 4 least significant bits set representing the 4 color channels
        COLOR_CHANNEL_MASK = 0xf,

        MAX_MIPS = 14,
        NUM_DXGI_FORMATS = 132,

        MAX_PLANES = 3
    };

    enum ShaderType
    {
        VERTEX_SHADER = 0,
        GEOMETRY_SHADER,
        PIXEL_SHADER,
        NUM_SHADER_TYPES
    };

    const UINT NUM_VS_ROOT_CBVS = 4;
    const UINT NUM_PS_ROOT_CBVS = 6;

    enum RootSignature
    {
        VS_ROOT_CBV0 = 0,
        PS_ROOT_CBV0 = VS_ROOT_CBV0 + NUM_VS_ROOT_CBVS,
        PS_SRVS = PS_ROOT_CBV0 + NUM_PS_ROOT_CBVS,
        VS_SRVS,
        PS_SAMPLERS,
        VS_SAMPLERS,
        NUM_SLOTS,
        FLOAT_CB_ROOT_INDEX = 0,
        INTEGER_CB_ROOT_INDEX = 1,
        BOOLEAN_CB_ROOT_INDEX = 2,
    };

    enum ShaderMasks
    {
        R_MASK = D3D10_SB_COMPONENT_MASK_R,
        RG_MASK = R_MASK | D3D10_SB_COMPONENT_MASK_G,
        RGB_MASK = RG_MASK | D3D10_SB_COMPONENT_MASK_B,
        RGBA_MASK = RGB_MASK | D3D10_SB_COMPONENT_MASK_A
    };

    const D3DFORMAT D3DFMT_INTZ = D3DFORMAT(MAKEFOURCC('I', 'N', 'T', 'Z'));
    const D3DFORMAT D3DFMT_RAWZ = D3DFORMAT(MAKEFOURCC('R', 'A', 'W', 'Z'));
    const D3DFORMAT D3DFMT_DF16 = D3DFORMAT(MAKEFOURCC('D', 'F', '1', '6'));
    const D3DFORMAT D3DFMT_DF24 = D3DFORMAT(MAKEFOURCC('D', 'F', '2', '4'));
    const D3DFORMAT D3DFMT_NULL = D3DFORMAT(MAKEFOURCC('N', 'U', 'L', 'L'));
    const D3DFORMAT D3DFMT_ATI1 = D3DFORMAT(MAKEFOURCC('A', 'T', 'I', '1'));
    const D3DFORMAT D3DFMT_ATI2 = D3DFORMAT(MAKEFOURCC('A', 'T', 'I', '2'));
    const D3DFORMAT D3DFMT_ATOC = D3DFORMAT(MAKEFOURCC('A', 'T', 'O', 'C')); //Nvidia alpha to coverage (ADAPTIVETESS_Y)
    const D3DFORMAT D3DFMT_A2M0 = D3DFORMAT(MAKEFOURCC('A', '2', 'M', '0')); //ATI disable Alpha to coverage (D3DRS_POINTSIZE)
    const D3DFORMAT D3DFMT_A2M1 = D3DFORMAT(MAKEFOURCC('A', '2', 'M', '1')); //ATI enable Alpha to coverage (D3DRS_POINTSIZE)
    const D3DFORMAT D3DFMT_NV12 = D3DFORMAT(MAKEFOURCC('N', 'V', '1', '2'));
    const D3DFORMAT D3DFMT_420O = D3DFORMAT(MAKEFOURCC('4', '2', '0', 'O'));
    const D3DFORMAT D3DFMT_NV11 = D3DFORMAT(MAKEFOURCC('N', 'V', '1', '1'));
    const D3DFORMAT D3DFMT_AI44 = D3DFORMAT(MAKEFOURCC('A', 'I', '4', '4'));
    const D3DFORMAT D3DFMT_IA44 = D3DFORMAT(MAKEFOURCC('I', 'A', '4', '4'));
    const D3DFORMAT D3DFMT_Y410 = D3DFORMAT(MAKEFOURCC('Y', '4', '1', '0'));
    const D3DFORMAT D3DFMT_Y416 = D3DFORMAT(MAKEFOURCC('Y', '4', '1', '6'));
    const D3DFORMAT D3DFMT_P010 = D3DFORMAT(MAKEFOURCC('P', '0', '1', '0'));
    const D3DFORMAT D3DFMT_P016 = D3DFORMAT(MAKEFOURCC('P', '0', '1', '6'));
    const D3DFORMAT D3DFMT_Y210 = D3DFORMAT(MAKEFOURCC('Y', '2', '1', '0'));
    const D3DFORMAT D3DFMT_Y216 = D3DFORMAT(MAKEFOURCC('Y', '2', '1', '6'));
    const D3DFORMAT D3DFMT_YV12 = D3DFORMAT(MAKEFOURCC('Y', 'V', '1', '2'));
    const D3DFORMAT D3DFMT_RESZ = D3DFORMAT(MAKEFOURCC('R', 'E', 'S', 'Z'));

    // This appears to be the value agreed on by IHVs, though this is mostly arbitrary
    const UINT BPP_FOR_IHV_BLOCK_COMPRESSED_FORMATS = 8;

    const D3D_FEATURE_LEVEL MinSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    const bool cSupportsPlanarSRVs = false;

    const UINT RESZ_CODE = 0x7fa05000;
}

DEFINE_GUID(DXVADDI_VideoProcD3D9On12CustomDeinterlaceDevice, 0xe064b480, 0xa8bb, 0x4d81, 0x81, 0x81, 0x59, 0xdc, 0x0b, 0xe5, 0x1d, 0x02);

#include <9on12Registry.h>
#include <9on12Warning.h>
#include <9on12.h>
#include <9on12Util.h>
#include <9on12DDI.h>
#include <9on12AppCompat.h>
#include <9on12FastUploadAllocator.h>
#include <9on12PipelineStateStructures.h>
#include <9on12InputLayout.h>
#include <9on12InputAssembly.h>
#include <9on12PixelStage.h>
#include <9on12PipelineStateCache.h>
#include <9on12Shader.h>
#include <9on12VertexStage.h>
#include <9on12PipelineState.h>
#include <9on12Resource.h>
#include <9on12SwapChain.h>
#include <9on12Constants.h>
#include <9on12Adapter.h>
#include <9on12DataLogger.h>
#include <9on12Fence.h>
#include <9on12VideoDevice.h>
#include <9on12Device.h>
#include <9on12Query.h>
#include <d3d9caps.h>
#include <9on12Caps.h>
#include <9on12VideoTranslate.h>
#include <9on12DecodeDevice.h>
#include <9on12VideoProcessDevice.h>

//INLs
#include <9on12DrawPrologEpilog.inl>
#include <9on12DrawTriangleFan.inl>
#include <9on12DrawApis.inl>
