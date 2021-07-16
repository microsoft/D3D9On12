// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    static const D3DCAPS9 cD3D9Caps =
    {
        // DeviceType  
        D3DDEVTYPE_HAL,
        // AdapterOrdinal  
        0,
        // Caps  
        DDCAPS_BLTFOURCC |
        D3DCAPS_READ_SCANLINE |
        //D3DCAPS_OVERLAY |      
        0,
        // Caps2  
        D3DCAPS2_CANAUTOGENMIPMAP |
        D3DCAPS2_FULLSCREENGAMMA |
        //D3DCAPS2_CANMANAGERESOURCE |  
        D3DCAPS2_DYNAMICTEXTURES |
        D3DCAPS2_CANSHARERESOURCE |
        //D3DCAPS2_CANCALIBRATEGAMMA |     
        0,
        // Caps3  
        //D3DCAPS3_RESERVED  
        D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD |
        //D3DCAPS3_MANAGEDDEVICE |  
        D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION |
        D3DCAPS3_COPY_TO_VIDMEM |
        D3DCAPS3_COPY_TO_SYSTEMMEM |
        D3DCAPS3_DXVAHD |
        //D3DCAPS3_DXVAHD_LIMITED |  
        0,
        // PresentationIntervals  
        D3DPRESENT_INTERVAL_ONE |
        D3DPRESENT_INTERVAL_TWO |
        D3DPRESENT_INTERVAL_THREE |
        D3DPRESENT_INTERVAL_FOUR |
        D3DPRESENT_INTERVAL_IMMEDIATE |
        0,
        // CursorCaps  
        D3DCURSORCAPS_COLOR,
        // DevCaps  
        //D3DDEVCAPS_FLOATTLVERTEX |  
        //D3DDEVCAPS_SORTINCREASINGZ |  
        //D3DDEVCAPS_SORTDECREASINGZ |  
        //D3DDEVCAPS_SORTEXACT |  
        D3DDEVCAPS_EXECUTESYSTEMMEMORY |
        D3DDEVCAPS_EXECUTEVIDEOMEMORY |
        D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
        D3DDEVCAPS_TLVERTEXVIDEOMEMORY |
        D3DDEVCAPS_TEXTURESYSTEMMEMORY |
        D3DDEVCAPS_TEXTUREVIDEOMEMORY |
        D3DDEVCAPS_DRAWPRIMTLVERTEX |
        D3DDEVCAPS_CANRENDERAFTERFLIP |
        D3DDEVCAPS_TEXTURENONLOCALVIDMEM |
        D3DDEVCAPS_DRAWPRIMITIVES2 |
        //D3DDEVCAPS_SEPARATETEXTUREMEMORIES |  
        D3DDEVCAPS_DRAWPRIMITIVES2EX |
        D3DDEVCAPS_HWTRANSFORMANDLIGHT |
        D3DDEVCAPS_CANBLTSYSTONONLOCAL |
        D3DDEVCAPS_HWRASTERIZATION |
        D3DDEVCAPS_PUREDEVICE |
        //D3DDEVCAPS_QUINTICRTPATCHES |  
        //D3DDEVCAPS_RTPATCHES |  
        //D3DDEVCAPS_RTPATCHHANDLEZERO |  
        //D3DDEVCAPS_NPATCHES |  
        D3DDEVCAPS_HWVERTEXBUFFER |
        D3DDEVCAPS_HWINDEXBUFFER |
        0,
        // PrimitiveMiscCaps  
        //D3DPMISCCAPS_MASKPLANES  
        D3DPMISCCAPS_MASKZ |
        //D3DPMISCCAPS_LINEPATTERNREP |  
        //D3DPMISCCAPS_CONFORMANT |  
        D3DPMISCCAPS_CULLNONE |
        D3DPMISCCAPS_CULLCW |
        D3DPMISCCAPS_CULLCCW |
        D3DPMISCCAPS_COLORWRITEENABLE |
        D3DPMISCCAPS_CLIPPLANESCALEDPOINTS |
        D3DPMISCCAPS_CLIPTLVERTS |
        D3DPMISCCAPS_TSSARGTEMP |
        D3DPMISCCAPS_BLENDOP |
        //D3DPMISCCAPS_NULLREFERENCE |  
        D3DPMISCCAPS_FOGINFVF |
        D3DPMISCCAPS_INDEPENDENTWRITEMASKS |
        D3DPMISCCAPS_PERSTAGECONSTANT |
        D3DPMISCCAPS_SEPARATEALPHABLEND |
        D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS |
        D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING |
        //D3DPMISCCAPS_FOGVERTEXCLAMPED |  
        D3DPMISCCAPS_POSTBLENDSRGBCONVERT |
        0,
        // RasterCaps,  
        //D3DPRASTERCAPS_DITHER |  
        //D3DPRASTERCAPS_ROP2 |  
        //D3DPRASTERCAPS_XOR |  
        //D3DPRASTERCAPS_PAT |  
        D3DPRASTERCAPS_ZTEST |
        //D3DPRASTERCAPS_SUBPIXEL |  
        //D3DPRASTERCAPS_SUBPIXELX |  
        D3DPRASTERCAPS_FOGVERTEX |
        D3DPRASTERCAPS_FOGTABLE |
        //D3DPRASTERCAPS_STIPPLE |  
        //D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT |  
        //D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT |  
        //D3DPRASTERCAPS_ANTIALIASEDGES |  
        D3DPRASTERCAPS_MIPMAPLODBIAS |
        //D3DPRASTERCAPS_ZBIAS |  
        //D3DPRASTERCAPS_ZBUFFERLESSHSR |  
        D3DPRASTERCAPS_FOGRANGE |
        D3DPRASTERCAPS_ANISOTROPY |
        //D3DPRASTERCAPS_WBUFFER |  
        //D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT |  
        D3DPRASTERCAPS_WFOG |
        D3DPRASTERCAPS_ZFOG |
        D3DPRASTERCAPS_COLORPERSPECTIVE |
        //D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE |  
        D3DPRASTERCAPS_SCISSORTEST |
        D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS |
        D3DPRASTERCAPS_DEPTHBIAS |
        D3DPRASTERCAPS_MULTISAMPLE_TOGGLE |
        0,
        // ZCmpCaps  
        D3DPCMPCAPS_NEVER |
        D3DPCMPCAPS_LESS |
        D3DPCMPCAPS_EQUAL |
        D3DPCMPCAPS_LESSEQUAL |
        D3DPCMPCAPS_GREATER |
        D3DPCMPCAPS_NOTEQUAL |
        D3DPCMPCAPS_GREATEREQUAL |
        D3DPCMPCAPS_ALWAYS |
        0,
        // SrcBlendCaps  
        D3DPBLENDCAPS_ZERO |
        D3DPBLENDCAPS_ONE |
        D3DPBLENDCAPS_SRCCOLOR |
        D3DPBLENDCAPS_INVSRCCOLOR |
        D3DPBLENDCAPS_SRCALPHA |
        D3DPBLENDCAPS_INVSRCALPHA |
        D3DPBLENDCAPS_DESTALPHA |
        D3DPBLENDCAPS_INVDESTALPHA |
        D3DPBLENDCAPS_DESTCOLOR |
        D3DPBLENDCAPS_INVDESTCOLOR |
        D3DPBLENDCAPS_SRCALPHASAT |
        D3DPBLENDCAPS_BOTHSRCALPHA |
        D3DPBLENDCAPS_BOTHINVSRCALPHA |
        D3DPBLENDCAPS_BLENDFACTOR |
        D3DPBLENDCAPS_SRCCOLOR2 |
        D3DPBLENDCAPS_INVSRCCOLOR2 |
        0,
        // DestBlendCaps  
        D3DPBLENDCAPS_ZERO |
        D3DPBLENDCAPS_ONE |
        D3DPBLENDCAPS_SRCCOLOR |
        D3DPBLENDCAPS_INVSRCCOLOR |
        D3DPBLENDCAPS_SRCALPHA |
        D3DPBLENDCAPS_INVSRCALPHA |
        D3DPBLENDCAPS_DESTALPHA |
        D3DPBLENDCAPS_INVDESTALPHA |
        D3DPBLENDCAPS_DESTCOLOR |
        D3DPBLENDCAPS_INVDESTCOLOR |
        D3DPBLENDCAPS_SRCALPHASAT |
        D3DPBLENDCAPS_BOTHSRCALPHA |
        D3DPBLENDCAPS_BOTHINVSRCALPHA |
        D3DPBLENDCAPS_BLENDFACTOR |
        D3DPBLENDCAPS_SRCCOLOR2 |
        D3DPBLENDCAPS_INVSRCCOLOR2 |
        0,
        // AlphaCmpCaps  
        D3DPCMPCAPS_NEVER |
        D3DPCMPCAPS_LESS |
        D3DPCMPCAPS_EQUAL |
        D3DPCMPCAPS_LESSEQUAL |
        D3DPCMPCAPS_GREATER |
        D3DPCMPCAPS_NOTEQUAL |
        D3DPCMPCAPS_GREATEREQUAL |
        D3DPCMPCAPS_ALWAYS |
        0,
        // ShadeCaps  
        //D3DPSHADECAPS_COLORFLATMONO |  
        //D3DPSHADECAPS_COLORFLATRGB |  
        //D3DPSHADECAPS_COLORGOURAUDMONO |  
        D3DPSHADECAPS_COLORGOURAUDRGB |
        //D3DPSHADECAPS_COLORPHONGMONO |  
        //D3DPSHADECAPS_COLORPHONGRGB |  
        //D3DPSHADECAPS_SPECULARFLATMONO |  
        //D3DPSHADECAPS_SPECULARFLATRGB |  
        //D3DPSHADECAPS_SPECULARGOURAUDMONO |  
        D3DPSHADECAPS_SPECULARGOURAUDRGB |
        //D3DPSHADECAPS_SPECULARPHONGMONO |  
        //D3DPSHADECAPS_SPECULARPHONGRGB |  
        //D3DPSHADECAPS_ALPHAFLATBLEND |  
        //D3DPSHADECAPS_ALPHAFLATSTIPPLED |  
        D3DPSHADECAPS_ALPHAGOURAUDBLEND |
        //D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED |  
        //D3DPSHADECAPS_ALPHAPHONGBLEND |  
        //D3DPSHADECAPS_ALPHAPHONGSTIPPLED |  
        //D3DPSHADECAPS_FOGFLAT |  
        D3DPSHADECAPS_FOGGOURAUD |
        //D3DPSHADECAPS_FOGPHONG |  
        0,
        // TextureCaps      
        D3DPTEXTURECAPS_PERSPECTIVE |
        //D3DPTEXTURECAPS_POW2 |  
        D3DPTEXTURECAPS_ALPHA |
        D3DPTEXTURECAPS_TRANSPARENCY |
        D3DPTEXTURECAPS_BORDER |
        //D3DPTEXTURECAPS_SQUAREONLY |  
        D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
        D3DPTEXTURECAPS_ALPHAPALETTE |
        //D3DPTEXTURECAPS_NONPOW2CONDITIONAL |      
        /*RESERVED |*/
        D3DPTEXTURECAPS_PROJECTED |
        D3DPTEXTURECAPS_CUBEMAP |
        D3DPTEXTURECAPS_VOLUMEMAP |
        D3DPTEXTURECAPS_MIPMAP |
        D3DPTEXTURECAPS_MIPVOLUMEMAP |
        D3DPTEXTURECAPS_MIPCUBEMAP |
        //D3DPTEXTURECAPS_CUBEMAP_POW2 |  
        //D3DPTEXTURECAPS_VOLUMEMAP_POW2 |      
        //D3DPTEXTURECAPS_NOPROJECTEDBUMPENV |  
        0,
        // TextureFilterCaps      
        //D3DPTFILTERCAPS_NEAREST |  
        //D3DPTFILTERCAPS_LINEAR |  
        //D3DPTFILTERCAPS_MIPNEAREST |  
        //D3DPTFILTERCAPS_MIPLINEAR |  
        //D3DPTFILTERCAPS_LINEARMIPNEAREST |  
        //D3DPTFILTERCAPS_LINEARMIPLINEAR |  
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MINFPOINT |
        D3DPTFILTERCAPS_MINFLINEAR |
        D3DPTFILTERCAPS_MINFANISOTROPIC |
        //D3DPTFILTERCAPS_MINFPYRAMIDALQUAD |  
        //D3DPTFILTERCAPS_MINFGAUSSIANQUAD |  
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MIPFPOINT |
        D3DPTFILTERCAPS_MIPFLINEAR |
        //D3DPTFILTERCAPS_CONVOLUTIONMONO |  
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MAGFPOINT |
        D3DPTFILTERCAPS_MAGFLINEAR |
        D3DPTFILTERCAPS_MAGFANISOTROPIC |
        //D3DPTFILTERCAPS_MAGFAFLATCUBIC |  
        //D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC |  
        0,
        // CubeTextureFilterCaps  
        //D3DPTFILTERCAPS_NEAREST |  
        //D3DPTFILTERCAPS_LINEAR |  
        //D3DPTFILTERCAPS_MIPNEAREST |  
        //D3DPTFILTERCAPS_MIPLINEAR |  
        //D3DPTFILTERCAPS_LINEARMIPNEAREST |  
        //D3DPTFILTERCAPS_LINEARMIPLINEAR |  
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MINFPOINT |
        D3DPTFILTERCAPS_MINFLINEAR |
        D3DPTFILTERCAPS_MINFANISOTROPIC |
        //D3DPTFILTERCAPS_MINFPYRAMIDALQUAD |  
        //D3DPTFILTERCAPS_MINFGAUSSIANQUAD |  
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MIPFPOINT |
        D3DPTFILTERCAPS_MIPFLINEAR |
        //D3DPTFILTERCAPS_CONVOLUTIONMONO |  
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MAGFPOINT |
        D3DPTFILTERCAPS_MAGFLINEAR |
        D3DPTFILTERCAPS_MAGFANISOTROPIC |
        //D3DPTFILTERCAPS_MAGFAFLATCUBIC |  
        //D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC |  
        0,
        // VolumeTextureFilterCaps  
        //D3DPTFILTERCAPS_NEAREST |  
        //D3DPTFILTERCAPS_LINEAR |  
        //D3DPTFILTERCAPS_MIPNEAREST |  
        //D3DPTFILTERCAPS_MIPLINEAR |  
        //D3DPTFILTERCAPS_LINEARMIPNEAREST |  
        //D3DPTFILTERCAPS_LINEARMIPLINEAR |  
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MINFPOINT |
        D3DPTFILTERCAPS_MINFLINEAR |
        D3DPTFILTERCAPS_MINFANISOTROPIC |
        //D3DPTFILTERCAPS_MINFPYRAMIDALQUAD |  
        //D3DPTFILTERCAPS_MINFGAUSSIANQUAD |  
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MIPFPOINT |
        D3DPTFILTERCAPS_MIPFLINEAR |
        //D3DPTFILTERCAPS_CONVOLUTIONMONO |  
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MAGFPOINT |
        D3DPTFILTERCAPS_MAGFLINEAR |
        D3DPTFILTERCAPS_MAGFANISOTROPIC |
        //D3DPTFILTERCAPS_MAGFAFLATCUBIC |  
        //D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC |  
        0,
        // TextureAddressCaps  
        D3DPTADDRESSCAPS_WRAP |
        D3DPTADDRESSCAPS_MIRROR |
        D3DPTADDRESSCAPS_CLAMP |
        D3DPTADDRESSCAPS_BORDER |
        D3DPTADDRESSCAPS_INDEPENDENTUV |
        D3DPTADDRESSCAPS_MIRRORONCE |
        0,
        // VolumeTextureAddressCaps  
        D3DPTADDRESSCAPS_WRAP |
        D3DPTADDRESSCAPS_MIRROR |
        D3DPTADDRESSCAPS_CLAMP |
        D3DPTADDRESSCAPS_BORDER |
        D3DPTADDRESSCAPS_INDEPENDENTUV |
        D3DPTADDRESSCAPS_MIRRORONCE |
        0,
        // LineCaps  
        // Not required in FL9_x, so not all driver respects D3D11_RASTERIZER_DESC.AntialiasedLineEnable.  
        // D3DLINECAPS_ANTIALIAS |  
        D3DLINECAPS_TEXTURE |
        D3DLINECAPS_ZTEST |
        D3DLINECAPS_BLEND |
        D3DLINECAPS_ALPHACMP |
        D3DLINECAPS_FOG |
        0,
        // MaxTextureWidth  
        MAX_TEXTURE_WIDTH_SIZE,
        // MaxTextureHeight  
        MAX_TEXTURE_HEIGHT_SIZE,
        // MaxVolumeExtent  
        MAX_VOLUME_EXTENT,
        // MaxTextureRepeat  
        MAX_TEXTURE_REPEAT,
        // MaxTextureAspectRatio  
        MAX_TEXTURE_ASPECT_RATIO,
        // MaxAnisotropy  
        16,
        // MaxVertexW  
        1.0e10,
        // GuardBandLeft  
        -1.0e8,
        // GuardBandTop  
        -1.0e8,
        // GuardBandRight  
        1.0e8,
        // GuardBandBottom  
        1.0e8,
        // ExtentsAdjust  
        0.0,
        // StencilCaps  
        D3DSTENCILCAPS_KEEP |
        D3DSTENCILCAPS_ZERO |
        D3DSTENCILCAPS_REPLACE |
        D3DSTENCILCAPS_INCRSAT |
        D3DSTENCILCAPS_DECRSAT |
        D3DSTENCILCAPS_INVERT |
        D3DSTENCILCAPS_INCR |
        D3DSTENCILCAPS_DECR |
        D3DSTENCILCAPS_TWOSIDED |
        0,
        // FVFCaps  
        //D3DFVFCAPS_DONOTSTRIPELEMENTS |  
        8 /*D3DFVFCAPS_TEXCOORDCOUNTMASK*/ |
        D3DFVFCAPS_PSIZE |
        0,
        // TextureOpCaps  
        D3DTEXOPCAPS_DISABLE |
        D3DTEXOPCAPS_SELECTARG1 |
        D3DTEXOPCAPS_SELECTARG2 |
        D3DTEXOPCAPS_MODULATE |
        D3DTEXOPCAPS_MODULATE2X |
        D3DTEXOPCAPS_MODULATE4X |
        D3DTEXOPCAPS_ADD |
        D3DTEXOPCAPS_ADDSIGNED |
        D3DTEXOPCAPS_ADDSIGNED2X |
        D3DTEXOPCAPS_SUBTRACT |
        D3DTEXOPCAPS_ADDSMOOTH |
        D3DTEXOPCAPS_BLENDDIFFUSEALPHA |
        D3DTEXOPCAPS_BLENDTEXTUREALPHA |
        D3DTEXOPCAPS_BLENDFACTORALPHA |
        D3DTEXOPCAPS_BLENDTEXTUREALPHAPM |
        D3DTEXOPCAPS_BLENDCURRENTALPHA |
        D3DTEXOPCAPS_PREMODULATE |
        D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR |
        D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA |
        D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
        D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
        D3DTEXOPCAPS_BUMPENVMAP |
        D3DTEXOPCAPS_BUMPENVMAPLUMINANCE |
        D3DTEXOPCAPS_DOTPRODUCT3 |
        D3DTEXOPCAPS_MULTIPLYADD |
        D3DTEXOPCAPS_LERP |
        0,
        // MaxTextureBlendStages  
        8,
        // MaxSimultaneousTextures  
        8,
        // VertexProcessingCaps  
        D3DVTXPCAPS_TEXGEN_SPHEREMAP |
        D3DVTXPCAPS_TEXGEN |
        D3DVTXPCAPS_MATERIALSOURCE7 |
        D3DVTXPCAPS_DIRECTIONALLIGHTS |
        D3DVTXPCAPS_POSITIONALLIGHTS |
        D3DVTXPCAPS_TWEENING |
        D3DVTXPCAPS_LOCALVIEWER |
        0,
        // MaxActiveLights  
        MAX_ACTIVE_LIGHTS,
        // MaxUserClipPlanes  
        MAX_USER_CLIPPLANES,
        // MaxVertexBlendMatrices  
        MAX_VERTEX_BLEND_MATRICES,
        // MaxVertexBlendMatrixIndex  
        MAX_VERTEX_BLEND_MATRIX_INDEX,
        // MaxPointSize  
        MAX_POINT_SIZE,
        // MaxPrimitiveCount  
        0x007fffff,
        // MaxVertexIndex  
        0x00ffffff,
        // MaxStreams  
        MAX_VERTEX_STREAMS,
        // MaxStreamStride  
        255,
        // VertexShaderVersion  
        D3DVS_VERSION(3, 0),
        // MaxVertexShaderConst  
        MAX_VS_CONSTANTSF,
        // PixelShaderVersion  
        D3DPS_VERSION(3, 0),
        // PixelShader1xMaxValue  
        FLT_MAX,
        // DevCaps2  
        D3DDEVCAPS2_STREAMOFFSET |
        D3DDEVCAPS2_VERTEXELEMENTSCANSHARESTREAMOFFSET |
        0,
        // MaxNpatchTessellationLevel  
        1,
        // Reserved5  
        0,
        // MasterAdapterOrdinal  
        0,
        // AdapterOrdinalInGroup  
        0,
        // NumberOfAdaptersInGroup  
        0,
        // DeclTypes  
        (D3DDTCAPS_UBYTE4 |
        D3DDTCAPS_UBYTE4N |
        D3DDTCAPS_SHORT2N |
        D3DDTCAPS_SHORT4N |
        D3DDTCAPS_USHORT2N |
        D3DDTCAPS_USHORT4N |
        D3DDTCAPS_UDEC3 |
        D3DDTCAPS_FLOAT16_2 |
        D3DDTCAPS_FLOAT16_4 |
        D3DDTCAPS_DEC3N),
        // NumSimultaneousRTs  
        MAX_RENDER_TARGETS,
        // StretchRectFilterCaps  
        D3DPTFILTERCAPS_MINFPOINT |
        D3DPTFILTERCAPS_MINFLINEAR |
        D3DPTFILTERCAPS_MAGFPOINT |
        D3DPTFILTERCAPS_MAGFLINEAR |
        0,
        // VS20Caps  
        {
            // Caps  
            D3DVS20CAPS_PREDICATION,
            // DynamicFlowControlDepth  
            D3DVS20_MAX_DYNAMICFLOWCONTROLDEPTH,
            // NumTemps  
            D3DVS20_MAX_NUMTEMPS,
            // StaticFlowControlDepth  
            D3DVS20_MAX_STATICFLOWCONTROLDEPTH,
        },
        // PS20Caps  
        {
            // Caps  
            D3DPS20CAPS_ARBITRARYSWIZZLE |
            D3DPS20CAPS_GRADIENTINSTRUCTIONS |
            D3DPS20CAPS_PREDICATION |
            D3DPS20CAPS_NODEPENDENTREADLIMIT |
            D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT |
            0,
            // DynamicFlowControlDepth  
            D3DPS20_MAX_DYNAMICFLOWCONTROLDEPTH,
            // NumTemps  
            D3DPS20_MAX_NUMTEMPS,
            // StaticFlowControlDepth  
            D3DPS20_MAX_STATICFLOWCONTROLDEPTH,
            // NumInstructionSlots  
            D3DPS20_MAX_NUMINSTRUCTIONSLOTS,
        },
        // VertexTextureFilterCaps  
        //D3DPTFILTERCAPS_NEAREST |  
        //D3DPTFILTERCAPS_LINEAR |  
        //D3DPTFILTERCAPS_MIPNEAREST |  
        //D3DPTFILTERCAPS_MIPLINEAR |  
        //D3DPTFILTERCAPS_LINEARMIPNEAREST |  
        //D3DPTFILTERCAPS_LINEARMIPLINEAR |  
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MINFPOINT |
        D3DPTFILTERCAPS_MINFLINEAR |
        //D3DPTFILTERCAPS_MINFANISOTROPIC |  
        //D3DPTFILTERCAPS_MINFPYRAMIDALQUAD |  
        //D3DPTFILTERCAPS_MINFGAUSSIANQUAD |  
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MIPFPOINT |
        D3DPTFILTERCAPS_MIPFLINEAR |
        //D3DPTFILTERCAPS_CONVOLUTIONMONO |  
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        /*RESERVED |*/
        D3DPTFILTERCAPS_MAGFPOINT |
        D3DPTFILTERCAPS_MAGFLINEAR |
        //D3DPTFILTERCAPS_MAGFANISOTROPIC |  
        //D3DPTFILTERCAPS_MAGFAFLATCUBIC |  
        //D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC |  
        0,
        // MaxVShaderInstructionsExecuted  
        D3DINFINITEINSTRUCTIONS,
        // MaxPShaderInstructionsExecuted  
        D3DINFINITEINSTRUCTIONS,
        // MaxVertexShader30InstructionSlots  
        D3DMAX30SHADERINSTRUCTIONS,
        // MaxPixelShader30InstructionSlots  
        D3DMAX30SHADERINSTRUCTIONS,
    };
    static void GetD3D9Caps(D3DCAPS9* pCaps)
    {
        memcpy(pCaps, &cD3D9Caps, sizeof(D3DCAPS9));
    }

    static void GetD3D8Caps(D3DCAPS8* pCaps)
    {
        memcpy(pCaps, &cD3D9Caps, sizeof(D3DCAPS8));
        pCaps->Caps = DDCAPS_3D |
            //DDCAPS_ALIGNBOUNDARYDEST |
            //DDCAPS_ALIGNSIZEDEST |
            //DDCAPS_ALIGNBOUNDARYSRC |
            //DDCAPS_ALIGNSIZESRC |
            //DDCAPS_ALIGNSTRIDE |
            DDCAPS_BLT |
            DDCAPS_BLTQUEUE |
            DDCAPS_BLTFOURCC |
            DDCAPS_BLTSTRETCH |
            //DDCAPS_GDI |
            //DDCAPS_OVERLAY |
            //DDCAPS_OVERLAYCANTCLIP |
            //DDCAPS_OVERLAYFOURCC |
            //DDCAPS_OVERLAYSTRETCH |
            //DDCAPS_PALETTE |
            //DDCAPS_PALETTEVSYNC |
            DDCAPS_READSCANLINE |
            //DDCAPS_RESERVED1 |
            //DDCAPS_VBI |
            //DDCAPS_ZBLTS |
            //DDCAPS_ZOVERLAYS |
            DDCAPS_COLORKEY |
            DDCAPS_ALPHA |
            //DDCAPS_COLORKEYHWASSIST |
            //DDCAPS_NOHARDWARE |
            DDCAPS_BLTCOLORFILL |
            //DDCAPS_BANKSWITCHED |
            DDCAPS_BLTDEPTHFILL |
            //DDCAPS_CANCLIP |
            //DDCAPS_CANCLIPSTRETCHED |
            DDCAPS_CANBLTSYSMEM |
            0;
        pCaps->Caps2 = DDCAPS2_CERTIFIED |
            //DDCAPS2_NO2DDURING3DSCENE |
            //DDCAPS2_VIDEOPORT |
            //DDCAPS2_AUTOFLIPOVERLAY |
            //DDCAPS2_CANBOBINTERLEAVED |
            //DDCAPS2_CANBOBNONINTERLEAVED |
            //DDCAPS2_COLORCONTROLOVERLAY |
            //DDCAPS2_COLORCONTROLPRIMARY |
            //DDCAPS2_CANDROPZ16BIT |
            DDCAPS2_NONLOCALVIDMEM |
            DDCAPS2_NONLOCALVIDMEMCAPS |
            DDCAPS2_NOPAGELOCKREQUIRED |
            DDCAPS2_WIDESURFACES |
            //DDCAPS2_CANFLIPODDEVEN |
            //DDCAPS2_CANBOBHARDWARE |
            DDCAPS2_COPYFOURCC |
            /*RESERVED |*/
            DDCAPS2_PRIMARYGAMMA |
            /*RESERVED |*/
            DDCAPS2_CANRENDERWINDOWED |
            //DDCAPS2_CANCALIBRATEGAMMA |
            DDCAPS2_FLIPINTERVAL |
            DDCAPS2_FLIPNOVSYNC |
            //DDCAPS2_CANMANAGETEXTURE |
            //DDCAPS2_TEXMANINNONLOCALVIDMEM |
            //DDCAPS2_STEREO |
            //DDCAPS2_SYSTONONLOCAL_AS_SYSTOLOCAL |
            //DDCAPS2_RESERVED1 |
            //DDCAPS2_CANMANAGERESOURCE |
            DDCAPS2_DYNAMICTEXTURES |
            DDCAPS2_CANAUTOGENMIPMAP |
            //DDCAPS2_CANSHARERESOURCE |
            0;
        pCaps->Caps3 = //D3DCAPS3_RESERVED
            D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD |
            //D3DCAPS3_MANAGEDDEVICE |
            D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION |
            //D3DCAPS3_COPY_TO_VIDMEM |
            //D3DCAPS3_COPY_TO_SYSTEMMEM |
            //D3DCAPS3_DXVAHD |
            //D3DCAPS3_DXVAHD_LIMITED |
            0;

#define D3DPRESENT_INTERVAL_DX8VALID    0x8000000fL
#define D3DCURSORCAPS_DX8VALID          0x00000003L
#define D3DDEVCAPS_DX8VALID             0x0ffbfff0L
#define D3DPMISCCAPS_DX8VALID           0x00003FF6L
#define D3DPRASTERCAPS_DX8VALID         0x00f7f19fL
#define D3DPCMPCAPS_DX8VALID            0x000000ffL
#define D3DPBLENDCAPS_DX8VALID          0x00001FFFL
#define D3DPSHADECAPS_DX8VALID          0x00084208L
#define D3DPTEXTURECAPS_DX8VALID        0x0007EDE7L
#define D3DPTFILTERCAPS_DX8VALID        0x1F030700L
#define D3DPTADDRESSCAPS_DX8VALID       0x0000003FL
#define D3DLINECAPS_DX8VALID            0x0000001FL
#define D3DSTENCILCAPS_DX8VALID         0x000000FFL
#define D3DFVFCAPS_DX8VALID             0x0018ffffL
#define D3DTEXOPCAPS_DX8VALID           0x03ffffffL
#define D3DVTXPCAPS_DX8VALID            0x000000fbL
        pCaps->PresentationIntervals &= D3DPRESENT_INTERVAL_DX8VALID;
        pCaps->CursorCaps &= D3DCURSORCAPS_DX8VALID;
        pCaps->DevCaps &= D3DDEVCAPS_DX8VALID;
        pCaps->PrimitiveMiscCaps &= D3DPMISCCAPS_DX8VALID;
        pCaps->RasterCaps &= D3DPRASTERCAPS_DX8VALID;
        pCaps->ZCmpCaps &= D3DPCMPCAPS_DX8VALID;
        pCaps->SrcBlendCaps &= D3DPBLENDCAPS_DX8VALID;
        pCaps->DestBlendCaps &= D3DPBLENDCAPS_DX8VALID;
        pCaps->AlphaCmpCaps &= D3DPCMPCAPS_DX8VALID;
        pCaps->ShadeCaps &= D3DPSHADECAPS_DX8VALID;
        pCaps->TextureCaps &= D3DPTEXTURECAPS_DX8VALID;
        pCaps->TextureFilterCaps &= D3DPTFILTERCAPS_DX8VALID;
        pCaps->CubeTextureFilterCaps &= D3DPTFILTERCAPS_DX8VALID;
        pCaps->VolumeTextureFilterCaps &= D3DPTFILTERCAPS_DX8VALID;
        pCaps->TextureAddressCaps &= D3DPTADDRESSCAPS_DX8VALID;
        pCaps->VolumeTextureAddressCaps &= D3DPTADDRESSCAPS_DX8VALID;
        pCaps->LineCaps &= D3DLINECAPS_DX8VALID;
        pCaps->StencilCaps &= D3DSTENCILCAPS_DX8VALID;
        pCaps->FVFCaps &= D3DFVFCAPS_DX8VALID;
        pCaps->TextureOpCaps &= D3DTEXOPCAPS_DX8VALID;
        pCaps->VertexProcessingCaps &= D3DVTXPCAPS_DX8VALID;
        pCaps->VertexShaderVersion = D3DVS_VERSION(1, 1);
        pCaps->PixelShaderVersion = D3DPS_VERSION(1, 4);
    }

    static void GetD3D7Caps(D3DHAL_D3DEXTENDEDCAPS* pCaps)
    {
        pCaps->dwSize = sizeof(D3DHAL_D3DEXTENDEDCAPS);
        pCaps->dwMinTextureWidth = 1;
        pCaps->dwMaxTextureWidth = cD3D9Caps.MaxTextureWidth;
        pCaps->dwMinTextureHeight = 1;
        pCaps->dwMaxTextureHeight = cD3D9Caps.MaxTextureHeight;
        pCaps->dwMinStippleWidth = 0;
        pCaps->dwMaxStippleWidth = 0;
        pCaps->dwMinStippleHeight = 0;
        pCaps->dwMaxStippleHeight = 0;
        pCaps->dwMaxTextureRepeat = cD3D9Caps.MaxTextureRepeat;
        pCaps->dwMaxTextureAspectRatio = cD3D9Caps.MaxTextureAspectRatio;
        pCaps->dwMaxAnisotropy = cD3D9Caps.MaxAnisotropy;
        pCaps->dvGuardBandLeft = cD3D9Caps.GuardBandLeft;
        pCaps->dvGuardBandTop = cD3D9Caps.GuardBandTop;
        pCaps->dvGuardBandRight = cD3D9Caps.GuardBandRight;
        pCaps->dvGuardBandBottom = cD3D9Caps.GuardBandBottom;
        pCaps->dvExtentsAdjust = cD3D9Caps.ExtentsAdjust;
        pCaps->dwStencilCaps = cD3D9Caps.StencilCaps & ~(D3DSTENCILCAPS_TWOSIDED);
        pCaps->dwFVFCaps = cD3D9Caps.FVFCaps & ~(D3DFVFCAPS_PSIZE);
        pCaps->dwTextureOpCaps = cD3D9Caps.TextureOpCaps & ~(D3DTEXOPCAPS_MULTIPLYADD | D3DTEXOPCAPS_LERP);
        pCaps->wMaxTextureBlendStages = (WORD)cD3D9Caps.MaxTextureBlendStages;
        pCaps->wMaxSimultaneousTextures = (WORD)cD3D9Caps.MaxSimultaneousTextures;
        pCaps->dwMaxActiveLights = cD3D9Caps.MaxActiveLights;
        pCaps->dvMaxVertexW = cD3D9Caps.MaxVertexW;
        pCaps->wMaxUserClipPlanes = (WORD)cD3D9Caps.MaxUserClipPlanes;
        pCaps->wMaxVertexBlendMatrices = (WORD)cD3D9Caps.MaxVertexBlendMatrices;
        pCaps->dwVertexProcessingCaps = cD3D9Caps.VertexProcessingCaps & ~(D3DVTXPCAPS_TWEENING | D3DVTXPCAPS_TEXGEN_SPHEREMAP);
        pCaps->dwReserved1 = 0;
        pCaps->dwReserved2 = 0;
        pCaps->dwReserved3 = 0;
        pCaps->dwReserved4 = 0;
    }

    static void GetDDrawCaps(DDRAW_CAPS* pCaps)
    {
        static const DDRAW_CAPS cDdrawCaps =
        {
            // Caps
            DDRAW_CAPS_ZBLTS |
            //DDRAW_CAPS_COLORKEY |
            DDRAW_CAPS_BLTDEPTHFILL |
            0,

            // Caps2
            //DDRAW_CAPS2_CANDROPZ16BIT |
            //DDRAW_CAPS2_FLIPINTERVAL |
            //DDRAW_CAPS2_FLIPNOVSYNC |
            DDRAW_CAPS2_DYNAMICTEXTURES |
            0,

            // CKeyCaps
            DDRAW_CKEYCAPS_SRCBLT |
            DDRAW_CKEYCAPS_DESTBLT |
            0,

            // FxCaps
            DDRAW_FXCAPS_BLTMIRRORLEFTRIGHT |
            DDRAW_FXCAPS_BLTMIRRORUPDOWN |
            0,

            // MaxVideoPorts
            0,
        };

        memcpy(pCaps, &cDdrawCaps, sizeof(DDRAW_CAPS));
    }

    static void GetD3D3Caps(D3DHAL_GLOBALDRIVERDATA* pCaps)
    {
        static const D3DHAL_GLOBALDRIVERDATA cD3d3Caps = {
            // dwSize
            sizeof(D3DHAL_GLOBALDRIVERDATA),
            // hwCaps
        {
            // dwSize
            sizeof(D3DDEVICEDESC_V1),

            // dwFlags
            D3DDD_COLORMODEL |
            D3DDD_DEVCAPS |
            //D3DDD_TRANSFORMCAPS |
            //D3DDD_LIGHTINGCAPS |
            //D3DDD_BCLIPPING |
            D3DDD_LINECAPS |
            D3DDD_TRICAPS |
            D3DDD_DEVICERENDERBITDEPTH |
            D3DDD_DEVICEZBUFFERBITDEPTH |
            //D3DDD_MAXBUFFERSIZE |
            //D3DDD_MAXVERTEXCOUNT |
            0,

            // dcmColorModel
            D3DCOLOR_RGB,

            // dwDevCaps
            D3DDEVCAPS_FLOATTLVERTEX |
            //D3DDEVCAPS_SORTINCREASINGZ |
            //D3DDEVCAPS_SORTDECREASINGZ |
            //D3DDEVCAPS_SORTEXACT |
            D3DDEVCAPS_EXECUTESYSTEMMEMORY |
            D3DDEVCAPS_EXECUTEVIDEOMEMORY |
            D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
            D3DDEVCAPS_TLVERTEXVIDEOMEMORY |
            D3DDEVCAPS_TEXTURESYSTEMMEMORY |
            D3DDEVCAPS_TEXTUREVIDEOMEMORY |
            D3DDEVCAPS_DRAWPRIMTLVERTEX |
            D3DDEVCAPS_CANRENDERAFTERFLIP |
            D3DDEVCAPS_TEXTURENONLOCALVIDMEM |
            D3DDEVCAPS_DRAWPRIMITIVES2 |
            //D3DDEVCAPS_SEPARATETEXTUREMEMORIES |
            D3DDEVCAPS_DRAWPRIMITIVES2EX |
            D3DDEVCAPS_HWTRANSFORMANDLIGHT |
            D3DDEVCAPS_CANBLTSYSTONONLOCAL |
            D3DDEVCAPS_HWRASTERIZATION |
            //D3DDEVCAPS_PUREDEVICE |
            //D3DDEVCAPS_QUINTICRTPATCHES |
            //D3DDEVCAPS_RTPATCHES |
            //D3DDEVCAPS_RTPATCHHANDLEZERO |
            //D3DDEVCAPS_NPATCHES |
            //D3DDEVCAPS_HWVERTEXBUFFER |
            //D3DDEVCAPS_HWINDEXBUFFER |
            0,

            // dtcTransformCaps
        {
            0, 0
        },
            // bClipping
            TRUE,
            // dlcLightingCaps
        {
            0, 0, 0, 0,
        },
            {   // dpcLineCaps
                sizeof(D3DPRIMCAPS),

                // dwMiscCaps
                //D3DPMISCCAPS_MASKPLANES
                D3DPMISCCAPS_MASKZ |
                //D3DPMISCCAPS_LINEPATTERNREP |
                //D3DPMISCCAPS_CONFORMANT |
                //D3DPMISCCAPS_CULLNONE |
                //D3DPMISCCAPS_CULLCW |
                //D3DPMISCCAPS_CULLCCW |
                //D3DPMISCCAPS_COLORWRITEENABLE |
                //D3DPMISCCAPS_CLIPPLANESCALEDPOINTS |
                //D3DPMISCCAPS_CLIPTLVERTS |
                //D3DPMISCCAPS_TSSARGTEMP |
                //D3DPMISCCAPS_BLENDOP |
                //D3DPMISCCAPS_NULLREFERENCE |
                //D3DPMISCCAPS_FOGINFVF |
                //D3DPMISCCAPS_INDEPENDENTWRITEMASKS |
                //D3DPMISCCAPS_PERSTAGECONSTANT |
                //D3DPMISCCAPS_SEPARATEALPHABLEND |
                //D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS |
                //D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING |
                //D3DPMISCCAPS_FOGVERTEXCLAMPED |
                //D3DPMISCCAPS_POSTBLENDSRGBCONVERT |
            0,

            // dwRasterCaps
            //D3DPRASTERCAPS_DITHER |
            //D3DPRASTERCAPS_ROP2 |
            //D3DPRASTERCAPS_XOR |
            //D3DPRASTERCAPS_PAT |
            D3DPRASTERCAPS_ZTEST |
            D3DPRASTERCAPS_SUBPIXEL |
            //D3DPRASTERCAPS_SUBPIXELX |
            D3DPRASTERCAPS_FOGVERTEX |
            D3DPRASTERCAPS_FOGTABLE |
            //D3DPRASTERCAPS_STIPPLE |
            //D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT |
            D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT |
            D3DPRASTERCAPS_ANTIALIASEDGES |
            D3DPRASTERCAPS_MIPMAPLODBIAS |
            D3DPRASTERCAPS_ZBIAS |
            //D3DPRASTERCAPS_ZBUFFERLESSHSR |
            D3DPRASTERCAPS_FOGRANGE |
            D3DPRASTERCAPS_ANISOTROPY |
            //D3DPRASTERCAPS_WBUFFER |
            //D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT |
            D3DPRASTERCAPS_WFOG |
            D3DPRASTERCAPS_ZFOG |
            //D3DPRASTERCAPS_COLORPERSPECTIVE |
            //D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE |
            //D3DPRASTERCAPS_SCISSORTEST |
            //D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS |
            //D3DPRASTERCAPS_DEPTHBIAS |
            //D3DPRASTERCAPS_MULTISAMPLE_TOGGLE |
            0,

            // dwZCmpCaps
            D3DPCMPCAPS_NEVER |
            D3DPCMPCAPS_LESS |
            D3DPCMPCAPS_EQUAL |
            D3DPCMPCAPS_LESSEQUAL |
            D3DPCMPCAPS_GREATER |
            D3DPCMPCAPS_NOTEQUAL |
            D3DPCMPCAPS_GREATEREQUAL |
            D3DPCMPCAPS_ALWAYS |
            0,

            // dwSrcBlendCaps
            D3DPBLENDCAPS_ZERO |
            D3DPBLENDCAPS_ONE |
            D3DPBLENDCAPS_SRCCOLOR |
            D3DPBLENDCAPS_INVSRCCOLOR |
            D3DPBLENDCAPS_SRCALPHA |
            D3DPBLENDCAPS_INVSRCALPHA |
            D3DPBLENDCAPS_DESTALPHA |
            D3DPBLENDCAPS_INVDESTALPHA |
            D3DPBLENDCAPS_DESTCOLOR |
            D3DPBLENDCAPS_INVDESTCOLOR |
            D3DPBLENDCAPS_SRCALPHASAT |
            D3DPBLENDCAPS_BOTHSRCALPHA |
            D3DPBLENDCAPS_BOTHINVSRCALPHA |
            //D3DPBLENDCAPS_BLENDFACTOR |
            //D3DPBLENDCAPS_SRCCOLOR2 |
            //D3DPBLENDCAPS_INVSRCCOLOR2 |
            0,

            // dwDestBlendCaps
            D3DPBLENDCAPS_ZERO |
            D3DPBLENDCAPS_ONE |
            D3DPBLENDCAPS_SRCCOLOR |
            D3DPBLENDCAPS_INVSRCCOLOR |
            D3DPBLENDCAPS_SRCALPHA |
            D3DPBLENDCAPS_INVSRCALPHA |
            D3DPBLENDCAPS_DESTALPHA |
            D3DPBLENDCAPS_INVDESTALPHA |
            D3DPBLENDCAPS_DESTCOLOR |
            D3DPBLENDCAPS_INVDESTCOLOR |
            D3DPBLENDCAPS_SRCALPHASAT |
            D3DPBLENDCAPS_BOTHSRCALPHA |
            D3DPBLENDCAPS_BOTHINVSRCALPHA |
            //D3DPBLENDCAPS_BLENDFACTOR |
            //D3DPBLENDCAPS_SRCCOLOR2 |
            //D3DPBLENDCAPS_INVSRCCOLOR2 |
            0,

            // dwAlphaCmpCaps
            D3DPCMPCAPS_NEVER |
            D3DPCMPCAPS_LESS |
            D3DPCMPCAPS_EQUAL |
            D3DPCMPCAPS_LESSEQUAL |
            D3DPCMPCAPS_GREATER |
            D3DPCMPCAPS_NOTEQUAL |
            D3DPCMPCAPS_GREATEREQUAL |
            D3DPCMPCAPS_ALWAYS |
            0,

            // dwShadeCaps
            //D3DPSHADECAPS_COLORFLATMONO |
            D3DPSHADECAPS_COLORFLATRGB |
            //D3DPSHADECAPS_COLORGOURAUDMONO |
            D3DPSHADECAPS_COLORGOURAUDRGB |
            //D3DPSHADECAPS_COLORPHONGMONO |
            //D3DPSHADECAPS_COLORPHONGRGB |
            //D3DPSHADECAPS_SPECULARFLATMONO |
            D3DPSHADECAPS_SPECULARFLATRGB |
            //D3DPSHADECAPS_SPECULARGOURAUDMONO |
            D3DPSHADECAPS_SPECULARGOURAUDRGB |
            //D3DPSHADECAPS_SPECULARPHONGMONO |
            //D3DPSHADECAPS_SPECULARPHONGRGB |
            D3DPSHADECAPS_ALPHAFLATBLEND |
            //D3DPSHADECAPS_ALPHAFLATSTIPPLED |
            D3DPSHADECAPS_ALPHAGOURAUDBLEND |
            //D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED |
            //D3DPSHADECAPS_ALPHAPHONGBLEND |
            //D3DPSHADECAPS_ALPHAPHONGSTIPPLED |
            D3DPSHADECAPS_FOGFLAT |
            D3DPSHADECAPS_FOGGOURAUD |
            //D3DPSHADECAPS_FOGPHONG |
            0,

            // dwTextureCaps
            D3DPTEXTURECAPS_PERSPECTIVE |
            //D3DPTEXTURECAPS_POW2 |
            D3DPTEXTURECAPS_ALPHA |
            D3DPTEXTURECAPS_TRANSPARENCY |
            D3DPTEXTURECAPS_BORDER |
            //D3DPTEXTURECAPS_SQUAREONLY |
            D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
            D3DPTEXTURECAPS_ALPHAPALETTE |
            //D3DPTEXTURECAPS_NONPOW2CONDITIONAL |
            /*RESERVED |*/
            D3DPTEXTURECAPS_PROJECTED |
            D3DPTEXTURECAPS_CUBEMAP |
            //D3DPTEXTURECAPS_VOLUMEMAP |
            //D3DPTEXTURECAPS_MIPMAP |
            //D3DPTEXTURECAPS_MIPVOLUMEMAP |
            //D3DPTEXTURECAPS_MIPCUBEMAP |
            //D3DPTEXTURECAPS_CUBEMAP_POW2 |
            //D3DPTEXTURECAPS_VOLUMEMAP_POW2 |
            //D3DPTEXTURECAPS_NOPROJECTEDBUMPENV |
            0,

            // dwTextureFilterCaps
            D3DPTFILTERCAPS_NEAREST |
            D3DPTFILTERCAPS_LINEAR |
            D3DPTFILTERCAPS_MIPNEAREST |
            D3DPTFILTERCAPS_MIPLINEAR |
            D3DPTFILTERCAPS_LINEARMIPNEAREST |
            D3DPTFILTERCAPS_LINEARMIPLINEAR |
            /*RESERVED |*/
            /*RESERVED |*/
            D3DPTFILTERCAPS_MINFPOINT |
            D3DPTFILTERCAPS_MINFLINEAR |
            D3DPTFILTERCAPS_MINFANISOTROPIC |
            //D3DPTFILTERCAPS_MINFPYRAMIDALQUAD |
            //D3DPTFILTERCAPS_MINFGAUSSIANQUAD |
            /*RESERVED |*/
            /*RESERVED |*/
            /*RESERVED |*/
            D3DPTFILTERCAPS_MIPFPOINT |
            D3DPTFILTERCAPS_MIPFLINEAR |
            //D3DPTFILTERCAPS_CONVOLUTIONMONO |
            /*RESERVED |*/
            /*RESERVED |*/
            /*RESERVED |*/
            /*RESERVED |*/
            /*RESERVED |*/
            D3DPTFILTERCAPS_MAGFPOINT |
            D3DPTFILTERCAPS_MAGFLINEAR |
            //D3DPTFILTERCAPS_MAGFANISOTROPIC |
            //D3DPTFILTERCAPS_MAGFAFLATCUBIC |
            //D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC |
            0,

            // dwTextureBlendCaps
            D3DPTBLENDCAPS_DECAL |
            D3DPTBLENDCAPS_MODULATE |
            D3DPTBLENDCAPS_DECALALPHA |
            D3DPTBLENDCAPS_MODULATEALPHA |
            //D3DPTBLENDCAPS_DECALMASK |
            //D3DPTBLENDCAPS_MODULATEMASK |
            D3DPTBLENDCAPS_COPY |
            D3DPTBLENDCAPS_ADD |
            0,

            // dwTextureAddressCaps
            D3DPTADDRESSCAPS_WRAP |
            D3DPTADDRESSCAPS_MIRROR |
            D3DPTADDRESSCAPS_CLAMP |
            D3DPTADDRESSCAPS_BORDER |
            D3DPTADDRESSCAPS_INDEPENDENTUV |
            //D3DPTADDRESSCAPS_MIRRORONCE |
            0,

            // dwStippleWidth
            0,

            // dwStippleHeight
            0,
            },
            {   // dpcTriCaps
                sizeof(D3DPRIMCAPS),

                // dwMiscCaps
                //D3DPMISCCAPS_MASKPLANES
                D3DPMISCCAPS_MASKZ |
                //D3DPMISCCAPS_LINEPATTERNREP |
                //D3DPMISCCAPS_CONFORMANT |
            D3DPMISCCAPS_CULLNONE |
            D3DPMISCCAPS_CULLCW |
            D3DPMISCCAPS_CULLCCW |
            //D3DPMISCCAPS_COLORWRITEENABLE |
            //D3DPMISCCAPS_CLIPPLANESCALEDPOINTS |
            //D3DPMISCCAPS_CLIPTLVERTS |
            //D3DPMISCCAPS_TSSARGTEMP |
            //D3DPMISCCAPS_BLENDOP |
            //D3DPMISCCAPS_NULLREFERENCE |
            //D3DPMISCCAPS_FOGINFVF |
            //D3DPMISCCAPS_INDEPENDENTWRITEMASKS |
            //D3DPMISCCAPS_PERSTAGECONSTANT |
            //D3DPMISCCAPS_SEPARATEALPHABLEND |
            //D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS |
            //D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING |
            //D3DPMISCCAPS_FOGVERTEXCLAMPED |
            //D3DPMISCCAPS_POSTBLENDSRGBCONVERT |
            0,

            // dwRasterCaps
            //D3DPRASTERCAPS_DITHER |
            //D3DPRASTERCAPS_ROP2 |
            //D3DPRASTERCAPS_XOR |
            //D3DPRASTERCAPS_PAT |
            D3DPRASTERCAPS_ZTEST |
            D3DPRASTERCAPS_SUBPIXEL |
            //D3DPRASTERCAPS_SUBPIXELX |
            D3DPRASTERCAPS_FOGVERTEX |
            D3DPRASTERCAPS_FOGTABLE |
            //D3DPRASTERCAPS_STIPPLE |
            //D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT |
            D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT |
            D3DPRASTERCAPS_ANTIALIASEDGES |
            D3DPRASTERCAPS_MIPMAPLODBIAS |
            D3DPRASTERCAPS_ZBIAS |
            //D3DPRASTERCAPS_ZBUFFERLESSHSR |
            D3DPRASTERCAPS_FOGRANGE |
            D3DPRASTERCAPS_ANISOTROPY |
            //D3DPRASTERCAPS_WBUFFER |
            //D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT |
            D3DPRASTERCAPS_WFOG |
            D3DPRASTERCAPS_ZFOG |
            //D3DPRASTERCAPS_COLORPERSPECTIVE |
            //D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE |
            //D3DPRASTERCAPS_SCISSORTEST |
            //D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS |
            //D3DPRASTERCAPS_DEPTHBIAS |
            //D3DPRASTERCAPS_MULTISAMPLE_TOGGLE |
            0,

            // dwZCmpCaps
            D3DPCMPCAPS_NEVER |
            D3DPCMPCAPS_LESS |
            D3DPCMPCAPS_EQUAL |
            D3DPCMPCAPS_LESSEQUAL |
            D3DPCMPCAPS_GREATER |
            D3DPCMPCAPS_NOTEQUAL |
            D3DPCMPCAPS_GREATEREQUAL |
            D3DPCMPCAPS_ALWAYS |
            0,

            // dwSrcBlendCaps
            D3DPBLENDCAPS_ZERO |
            D3DPBLENDCAPS_ONE |
            D3DPBLENDCAPS_SRCCOLOR |
            D3DPBLENDCAPS_INVSRCCOLOR |
            D3DPBLENDCAPS_SRCALPHA |
            D3DPBLENDCAPS_INVSRCALPHA |
            D3DPBLENDCAPS_DESTALPHA |
            D3DPBLENDCAPS_INVDESTALPHA |
            D3DPBLENDCAPS_DESTCOLOR |
            D3DPBLENDCAPS_INVDESTCOLOR |
            D3DPBLENDCAPS_SRCALPHASAT |
            D3DPBLENDCAPS_BOTHSRCALPHA |
            D3DPBLENDCAPS_BOTHINVSRCALPHA |
            //D3DPBLENDCAPS_BLENDFACTOR |
            //D3DPBLENDCAPS_SRCCOLOR2 |
            //D3DPBLENDCAPS_INVSRCCOLOR2 |
            0,

            // dwDestBlendCaps
            D3DPBLENDCAPS_ZERO |
            D3DPBLENDCAPS_ONE |
            D3DPBLENDCAPS_SRCCOLOR |
            D3DPBLENDCAPS_INVSRCCOLOR |
            D3DPBLENDCAPS_SRCALPHA |
            D3DPBLENDCAPS_INVSRCALPHA |
            D3DPBLENDCAPS_DESTALPHA |
            D3DPBLENDCAPS_INVDESTALPHA |
            D3DPBLENDCAPS_DESTCOLOR |
            D3DPBLENDCAPS_INVDESTCOLOR |
            D3DPBLENDCAPS_SRCALPHASAT |
            D3DPBLENDCAPS_BOTHSRCALPHA |
            D3DPBLENDCAPS_BOTHINVSRCALPHA |
            //D3DPBLENDCAPS_BLENDFACTOR |
            //D3DPBLENDCAPS_SRCCOLOR2 |
            //D3DPBLENDCAPS_INVSRCCOLOR2 |
            0,

            // dwAlphaCmpCaps
            D3DPCMPCAPS_NEVER |
            D3DPCMPCAPS_LESS |
            D3DPCMPCAPS_EQUAL |
            D3DPCMPCAPS_LESSEQUAL |
            D3DPCMPCAPS_GREATER |
            D3DPCMPCAPS_NOTEQUAL |
            D3DPCMPCAPS_GREATEREQUAL |
            D3DPCMPCAPS_ALWAYS |
            0,

            // dwShadeCaps
            //D3DPSHADECAPS_COLORFLATMONO |
            D3DPSHADECAPS_COLORFLATRGB |
            //D3DPSHADECAPS_COLORGOURAUDMONO |
            D3DPSHADECAPS_COLORGOURAUDRGB |
            //D3DPSHADECAPS_COLORPHONGMONO |
            //D3DPSHADECAPS_COLORPHONGRGB |
            //D3DPSHADECAPS_SPECULARFLATMONO |
            D3DPSHADECAPS_SPECULARFLATRGB |
            //D3DPSHADECAPS_SPECULARGOURAUDMONO |
            D3DPSHADECAPS_SPECULARGOURAUDRGB |
            //D3DPSHADECAPS_SPECULARPHONGMONO |
            //D3DPSHADECAPS_SPECULARPHONGRGB |
            D3DPSHADECAPS_ALPHAFLATBLEND |
            //D3DPSHADECAPS_ALPHAFLATSTIPPLED |
            D3DPSHADECAPS_ALPHAGOURAUDBLEND |
            //D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED |
            //D3DPSHADECAPS_ALPHAPHONGBLEND |
            //D3DPSHADECAPS_ALPHAPHONGSTIPPLED |
            D3DPSHADECAPS_FOGFLAT |
            D3DPSHADECAPS_FOGGOURAUD |
            //D3DPSHADECAPS_FOGPHONG |
            0,

            // dwTextureCaps
            D3DPTEXTURECAPS_PERSPECTIVE |
            //D3DPTEXTURECAPS_POW2 |
            D3DPTEXTURECAPS_ALPHA |
            D3DPTEXTURECAPS_TRANSPARENCY |
            D3DPTEXTURECAPS_BORDER |
            //D3DPTEXTURECAPS_SQUAREONLY |
            D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
            D3DPTEXTURECAPS_ALPHAPALETTE |
            //D3DPTEXTURECAPS_NONPOW2CONDITIONAL |
            /*RESERVED |*/
            D3DPTEXTURECAPS_PROJECTED |
            D3DPTEXTURECAPS_CUBEMAP |
            //D3DPTEXTURECAPS_VOLUMEMAP |
            //D3DPTEXTURECAPS_MIPMAP |
            //D3DPTEXTURECAPS_MIPVOLUMEMAP |
            //D3DPTEXTURECAPS_MIPCUBEMAP |
            //D3DPTEXTURECAPS_CUBEMAP_POW2 |
            //D3DPTEXTURECAPS_VOLUMEMAP_POW2 |
            //D3DPTEXTURECAPS_NOPROJECTEDBUMPENV |
            0,

            // dwTextureFilterCaps
            D3DPTFILTERCAPS_NEAREST |
            D3DPTFILTERCAPS_LINEAR |
            D3DPTFILTERCAPS_MIPNEAREST |
            D3DPTFILTERCAPS_MIPLINEAR |
            D3DPTFILTERCAPS_LINEARMIPNEAREST |
            D3DPTFILTERCAPS_LINEARMIPLINEAR |
            /*RESERVED |*/
            /*RESERVED |*/
            D3DPTFILTERCAPS_MINFPOINT |
            D3DPTFILTERCAPS_MINFLINEAR |
            D3DPTFILTERCAPS_MINFANISOTROPIC |
            //D3DPTFILTERCAPS_MINFPYRAMIDALQUAD |
            //D3DPTFILTERCAPS_MINFGAUSSIANQUAD |
            /*RESERVED |*/
            /*RESERVED |*/
            /*RESERVED |*/
            D3DPTFILTERCAPS_MIPFPOINT |
            D3DPTFILTERCAPS_MIPFLINEAR |
            //D3DPTFILTERCAPS_CONVOLUTIONMONO |
            /*RESERVED |*/
            /*RESERVED |*/
            /*RESERVED |*/
            /*RESERVED |*/
            /*RESERVED |*/
            D3DPTFILTERCAPS_MAGFPOINT |
            D3DPTFILTERCAPS_MAGFLINEAR |
            D3DPTFILTERCAPS_MAGFANISOTROPIC |
            //D3DPTFILTERCAPS_MAGFAFLATCUBIC |
            //D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC |
            0,

            // dwTextureBlendCaps
            D3DPTBLENDCAPS_DECAL |
            D3DPTBLENDCAPS_MODULATE |
            D3DPTBLENDCAPS_DECALALPHA |
            D3DPTBLENDCAPS_MODULATEALPHA |
            //D3DPTBLENDCAPS_DECALMASK |
            //D3DPTBLENDCAPS_MODULATEMASK |
            D3DPTBLENDCAPS_COPY |
            D3DPTBLENDCAPS_ADD |
            0,

            // dwTextureAddressCaps
            D3DPTADDRESSCAPS_WRAP |
            D3DPTADDRESSCAPS_MIRROR |
            D3DPTADDRESSCAPS_CLAMP |
            D3DPTADDRESSCAPS_BORDER |
            D3DPTADDRESSCAPS_INDEPENDENTUV |
            //D3DPTADDRESSCAPS_MIRRORONCE |
            0,

            // dwStippleWidth
            0,

            // dwStippleHeight
            0,
            },
            DDBD_16 | DDBD_32,
            DDBD_16 | DDBD_24 | DDBD_32,
            0,
            0,
        },
            0,
            0,
            0,
            NULL
        };

        memcpy(pCaps, &cD3d3Caps, sizeof(D3DHAL_GLOBALDRIVERDATA));
    }

    /*
    * List of operations supported on formats in DX8+ texture list.
    * See the DX8 DDK for a complete description of these flags.
    */
