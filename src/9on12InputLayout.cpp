// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY CreateVertexShaderDecl(
        _In_ HANDLE hDevice,
        _Inout_ D3DDDIARG_CREATEVERTEXSHADERDECL*pCreateVertexShaderDeclArg,
        _In_ CONST D3DDDIVERTEXELEMENT* pVertexElements)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pCreateVertexShaderDeclArg == nullptr)
        {
            return E_INVALIDARG;
        }

        if (pCreateVertexShaderDeclArg->NumVertexElements > 0 && pVertexElements == nullptr)
        {
            return E_INVALIDARG;
        }

        HRESULT hr = S_OK;
        InputLayout *pInputLayout = new InputLayout(*pDevice);
        if (pInputLayout == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
            hr = pInputLayout->Init(pVertexElements, pCreateVertexShaderDeclArg->NumVertexElements);
        }

        if (SUCCEEDED(hr))
        {
            pCreateVertexShaderDeclArg->ShaderHandle = InputLayout::GetHandleFromInputLayout(pInputLayout);
        }

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY DeleteVertexShaderDecl(_In_ HANDLE hDevice, _In_ HANDLE hShader)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        InputLayout *pInputLayout = InputLayout::GetInputLayoutFromHandle(hShader);

        if (pDevice == nullptr || pInputLayout == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        delete(pInputLayout);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetStreamSourceFreq(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETSTREAMSOURCEFREQ *pStreamSourceFreqArg)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pStreamSourceFreqArg == nullptr || pStreamSourceFreqArg->Stream >= MAX_VERTEX_STREAMS)
        {
            return E_INVALIDARG;
        }

        pDevice->SetStreamFrequency(pStreamSourceFreqArg->Stream, pStreamSourceFreqArg->Divider);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetVertexShaderDecl(_In_ HANDLE hDevice, _In_ HANDLE hInputLayout)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        InputLayout *pInputLayout = InputLayout::GetInputLayoutFromHandle(hInputLayout);
        if (pDevice == nullptr)
        {
            return E_INVALIDARG;
        }

        pDevice->GetPipelineState().GetInputAssembly().SetVertexDeclaration(pInputLayout);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    InputLayout::InputLayout(Device &device) :
        m_hasPreTransformedVertices(false),
        m_hasPerVertexPointSize(false),
        m_hash(0),
        m_numVertexElements(0),
        m_streamMask(0),
        m_vsInputDecls(MAXD3DDECLLENGTH),
        m_device(device)
    {
        memset(m_pVertexElements, 0, sizeof(m_pVertexElements));
    }

    InputLayout::~InputLayout()
    {
        auto &InputAssembly = m_device.GetPipelineState().GetInputAssembly();
        if (&InputAssembly.GetInputLayout() == this)
        {
            InputAssembly.SetVertexDeclaration(nullptr);
        }
    }

    HRESULT InputLayout::Init(_In_ CONST D3DDDIVERTEXELEMENT* pVertexElements, UINT numElements)
    {
        Check9on12(numElements < MAX_INPUT_ELEMENTS);
        m_numVertexElements = numElements;

        for (UINT i = 0; i < numElements; i++)
        {
            UINT isTransformedPosition = 0;
            m_pVertexElements[i] = pVertexElements[i];

            if (m_pVertexElements[i].Usage == D3DDECLUSAGE_POSITIONT && m_pVertexElements[i].UsageIndex == 0)
            {
                m_pVertexElements[i].Usage = D3DDECLUSAGE_POSITION;
                m_hasPreTransformedVertices = true;
                isTransformedPosition = 1;
            }
            if (m_pVertexElements[i].Usage == D3DDECLUSAGE_PSIZE)
            {
                m_hasPerVertexPointSize = true;
            }

            UINT conversions = [](D3DDECLTYPE type)
            {
                switch (type)
                {
                case D3DDECLTYPE_UDEC3:
                    return ShaderConv::VSInputDecl::UDEC3;
                case D3DDECLTYPE_DEC3N:
                    return ShaderConv::VSInputDecl::DEC3N;
                default:
                    return IsIntType(type) ?
                        ShaderConv::VSInputDecl::NeedsIntToFloatConversion :
                        ShaderConv::VSInputDecl::None;
                }
            }((D3DDECLTYPE)m_pVertexElements[i].Type);
            m_vsInputDecls.AddDecl(m_pVertexElements[i].Usage, m_pVertexElements[i].UsageIndex, i, isTransformedPosition, conversions);

            m_streamMask |= BIT(m_pVertexElements[i].Stream);
        }

        GetHash();
        return S_OK;
    }


    WeakHash InputLayout::GetHash()
    {
        if (m_hash.Initialized() == false)
        {
            if (m_numVertexElements != 0)
            {
                m_hash = HashData(m_pVertexElements, m_numVertexElements * sizeof(m_pVertexElements[0]), m_hash);
                m_hash = HashData(&m_vsInputDecls[0], sizeof(m_vsInputDecls[0]) * m_vsInputDecls.GetSize(), m_hash);
            }
            else
            {
                m_hash = WeakHash::GetHashForEmptyObject();
            }
        }

        return m_hash;
    }
};