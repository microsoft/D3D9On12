// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12::Test
{
	class PipelineStateMock : public PipelineState
	{
	public:
		PipelineStateMock(Device& device) : PipelineState(device, vertexStageFactory) {}

	private:
		VertexStageFactoryFake vertexStageFactory;
	};

	class PipelineStateFactoryFake : public PipelineStateFactory
	{
	public:
		PipelineState Create(Device& device) { 
			return PipelineStateMock(device); 
		}
	};
}