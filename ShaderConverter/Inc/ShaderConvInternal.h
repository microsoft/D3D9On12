// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "ShaderConv.h"
#include <vector>

namespace ShaderConv
{
    class IObject
    {
    public:

        virtual LONG AddRef() const = 0;
        virtual LONG Release() const = 0;

    protected:

        IObject() {}
        virtual ~IObject() {}
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    class CObject : public IObject
    {
    public:

        LONG AddRef() const;
        LONG Release() const;

    protected:

        CObject() : m_lRefCount(0) {}

        virtual ~CObject() {}

        mutable LONG m_lRefCount;
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    class CShaderDesc : public CObject
    {
    public:
        UINT GetShaderSettings() const
        {
            return m_shaderSettings;
        }

        UINT GetVersion() const
        {
            return m_version;
        }

        UINT GetMinUsedConsts(eConstantBuffers type) const
        {
            return m_minUsedConsts[type];
        }

        UINT GetMaxUsedConsts(eConstantBuffers type) const
        {
            return m_maxUsedConsts[type];
        }

        bool HasRelAddrConsts(eConstantBuffers type) const
        {
            return m_bRelAddrConsts[type];
        }

        const InputRegs& GetInputRegs() const
        {
            return m_inputRegs;
        }

        const OutputRegs& GetOutputRegs() const
        {
            return m_outputRegs;
        }

        const DWORD* GetInstructions() const
        {
            return m_pdwInstrs;
        }

        UINT GetCodeSize() const
        {
            return m_cbCodeSize;
        }

        BYTE GetNumLoopRegs() const
        {
            return m_numLoopRegs;
        }

        BYTE GetNumTempRegs() const
        {
            return m_numTempRegs;
        }

        const ShaderConsts& GetInlineConstants(eConstantBuffers type) const
        {
            return m_inlineConsts[type];
        }

        ShaderConsts&& MoveInlineConstants(eConstantBuffers type)
        {
            return std::move(m_inlineConsts[type]);
        }

        bool FindInlineConstant(eConstantBuffers type, UINT regIndex, ShaderConst* pShaderConst = NULL) const;

    protected:

        CShaderDesc() : m_pdwInstrs(NULL),
            m_cbCodeSize(0),
            m_numLoopRegs(0),
            m_numTempRegs(0),
            m_version(0)
        {
            for (UINT i = 0; i < _countof(m_minUsedConsts); ++i)
            {
                m_minUsedConsts[i] = 0xFFFFFFFF;
                m_maxUsedConsts[i] = 0;
                m_bRelAddrConsts[i] = false;
            }
        }

        virtual ~CShaderDesc()
        {
            if (m_pdwInstrs)
            {
                delete(m_pdwInstrs);
            }
        }

        void SetShaderSettings(UINT shaderSettings)
        {
            m_shaderSettings = shaderSettings;
        }

        void SetVersion(UINT version)
        {
            m_version = version;
        }

        void SetMinUsedConsts(eConstantBuffers type, UINT value)
        {
            m_minUsedConsts[type] = value;
        }

        void SetMaxUsedConsts(eConstantBuffers type, UINT value)
        {
            m_maxUsedConsts[type] = value;
        }

        void SetRelAddrConsts(eConstantBuffers type, bool bValue)
        {
            m_bRelAddrConsts[type] = bValue;
        }

        HRESULT AddInlineConstsF(UINT regIndex, const FLOAT value[4])
        {
            m_inlineConsts[CB_FLOAT].push_back(ShaderConst(regIndex * 4, value));
            return S_OK;
        }

        HRESULT AddInlineConstsI(UINT regIndex, const INT value[4])
        {
            m_inlineConsts[CB_INT].push_back(ShaderConst(regIndex * 4, value));
            return S_OK;
        }

        HRESULT AddInlineConstsB(UINT regIndex, const BOOL value)
        {
            m_inlineConsts[CB_BOOL].push_back(ShaderConst(regIndex, value));
            return S_OK;
        }

        void SetInputRegs(const InputRegs& inputRegs)
        {
            m_inputRegs = inputRegs;
        }

        void SetOutputRegs(const OutputRegs& outputRegs)
        {
            m_outputRegs = outputRegs;
        }

        void SetNumTempRegs(BYTE numTempRegs)
        {
            m_numTempRegs = numTempRegs;
        }

        void SetNumLoopRegs(BYTE numLoopRegs)
        {
            m_numLoopRegs = numLoopRegs;
        }

        HRESULT CopyInstructions(const void* pInstrs, UINT cbSize);

        ShaderConsts m_inlineConsts[3];

        UINT m_minUsedConsts[3];
        UINT m_maxUsedConsts[3];
        bool m_bRelAddrConsts[3];

        InputRegs  m_inputRegs;
        OutputRegs m_outputRegs;

        DWORD* m_pdwInstrs;
        UINT   m_cbCodeSize;

        BYTE m_numLoopRegs;
        BYTE m_numTempRegs;

        UINT m_version;
        UINT m_shaderSettings;

        friend class CTranslator;
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    class CVertexShaderDesc : public CShaderDesc
    {
    public:

        const VSInputDecls& GetInputDecls() const
        {
            return m_inputDecls;
        }

        const VSOutputDecls& GetOutputDecls() const
        {
            return m_outputDecls;
        }

        VSUsageFlags GetUsageFlags() const
        {
            return m_usageFlags;
        }

    protected:

        CVertexShaderDesc() : m_inputDecls(MAX_VS_INPUT_REGS) {}
        virtual ~CVertexShaderDesc() {}

        static HRESULT Create(CVertexShaderDesc** ppVertexShaderDesc);

        void SetInputDecls(const VSInputDecls& inputDecls)
        {
            m_inputDecls = inputDecls;
        }

        void SetOutputDecls(const VSOutputDecls& outputDecls)
        {
            m_outputDecls = outputDecls;
        }

        void SetUsageFlags(const VSUsageFlags& usageFlags)
        {
            m_usageFlags = usageFlags;
        }

        VSOutputDecls m_outputDecls;
        VSInputDecls  m_inputDecls;
        VSUsageFlags  m_usageFlags;

        friend class CTranslator;
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    class CTLVertexShaderDesc : public CObject
    {
    public:

        CTLVertexShaderDesc() : m_vsInputDecls(MAX_VS_INPUT_REGS) {}

        CTLVertexShaderDesc(const VSInputDecls& vsInputDecls, UINT shaderSettings);

        virtual ~CTLVertexShaderDesc() {}

        UINT GetShaderSettings() const 
        { 
            return m_shaderSettings; 
        }

        const VSInputDecls& GetInputDecls() const
        {
            return m_vsInputDecls;
        }

        const VSOutputDecls& GetOutputDecls() const
        {
            return m_vsOutputDecls;
        }

    protected:
        UINT m_shaderSettings;
        VSInputDecls  m_vsInputDecls;
        VSOutputDecls m_vsOutputDecls;
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    class CPixelShaderDesc : public CShaderDesc
    {
    public:
        const VSOutputDecls& GetInputDecls() const
        {
            return m_inputDecls;
        }

        const PSUsageFlags& GetUsageFlags() const
        {
            return m_usageFlags;
        }

        UINT8 GetOutputRegistersMask() const
        {
            return m_outputRegistersMask;
        }

        void AddOutputDepthRegister()
        {
            m_outputRegistersMask |= DEPTH_OUTPUT_MASK;
        }

        void AddOutputRegisters(UINT registerIndex)
        {
            m_outputRegistersMask |= (1 << registerIndex);
        }

        HRESULT UpdateInputDecls(const VSOutputDecls& vsOuputDecls,
            const ShaderConv::RasterStates& rasterStates,
            VSOutputDecls* pMergedDecls);

        BYTE GetPositionRegister() const { return m_positionRegister; }

    protected:

        CPixelShaderDesc() : m_outputRegistersMask(0), m_positionRegister(0) {}
        virtual ~CPixelShaderDesc() {}

        static HRESULT Create(CPixelShaderDesc** ppPixelShaderDesc);

        void SetUsageFlags(const PSUsageFlags& usageFlags)
        {
            m_usageFlags = usageFlags;
        }

        void SetInputDecls(const VSOutputDecls& inputDecls)
        {
            m_inputDecls = inputDecls;
        }

        void SetPositionRegister(BYTE positionRegister) { m_positionRegister = positionRegister; }

        UINT8 m_outputRegistersMask;
        BYTE m_positionRegister;
        VSOutputDecls m_inputDecls;
        PSUsageFlags  m_usageFlags;

        friend class CTranslator;
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    struct GSFLAGS
    {
        union
        {
            UINT Value;
            struct
            {
                UINT PrimitiveSize : 2;
                UINT PointFill : 1;
                UINT FlatColorFill : 1;
                UINT PointSize : 1;
                UINT PointSprite : 1;
                UINT TextureWrap : 1;
                UINT HasTLVertices : 1;
                UINT UserClipPlanes : MAX_CLIPLANES;
            };
        };

        GSFLAGS() : Value(0)
        {
            C_ASSERT(sizeof(GSFLAGS) == sizeof(Value));
        }
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    class CGeometryShaderDesc : public CObject
    {
    public:

        CGeometryShaderDesc()
        {
            this->Clear();
        }

        CGeometryShaderDesc(UINT runtimeVersion,
            UINT shaderSettings,
            const VSOutputDecls& vsOutputDecls,
            const RasterStates& rasterStates);

        virtual ~CGeometryShaderDesc() {}

        void Clear()
        {
            memset(m_textureWraps, 0, sizeof(m_textureWraps));
            m_flags.Value = 0;
        }

        bool IsValid() const
        {
            return (m_flags.Value != 0);
        }

        UINT GetShaderSettings() const 
        {
            return m_shaderSettings;
        };

        UINT GetPrimitiveSize() const
        {
            return m_flags.PrimitiveSize;
        }

        const VSOutputDecls& GetInputDecls() const
        {
            return m_inputDecls;
        }

        const VSOutputDecls& GetOutputDecls() const
        {
            return m_outputDecls;
        }

        bool PointFillEnable() const
        {
            return (m_flags.PointFill != 0);
        }

        bool FlatColorFillEnable() const
        {
            return (m_flags.FlatColorFill != 0);
        }

        bool PointSizeEnable() const
        {
            return (m_flags.PointSize != 0);
        }

        bool PointSpriteEnable() const
        {
            return (m_flags.PointSprite != 0);
        }

        bool TextureWrapEnable() const
        {
            return (m_flags.TextureWrap != 0);
        }

        bool HasTLVertices() const
        {
            return (m_flags.HasTLVertices != 0);
        }

        UINT GetUserClipPlanes() const
        {
            return m_flags.UserClipPlanes;
        }

        UINT GetTextureWrap(UINT uiTexCoordIndex) const
        {
            return m_textureWraps[uiTexCoordIndex];
        }

        UINT GetFlagsID() const
        {
            return m_flags.Value;
        }

        bool operator==(const CGeometryShaderDesc& rhs) const
        {
            return (m_flags.Value == rhs.m_flags.Value)
                && (m_inputDecls == rhs.m_inputDecls)
                && (0 == memcmp(m_textureWraps, rhs.m_textureWraps, sizeof(m_textureWraps)));
        }

        bool operator!=(const CGeometryShaderDesc& rhs) const
        {
            return !(*this == rhs);
        }

    private:

        VSOutputDecls m_inputDecls;
        VSOutputDecls m_outputDecls;
        BYTE          m_textureWraps[MAX_PS_SAMPLER_REGS];
        GSFLAGS       m_flags;
        UINT m_shaderSettings;
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    class CCodeBlob : public CObject
    {
    public:

        static HRESULT Create(size_t cbSize, const void* pBuffer, CCodeBlob** ppCodeBlob);

        void* GetBufferPointer() const
        {
            return m_pBits;
        }

        size_t GetBufferSize() const
        {
            return m_cbSize;
        }

    protected:

        CCodeBlob() : m_cbSize(0),
            m_pBits(NULL) {}

        virtual ~CCodeBlob()
        {
            if (m_pBits) { delete(m_pBits); }
        }

        size_t m_cbSize;
        LPVOID m_pBits;
    };

    ///---------------------------------------------------------------------------
    /// <summary>
    /// </summary>
    ///---------------------------------------------------------------------------
    class ITranslator : public CObject
    {
    public:

        virtual HRESULT AnalyzeVS(const void* pSrcBytes,
            UINT cbCodeSize,
            UINT shaderSettings,
            const RasterStates& rasterStates,
            const VSInputDecls *pReferenceInputDecls,
            CVertexShaderDesc** ppShaderDesc) = 0;

        virtual HRESULT AnalyzePS(const void* pSrcBytes,
            UINT cbCodeSize,
            UINT shaderSettings,
            const RasterStates& rasterStates,
            CPixelShaderDesc** ppShaderDesc) = 0;

        virtual HRESULT TranslateVS(const CVertexShaderDesc* pShaderDesc,
            const RasterStates& rasterStates,
            CCodeBlob** ppCodeBlob) = 0;

        virtual HRESULT TranslateTLVS(const CTLVertexShaderDesc* pShaderDesc,
            CCodeBlob** ppCodeBlob) = 0;

        virtual HRESULT TranslatePS(const CPixelShaderDesc* pShaderDesc,
            const RasterStates& rasterStates,
            const ShaderConv::VSOutputDecls& inputDecls,
            CCodeBlob** ppCodeBlob) = 0;

        virtual HRESULT TranslateGS(const CGeometryShaderDesc* pShaderDesc,
            CCodeBlob** ppCodeBlob) = 0;

        struct TranslationData
        {
            UINT NumInstructionsEmitted;
            UINT NumExtraInstructionsEmitted;
        };

        virtual TranslationData GetTranslationData() = 0;
    protected:

        ITranslator() {}
        virtual ~ITranslator() {}
    };


    TEXTURETYPE ToTextureType(D3D10DDIRESOURCE_TYPE resourceType);

    HRESULT CreateTranslator(UINT runtimeVersion, ITranslator** ppTranslator);
};