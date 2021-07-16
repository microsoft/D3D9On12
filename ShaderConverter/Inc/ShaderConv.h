// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Header Definitions for the Shader Translator
*
****************************************************************************/
#pragma once
namespace ShaderConv
{

#define Check(a) {if(!(a)) {DebugBreak();}}

#define D3DDECLUSAGE_VFACE (D3DDECLUSAGE_SAMPLE+1)
#define D3DDECLUSAGE_VPOS  (D3DDECLUSAGE_VFACE+1)
#define D3DDECLUSAGE_CLIPDISTANCE  (D3DDECLUSAGE_VPOS+1)
#define D3DDECLUSAGE_POINTSPRITE  (D3DDECLUSAGE_CLIPDISTANCE+1)

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
enum eConstants
{
    //--
    MAX_CLIPLANES       = 6,
    MAX_TEXCOORDS       = 8,

    // VS input registers
    MAX_VS_INPUT_REGS   = 16,
    MAX_VS_TEMP_REGS    = 32,

    // VS output registers
    MAX_VS_OUTPUT_REGS  = 12,
    MAX_VS_COLOR_REGS   = 2,    // vs 1.x & 2.0
    MAX_VS_TEXCOORD_REGS= MAX_TEXCOORDS,    // vs 1.x & 2.0
    MAX_VS_SAMPLER_REGS = 4,    // vs 3.0

    // PS input registers
    MAX_PS_INPUT_REGS   = 10,    // ps 3.0
    MAX_PS_COLORIN_REGS = 2,    // ps 2.0
    MAX_PS_TEXCOORD_REGS= MAX_TEXCOORDS,    // ps 2.0
    MAX_PS_TEMP_REGS    = 32,
    MAX_PS1X_TEXTURE_REGS=4,    // ps 1.x
    MAX_PS1X_SAMPLER_REGS=8,    // ps 1.x
    MAX_PS_SAMPLER_REGS = 16,   // ps 2.0+

    // PS output registers
    MAX_PS_COLOROUT_REGS= 4,
    MAX_PS_DEPTHOUT_REGS= 1,    // ps 2.0+

    DEPTH_OUTPUT_MASK = 1 << MAX_PS_COLOROUT_REGS,

    // Common registers
    MAX_INPUT_REGS      = ( MAX_VS_INPUT_REGS > MAX_PS_INPUT_REGS ) ? MAX_VS_INPUT_REGS : MAX_PS_INPUT_REGS,
    MAX_TEMP_REGS       = ( MAX_VS_TEMP_REGS > MAX_PS_TEMP_REGS ) ? MAX_VS_TEMP_REGS : MAX_PS_TEMP_REGS,
    MAX_SAMPLER_REGS    = ( MAX_VS_SAMPLER_REGS > MAX_PS_SAMPLER_REGS ) ? MAX_VS_SAMPLER_REGS : MAX_PS_SAMPLER_REGS,

    //--
    INVALID_INDEX       = 0xff,

    //--
    __D3DSP_WRITEMASK_MASK  = 0x000F0000,
    __D3DSP_WRITEMASK_SHIFT = 16,

    //--
    MAX_VS_CONSTANTSF    = 256,
    MAX_VS_CONSTANTSI    = 16,
    MAX_VS_CONSTANTSB    = 16,

    //--
    MAX_PS_CONSTANTSF    = 224,
    MAX_PS_CONSTANTSI    = 16,
    MAX_PS_CONSTANTSB    = 16,

