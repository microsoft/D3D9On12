// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class VideoTranslate
    {
    public:
        static inline D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE GetInterlaceType(DXVADDI_SAMPLEFORMAT format)
        {
            if (format == DXVADDI_SampleFieldInterleavedEvenFirst ||
                format == DXVADDI_SampleFieldInterleavedOddFirst ||
                format == DXVADDI_SampleFieldSingleEven ||
                format == DXVADDI_SampleFieldSingleOdd)
            {
                return D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_FIELD_BASED;
            }
            else
            {
                return D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;
            }
        }

        static inline UINT GetBitstreamRawType(REFGUID Profile)
        {
            struct BitstreamTypePair {
                const GUID* DecodeProfile;
                UINT BitstreamRawType;
            };

            static const BitstreamTypePair map[] =
            {
                // H264 VLD SHORT slice = 2. (H264 DXVA Spec: 2.2 Semantics)
                &DXVA_ModeH264_F,                            2,
                &DXVA_ModeH264_E,                            2,
                &DXVA_ModeH264_VLD_WithFMOASO_NoFGT,         2,
                &DXVA_ModeH264_VLD_Stereo_Progressive_NoFGT, 2,
                &DXVA_ModeH264_VLD_Stereo_NoFGT,             2,
                &DXVA_ModeH264_VLD_Multiview_NoFGT,          2,
            };

            for (UINT Index = 0; Index < ARRAYSIZE(map); Index++)
            {
                if (IsEqualGUID(*map[Index].DecodeProfile, Profile))
                {
                    return map[Index].BitstreamRawType;
                }
            }

            return 1; // All other decode profiles should use bitstream type 1.
        }

        static inline void VideoDecodeConfig(_In_ const D3D12TranslationLayer::VIDEO_DECODE_DESC *pDesc, _In_ const D3D12TranslationLayer::VIDEO_DECODE_CONFIG *pConfig, _Out_ DXVADDI_CONFIGPICTUREDECODE *pDXVAConfig)
        {
            ZeroMemory(pDXVAConfig, sizeof(*pDXVAConfig));
            pDXVAConfig->guidConfigMBcontrolEncryption = DXVA_NoEncrypt;
            pDXVAConfig->guidConfigResidDiffEncryption = DXVA_NoEncrypt;
            pDXVAConfig->guidConfigBitstreamEncryption = DXVA_NoEncrypt;
            pDXVAConfig->ConfigBitstreamRaw = GetBitstreamRawType(pDesc->DecodeProfile);                      // always VLD, always SHORT slice
            pDXVAConfig->ConfigDecoderSpecific = (pConfig->ConfigDecoderSpecific & ~D3D12TranslationLayer::VIDEO_DECODE_CONFIG_SPECIFIC_ARRAY_OF_TEXTURES);
            pDXVAConfig->ConfigMinRenderTargetBuffCount = 0;
        }

        static inline void VideoDecodeDesc(_In_ const DXVADDI_DECODEINPUT *pDXVADesc, _Out_ D3D12TranslationLayer::VIDEO_DECODE_DESC *pDesc)
        {
            pDesc->DecodeProfile = *pDXVADesc->pGuid;
            pDesc->Width = pDXVADesc->VideoDesc.SampleWidth;
            pDesc->Height = pDXVADesc->VideoDesc.SampleHeight;
            pDesc->DecodeFormat = ConvertFromDDIToDXGIFORMAT(pDXVADesc->VideoDesc.Format);
        }

        static inline D3DDDIFORMAT VideoDecodeBufferType(D3D12TranslationLayer::VIDEO_DECODE_BUFFER_TYPE Type)
        {
            static const D3DDDIFORMAT map[] =
            {
                D3DDDIFMT_PICTUREPARAMSDATA,
                D3DDDIFMT_INVERSEQUANTIZATIONDATA,
                D3DDDIFMT_SLICECONTROLDATA,
                D3DDDIFMT_BITSTREAMDATA,
                D3DDDIFMT_DXVACOMPBUFFER_BASE                     // TODO: what to do for VP9 probability buffer???
            };
            return map[Type];
        }

        static inline void VideoDecodeBufferInfo(D3D12TranslationLayer::VIDEO_DECODE_BUFFER_TYPE type, UINT size, _Out_ DXVADDI_DECODEBUFFERINFO *pDXVABufferInfo)
        {
            pDXVABufferInfo->CompressedBufferType = VideoDecodeBufferType(type);
            // D3D9 uses width & height instead of a direct size for the linear buffers.
            pDXVABufferInfo->CreationWidth = size;
            pDXVABufferInfo->CreationHeight = 1;
            pDXVABufferInfo->CreationPool = D3DDDIPOOL_NONLOCALVIDMEM;
        }

        static inline D3D12_VIDEO_DECODE_ARGUMENT_TYPE VideoDecodeArgumentType(D3DDDIFORMAT Type9)
        {
            switch (Type9)
            {
            case D3DDDIFMT_PICTUREPARAMSDATA:
                return D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS;
            case D3DDDIFMT_INVERSEQUANTIZATIONDATA:
                return D3D12_VIDEO_DECODE_ARGUMENT_TYPE_INVERSE_QUANTIZATION_MATRIX;
            case D3DDDIFMT_SLICECONTROLDATA:
                return D3D12_VIDEO_DECODE_ARGUMENT_TYPE_SLICE_CONTROL;
            default:
                assert(false);
                return (D3D12_VIDEO_DECODE_ARGUMENT_TYPE)-1;
            }
        }

        static inline void VideoColor(_In_ const DXVADDI_AYUVSAMPLE16 *pInput, _Out_writes_all_(4) float Output[4])
        {
            Output[0] = ((float)pInput->Y) / 0xffff;;
            Output[1] = ((float)pInput->Cb) / 0xffff;
            Output[2] = ((float)pInput->Cr) / 0xffff;
            Output[3] = ((float)pInput->Alpha) / 0xffff;
        }

        static inline void VideoColor(_In_ const DXVAHDDDI_COLOR *pInput, _Out_writes_all_(4) float Output[4])
        {
            Output[0] = pInput->YCbCr.Y;
            Output[1] = pInput->YCbCr.Cb;
            Output[2] = pInput->YCbCr.Cr;
            Output[3] = pInput->YCbCr.A;
        }

        static inline DXGI_COLOR_SPACE_TYPE ColorSpaceType(_In_ DXGI_FORMAT format, _In_ UINT height, _In_ DXVADDI_EXTENDEDFORMAT extendedFormat)
        {
            DXGI_COLOR_SPACE_TYPE dxgiColorSpace = (DXGI_COLOR_SPACE_TYPE)0;
            DXVADDI_VIDEOTRANSFERMATRIX videoTransferMatrix = (DXVADDI_VIDEOTRANSFERMATRIX)extendedFormat.VideoTransferMatrix;

            if (extendedFormat.VideoTransferMatrix == DXVADDI_VideoTransferMatrix_Unknown)
            {
                // according to MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/ms698715(v=vs.85).aspx
                // DXVA2_VideoTransferMatrix_Unknown For standard - definition content, treat as DXVA2_VideoTransferMatrix_BT601.
                // For high - definition content, treat as DXVA2_VideoTransferMatrix_BT709. (High - definition content is defined for this purpose 
                // as anything with a source height greater than 576 lines.)
                if (height > 576)
                {
                    videoTransferMatrix = DXVADDI_VideoTransferMatrix_BT709;
                }
                else
                {
                    videoTransferMatrix = DXVADDI_VideoTransferMatrix_BT601;
                }
            }

            // if VideoTransferMatrix not equal to 709, we assume it is 601.

            if (CD3D11FormatHelper::YUV(format))
            {
                if (extendedFormat.NominalRange == DXVADDI_NominalRange_16_235)    // studio
                {
                    dxgiColorSpace = (videoTransferMatrix == DXVADDI_VideoTransferMatrix_BT709) ?
                        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709 :
                        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601;
                }
                else                                                               // assume full
                {
                    dxgiColorSpace = (videoTransferMatrix == DXVADDI_VideoTransferMatrix_BT709) ?
                        DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709 :
                        DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601;
                }
            }
            else
            {
                // RGB format

                // Note that there are no RGB P601 color spaces, so we will map to P709.
                if (extendedFormat.NominalRange == DXVADDI_NominalRange_16_235)    // studio
                {
                    // assume G22 as there is no G10 studio color space
                    dxgiColorSpace = DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709;
                }
                else                                                               // assume full
                {
                    if (extendedFormat.VideoTransferFunction == DXVADDI_VideoTransFunc_10)
                    {
                        dxgiColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
                    }
                    else                                                           // assume G22
                    {
                        dxgiColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
                    }
                }
            }

            return dxgiColorSpace;
        }

        static inline DXGI_COLOR_SPACE_TYPE ColorSpaceType(_In_ DXGI_FORMAT format, bool RGBStudio, bool YUVStudio, bool BT709)
        {
            DXGI_COLOR_SPACE_TYPE dxgiColorSpace = (DXGI_COLOR_SPACE_TYPE)0;

            // if VideoTransferMatrix not equal to 709, we assume it is 601.
            if (CD3D11FormatHelper::YUV(format))
            {
                if (YUVStudio)
                {
                    dxgiColorSpace = (BT709) ?
                        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709 :
                        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601;
                }
                else
                {
                    dxgiColorSpace = (BT709) ?
                        DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709 :
                        DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601;
                }
            }
            else
            {
                // RGB format, assume G22
                if (RGBStudio)
                {
                    dxgiColorSpace = DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709;
                }
                else
                {
                    dxgiColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
                }
            }

            return dxgiColorSpace;
        }


        template <typename T>
        static inline DXGI_COLOR_SPACE_TYPE ColorSpaceType(_In_ DXGI_FORMAT format, _In_ const T &colorSpaceData)
        {
            bool RGBStudio = colorSpaceData.RGB_Range == 1 ? true : false;
            bool YUVStudio = colorSpaceData.Nominal_Range == DXVAHDDDI_NOMINAL_RANGE_16_235 ? true : (colorSpaceData.YCbCr_xvYCC == 0 ? true : false);
            bool BT709 = colorSpaceData.YCbCr_Matrix == 1 ? true : false;
            return ColorSpaceType(format, RGBStudio, YUVStudio, BT709);
        }

        template <typename T>
        static inline T FromFixed32(_In_ const DXVADDI_FIXED32 &fixed)
        {
            DXVA2_Fixed32 intermediate = *(DXVA2_Fixed32 *)&fixed;
            return (T)DXVA2FixedToFloat(intermediate);
        }

        template <typename T>
        static inline DXVADDI_FIXED32 ToFixed32(_In_ T input)
        {
            DXVA2_Fixed32 intermediate = DXVA2FloatToFixed((float)input);
            return *(DXVADDI_FIXED32 *)&intermediate;
        }

        static inline D3D12_VIDEO_FIELD_TYPE VideoFieldType(DXVAHDDDI_FRAME_FORMAT frameFormat)
        {
            static const D3D12_VIDEO_FIELD_TYPE map[] =
            {
                D3D12_VIDEO_FIELD_TYPE_NONE,                           // DXVAHDDDI_FRAME_FORMAT_PROGRESSIVE
                D3D12_VIDEO_FIELD_TYPE_INTERLACED_TOP_FIELD_FIRST,     // DXVAHDDDI_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST
                D3D12_VIDEO_FIELD_TYPE_INTERLACED_BOTTOM_FIELD_FIRST,  // DXVAHDDDI_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST
            };
            return map[frameFormat];
        }

        static inline D3D12_VIDEO_FIELD_TYPE VideoFieldType(UINT frameFormat)
        {
            static const D3D12_VIDEO_FIELD_TYPE map[] =
            {
                D3D12_VIDEO_FIELD_TYPE_NONE,                            // DXVADDI_SampleUnknown = 0,
                D3D12_VIDEO_FIELD_TYPE_NONE,                            // 1
                D3D12_VIDEO_FIELD_TYPE_NONE,                            // DXVADDI_SampleProgressiveFrame = 2,
                D3D12_VIDEO_FIELD_TYPE_INTERLACED_TOP_FIELD_FIRST,      // DXVADDI_SampleFieldInterleavedEvenFirst = 3,
                D3D12_VIDEO_FIELD_TYPE_INTERLACED_BOTTOM_FIELD_FIRST,   // DXVADDI_SampleFieldInterleavedOddFirst = 4,
                D3D12_VIDEO_FIELD_TYPE_NONE,                            // DXVADDI_SampleFieldSingleEven = 5,
                D3D12_VIDEO_FIELD_TYPE_NONE,                            // DXVADDI_SampleFieldSingleOdd = 6,
                D3D12_VIDEO_FIELD_TYPE_NONE,                            // DXVADDI_SampleSubStream = 7
            };
            return map[frameFormat];
        }

        static inline DXGI_RATIONAL FrameRate(DXVAHDDDI_RATIONAL frameRate)
        {
            static_assert(sizeof(DXVAHDDDI_RATIONAL) == sizeof(DXGI_RATIONAL), "sizes must match");
            return *(DXGI_RATIONAL *)&frameRate;
        }

        static inline DXGI_RATIONAL FrameRate(DXVADDI_FREQUENCY frameRate)
        {
            static_assert(sizeof(DXVADDI_FREQUENCY) == sizeof(DXGI_RATIONAL), "sizes must match");
            return *(DXGI_RATIONAL *)&frameRate;
        }

        static inline D3D12_VIDEO_PROCESS_FILTER_FLAGS VideoProcessFilterFlag(_In_ DXVAHDDDI_STREAM_STATE StreamState)
        {
            D3D12_VIDEO_PROCESS_FILTER_FLAGS flag = (D3D12_VIDEO_PROCESS_FILTER_FLAGS)0;
            switch (StreamState)
            {
                case  DXVAHDDDI_STREAM_STATE_FILTER_BRIGHTNESS:
                    flag = D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS;
                    break;

                case  DXVAHDDDI_STREAM_STATE_FILTER_CONTRAST:
                    flag = D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST;
                    break;

                case  DXVAHDDDI_STREAM_STATE_FILTER_HUE:
                    flag = D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE;
                    break;

                case  DXVAHDDDI_STREAM_STATE_FILTER_SATURATION:
                    flag = D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION;
                    break;

                case  DXVAHDDDI_STREAM_STATE_FILTER_NOISE_REDUCTION:
                    flag = D3D12_VIDEO_PROCESS_FILTER_FLAG_NOISE_REDUCTION;
                    break;

                case  DXVAHDDDI_STREAM_STATE_FILTER_EDGE_ENHANCEMENT:
                    flag = D3D12_VIDEO_PROCESS_FILTER_FLAG_EDGE_ENHANCEMENT;
                    break;

                case  DXVAHDDDI_STREAM_STATE_FILTER_ANAMORPHIC_SCALING:
                    flag = D3D12_VIDEO_PROCESS_FILTER_FLAG_ANAMORPHIC_SCALING;
                    break;

                default:
                    ThrowFailure(E_INVALIDARG);
                    break;
            }
            return flag;
        }

        static inline D3D12_VIDEO_PROCESS_FILTER VideoProcessFilter(_In_ DXVAHDDDI_STREAM_STATE StreamState)
        {
            D3D12_VIDEO_PROCESS_FILTER filter = (D3D12_VIDEO_PROCESS_FILTER)0;
            switch (StreamState)
            {
            case  DXVAHDDDI_STREAM_STATE_FILTER_BRIGHTNESS:
                filter = D3D12_VIDEO_PROCESS_FILTER_BRIGHTNESS;
                break;

            case  DXVAHDDDI_STREAM_STATE_FILTER_CONTRAST:
                filter = D3D12_VIDEO_PROCESS_FILTER_CONTRAST;
                break;

            case  DXVAHDDDI_STREAM_STATE_FILTER_HUE:
                filter = D3D12_VIDEO_PROCESS_FILTER_HUE;
                break;

            case  DXVAHDDDI_STREAM_STATE_FILTER_SATURATION:
                filter = D3D12_VIDEO_PROCESS_FILTER_SATURATION;
                break;

            case  DXVAHDDDI_STREAM_STATE_FILTER_NOISE_REDUCTION:
                filter = D3D12_VIDEO_PROCESS_FILTER_NOISE_REDUCTION;
                break;

            case  DXVAHDDDI_STREAM_STATE_FILTER_EDGE_ENHANCEMENT:
                filter = D3D12_VIDEO_PROCESS_FILTER_EDGE_ENHANCEMENT;
                break;

            case  DXVAHDDDI_STREAM_STATE_FILTER_ANAMORPHIC_SCALING:
                filter = D3D12_VIDEO_PROCESS_FILTER_ANAMORPHIC_SCALING;
                break;

            default:
                ThrowFailure(E_INVALIDARG);
                break;
            }
            return filter;
        }

        static inline void VideoProcessDesc(_In_ const DXVADDI_VIDEOPROCESSORINPUT *pVideoProcessorInput, UINT maxInputStreams, _Out_ D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS &args, _Out_ D3DDDIFORMAT &rtFormat)
        {
            VideoProcessDesc(&pVideoProcessorInput->VideoDesc, maxInputStreams, args, rtFormat);
            rtFormat = pVideoProcessorInput->RenderTargetFormat;
        }

        static inline void VideoProcessDesc(_In_ const DXVADDI_QUERYPROCAMPINPUT *pQueryProcAmpInput, UINT maxInputStreams, _Out_ D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS &args, _Out_ D3DDDIFORMAT &rtFormat)
        {
            VideoProcessDesc(&pQueryProcAmpInput->VideoDesc, maxInputStreams, args, rtFormat);
            rtFormat = pQueryProcAmpInput->RenderTargetFormat;
        }

        static inline void VideoProcessDesc(_In_ const DXVADDI_VIDEODESC *pVideoDesc, UINT maxInputStreams, _Out_ D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS &args, _Out_ D3DDDIFORMAT &rtFormat)
        {
            args.InputFieldType = VideoFieldType(pVideoDesc->SampleFormat.SampleFormat);
            args.InputFrameRate = FrameRate(pVideoDesc->InputSampleFreq);
            args.InputWidth = pVideoDesc->SampleWidth;
            args.InputHeight = pVideoDesc->SampleHeight;
            args.OutputFrameRate = FrameRate(pVideoDesc->OutputFrameFreq);
            args.OutputWidth = pVideoDesc->SampleWidth;
            args.OutputHeight = pVideoDesc->SampleHeight;
            args.MaxInputStreams = maxInputStreams;
            rtFormat = pVideoDesc->Format;
        }

        static inline void VideoProcessDesc(_In_ const DXVAHDDDI_CONTENT_DESC *pVideoDesc, UINT maxInputStreams, _Out_ D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS &args, _Out_ D3DDDIFORMAT &rtFormat)
        {
            args.InputFieldType = VideoFieldType(pVideoDesc->InputFrameFormat);
            args.InputFrameRate = FrameRate(pVideoDesc->InputFrameRate);
            args.InputWidth = pVideoDesc->InputWidth;
            args.InputHeight = pVideoDesc->InputHeight;
            args.OutputFrameRate = FrameRate(pVideoDesc->OutputFrameRate);
            args.OutputWidth = pVideoDesc->OutputWidth;
            args.OutputHeight = pVideoDesc->OutputHeight;
            args.MaxInputStreams = maxInputStreams;
            rtFormat = D3DDDIFMT_UNKNOWN;
        }


        static inline void VideoProcessDesc(_In_ const DXVAHDDDI_DEVICE_DESC *pDesc, UINT maxInputStreams, _Out_ D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS &args, _Out_ D3DDDIFORMAT &rtFormat)
        {
            VideoProcessDesc(pDesc->pContentDesc, maxInputStreams, args, rtFormat);
        }

        static inline UINT MapFlags(const UINT *pMap, UINT inputFlags)
        {
            UINT index = 0;
            UINT outputFlags = 0;
            while (inputFlags)
            {
                if (inputFlags & 0x1)
                {
                    outputFlags |= pMap[index];
                }
                inputFlags >>= 1;
                ++index;
            }
            return outputFlags;
        }


        static inline UINT ToDXVAHDVPDevCaps(UINT colorConversionCaps)
        {
            static_assert(DXVAHDDDI_DEVICE_CAPS_LINEAR_SPACE            == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_LINEAR_SPACE &&
                          DXVAHDDDI_DEVICE_CAPS_xvYCC                   == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_xvYCC &&
                          DXVAHDDDI_DEVICE_CAPS_RGB_RANGE_CONVERSION    == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_RGB_RANGE_CONVERSION &&
                          DXVAHDDDI_DEVICE_CAPS_YCbCr_MATRIX_CONVERSION == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_YCbCr_MATRIX_CONVERSION,
                          "DeviceCaps must match");
            colorConversionCaps &= ~D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_NOMINAL_RANGE;

            return colorConversionCaps;
        }

        static inline void AddVideoProcessCaps(_In_ D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT &dx12Support, _Out_ DXVAHDDDI_VPDEVCAPS &dxvaHDVPDevCaps, _Out_ DXVAHDDDI_FILTER_RANGE_DATA *pFilterRanges, _In_ UINT cFilterRanges, _In_ D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS colorConversionCap, _Out_ bool& fAlphaBlending)
        {
            UNREFERENCED_PARAMETER(cFilterRanges);

            //
            // FeatureCaps: DXVAHDDDI_FEATURE_CAPS & 
            //
            static const UINT mapDXVAHDFeatureCaps[] =
            {
                DXVAHDDDI_FEATURE_CAPS_ALPHA_FILL,          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_FILL
                DXVAHDDDI_FEATURE_CAPS_LUMA_KEY,            // D3D12_VIDEO_PROCESS_FEATURE_FLAG_LUMA_KEY
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_STEREO
                DXVAHDDDI_FEATURE_CAPS_ROTATION,            // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ROTATION
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_FLIP
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_BLENDING
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_PIXEL_ASPECT_RATIO
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_AUTO_PROCESSING
            };
            static const UINT mapDXVAFeatureCaps[] =
            {
                DXVADDI_VIDEOPROCESS_PLANARALPHA,           // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_FILL
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_LUMA_KEY
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_STEREO
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ROTATION
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_FLIP
                DXVADDI_VIDEOPROCESS_ALPHABLEND,            // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_BLENDING
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_PIXEL_ASPECT_RATIO
                0,                                          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_AUTO_PROCESSING
            };
            dxvaHDVPDevCaps.FeatureCaps |= MapFlags(mapDXVAHDFeatureCaps, (UINT)dx12Support.FeatureSupport);
            fAlphaBlending = (dx12Support.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_BLENDING) ? true : fAlphaBlending;

            //
            // Filter support: DXVAHDDDI_FILTER_CAPS & DXVADDI_VIDEOPROCESSORCAPS.ProcAmpControlCaps
            //
            // we assume identical filters for DXVA / DX12
            static_assert(DXVAHDDDI_FILTER_CAPS_BRIGHTNESS         == D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS, "DXVAHD and 12 filter flags should match");
            static_assert(DXVAHDDDI_FILTER_CAPS_CONTRAST           == D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST, "DXVAHD and 12 filter flags should match");
            static_assert(DXVAHDDDI_FILTER_CAPS_HUE                == D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE, "DXVAHD and 12 filter flags should match");
            static_assert(DXVAHDDDI_FILTER_CAPS_SATURATION         == D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION, "DXVAHD and 12 filter flags should match");
            static_assert(DXVAHDDDI_FILTER_CAPS_NOISE_REDUCTION    == D3D12_VIDEO_PROCESS_FILTER_FLAG_NOISE_REDUCTION, "DXVAHD and 12 filter flags should match");
            static_assert(DXVAHDDDI_FILTER_CAPS_EDGE_ENHANCEMENT   == D3D12_VIDEO_PROCESS_FILTER_FLAG_EDGE_ENHANCEMENT, "DXVAHD and 12 filter flags should match");
            static_assert(DXVAHDDDI_FILTER_CAPS_ANAMORPHIC_SCALING == D3D12_VIDEO_PROCESS_FILTER_FLAG_ANAMORPHIC_SCALING, "DXVAHD and 12 filter flags should match");

            static_assert(DXVADDI_PROCAMP_BRIGHTNESS == D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS, "DXVA and 12 filter flags should match");
            static_assert(DXVADDI_PROCAMP_CONTRAST   == D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST, "DXVA and 12 filter flags should match");
            static_assert(DXVADDI_PROCAMP_HUE        == D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE, "DXVA and 12 filter flags should match");
            static_assert(DXVADDI_PROCAMP_SATURATION == D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION, "DXVA and 12 filter flags should match");

            UINT origFilterCaps = dxvaHDVPDevCaps.FilterCaps;
            dxvaHDVPDevCaps.FilterCaps |= (UINT)dx12Support.FilterSupport;

            // turn off flags not present in DXVA / DXVAHD
            dxvaHDVPDevCaps.FilterCaps &= ~D3D12_VIDEO_PROCESS_FILTER_FLAG_STEREO_ADJUSTMENT;

            UINT newMask = dxvaHDVPDevCaps.FilterCaps & (~origFilterCaps);
            UINT filter = 0;
            while (newMask)
            {
                assert(filter < cFilterRanges);
                if (newMask & 0x1)
                {
                    pFilterRanges[filter].Minimum = dx12Support.FilterRangeSupport[filter].Minimum;
                    pFilterRanges[filter].Maximum = dx12Support.FilterRangeSupport[filter].Maximum;
                    pFilterRanges[filter].Default = dx12Support.FilterRangeSupport[filter].Default;
                    pFilterRanges[filter].Multiplier = dx12Support.FilterRangeSupport[filter].Multiplier;
                }
                newMask >>= 1;
                ++filter;
            }

            //
            // InputFormatCaps: DXVAHDDDI_INPUT_FORMAT_CAPS
            // 
            if (!CD3D11FormatHelper::YUV(dx12Support.InputSample.Format.Format))
            {
                if (dx12Support.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_LUMA_KEY)
                {
                    dxvaHDVPDevCaps.InputFormatCaps |= DXVAHDDDI_INPUT_FORMAT_CAPS_RGB_LUMA_KEY;
                }
                if (dx12Support.FilterSupport & (D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS | D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST | D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE | D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION))
                {
                    // Note: the below should be true, but we have several outputs indicating no RGB_PROCAMP even if the condition above is true. So, ignoring RGB_PROCAMP.
                    // dxvaHDVPDevCaps.InputFormatCaps |= DXVAHDDDI_INPUT_FORMAT_CAPS_RGB_PROCAMP;
                }
                if (dx12Support.InputFieldType != D3D12_VIDEO_FIELD_TYPE_NONE)
                {
                    dxvaHDVPDevCaps.InputFormatCaps |= DXVAHDDDI_INPUT_FORMAT_CAPS_RGB_INTERLACED;
                }
            }

            //
            // color conversion caps
            //
            dxvaHDVPDevCaps.DeviceCaps |= ToDXVAHDVPDevCaps(colorConversionCap);
        }

        static inline void AddVideoProcessCaps(UINT MaxInputStreams, _Out_ DXVAHDDDI_VPDEVCAPS &dxvaHDVPDevCaps)
        {
            dxvaHDVPDevCaps.MaxInputStreams = MaxInputStreams;
            dxvaHDVPDevCaps.MaxStreamStates = MaxInputStreams;       // Assuming identical values for stream states & input streams.
        }
   };
}