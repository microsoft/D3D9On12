#include <CommonTestHeader.h>

#include <Mocks.h>
#include <DrawPrologEpilogMock.h>

namespace D3D9on12
{
    static Test::DrawPrologEpilogMock sDrawPrologEpilogMock;

    GENERATE_DRAWPROLOG_DRAWEPILOG_FORWARDING(sDrawPrologEpilogMock);
}

#include <9on12DrawApis.inl>

namespace D3D9on12::Test
{
#define SUITE_NAME D3D9on12DrawTest
    

    TEST(SUITE_NAME, DrawIndexedPrimitive2_ShouldGetVertexCountFrom_pData) 
    {
        AdapterMock adapter();
        DeviceMock device();

        HANDLE deviceHandle = &device;
        D3DDDIARG_DRAWINDEXEDPRIMITIVE2 data;
        data.BaseVertexOffset = 0;
        data.MinIndex = 0;
        data.NumVertices = 4;
        data.PrimitiveCount = 2;
        data.PrimitiveType = D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST;
        data.StartIndexOffset = 0;

        UINT indicesSize = sizeof(UINT16);
        UINT16 indexBuffer[] = {0,1,2,1,3,2};

        DrawIndexedPrimitive2(deviceHandle, &data, indicesSize, &indexBuffer, nullptr);
    }
}