    //--
    TCIMASK_PASSTHRU     = 0x76543210,

};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct VSCBExtension
{
    enum
    {
        VIEWPORTSCALE = 0,
        POINTSIZE     = 1,
        CLIPPLANE0    = 2,
        SCREENTOCLIPOFFSET = 8,
        SCREENTOCLIPSCALE = 9,
    };

    FLOAT vViewPortScale[4]; // { 1/vw, -1/vh, 0, 0 }
    FLOAT vPointSize[4];     // { psize, min, max, 0 }
    FLOAT vClipPlanes[MAX_CLIPLANES][4];
    FLOAT vScreenToClipOffset[4];
    FLOAT vScreenToClipScale[4];
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct GSCBExtension
{
    enum
    {
        POINTSIZE = 0,
    };

    FLOAT vPointSize[4]; // { psize, min, max, 0 }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct PSCBExtension
{
    enum
    {
        FOGCOLOR_XYZ,
        ALPHATEST_W  = FOGCOLOR_XYZ,
        FOGSTART_X,
        FOGEND_Y     = FOGSTART_X,
        FOGDISTINV_Z = FOGSTART_X,
        FOGDENSITY_W = FOGSTART_X,
    };

    FLOAT   vFogColor[3];
    FLOAT   fAlphaRef;

    FLOAT   fFogStart;
    FLOAT   fFogEnd;
    FLOAT   fFogDistInv;
    FLOAT   fFogDensity;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct PSCBExtension2
{
    enum
    {
        BUMPENVMAT0,
        BUMPENVMAT1,
        BUMPENVMAT2,
        BUMPENVMAT3,
        BUMPENVMAT4,
        BUMPENVMAT5,
        BUMPENVMAT6,
        BUMPENVMAT7,
        BUMPENVL0,
        BUMPENVL1,
        BUMPENVL2,
        BUMPENVL3,
        BUMPENVL4,
        BUMPENVL5,
        BUMPENVL6,
        BUMPENVL7,
    };

    FLOAT vBumpEnvMat[8][4];
    FLOAT vBumpEnvL[8][4];
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct PSCBExtension3
{
    enum
    {
        COLORKEY0,
        COLORKEY1,
        COLORKEY2,
        COLORKEY3,
        COLORKEY4,
        COLORKEY5,
        COLORKEY6,
        COLORKEY7,
    };

    FLOAT vColorKey[8][4];
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
enum eConstantBuffers
{
    CB_FLOAT    = 0,
    CB_INT      = 1,
    CB_BOOL     = 2,
    CB_VS_EXT   = 3,
    CB_PS_EXT   = 3,
    CB_PS_EXT2  = 4,
    CB_PS_EXT3  = 5,
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
enum TEXTURETYPE
{
    TEXTURETYPE_UNKNOWN,
    TEXTURETYPE_2D,
    TEXTURETYPE_CUBE,
    TEXTURETYPE_VOLUME,
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct SamplerInfo
{
    union
    {
        BYTE Value;
        struct
        {
            BYTE TextureType  : 2;
            BYTE TexCoordWrap : 4;
        };
    };

    SamplerInfo() : Value( 0 )
    {
        C_ASSERT( sizeof( SamplerInfo ) == sizeof( Value ) );
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
enum SAMPLER_SWIZZLE
{
    SAMPLER_SWIZZLE_NONE = 0,
    SAMPLER_SWIZZLE_RRRA = 1,   // For luminance texture formats.
    SAMPLER_SWIZZLE_RAAA = 2,   // For single channel texture formats.
    SAMPLER_SWIZZLE_RGAA = 3,   // For two channel texture formats.
    
    // Two bits of state per sampler.
    SAMPLER_SWIZZLE_BITS = 2,
    SAMPLER_SWIZZLE_MASK = 3,
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct RasterStates
{
    SamplerInfo PSSamplers[D3DHAL_SAMPLER_MAXSAMP];
    UINT TCIMapping;

    union
    {
        struct
        {
            UINT Flags1;
            UINT Flags2;
            UINT Flags3;
        };

        struct
        {
            UINT ProjectedTCsMask   : MAX_TEXCOORDS;
            UINT UserClipPlanes     : MAX_CLIPLANES;
            UINT FillMode           : 2;
            UINT ShadeMode          : 2;
            UINT PrimitiveType      : 3;
            UINT AlphaFunc          : 4;
            UINT FogEnable          : 1;
            UINT FogTableMode       : 2;
            UINT WFogEnable         : 1;
            UINT AlphaTestEnable    : 1;
            UINT PointSizeEnable    : 1;
            UINT PointSpriteEnable  : 1;
            UINT ColorKeyEnable     : 1;
            UINT ColorKeyBlendEnable: 1;
            UINT ColorKeyTSSDisable : MAX_TEXCOORDS;
            UINT HasTLVertices      : 1;
            UINT HardwareShadowMappingRequiredPS : MAX_PS_SAMPLER_REGS;
            UINT HardwareShadowMappingRequiredVS : MAX_VS_SAMPLER_REGS;
            UINT SwapRBOnOutputMask : MAX_PS_COLOROUT_REGS;
        };
    };

    UINT SamplerSwizzleMask;

    RasterStates() :
        TCIMapping( 0 ),
        Flags1( 0 ),
        Flags2( 0 ),
        Flags3( 0 ),
        SamplerSwizzleMask( 0 )
    {
        static_assert( sizeof( RasterStates ) == sizeof( PSSamplers ) +
                                                 sizeof( TCIMapping ) +
                                                 sizeof( Flags1 ) +
                                                 sizeof( Flags2 ) +
                                                 sizeof( Flags3 ) +
                                                 sizeof( SamplerSwizzleMask ), "Struct packing broke." );
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct VSInputDecl
{
    enum { None, NeedsIntToFloatConversion, UDEC3, DEC3N };
    WORD Usage      : 4;
    WORD UsageIndex : 4;
    WORD RegIndex   : 4;
    WORD IsTransformedPosition : 1;
    WORD InputConversion : 2;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct VSInputDecls
{
public:

    union
    {
        UINT Flags;
        struct
        {
            UINT Position   : 1;
            UINT BlendWeight: 1;
            UINT BlendIndices:1;
            UINT Normal     : 1;
            UINT PSize      : 1;
            UINT TexCoord   : 1;
            UINT Tangent    : 1;
            UINT Binormal   : 1;
            UINT TessFactor : 1;
            UINT PositionT  : 1;
            UINT Color      : 1;            
            UINT Fog        : 1;
            UINT Depth      : 1;
            UINT Sample     : 1;
        };
    };

    enum { INVALID_INDEX = -1 };

    VSInputDecls(UINT maxAllowableEntries)
    {
        Flags = 0;
        m_maxAllowableEntries = maxAllowableEntries;
    }

    bool operator==( const VSInputDecls& rhs ) const
    {
        return Flags == rhs.Flags
            && m_maxAllowableEntries == rhs.m_maxAllowableEntries
            && m_entries.size() == rhs.m_entries.size()
            && (0 == memcmp( m_entries.data(), rhs.m_entries.data(), m_entries.size() * sizeof( VSInputDecl ) ));
    }

    bool operator!=( const VSInputDecls& rhs ) const
    {
        return !(*this==rhs);
    }

    const VSInputDecl& operator[] ( UINT index ) const
    {
        Check( index < GetSize() );
        return m_entries[index];
    }

    VSInputDecl& operator[] ( UINT index )
    {
        Check( index < GetSize());
        return m_entries[index];
    }

    UINT GetSize() const
    {
        return static_cast<UINT>(m_entries.size());
    }

    void AddDecl(UINT usage, UINT usageIndex, UINT regIndex, UINT isTransformedPosition = 0, UINT InputConversion = 0)
    {
        if (m_entries.size() + 1 > m_maxAllowableEntries)
        {
            Check(false);
            return;
        }

        VSInputDecl entry;
        entry.Usage = usage;
        entry.UsageIndex = usageIndex;
        entry.RegIndex = regIndex;
        entry.IsTransformedPosition = isTransformedPosition;
        entry.InputConversion = InputConversion;
        m_entries.push_back(entry);

        this->Flags |= (1 << usage);
    }

    UINT FindRegisterIndex( UINT usage, UINT usageIndex ) const
    {
        const VSInputDecl *pDecl = FindInputDecl(usage, usageIndex);

        return pDecl ? pDecl->RegIndex : INVALID_INDEX;
    }

    const VSInputDecl *FindInputDecl(UINT usage, UINT usageIndex) const
    {
        for (size_t i = 0, n = m_entries.size(); i < n; ++i)
        {
            if (m_entries[i].Usage == usage &&
                m_entries[i].UsageIndex == usageIndex)
            {
                return &m_entries[i];
            }
        }

        return nullptr;
    }


protected:
    std::vector<VSInputDecl> m_entries;
    UINT m_maxAllowableEntries;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct VSOutputDecl
{
    UINT Usage      : 5;
    UINT UsageIndex : 4;
    UINT RegIndex   : 4;
    UINT WriteMask  : 4;    
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct VSOutputDecls
{
public:  

    enum {
        MAX_SIZE_VS   = 1 + (MAX_VS_OUTPUT_REGS - 1) * 4, // Removed 1 for Position
        MAX_SIZE_PS   = MAX_PS_INPUT_REGS * 4,
        MAX_SIZE      = (MAX_SIZE_VS > MAX_SIZE_PS) ? MAX_SIZE_VS : MAX_SIZE_PS,
        INVALID_INDEX = MAX_SIZE,
    };

    union
    {
        UINT Flags;
        struct
        {
            UINT Position  : 1;
            UINT Colors    : 2;
            UINT TexCoords : MAX_SAMPLER_REGS;
            UINT Fog       : 1;
            UINT PointSize : 1;
            UINT vFace     : 1;
            UINT vPos      : 1;
        };
    }; 

    union 
    {
        UINT64 CentroidMask : MAX_SIZE;
    };

    VSOutputDecls()
    {   
        this->CentroidMask = 0;
        this->Flags = 0;        
        m_size = 0;
        memset(m_entries, 0, sizeof(m_entries));
    }  

    VSOutputDecls(const VSOutputDecls& rhs) 
    {
        *this = rhs;
    }

    void operator=(const VSOutputDecls& rhs)
    {
        this->Flags = rhs.Flags;     
        this->CentroidMask = rhs.CentroidMask;        
        memcpy(m_entries, &rhs.m_entries, rhs.m_size * sizeof(VSOutputDecl));        
        m_size = rhs.m_size;       
    } 

    bool operator==(const VSOutputDecls& rhs) const
    {
        return (this->Flags == rhs.Flags)
            && (this->CentroidMask == rhs.CentroidMask)
            && (m_size == rhs.m_size) 
            && ((0 == rhs.m_size) 
             || (0 == memcmp(m_entries, &rhs.m_entries, rhs.m_size * sizeof(VSOutputDecl))));
    } 

    const VSOutputDecl& operator[](UINT index) const
    {
        Check(index < m_size);
        return m_entries[index];
    }

    VSOutputDecl& operator[](UINT index)
    {
        Check(index < m_size);
        return m_entries[index];
    }

    UINT GetSize() const
    {
        return m_size;
    }
    
    void AddDecl(UINT usage, UINT usageIndex, UINT regIndex, UINT writeMask, bool bCentroid = false)
    {
        // Should never hit a vertex declaration bigger than MAX_SIZE
        if (m_size >= MAX_SIZE)
        {
            Check(false);
            return;
        }

        {
            // Compress the writemask
            writeMask = (writeMask & __D3DSP_WRITEMASK_MASK) >> __D3DSP_WRITEMASK_SHIFT;

            // First check if the entry already exists
            for (UINT i = 0, n = m_size; i < n; ++i)
            {
                if ((m_entries[i].Usage == usage)
                    && (m_entries[i].UsageIndex == usageIndex))
                {
                    // Ensure unique register assignment
                    //SHADER_CONV_ASSERT(m_entries[i].RegIndex == regIndex);

                    // Extend to the existing writemask
                    m_entries[i].WriteMask |= writeMask;
                    return;
                }
            }

            const UINT index = m_size++;
            Check(index < _countof(m_entries));

            m_entries[index].Usage = usage;
            m_entries[index].UsageIndex = usageIndex;
            m_entries[index].RegIndex = regIndex;
            m_entries[index].WriteMask = writeMask;

            this->CentroidMask |= bCentroid ? (((__int64)1) << index) : 0;

            switch (usage)
            {
            case D3DDECLUSAGE_POSITION:
            case D3DDECLUSAGE_POSITIONT:
                if (0 == usageIndex)
                {
                    this->Position = 1;
                }
                break;

            case D3DDECLUSAGE_COLOR:
                if (usageIndex <= 1)
                {
                    this->Colors |= (1 << usageIndex);
                }
                break;

            case D3DDECLUSAGE_TEXCOORD:
                //SHADER_CONV_ASSERT(usageIndex < MAX_SAMPLER_REGS);
                this->TexCoords |= (1 << usageIndex);
                break;

            case D3DDECLUSAGE_FOG:
                if (0 == usageIndex)
                {
                    this->Fog = 1;
                }
                break;

            case D3DDECLUSAGE_PSIZE:
                if (0 == usageIndex)
                {
                    this->PointSize = 1;
                }
                break;

            case D3DDECLUSAGE_VFACE:
                this->vFace = 1;
                break;

            case D3DDECLUSAGE_VPOS:
                this->vPos = 1;
                break;
            }
        }
    }
    
    void AddDecl(const VSOutputDecl& decl)
    {
        this->AddDecl(decl.Usage, decl.UsageIndex, decl.RegIndex, decl.WriteMask << __D3DSP_WRITEMASK_SHIFT);
    }

    UINT FindRegisterIndex(UINT usage, UINT usageIndex) const
    {
        const VSOutputDecl *pDecl = FindOutputDecl(usage, usageIndex);

        return pDecl ? pDecl->RegIndex : INVALID_INDEX;
    }

    const VSOutputDecl *FindOutputDecl(UINT usage, UINT usageIndex) const
    {
        for (size_t i = 0, n = m_size; i < n; ++i)
        {
            if (m_entries[i].Usage == usage &&
                m_entries[i].UsageIndex == usageIndex)
            {
                return &m_entries[i];
            }
        }

        return nullptr;
    }

protected:    

    VSOutputDecl m_entries[MAX_SIZE];    
    _Field_range_(0, MAX_SIZE) UINT m_size;
};

class InputRegister
{
public:
    enum RegisterType { Undeclared, Input, Temp };

    InputRegister(BYTE nReg, RegisterType type, BYTE nWriteMask = D3DSP_WRITEMASK_ALL >> __D3DSP_WRITEMASK_SHIFT) : m_Reg(nReg), m_Type(type), m_WriteMask(nWriteMask) {}
    InputRegister() : InputRegister(INVALID_INDEX, Undeclared, 0) {}

    RegisterType GetType() const { return m_Type; }

    BYTE Reg() const { return m_Reg; }
    BYTE WriteMask() const { return m_WriteMask; }
private:
    RegisterType m_Type;
    BYTE m_Reg;
    BYTE m_WriteMask;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct InputRegs
{
    BYTE s[MAX_SAMPLER_REGS];
    InputRegister v[MAX_INPUT_REGS];
    BYTE r[MAX_TEMP_REGS];

    union
    {
        struct // Pixel shader specifics
        {
            BYTE _t[MAX_PS1X_TEXTURE_REGS];
        };
        struct // Vertex shader specifics
        {
            BYTE a0;
        };
    };

    BYTE aL;
    BYTE p0;

    InputRegs()
    {
        aL = p0 = INVALID_INDEX;
        memset(_t, INVALID_INDEX, sizeof(_t));
        memset(s, INVALID_INDEX, sizeof(s));
        memset(r, INVALID_INDEX, sizeof(r));
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct OutputRegs
{
    union
    {
        // Vertex shader ouput registers
        struct
        {
            BYTE O[MAX_VS_OUTPUT_REGS];
        };

        // Pixel shader ouput registers
        struct
        {
            BYTE oC[MAX_PS_COLOROUT_REGS];
            BYTE oDepth;
        };
    };

    OutputRegs()
    {
        memset( this, INVALID_INDEX, sizeof( OutputRegs ) );
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct PSUsageFlags
{
    union
    {
        UINT Value;
        struct
        {
            UINT BumpEnvMat       : 1;
            UINT ProjectedTCsMask : 8;
            UINT IndexableInputs  : 1;
        };
    };

    PSUsageFlags() : Value( 0 )
    {
        C_ASSERT( sizeof( PSUsageFlags ) == sizeof( Value ) );
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct VSUsageFlags
{
    union
    {
        UINT Value;
        struct
        {
            UINT OutputRegsAddressing : 1;
        };
    };

    VSUsageFlags() : Value( 0 )
    {
        C_ASSERT( sizeof( VSUsageFlags ) == sizeof( Value ) );
    }
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
enum eDirtyConsts
{
    DIRTY_CONSTSF = 1 << 0,
    DIRTY_CONSTSI = 1 << 1,
    DIRTY_CONSTSB = 1 << 2,

    DIRTY_CONSTS_ALL = ( DIRTY_CONSTSF |
                         DIRTY_CONSTSI |
                         DIRTY_CONSTSB ),
};

struct ByteCode
{
    ByteCode() : m_pByteCode(nullptr), m_byteCodeSize(0){}

    void* m_pByteCode;
    size_t m_byteCodeSize;
};

struct ShaderConst
{
    ShaderConst() {}

    ShaderConst(UINT regIndex, const FLOAT value[4])
    {
        this->RegIndex = regIndex;
        this->fValue[0] = value[0];
        this->fValue[1] = value[1];
        this->fValue[2] = value[2];
        this->fValue[3] = value[3];
    }

    ShaderConst(UINT regIndex, const INT value[4])
    {
        this->RegIndex = regIndex;
        this->iValue[0] = value[0];
        this->iValue[1] = value[1];
        this->iValue[2] = value[2];
        this->iValue[3] = value[3];
    }

    ShaderConst(UINT regIndex, const BOOL value)
    {
        this->RegIndex = regIndex;
        this->bValue = value;
    }

    UINT  RegIndex;

    union
    {
        struct
        {
            UINT Value[4];
        };
        struct
        {
            FLOAT fValue[4];
        };
        struct
        {
            INT iValue[4];
        };
        struct
        {
            BOOL  bValue;
        };
    };
};

//--
typedef std::vector<ShaderConst> ShaderConsts;

enum ShaderSettings
{
    AnythingTimes0Equals0 = 0x1
};

struct ConvertShaderArgs
{
    enum class SHADER_TYPE
    {
        SHADER_TYPE_VERTEX,
        SHADER_TYPE_PIXEL
    };

    ConvertShaderArgs(UINT d3dAPIVersionm, UINT d3dShaderSettings, const RasterStates& raster) :
        apiVersion(d3dAPIVersionm),
        shaderSettings(d3dShaderSettings),
        rasterStates(raster),
        convertedByteCode(),
        legacyByteCode(),
        maxFloatConstsUsed(0),
        maxIntConstsUsed(0),
        maxBoolConstsUsed(0)
    {}

    SHADER_TYPE type;
    UINT shaderSettings;
    UINT apiVersion;
    _In_ const RasterStates& rasterStates;
    _In_ VSOutputDecls* pPsInputDecl;
    _Inout_ VSInputDecls* pVsInputDecl;
    _In_ ByteCode legacyByteCode;
    _Out_ VSOutputDecls *pVsOutputDecl;
    _Out_ ByteCode convertedByteCode;

    _Out_ UINT8 outputRegistersMask;

    _Out_ ShaderConsts m_inlineConsts[3];

    _Out_ UINT maxFloatConstsUsed;
    _Out_ UINT maxIntConstsUsed;
    _Out_ UINT maxBoolConstsUsed;

    _Out_ UINT totalInstructionsEmitted;
    _Out_ UINT totalExtraInstructionsEmitted;
    _Out_ std::vector<VSOutputDecl> AddedSystemSemantics;
};

struct ConvertTLShaderArgs
{
    ConvertTLShaderArgs(UINT d3dAPIVersion, UINT d3dShaderSettings, VSInputDecls& inputDecl, VSOutputDecls &outputDecl) :
        apiVersion(d3dAPIVersion),
        shaderSettings(d3dShaderSettings),
        vsInputDecl(inputDecl),
        vsOutputDecl(outputDecl),
        maxFloatConstsUsed(0),
        maxIntConstsUsed(0),
        maxBoolConstsUsed(0)
    {}

    UINT apiVersion;
    _In_ UINT shaderSettings;

    _In_ VSInputDecls& vsInputDecl;
    _Out_ ByteCode convertedByteCode;
    _Out_ VSOutputDecls& vsOutputDecl;

    _Out_ UINT maxFloatConstsUsed;
    _Out_ UINT maxIntConstsUsed;
    _Out_ UINT maxBoolConstsUsed;

    _Out_ UINT totalInstructionsEmitted;
    _Out_ UINT totalExtraInstructionsEmitted;
};

struct CreateGeometryShaderArgs
{
    CreateGeometryShaderArgs(UINT apiVersion, UINT d3dShaderSettings, VSOutputDecls& vsOutputDecls, VSOutputDecls *pGSOutputDecls, const RasterStates& rasterStates) :
        m_ApiVersion(apiVersion),
        m_ShaderSettings(d3dShaderSettings),
        m_VsOutputDecls(vsOutputDecls),
        m_pGsOutputDecls(pGSOutputDecls),
        m_RasterStates(rasterStates),
        m_GSByteCode()
    {};

    _In_ UINT m_ShaderSettings;
    _In_ UINT m_ApiVersion;
    _In_ VSOutputDecls& m_VsOutputDecls;
    _Out_ VSOutputDecls *m_pGsOutputDecls;
    _In_ const RasterStates& m_RasterStates;

    _Out_ ByteCode m_GSByteCode;
};

class ITranslator;
struct ShaderConverterAPI
{
    ShaderConverterAPI() = default;
    ~ShaderConverterAPI();

    HRESULT ConvertShader(ConvertShaderArgs& args);
    HRESULT ConvertTLShader(ConvertTLShaderArgs& args);
    HRESULT CreateGeometryShader(CreateGeometryShaderArgs& args);

    static void CleanUpConvertedShader(ByteCode& byteCode);

private:
    ITranslator* m_pTranslator = nullptr;
};

} // namespace ShaderConv