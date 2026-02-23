// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include <intrin.h>

#if _M_AMD64
#define SSE4_CRC_INPUT_TYPE UINT64
#define SSE4_CRC_FUNCTION _mm_crc32_u64
#elif _M_IX86
#define SSE4_CRC_INPUT_TYPE UINT32
#define SSE4_CRC_FUNCTION _mm_crc32_u32
#endif

#pragma warning(disable: 4063) // Cases in a switch with values that don't belong to the enum

namespace D3D9on12
{
    static volatile long g_CommandID = 0;

    template <typename T> using unique_comptr = D3D12TranslationLayer::unique_comptr<T>;

    struct CleanupResourceBindingsAndRelease
    {
        void operator()(D3D12TranslationLayer::Resource*);
    };
    typedef D3D12TranslationLayer::unique_comptr<D3D12TranslationLayer::Resource,
        CleanupResourceBindingsAndRelease> unique_unbind_resourceptr;

    template<typename T>
    struct inline_destructor
    {
        void operator()(T *pT) const
        {
            pT->~T();
        }
    };

    class Result
    {
    public:
        enum Value : uint8_t
        {
            S_SUCCESS = 0x00,
            S_CHANGE = 0x01,
            E_FAILURE = 0x10,
            E_INVALID_ARG = 0x20
        };

        Result() = default;
        constexpr Result(Value result) : value(result) {}

        constexpr Result operator||(const Result result) { return result.value > value ? result : *this; }

        constexpr bool Succeeded() const { return value < Result::E_FAILURE; }
        constexpr bool Failed() const { return value >= Result::E_FAILURE; }
        constexpr bool operator==(Result other) const noexcept { return value == other.value; }
        constexpr bool operator!=(Result other) const noexcept { return value != other.value; }

        //Get the underlying enum value. This allows nice switch syntax without enabling implicit casting of Result to HRESULT
        constexpr Value AsEnum() const { return value; }
        //Explicit way of translating to HRESULT for communicating with parts of the codebase that haven't been migrated from HRESULT.
        constexpr HRESULT AsHresult() const 
        {
            switch (value)
            {
            case S_SUCCESS:
            case S_CHANGE:
                return S_OK;
            case E_FAILURE: return E_FAIL;
            case E_INVALID_ARG: return E_INVALIDARG;
            default:
                Check9on12(false);
                return E_ILLEGAL_STATE_CHANGE;
            }
        }

        /* Updates the result such that the higher valued result takes precedence */
        void Update(Result newResult)
        {
            if (newResult.value > value)
            {
                value = newResult.value;
            }
        }

    private:
        Value value;
    };

    //
    // Translates DXGI error codes to their D3D9 equivalents so that the
    // correct HRESULTs propagate to the D3D9 runtime.  Non-DXGI codes pass
    // through unchanged.
    //
    // Not an exhuastive list, but the ones most likely to be hit
    //
    inline HRESULT TranslateDxgiHrToD3D9(HRESULT hr)
    {
        switch (hr)
        {
        case DXGI_ERROR_DEVICE_REMOVED:         return D3DDDIERR_DEVICEREMOVED;
        case DXGI_ERROR_DEVICE_HUNG:            return D3DERR_DEVICEHUNG;
        case DXGI_ERROR_DEVICE_RESET:           return D3DERR_DEVICELOST;
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR:  return D3DERR_DRIVERINTERNALERROR;
        case DXGI_ERROR_INVALID_CALL:           return D3DERR_DRIVERINTERNALERROR; // This is a driver error from the app's perspective as it means that 9on12 made an invalid call
        default:                                return hr;
        }
    }

#define D3D9on12_DDI_ENTRYPOINT_START(implemented) \
__pragma(warning(suppress:4127)) /* conditional is constant due to constant macro parameter(s) */ \
    if((implemented) == false && RegistryConstants::g_cBreakOnMissingDDI) \
    { \
        DebugBreak();\
    } \
    if(RegistryConstants::g_cEnableDDISpew)\
    {\
        PrintDebugMessage(std::string(std::string("Command ID: ") + std::to_string(InterlockedIncrement(&g_CommandID))));\
        PrintDebugMessage(WarningStrings::g_cDDIEntryHeader + std::string(__FUNCTION__));\
    }\
    \
    HRESULT EntryPointHr = S_OK; \
    try \
    { \

#define CLOSE_TRYCATCH_AND_STORE_HRESULT(hr) \
        EntryPointHr = hr; \
    } \
    catch (_com_error& hrEx) \
    { \
        EntryPointHr = hrEx.Error(); \
    } \
    catch (std::bad_alloc&) \
    { \
        EntryPointHr = E_OUTOFMEMORY; \
    } \
    EntryPointHr = TranslateDxgiHrToD3D9(EntryPointHr); \

#define D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr) \
    CLOSE_TRYCATCH_AND_STORE_HRESULT(hr) \
    return EntryPointHr; \

#define D3D9on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr) \
    CLOSE_TRYCATCH_AND_STORE_HRESULT(hr) \
__pragma(warning(suppress:4127)) /* conditional is constant due to constant macro parameter(s) */ \
    if(FAILED(hr)) \
    { \
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);  \
        if (pDevice) \
            pDevice->ReportError(EntryPointHr); \
    }

#define D3D9on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN(hDevice, hr, value) \
    CLOSE_TRYCATCH_AND_STORE_HRESULT(hr) \
__pragma(warning(suppress:4127)) /* conditional is constant due to constant macro parameter(s) */ \
    if(FAILED(hr)) \
    { \
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);  \
        if (pDevice) \
            pDevice->ReportError(EntryPointHr); \
    } \
    return value;


