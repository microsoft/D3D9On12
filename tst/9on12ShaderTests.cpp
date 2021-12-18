//Copyright (c) Microsoft Corporation.
//Licensed under the MIT license.

#include <CommonTestHeader.h>
#include <Mocks.h>

namespace D3D9on12::Test
{
#define SUITE_NAME D3D9on12ShaderTest

    TEST(SUITE_NAME, ShaderMoveCopy) 
    {
        DeviceMock device = DeviceMockBuilder().Build();
        PipelineStateCacheMock pipelineStateCache(device);
        Shader from(device);
        byte bytecode[] = { 0,1,2,3,4,5,6,7 };
        size_t size = 8;
        from.Init(bytecode[0], size);

        Shader to(std::move(from));

    }
}