#define D3DFORMAT_OP_TEXTURE                    0x00000001L
#define D3DFORMAT_OP_VOLUMETEXTURE              0x00000002L
#define D3DFORMAT_OP_CUBETEXTURE                0x00000004L
#define D3DFORMAT_OP_OFFSCREEN_RENDERTARGET     0x00000008L
#define D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET   0x00000010L
#define D3DFORMAT_OP_ZSTENCIL                   0x00000040L
#define D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH 0x00000080L

    /*
    * This DDPF flag is used by drivers to signify that this format is new and may be
    * a candidate for hiding from certain applications
    * KEEP THIS DEFINITION IN SYNC WITH THAT OF DDPF_RESERVED1 IN DDRAW.H
    */
#define DDPF_NOVEL_TEXTURE_FORMAT                               0x00100000l


    /*
    * This DDPF flag is used to indicate a DX8+ format capability entry in
    * the texture format list. It is not visible to applications.
    */
#define DDPF_D3DFORMAT                                          0x00200000l

    /*
    * List of operations supported on formats in DX8+ texture list.
    * See the DX8 DDK for a complete description of these flags.
    */
#define D3DFORMAT_OP_TEXTURE                    0x00000001L
#define D3DFORMAT_OP_VOLUMETEXTURE              0x00000002L
#define D3DFORMAT_OP_CUBETEXTURE                0x00000004L
#define D3DFORMAT_OP_OFFSCREEN_RENDERTARGET     0x00000008L
#define D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET   0x00000010L
#define D3DFORMAT_OP_ZSTENCIL                   0x00000040L
#define D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH 0x00000080L

    // This format can be used as a render target if the current display mode
    // is the same depth if the alpha channel is ignored. e.g. if the device 
    // can render to A8R8G8B8 when the display mode is X8R8G8B8, then the
    // format op list entry for A8R8G8B8 should have this cap. 