#define RETURN_E_INVALIDARG_AND_CHECK() \
    { \
        if (RegistryConstants::g_cCheckOnInvalidArg) \
        { \
            DebugBreak(); \
        } \
        return E_INVALIDARG; \
    }

    //
    // Converts an HRESULT to an exception.  This matches ThrowFailure used
    // elsewhere in dxg.
    //
    inline void ThrowFailure(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw _com_error(hr);
        }
    }

    static void PrintErrorText()
    {
        DWORD retSize;
        LPTSTR pString = NULL;

        retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            NULL,
            GetLastError(),
            LANG_NEUTRAL,
            (LPTSTR)&pString,
            0,
            NULL);
        PrintDebugMessage(std::string(CStringA(pString)));
    }

    const BYTE g_redOutputPS[] =
    {
        0, 2, 255, 255, 254, 255,
        22, 0, 67, 84, 65, 66,
        28, 0, 0, 0, 35, 0,
        0, 0, 0, 2, 255, 255,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0,
        28, 0, 0, 0, 112, 115,
        95, 50, 95, 48, 0, 77,
        105, 99, 114, 111, 115, 111,
        102, 116, 32, 40, 82, 41,
        32, 72, 76, 83, 76, 32,
        83, 104, 97, 100, 101, 114,
        32, 67, 111, 109, 112, 105,
        108, 101, 114, 32, 57, 46,
        50, 55, 46, 57, 53, 50,
        46, 51, 48, 50, 50, 0,
        81, 0, 0, 5, 0, 0,
        15, 160, 0, 0, 128, 63,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 128, 63,
        1, 0, 0, 2, 0, 8,
        15, 128, 0, 0, 228, 160,
        255, 255, 0, 0
    };

    const BYTE g_passThroughVS[] =
    {
        0, 2, 254, 255, 254, 255,
        22, 0, 67, 84, 65, 66,
        28, 0, 0, 0, 35, 0,
        0, 0, 0, 2, 254, 255,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0,
        28, 0, 0, 0, 118, 115,
        95, 50, 95, 48, 0, 77,
        105, 99, 114, 111, 115, 111,
        102, 116, 32, 40, 82, 41,
        32, 72, 76, 83, 76, 32,
        83, 104, 97, 100, 101, 114,
        32, 67, 111, 109, 112, 105,
        108, 101, 114, 32, 49, 48,
        46, 48, 46, 49, 48, 48,
        49, 49, 46, 48, 0, 171,
        31, 0, 0, 2, 0, 0,
        0, 128, 0, 0, 15, 144,
        1, 0, 0, 2, 0, 0,
        15, 192, 0, 0, 228, 144,
        255, 255, 0, 0
    };

    // Can't use the std libraries implementation because the internal build tools don't have the latest std library
    static inline BOOL isnan(const float& a)
    {
        static const UINT exponentMask = 0x7f800000;
        static const UINT mantissaMask = 0x007fffff;

        UINT u = *(const UINT*)&a;
        return (((u & exponentMask) == exponentMask) && (u & mantissaMask)); // NaN  
    }

    static bool isinf(const float &f)
    {
        static const INT32 Max32 = 0x7f7FFFFF;
        UINT n = *(UINT*)&f;
        UINT abs = n & 0x7FFFFFFF;
        return abs > Max32;
    }

    static void AssertFloatNotInfOrNan(float f)
    {
        Check9on12(!isnan(f));
        Check9on12(!isinf(f));
    }

    template<typename DataType>
    static FORCEINLINE DataType Align(DataType location, size_t alignment)
    {
        return (DataType)((location + (alignment - 1)) & ~(alignment - 1));
    }

    template<typename DataType>
    static FORCEINLINE bool IsAligned(DataType location, size_t alignment)
    {
        return location % alignment == 0;
    }

    static FORCEINLINE UINT CeilToClosestPowerOfTwo(UINT32 n)
    {
        // Accomplished using bit magic, only works on numbers
        // won't overflow a 32 bit UINT
        assert(n <= BIT(31));

        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;

        return n;
    }

    static RECT RectThatCoversEntireResource(const D3D12_PLACED_SUBRESOURCE_FOOTPRINT &footprint)
    {
        RECT rect = {};
        rect.right = (LONG)footprint.Footprint.Width;
        rect.bottom = (LONG)footprint.Footprint.Height;
        return rect;
    }

    static RECT RectThatCoversEntireResource(const D3D12_RESOURCE_DESC& desc)
    {
        RECT rect = {};
        rect.right = (LONG)desc.Width;
        rect.bottom = (LONG)desc.Height;
        return rect;
    }

    static bool RectsAreEqual(const RECT& a, const RECT& b)
    {
        return a.bottom == b.bottom && a.left == b.left && a.right == b.right && a.top == b.top;
    }

    static UINT BoxWidth(const D3D12_BOX &b)
    {
        Check9on12(b.right >= b.left);
        return b.right - b.left;
    }

    static UINT BoxHeight(const D3D12_BOX &b)
    {
        Check9on12(b.bottom >= b.top);
        return b.bottom - b.top;
    }

    static UINT BoxDepth(const D3D12_BOX &b)
    {
        Check9on12(b.back >= b.front);
        return b.back - b.front;
    }

    static UINT RectHeight(const RECT& r)
    {
        Check9on12(r.bottom >= r.top);
        return r.bottom - r.top;
    }

    static UINT RectWidth(const RECT& r)
    {
        Check9on12(r.right >= r.left);
        return r.right - r.left;
    }

    static bool IsRectEmpty(const RECT& r)
    {
        return r.left == 0 && r.right == 0 && r.top == 0 && r.bottom == 0;
    }

    static bool IsBoxValid(const D3D12_BOX& box)
    {
        return box.right > box.left && box.bottom > box.top && box.back > box.front;
    }

    static D3D12_BOX BoxFromDesc(const D3D12_RESOURCE_DESC &desc)
    {
        return CD3DX12_BOX(0, 0, 0, static_cast<LONG>(desc.Width), static_cast<LONG>(desc.Height), static_cast<LONG>(desc.DepthOrArraySize));
    }

    // We round down on for left and top; and we round up for  
    // right and bottom  inline void CBaseTexture::ScaleRectDown(RECT *pRect, UINT PowersOfTwo)  
    static void ScaleRectDown(RECT& rect, UINT PowersOfTwo)
    {
        Check9on12(PowersOfTwo > 0);
        Check9on12(PowersOfTwo < 32);
        Check9on12(rect.right > 0);
        Check9on12(rect.bottom > 0);
        Check9on12(rect.left < rect.right);
        Check9on12(rect.top < rect.bottom);
        Check9on12(rect.left >= 0);
        Check9on12(rect.top >= 0);

        // Rounding down is automatic with the shift operator
        rect.left >>= PowersOfTwo; 
        rect.top >>= PowersOfTwo;

        if (rect.right & ((1 << PowersOfTwo) - 1))
        {
            rect.right >>= PowersOfTwo;
            rect.right++;
        }
        else
        {
            rect.right >>= PowersOfTwo;
        }

        if (rect.bottom & ((1 << PowersOfTwo) - 1))
        {
            rect.bottom >>= PowersOfTwo;
            rect.bottom++;
        }
        else
        {
            rect.bottom >>= PowersOfTwo;
        }
    }

    static void ClipBox(D3D12_BOX& box, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT &footprint)
    {
        if (BoxWidth(box) > footprint.Footprint.Width)
        {
            box.right = box.left + footprint.Footprint.Width;
        }

        if (BoxHeight(box) > footprint.Footprint.Height)
        {
            box.bottom = box.left + footprint.Footprint.Height;
        }

        if (BoxDepth(box) > footprint.Footprint.Depth)
        {
            box.back = box.front + footprint.Footprint.Depth;
        }
    }

    static void ScaleBoxDown(D3D12_BOX& box, UINT PowersOfTwo)
    {
        Check9on12(box.front < box.back);
        Check9on12(box.back > 0);

        RECT tempRect = {};
        tempRect.left = box.left;
        tempRect.right = box.right;
        tempRect.top = box.top;
        tempRect.bottom = box.bottom;

        ScaleRectDown(tempRect, PowersOfTwo);

        box.left= tempRect.left;
        box.right = tempRect.right;
        box.top = tempRect.top;
        box.bottom = tempRect.bottom;

        // Rounding down is automatic with the shift operator      
        box.front >>= PowersOfTwo;  
        if (box.back & ((1 << PowersOfTwo) - 1))
        {
            box.back >>= PowersOfTwo;
            box.back++;
        }
        else
        {
            box.back >>= PowersOfTwo;
        }
    }

    static bool IsLumaPlane(UINT PlaneIndex)
    {
        return PlaneIndex == 0;
    }

    static bool IsChromaPlane(UINT PlaneIndex)
    {
        return !IsLumaPlane(PlaneIndex);
    }

    static RECT ConvertLumaRectToChromaRect(const RECT& lumaRect, UINT subsampleX, UINT subsampleY)
    {
        RECT chromaRect;
        chromaRect.left = (lumaRect.left + subsampleX - 1) / subsampleX;
        chromaRect.right = (lumaRect.right + subsampleX - 1) / subsampleX;
        chromaRect.top = (lumaRect.top + subsampleY - 1) / subsampleY;
        chromaRect.bottom = (lumaRect.bottom + subsampleY - 1) / subsampleY;
        return chromaRect;
    }

    static D3DDDI_UNLOCKFLAGS ConvertUnlockAsyncFlag(const D3DDDI_UNLOCKASYNCFLAGS &asyncFlag)
    {
        D3DDDI_UNLOCKFLAGS unlockFlag = {};
        unlockFlag.NotifyOnly = asyncFlag.NotifyOnly;
        return unlockFlag;
    }


    static D3DDDI_LOCKFLAGS ConvertLockAsyncFlags(const D3DDDI_LOCKASYNCFLAGS &asyncFlag)
    {
        D3DDDI_LOCKFLAGS lockFlag = {};

        // TODO: Can we do anything with this info?
        // NoExistingReferences : 1;           // 0x00000020

        lockFlag.NoOverwrite = asyncFlag.NoOverwrite;
        lockFlag.Discard = asyncFlag.Discard;
        lockFlag.RangeValid = asyncFlag.RangeValid;
        lockFlag.AreaValid = asyncFlag.AreaValid;
        lockFlag.BoxValid = asyncFlag.BoxValid;
        lockFlag.NotifyOnly = asyncFlag.NotifyOnly;
        lockFlag.WriteOnly = TRUE;

        return lockFlag;
    }

    static SECURITY_ATTRIBUTES ConvertSecurityDescriptorToSecurityAttributes(const SECURITY_DESCRIPTOR *pDesc)
    {
        SECURITY_ATTRIBUTES attributes = {};
        attributes.lpSecurityDescriptor = (LPVOID)pDesc;
        return attributes;
    }

    // Clip the src/dst rects for a copy based on a boundingRect (usually either a dirty rect or resource boundaries)
    static void ClipCopyRectsWithBoudingRect(RECT &sourceRect, RECT &destRect, RECT &boundingRect)
    {
        const FLOAT widthScalingFactor = (FLOAT)RectWidth(destRect) / (FLOAT)RectWidth(sourceRect);
        const FLOAT heightScalingFactor = (FLOAT)RectHeight(destRect) / (FLOAT)RectHeight(sourceRect);

        // Clamp any negative values
        boundingRect.left = max(0l, boundingRect.left);
        boundingRect.top = max(0l, boundingRect.top);

        // Using ceil for all floating point operation here is what allows us to be "conformant" for the HLK Present2.exe
        if (sourceRect.left < boundingRect.left)
        {
            const FLOAT clippedAmount = (FLOAT)(boundingRect.left - sourceRect.left);
            sourceRect.left = boundingRect.left;
            destRect.left += (LONG)ceil(clippedAmount * widthScalingFactor);
        }

        if (sourceRect.right > boundingRect.right)
        {
            const FLOAT clippedAmount = (FLOAT)(sourceRect.right - boundingRect.right);
            sourceRect.right = boundingRect.right;
            destRect.right -= (LONG)ceil(clippedAmount * widthScalingFactor);
        }

        if (sourceRect.top < boundingRect.top)
        {
            const FLOAT clippedAmount = (FLOAT)(boundingRect.top - sourceRect.top);
            sourceRect.top = boundingRect.top;
            destRect.top += (LONG)ceil(clippedAmount * heightScalingFactor);
        }

        if (sourceRect.bottom > boundingRect.bottom)
        {
            const FLOAT clippedAmount = (FLOAT)(sourceRect.bottom - boundingRect.bottom);
            sourceRect.bottom = boundingRect.bottom;
            destRect.bottom -= (LONG)ceil(clippedAmount * heightScalingFactor);
        }
    }

    // Clips rects being used for a copy so that they fit DX12 requirements. PairedRect is the other source/destination
    // rect corresponding to rectToClip
    static void ClipCopyRect(RECT &rectToClip, RECT &pairedRect, INT resourceWidth, INT resourceHeight)
    {
        RECT resourceBounds;
        resourceBounds.left = 0;
        resourceBounds.top = 0;
        resourceBounds.right = resourceWidth;
        resourceBounds.bottom = resourceHeight;
        ClipCopyRectsWithBoudingRect(rectToClip, pairedRect, resourceBounds);
    }

    static const D3D12TranslationLayer::EQueryType g_cNoD3D12EquivalentQuery = (D3D12TranslationLayer::EQueryType) - 1;
    static D3D12TranslationLayer::EQueryType ConvertQueryType(D3DDDIQUERYTYPE type)
    {
        switch (type)
        {
        case D3DDDIQUERYTYPE_OCCLUSION:
            return D3D12TranslationLayer::e_QUERY_OCCLUSION;
        case D3DDDIQUERYTYPE_TIMESTAMP:
            return D3D12TranslationLayer::e_QUERY_TIMESTAMP;
        case D3DDDIQUERYTYPE_TIMESTAMPDISJOINT:
            return D3D12TranslationLayer::e_QUERY_TIMESTAMPDISJOINT;
        case D3DDDIQUERYTYPE_EVENT:
            return D3D12TranslationLayer::e_QUERY_EVENT;
        case D3DDDIQUERYTYPE_TIMESTAMPFREQ:
        case D3DDDIQUERYTYPE_VCACHE:
        case D3DDDIQUERYTYPE_RESOURCEMANAGER:
        case D3DDDIQUERYTYPE_VERTEXSTATS:
        case D3DDDIQUERYTYPE_DDISTATS:
        case D3DDDIQUERYTYPE_PIPELINETIMINGS:
        case D3DDDIQUERYTYPE_INTERFACETIMINGS:
        case D3DDDIQUERYTYPE_VERTEXTIMINGS:
        case D3DDDIQUERYTYPE_PIXELTIMINGS:
        case D3DDDIQUERYTYPE_BANDWIDTHTIMINGS:
        case D3DDDIQUERYTYPE_CACHEUTILIZATION:
        case D3DDDIQUERYTYPE_COUNTER_DEVICE_DEPENDENT:
        default:
            Check9on12(false);
            break; 
        }

        return g_cNoD3D12EquivalentQuery;
    }

    static D3D12TranslationLayer::EShaderStage ConvertFrom9on12ShaderType(ShaderType shaderType)
    {
        switch (shaderType )
        {
        case D3D9on12::VERTEX_SHADER:
            return D3D12TranslationLayer::e_VS;
        case D3D9on12::GEOMETRY_SHADER:
            return D3D12TranslationLayer::e_GS;
        case D3D9on12::PIXEL_SHADER:
            return D3D12TranslationLayer::e_PS;
        case D3D9on12::NUM_SHADER_TYPES:
        default:
            Check9on12(false);
            return D3D12TranslationLayer::e_VS;
        }
    }

    static D3D12_TEXTURE_ADDRESS_MODE ConvertToD3D12TextureAddress(D3DTEXTUREADDRESS d3dTextureAddress)
    {
        switch (d3dTextureAddress)
        {
        case D3DTADDRESS_WRAP:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case D3DTADDRESS_MIRROR:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case D3DTADDRESS_CLAMP:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case D3DTADDRESS_BORDER:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case D3DTADDRESS_MIRRORONCE:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        default:
            Check9on12(false);
            return (D3D12_TEXTURE_ADDRESS_MODE)-1;
        }
    }

    static D3D12_FILTER ConvertToD3D12ComparisonFilter(D3D12_FILTER filter)
    {
        switch (filter)
        {
            case D3D12_FILTER_MIN_MAG_MIP_POINT              :
                return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            case D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR       :
                return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
            case D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT :
                return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
            case D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR       :
                return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
            case D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT       :
                return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
            case D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR: 
                return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            case D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT       :
                return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
            case D3D12_FILTER_MIN_MAG_MIP_LINEAR             :
                return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            case D3D12_FILTER_ANISOTROPIC                    :
                return D3D12_FILTER_COMPARISON_ANISOTROPIC;
            case D3D12_FILTER_MIN_MAG_ANISOTROPIC_MIP_POINT  :
                return D3D12_FILTER_COMPARISON_MIN_MAG_ANISOTROPIC_MIP_POINT;
            default:
                Check9on12(false);
                return (D3D12_FILTER)-1;
        }
    }

    static D3D12_FILTER ConvertToD3D12Filter(
        D3DTEXTUREFILTERTYPE magFilter,
        D3DTEXTUREFILTERTYPE minFilter,
        D3DTEXTUREFILTERTYPE mipFilter,
        bool supportAnisoPointMip)
    {
        if ((D3DTEXF_ANISOTROPIC == minFilter)
            || (D3DTEXF_ANISOTROPIC == magFilter))
        {
            return (mipFilter == D3DTEXF_POINT && supportAnisoPointMip) ?
                D3D12_FILTER_MIN_MAG_ANISOTROPIC_MIP_POINT :
                D3D12_FILTER_ANISOTROPIC;
        }
        switch (mipFilter)
        {
        case D3DTEXF_POINT:
            if ((D3DTEXF_POINT == minFilter)
                && (D3DTEXF_POINT == magFilter))
            {
                return D3D12_FILTER_MIN_MAG_MIP_POINT;
            }
            if ((D3DTEXF_POINT == minFilter)
                && (D3DTEXF_LINEAR == magFilter))
            {
                return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }
            if ((D3DTEXF_LINEAR == minFilter)
                && (D3DTEXF_POINT == magFilter))
            {
                return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            }
            if ((D3DTEXF_LINEAR == minFilter)
                && (D3DTEXF_LINEAR == magFilter))
            {
                return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            }
            break;
        default:
        // D3D12 doesn't support a "none", filter. Instead we fake this by disabling mips outside of mip 0 via SRVs.
        // It's important that we pick LINEAR for the mip filter instead of POINT because hardware that does LOD 
        // clamping BEFORE determining whether to minify vs magnify (QC), will have a 0.5 rounding factor to the LOD
        // when doing POINT. This is enough to change the decision on whether to minify/magnify. This causes multiple 
        // issues in the DX9 HCK tests.
        //
        // Note DX9 tests assumed clamping happened after the minify/magnify decision, with contradicts the DX11 HW spec
        // that says it's undefined when clamping happens. This means that 9on12 can't properly support tests that try to
        // verify mip:point + mismatching min/mag filters match exactly with the ref rast. These tests have been modified to
        // no longer have this requirement
        case D3DTEXF_NONE:
        case D3DTEXF_LINEAR:
            if ((D3DTEXF_POINT == minFilter)
                && (D3DTEXF_POINT == magFilter))
            {
                return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            }
            if ((D3DTEXF_POINT == minFilter)
                && (D3DTEXF_LINEAR == magFilter))
            {
                return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            }
            if ((D3DTEXF_LINEAR == minFilter)
                && (D3DTEXF_POINT == magFilter))
            {
                return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            if ((D3DTEXF_LINEAR == minFilter)
                && (D3DTEXF_LINEAR == magFilter))
            {
                return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            }
            break;
        }
        return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }


    static D3D12_FILTER ConvertToD3D12SamplerFilter(
        D3DTEXTUREFILTERTYPE magFilter,
        D3DTEXTUREFILTERTYPE minFilter,
        D3DTEXTUREFILTERTYPE mipFilter,
        bool comparisonEnabled,
        bool supportAnisoPointMip)
    {
        D3D12_FILTER filter = ConvertToD3D12Filter(magFilter, minFilter, mipFilter, supportAnisoPointMip);
        if (comparisonEnabled)
        {
            filter = ConvertToD3D12ComparisonFilter(filter);
        }
        return filter;
    }

    static D3D12_STENCIL_OP ConvertToD3D12StencilOp(D3DSTENCILOP d3dStencilOp)
    {
        switch (d3dStencilOp)
        {
        default:
            Check9on12(false);
            return (D3D12_STENCIL_OP)-1;
        case D3DSTENCILOP_KEEP:
            return D3D12_STENCIL_OP_KEEP;
        case D3DSTENCILOP_ZERO:
            return D3D12_STENCIL_OP_ZERO;
        case D3DSTENCILOP_REPLACE:
            return D3D12_STENCIL_OP_REPLACE;
        case D3DSTENCILOP_INCRSAT:
            return D3D12_STENCIL_OP_INCR_SAT;
        case D3DSTENCILOP_DECRSAT:
            return D3D12_STENCIL_OP_DECR_SAT;
        case D3DSTENCILOP_INVERT:
            return D3D12_STENCIL_OP_INVERT;
        case D3DSTENCILOP_INCR:
            return D3D12_STENCIL_OP_INCR;
        case D3DSTENCILOP_DECR:
            return D3D12_STENCIL_OP_DECR;
        }
    }

    static D3D12_COMPARISON_FUNC ConvertToD3D12ComparisonFunc(D3DCMPFUNC d3dFunc)
    {
        switch (d3dFunc)
        {
        default:
            Check9on12(false);
            return (D3D12_COMPARISON_FUNC)-1;
        case D3DCMP_NEVER:
            return D3D12_COMPARISON_FUNC_NEVER;
        case D3DCMP_LESS:
            return D3D12_COMPARISON_FUNC_LESS;
        case D3DCMP_EQUAL:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case D3DCMP_LESSEQUAL:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case D3DCMP_GREATER:
            return D3D12_COMPARISON_FUNC_GREATER;
        case D3DCMP_NOTEQUAL:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case D3DCMP_GREATEREQUAL:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case D3DCMP_ALWAYS:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        }
    }

    static D3D12_BLEND_OP ConvertToD3D12BlendOp(D3DBLENDOP d3dBlendOp)
    {
        switch (d3dBlendOp)
        {
        default:
            Check9on12(false);
            // This is still an unexpected scenario, but we've encountered at least
            // one app that hits this (World of Warplanes). To avoid failing PSO creation,
            // we're falling through to a 'reasonable' fallback.
        case D3DBLENDOP_ADD:
            return D3D12_BLEND_OP_ADD;
        case D3DBLENDOP_SUBTRACT:
            return D3D12_BLEND_OP_SUBTRACT;
        case D3DBLENDOP_REVSUBTRACT:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case D3DBLENDOP_MIN:
            return D3D12_BLEND_OP_MIN;
        case D3DBLENDOP_MAX:
            return D3D12_BLEND_OP_MAX;
        }
    }

    static D3D12_BLEND ConvertToD3D12Blend(D3DBLEND d3dBlend)
    {
        switch (d3dBlend)
        {
        default:
        case D3DBLEND_ZERO:
            return D3D12_BLEND_ZERO;
        case D3DBLEND_ONE:
            return D3D12_BLEND_ONE;
        case D3DBLEND_SRCCOLOR:
            return D3D12_BLEND_SRC_COLOR;
        case D3DBLEND_INVSRCCOLOR:
            return D3D12_BLEND_INV_SRC_COLOR;
        case D3DBLEND_SRCALPHA:
            return D3D12_BLEND_SRC_ALPHA;
        case D3DBLEND_INVSRCALPHA:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case D3DBLEND_DESTALPHA:
            return D3D12_BLEND_DEST_ALPHA;
        case D3DBLEND_INVDESTALPHA:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case D3DBLEND_DESTCOLOR:
            return D3D12_BLEND_DEST_COLOR;
        case D3DBLEND_INVDESTCOLOR:
            return D3D12_BLEND_INV_DEST_COLOR;
        case D3DBLEND_SRCALPHASAT:
            return D3D12_BLEND_SRC_ALPHA_SAT;
        case D3DBLEND_BLENDFACTOR:
            return D3D12_BLEND_BLEND_FACTOR;
        case D3DBLEND_INVBLENDFACTOR:
            return D3D12_BLEND_INV_BLEND_FACTOR;
        case D3DBLEND_SRCCOLOR2:
            return D3D12_BLEND_SRC1_COLOR;
        case D3DBLEND_INVSRCCOLOR2:
            return D3D12_BLEND_INV_SRC1_COLOR;
        }
    }

    // In DX9 "COLOR" referred to both COLOR and ALPHA, but in DX12 COLOR specifically 
    // means just the color and not the alpha. Convert anything that says "COLOR"
    // to just it's "ALPHA" variant
    static D3D12_BLEND ConvertToD3D12AlphaBlend(D3DBLEND d3dBlend)
    {
        switch (d3dBlend)
        {
        default:
        case D3DBLEND_ZERO:
            return D3D12_BLEND_ZERO;
        case D3DBLEND_ONE:
            return D3D12_BLEND_ONE;
        case D3DBLEND_SRCCOLOR:
        case D3DBLEND_SRCALPHA:
            return D3D12_BLEND_SRC_ALPHA;
        case D3DBLEND_INVSRCCOLOR:
        case D3DBLEND_INVSRCALPHA:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case D3DBLEND_DESTALPHA:
        case D3DBLEND_DESTCOLOR:
            return D3D12_BLEND_DEST_ALPHA;
        case D3DBLEND_INVDESTALPHA:
        case D3DBLEND_INVDESTCOLOR:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case D3DBLEND_SRCALPHASAT:
            return D3D12_BLEND_SRC_ALPHA_SAT;
        case D3DBLEND_BLENDFACTOR:
            return D3D12_BLEND_BLEND_FACTOR;
        case D3DBLEND_INVBLENDFACTOR:
            return D3D12_BLEND_INV_BLEND_FACTOR;
        case D3DBLEND_SRCCOLOR2:
            return D3D12_BLEND_SRC1_ALPHA;
        case D3DBLEND_INVSRCCOLOR2:
            return D3D12_BLEND_INV_SRC1_ALPHA;
        }
    }

    static bool D3D12BlendAppliesToAlpha(D3D12_BLEND d3dBlend)
    {
        switch (d3dBlend)
        {
        case D3D12_BLEND_SRC_COLOR:
        case D3D12_BLEND_INV_SRC_COLOR:
        case D3D12_BLEND_DEST_COLOR:
        case D3D12_BLEND_INV_DEST_COLOR:
        case D3D12_BLEND_SRC1_COLOR:
        case D3D12_BLEND_INV_SRC1_COLOR:
            return false;
        case D3D12_BLEND_SRC_ALPHA:
        case D3D12_BLEND_INV_SRC_ALPHA:
        case D3D12_BLEND_DEST_ALPHA:
        case D3D12_BLEND_INV_DEST_ALPHA:
        case D3D12_BLEND_SRC_ALPHA_SAT:
        case D3D12_BLEND_BLEND_FACTOR:
        case D3D12_BLEND_INV_BLEND_FACTOR:
        case D3D12_BLEND_ZERO:
        case D3D12_BLEND_ONE:
        case D3D12_BLEND_SRC1_ALPHA:
        case D3D12_BLEND_INV_SRC1_ALPHA:
            return true;
        default:
            Check9on12(FALSE);
            return false;
        }
        
    }

    static DirectX::XMFLOAT4 ARGBToUNORMFloat(D3DCOLOR color)
    {
        DirectX::XMFLOAT4 output;
        output.x = float((color >> 16) & 0xff) / 255.0f;
        output.y = float((color >> 8) & 0xff) / 255.0f;
        output.z = float(color & 0xff) / 255.0f;
        output.w = float((color >> 24) & 0xff) / 255.0f;
        return output;
    }

    static D3DFORMAT ConvertDDIFormatToAPI(D3DDDIFORMAT format)
    {
        // DDI format is equivalent to the API format
        return (D3DFORMAT)format;
    }

    static DXGI_FORMAT ConvertToSRGB(DXGI_FORMAT Format)
    {
        const DXGI_FORMAT *pFormatSet = CD3D11FormatHelper::GetFormatCastSet(Format);
        for (; *pFormatSet != DXGI_FORMAT_UNKNOWN; pFormatSet++)
        {
            if (CD3D11FormatHelper::IsSRGBFormat(*pFormatSet))
            {
                return *pFormatSet;
            }
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    static bool IsSRGBCompatible(DXGI_FORMAT Format)
    {
        return ConvertToSRGB(Format) != DXGI_FORMAT_UNKNOWN;
    }

    static DXGI_FORMAT ConvertToTypeless(DXGI_FORMAT format)
    {
        return CD3D11FormatHelper::GetParentFormat(format);
    }

    static bool IsBlockCompressedFormat(DXGI_FORMAT format)
    {
        return CD3D11FormatHelper::IsBlockCompressFormat(format);
    }

    static bool IsYUVFormat(DXGI_FORMAT format)
    {
        return CD3D11FormatHelper::YUV(format);
    }

    static bool IsDepthStencilFormat(DXGI_FORMAT format)
    {
        return CD3D11FormatHelper::GetComponentName(format, 0) == D3D11FCN_D;
    }

    static DXGI_FORMAT ConvertDepthFormatToCompanionSRVFormat(DXGI_FORMAT format)
    {
        Check9on12(IsDepthStencilFormat(format));

        D3D11_FORMAT_COMPONENT_INTERPRETATION componentInterpretation = CD3D11FormatHelper::GetFormatComponentInterpretation(format, 0);
        const DXGI_FORMAT *pFormatSet = CD3D11FormatHelper::GetFormatCastSet(format);
        for (; *pFormatSet != DXGI_FORMAT_UNKNOWN; pFormatSet++)
        {
            if (CD3D11FormatHelper::GetTypeLevel(*pFormatSet) == D3D11_FORMAT_TYPE_LEVEL::D3D11FTL_FULL_TYPE && 
                CD3D11FormatHelper::GetComponentName(*pFormatSet, 0) == D3D11FCN_R &&
                CD3D11FormatHelper::GetFormatComponentInterpretation(*pFormatSet, 0) == componentInterpretation)
            {
                return *pFormatSet;
            }
        }

        Check9on12(false); // We should always be able to find a companion SRV format
        return DXGI_FORMAT_UNKNOWN;
    }

    static bool IsTypelessFormat(DXGI_FORMAT format)
    {
        return CD3D11FormatHelper::GetTypeLevel(format) == D3D11_FORMAT_TYPE_LEVEL::D3D11FTL_PARTIAL_TYPE;
    }

    static DXGI_FORMAT ConvertToDisplayCompatibleFormat(DXGI_FORMAT format)
    {
        if (IsTypelessFormat(format))
        {
            const DXGI_FORMAT *pFormatSet = CD3D11FormatHelper::GetFormatCastSet(format);
            for (; *pFormatSet != DXGI_FORMAT_UNKNOWN; pFormatSet++)
            {
                if (CD3D11FormatHelper::GetTypeLevel(*pFormatSet) == D3D11_FORMAT_TYPE_LEVEL::D3D11FTL_FULL_TYPE && !CD3D11FormatHelper::IsSRGBFormat(*pFormatSet))
                {
                    return *pFormatSet;
                }
            }
        }
        return format;
    }

    static bool FormatSupportsStencil(DXGI_FORMAT format)
    {
        return CD3D11FormatHelper::GetComponentName(format, 1) == D3D11FCN_S;
    }

    static bool IsCompatibleAsSourceForBltPresent(const D3D12_RESOURCE_DESC &desc)
    {
        return !IsTypelessFormat(desc.Format) && desc.SampleDesc.Count == 1;
    }

    static bool IsIHVFormat(D3DFORMAT d3dFormat)
    {
        switch (d3dFormat)
        {
        case D3DFMT_ATI2:
        case D3DFMT_ATI1:
        case D3DFMT_INTZ:
        case D3DFMT_RAWZ:
        case D3DFMT_DF16:
        case D3DFMT_DF24:
        case D3DFMT_NULL:
            return true;
        default:
            return false;
        }
    }

    static D3DFORMAT GetFormatWithSwappedRBChannels(D3DFORMAT format)
    {
        D3DFORMAT formatWithSwappedRBChannels = D3DFMT_UNKNOWN;
        switch (format)
        {
        case D3DFMT_A2R10G10B10:
            formatWithSwappedRBChannels = D3DFMT_A2B10G10R10;
        }

        return formatWithSwappedRBChannels;
    }

    static bool FormatRequiresSwappedRBChannels(D3DFORMAT format)
    {
        return GetFormatWithSwappedRBChannels(format) != D3DFMT_UNKNOWN;
    }

    static bool IsIHVFormat(D3DDDIFORMAT d3dFormat)
    {
        return IsIHVFormat(ConvertDDIFormatToAPI(d3dFormat));
    }

    static DXGI_FORMAT ConvertToDXGIFORMAT(D3DFORMAT d3dFormat)
    {
        switch ((DWORD)d3dFormat)
        {
        case D3DFMT_A8:
            return DXGI_FORMAT_A8_UNORM;
        case D3DFMT_L8:
            return DXGI_FORMAT_R8_UNORM;  
        case D3DFMT_A8L8:
            return DXGI_FORMAT_R8G8_UNORM;
        case D3DFMT_L16:
            return DXGI_FORMAT_R16_UNORM;  
        case D3DFMT_R5G6B5:
            return DXGI_FORMAT_B5G6R5_UNORM;
        case D3DFMT_A1R5G5B5:
        case D3DFMT_X1R5G5B5:
            return DXGI_FORMAT_B5G5R5A1_UNORM;
        case D3DFMT_A8R8G8B8:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case D3DFMT_X8R8G8B8:
            return DXGI_FORMAT_B8G8R8X8_UNORM;
        case D3DFMT_A8B8G8R8:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case D3DFMT_AYUV:
            return DXGI_FORMAT_AYUV;
        case D3DFMT_YUY2:
            return DXGI_FORMAT_YUY2;
        case D3DFMT_YV12:
        case D3DFMT_NV12:
            return DXGI_FORMAT_NV12;
        case  D3DFMT_420O:
            return  DXGI_FORMAT_420_OPAQUE;
        case  D3DFMT_NV11:
            return DXGI_FORMAT_NV11;
        case  D3DFMT_AI44:
            return DXGI_FORMAT_AI44;
        case  D3DFMT_IA44:
            return DXGI_FORMAT_IA44;
        case D3DFMT_Y410:
            return DXGI_FORMAT_Y410;
        case  D3DFMT_Y416:
            return DXGI_FORMAT_Y416;
        case  D3DFMT_P010:
            return DXGI_FORMAT_P010;
        case  D3DFMT_P016:
            return DXGI_FORMAT_P016;
        case  D3DFMT_Y210:
            return DXGI_FORMAT_Y210;
        case  D3DFMT_Y216:
            return DXGI_FORMAT_Y216;
        case D3DFMT_DXT1:
            return DXGI_FORMAT_BC1_UNORM;
        case D3DFMT_DXT2:
        case D3DFMT_DXT3:
            return DXGI_FORMAT_BC2_UNORM;
        case D3DFMT_DXT4:
        case D3DFMT_DXT5:
            return DXGI_FORMAT_BC3_UNORM;
        case D3DFMT_ATI1:
            return DXGI_FORMAT_BC4_UNORM;
        case D3DFMT_ATI2:
            return DXGI_FORMAT_BC5_UNORM;
        case D3DFMT_V8U8:
            return DXGI_FORMAT_R8G8_SNORM;
        case D3DFMT_D16:
        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_RAWZ:
        case D3DFMT_DF16:
            return DXGI_FORMAT_D16_UNORM;
        case D3DFMT_D32F_LOCKABLE:
            return DXGI_FORMAT_D32_FLOAT;
        case D3DFMT_S8D24:
        case D3DFMT_X8D24:
        case D3DFMT_D24S8:
        case D3DFMT_D24FS8:
        case D3DFMT_D24X8:
        case D3DFMT_DF24:
        case D3DFMT_INTZ:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case D3DFMT_A16B16G16R16:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case D3DFMT_R16F:
            return DXGI_FORMAT_R16_FLOAT;
        case D3DFMT_G16R16F:
            return DXGI_FORMAT_R16G16_FLOAT;
        case D3DFMT_A16B16G16R16F:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case D3DFMT_R32F:
            return DXGI_FORMAT_R32_FLOAT;
        case D3DFMT_G32R32F:
            return DXGI_FORMAT_R32G32_FLOAT;
        case D3DFMT_A32B32G32R32F:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case D3DFMT_A2B10G10R10:
        case D3DFMT_A2R10G10B10:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case D3DFMT_G16R16:
            return DXGI_FORMAT_R16G16_UNORM;
        case D3DFMT_V16U16:
            return DXGI_FORMAT_R16G16_SNORM;
        case D3DFMT_Q8W8V8U8:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case D3DFMT_Q16W16V16U16:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case D3DFMT_A4R4G4B4:
            return DXGI_FORMAT_B4G4R4A4_UNORM;
        case D3DFMT_P8:
            return DXGI_FORMAT_P8;
        case D3DFMT_A8P8:
            return DXGI_FORMAT_A8P8;
        case D3DFMT_X4R4G4B4:
            return DXGI_FORMAT_B4G4R4A4_UNORM;
        case D3DFMT_VERTEXDATA:
        case D3DFMT_INDEX16:
        case D3DFMT_INDEX32:
        case D3DFMT_UNKNOWN:
            return DXGI_FORMAT_UNKNOWN;
        default:
            {
                Check9on12(FALSE);
                return DXGI_FORMAT_UNKNOWN;
            }
        }
    }

    static bool IsLockable(D3DFORMAT d3dFormat)
    {
        switch (d3dFormat)
        {
        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_D32F_LOCKABLE:
            return true;
        default:
            return IsIHVFormat(d3dFormat) || !IsDepthStencilFormat(ConvertToDXGIFORMAT(d3dFormat));
        }
    }

    static DXGI_FORMAT ConvertFromDDIToDXGIFORMAT(D3DDDIFORMAT ddiFormat)
    {
        return ConvertToDXGIFORMAT((D3DFORMAT)ddiFormat);
    }

    static UINT ConvertFormatToShaderResourceViewComponentMapping(D3DFORMAT d3dFormat, bool disableAlpha = false)
    {
        // From MSDN:
        // The default value for formats that contain undefined channels (G16R16, A8, and so on) is 1. 
        // The only exception is the A8 format, which is initialized to 000 for the three color channels.

        // In DX10+ reading from undefined channels is usually either 0 or undefined so we're explicitly filling these in
        // with D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1 (with the special exception of D3DFMT_A8)

        switch (d3dFormat)
        {
        case D3DFMT_ATI1:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
        case D3DFMT_ATI2:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(1, 0, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
        case D3DFMT_A8:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0, 3);
        case D3DFMT_A8R8G8B8:
        case D3DFMT_A1R5G5B5:
        case D3DFMT_A4R4G4B4:
        case D3DFMT_A8R3G3B2:
        case D3DFMT_A2B10G10R10:
        case D3DFMT_A8B8G8R8:
        case D3DFMT_A16B16G16R16:
        case D3DFMT_A2W10V10U10:
        case D3DFMT_Q8W8V8U8:
        case D3DFMT_Q16W16V16U16:
        case D3DFMT_A16B16G16R16F:
        case D3DFMT_A32B32G32R32F:
        case D3DFMT_A2B10G10R10_XR_BIAS:
        case D3DFMT_DXT1:
        case D3DFMT_DXT2:
        case D3DFMT_DXT3:
        case D3DFMT_DXT4:
        case D3DFMT_DXT5:
        case D3DFMT_MULTI2_ARGB8:
            if (disableAlpha)
            {
                return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 1, 2, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
            }
            else
            {
                return D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            }
        case D3DFMT_A2R10G10B10:
                return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(2, 1, 0, disableAlpha ? D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1 : 3);
        case D3DFMT_R8G8B8:
        case D3DFMT_X8R8G8B8:
        case D3DFMT_R5G6B5:
        case D3DFMT_X1R5G5B5:
        case D3DFMT_R3G3B2:
        case D3DFMT_X4R4G4B4:
        case D3DFMT_X8B8G8R8:
        case D3DFMT_L6V5U5:
        case D3DFMT_X8L8V8U8:
        case D3DFMT_UYVY:
        case D3DFMT_R8G8_B8G8:
        case D3DFMT_G8R8_G8B8:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 1, 2, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
        case D3DFMT_G16R16:
        case D3DFMT_V8U8:
        case D3DFMT_V16U16:
        case D3DFMT_G16R16F:
        case D3DFMT_G32R32F:
        case D3DFMT_CxV8U8:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
        case D3DFMT_YUY2:
        case D3DFMT_NV12:
        case D3DFMT_NV11:
        case D3DFMT_AI44:
        case D3DFMT_IA44:
        case D3DFMT_Y410:
        case D3DFMT_Y416:
        case D3DFMT_P010:
        case D3DFMT_P016:
        case D3DFMT_Y210:
        case D3DFMT_Y216:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);

        case D3DFMT_R16F:
        case D3DFMT_R32F:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
        
        // From MSDN: A one - bit surface has one bit per texel; therefore, a one would mean that all components(r, g, b, a) of a pixel are 1, 
        // and zero would mean that all components are equal to 0. You may use one - bit surfaces with the following APIs : 
        // ColorFill, UpdateSurface and UpdateTexture.
        case D3DFMT_A1:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 0, 0, 0);

        // Luminance applies to all color channels
        case D3DFMT_L16:
        case D3DFMT_L8:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 0, 0, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
        case D3DFMT_A8L8:
        case D3DFMT_A4L4:
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 0, 0, 1);
        case D3DFMT_X8D24:
        case D3DFMT_D24S8:
        case D3DFMT_D24X8:
        case D3DFMT_INTZ:
        case D3DFMT_RAWZ:
        case D3DFMT_DF16:
        case D3DFMT_DF24:
        case D3DFMT_D16:
        case D3DFMT_D32:
            // It is important that the y,z, and w values are forced to 1, games depend on this behavior (most notably is Diablo III)
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
        //TODO: These formats when they get hit
        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_D15S1:
        case D3DFMT_D24X4S4:
        case D3DFMT_D32F_LOCKABLE:
        case D3DFMT_D24FS8:
        case D3DFMT_D32_LOCKABLE:
        case  D3DFMT_S8_LOCKABLE:
        // Not expecting VB/IBbuffer formats either
        case D3DFMT_VERTEXDATA:
        case D3DFMT_INDEX16:
        case D3DFMT_INDEX32:
        case D3DFMT_BINARYBUFFER:
        case D3DFMT_UNKNOWN:
        // Need to figure out expectations for palletized formats
        case D3DFMT_P8:
        case D3DFMT_A8P8:
        default:
            Check9on12(false);
            return (UINT)-1;

        }
    }

    static UINT8 SwizzleRenderTargetMask(UINT8 mask, UINT rtvShaderComponentMapping)
    {
        if (rtvShaderComponentMapping == D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING)
        {
            return mask;
        }

        UINT8 swizzledMask = 0;
        for (UINT srcChannelIndex = 0; srcChannelIndex < 4; srcChannelIndex++)
        {
            UINT dstChannelIndex = D3D12_SHADER_COMPONENT_MAPPING_MASK & (rtvShaderComponentMapping >> (D3D12_SHADER_COMPONENT_MAPPING_SHIFT * srcChannelIndex));
            
            // Can be greater than D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3 if using FORCE_VALUE.
            // In these cases the value is never read so we can safely ignore them
            if (dstChannelIndex <= D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3)
            {
                UINT bit = (mask & BIT(srcChannelIndex)) != 0;
                swizzledMask |= bit << dstChannelIndex;
            }
        }
        return swizzledMask;
    }

    // Completely arbitrary picked default formats, should investigate if these are optimal
    static D3DFORMAT GetDefaultFormat(D3DDDI_RESOURCEFLAGS flag)
    {
        if (flag.IndexBuffer || flag.VertexBuffer)
        {
            return D3DFMT_R32F;
        }
        else
        {
            return D3DFMT_UNKNOWN;
        }
    }

    static D3D12_HEAP_TYPE ConvertToD3D12HeapType(D3DDDI_POOL PoolType)
    {
        switch (PoolType)
        {
        case D3DDDIPOOL_SYSTEMMEM:
            return D3D12_HEAP_TYPE_UPLOAD;
        case D3DDDIPOOL_VIDEOMEMORY:
        case D3DDDIPOOL_LOCALVIDMEM:
        case D3DDDIPOOL_NONLOCALVIDMEM:
            return D3D12_HEAP_TYPE_DEFAULT;
        case D3DDDIPOOL_STAGINGMEM:
            // Only used for 10level9
            Check9on12(false);
        }
        return (D3D12_HEAP_TYPE)-1;
    }

    static bool IsBuffer(D3DDDI_RESOURCEFLAGS ResourceFlags)
    {
        return ResourceFlags.IndexBuffer || ResourceFlags.VertexBuffer  || ResourceFlags.DecodeCompressedBuffer;
    }

    static D3D12_RESOURCE_DIMENSION ConvertToD3D12Dimension(D3DDDI_RESOURCEFLAGS ResourceFlags, UINT Width, UINT Height, UINT Depth)
    {
        D3D12_RESOURCE_DIMENSION Dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
        if (IsBuffer(ResourceFlags))
        {
            Check9on12(Depth <= 1 && Height <= 1);
            Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        }
        else
        {
            if (Depth > 1)
            {
                Check9on12(Height >= 1 && Width >= 1);
                Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            }
            else
            {
                Check9on12(Depth <= 1);
                Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            }
        }
        return Dimension;
    }
    
    // This excludes CBVs, which are can be created on any memory
    static bool PoolAllowsViews(D3DDDI_POOL poolType)
    {
        return poolType != D3DDDIPOOL_SYSTEMMEM && poolType != D3DDDIPOOL_STAGINGMEM;
    }

    static bool NeedsDepthStencil(D3DDDI_RESOURCEFLAGS ResourceFlags, D3DDDI_POOL poolType)
    {
        return PoolAllowsViews(poolType) && ResourceFlags.ZBuffer;
    }

    static bool IsDecoderSurface(D3DDDI_RESOURCEFLAGS ResourceFlags)
    {
        return ResourceFlags.DecodeRenderTarget  ||  ResourceFlags.DecodeCompressedBuffer;
    }

    static bool NeedsRenderTarget(D3DDDI_RESOURCEFLAGS ResourceFlags, D3DDDI_POOL poolType)
    {
        return PoolAllowsViews(poolType) &&
            (ResourceFlags.RenderTarget ||
            ResourceFlags.AutogenMipmap); // We'll need to internally create an RTV to the resource if we want to generate mipmaps
    }

    static bool NeedsShaderResourceView(D3DDDI_RESOURCEFLAGS ResourceFlags, D3DDDI_POOL poolType)
    {
        return PoolAllowsViews(poolType) && !ResourceFlags.DecodeRenderTarget && !ResourceFlags.VideoProcessRenderTarget && (ResourceFlags.Texture || ResourceFlags.CubeMap || ResourceFlags.Volume);
    }

    static bool NeedsVideoDecoderOutputView(D3DDDI_RESOURCEFLAGS ResourceFlags, D3DDDI_POOL poolType)
    {
        return PoolAllowsViews(poolType) && (ResourceFlags.DecodeRenderTarget);
    }

    static bool NeedsVideoProcessorOutputView(D3DDDI_RESOURCEFLAGS ResourceFlags, D3DDDI_POOL poolType)
    {
        return PoolAllowsViews(poolType) && (ResourceFlags.VideoProcessRenderTarget);
    }

    static D3D12_HEAP_FLAGS ConvertToD3D12HeapFlag(D3DDDI_RESOURCEFLAGS ResourceFlags, D3DDDI_POOL poolType)
    {
        D3D12_HEAP_FLAGS Flag = D3D12_HEAP_FLAG_NONE;
        if (IsBuffer(ResourceFlags))
        {
            Flag = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        }
        else
        {
            
            if (NeedsRenderTarget(ResourceFlags, poolType) || NeedsDepthStencil(ResourceFlags, poolType))
            {
                Flag = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
            }
            else
            {
                Flag = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
            }
        }
        return Flag;
    }

    static D3D12_RESOURCE_FLAGS ConvertToD3D12ResourceFlags(D3DDDI_RESOURCEFLAGS dx9Flag, D3DDDI_RESOURCEFLAGS2 dx9Flag2, D3DDDI_POOL poolType)
    {
        D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_NONE;
        if (NeedsRenderTarget(dx9Flag, poolType))
        {
            flag |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        if (NeedsDepthStencil(dx9Flag, poolType))
        {
            flag |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            if (!dx9Flag.Texture)
            {
                flag |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

            }
        }

        if (dx9Flag2.CrossAdapter)
        {
            flag |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
        }
        return flag;
    }

    static D3D12_TEXTURE_LAYOUT ConvertToD3D12TextureLayout(D3D12_RESOURCE_DIMENSION dimension, D3DDDI_RESOURCEFLAGS2 dx9Flag2)
    {
        if (dimension == D3D12_RESOURCE_DIMENSION_BUFFER || (dx9Flag2.CrossAdapter && dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D))
        {
            return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        }
        else
        {
            return D3D12_TEXTURE_LAYOUT_UNKNOWN;
        }
    }

    static D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertToD3D12TopologyType(D3D12_PRIMITIVE_TOPOLOGY primitiveType)
    {
        switch (primitiveType)
        {
        case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
        case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:
            Check9on12(false); // DX9 shouldn't trigger any other topologies
            return (D3D12_PRIMITIVE_TOPOLOGY_TYPE)-1;
        }
    }

    static bool IsTriangleFan(D3DPRIMITIVETYPE primitiveType)
    {
        return primitiveType == D3DPT_TRIANGLEFAN;
    }

    static D3D_PRIMITIVE_TOPOLOGY ConvertToD3D12Topology(D3DPRIMITIVETYPE primitiveType)
    {
        switch (primitiveType)
        {
        case D3DPT_POINTLIST:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case D3DPT_LINELIST:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case D3DPT_LINESTRIP:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case D3DPT_TRIANGLELIST:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case D3DPT_TRIANGLESTRIP:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

        // We convert triangle fans into triangle fans
        case D3DPT_TRIANGLEFAN:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        //case D3D_PT_ODD_TRIANGLE_STRIP:
        //case D3D_PT_TRIANGLE_FAN2:
        default:
            Check9on12(false); // Topologies not supported yet
            return (D3D_PRIMITIVE_TOPOLOGY)-1;
        }
    }

    static bool IsIntType(D3DDECLTYPE d3dDeclType)
    {
        switch (d3dDeclType)
        {
            case D3DDECLTYPE_FLOAT1       :
            case D3DDECLTYPE_FLOAT2       :
            case D3DDECLTYPE_FLOAT3       :
            case D3DDECLTYPE_FLOAT4       :
            case D3DDECLTYPE_D3DCOLOR     :
            case D3DDECLTYPE_FLOAT16_2:
            case D3DDECLTYPE_FLOAT16_4:
            //We are making the asumption that normalized values are floats
            case D3DDECLTYPE_UBYTE4N:
            case D3DDECLTYPE_SHORT2N:
            case D3DDECLTYPE_SHORT4N:
            case D3DDECLTYPE_USHORT2N:
            case D3DDECLTYPE_USHORT4N:
            case D3DDECLTYPE_DEC3N:
                return false;
            case D3DDECLTYPE_UDEC3:
            case D3DDECLTYPE_UBYTE4       :
            case D3DDECLTYPE_SHORT2       :
            case D3DDECLTYPE_SHORT4       :
                return true;
            default:
            case D3DDECLTYPE_UNUSED:
                Check9on12(false); 
                return false;
        }
    }

    static DXGI_FORMAT ConvertToDXGIFormat(D3DDECLTYPE d3dDeclType)
    {
        static const DXGI_FORMAT s_declTypeToDXGIFORMAT[] = {
            DXGI_FORMAT_R32_FLOAT,          // D3DDECLTYPE_FLOAT1  
            DXGI_FORMAT_R32G32_FLOAT,       // D3DDECLTYPE_FLOAT2  
            DXGI_FORMAT_R32G32B32_FLOAT,    // D3DDECLTYPE_FLOAT3  
            DXGI_FORMAT_R32G32B32A32_FLOAT, // D3DDECLTYPE_FLOAT4  
            DXGI_FORMAT_B8G8R8A8_UNORM,     // D3DDECLTYPE_D3DCOLOR  
            DXGI_FORMAT_R8G8B8A8_UINT,      // D3DDECLTYPE_UBYTE4  
            DXGI_FORMAT_R16G16_SINT,        // D3DDECLTYPE_SHORT2  
            DXGI_FORMAT_R16G16B16A16_SINT,  // D3DDECLTYPE_SHORT4  
            DXGI_FORMAT_R8G8B8A8_UNORM,     // D3DDECLTYPE_UBYTE4N  
            DXGI_FORMAT_R16G16_SNORM,       // D3DDECLTYPE_SHORT2N  
            DXGI_FORMAT_R16G16B16A16_SNORM, // D3DDECLTYPE_SHORT4N  
            DXGI_FORMAT_R16G16_UNORM,       // D3DDECLTYPE_USHORT2N  
            DXGI_FORMAT_R16G16B16A16_UNORM, // D3DDECLTYPE_USHORT4N  
            DXGI_FORMAT_R10G10B10A2_UINT,   // D3DDECLTYPE_UDEC3  
            DXGI_FORMAT_R10G10B10A2_UINT,   // D3DDECLTYPE_DEC3N  
            DXGI_FORMAT_R16G16_FLOAT,       // D3DDECLTYPE_FLOAT16_2  
            DXGI_FORMAT_R16G16B16A16_FLOAT, // D3DDECLTYPE_FLOAT16_4  
            DXGI_FORMAT_UNKNOWN,            // D3DDECLTYPE_UNUSED  
        };
        return s_declTypeToDXGIFORMAT[d3dDeclType];
    }

    static D3DFORMAT ConvertToD3DFORMAT(DXGI_FORMAT dxgiFormat)
    {
        switch ((DWORD)dxgiFormat)
        {
        case DXGI_FORMAT_UNKNOWN:
            return D3DFMT_UNKNOWN;
        case DXGI_FORMAT_A8_UNORM:
            return D3DFMT_A8;
        case DXGI_FORMAT_R8_UNORM:
            return D3DFMT_L8;
        case DXGI_FORMAT_R16_UNORM:
            return D3DFMT_L16;
        case DXGI_FORMAT_B5G6R5_UNORM:
            return D3DFMT_R5G6B5;
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            return D3DFMT_A1R5G5B5;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            return D3DFMT_A8R8G8B8;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            return D3DFMT_X8R8G8B8;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return D3DFMT_A8B8G8R8;
        case DXGI_FORMAT_AYUV:
            return D3DFMT_AYUV;
        case DXGI_FORMAT_YUY2:
            return D3DFMT_YUY2;
        case DXGI_FORMAT_NV12:
            return D3DFORMAT(MAKEFOURCC('N', 'V', '1', '2'));
        case  DXGI_FORMAT_420_OPAQUE:
            return D3DFORMAT(MAKEFOURCC('4', '2', '0', 'O'));
        case DXGI_FORMAT_NV11:
            return D3DFORMAT(MAKEFOURCC('N', 'V', '1', '1'));
        case DXGI_FORMAT_AI44:
            return D3DFORMAT(MAKEFOURCC('A', 'I', '4', '4'));
        case DXGI_FORMAT_IA44:
            return D3DFORMAT(MAKEFOURCC('I', 'A', '4', '4'));
        case DXGI_FORMAT_Y410:
            return  D3DFORMAT(MAKEFOURCC('Y', '4', '1', '0'));
        case DXGI_FORMAT_Y416:
            return D3DFORMAT(MAKEFOURCC('Y', '4', '1', '6'));
        case DXGI_FORMAT_P010:
            return D3DFORMAT(MAKEFOURCC('P', '0', '1', '0'));
        case DXGI_FORMAT_P016:
            return D3DFORMAT(MAKEFOURCC('P', '0', '1', '6'));
        case DXGI_FORMAT_Y210:
            return D3DFORMAT(MAKEFOURCC('Y', '2', '1', '0'));
        case DXGI_FORMAT_Y216:
            return D3DFORMAT(MAKEFOURCC('Y', '2', '1', '6'));
        case DXGI_FORMAT_BC1_UNORM:
            return D3DFMT_DXT1;
        case DXGI_FORMAT_BC2_UNORM:
            return D3DFMT_DXT2;
        case DXGI_FORMAT_BC3_UNORM:
            return D3DFMT_DXT4;
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_TYPELESS:
            return D3DFMT_V8U8;
        case DXGI_FORMAT_D16_UNORM:
            return D3DFMT_D16;
        case DXGI_FORMAT_D32_FLOAT:
            return D3DFMT_D32F_LOCKABLE;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return D3DFMT_D24S8;
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            return D3DFMT_A16B16G16R16;
        case DXGI_FORMAT_R16_FLOAT:
            return D3DFMT_R16F;
        case DXGI_FORMAT_R16G16_FLOAT:
            return D3DFMT_G16R16F;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return D3DFMT_A16B16G16R16F;
        case DXGI_FORMAT_R32_FLOAT:
            return D3DFMT_R32F;
        case DXGI_FORMAT_R32G32_FLOAT:
            return D3DFMT_G32R32F;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return D3DFMT_A32B32G32R32F;
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            return D3DFMT_A2B10G10R10;
        case DXGI_FORMAT_R16G16_UNORM:
            return D3DFMT_G16R16;
        case DXGI_FORMAT_R16G16_SNORM:
            return D3DFMT_V16U16;
        case DXGI_FORMAT_R8G8B8A8_SNORM:
            return D3DFMT_Q8W8V8U8;
        case DXGI_FORMAT_R16G16B16A16_SNORM:
            return D3DFMT_Q16W16V16U16;
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return D3DFMT_A4R4G4B4;
        case DXGI_FORMAT_R8G8_UNORM:
            return D3DFMT_A8L8;
        // Can't convert DXGI_FORMAT_R16G16_TYPELESS because it could either bet D3DFMT_G16R16 or D3DFMT_V16U16
        case DXGI_FORMAT_R16G16_TYPELESS:
        default:
            // Do nothing        
            Check9on12(false);
            return D3DFMT_UNKNOWN;
        }

    }

    static D3DDDIFORMAT ConvertD3DFormatToDDIFormat(D3DFORMAT format)
    {
        // These enums map 1-1 so this is a safe cast
        return (D3DDDIFORMAT)format;
    }

    static D3DDDIFORMAT ConvertFromDXGIFormatToD3DDDIFormat(DXGI_FORMAT dxgiFormat)
    {
        return ConvertD3DFormatToDDIFormat(ConvertToD3DFORMAT(dxgiFormat));
    }

    static D3D12TranslationLayer::AppResourceDesc ConvertToAppResourceDesc(const D3D12_RESOURCE_DESC &desc12, D3D12TranslationLayer::RESOURCE_USAGE Usage, DWORD Access, DWORD BindFlags)
    {
        UINT16 depth = desc12.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D ? 1 : desc12.DepthOrArraySize;
        UINT16 arraySize = desc12.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? 1 : desc12.DepthOrArraySize;
        UINT8 nonOpaquePlaneCount = (UINT8)CD3D11FormatHelper::NonOpaquePlaneCount(desc12.Format);
        UINT numSubresources = desc12.MipLevels * arraySize * nonOpaquePlaneCount;

        return D3D12TranslationLayer::AppResourceDesc(
            desc12.MipLevels * arraySize,
            nonOpaquePlaneCount,
            numSubresources,
            (UINT8)desc12.MipLevels,
            arraySize,
            depth,
            (UINT)desc12.Width,
            (UINT)desc12.Height,
            desc12.Format,
            desc12.SampleDesc.Count,
            desc12.SampleDesc.Quality,
            Usage,
            (D3D12TranslationLayer::RESOURCE_CPU_ACCESS)Access,
            (D3D12TranslationLayer::RESOURCE_BIND_FLAGS)BindFlags,
            desc12.Dimension);
    }

    static UINT ConvertTranslationLayerBindFlagsToD3D11BindFlags(UINT flags)
    {
        UINT d3d11Flags = 0;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_VERTEX_BUFFER    ) d3d11Flags |= D3D11_BIND_VERTEX_BUFFER;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_INDEX_BUFFER     ) d3d11Flags |= D3D11_BIND_INDEX_BUFFER;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_CONSTANT_BUFFER  ) d3d11Flags |= D3D11_BIND_CONSTANT_BUFFER;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_SHADER_RESOURCE  ) d3d11Flags |= D3D11_BIND_SHADER_RESOURCE;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_STREAM_OUTPUT    ) d3d11Flags |= D3D11_BIND_STREAM_OUTPUT;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_RENDER_TARGET    ) d3d11Flags |= D3D11_BIND_RENDER_TARGET;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_DEPTH_STENCIL    ) d3d11Flags |= D3D11_BIND_DEPTH_STENCIL;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_UNORDERED_ACCESS ) d3d11Flags |= D3D11_BIND_UNORDERED_ACCESS;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_DECODER          ) d3d11Flags |= D3D11_BIND_DECODER;
        if (flags & D3D12TranslationLayer::RESOURCE_BIND_VIDEO_ENCODER    ) d3d11Flags |= D3D11_BIND_VIDEO_ENCODER;

        return d3d11Flags;
    }

    static D3D12TranslationLayer::ResourceCreationArgs GetCreateArgsFromExistingResource(ID3D12Device *pDevice, const D3D12_RESOURCE_DESC &desc12)
    {
        D3D12TranslationLayer::ResourceCreationArgs CreateArgs{};
        CreateArgs.m_desc12 = desc12;
        UINT bindFlags = 0;

        // TODO:  #11976715 Workaround for a bug where the surface we get back from DWM has a invalid desc (alignment must be
        // D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT for resources with ALLOW_RENDER_TARGET/DEPTH_STENCIL, 4096 instead). 
        // Working around this by putting a "correct" alignment
        if ((desc12.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) && desc12.Alignment == 4096)
        {
            CreateArgs.m_desc12.Alignment = 0;
        }

        if (CreateArgs.m_desc12.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
        {
            bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_RENDER_TARGET;
        }

        if (CreateArgs.m_desc12.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
        {
            bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_DEPTH_STENCIL;
        }

        if ((CreateArgs.m_desc12.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0)
        {
            bindFlags |= D3D12TranslationLayer::RESOURCE_BIND_SHADER_RESOURCE;
        }

        CreateArgs.m_appDesc = ConvertToAppResourceDesc(CreateArgs.m_desc12,
            D3D12TranslationLayer::RESOURCE_USAGE_DEFAULT,
            (D3D12TranslationLayer::RESOURCE_CPU_ACCESS)0,
            (D3D12TranslationLayer::RESOURCE_BIND_FLAGS)bindFlags);
        CreateArgs.m_heapDesc = CD3DX12_HEAP_DESC(pDevice->GetResourceAllocationInfo(0, 1, &CreateArgs.m_desc12), D3D12_HEAP_TYPE_DEFAULT);
        CreateArgs.m_bBoundForStreamOut = false;
        CreateArgs.m_isPlacedTexture = 0;
        CreateArgs.m_OffsetToStreamOutputSuffix = 0;

        return CreateArgs;
    }

    static D3D12TranslationLayer::ResourceCreationArgs GetCreateBufferArgs(ID3D12Device *pDevice, size_t SizeInBytes, size_t AlignmentRequired, D3D12_HEAP_TYPE heapType)
    {
        D3D12TranslationLayer::RESOURCE_CPU_ACCESS cpuAccess;
        D3D12TranslationLayer::RESOURCE_USAGE usage;
        if (heapType == D3D12_HEAP_TYPE_READBACK)
        {
            cpuAccess = D3D12TranslationLayer::RESOURCE_CPU_ACCESS_READ;
            usage = D3D12TranslationLayer::RESOURCE_USAGE_STAGING;
        }
        else
        {
            cpuAccess = D3D12TranslationLayer::RESOURCE_CPU_ACCESS_WRITE;
            usage = D3D12TranslationLayer::RESOURCE_USAGE_DYNAMIC;
        }

        D3D12TranslationLayer::ResourceCreationArgs args{};
        AlignmentRequired = Align(AlignmentRequired, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
        args.m_desc12 = CD3DX12_RESOURCE_DESC::Buffer(SizeInBytes, D3D12_RESOURCE_FLAG_NONE, AlignmentRequired);
        UINT BindFlags = D3D12TranslationLayer::RESOURCE_BIND_INDEX_BUFFER | D3D12TranslationLayer::RESOURCE_BIND_CONSTANT_BUFFER | D3D12TranslationLayer::RESOURCE_BIND_VERTEX_BUFFER;
        args.m_appDesc = ConvertToAppResourceDesc(args.m_desc12, usage, cpuAccess, (D3D12TranslationLayer::RESOURCE_BIND_FLAGS)BindFlags);
        args.m_heapDesc = CD3DX12_HEAP_DESC(pDevice->GetResourceAllocationInfo(0, 1, &args.m_desc12), CD3DX12_HEAP_PROPERTIES(heapType));

        return args;
    }

    static D3D12TranslationLayer::ResourceCreationArgs GetCreateReadbackBufferArgs(ID3D12Device *pDevice, size_t SizeInBytes, size_t AlignmentRequired)
    {
        return GetCreateBufferArgs(pDevice, SizeInBytes, AlignmentRequired, D3D12_HEAP_TYPE_READBACK);
    }

    static D3D12TranslationLayer::ResourceCreationArgs GetCreateUploadBufferArgs(ID3D12Device *pDevice, size_t SizeInBytes, size_t AlignmentRequired)
    {
        return GetCreateBufferArgs(pDevice, SizeInBytes, AlignmentRequired, D3D12_HEAP_TYPE_UPLOAD);
    }

    static D3D10_REGISTER_COMPONENT_TYPE ConvertToRegisterComponentType(D3DDECLTYPE d3dDeclType)
    {
        switch (d3dDeclType)
        {
             case D3DDECLTYPE_FLOAT1      :
             case D3DDECLTYPE_FLOAT2      :
             case D3DDECLTYPE_FLOAT3      :
             case D3DDECLTYPE_FLOAT4      :
             case D3DDECLTYPE_FLOAT16_2:
             case D3DDECLTYPE_FLOAT16_4:
             case D3DDECLTYPE_D3DCOLOR:
             case D3DDECLTYPE_UBYTE4N:
             case D3DDECLTYPE_USHORT2N:
             case D3DDECLTYPE_USHORT4N:
             case D3DDECLTYPE_SHORT2N:
             case D3DDECLTYPE_SHORT4N:
             case D3DDECLTYPE_DEC3N:
                 return D3D10_REGISTER_COMPONENT_FLOAT32;
             case D3DDECLTYPE_UBYTE4      :
             case D3DDECLTYPE_UDEC3       :
                 return D3D10_REGISTER_COMPONENT_UINT32;
             case D3DDECLTYPE_SHORT2:
             case D3DDECLTYPE_SHORT4:
                 return D3D10_REGISTER_COMPONENT_SINT32;
             case D3DDECLTYPE_UNUSED:
            default:
                Check9on12(false); // TODO: Unsure what these fall under
                return D3D10_REGISTER_COMPONENT_UNKNOWN;
        }
    }

    static D3D12TranslationLayer::RESOURCE_USAGE GetResourceUsage(D3DDDI_RESOURCEFLAGS flags, UINT cpuAccessFlags)
    {
        if (flags.DecodeCompressedBuffer)
        {
            return D3D12TranslationLayer::RESOURCE_USAGE_DYNAMIC;
        }
        else if(cpuAccessFlags & (D3D12TranslationLayer::RESOURCE_CPU_ACCESS_READ | D3D12TranslationLayer::RESOURCE_CPU_ACCESS_WRITE))
        {
            return  D3D12TranslationLayer::RESOURCE_USAGE_DYNAMIC;
        }
        else
        {
            return D3D12TranslationLayer::RESOURCE_USAGE_DEFAULT;
        }
    }

    // Returns a mask that represents which color channels that shader will use
    // (i.e. Using just color channels R and G would return the bitmask 0011).
    static BYTE ConvertToMask(D3DDECLTYPE d3dDeclType)
    {
        switch (d3dDeclType)
        {
        case D3DDECLTYPE_FLOAT1:
            return R_MASK;
        case D3DDECLTYPE_FLOAT2:
        case D3DDECLTYPE_FLOAT16_2:
        case D3DDECLTYPE_USHORT2N:
        case D3DDECLTYPE_SHORT2N:
        case D3DDECLTYPE_SHORT2:
            return RG_MASK;
        case D3DDECLTYPE_UDEC3:
        case D3DDECLTYPE_DEC3N:
        case D3DDECLTYPE_FLOAT3:
            return RGB_MASK;
        case D3DDECLTYPE_SHORT4N:
        case D3DDECLTYPE_SHORT4:
        case D3DDECLTYPE_FLOAT4:
        case D3DDECLTYPE_UBYTE4:
        case D3DDECLTYPE_FLOAT16_4:
        case D3DDECLTYPE_UBYTE4N:
        case D3DDECLTYPE_USHORT4N:
        case D3DDECLTYPE_D3DCOLOR:
            return RGBA_MASK;
        case D3DDECLTYPE_UNUSED:
        default:
            Check9on12(false); // TODO: Unsure what these fall under
            return 0;
        }
    }

    static BYTE ConvertToRWMask(D3DDECLTYPE d3dDeclType)
    {
        switch (d3dDeclType)
        {
        case D3DDECLTYPE_FLOAT1:
            return R_MASK;
        case D3DDECLTYPE_FLOAT2:
        case D3DDECLTYPE_FLOAT16_2:
        case D3DDECLTYPE_USHORT2N:
        case D3DDECLTYPE_SHORT2N:
        case D3DDECLTYPE_SHORT2:
            return RG_MASK;
        case D3DDECLTYPE_UDEC3:
        case D3DDECLTYPE_DEC3N:
        case D3DDECLTYPE_FLOAT3:
            return RGB_MASK;
        case D3DDECLTYPE_SHORT4N:
        case D3DDECLTYPE_SHORT4:
        case D3DDECLTYPE_FLOAT4:
        case D3DDECLTYPE_UBYTE4:
        case D3DDECLTYPE_FLOAT16_4:
        case D3DDECLTYPE_UBYTE4N:
        case D3DDECLTYPE_USHORT4N:
        case D3DDECLTYPE_D3DCOLOR:
            return RGBA_MASK;
        case D3DDECLTYPE_UNUSED:
        default:
            Check9on12(false); // TODO: Unsure what these fall under
            return 0;
        }
    }

    static D3D_REGISTER_COMPONENT_TYPE ConvertToComponentType(D3DDECLUSAGE usage)
    {
        //  D3D9 can only work in floats, VFACE (a.k.a. IsFrontFace) is the only exception
        if (usage == D3DDECLUSAGE_VFACE)
        {
            return D3D_REGISTER_COMPONENT_UINT32;
        }
        return D3D_REGISTER_COMPONENT_FLOAT32;
    }

    static D3D10_NAME ConvertToName(D3DDECLUSAGE usage)
    {
        switch (usage)
        {
        case D3DDECLUSAGE_VPOS:
        case D3DDECLUSAGE_POSITION:
            return D3D10_NAME_POSITION;
        case D3DDECLUSAGE_CLIPDISTANCE:
            return D3D10_NAME_CLIP_DISTANCE;
        case D3DDECLUSAGE_VFACE:
            return D3D_NAME_IS_FRONT_FACE;
        case D3DDECLUSAGE_DEPTH:
        case D3DDECLUSAGE_TEXCOORD:
        case D3DDECLUSAGE_COLOR:
        case D3DDECLUSAGE_NORMAL:
        case D3DDECLUSAGE_TANGENT:
        case D3DDECLUSAGE_BLENDWEIGHT:
        case D3DDECLUSAGE_BLENDINDICES:
        case D3DDECLUSAGE_FOG:
        case D3DDECLUSAGE_SAMPLE:
        case D3DDECLUSAGE_PSIZE:
        case D3DDECLUSAGE_BINORMAL:
        case D3DDECLUSAGE_TESSFACTOR:
        case D3DDECLUSAGE_POINTSPRITE:
            return D3D10_NAME_UNDEFINED;
        case D3DDECLUSAGE_POSITIONT:
        default:
            // Verify if this should be getting past the FFconversion layer
            Check9on12(false);
            return D3D10_NAME_UNDEFINED;
        }
    }

    // Returns the semantic name without any number appended at the end
    static const char *ConvertToSemanticNameForInputLayout(D3DDECLUSAGE usage)
    {
        switch (usage)
        {
        case D3DDECLUSAGE_POSITION:
            return "POSITION";
        case D3DDECLUSAGE_TEXCOORD:
            return "TEXCOORD";
        case D3DDECLUSAGE_COLOR:
            return "COLOR";
        case D3DDECLUSAGE_NORMAL:
            return "NORMAL";
        case D3DDECLUSAGE_TANGENT:
            return "TANGENT";
        case D3DDECLUSAGE_BLENDWEIGHT:
            return "BLENDWEIGHT";
        case D3DDECLUSAGE_BLENDINDICES:
            return "BLENDINDICES";
        case D3DDECLUSAGE_PSIZE:
            return "PSIZE";
        case D3DDECLUSAGE_BINORMAL:
            return "BINORMAL";
        case D3DDECLUSAGE_TESSFACTOR:
            return "TESSFACTOR";
        case D3DDECLUSAGE_FOG:
            return "FOG";
        case D3DDECLUSAGE_DEPTH:
            return "DEPTH";
        case D3DDECLUSAGE_SAMPLE:
            return "SAMPLE";
        case D3DDECLUSAGE_POSITIONT:
        default:
            Check9on12(false);
            return "";
        }
    }

    static const char *ConvertToSemanticNameForDXBCLinkage(D3DDECLUSAGE usage)
    {
        switch (usage)
        {
        case D3DDECLUSAGE_VPOS:
        case D3DDECLUSAGE_POSITION:
            return "SV_POSITION";
        case D3DDECLUSAGE_TEXCOORD:
            return "TEXCOORD";
        case D3DDECLUSAGE_COLOR:
            return "COLOR";
        case D3DDECLUSAGE_NORMAL:
            return "NORMAL";
        case D3DDECLUSAGE_TANGENT:
            return "TANGENT";
        case D3DDECLUSAGE_BLENDWEIGHT:
            return "BLENDWEIGHT";
        case D3DDECLUSAGE_BLENDINDICES:
            return "BLENDINDICES";
        case D3DDECLUSAGE_PSIZE:
            return "PSIZE";
        case D3DDECLUSAGE_BINORMAL:
            return "BINORMAL";
        case D3DDECLUSAGE_TESSFACTOR:
            return "TESSFACTOR";
        case D3DDECLUSAGE_FOG:
            return "FOG";
        case D3DDECLUSAGE_DEPTH:
            return "DEPTH";
        case D3DDECLUSAGE_SAMPLE:
            return "SAMPLE";
        case D3DDECLUSAGE_CLIPDISTANCE:
            return "SV_ClipDistance";
        case D3DDECLUSAGE_POINTSPRITE:
            return "POINTSPRITE";
        case D3DDECLUSAGE_VFACE:
            return "SV_IsFrontFace";
        case D3DDECLUSAGE_POSITIONT:
        default:
            Check9on12(false);
            return "";
        }
    }

    static ShaderConv::TEXTURETYPE ConvertShaderResourceDimensionToTextureType(D3D12_SRV_DIMENSION dim)
    {
        switch (dim)
        {
            case D3D12_SRV_DIMENSION_TEXTURE2D:
            case D3D12_SRV_DIMENSION_TEXTURE2DMS:
                return ShaderConv::TEXTURETYPE_2D;
            case D3D12_SRV_DIMENSION_TEXTURE3D:
                return ShaderConv::TEXTURETYPE_VOLUME;
            case D3D12_SRV_DIMENSION_TEXTURECUBE:
                return ShaderConv::TEXTURETYPE_CUBE;
            case D3D12_SRV_DIMENSION_BUFFER:
            case D3D12_SRV_DIMENSION_UNKNOWN:
            case D3D12_SRV_DIMENSION_TEXTURE1D:
            case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
            case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
            case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
            case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
            default:
                Check9on12(false); // Unsupported SRV dimension
                return ShaderConv::TEXTURETYPE_UNKNOWN;
        }
    }

    static void ConvertToRGBA(DWORD dwValueIn, FLOAT vOut[4])
    {
        vOut[0] = ((dwValueIn >> 16) & 0xFF) / 255.0f;
        vOut[1] = ((dwValueIn >> 8) & 0xFF) / 255.0f;
        vOut[2] = ((dwValueIn >> 0) & 0xFF) / 255.0f;
        vOut[3] = ((dwValueIn >> 24) & 0xFF) / 255.0f;
    }

    static void ConvertToRGB(DWORD dwValueIn, FLOAT vOut[3])
    {
        vOut[0] = ((dwValueIn >> 16) & 0xFF) / 255.0f;
        vOut[1] = ((dwValueIn >> 8) & 0xFF) / 255.0f;
        vOut[2] = ((dwValueIn >> 0) & 0xFF) / 255.0f;
    }

    static float ConvertToFloat(DWORD dwValueIn)
    {
        return (dwValueIn & 0xFF) / 255.0f;
    }

    // DX9 combined both the specified shader and shader register into a single DWORD
    static void ConvertDX9TextureIndexToShaderStageAndRegisterIndex(_In_ DWORD dwStage, _Out_ D3D12TranslationLayer::EShaderStage &shaderStage, _Out_ DWORD &shaderRegister)
    {
        if (dwStage < MAX_PIXEL_SAMPLERS)
        {
            shaderStage = D3D12TranslationLayer::e_PS;
            shaderRegister = dwStage;
        }
        else if ((dwStage >= VERTEX_SAMPLER0) && (dwStage < (VERTEX_SAMPLER0 + D3DHAL_SAMPLER_MAXVERTEXSAMP)))
        {
            shaderStage = D3D12TranslationLayer::e_VS;;
            shaderRegister = dwStage - VERTEX_SAMPLER0;
        }
        else
        {
            Check9on12(false);
        }
    }

    static DXGI_FORMAT ConvertStrideToIndexBufferFormat(UINT stride)
    {
        switch (stride)
        {
        case 2:
            return DXGI_FORMAT_R16_UINT;
        case 4:
            return DXGI_FORMAT_R32_UINT;
        default:
            Check9on12(false);
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    static UINT CalcVertexCount(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount)
    {
        switch (primitiveType)
        {
        case D3DPT_POINTLIST:
            return primitiveCount;
        case D3DPT_LINELIST:
            return primitiveCount * 2;
        case D3DPT_LINESTRIP:
            return primitiveCount + 1;
        case D3DPT_TRIANGLELIST:
            return primitiveCount * 3;
        case D3DPT_TRIANGLESTRIP:
            return primitiveCount + 2;
        case D3DPT_TRIANGLEFAN:
            return primitiveCount + 2;
        }
        return 0;
    }

    static D3D12_RESOURCE_STATES GetDefaultResourceState(D3D12_HEAP_TYPE heapType)
    {
        switch (heapType)
        {
        case D3D12_HEAP_TYPE_READBACK:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        case D3D12_HEAP_TYPE_UPLOAD:
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        default:
            return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    static UINT8 GetBlockWidth(DXGI_FORMAT format)
    {
        if (IsBlockCompressedFormat(format))
        {
            return 4;
        }
        else
        {
            return 1;
        }

    }


    static UINT8 GetBytesPerUnit(DXGI_FORMAT format)
    {
        return static_cast<UINT8>(CD3D11FormatHelper::GetBitsPerUnit(format) / 8);
    }

    static UINT DepthBiasFactorFromDSVFormat(DXGI_FORMAT dxgiFormat)
    {
        switch (dxgiFormat)
        {
        default:
            Check9on12(false);
            return 0;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_D32_FLOAT:
            return 1 << 23;
        case DXGI_FORMAT_D16_UNORM:
            return 1 << 16;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return 1 << 24;
        }
    }

    static D3D12_FILTER_TYPE ConvertD3D9GenMipsFilterType(D3DDDITEXTUREFILTERTYPE filterType)
    {
        switch (filterType)
        {
        case D3DDDITEXF_NONE:    // filtering disabled (valid for mip filter only)
        case D3DDDITEXF_POINT:    // nearest
            return D3D12_FILTER_TYPE_POINT;
        case D3DDDITEXF_LINEAR:    // linear interpolation
        case D3DDDITEXF_ANISOTROPIC:    // anisotropic
            return D3D12_FILTER_TYPE_LINEAR;
            break;
        case D3DDDITEXF_PYRAMIDALQUAD:    // 4-sample tent
        case D3DDDITEXF_GAUSSIANQUAD:    // 4-sample gaussian
        default:
            Check9on12(false); //TODO: will we need a custom sample pattern?
            return (D3D12_FILTER_TYPE)-1;
        }
    }

    static D3D12_FILTER ConvertD3D9FilterType(D3DDDITEXTUREFILTERTYPE filterType)
    {
        D3D12_FILTER filterD3D12 = D3D12_FILTER_MIN_MAG_MIP_POINT;

        switch (filterType)
        {
        case D3DDDITEXF_NONE:    // filtering disabled (valid for mip filter only)
        case D3DDDITEXF_POINT:    // nearest
            filterD3D12 = D3D12_FILTER_MIN_MAG_MIP_POINT;
            break;
        case D3DDDITEXF_LINEAR:    // linear interpolation
            filterD3D12 = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            break;
        case D3DDDITEXF_ANISOTROPIC:    // anisotropic
            filterD3D12 = D3D12_FILTER_ANISOTROPIC;
            break;
        case D3DDDITEXF_PYRAMIDALQUAD:    // 4-sample tent
        case D3DDDITEXF_GAUSSIANQUAD:    // 4-sample gaussian
        default:
            Check9on12(false); //TODO: will we need a custom sample pattern?
        }

        return filterD3D12;
    }

    template<typename DataType>
    static bool IsLockingSubrange(const DataType &LockFlags)
    {
        return LockFlags.AreaValid || LockFlags.BoxValid || LockFlags.RangeValid;
    }

    static D3DDDI_LOCKFLAGS convertAsyncFlags(D3DDDI_LOCKASYNCFLAGS asyncFlags)
    {
        D3DDDI_LOCKFLAGS flags;

        flags.NoOverwrite = asyncFlags.NoOverwrite;
        flags.Discard = asyncFlags.Discard;
        flags.RangeValid = asyncFlags.RangeValid;
        flags.AreaValid = asyncFlags.AreaValid;
        flags.BoxValid = asyncFlags.BoxValid;
        flags.NotifyOnly = asyncFlags.NotifyOnly;

        //TODO: the following flags is not being taken in consideration in this conversion.
        //asyncFlags.NoExistingReferences

        return flags;
    }


    static D3D12TranslationLayer::MAP_TYPE GetMapTypeFlag(bool lockReadOnly, bool lockWriteOnly, bool lockDiscard, bool lockNoOverwrite, bool videoCompressedBuffer, D3D12TranslationLayer::RESOURCE_CPU_ACCESS CPUAccessFlag)
    {
        UNREFERENCED_PARAMETER(lockWriteOnly);

        if (lockDiscard &&  videoCompressedBuffer)
        {
            return D3D12TranslationLayer::MAP_TYPE_WRITE_DISCARD;
        }

        bool bIsWrite = !lockReadOnly && (CPUAccessFlag & D3D12TranslationLayer::RESOURCE_CPU_ACCESS_WRITE);
        // TODO: (11369816) Any map involving a read resource is demoted to READWRITE map. If we find apps that 
        // frequently hit this path, we'll need to support the odd semantics of nooverwrite/discard
        // on a cpu readable resource
        bool bIsRead = (CPUAccessFlag & D3D12TranslationLayer::RESOURCE_CPU_ACCESS_READ);

        if (bIsWrite)
        {
            if (bIsRead)
            {
                return D3D12TranslationLayer::MAP_TYPE_READWRITE;
            }
            else
            {
                if (lockDiscard)
                {
                    return D3D12TranslationLayer::MAP_TYPE_WRITE_DISCARD;
                }
                else if (lockNoOverwrite)
                {
                    return D3D12TranslationLayer::MAP_TYPE_WRITE_NOOVERWRITE;
                }
                else
                {
                    return D3D12TranslationLayer::MAP_TYPE_WRITE;
                }
            }
        }
        else if (bIsRead)
        {
            return D3D12TranslationLayer::MAP_TYPE_READ;
        }
        else
        {
            // If an app requests to read from a vertex or index buffer,
            // they can just go ahead and read. However, there are some apps that
            // write to buffers that they locked read-only anyway, so we'll make this
            // a synchronizing map for write.
            return D3D12TranslationLayer::MAP_TYPE_WRITE;
        }
    }

    typedef std::bitset<MAX_RENDER_TARGETS> RTVBitMask;

    struct WeakHash
    {
        WeakHash(UINT32 value) :m_data(value){}

        WeakHash() :m_data(0){}

        bool operator==(const WeakHash& other) const { return other.m_data == m_data; }

        WeakHash& operator=(const UINT32 value)
        {
            m_data = value;
            return *this;
        }

        //Required for stl map
        struct Hasher{
            size_t operator()(WeakHash const& other) const { return size_t(other.m_data); }
        };

        static WeakHash GetHashForEmptyObject() { return WeakHash(1); }

        bool Initialized() { return m_data != 0; }

        UINT32 m_data;
    };

    //+---------------------------------------------------------------------------
    //
    //  Copyright (c) Microsoft, 2011. All rights reserved.
    //
    //  Contents:   Implementation of 32 bit Cyclic Redundancy Check.  This is the same
    //              as implementation as in Base\ntos\rtl\checksum.c
    //
    //  Author:     Niklas Borson (niklasb@microsoft.com)
    //
    //  History:    05-19-2011   niklasb    Created
    //
    //----------------------------------------------------------------------------


    //
    // This is the precomputed data table for the CRC32 algorithm as specified
    // in IS0 3309. See RFC-1662 and RFC-1952 for implementation details and
    // references.
    namespace
    {
        static const UINT32 g_crcLookupTable[] = {
            0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
            0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
            0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
            0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
            0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
            0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
            0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
            0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
            0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
            0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
            0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
            0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
            0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
            0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
            0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
            0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
            0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
            0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
            0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
            0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
            0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
            0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
            0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
            0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
            0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
            0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
            0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
            0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
            0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
            0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
            0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
            0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
            0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
            0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
            0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
            0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
            0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
            0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
            0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
            0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
            0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
            0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
            0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
        };

    } // end anonymous namespace

    // ComputeCrc32 - Compute the CRC32 as specified in in IS0 3309.
    //      See RFC-1662 and RFC-1952 for implementation details and references.
    //
    // The partialCrc is the result from a previous call, if computing the CRC
    // for non-contiguous data. On the first call, this parameter should be zero.
    //
    static UINT32 ComputeCrc32(_In_reads_bytes_(length) const void* buffer, _In_ size_t length, _In_ UINT32 partialCrc = 0)
    {
        auto pfnCalcCrcForUINT32 = [](UINT32 crc, UINT32 data) { return g_crcLookupTable[(crc ^ data) & 0xff] ^ (crc >> 8); };
        UINT32 crc = ~partialCrc;

        UINT32* data = (UINT32*)buffer;
        const size_t iterations = length / sizeof(UINT32);
        for (size_t i = 0; i < iterations; i++)
        {
            crc = pfnCalcCrcForUINT32(crc, data[i]);
        }

        const size_t uint8Iterations = length % sizeof(UINT32);
        if (uint8Iterations)
        {
            UINT8* remainderData = (UINT8 *)(&data[iterations]);
            UINT32 combinedRemainderData = 0;
            for (size_t i = 0; i < uint8Iterations; i++)
            {
                combinedRemainderData |= remainderData[i] << (i * 8);
            }
            crc = pfnCalcCrcForUINT32(crc, combinedRemainderData);
        }

        return ~crc;
    }

    // Note that cpuid instruction is not support on some older CPUs. This
    // is fine given that this test will not produce results in any reasonable
    // amount of time on such processors.
    static bool CanUseSSE4_2()
    {
#if (_M_IX86 || _M_AMD64 )
        int cpu_info[4] = {};

        __cpuid(cpu_info, 1);

        return (cpu_info[2] >> 20) & 1;
#else
        return false;
#endif /* (_M_IX86 || _M_AMD64 ) */
    }

    static const bool g_cUseSSE4_2 = CanUseSSE4_2();

    static WeakHash HashData(_In_reads_bytes_(numBytes) const void* data, _In_ size_t numBytes, _In_ WeakHash hash = 0)
    {
#if (_M_IX86 || _M_AMD64) && !defined(_M_HYBRID_X86_ARM64)
        const size_t SSE4_CRC_ALIGNEMENT = sizeof(SSE4_CRC_INPUT_TYPE);
        if (g_cUseSSE4_2)
        {
            const size_t roundingIterations = (numBytes & (SSE4_CRC_ALIGNEMENT - 1));
            UINT8* unalignedData = (UINT8*)data;
            for (size_t i = 0; i < roundingIterations; i++)
            {
                hash = _mm_crc32_u8(hash.m_data, unalignedData[i]);
            }
            unalignedData += roundingIterations;
            numBytes -= roundingIterations;

            SSE4_CRC_INPUT_TYPE* alignedData = (SSE4_CRC_INPUT_TYPE*)unalignedData;
            Check9on12((numBytes % SSE4_CRC_ALIGNEMENT) == 0);
            const size_t numIterations = (numBytes / SSE4_CRC_ALIGNEMENT);
            for (size_t i = 0; i < numIterations; i++)
            {
                hash = (UINT32)SSE4_CRC_FUNCTION((SSE4_CRC_INPUT_TYPE)hash.m_data, alignedData[i]);
            }

            return WeakHash(hash);
        }
        else
        {
            return WeakHash(ComputeCrc32(data, numBytes, hash.m_data));
        }
#else
        return WeakHash(ComputeCrc32(data, numBytes, hash.m_data));
#endif
    }

    enum OffsetType {OFFSET_IN_BYTES, OFFSET_IN_VERTICES, OFFSET_IN_INDICES};
    class OffsetArg
    {
    public:
        OffsetType m_type;

        static OffsetArg AsOffsetInVertices(INT offsetInVertices)
        {
            OffsetArg offset;
            offset.m_type = OFFSET_IN_VERTICES;
            offset.m_offsetInVertices = offsetInVertices;
            return offset;
        }

        static OffsetArg AsOffsetInBytes(INT offsetInBytes)
        {
            OffsetArg offset;
            offset.m_type = OFFSET_IN_BYTES;
            offset.m_offsetInBytes = offsetInBytes;
            return offset;
        }

        static OffsetArg AsOffsetInIndices(UINT offsetInIndicies)
        {
            OffsetArg offset;
            offset.m_type = OFFSET_IN_INDICES;
            offset.m_offsetInIndices = offsetInIndicies;
            return offset;
        }

        INT GetOffsetInBytes() { Check9on12(m_type == OffsetType::OFFSET_IN_BYTES); return m_offsetInBytes; };
        INT GetOffsetInVertices() { Check9on12(m_type == OffsetType::OFFSET_IN_VERTICES); return m_offsetInVertices; };
        UINT GetOffsetInIndices() { Check9on12(m_type == OffsetType::OFFSET_IN_INDICES); return m_offsetInIndices; };
    private:
        union
        {
            INT m_offsetInBytes;
            INT m_offsetInVertices;
            UINT m_offsetInIndices;
        };

    };
};
