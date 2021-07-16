// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class InputLayout
    {
    public:
        InputLayout(Device &device);
        ~InputLayout();

        static FORCEINLINE HANDLE GetHandleFromInputLayout(InputLayout* pInputLayout){ return static_cast<HANDLE>(pInputLayout); }
        static FORCEINLINE InputLayout* GetInputLayoutFromHandle(HANDLE hInputLayout){ return static_cast<InputLayout*>(hInputLayout); }

        HRESULT Init(_In_ CONST D3DDDIVERTEXELEMENT* pVertexElements, UINT numElements);

        UINT GetVertexElementCount() { return m_numVertexElements; }
        D3DDDIVERTEXELEMENT &GetVertexElement(UINT elementIndex) 
        { 
            Check9on12(elementIndex < m_numVertexElements); 
            return m_pVertexElements[elementIndex]; 
        }

        bool VerticesArePreTransformed(){ return m_hasPreTransformedVertices; }
        bool HasPointSizePerVertex(){ return m_hasPerVertexPointSize; }
        ShaderConv::VSInputDecls& GetVSInputDecls() { return m_vsInputDecls; }

        WeakHash GetHash();
        UINT GetStreamMask() { return m_streamMask; }

    private:
        D3DDDIVERTEXELEMENT m_pVertexElements[MAX_INPUT_ELEMENTS];
        UINT m_numVertexElements;
        UINT m_streamMask;

        bool m_hasPreTransformedVertices;

        //If the vertex has a point size, we must use that over the 
        //size set by raster states. Also hints that we need a GS.
        bool m_hasPerVertexPointSize;
        ShaderConv::VSInputDecls m_vsInputDecls;
        Device &m_device;

        WeakHash m_hash;
    };
}