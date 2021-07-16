// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#include <DxbcBuilder.hpp>

typedef struct _D3D10_INTERNALSHADER_SIGNATURE
{
    UINT Parameters;      // Number of parameters
    UINT ParameterInfo;   // Offset to D3D10_INTERNALSHADER_PARAMETER[Parameters]
} D3D10_INTERNALSHADER_SIGNATURE, *LPD3D10_INTERNALSHADER_SIGNATURE;

typedef struct _D3D11_INTERNALSHADER_PARAMETER_11_1
{
    UINT Stream;                                    // Stream index (parameters must appear in non-decreasing stream order)
    UINT SemanticName;                              // Offset to LPCSTR
    UINT SemanticIndex;                             // Semantic Index
    D3D10_NAME SystemValue;                         // Internally defined enumeration
    D3D10_REGISTER_COMPONENT_TYPE  ComponentType;   // Type of  of bits
    UINT Register;                                  // Register Index
    BYTE Mask;                                      // Combination of D3D10_COMPONENT_MASK values

    // The following unioned fields, NeverWrites_Mask and AlwaysReads_Mask, are exclusively used for 
    // output signatures or input signatures, respectively.
    //
    // For an output signature, NeverWrites_Mask indicates that the shader the signature belongs to never 
    // writes to the masked components of the output register.  Meaningful bits are the ones set in Mask above.
    //
    // For an input signature, AlwaysReads_Mask indicates that the shader the signature belongs to always
    // reads the masked components of the input register.  Meaningful bits are the ones set in the Mask above.
    //
    // This allows many shaders to share similar signatures even though some of them may not happen to use
    // all of the inputs/outputs - something which may not be obvious when authored.  The NeverWrites_Mask
    // and AlwaysReads_Mask can be checked in a debug layer at runtime for the one interesting case: that a 
    // shader that always reads a value is fed by a shader that always writes it.  Cases where shaders may
    // read values or may not cannot be validated unfortunately.  
    //
    // In scenarios where a signature is being passed around standalone (so it isn't tied to input or output 
    // of a given shader), this union can be zeroed out in the absence of more information.  This effectively
    // forces off linkage validation errors with the signature, since if interpreted as a input or output signature
    // somehow, since the meaning on output would be "everything is always written" and on input it would be 
    // "nothing is always read".
    union
    {
        BYTE NeverWrites_Mask;  // For an output signature, the shader the signature belongs to never 
                                // writes the masked components of the output register.
        BYTE AlwaysReads_Mask;  // For an input signature, the shader the signature belongs to always
                                // reads the masked components of the input register.
    };

    D3D_MIN_PRECISION MinPrecision;                 // Minimum precision of input/output data
} D3D11_INTERNALSHADER_PARAMETER_11_1, *LPD3D11_INTERNALSHADER_PARAMETER_11_1;

namespace D3D9on12
{
    struct SizedBuffer
    {
        SizedBuffer() : m_ptr(nullptr), m_size(0) {};
        SizedBuffer(byte* ptr, size_t size) : m_ptr(ptr), m_size(size) {};
        SizedBuffer(void* ptr, size_t size) : m_ptr((byte*)ptr), m_size(size) {};

        byte* m_ptr;
        size_t m_size;
    };

    class InputLayout;
    class DXBCInputSignatureBuilder
    {
    public:
        DXBCInputSignatureBuilder() : m_dataSize(0) {}

        // Semantic Names are passed in separately in pNameList so that they can be serialized, therefore the
        // SemanticName field in pInputParameters can be left uninitialized
        HRESULT SetParameters(_D3D11_INTERNALSHADER_PARAMETER_11_1 *pInputParameters, std::string *pNameList, UINT numParameters);

        // Data valid until DXBCInputSignatureBuilder goes out of scope;
        SizedBuffer GetData() { return SizedBuffer(m_pData.get(), m_dataSize);}

    private:
        std::vector<_D3D11_INTERNALSHADER_PARAMETER_11_1> m_pParameters;
        std::unique_ptr<byte[]> m_pData;
        UINT m_dataSize;
    };

