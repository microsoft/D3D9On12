// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Header Definitions for The translator context
*
****************************************************************************/

#pragma once

#include "ShaderConv.hpp"

namespace ShaderConv
{

class CContext;

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
enum eSystemRegister
{
    SREG_TMP0,
    SREG_TMP1,
    SREG_TMP2,

    SREG_MUL0,
    SREG_MUL1,

    SREG_MOD0,
    SREG_MOD1,
    SREG_MOD2,
    SREG_MOD3,

    SREG_PRED,

    SREG_SIZE,
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
struct Modifiers
{
    union
    {
        UINT Value;
        struct
        {
            UINT _SHIFT       : 4;
            UINT _SATURATE    : 1;
            UINT _CENTROID    : 1;
            UINT _PPRECISION  : 1;
            UINT _PREDICATE   : 1;
            UINT _COISSUE     : 1;
        };
    };

    Modifiers() : Value( 0 ) {}
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CInstr
{
public:

    enum
    {
        SRC_REG0,
        SRC_REG1,
        SRC_REG2,
        SRC_REG3,
        SRC_PREDICATE,
        MAX_SRC_REGISTERS,
    };

    struct Token
    {
        UINT Value;
        UINT Address;

        Token() : Value( 0 ), Address( 0 ) {}

        Token( UINT value, UINT address ) : Value( value ), Address( address ) {}
    };

    CInstr( DWORD dwInstr, const CContext* pContext );

    const DWORD* SetDstToken( const DWORD* pdwToken );

    const DWORD* AddSrcToken( const DWORD* pdwToken );

    const DWORD* SetPredicate( const DWORD* pdwToken )
    {
        SHADER_CONV_ASSERT( pdwToken );
        m_srcTokens[SRC_PREDICATE].Value = *pdwToken++;
        return pdwToken;
    }

    COperandBase CreateDstOperand() const;

    COperandBase CreateSrcOperand( DWORD dwToken, DWORD dwOffset = 0 ) const;

    UINT GetToken() const
    {
        return m_token;
    }

    D3DSHADER_INSTRUCTION_OPCODE_TYPE GetOpCode() const
    {
        return m_opCode;
    }

    UINT GetLength() const
    {
        return m_length;
    }

    bool HasPredicate() const
    {
        return m_modifiers._PREDICATE ? true : false;
    }

    void ClearPredicateFlag()
    {
        m_modifiers._PREDICATE = 0;
    }

    bool HasCoissue() const
    {
        return m_modifiers._COISSUE ? true : false;
    }

    const Modifiers& GetModifiers() const
    {
        return m_modifiers;
    }

    UINT GetDstToken() const
    {
        return m_dstToken.Value;
    }

    UINT GetDstAddress() const
    {
        return m_dstToken.Address;
    }

    UINT GetSrcToken( UINT index ) const
    {
        SHADER_CONV_ASSERT( index < _countof( m_srcTokens ) );
        return m_srcTokens[index].Value;
    }

    UINT GetSrcAddress( UINT index ) const
    {
        SHADER_CONV_ASSERT( index < _countof( m_srcTokens ) );
        return m_srcTokens[index].Address;
    }

    UINT GetSrcTokenCount() const
    {
        return m_srcTokenCount;
    }

    UINT GetPredicate() const
    {
        return m_srcTokens[SRC_PREDICATE].Value;
    }

    void ApplyAddressing( COperandBase& operand,
                          DWORD dwAddrToken,
                          UINT relIndex ) const;

private:

    D3DSHADER_INSTRUCTION_OPCODE_TYPE m_opCode;

    UINT        m_token;
    UINT        m_length;
    Token       m_dstToken;
    Modifiers   m_modifiers;

    Token       m_srcTokens[MAX_SRC_REGISTERS];
    UINT        m_srcTokenCount;

    UINT                m_version;
    const InputRegs&    m_inputRegs;
    const OutputRegs&   m_outputRegs;
    const CContext*     m_pContext;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class IContext
{
public:

    IContext( UINT runtimeVersion, CShaderAsmWrapper* pShaderAsm ) :
        m_runtimeVersion( runtimeVersion ),
        m_pShaderAsm( pShaderAsm ) {}

    virtual ~IContext() {}

    virtual HRESULT WriteDeclarations() = 0;

    virtual HRESULT TranslateInstructions() = 0;

    virtual void WriteOutputs() = 0;

protected:
    void WriteClipplanes( UINT activeClipPlanesMask,
                          const COperandBase& position );

    UINT        m_runtimeVersion;
    CShaderAsmWrapper* m_pShaderAsm;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CContext : public IContext
{
public:

    CContext( UINT runtimeVersion,
              const CShaderDesc* pShaderDesc,
              const RasterStates& rasterStates,
              CShaderAsmWrapper* pShaderAsm ) :
        IContext( runtimeVersion, pShaderAsm ),
        m_rasterStates( rasterStates ),
        m_version( pShaderDesc->GetVersion() ),
        m_inputRegs( pShaderDesc->GetInputRegs() ),
        m_outputRegs( pShaderDesc->GetOutputRegs() ),
        m_toggleTEXM3x3PAD( 0 ),
        m_controlFlowDepth( 0 ),
        m_nextLoopRegister( 0 ),
        m_bOutputWritten( false ),
        m_pShaderDesc( pShaderDesc ),
        m_bLastInstrBreak( false )
    {
        m_loopNestingDepth = 0xff;
    }

    virtual ~CContext()
    {
        //--
    }

    virtual HRESULT WriteDeclarations() = 0;

    HRESULT TranslateInstructions();

    virtual void WriteOutputs() = 0;

protected:

    void Translate_MOV( const CInstr& instr );
    void Translate_ADD( const CInstr& instr );
    void Translate_SUB( const CInstr& instr );
    void Translate_MAD( const CInstr& instr );
    void Translate_MUL( const CInstr& instr );
    void Translate_RSQ( const CInstr& instr );
    void Translate_DP3( const CInstr& instr );
    void Translate_DP4( const CInstr& instr );
    void Translate_MIN( const CInstr& instr );
    void Translate_MAX( const CInstr& instr );
    void Translate_SLT( const CInstr& instr );
    void Translate_EXP( const CInstr& instr );
    void Translate_EXPP( const CInstr& instr );
    void Translate_FRC( const CInstr& instr );
    void Translate_MOVA( const CInstr& instr );
    void Translate_LIT( const CInstr& instr );
    void Translate_DST( const CInstr& instr );
    void Translate_LRP( const CInstr& instr );
    void Translate_M4x4( const CInstr& instr );
    void Translate_M4x3( const CInstr& instr );
    void Translate_M3x4( const CInstr& instr );
    void Translate_M3x3( const CInstr& instr );
    void Translate_M3x2( const CInstr& instr );
    void Translate_POW( const CInstr& instr );
    void Translate_CRS( const CInstr& instr );
    void Translate_SGN( const CInstr& instr );
    void Translate_ABS( const CInstr& instr );
    void Translate_NRM( const CInstr& instr );
    void Translate_SINCOS( const CInstr& instr );
    void Translate_CMP( const CInstr& instr );
    void Translate_DP2ADD( const CInstr& instr );
    void Translate_RCP( const CInstr& instr );
    void Translate_CND( const CInstr& instr );
    void Translate_SGE( const CInstr& instr );
    void Translate_LOG( const CInstr& instr );
    void Translate_LOGP( const CInstr& instr );
    void Translate_DSX( const CInstr& instr );
    void Translate_DSY( const CInstr& instr );
    void Translate_PHASE( const CInstr& instr );

    void Translate_LABEL( const CInstr& instr );
    void Translate_LOOP( const CInstr& instr );
    void Translate_ENDLOOP( const CInstr& instr );
    void Translate_ENDREP( const CInstr& instr );
    void Translate_ENDIF( const CInstr& instr );
    void Translate_REP( const CInstr& instr );
    void Translate_CALL( const CInstr& instr );
    void Translate_CALLNZ( const CInstr& instr );
    void Translate_RET( const CInstr& instr );
    void Translate_IF( const CInstr& instr );
    void Translate_IFC( const CInstr& instr );
    void Translate_ELSE( const CInstr& instr );
    const DWORD* Translate_BREAK( const CInstr& instr, const DWORD* pdwCurToken );
    void Translate_BREAKC( const CInstr& instr );
    void Translate_BREAKP( const CInstr& instr );

    void Translate_TEXKILL( const CInstr& instr );
    void Translate_TEX( const CInstr& instr );
    void Translate_TEXLDD( const CInstr& instr );
    void Translate_TEXLDL( const CInstr& instr );
    void Translate_TEXREG2AR( const CInstr& instr );
    void Translate_TEXREG2GB( const CInstr& instr );
    void Translate_TEXREG2RGB( const CInstr& instr );
    void Translate_TEXM3x2PAD( const CInstr& instr );
    void Translate_TEXM3x2TEX( const CInstr& instr );
    void Translate_TEXM3x2DEPTH( const CInstr& instr );
    void Translate_TEXM3x3PAD( const CInstr& instr );
    void Translate_TEXM3x3( const CInstr& instr );
    void Translate_TEXM3x3TEX( const CInstr& instr );
    void Translate_TEXM3x3SPEC( const CInstr& instr );
    void Translate_TEXM3x3VSPEC( const CInstr& instr );
    void Translate_TEXCOORD( const CInstr& instr );
    void Translate_BEM( const CInstr& instr );
    void Translate_TEXDEPTH( const CInstr& instr );
    void Translate_TEXDP3( const CInstr& instr );
    void Translate_TEXDP3TEX( const CInstr& instr );
    void Translate_TEXBEM( const CInstr& instr );
    void Translate_TEXBEML( const CInstr& instr );
    void Translate_SETP( const CInstr& instr );

    void Compare( D3DSHADER_COMPARISON comp,
                  const COperandBase& dstOperand,
                  const COperandBase& srcOperand0,
                  const COperandBase& srcOperand1 ) const;

    void EmitShiftModifier( UINT shiftMask,
                            bool bSaturate,
                            const COperandBase& dstOperand,
                            const COperandBase& srcOperand );

    void EmitSamplerSwizzle( const COperandBase& dstOperand,
                             const COperandBase& srcOperand,
                             SAMPLER_SWIZZLE swizzle );

    void EmitColorKeyModifier( UINT stage,
                               const COperandBase& dstOperand,
                               const COperandBase& srcOperand );

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             UINT stage,
                             UINT NumSrcOperands,
                             const COperandBase* pDst,
                             ...);

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand )
    {
        this->EmitDstInstruction(modifiers, opCode, 0, 1, &dstOperand, &srcOperand);
    }

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand0,
                             const COperandBase& srcOperand1 )
    {
        this->EmitDstInstruction(modifiers, opCode, 0, 2, &dstOperand, &srcOperand0, &srcOperand1);
    }

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand0,
                             const COperandBase& srcOperand1,
                             const COperandBase& srcOperand2 )
    {
        this->EmitDstInstruction(modifiers, opCode, 0, 3, &dstOperand, &srcOperand0, &srcOperand1, &srcOperand2);
    }

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand0,
                             const COperandBase& srcOperand1,
                             const COperandBase& srcOperand2,
                             const COperandBase& srcOperand3 )
    {
        this->EmitDstInstruction(modifiers, opCode, 0, 4, &dstOperand, &srcOperand0, &srcOperand1, &srcOperand2, &srcOperand3);
    }

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand0,
                             const COperandBase& srcOperand1,
                             const COperandBase& srcOperand2,
                             const COperandBase& srcOperand3,
                             const COperandBase& srcOperand4 )
    {
        this->EmitDstInstruction(modifiers, opCode, 0, 5, &dstOperand, &srcOperand0, &srcOperand1, &srcOperand2, &srcOperand3, &srcOperand4);
    }

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             UINT stage,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand0,
                             const COperandBase& srcOperand1,
                             const COperandBase& srcOperand2 )
    {
        this->EmitDstInstruction(modifiers, opCode, stage, 3, &dstOperand, &srcOperand0, &srcOperand1, &srcOperand2);
    }

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             UINT stage,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand0,
                             const COperandBase& srcOperand1,
                             const COperandBase& srcOperand2,
                             const COperandBase& srcOperand3 )
    {
        this->EmitDstInstruction(modifiers, opCode, stage, 4, &dstOperand, &srcOperand0, &srcOperand1, &srcOperand2, &srcOperand3);
    }

    void EmitDstInstruction( const Modifiers& modifiers,
                             D3D10_SB_OPCODE_TYPE opCode,
                             UINT stage,
                             const COperandBase& dstOperand,
                             const COperandBase& srcOperand0,
                             const COperandBase& srcOperand1,
                             const COperandBase& srcOperand2,
                             const COperandBase& srcOperand3,
                             const COperandBase& srcOperand4 )
    {
        this->EmitDstInstruction(modifiers, opCode, stage, 5, &dstOperand, &srcOperand0, &srcOperand1, &srcOperand2, &srcOperand3, &srcOperand4);
    }

    void EmitSampleCmp(const CInstr& instr, const COperandBase dest, const DWORD dwStage, const COperandBase src0);

    COperandBase EmitSrcOperand( const CInstr& instr,
                                 DWORD dwIndex,
                                 DWORD dwOffset = 0,
                                 DWORD dwSwizzleCount = 4 );

    COperandBase EmitImmOperand( COperandBase& srcOperand, DWORD dwModifier, UINT swizzle );

    COperandBase EmitPredOperand( const CInstr& instr )
    {
        return this->EmitSrcOperand( instr, CInstr::SRC_PREDICATE );
    }

    COperandBase EmitInputOperand(DWORD dwRegIndex, UINT sizzle = 0) const;

    void AllocateLoopRegister()
    {
        m_loopRegs[m_loopNestingDepth] = m_nextLoopRegister++;
    }

    UINT GetLoopRegister() const
    {
        return m_loopRegs[m_loopNestingDepth];
    }

    bool IsImplicitFogCalculationNeeded() { return ShaderConv::IsImplicitFogCalculationNeeded(m_rasterStates, m_version); }

    UINT GetDefaultInputRegister(UINT regIndex) const
    {
        SHADER_CONV_ASSERT(regIndex < _countof(m_defaultInputRegs));
        return m_defaultInputRegs[regIndex];
    }

    template <typename InputDecls>
    void AllocateTempRegistersForUndeclaredInputs(const InputDecls &inputDecls, BYTE &uiTotalTempRegs)
    {
        for (UINT i = 0, n = inputDecls.GetSize(); i < n; ++i)
        {
            const UINT regIndex = inputDecls[i].RegIndex;
            if (m_inputRegs.v[regIndex].GetType() == InputRegister::Undeclared)
            {
                m_defaultInputRegs[regIndex] = uiTotalTempRegs++;
            }
        }
    }

    template <typename InputDecls>
    void InitializeTempRegistersForUndeclaredInputs(const InputDecls &inputDecls, UINT numTempRegisters)
    {
        std::vector<bool> registerIsInitialized(numTempRegisters, false);
        for (UINT i = 0, n = inputDecls.GetSize(); i < n; ++i)
        {
            UINT regIndex = inputDecls[i].RegIndex;
            switch(m_inputRegs.v[regIndex].GetType())
            {
                case InputRegister::Undeclared:
                {
                    COperandBase defaultValue;

                    const UINT usage = inputDecls[i].Usage;
                    switch (usage)
                    {
                    case D3DDECLUSAGE_COLOR:
                    case D3DDECLUSAGE_PSIZE:
                        defaultValue = COperand(1.0f);
                        break;

                    case D3DDECLUSAGE_TEXCOORD:
                        defaultValue = COperand(0.0f, 0.0f, 0.0f, 1.0f);
                        break;

                    default:
                        defaultValue = COperand(0.0f);
                        break;
                    }

                    // Assign default value to the input register
                    regIndex = inputDecls[i].RegIndex;
                    m_pShaderAsm->EmitInstruction(
                        CInstruction(D3D10_SB_OPCODE_MOV,
                            CTempOperandDst(m_defaultInputRegs[regIndex]),
                            defaultValue));
                    registerIsInitialized[m_defaultInputRegs[regIndex]] = true;
                    break;
                }
                case  InputRegister::Temp:
                    registerIsInitialized[m_inputRegs.v[regIndex].Reg()] = true;
                    break;
            }
            
        }
        
        
        for (UINT i = 0; i < numTempRegisters; i++)
        {
            if (!registerIsInitialized[i])
            {
                m_pShaderAsm->EmitInstruction(
                    CInstruction(D3D10_SB_OPCODE_MOV,
                        CTempOperandDst(i),
                        COperand(0.0f)));
                registerIsInitialized[i] = true;
            }
        }
    }

    bool TextureUsesHardwareShadowMapping(DWORD stage) const;
    
