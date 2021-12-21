// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"
#include <DxbcSigner.hpp>

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY CreatePixelShader(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEPIXELSHADER* pCreatePixelShader, _In_ CONST UINT* pByteCode)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pCreatePixelShader == nullptr || pByteCode == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        const byte* pShaderByteCode = (RegistryConstants::g_cDebugRedPixelShader) ? g_redOutputPS : (byte*)pByteCode;
        const size_t byteCodeSize = (RegistryConstants::g_cDebugRedPixelShader) ? sizeof(g_redOutputPS) : pCreatePixelShader->CodeSize;
        PixelShader* pShader = pDevice->m_PSDedupe.GetOrCreate(*pDevice, pShaderByteCode, byteCodeSize);

        pCreatePixelShader->ShaderHandle = Shader::GetHandleFromShader(pShader);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DeletePixelShader(_In_ HANDLE hDevice, _In_ HANDLE hShader)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        Shader* pShader = Shader::GetShaderFromHandle(hShader);
        if (pDevice == nullptr || pShader == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->m_PSDedupe.Release(static_cast<PixelShader*>(pShader));

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY CreateVertexShaderFunc(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEVERTEXSHADERFUNC* pCreateVertexShader, _In_ CONST UINT* pByteCode)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pCreateVertexShader == nullptr || pByteCode == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        const byte* pShaderByteCode = (RegistryConstants::g_cDebugPassThroughVertexShader) ? g_passThroughVS : (byte*)pByteCode;
        const size_t byteCodeSize = (RegistryConstants::g_cDebugPassThroughVertexShader) ? sizeof(g_passThroughVS) : pCreateVertexShader->Size;
        VertexShader* pShader = pDevice->m_VSDedupe.GetOrCreate(*pDevice, pShaderByteCode, byteCodeSize);

        pCreateVertexShader->ShaderHandle = Shader::GetHandleFromShader(pShader);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY DeleteVertexShaderFunc(_In_ HANDLE hDevice, _In_ HANDLE hShader)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        VertexShader* pShader = (VertexShader*)Shader::GetShaderFromHandle(hShader);
        if (pDevice == nullptr || pShader == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        pDevice->m_VSDedupe.Release(pShader);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    Shader::Shader(Device& device) :
        PipelineStateCacheKeyComponent(device.GetPipelineStateCache().GetCache()),
        m_parentDevice(device),
        m_DXBCBuilder(false),
        m_legacyCodeHash(0)
    {
    }

    Shader::Shader(Device& device, _In_ CONST byte& byteCode, _In_ size_t byteCodeLength, WeakHash hash) :
        PipelineStateCacheKeyComponent(device.GetPipelineStateCache().GetCache()),
        m_parentDevice(device),
        m_DXBCBuilder(false),
        m_legacyCodeHash(hash)
    {
        m_d3d9ByteCode = SizedBuffer(new byte[byteCodeLength], byteCodeLength);
        memcpy(m_d3d9ByteCode.m_ptr, &byteCode, byteCodeLength);
    }

    VertexShader::VertexShader(Device& parentDevice, _In_ CONST byte& byteCode, _In_ size_t byteCodeLength, WeakHash hash) :
        Shader(parentDevice, byteCode, byteCodeLength, hash) {}
    VertexShader::VertexShader(Device& parentDevice) :
        Shader(parentDevice) {}

    PixelShader::PixelShader(Device& parentDevice, _In_ CONST byte& byteCode, _In_ size_t byteCodeLength, WeakHash hash) :
        Shader(parentDevice, byteCode, byteCodeLength, hash) {}

    GeometryShader::GeometryShader(Device& parentDevice, _In_ CONST byte& byteCode, _In_ size_t byteCodeLength, WeakHash hash) :
        Shader(parentDevice, byteCode, byteCodeLength, hash) {}
    GeometryShader::GeometryShader(Device& parentDevice) :
        Shader(parentDevice) {}

    Shader::~Shader()
    {
        if (m_d3d9ByteCode.m_ptr) { delete[] m_d3d9ByteCode.m_ptr; }
    }

    void Shader::GenerateSignatureFromVSOutput(ShaderConv::VSOutputDecls& vsOut, DXBCInputSignatureBuilder& signature)
    {
        typedef std::tuple<_D3D11_INTERNALSHADER_PARAMETER_11_1, const char *> tupleType;

        // DXBC Linkage
        {
            std::vector<_D3D11_INTERNALSHADER_PARAMETER_11_1> parameters = std::vector<_D3D11_INTERNALSHADER_PARAMETER_11_1>(vsOut.GetSize());
            std::vector<std::string> semanticNames = std::vector<std::string>(vsOut.GetSize());

            std::vector<tupleType> pairs = std::vector<tupleType>(vsOut.GetSize());

            for (UINT i = 0; i < vsOut.GetSize(); i++)
            {
                D3DDECLUSAGE usage = static_cast<D3DDECLUSAGE>(vsOut[i].Usage);

                auto& parameter = std::get<0>(pairs[i]);
                parameter = {};
                parameter.SemanticIndex = vsOut[i].UsageIndex;
                parameter.SystemValue = ConvertToName(usage);
                parameter.Register = vsOut[i].RegIndex;
                parameter.Mask = vsOut[i].WriteMask;
                parameter.ComponentType = ConvertToComponentType(usage);

                auto& semantic = std::get<1>(pairs[i]);
                semantic = ConvertToSemanticNameForDXBCLinkage(usage);
            }

            // Signatures need to be sorted by their register value
            std::sort(pairs.begin(), pairs.end(), less_than_key<tupleType>());

            for (size_t i = 0; i < pairs.size(); i++)
            {
                parameters[i] = std::get<0>(pairs[i]);
                semanticNames[i] = std::get<1>(pairs[i]);
            }

            signature.SetParameters(&parameters[0], &semanticNames[0], static_cast<UINT>(parameters.size()));
        }
    }

    template<typename MapType>
    static void ClearMap(MapType& map)
    {
        for (auto derivedShader : map)
        {
            if (derivedShader.second.GetUnderlying())
            {
                derivedShader.second.Destroy();
            }
        }
    }

    VertexShader::~VertexShader()
    {
        if (m_parentDevice.GetPipelineState().GetVertexStage().GetCurrentD3D9VertexShader() == this)
        {
            m_parentDevice.GetPipelineState().GetVertexStage().SetVertexShader(nullptr);
        }

        ClearMap(m_derivedShaders);
    }

    PixelShader::~PixelShader()
    {
        if (m_parentDevice.GetPipelineState().GetPixelStage().GetCurrentD3D9PixelShader() == this)
        {
            m_parentDevice.GetPipelineState().GetPixelStage().SetPixelShader(nullptr);
        }

        ClearMap(m_derivedShaders);
    }

    GeometryShader::~GeometryShader()
    {
        // The GS is linked to the VS, deleting the Vertex Shader handles all the GS clean-up as well
        ClearMap(m_derivedShaders);
    }

    D3D12PixelShader& PixelShader::GetD3D12Shader(const ShaderConv::RasterStates &rasterStates, ShaderConv::VSOutputDecls& vsOutputDecls, D3D12Shader &inputShader)
    {
        HRESULT hr = S_OK;

        DerivedPixelShaderKey key(rasterStates, vsOutputDecls, GetHashForLegacyByteCode());

        auto derivedShader = m_derivedShaders.find(key);

        if (derivedShader != m_derivedShaders.end())
        {
            return derivedShader->second;
        }
        else
        {
            hr = ShaderConversionPrologue();
            CHECK_HR(hr);

            m_derivedShaders[key] = D3D12PixelShader(this);
            D3D12PixelShader& newPixelShader = m_derivedShaders[key];

            bool applyAnythingTimes0Equals0 = RegistryConstants::g_cAnythingTimes0Equals0 || (g_AppCompatInfo.AnythingTimes0Equals0ShaderMask & D3D9ON12_PIXEL_SHADER_MASK);
            ShaderConv::VSInputDecls vsInputDecls(ShaderConv::MAX_VS_INPUT_REGS);
            ShaderConv::ConvertShaderArgs convertArgs = ShaderConv::ConvertShaderArgs(
                m_parentDevice.GetD3D9ApiVersion(), 
                applyAnythingTimes0Equals0 ? ShaderConv::AnythingTimes0Equals0 : 0,
                rasterStates);
            convertArgs.type = ShaderConv::ConvertShaderArgs::SHADER_TYPE::SHADER_TYPE_PIXEL;
            convertArgs.pVsInputDecl = &vsInputDecls;
            convertArgs.pPsInputDecl = &vsOutputDecls;
            convertArgs.legacyByteCode.m_pByteCode = m_d3d9ByteCode.m_ptr;
            convertArgs.legacyByteCode.m_byteCodeSize = m_d3d9ByteCode.m_size;
            for (UINT i = 0; i < ARRAYSIZE(newPixelShader.m_inlineConsts); i++)
            {
                newPixelShader.m_inlineConsts[i] = std::move(convertArgs.m_inlineConsts[i]);
            }

            hr = m_parentDevice.m_ShaderConvAPI.ConvertShader(convertArgs);
            CHECK_HR(hr);

            const bool bVSOutputMatchesPSInput = convertArgs.AddedSystemSemantics.size() == 0;
            DXBCInputSignatureBuilder inputSignatureBuilder;
            SizedBuffer inputSignatureBuffer = {};
            if (bVSOutputMatchesPSInput)
            {
                inputSignatureBuffer = inputShader.m_outputSignature;
            }
            else
            {
                ShaderConv::VSOutputDecls patchedVSOutput = vsOutputDecls;
                for (auto &decl : convertArgs.AddedSystemSemantics)
                {
                    patchedVSOutput.AddDecl(decl);
                }
                GenerateSignatureFromVSOutput(patchedVSOutput, inputSignatureBuilder);
                inputSignatureBuffer = inputSignatureBuilder.GetData();
            }

            if (SUCCEEDED(hr) && convertArgs.convertedByteCode.m_pByteCode)
            {
                D3D12_SHADER_BYTECODE newByteCode = {};

                DXBCInputSignatureBuilder outputSignatureBuilder;
                
                //DXBC Output Linkage
                {
                    const int cMaxOutputParameters = ShaderConv::MAX_PS_COLOROUT_REGS + ShaderConv::MAX_PS_DEPTHOUT_REGS;
                    _D3D11_INTERNALSHADER_PARAMETER_11_1 parameters[cMaxOutputParameters] = {};
                    std::string parametersNames[cMaxOutputParameters] = {};
                    UINT numParameters = 0;
                    for (UINT i = 0; i < ShaderConv::MAX_PS_COLOROUT_REGS; i++)
                    {
                        if (convertArgs.outputRegistersMask & (1 << i))
                        {
                            parametersNames[numParameters] = "SV_Target";
                            _D3D11_INTERNALSHADER_PARAMETER_11_1 &parameter = parameters[numParameters];
                            parameter.SemanticIndex = i;
                            parameter.SystemValue = D3D10_NAME_TARGET;
                            parameter.Register = i;
                            parameter.ComponentType = D3D_REGISTER_COMPONENT_FLOAT32;//Can make this assumption because D3D9 can only work in floats
                            parameter.Mask = RGBA_MASK;

                            numParameters++;
                        }
                    }

                    if (convertArgs.outputRegistersMask & ShaderConv::DEPTH_OUTPUT_MASK)
                    {
                        parametersNames[numParameters] = "SV_Depth";
                        _D3D11_INTERNALSHADER_PARAMETER_11_1 &parameter = parameters[numParameters];
                        parameter.SemanticIndex = 0;
                        parameter.SystemValue = D3D10_NAME_DEPTH;
                        parameter.Register = (UINT)-1;
                        parameter.ComponentType = D3D_REGISTER_COMPONENT_FLOAT32;//Can make this assumption because D3D9 can only work in floats
                        parameter.Mask = R_MASK;

                        numParameters++;
                    }

                    assert(numParameters <= cMaxOutputParameters);
                    outputSignatureBuilder.SetParameters(parameters, parametersNames, numParameters);
                }

                SizedBuffer upgradedByteCode = SizedBuffer(convertArgs.convertedByteCode.m_pByteCode,convertArgs.convertedByteCode.m_byteCodeSize);
                SizedBuffer outputSignatureBuilderData = outputSignatureBuilder.GetData();

                hr = GenerateFinalD3D12Shader(upgradedByteCode,
                    newPixelShader, inputSignatureBuffer, outputSignatureBuilderData);

                ShaderConv::ShaderConverterAPI::CleanUpConvertedShader(convertArgs.convertedByteCode);
            }

            newPixelShader.m_floatConstsUsed = convertArgs.maxFloatConstsUsed;
            newPixelShader.m_intConstsUsed = convertArgs.maxIntConstsUsed;
            newPixelShader.m_boolConstsUsed = convertArgs.maxBoolConstsUsed;
            m_parentDevice.GetDataLogger().AddShaderData(D3D10_SB_PIXEL_SHADER, convertArgs.totalInstructionsEmitted, convertArgs.totalExtraInstructionsEmitted);

            return newPixelShader;
        }
    }

    D3D12GeometryShader& GeometryShader::GetD3D12Shader(D3D12VertexShader& currentVS, const ShaderConv::RasterStates& rasterStates)
    {
        DerivedGeometryShaderKey key(rasterStates, currentVS.m_vsOutputDecls);
        auto derivedShader = m_derivedShaders.find(key);

        if (derivedShader != m_derivedShaders.end())
        {
            return derivedShader->second;
        }
        else
        {
            HRESULT hr = ShaderConversionPrologue();
            CHECK_HR(hr);

            m_derivedShaders[key] = D3D12GeometryShader(this);
            D3D12GeometryShader& newGeometryShader = m_derivedShaders[key];

            // We don't pass AnythingTimes0Equals0 flag to the shader converter for 
            // geometry shaders since 9on12 controls the input to the GS and shouldn't
            // generally result in nan or inf results
            ShaderConv::CreateGeometryShaderArgs args = ShaderConv::CreateGeometryShaderArgs(
                m_parentDevice.GetD3D9ApiVersion(), 
                0, 
                currentVS.m_vsOutputDecls,
                &newGeometryShader.m_gsOutputDecls,
                rasterStates);
            hr = m_parentDevice.m_ShaderConvAPI.CreateGeometryShader(args);
            CHECK_HR(hr);

            if (SUCCEEDED(hr))
            {
                SizedBuffer upgradedByteCode = SizedBuffer(args.m_GSByteCode.m_pByteCode, args.m_GSByteCode.m_byteCodeSize);
                
                newGeometryShader.m_inputSignature = currentVS.m_outputSignature;
                newGeometryShader.m_outputSignature = currentVS.m_outputSignature;

                // In most cases, the GS inputs and outputs are the same, but if the GS does add an extra output
                // (i.e. PointSprites), we need to generate a separate output signature
                DXBCInputSignatureBuilder outputSignatureBuilder;
                if (!(args.m_VsOutputDecls == newGeometryShader.m_gsOutputDecls))
                {
                    GenerateSignatureFromVSOutput(newGeometryShader.m_gsOutputDecls, outputSignatureBuilder);
                    newGeometryShader.m_outputSignature = outputSignatureBuilder.GetData();
                }

                hr = GenerateFinalD3D12Shader(upgradedByteCode, newGeometryShader, newGeometryShader.m_inputSignature, newGeometryShader.m_outputSignature);
                CHECK_HR(hr);
            }

            ShaderConv::ShaderConverterAPI::CleanUpConvertedShader(args.m_GSByteCode);

            return newGeometryShader;
        }
    }

    D3D12VertexShader& VertexShader::GetD3D12Shader(const ShaderConv::RasterStates &rasterStates, InputLayout& inputLayout)
    {
        HRESULT hr = S_OK;

        DerivedVertexShaderKey key(rasterStates, inputLayout, GetHashForLegacyByteCode(), m_parentDevice.GetPointerToStreamFrequencies());

        auto derivedShader = m_derivedShaders.find(key);

        if (derivedShader != m_derivedShaders.end())
        {
            return derivedShader->second;
        }
        else
        {
            hr = ShaderConversionPrologue();
            CHECK_HR(hr);

            m_derivedShaders[key] = D3D12VertexShader(this);
            D3D12VertexShader& newVertexShader = m_derivedShaders[key];

            ShaderConv::VSInputDecls vsInputDecls = inputLayout.GetVSInputDecls();

            bool applyAnythingTimes0Equals0 = RegistryConstants::g_cAnythingTimes0Equals0 || (g_AppCompatInfo.AnythingTimes0Equals0ShaderMask & D3D9ON12_VERTEX_SHADER_MASK);
            ShaderConv::ConvertShaderArgs convertArgs = ShaderConv::ConvertShaderArgs(
                m_parentDevice.GetD3D9ApiVersion(),
                applyAnythingTimes0Equals0 ? ShaderConv::AnythingTimes0Equals0 : 0,
                rasterStates);
            convertArgs.type = ShaderConv::ConvertShaderArgs::SHADER_TYPE::SHADER_TYPE_VERTEX;
            convertArgs.pVsOutputDecl = &newVertexShader.m_vsOutputDecls;
            convertArgs.pVsInputDecl = &vsInputDecls;
            convertArgs.pPsInputDecl = nullptr;
            convertArgs.legacyByteCode.m_pByteCode = m_d3d9ByteCode.m_ptr;
            convertArgs.legacyByteCode.m_byteCodeSize = m_d3d9ByteCode.m_size;

            hr = m_parentDevice.m_ShaderConvAPI.ConvertShader(convertArgs);
            CHECK_HR(hr);

            if (SUCCEEDED(hr) && convertArgs.convertedByteCode.m_pByteCode)
            {
                SizedBuffer upgradedByteCode = SizedBuffer(convertArgs.convertedByteCode.m_pByteCode, convertArgs.convertedByteCode.m_byteCodeSize);
                hr = SetupInputSignaturesAndGetFinalCode(upgradedByteCode, vsInputDecls, &inputLayout, newVertexShader);
            }

            newVertexShader.m_floatConstsUsed = convertArgs.maxFloatConstsUsed;
            newVertexShader.m_intConstsUsed = convertArgs.maxIntConstsUsed;
            newVertexShader.m_boolConstsUsed = convertArgs.maxBoolConstsUsed;
            for (UINT i = 0; i < ARRAYSIZE(newVertexShader.m_inlineConsts); i++)
            {
                newVertexShader.m_inlineConsts[i] = std::move(convertArgs.m_inlineConsts[i]);
            }

            m_parentDevice.GetDataLogger().AddShaderData(D3D10_SB_VERTEX_SHADER, convertArgs.totalInstructionsEmitted, convertArgs.totalExtraInstructionsEmitted);
            ShaderConv::ShaderConverterAPI::CleanUpConvertedShader(convertArgs.convertedByteCode);

            return newVertexShader;
        }
    }

    D3D12VertexShader& VertexShader::GetD3D12ShaderForTL(InputLayout& inputLayout, const ShaderConv::RasterStates &rasterStates)
    {
        HRESULT hr = S_OK;

        // note: we always give a constant hash for the legacy shader code because TL shaders don't have a VS.
        DerivedVertexShaderKey key(rasterStates, inputLayout, WeakHash(0), m_parentDevice.GetPointerToStreamFrequencies());

        auto derivedShader = m_derivedShaders.find(key);

        if (derivedShader != m_derivedShaders.end())
        {
            return derivedShader->second;
        }
        else
        {
            hr = ShaderConversionPrologue();

            m_derivedShaders[key] = D3D12VertexShader(this);
            D3D12VertexShader& newVertexShader = m_derivedShaders[key];

            auto vsInputDecls = inputLayout.GetVSInputDecls();
            ShaderConv::VSOutputDecls &vsOutputDecl = newVertexShader.m_vsOutputDecls;

            auto convertArgs = ShaderConv::ConvertTLShaderArgs(
                m_parentDevice.GetD3D9ApiVersion(), 
                0,
                vsInputDecls, 
                vsOutputDecl);

            hr = m_parentDevice.m_ShaderConvAPI.ConvertTLShader(convertArgs);
            CHECK_HR(hr);

            if (SUCCEEDED(hr) && convertArgs.convertedByteCode.m_pByteCode)
            {
                SizedBuffer upgradedByteCode = SizedBuffer(convertArgs.convertedByteCode.m_pByteCode, convertArgs.convertedByteCode.m_byteCodeSize);
                hr = SetupInputSignaturesAndGetFinalCode(upgradedByteCode, vsInputDecls, &inputLayout, newVertexShader);
            }

            m_parentDevice.GetDataLogger().AddShaderData(D3D10_SB_VERTEX_SHADER, convertArgs.totalInstructionsEmitted, convertArgs.totalExtraInstructionsEmitted);
            ShaderConv::ShaderConverterAPI::CleanUpConvertedShader(convertArgs.convertedByteCode);

            return newVertexShader;
        }
    }

    HRESULT VertexShader::SetupInputSignaturesAndGetFinalCode(SizedBuffer& upgradedByteCode , ShaderConv::VSInputDecls& vsInputDecls, InputLayout* pInputLayout, _Out_ D3D12VertexShader& outputVS)
    {
        DXBCInputSignatureBuilder inputSignatureBuilder;
        DXBCInputSignatureBuilder outputSignatureBuilder;

        if (vsInputDecls.GetSize() > 0 && pInputLayout)
        {
            // Converting pInputSignature into the proper DXBC format. It must be prepended with a header 
            // and the strings must be serialized
            std::vector<_D3D11_INTERNALSHADER_PARAMETER_11_1> pParameters;
            outputVS.m_inputElementDescs.clear();
            outputVS.m_inputElementDescs.reserve(pInputLayout->GetVertexElementCount());

            // Generate both the descriptors for the D3D12 input layout (D3D12_INPUT_ELEMENT_DESC) and the 
            // descriptors for the DXBC header (_D3D11_INTERNALSHADER_PARAMETER_11_1)
            std::vector<std::string> semanticNameList;

            // The d3d12 runtime expects input signature items to be ordered by register index.
            // However, this code originally produced signatures with out of order registers
            // because of the way FindRegisterIndex works.
            // To solve this we collect up the Parameters, Elements and Semantic names and
            // sort them based on the parameter register.
            typedef std::tuple<_D3D11_INTERNALSHADER_PARAMETER_11_1, D3D12_INPUT_ELEMENT_DESC, const char *> tupleType;
            std::vector<tupleType> pairs;

            for (UINT i = 0; i < pInputLayout->GetVertexElementCount(); i++)
            {
                D3DDDIVERTEXELEMENT &inputDesc = pInputLayout->GetVertexElement(i);
                UINT regIndex = vsInputDecls.FindRegisterIndex(inputDesc.Usage, inputDesc.UsageIndex);

                if (regIndex != ShaderConv::VSInputDecls::INVALID_INDEX)
                {
                    const char *semanticName = ConvertToSemanticNameForInputLayout((D3DDECLUSAGE)inputDesc.Usage);

                    _D3D11_INTERNALSHADER_PARAMETER_11_1 parameter = {};

                    // This Stream refers to StreamOut, and should always be 0 for 9on12. This is different from the
                    // inputDesc.Stream which refers to the VertexBuffer slot
                    parameter.Stream = 0;
                    
                    parameter.SemanticIndex = inputDesc.UsageIndex;
                    parameter.ComponentType = ConvertToRegisterComponentType(static_cast<D3DDECLTYPE>(inputDesc.Type));
                    parameter.Mask = RGBA_MASK;
                    parameter.Register = regIndex;
                    parameter.AlwaysReads_Mask = parameter.Mask;
                    // VS's input system values must always be undefined, these are only meaningful
                    // when passing data to the PS
                    parameter.SystemValue = D3D10_NAME_UNDEFINED; 

                    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
                    inputElementDesc.SemanticName = semanticName;
                    inputElementDesc.SemanticIndex = parameter.SemanticIndex;
                    inputElementDesc.Format = ConvertToDXGIFormat(static_cast<D3DDECLTYPE>(inputDesc.Type));
                    inputElementDesc.InputSlot = inputDesc.Stream;
                    inputElementDesc.AlignedByteOffset = inputDesc.Offset;

                    UINT streamFrequence = m_parentDevice.GetStreamFrequency(inputElementDesc.InputSlot);

                    if (streamFrequence & D3DSTREAMSOURCE_INSTANCEDATA)
                    {
                        inputElementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                        inputElementDesc.InstanceDataStepRate = streamFrequence &~D3DSTREAMSOURCE_INSTANCEDATA;
                    }
                    else
                    {
                        inputElementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                        inputElementDesc.InstanceDataStepRate = 0;
                    }

                    tupleType newPair(parameter, inputElementDesc, semanticName);
                    pairs.push_back(newPair);
                }
            }

            // Registers must appear in increasing order.
            std::sort(pairs.begin(), pairs.end(), less_than_key<tupleType>());

            for (auto& pair : pairs)
            {
                pParameters.push_back(std::get<0>(pair));
                outputVS.m_inputElementDescs.push_back(std::get<1>(pair));
                semanticNameList.push_back(std::get<2>(pair));
            }

            Check9on12(pParameters.size() == semanticNameList.size());
            inputSignatureBuilder.SetParameters(&pParameters[0], &semanticNameList[0], static_cast<UINT>(pParameters.size()));
        }
        else
        {
            inputSignatureBuilder.SetParameters(nullptr, nullptr, 0);
        }

        // Setup the DXBC signature for the VS output
        {
            GenerateSignatureFromVSOutput(outputVS.m_vsOutputDecls, outputSignatureBuilder);
        }

        SizedBuffer inputSignatureBuilderData = inputSignatureBuilder.GetData();
        SizedBuffer outputSignatureBuilderData = outputSignatureBuilder.GetData();

        HRESULT hr = GenerateFinalD3D12Shader(upgradedByteCode, outputVS, inputSignatureBuilderData, outputSignatureBuilderData);
        CHECK_HR(hr);
        return hr;
    }

    HRESULT D3D12Shader::Create(Device &device, std::unique_ptr<BYTE[]> byteCode, SIZE_T bytecodeSize)
    {
        HRESULT hr = S_OK;
        m_pUnderlying = new (m_pUnderlyingSpace) D3D12TranslationLayer::Shader(&device.GetContext(), std::move(byteCode), bytecodeSize);
        if (m_pUnderlying)
        {
            hr = E_OUTOFMEMORY;
        }
        return hr;
    }

    void D3D12Shader::Destroy()
    {
        m_pUnderlying->~Shader();
    }

    HRESULT Shader::GenerateFinalD3D12Shader(SizedBuffer& upgradedByteCode, _Out_ D3D12Shader& d3d12ShaderOut, SizedBuffer& inputSignature, SizedBuffer& outputSignature)
    {
        HRESULT hr = S_OK;

        // Input Signature
        {
            hr = m_DXBCBuilder.AppendBlob(DXBC_InputSignature11_1, static_cast<UINT>(inputSignature.m_size), inputSignature.m_ptr);
        }

        // Output Signature
        if (SUCCEEDED(hr))
        {
            hr = m_DXBCBuilder.AppendBlob(DXBC_OutputSignature11_1, static_cast<UINT>(outputSignature.m_size), outputSignature.m_ptr);
        }

        if (SUCCEEDED(hr))
        {
            hr = m_DXBCBuilder.AppendBlob(DXBC_GenericShader, static_cast<UINT>(upgradedByteCode.m_size), upgradedByteCode.m_ptr);
        }

        if (SUCCEEDED(hr))
        {
            UINT32 combinedLength = 0;

            hr = m_DXBCBuilder.GetFinalDXBC(nullptr, &combinedLength);

            if (SUCCEEDED(hr) && combinedLength)
            {
                std::unique_ptr<BYTE[]> combinedCode(new BYTE[combinedLength]); // throw( bad_alloc )
                hr = m_DXBCBuilder.GetFinalDXBC(combinedCode.get(), &combinedLength);

                if (SUCCEEDED( hr ))
                {
                    hr = SignDxbc( combinedCode.get(), combinedLength );
                }

                if (SUCCEEDED(hr))
                {
                    // In order to point to the raw input and output signatures we must first offset past the
                    // DXBC header and then past the individual Blob headers.
                    DXBCHeader* header = (DXBCHeader*)combinedCode.get();
                    byte* firstBlob = (byte*)combinedCode.get() + sizeof(DXBCHeader) + (header->BlobCount * sizeof(UINT));

                    byte* inputSignaturePointer = firstBlob + sizeof(DXBCBlobHeader);
                    d3d12ShaderOut.m_inputSignature = SizedBuffer(inputSignaturePointer, inputSignature.m_size);

                    byte* outputSignaturePointer = inputSignaturePointer + inputSignature.m_size + sizeof(DXBCBlobHeader);
                    d3d12ShaderOut.m_outputSignature = SizedBuffer(outputSignaturePointer, outputSignature.m_size);

                    d3d12ShaderOut.Create(m_parentDevice, std::move(combinedCode), combinedLength);
                }
            }
        }

        // Start up a new contain if they change up the shader via RasterStates
        m_DXBCBuilder.StartNewContainer();

        CHECK_HR(hr);

        if (RegistryConstants::g_cSpewConvertedShaders)
        {   
            CComPtr<ID3DBlob> debugBlob;
            HRESULT result = DisassembleShader(debugBlob, d3d12ShaderOut.GetUnderlying()->GetByteCode());

            if (SUCCEEDED(result))
            {
                PrintDebugMessage(std::string((char *)debugBlob->GetBufferPointer()));
            }
        }

        if (RegistryConstants::g_cValidateShaders)
        {
            ThrowFailure(ValidateShader(d3d12ShaderOut.GetUnderlying()->GetByteCode()));
        }


        return hr;
    }

    HRESULT Shader::DisassembleShader(CComPtr<ID3DBlob> &pBlob, const D3D12_SHADER_BYTECODE &shaderByteCode)
    {
        typedef HRESULT(WINAPI* D3DDisassemble)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
            _In_ SIZE_T SrcDataSize,
            _In_ UINT Flags,
            _In_opt_ LPCSTR szComments,
            _Out_ ID3DBlob** ppDisassembly);

        HMODULE hCompiler = 0;
        hCompiler = LoadLibraryEx("D3DCompiler_47.dll", NULL, NULL);

        D3DDisassemble disassembleFunction = nullptr;

        if (hCompiler != 0)
        {
            disassembleFunction = (D3DDisassemble)GetProcAddress(hCompiler, "D3DDisassemble");
        }

        if (disassembleFunction == nullptr)
        {
            Check9on12(false);
            return E_FAIL;
        }

        return disassembleFunction(shaderByteCode.pShaderBytecode, shaderByteCode.BytecodeLength, 0, nullptr, &pBlob);
    }


    HRESULT Shader::ShaderConversionPrologue()
    {
        HRESULT result = S_OK;
        if (RegistryConstants::g_cSpewD3D9Shaders && m_d3d9ByteCode.m_size > 0)
        {
            CComPtr<ID3DBlob> debugBlob;
            D3D12_SHADER_BYTECODE d3d9ByteCode;
            d3d9ByteCode.pShaderBytecode = m_d3d9ByteCode.m_ptr;
            d3d9ByteCode.BytecodeLength = m_d3d9ByteCode.m_size;
            result = DisassembleShader(debugBlob, d3d9ByteCode);

            if (SUCCEEDED(result))
            {
                PrintDebugMessage(std::string((char *)debugBlob->GetBufferPointer()));
            }
        }
        return result;
    }

    HRESULT Shader::ValidateShader(const D3D12_SHADER_BYTECODE &shaderByteCode)
    {
        typedef HRESULT(WINAPI* ValidateShader)(CONST BYTE* pShaderCode);

        HMODULE hValidator = 0;
        hValidator = LoadLibraryEx("D3D9on12ShaderValidator.dll", NULL, NULL);

        ValidateShader validateFunction = nullptr;

        if (hValidator != 0)
        {
            validateFunction = (ValidateShader)GetProcAddress(hValidator, "ValidateShader");
        }

        if (validateFunction == nullptr)
        {
            Check9on12(false);
            return E_FAIL;
        }

        return validateFunction((CONST BYTE *)shaderByteCode.pShaderBytecode);
    }

    HRESULT DXBCInputSignatureBuilder::SetParameters(_D3D11_INTERNALSHADER_PARAMETER_11_1 *pInputParameters, std::string *pNameList, UINT numParameters)
    {
        // TODO: doesn't optimize for duplicate strings
        UINT charCacheSize = 0;

        if (pNameList != nullptr)
        {
            for (UINT i = 0; i < numParameters; i++)
            {
                charCacheSize += static_cast<UINT>(pNameList[i].size()) + 1;
            }
        }

        m_dataSize = sizeof(D3D10_INTERNALSHADER_SIGNATURE) + sizeof(_D3D11_INTERNALSHADER_PARAMETER_11_1) * numParameters + charCacheSize;
        m_pData = std::unique_ptr<byte[]>(new byte[m_dataSize]);
        if (m_pData.get() == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        D3D10_INTERNALSHADER_SIGNATURE *pHeader = (D3D10_INTERNALSHADER_SIGNATURE*)m_pData.get();
        _D3D11_INTERNALSHADER_PARAMETER_11_1 *pParameters = (_D3D11_INTERNALSHADER_PARAMETER_11_1 *)(pHeader + 1);
        char *pCharCache = (char *)(pParameters + numParameters);

        pHeader->Parameters = numParameters;
        pHeader->ParameterInfo = sizeof(D3D10_INTERNALSHADER_SIGNATURE);

        if (pNameList != nullptr)
        {
            for (UINT i = 0; i < numParameters; i++)
            {
                pParameters[i] = pInputParameters[i];

                // Serialize the string in the charCache and save the string offset
                const char *pSemanticName = pNameList[i].c_str();
                const size_t bufferSizeRemaining = (size_t)(m_pData.get() + m_dataSize) - (size_t)pCharCache;

                strcpy_s(pCharCache, bufferSizeRemaining, pSemanticName);
                pParameters[i].SemanticName = (UINT)((BYTE *)pCharCache - (BYTE *)pHeader);
                pCharCache += strlen(pSemanticName) + 1;
            }
        }

        return S_OK;
    }
};