    class Shader;
    struct D3D12Shader
    {
        D3D12Shader(Shader *pD3D9ParentShader) : 
            m_pD3D9ParentShader(pD3D9ParentShader), m_pUnderlying(nullptr), m_floatConstsUsed(0), m_intConstsUsed(0), m_boolConstsUsed(0){};
        // These point to their corresponding positions in the 
        // DXBC blob preamble of the m_byteCode member

        HRESULT Create(Device &device, std::unique_ptr<BYTE[]> byteCode, SIZE_T bytecodeSize);
        void Destroy();
        SizedBuffer m_inputSignature;
        SizedBuffer m_outputSignature;

        _Out_ ShaderConv::ShaderConsts m_inlineConsts[3];

        UINT m_floatConstsUsed;
        UINT m_intConstsUsed;
        UINT m_boolConstsUsed;

        D3D12TranslationLayer::Shader* GetUnderlying() { return m_pUnderlying; }
        Shader *GetD3D9ParentShader() const { return m_pD3D9ParentShader; }
    protected:
        Shader *m_pD3D9ParentShader;
        byte m_pUnderlyingSpace[sizeof(D3D12TranslationLayer::Shader)];
        D3D12TranslationLayer::Shader *m_pUnderlying;
    };

    struct D3D12VertexShader : public D3D12Shader
    {
        D3D12VertexShader(Shader *pD3D9ParentShader = nullptr) : m_vsOutputDecls(), D3D12Shader(pD3D9ParentShader){};

        std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs;
        ShaderConv::VSOutputDecls m_vsOutputDecls;
    };

    struct D3D12GeometryShader : public D3D12Shader
    { 
        D3D12GeometryShader(Shader *pD3D9ParentShader = nullptr) : D3D12Shader(pD3D9ParentShader){}; 

        ShaderConv::VSOutputDecls m_gsOutputDecls;
    };

    struct D3D12PixelShader : public D3D12Shader
    { 
        D3D12PixelShader(Shader *pD3D9ParentShader = nullptr) : D3D12Shader(pD3D9ParentShader){}; 
    };

    class Shader : public PipelineStateCacheKeyComponent
    {
    public:
        Shader(Device& device);
        ~Shader();

        HRESULT Init(_In_ CONST byte& byteCode, _In_ size_t byteCodeLength);

        static FORCEINLINE HANDLE GetHandleFromShader(Shader* pShader){ return static_cast<HANDLE>(pShader); }
        static FORCEINLINE Shader* GetShaderFromHandle(HANDLE hShader){ return static_cast<Shader*>(hShader); }

        WeakHash GetHashForLegacyByteCode() { return m_legacyCodeHash; }

        HRESULT ShaderConversionPrologue();
    protected:
        static void GenerateSignatureFromVSOutput(ShaderConv::VSOutputDecls& vsOut, DXBCInputSignatureBuilder& signature);

        HRESULT GenerateFinalD3D12Shader(SizedBuffer& upgradedByteCode, _Out_ D3D12Shader& d3d12ShaderOut, SizedBuffer& inputSignature, SizedBuffer& outputSignature);

        static HRESULT DisassembleShader(CComPtr<ID3DBlob> &pBlob, const D3D12_SHADER_BYTECODE &shaderByteCode);
        static HRESULT ValidateShader(const D3D12_SHADER_BYTECODE &shaderByteCode);

        SizedBuffer m_d3d9ByteCode;

        WeakHash m_legacyCodeHash;

        CDXBCBuilder m_DXBCBuilder;

        Device& m_parentDevice;

        // TODO: The map looks ups require us to make deep copies of the state. Technically
        //       we only need references to these objects during a map.find(key) (so that it 
        //       can do comparisons during a hash collision). If this path turns out to be hot 
        //       we should rethink this and possibly use our own map implementation.
        struct DerivedShaderKey
        {
            DerivedShaderKey(const ShaderConv::RasterStates& rasterStates) : m_hash(0)
            {
                //memcpy because assignment can add alignment which can throw off hashing
                memcpy(&m_rasterStates, &rasterStates, sizeof(m_rasterStates));
            };

            ShaderConv::RasterStates m_rasterStates;
            size_t m_hash;