#define D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET 0x00000100L

    // This format contains DirectDraw support (including Flip).  This flag
    // should not to be set on alpha formats.
#define D3DFORMAT_OP_DISPLAYMODE                0x00000400L

    // The rasterizer can support some level of Direct3D support in this format
    // and implies that the driver can create a Context in this mode (for some 
    // render target format).  When this flag is set, the D3DFORMAT_OP_DISPLAYMODE
    // flag must also be set.
#define D3DFORMAT_OP_3DACCELERATION             0x00000800L

    // If the driver needs a private format to be D3D or driver manageable,
    // then it needs to tell D3D the pixelsize in bits per pixel by setting
    // dwPrivateFormatBitCount in DDPIXELFORMAT and by setting the below
    // format op. If the below format op is not set, then D3D or the driver
    // will NOT be allowed to manage the format.
#define D3DFORMAT_OP_PIXELSIZE                  0x00001000L

    // Indicates that this format can be converted to any RGB format for which
    // D3DFORMAT_MEMBEROFGROUP_ARGB is specified
#define D3DFORMAT_OP_CONVERT_TO_ARGB            0x00002000L

    // Indicates that this format can be used to create offscreen plain surfaces.
#define D3DFORMAT_OP_OFFSCREENPLAIN             0x00004000L

    // Indicated that this format can be read as an SRGB texture (meaning that the
    // sampler will linearize the looked up data)
