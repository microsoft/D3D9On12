// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12::Test
{
    class AdapterMock : public Adapter
    {
    public:
        AdapterMock(D3DDDIARG_OPENADAPTER args) : Adapter(args) {}
    };
}