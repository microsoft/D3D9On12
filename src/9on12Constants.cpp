// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY SetVertexShaderConstF(_In_ HANDLE hDevice, _In_ const D3DDDIARG_SETVERTEXSHADERCONST* pRegisters, _In_ const VOID* pData)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pRegisters == nullptr || (pRegisters->Count > 0 && pData == nullptr))
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetConstantsManager().GetVertexShaderConstants().GetConstantBufferData(ShaderConv::CB_FLOAT).SetData(pData, pRegisters->Register, pRegisters->Count);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetVertexShaderConstI(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETVERTEXSHADERCONSTI* pRegisters, _In_ CONST INT* pData)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pRegisters == nullptr || (pRegisters->Count > 0 && pData == nullptr))
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetConstantsManager().GetVertexShaderConstants().GetConstantBufferData(ShaderConv::CB_INT).SetData(pData, pRegisters->Register, pRegisters->Count);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetVertexShaderConstB(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETVERTEXSHADERCONSTB* pRegisters, _In_ CONST BOOL* pData)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pRegisters == nullptr || (pRegisters->Count > 0 && pData == nullptr))
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetConstantsManager().GetVertexShaderConstants().GetConstantBufferData(ShaderConv::CB_BOOL).SetData(pData, pRegisters->Register, pRegisters->Count);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetPixelShaderConstF(_In_ HANDLE hDevice, _In_ const D3DDDIARG_SETPIXELSHADERCONST* pRegisters, _In_ const FLOAT* pData)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);

        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pRegisters == nullptr || (pRegisters->Count > 0 && pData == nullptr))
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetConstantsManager().GetPixelShaderConstants().GetConstantBufferData(ShaderConv::CB_FLOAT).SetData(pData, pRegisters->Register, pRegisters->Count);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetPixelShaderConstI(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPIXELSHADERCONSTI* pRegisters, _In_ CONST INT* pData)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pRegisters == nullptr || (pRegisters->Count > 0 && pData == nullptr))
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetConstantsManager().GetPixelShaderConstants().GetConstantBufferData(ShaderConv::CB_INT).SetData(pData, pRegisters->Register, pRegisters->Count);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetPixelShaderConstB(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_SETPIXELSHADERCONSTB* pRegisters, _In_ CONST BOOL* pData)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pRegisters == nullptr || (pRegisters->Count > 0 && pData == nullptr))
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->GetConstantsManager().GetPixelShaderConstants().GetConstantBufferData(ShaderConv::CB_BOOL).SetData(pData, pRegisters->Register, pRegisters->Count);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    HRESULT ConstantsManager::Init()
    {
        m_nullCB.m_buffer = m_nullCB.m_allocator.Allocate(g_cMaxConstantBufferSize);

        memset(m_nullCB.m_buffer.m_pMappedAddress, 0, g_cMaxConstantBufferSize);

        //We should never be writing to the null CB
        m_nullCB.m_buffer.m_pMappedAddress = nullptr;

        // There must be a resource bound to each root constant slot
        m_vertexShaderData.NullOutBindings(m_device, m_nullCB);
        m_pixelShaderData.NullOutBindings(m_device, m_nullCB);

        return S_OK;
    }

    void ConstantsManager::Destroy()
    {
        m_vertexShaderData.Destroy();
        m_geometryShaderData.Destroy();
        m_pixelShaderData.Destroy();
    }

    void ConstantBufferBinding::Destroy()
    {
        m_allocator.Destroy();
    }

    void ConstantBufferBinding::Version(const void* pData, UINT dataSize)
    {
        m_buffer = m_allocator.Allocate(dataSize);

        if (pData)
        {
            memcpy(m_buffer.m_pMappedAddress, pData, dataSize);
        }
    }

    void ConstantsManager::BindToPipeline(Device& device, ConstantBufferBinding& buffer, D3D12TranslationLayer::EShaderStage shaderStage)
    {
        switch (shaderStage)
        {
        case D3D12TranslationLayer::e_VS:
            device.SetConstantBuffer<D3D12TranslationLayer::e_VS>(buffer.m_shaderRegister, buffer.m_buffer.m_pResource, buffer.m_buffer.m_offsetFromBase);
            break;
        case D3D12TranslationLayer::e_GS:
            device.SetConstantBuffer<D3D12TranslationLayer::e_GS>(buffer.m_shaderRegister, buffer.m_buffer.m_pResource, buffer.m_buffer.m_offsetFromBase);
            break;
        case D3D12TranslationLayer::e_PS:
            device.SetConstantBuffer<D3D12TranslationLayer::e_PS>(buffer.m_shaderRegister, buffer.m_buffer.m_pResource, buffer.m_buffer.m_offsetFromBase);
            break;
        default:
            assert(false);
        }
    }

    void ConstantsManager::StageConstants::Destroy()
    {
        m_floats.Destroy();
        m_integers.Destroy();
        m_booleans.Destroy();
    }

    void ConstantsManager::PixelShaderConstants::Destroy()
    {
        StageConstants::Destroy();
        m_extension1.Destroy();
        m_extension2.Destroy();
        m_extension3.Destroy();
    }

    void ConstantsManager::VertexShaderConstants::Destroy()
    {
        StageConstants::Destroy();
        m_extension.Destroy();
    }

    void ConstantsManager::GeometryShaderConstants::Destroy()
    {
        m_extension.Destroy();
    }

    void ConstantsManager::StageConstants::UpdateAppVisibleAndBindToPipeline(Device& device, UINT maxFloats, UINT maxInts, UINT maxBools)
    {
        Check9on12(maxFloats % 4 == 0);
        Result result = m_floats.UpdateData(device, maxFloats / 4);
        if (maxFloats && result == Result::S_CHANGE)
        {
            BindToPipeline(device, m_floats.m_binding, m_shaderType);
        }

        Check9on12(maxInts % 4 == 0);
        result = m_integers.UpdateData(device, maxInts / 4);
        if (maxInts && result == Result::S_CHANGE)
        {
            BindToPipeline(device, m_integers.m_binding, m_shaderType);
        }

        result = m_booleans.UpdateData(device, maxBools);
        if (maxBools && result == Result::S_CHANGE)
        {
            BindToPipeline(device, m_booleans.m_binding, m_shaderType);
        }
    }

    void ConstantsManager::StageConstants::NullOutBindings(Device& /*device*/, ConstantBufferBinding& nullCB)
    {
        m_floats.ReplaceResource(nullCB);
        m_integers.ReplaceResource(nullCB);
        m_booleans.ReplaceResource(nullCB);
    }

    void ConstantsManager::BindShaderConstants()
    {
        D3D12VertexShader* pVs = m_device.GetPipelineState().GetVertexStage().GetCurrentD3D12VertexShader();
        D3D12PixelShader* pPs = m_device.GetPipelineState().GetPixelStage().GetCurrentD3D12PixelShader();

        m_vertexShaderData.UpdateAppVisibleAndBindToPipeline(m_device, pVs->m_floatConstsUsed, pVs->m_intConstsUsed, pVs->m_boolConstsUsed);
        m_pixelShaderData.UpdateAppVisibleAndBindToPipeline(m_device, pPs->m_floatConstsUsed, pPs->m_intConstsUsed, pPs->m_boolConstsUsed);

        //Bind Internal Extension Constants (used by the Shader Converter)
        {
            BindToPipeline(m_device, m_vertexShaderData.m_extension, D3D12TranslationLayer::EShaderStage::e_VS);
            BindToPipeline(m_device, m_geometryShaderData.m_extension, D3D12TranslationLayer::EShaderStage::e_GS);
            BindToPipeline(m_device, m_pixelShaderData.m_extension1, D3D12TranslationLayer::EShaderStage::e_PS);
            BindToPipeline(m_device, m_pixelShaderData.m_extension2, D3D12TranslationLayer::EShaderStage::e_PS);
            BindToPipeline(m_device, m_pixelShaderData.m_extension3, D3D12TranslationLayer::EShaderStage::e_PS);
        }
    }

    void ConstantsManager::UpdateVertexShaderExtension(const ShaderConv::VSCBExtension& data)
    {
        m_vertexShaderData.m_extension.Version(&data, sizeof(data));
    }

    void ConstantsManager::UpdateGeometryShaderExtension(const ShaderConv::VSCBExtension& data)
    {
        m_geometryShaderData.m_extension.Version(&data, sizeof(data));
    }

    Result ConstantsManager::UpdatePixelShaderExtension(const ShaderConv::eConstantBuffers extension, const void* pData, size_t dataSize)
    {
        ConstantBufferBinding* pBinding;
        switch (extension)
        {
        case ShaderConv::CB_PS_EXT:
            pBinding = &m_pixelShaderData.m_extension1;
            break;
        case ShaderConv::CB_PS_EXT2:
            pBinding = &m_pixelShaderData.m_extension2;
            break;
        case ShaderConv::CB_PS_EXT3:
            pBinding = &m_pixelShaderData.m_extension3;
            break;
        default:
            Check9on12(false);
            pBinding = nullptr;
            return Result::E_INVALID_ARG;
        }
        assert(dataSize <= UINT_MAX);
        pBinding->Version(pData, static_cast<UINT>(dataSize));

        return Result::S_SUCCESS;
    }
}