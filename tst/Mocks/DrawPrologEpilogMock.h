// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12::Test
{
    class DrawPrologEpilogMock 
    {
    public:
        MOCK_METHOD(HRESULT, DrawProlog, (Device& device, OffsetArg BaseVertexStart, UINT vertexCount, OffsetArg baseIndexLocation, UINT indexCount, D3DPRIMITIVETYPE primitiveType, UINT& instancesToDraw, bool& skipDraw));
        MOCK_METHOD(HRESULT, DrawEpilog, (Device& device));
    };

#define GENERATE_DRAWPROLOG_DRAWEPILOG_FORWARDING(mock) \
    static HRESULT DrawProlog(Device& device, OffsetArg BaseVertexStart, UINT vertexCount, OffsetArg baseIndexLocation, UINT indexCount, D3DPRIMITIVETYPE primitiveType, UINT& instancesToDraw, bool& skipDraw) \
    { \
        return mock.DrawProlog(device, BaseVertexStart, vertexCount, baseIndexLocation, indexCount, primitiveType, instancesToDraw, skipDraw); \
    } \
    static HRESULT DrawEpilogue(Device& device) { return mock.DrawEpilog(device); }

}