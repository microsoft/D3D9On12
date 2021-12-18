// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12::Test
{
    class AdapterMock : public Adapter
    {
    public:
        /*Helper ctor that uses placeholder open args*/
        AdapterMock() : Adapter(openAdapterPlaceholder) {}
        AdapterMock(D3DDDIARG_OPENADAPTER args) : Adapter(args) {}

    private:
        D3DDDIARG_OPENADAPTER openAdapterPlaceholder;
    };
}