// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12::Test
{
	class PipelineStateCacheMock : public PipelineStateCache
	{
	public:
		PipelineStateCacheMock(Device& device) : PipelineStateCache(device) {}
	};

	class PipelineStateCacheFactoryFake : public PipelineStateCacheFactory
	{
	public:
		PipelineStateCache Create(Device& device) { return PipelineStateCacheMock(device); }
	};
}