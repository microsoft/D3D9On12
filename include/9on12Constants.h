// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class HeapAllocator;

    struct Float4
    {
        float data[4];

        Float4(){};

        Float4(const Float4&  other)
        {
            data[0] = other.data[0];
            data[1] = other.data[1];
            data[2] = other.data[2];
            data[3] = other.data[3];
        }
    };

    struct Int4
    {
        UINT data[4];
        Int4(){};

        Int4(const Int4&  other)
        {
            data[0] = other.data[0];
            data[1] = other.data[1];
            data[2] = other.data[2];
            data[3] = other.data[3];
        }
    };

    struct ConstantBufferBinding
    {
        ConstantBufferBinding(UINT shaderRegister, Device& device) :
            m_allocator(device, 1024*1024, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT),
            m_shaderRegister(shaderRegister){};

        void Version(const void* pData, UINT dataSize);
        void Destroy();

        // Because a roll over may happen at any time, 
        // each binding must have it's own allocator so 
        // that the resource doesn't get pulled from under it
        FastUploadAllocator m_allocator;
        FastUploadAllocator::SubBuffer m_buffer;
        UINT m_shaderRegister;
    };

    class ConstantsManager
    {
        struct ConstantBufferData
        {
            ConstantBufferData(UINT SizePerElement, UINT NumElements, UINT shaderRegister, Device& device) :
                m_sizePerElement(SizePerElement),
                m_lastCopySize(0),
                m_dataDirty(false),
                m_binding(shaderRegister, device)
            {
                m_data.resize(m_sizePerElement * NumElements);
                memset(m_data.data(), 0, m_data.size());
            }

            void Destroy() { m_binding.Destroy(); }

            void SetData(const void* pData, UINT startReg, UINT num)
            {
                UINT copySize = num * m_sizePerElement;
                UINT startOffset = startReg * m_sizePerElement;
                BYTE *pDest = m_data.data() + startOffset;
                Check9on12(startOffset + copySize <= m_data.size());

                memcpy(pDest, pData, copySize);
                m_dataDirty = true;
            }

            Result UpdateData(Device& /*device*/, UINT maxSet)
            {
                Result result = Result::S_SUCCESS;
                // Even if the contents aren't dirty, we should update the data if 
                // the maxSet has increased from the last binding
                if (maxSet && (m_dataDirty || maxSet > m_lastCopySize))
                {
                    UINT dataSize = min(maxSet * m_sizePerElement, static_cast<UINT>(m_data.size()));
                    m_binding.Version(m_data.data(), dataSize);

                    m_dataDirty = false;
                    m_lastCopySize = maxSet;
                    result = Result::S_CHANGE;
                }

                return result;
            }

            void ReplaceResource(ConstantBufferBinding& nullCB)
            {
                m_binding.m_buffer = nullCB.m_buffer;
            }

            ConstantBufferBinding m_binding;

        private:
            std::vector<BYTE> m_data;
            const UINT m_sizePerElement;
            size_t m_lastCopySize;

            bool m_dataDirty;
        };

        struct StageConstants
        {
            StageConstants(D3D12TranslationLayer::EShaderStage type, Device& device) :
                m_floats(sizeof(Float4), max(MAX_VS_CONSTANTSF, MAX_PS_CONSTANTSF), ShaderConv::CB_FLOAT, device),
                m_integers(sizeof(Int4), max(MAX_VS_CONSTANTSI, MAX_PS_CONSTANTSI), ShaderConv::CB_INT, device),
                m_booleans(sizeof(BOOL), max(MAX_VS_CONSTANTSB, MAX_PS_CONSTANTSB), ShaderConv::CB_BOOL, device),
                m_shaderType(type){}

            void Destroy();

            ConstantBufferData &GetConstantBufferData(ShaderConv::eConstantBuffers type)
            {
                switch (type)
                {
                case ShaderConv::CB_FLOAT:
                    return m_floats;
                case ShaderConv::CB_INT:
                    return m_integers;
                case ShaderConv::CB_BOOL:
                    return m_booleans;
                default:
                    Check9on12(false); // Invalid type passed in
                    return *(ConstantBufferData*)nullptr;
                }
            }

            void UpdateAppVisibleAndBindToPipeline(Device& device, UINT maxFloats, UINT maxInts, UINT maxBools);
            void NullOutBindings(Device& device, ConstantBufferBinding& nullCB);

        private:
            const D3D12TranslationLayer::EShaderStage m_shaderType;
            ConstantBufferData m_floats;
            ConstantBufferData m_integers;
            ConstantBufferData m_booleans;
        };

        struct VertexShaderConstants : public StageConstants
        {
            VertexShaderConstants(Device& device) :
                m_extension(ShaderConv::CB_VS_EXT, device),
                StageConstants(D3D12TranslationLayer::EShaderStage::e_VS, device){};
            void Destroy();

            ConstantBufferBinding m_extension;
        };

        struct GeometryShaderConstants
        {
            GeometryShaderConstants(Device& device) :
                m_extension(ShaderConv::CB_VS_EXT, device) {};

            void Destroy();

            ConstantBufferBinding m_extension;
        };

        struct PixelShaderConstants : public StageConstants
        {
            PixelShaderConstants(Device& device) :
                m_extension1(ShaderConv::CB_PS_EXT, device),
                m_extension2(ShaderConv::CB_PS_EXT2, device),
                m_extension3(ShaderConv::CB_PS_EXT3, device),
                StageConstants(D3D12TranslationLayer::EShaderStage::e_PS, device){};
            void Destroy();

            ConstantBufferBinding m_extension1;
            ConstantBufferBinding m_extension2;
            ConstantBufferBinding m_extension3;
        };

    public:
        ConstantsManager(Device &device) :
            m_device(device),
            m_vertexShaderData(device),
            m_geometryShaderData(device),
            m_pixelShaderData(device),
            m_nullCB(0, device)
            {}

        void Destroy();

        HRESULT Init();

        void BindShaderConstants();

        VertexShaderConstants& GetVertexShaderConstants() { return m_vertexShaderData; }
        PixelShaderConstants& GetPixelShaderConstants() { return m_pixelShaderData; }
        ConstantBufferBinding& NullCB() { return m_nullCB; }

        void UpdateVertexShaderExtension(const ShaderConv::VSCBExtension& data);
        void UpdateGeometryShaderExtension(const ShaderConv::VSCBExtension& data);
        Result UpdatePixelShaderExtension(const ShaderConv::eConstantBuffers extension, const void* pData, size_t dataSize);

        static void BindToPipeline(Device& device, ConstantBufferBinding& buffer, D3D12TranslationLayer::EShaderStage shaderStage);

    private:

        Device &m_device;
        VertexShaderConstants m_vertexShaderData;
        GeometryShaderConstants m_geometryShaderData;
        PixelShaderConstants m_pixelShaderData;

        static const size_t g_cMaxConstantBufferSize = 256 * sizeof(float);
        ConstantBufferBinding m_nullCB;
    };

};