protected:
    UINT m_version;

    const RasterStates& m_rasterStates;

    const InputRegs&    m_inputRegs;
    const OutputRegs&   m_outputRegs;
    const CShaderDesc*  m_pShaderDesc;

    BYTE m_defaultInputRegs[MAX_INPUT_REGS];

    UINT m_loopRegs[D3DVS20_MAX_STATICFLOWCONTROLDEPTH];
    BYTE m_toggleTEXM3x3PAD;
    BYTE m_controlFlowDepth;
    bool m_bOutputWritten;
    BYTE m_loopNestingDepth;
    UINT m_nextLoopRegister;
    bool m_bLastInstrBreak;

    friend class CInstr;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CVSContext : public CContext
{
public:

    CVSContext( UINT runtimeVersion,
                const CVertexShaderDesc* pShaderDesc,
                const RasterStates& rasterStates,
                CShaderAsmWrapper* pShaderAsm );

    HRESULT WriteDeclarations();

    void WriteOutputs();

protected:

    const VSInputDecls&     m_inputDecls;
    const VSOutputDecls&    m_outputDecls;
    const VSUsageFlags      m_usageFlags;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CTLVSContext : public IContext
{
public:

    CTLVSContext(
        UINT runtimeVersion, 
        const CTLVertexShaderDesc* pShaderDesc, 
        CShaderAsmWrapper* pShaderAsm
        );

    HRESULT WriteDeclarations();

    HRESULT TranslateInstructions();

    void WriteOutputs();

protected:

    const CTLVertexShaderDesc* m_pShaderDesc;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CPSContext : public CContext
{
public:

    CPSContext( UINT runtimeVersion,
                const CPixelShaderDesc* pShaderDesc,
                const RasterStates& rasterStates,
                const ShaderConv::VSOutputDecls& inputDecls,
                CShaderAsmWrapper* pShaderAsm );

    HRESULT WriteDeclarations();

    void WriteOutputs();

    const VSOutputDecls& GetInputDecls() const
    {
        return m_inputDecls;
    }

protected:

    void ComputeAlphaTest();

    void ComputeFogBlend();

    void ComputePixelFog();


    const VSOutputDecls& m_inputDecls;
    const PSUsageFlags   m_usageFlags;
    const VSOutputDecl*  m_pInputFog;
    const VSOutputDecl*  m_pInputSpecular;
    const BYTE           m_positionRegister;
};

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
class CGSContext : public IContext
{
public:

    CGSContext( UINT runtimeVersion,
                const CGeometryShaderDesc* pShaderDesc,
                CShaderAsmWrapper* pShaderAsm );

    HRESULT WriteDeclarations();

    HRESULT TranslateInstructions();

    void WriteOutputs();

protected:

    void EmitTexWrap( UINT primitive, UINT usageIndex, const InputRegister &regIndex );

    COperandBase CreateInputSrcOperand( UINT primitive, const InputRegister &inputReg, UINT swizzle = 0 );
    COperandBase CreateInputDstOperand( BYTE tempRegIndex, UINT primitive, UINT regIndex, UINT writeMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL );

    BYTE                    m_tempRegs[3][MAX_VS_OUTPUT_REGS];
    BYTE                    m_uiNumTempRegs;
    VSOutputDecl            m_texcoordDecls[MAX_SAMPLER_REGS];
    VSOutputDecl            m_positionDecl;
    VSOutputDecl            m_psizeDecl;
    UINT                    m_uiPrimitiveSize;
    const CGeometryShaderDesc* m_pShaderDesc;
};

} // namespace ShaderConv