            template<typename KeyType>
            struct Hasher
            {
                std::size_t operator()(KeyType const& other) const
                {
                    return other.m_hash;
                }
            };
        };
    };

    class VertexShader : public Shader
    {
    public:

        VertexShader(Device& parentDevice);
        ~VertexShader();

        D3D12VertexShader& GetD3D12Shader(const ShaderConv::RasterStates &rasterStates, InputLayout& inputLayout);

        // Get the shader for pre-Transformed and Lit vertices (essentially a pass through).
        D3D12VertexShader& GetD3D12ShaderForTL(InputLayout& inputLayout, const ShaderConv::RasterStates &rasterStates);

    private:

        HRESULT SetupInputSignaturesAndGetFinalCode(SizedBuffer& upgradedByteCode, ShaderConv::VSInputDecls& vsInputDecls, InputLayout* pInputLayout, _Out_ D3D12VertexShader& outputVS);

        struct DerivedVertexShaderKey : public DerivedShaderKey
        {
            // Hash in the constructor so that his key can be used several times efficiently
            DerivedVertexShaderKey(const ShaderConv::RasterStates& rasterStates, InputLayout& inputLayout, WeakHash legacyByteCodeHash, _In_reads_(MAX_VERTEX_STREAMS) UINT* streamFrequencies) : DerivedShaderKey(rasterStates)
            {
                //memcpy because assignment can add alignment which can throw off hashing
                memcpy(&m_inputLayoutHash, &inputLayout.GetHash(), sizeof(m_inputLayoutHash));
                memcpy(&m_legacyByteCodeHash, &legacyByteCodeHash, sizeof(m_legacyByteCodeHash));
                memcpy(m_streamFrequencies, streamFrequencies, sizeof(m_streamFrequencies));

                WeakHash hash = HashData(&m_rasterStates, sizeof(m_rasterStates), m_inputLayoutHash);//Add the hash from the IL
                hash = HashData(&m_legacyByteCodeHash, sizeof(m_legacyByteCodeHash), hash);
                hash = HashData(streamFrequencies, sizeof(streamFrequencies), hash);
                m_hash = size_t(hash.m_data);
            };

            //Needs to be a deep copy as it's essentially a snapshot of the state at the time
            WeakHash m_inputLayoutHash;
            WeakHash m_legacyByteCodeHash;
            UINT     m_streamFrequencies[MAX_VERTEX_STREAMS];

            struct Comparator
            {
                bool operator()(const DerivedVertexShaderKey& a, const DerivedVertexShaderKey& b) const
                {
                    return memcmp(&a.m_rasterStates, &b.m_rasterStates, sizeof(a.m_rasterStates)) == 0 &&
                        memcmp(&a.m_streamFrequencies, &b.m_streamFrequencies, sizeof(a.m_streamFrequencies)) == 0 &&
                        a.m_inputLayoutHash == b.m_inputLayoutHash &&
                        a.m_legacyByteCodeHash == b.m_legacyByteCodeHash;
                }
            };
        };

        typedef std::unordered_map<DerivedVertexShaderKey, D3D12VertexShader, DerivedShaderKey::Hasher<DerivedVertexShaderKey>, DerivedVertexShaderKey::Comparator> MapType;
        MapType m_derivedShaders;
    };

    class GeometryShader : public Shader
    {
    public:
        GeometryShader(Device& parentDevice);
        ~GeometryShader();

        D3D12GeometryShader& GetD3D12Shader(D3D12VertexShader& currentVS, const ShaderConv::RasterStates& rasterStates);

    private:

        struct DerivedGeometryShaderKey : public DerivedShaderKey
        {
            // Hash in the constructor so that his key can be used several times efficiently
            DerivedGeometryShaderKey(const ShaderConv::RasterStates& rasterStates, ShaderConv::VSOutputDecls& vsOutDecls) : DerivedShaderKey(rasterStates)
            {
                //memcpy because assignment can add alignment which can throw off hashing
                memcpy(&m_vsOutDecls, &vsOutDecls, sizeof(m_vsOutDecls));

                WeakHash hash = HashData(&m_rasterStates, sizeof(m_rasterStates));
                hash = HashData(&m_vsOutDecls[0], m_vsOutDecls.GetSize() * sizeof(m_vsOutDecls[0]), hash);
                m_hash = size_t(hash.m_data);
            };

