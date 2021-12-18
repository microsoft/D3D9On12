// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12::Test
{
    class ConstantsManagerMock : public ConstantsManager
    {
    public:
        ConstantsManagerMock(Device& device) : ConstantsManager(device) {}
    };

    class ConstantsManagerFactoryFake : public ConstantsManagerFactory
    {
    public:
        ConstantsManager Create(Device& device) override { return ConstantsManager(device); }
    };
}