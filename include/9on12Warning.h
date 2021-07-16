// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    namespace WarningStrings
    {
        static const std::string g_cDXGIWarningHeader = "D3D9on12 Warning: Forcing DXGI format conversion: ";
        static const std::string g_cPerformanceWarningHeader = "D3D9on12 Performance Warning: ";
        static const std::string g_cDDIEntryHeader = "D3D9on12 DDI Entrypoint: ";

        static const std::string g_cSystemMemoryBoundWarning = "Binding system memory resource directly to pipeline.";
        static const std::string g_cSystemMemoryBoundAsShaderResourceWarning = "Binding system memory resource as a shader resource. This path is a serious perf concern and should be reviewed if this gets hit outside of the HLK.";
        static const std::string g_cCopyToSystemMemoryWarning = "Copy is being called with system memory as the destination";
        static const std::string g_cIgnoringMultipleDirtyRectsWarning = "Suboptimally ignoring presents with multiple dirty rects";
        static const std::string g_cInefficientUnbindingArrayDeletion = "A resource has a large amount of bindings causing inefficient O(N) deletions when being unbound";
    };

    static void PrintDebugMessage(std::string message)
    {
        OutputDebugStringA(message.c_str());
        OutputDebugStringA("\n");
    }

#define DXGI_FORMAT_WARNING(message) \
        { \
            if (RegistryConstants::g_cSpewDXGIFormatWarnings) \
            { \
                PrintDebugMessage(WarningStrings::g_cDXGIWarningHeader); \
                PrintDebugMessage(std::string(message)); \
            } \
            if (RegistryConstants::g_cAssertDXGIFormatWarnings) \
            { \
                Check9on12(false); \
            } \
        }

#define PERFORMANCE_WARNING(message) \
        { \
            if (RegistryConstants::g_cSpewPerformanceWarnings) \
            { \
                PrintDebugMessage(WarningStrings::g_cPerformanceWarningHeader + std::string(message)); \
            } \
        }

#define SYSTEM_MEMORY_RESOURCE_BOUND_WARNING() PERFORMANCE_WARNING(WarningStrings::g_cSystemMemoryBoundWarning)
#define SYSTEM_MEMORY_RESOURCE_BOUND_AS_SHADER_RESOURCE_WARNING() PERFORMANCE_WARNING(WarningStrings::g_cSystemMemoryBoundAsShaderResourceWarning)
#define COPY_TO_SYSTEM_MEMORY_WARNING() PERFORMANCE_WARNING(WarningStrings::g_cCopyToSystemMemoryWarning)
#define IGNORING_MULTIPLE_DIRTY_RECTS() PERFORMANCE_WARNING(WarningStrings::g_cIgnoringMultipleDirtyRectsWarning)
#define INEFFICIENT_UNBINDING_ARRAY_DELETION() PERFORMANCE_WARNING(WarningStrings::g_cInefficientUnbindingArrayDeletion)

};