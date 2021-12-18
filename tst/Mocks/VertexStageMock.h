// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12::Test 
{
    class VertexStageMock : public VertexStage
    {
    public:
        VertexStageMock(VertexShader&& tlShaderCache, GeometryShader&& geometryShaderCache, PipelineStateDirtyFlags& pipelineStateDirtyFlags, RasterStatesWrapper& rasterStates)
            : VertexStage(std::move(tlShaderCache), std::move(geometryShaderCache), pipelineStateDirtyFlags, rasterStates) {}

    private:

    };

    class VertexStageFactoryFake : public VertexStageFactory
    {
    public:
        VertexStage Create(Device& device, PipelineStateDirtyFlags& pipelineStateDirtyFlags, RasterStatesWrapper& rasterStates) override {
            return VertexStageMock(VertexShader(device), GeometryShader(device), pipelineStateDirtyFlags, rasterStates);
        }
    };
}