#define D3DFORMAT_OP_SRGBREAD                   0x00008000L

    // Indicates that this format can be used in the bumpmap instructions
#define D3DFORMAT_OP_BUMPMAP                    0x00010000L

    // Indicates that this format can be sampled by the displacement map sampler
#define D3DFORMAT_OP_DMAP                       0x00020000L

    // Indicates that this format cannot be used with texture filtering
#define D3DFORMAT_OP_NOFILTER                   0x00040000L

    // Indicates that format conversions are supported to this RGB format if
    // D3DFORMAT_OP_CONVERT_TO_ARGB is specified in the source format.
#define D3DFORMAT_MEMBEROFGROUP_ARGB            0x00080000L

    // Indicated that this format can be written as an SRGB target (meaning that the
    // pixel pipe will DE-linearize data on output to format)
#define D3DFORMAT_OP_SRGBWRITE                  0x00100000L

    // Indicates that this format cannot be used with alpha blending
#define D3DFORMAT_OP_NOALPHABLEND               0x00200000L

    //Indicates that the device can auto-generated sublevels for resources
    //of this format
#define D3DFORMAT_OP_AUTOGENMIPMAP              0x00400000L

    // Indicates that this format cannot be used by vertex texture sampler
#define D3DFORMAT_OP_VERTEXTEXTURE              0x00800000L 

    // Indicates that this format supports neither texture coordinate wrap modes, nor mipmapping
#define D3DFORMAT_OP_NOTEXCOORDWRAPNORMIP		0x01000000L

    static UINT GetD3DQueryTypes(D3DQUERYTYPE* pQueryTypes, UINT NumQueries)
    {
        UINT NumQueriesOut = _countof(g_cSupportedQueryTypes);
        if (pQueryTypes)
        {
            NumQueriesOut = min(NumQueriesOut, NumQueries);
            memcpy(pQueryTypes, g_cSupportedQueryTypes, sizeof(D3DQUERYTYPE) * NumQueriesOut);
        }
        return NumQueriesOut;
    }
}