            //Needs to be a deep copy as it's essentially a snapshot of the state at the time
            ShaderConv::VSOutputDecls m_vsOutDecls;

            struct Comparator
            {
                bool operator()(const DerivedGeometryShaderKey& a, const DerivedGeometryShaderKey& b) const
                {
                    if (a.m_vsOutDecls.GetSize() != b.m_vsOutDecls.GetSize()) { return false; }

                    for (UINT i = 0; i < a.m_vsOutDecls.GetSize(); i++)
                    {
                        if (memcmp(&a.m_vsOutDecls[i], &b.m_vsOutDecls[i], sizeof(a.m_vsOutDecls[i])) != 0) { return false; }
                    }
                    return memcmp(&a.m_rasterStates, &b.m_rasterStates, sizeof(a.m_rasterStates)) == 0;
                }
            };
        };

        typedef std::unordered_map<DerivedGeometryShaderKey, D3D12GeometryShader, DerivedShaderKey::Hasher<DerivedGeometryShaderKey>, DerivedGeometryShaderKey::Comparator> MapType;
        MapType m_derivedShaders;
    };

    class PixelShader : public Shader
    {
    public:
        PixelShader(Device& parentDevice);
        ~PixelShader();

        D3D12PixelShader& GetD3D12Shader(const ShaderConv::RasterStates &rasterStates, ShaderConv::VSOutputDecls& vsOutputDecls, D3D12Shader& inputShader);

    private:

        struct DerivedPixelShaderKey : public DerivedShaderKey
        {
            // Hash in the constructor so that his key can be used several times efficiently
            DerivedPixelShaderKey(const ShaderConv::RasterStates& rasterStates, ShaderConv::VSOutputDecls& vsOutDecls, WeakHash legacyByteCodeHash) : DerivedShaderKey(rasterStates)
            {
                //memcpy because assignment can add alignment which can throw off hashing
                memcpy(&m_vsOutDecls, &vsOutDecls, sizeof(m_vsOutDecls));
                memcpy(&m_legacyByteCodeHash, &legacyByteCodeHash, sizeof(m_legacyByteCodeHash));

                WeakHash hash = HashData(&m_rasterStates, sizeof(m_rasterStates), m_legacyByteCodeHash);
                hash = HashData(&m_vsOutDecls[0], m_vsOutDecls.GetSize() * sizeof(m_vsOutDecls[0]), hash);
                m_hash = size_t(hash.m_data);
            };

            //Needs to be a deep copy as it's essentially a snapshot of the state at the time
            ShaderConv::VSOutputDecls m_vsOutDecls;
            WeakHash m_legacyByteCodeHash;

            struct Comparator
            {
                bool operator()(const DerivedPixelShaderKey& a, const DerivedPixelShaderKey& b) const
                {
                    if (a.m_vsOutDecls.GetSize() != b.m_vsOutDecls.GetSize()) { return false; }

                    for (UINT i = 0; i < a.m_vsOutDecls.GetSize(); i++)
                    {
                        if (memcmp(&a.m_vsOutDecls[i], &b.m_vsOutDecls[i], sizeof(a.m_vsOutDecls[i])) != 0) { return false; }
                    }

                    return memcmp(&a.m_rasterStates, &b.m_rasterStates, sizeof(a.m_rasterStates)) == 0 &&
                        a.m_legacyByteCodeHash == b.m_legacyByteCodeHash;
                }
            };
        };

        typedef std::unordered_map<DerivedPixelShaderKey, D3D12PixelShader, DerivedShaderKey::Hasher<DerivedPixelShaderKey>, DerivedPixelShaderKey::Comparator> MapType;
        MapType m_derivedShaders;
    };

    template<typename tupleType>
    struct less_than_key
    {
        inline bool operator() (const tupleType& struct1, const tupleType& struct2)
        {
            return (std::get<0>(struct1).Register < std::get<0>(struct2).Register);
        }
    };
};