// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Implementation for The Shader Disassembler for HLSL 2.0
*
****************************************************************************/

#include "pch.h"
#include "disasm.hpp"

#if DBG && WARP_INTERNAL

// SHADER_CONV_ASSERT with simple string
#undef _SHADER_CONV_ASSERT
#define _SHADER_CONV_ASSERT( value, string )    \
if ( !(value) ) {                   \
    WarpError( string );            \
}

#define __IS_VS( version ) \
    ( ( version & 0xFFFF0000 ) == 0xFFFE0000 )

#define __IS_PS( version ) \
    ( ( version & 0xFFFF0000 ) == 0xFFFF0000 )

#define SWIZZLE_BUF_SIZE    10
#define DISASM_STR_LENGTH   256
#define _ADDSTR( _Str )             {StringCchPrintfA( pStr, DISASM_STR_LENGTH, "%s" _Str , pStr );}
#define _ADDSTRP( _Str, _Param )    {StringCchPrintfA( pStr, DISASM_STR_LENGTH, "%s" _Str , pStr, _Param );}

// sizes for internal register arrays
const DWORD PSTR_MAX_COISSUED_INSTRUCTIONS  = 2;
const DWORD PSTR_NUM_COMPONENTS_IN_REGISTER = 4;
const DWORD PSTR_PIXEL_QUAD                 = 4;
const DWORD PSTR_MAX_REGISTER_STACK_DEPTH   = 8; // 4 * (aL+internal loop counter); aka
const DWORD PSTR_MAX_NUMQUEUEDWRITEREG      = PSTR_MAX_COISSUED_INSTRUCTIONS - 1;
const DWORD PSTR_MAX_NUMSRCPARAMS           = 4;
const DWORD PSTR_MAX_NUMPOSTMODSRCREG       = PSTR_MAX_NUMSRCPARAMS;
const DWORD PSTR_MAX_NUMSCRATCHREG          = 5;

// refdev-specific pixel shader 'instructions' to match legacy pixel processing
#define D3DSIO_TEXBEM_LEGACY    ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC001)
#define D3DSIO_TEXBEML_LEGACY   ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC002)

namespace ShaderConv
{

    //-----------------------------------------------------------------------------
// GetSwizzleName - Returns a string name for a swizzle in buffer provided.
// ----------------------------------------------------------------------------
HRESULT
GetSwizzleName( __out_ecount(StrCharsRet) char* pStrRet, int StrCharsRet, DWORD Swizzle )
{
    char pStr[DISASM_STR_LENGTH] = "";
    switch (Swizzle)
    {
    case D3DSP_NOSWIZZLE:       break;
    case D3DSP_REPLICATEALPHA:  _ADDSTR(".a"); break;
    case D3DSP_REPLICATERED:    _ADDSTR(".r"); break;
    case D3DSP_REPLICATEGREEN:  _ADDSTR(".g"); break;
    case D3DSP_REPLICATEBLUE:   _ADDSTR(".b"); break;
    default:
        _ADDSTR(".");
        for(UINT j = 0; j < 4; j++)
        {
            switch((Swizzle >> (D3DVS_SWIZZLE_SHIFT + 2*j)) & 0x3)
            {
            case 0:
                _ADDSTR("r");
                break;
            case 1:
                _ADDSTR("g");
                break;
            case 2:
                _ADDSTR("b");
                break;
            case 3:
                _ADDSTR("a");
                break;
            }
        }
        break;
    }

    return StringCchPrintfA( pStrRet, StrCharsRet, "%s", pStr );
}

//-----------------------------------------------------------------------------
//
// DisAsmInstr - Generates instruction disassembly string for a single
// D3D (pre-translation) pixel shader instruction.
// String interface is similar to _snprintf.
//
//-----------------------------------------------------------------------------
HRESULT
DisAsmInstr(
    __out_ecount(cchNumChars) LPSTR lpszRet,
    int                             cchNumChars,
    const DWORD**                   ppdwShader,
    DWORD                           dwVersion )
{
    SHADER_CONV_ASSERT( lpszRet && ppdwShader );

    UINT    i;
    const DWORD* pdwToken = *ppdwShader;
    char SwizzleBuf[SWIZZLE_BUF_SIZE];

    // stage in local string, then copy
    char pStr[DISASM_STR_LENGTH] = "";
    char pPrefix[DISASM_STR_LENGTH] = "";

    DWORD dwInstr = *pdwToken++;

    if( D3DSI_COISSUE & dwInstr )
    {
        _ADDSTR("+");
    }

    DWORD Opcode = (dwInstr & D3DSI_OPCODE_MASK);
    DWORD DstParam = 0;
    DWORD SrcParam[PSTR_MAX_NUMSRCPARAMS];
    DWORD SrcRelAddrToken[PSTR_MAX_NUMSRCPARAMS];
    DWORD SrcParamCount = 0;
    DWORD DstParamCount = 0;
    DWORD DclInfoToken = 0;
    BOOL bPredicated = (D3DSHADER_INSTRUCTION_PREDICATED & dwInstr);
    DWORD SrcPredicateToken = 0;

    union
    {
        // DEF'd constants (only for DEF statements)
        FLOAT   fDefValues[PSTR_NUM_COMPONENTS_IN_REGISTER];
        INT     iDefValues[PSTR_NUM_COMPONENTS_IN_REGISTER];
        DWORD   dwDefValues[PSTR_NUM_COMPONENTS_IN_REGISTER];
        BOOL    bDefValue;
    };

    switch ( D3DSI_OPCODE_MASK & dwInstr )
    {
    case D3DSIO_COMMENT:
        _ADDSTR("/* ");
        _ADDSTRP("%s", pdwToken);
        _ADDSTR(" */");
        pdwToken += ( dwInstr & D3DSI_COMMENTSIZE_MASK ) >> D3DSI_COMMENTSIZE_SHIFT;
        goto EXIT;

    case D3DSIO_NOP:
        goto EXIT;

    case D3DSIO_DCL:
        DclInfoToken = *pdwToken++;
        break;

    case D3DSIO_END:
        _ADDSTR("end");
        goto EXIT;
    }

    // Decode dest param.
    if ( *pdwToken & ( 1L << 31 ) )
    {
        switch(D3DSI_OPCODE_MASK & dwInstr)
        {
        case D3DSIO_CALL:
        case D3DSIO_REP:
        case D3DSIO_LOOP:
        case D3DSIO_CALLNZ:
        case D3DSIO_IF:
        case D3DSIO_IFC:
        case D3DSIO_BREAKC:
        case D3DSIO_BREAKP:
        case D3DSIO_LABEL:
            // no dst param, only src params.
            break;
        case D3DSIO_DEF:
        case D3DSIO_DEFI:
            DstParam = *pdwToken++;
            DstParamCount++;
            for(UINT Component = 0; Component < PSTR_NUM_COMPONENTS_IN_REGISTER; Component++)
            {
                dwDefValues[Component] = *pdwToken++;
            }
            break;
        case D3DSIO_DEFB:
            DstParam = *pdwToken++;
            DstParamCount++;
            bDefValue = *pdwToken++;
            break;
        default:
            DstParam = *pdwToken++;
            DstParamCount++;
            break;
        }

        // Decode predicate token
        if(bPredicated)
        {
            _SHADER_CONV_ASSERT(*pdwToken & (1L<<31),"D3DPSInstDisAsm - Expected predicate token");
            SrcPredicateToken = *pdwToken++;
        }

        // Decode source tokens
        while ( *pdwToken & ( 1L << 31 ) )
        {
            SHADER_CONV_ASSERT( SrcParamCount < _countof( SrcParam ) );
            SrcParam[SrcParamCount] = *pdwToken++;
            if ( D3DSI_GETADDRESSMODE( SrcParam[SrcParamCount] ) & D3DSHADER_ADDRMODE_RELATIVE )
            {
                if ( ( __IS_VS( dwVersion ) && dwVersion >= D3DVS_VERSION(2,0) ) ||
                     ( __IS_PS( dwVersion ) && dwVersion >= D3DPS_VERSION(3,0) ) )
                {
                    SrcRelAddrToken[SrcParamCount] = *pdwToken++;
                }
                else
                {
                    SrcRelAddrToken[SrcParamCount] = 0xffffffff;
                }
            }
            else
            {
                SrcRelAddrToken[SrcParamCount] = 0;
            }
            SrcParamCount++;
        }
    }

    // Emit disassembly

    // Emit predicate
    if(bPredicated)
    {
        _ADDSTR("(");

        switch (SrcPredicateToken & D3DSP_SRCMOD_MASK)
        {
        case D3DSPSM_NONE:
            break;
        case D3DSPSM_NOT:
            _ADDSTR("!");
            break;
        default:
            WarpError("D3DPSInstDisAsm - Unexpected source modifier on predicate token.");
        }
        switch (D3DSI_GETREGTYPE(SrcPredicateToken))
        {
        case D3DSPR_PREDICATE: _ADDSTR( "p" ); _SHADER_CONV_ASSERT(0==D3DSI_GETREGNUM(SrcPredicateToken),
                                   "D3DPSInstDisAsm - predicate register with nonzero regnum is unexpected");
            break;
        default:
            WarpError("D3DPSInstDisAsm - Expected predicate token to have predicate register type.");
            break;
        }
        _ADDSTRP( "%d", D3DSI_GETREGNUM(SrcPredicateToken));

        switch (SrcPredicateToken & D3DSP_SRCMOD_MASK)
        {
        case D3DSPSM_NONE:
        case D3DSPSM_NOT:
            break;
        default:
            WarpError("Unexpected srcmod for predicate token.");
            break;
        }
        GetSwizzleName(SwizzleBuf,SWIZZLE_BUF_SIZE,SrcPredicateToken & D3DVS_SWIZZLE_MASK);
        _ADDSTRP("%s",SwizzleBuf);
        _ADDSTR(") ");
    }

    switch (Opcode)
    {
    case D3DSIO_BEM:    _ADDSTR("bem"); break;
    case D3DSIO_PHASE:  _ADDSTR("phase"); break;
    case D3DSIO_ABS:    _ADDSTR("abs"); break;
    case D3DSIO_ADD:    _ADDSTR("add"); break;
    case D3DSIO_BREAK:  _ADDSTR("break"); break;
    case D3DSIO_BREAKC: _ADDSTR("break"); break;
    case D3DSIO_BREAKP: _ADDSTR("breakp"); break;
    case D3DSIO_CALL:   _ADDSTR("call"); break;
    case D3DSIO_CALLNZ: _ADDSTR("callnz"); break;
    case D3DSIO_CMP:    _ADDSTR("cmp"); break;
    case D3DSIO_CND:    _ADDSTR("cnd"); break;
    case D3DSIO_CRS:    _ADDSTR("crs"); break;
    case D3DSIO_DCL:    _ADDSTR("dcl"); break;
    case D3DSIO_DEF:    _ADDSTR("def"); break;
    case D3DSIO_DEFI:   _ADDSTR("defi"); break;
    case D3DSIO_DEFB:   _ADDSTR("defb"); break;
    case D3DSIO_DP2ADD: _ADDSTR("dp2add"); break;
    case D3DSIO_DP3:    _ADDSTR("dp3"); break;
    case D3DSIO_DP4:    _ADDSTR("dp4"); break;
    case D3DSIO_DSX:    _ADDSTR("dsx"); break;
    case D3DSIO_DSY:    _ADDSTR("dsy"); break;
    case D3DSIO_ELSE:   _ADDSTR("else");  break;
    case D3DSIO_ENDIF:  _ADDSTR("endif"); break;
    case D3DSIO_ENDLOOP:_ADDSTR("endloop"); break;
    case D3DSIO_ENDREP: _ADDSTR("endrep"); break;
    case D3DSIO_EXP:    _ADDSTR("exp"); break;
    case D3DSIO_EXPP:   _ADDSTR("expp"); break;
    case D3DSIO_FRC:    _ADDSTR("frc"); break;
    case D3DSIO_IF:     _ADDSTR("if");  break;
    case D3DSIO_IFC:    _ADDSTR("if");  break;
    case D3DSIO_LABEL:  _ADDSTR("label");  break;
    case D3DSIO_LOG:    _ADDSTR("log"); break;
    case D3DSIO_LOGP:   _ADDSTR("logp"); break;
    case D3DSIO_LOOP:   _ADDSTR("loop"); break;
    case D3DSIO_LRP:    _ADDSTR("lrp"); break;
    case D3DSIO_M3x2:   _ADDSTR("m3x2"); break;
    case D3DSIO_M3x3:   _ADDSTR("m3x3"); break;
    case D3DSIO_M3x4:   _ADDSTR("m3x4"); break;
    case D3DSIO_M4x3:   _ADDSTR("m4x3"); break;
    case D3DSIO_M4x4:   _ADDSTR("m4x4"); break;
    case D3DSIO_MAD:    _ADDSTR("mad"); break;
    case D3DSIO_MIN:    _ADDSTR("min"); break;
    case D3DSIO_MAX:    _ADDSTR("max"); break;
    case D3DSIO_MOV:    _ADDSTR("mov"); break;
    case D3DSIO_MOVA:   _ADDSTR("mova"); break;
    case D3DSIO_MUL:    _ADDSTR("mul"); break;
    case D3DSIO_NOP:    _ADDSTR("nop"); break;
    case D3DSIO_NRM:    _ADDSTR("nrm"); break;
    case D3DSIO_RCP:    _ADDSTR("rcp"); break;
    case D3DSIO_REP:    _ADDSTR("rep"); break;
    case D3DSIO_RET:    _ADDSTR("ret"); break;
    case D3DSIO_RSQ:    _ADDSTR("rsq"); break;
    case D3DSIO_POW:    _ADDSTR("pow"); break;
    case D3DSIO_SETP:   _ADDSTR("setp"); break;
    case D3DSIO_SINCOS: _ADDSTR("sincos"); break;
    case D3DSIO_SUB:    _ADDSTR("sub"); break;
    case D3DSIO_LIT:    _ADDSTR("lit"); break;
    case D3DSIO_SGE:    _ADDSTR("sge"); break;
    case D3DSIO_SLT:    _ADDSTR("slt"); break;
    case D3DSIO_SGN:    _ADDSTR("sgn"); break;
    case D3DSIO_DST:    _ADDSTR("dst"); break;
    case D3DSIO_TEXCOORD:
        if(SrcParamCount)
            _ADDSTR("texcrd")
        else
            _ADDSTR("texcoord")
        break;
    case D3DSIO_TEX:
        if(SrcParamCount)
        {
          _ADDSTR("texld")
          if(D3DSI_TEXLD_PROJECT==(dwInstr&D3DSP_OPCODESPECIFICCONTROL_MASK))
              _ADDSTR("p")
          else if(D3DSI_TEXLD_BIAS==(dwInstr&D3DSP_OPCODESPECIFICCONTROL_MASK))
              _ADDSTR("b")
        }
        else
          _ADDSTR("tex")
        break;
    case D3DSIO_TEXLDD      : _ADDSTR("texldd"); break;
    case D3DSIO_TEXLDL      : _ADDSTR("texldl"); break;
    case D3DSIO_TEXKILL     : _ADDSTR("texkill"); break;
    case D3DSIO_TEXBEM_LEGACY:
    case D3DSIO_TEXBEM      : _ADDSTR("texbem"); break;
    case D3DSIO_TEXBEML_LEGACY:
    case D3DSIO_TEXBEML     : _ADDSTR("texbeml"); break;
    case D3DSIO_TEXREG2AR   : _ADDSTR("texreg2ar"); break;
    case D3DSIO_TEXREG2GB   : _ADDSTR("texreg2gb"); break;
    case D3DSIO_TEXM3x2PAD  : _ADDSTR("texm3x2pad"); break;
    case D3DSIO_TEXM3x2TEX  : _ADDSTR("texm3x2tex"); break;
    case D3DSIO_TEXM3x3PAD  : _ADDSTR("texm3x3pad"); break;
    case D3DSIO_TEXM3x3TEX  : _ADDSTR("texm3x3tex"); break;
    case D3DSIO_TEXM3x3SPEC : _ADDSTR("texm3x3spec"); break;
    case D3DSIO_TEXM3x3VSPEC: _ADDSTR("texm3x3vspec"); break;
    case D3DSIO_TEXM3x2DEPTH: _ADDSTR("texm3x2depth"); break;
    case D3DSIO_TEXDP3      : _ADDSTR("texdp3"); break;
    case D3DSIO_TEXREG2RGB  : _ADDSTR("texreg2rgb"); break;
    case D3DSIO_TEXDEPTH    : _ADDSTR("texdepth"); break;
    case D3DSIO_TEXDP3TEX   : _ADDSTR("texdp3tex"); break;
    case D3DSIO_TEXM3x3     : _ADDSTR("texm3x3"); break;
    case D3DSIO_END         : _ADDSTR("end"); break;
    default:
        WarpError("Attempt to disassemble unknown instruction!");
        break;
    }

    switch (Opcode)
    {
    case D3DSIO_IFC:
    case D3DSIO_BREAKC:
    case D3DSIO_SETP:
        switch(D3DSI_GETCOMPARISON(dwInstr))
        {
        case D3DSPC_GT: _ADDSTR("_gt"); break;
        case D3DSPC_EQ: _ADDSTR("_eq"); break;
        case D3DSPC_GE: _ADDSTR("_ge"); break;
        case D3DSPC_LT: _ADDSTR("_lt"); break;
        case D3DSPC_NE: _ADDSTR("_ne"); break;
        case D3DSPC_LE: _ADDSTR("_le"); break;
        default:
            WarpError("D3DPSInstDisAsm - Unexpected comparison type.");
            break;
        }
        break;
    case D3DSIO_DCL:
        switch (D3DSI_GETTEXTURETYPE(DclInfoToken))
        {
        case D3DSTT_2D:
            _ADDSTR("_2d");
            break;

        case D3DSTT_CUBE:
            _ADDSTR("_cube");
            break;

        case D3DSTT_VOLUME:
            _ADDSTR("_volume");
            break;

        default:
            switch (D3DSI_GETUSAGE(DclInfoToken))
            {
            case D3DDECLUSAGE_POSITION:
                _ADDSTRP("_position%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_BLENDWEIGHT:
                _ADDSTRP("_blendweight%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_BLENDINDICES:
                _ADDSTRP("_blendindices%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_NORMAL:
                _ADDSTRP("_normal%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_PSIZE:
                _ADDSTRP("_psize%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_TEXCOORD:
                _ADDSTRP("_texcoord%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_TANGENT:
                _ADDSTRP("_tangent%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_BINORMAL:
                _ADDSTRP("_binormal%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_TESSFACTOR:
                _ADDSTRP("_tessfactor%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_POSITIONT:
                _ADDSTRP("_positiont%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_COLOR:
                _ADDSTRP("_color%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_FOG:
                _ADDSTRP("_fog%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_DEPTH:
                _ADDSTRP("_depth%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            case D3DDECLUSAGE_SAMPLE:
                _ADDSTRP("_sample%d",D3DSI_GETUSAGEINDEX(DclInfoToken));
                break;
            }
        }
        break;
    }
    if (DstParam)
    {
        switch ( (DstParam & D3DSP_DSTSHIFT_MASK) >> D3DSP_DSTSHIFT_SHIFT )
        {
        default:
        case 0x0: break;
        case 0x1: _ADDSTR("_x2"); break;
        case 0x2: _ADDSTR("_x4"); break;
        case 0x3: _ADDSTR("_x8"); break;
        case 0xF: _ADDSTR("_d2"); break;
        case 0xE: _ADDSTR("_d4"); break;
        case 0xD: _ADDSTR("_d8"); break;
        }
        if(DstParam & D3DSPDM_SATURATE)
        {
            _ADDSTR("_sat");
        }
        if(DstParam & D3DSPDM_PARTIALPRECISION)
        {
            _ADDSTR("_pp");
        }
        if(DstParam & D3DSPDM_MSAMPCENTROID)
        {
            _ADDSTR("_centroid");
        }

        switch (D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(DstParam))
        {
        case D3DSPR_RASTOUT:
            switch ( D3DSI_GETREGNUM(DstParam) )
            {
            case D3DSRO_POSITION:   _ADDSTR(" oPos"); break;
            case D3DSRO_FOG:        _ADDSTR(" oFog"); break;
            case D3DSRO_POINT_SIZE: _ADDSTR(" oPts"); break;
            default:
                WarpError("D3DPSInstDisAsm - Unexpected dest register type.");
                break;
            }
            break;

        case D3DSPR_ATTROUT:    _ADDSTRP(" oD%d", D3DSI_GETREGNUM(DstParam) ); break;
        case D3DSPR_TEXCRDOUT:  _ADDSTRP(" oT%d", D3DSI_GETREGNUM(DstParam) ); break;
        case D3DSPR_INPUT:      _ADDSTRP(" v%d", D3DSI_GETREGNUM(DstParam) ); break;
        case D3DSPR_TEMP:       _ADDSTRP(" r%d", D3DSI_GETREGNUM(DstParam) ); break;

        case D3DSPR_TEXTURE:
        //   D3DSPR_ADDR:
            if ( __IS_VS(dwVersion) )
            {
                _ADDSTR(" a0");
            }
            else
            {
                _ADDSTRP(" t%d", D3DSI_GETREGNUM(DstParam) );
            }
            break;

        case D3DSPR_CONST:      _ADDSTRP(" c%d", D3DSI_GETREGNUM_RESOLVING_CONSTANTS(DstParam) ); break;
        case D3DSPR_CONSTINT:   _ADDSTRP(" i%d", D3DSI_GETREGNUM(DstParam) ); break;
        case D3DSPR_CONSTBOOL:  _ADDSTRP(" b%d", D3DSI_GETREGNUM(DstParam) ); break;
        case D3DSPR_SAMPLER:    _ADDSTRP(" s%d", D3DSI_GETREGNUM(DstParam) ); break;
        case D3DSPR_DEPTHOUT:   _ADDSTR(" oDepth"); _SHADER_CONV_ASSERT(0==D3DSI_GETREGNUM(DstParam),"D3DPSInstDisAsm - oDepth with nonzero regnum is unexpected"); break;
        case D3DSPR_COLOROUT:   _ADDSTRP(" oC%d", D3DSI_GETREGNUM(DstParam) ); break;
        case D3DSPR_LABEL:      _ADDSTRP(" l%d", D3DSI_GETREGNUM(DstParam) ); break;
        case D3DSPR_LOOP:       _ADDSTR(" aL"); _SHADER_CONV_ASSERT(0==D3DSI_GETREGNUM(DstParam),"D3DPSInstDisAsm - aL with nonzero regnum is unexpected"); break;
        case D3DSPR_PREDICATE:  _ADDSTRP(" p%d", D3DSI_GETREGNUM(DstParam) ); _SHADER_CONV_ASSERT(0==D3DSI_GETREGNUM(DstParam),"D3DPSInstDisAsm - predicate with nonzero regnum is unexpected"); break;
        case D3DSPR_MISCTYPE:
            switch(D3DSI_GETREGNUM(DstParam))
            {
            case D3DSMO_POSITION:
                _ADDSTR(" vPos");
                break;
            case D3DSMO_FACE:
                _ADDSTR(" vFace");
                break;
            }
            break;
        default:
            WarpError("D3DPSInstDisAsm - Unexpected dest register type.");
            break;
        }
        if (D3DSP_WRITEMASK_ALL != (DstParam & D3DSP_WRITEMASK_ALL))
        {
            _ADDSTR(".");
            if (DstParam & D3DSP_WRITEMASK_0) _ADDSTR("r");
            if (DstParam & D3DSP_WRITEMASK_1) _ADDSTR("g");
            if (DstParam & D3DSP_WRITEMASK_2) _ADDSTR("b");
            if (DstParam & D3DSP_WRITEMASK_3) _ADDSTR("a");
        }
    }

    if( D3DSIO_DEF == Opcode )
    {
        _ADDSTRP(", %f, ",fDefValues[0]);
        _ADDSTRP("%f, ",fDefValues[1]);
        _ADDSTRP("%f, ",fDefValues[2]);
        _ADDSTRP("%f",fDefValues[3]);
        goto EXIT;
    }
    else if( D3DSIO_DEFI == Opcode )
    {
        _ADDSTRP(", %d, ",iDefValues[0]);
        _ADDSTRP("%d, ",iDefValues[1]);
        _ADDSTRP("%d, ",iDefValues[2]);
        _ADDSTRP("%d",iDefValues[3]);
        goto EXIT;
    }
    else if( D3DSIO_DEFB == Opcode )
    {
        _ADDSTRP(", %s",bDefValue ? "true" : "false");
        goto EXIT;
    }

    if(!DstParam && SrcParamCount)
    {
        _ADDSTR(" ");
    }

    for( i = 0; i < SrcParamCount; i++ )
    {
        if( DstParamCount || (i > 0) )
        {
            _ADDSTR( ", ");
        }

        switch (SrcParam[i] & D3DSP_SRCMOD_MASK)
        {
        default:
        case D3DSPSM_NONE:
        case D3DSPSM_BIAS:
        case D3DSPSM_SIGN:
        case D3DSPSM_X2:
        case D3DSPSM_DZ:
        case D3DSPSM_DW:
            break;
        case D3DSPSM_NEG:
        case D3DSPSM_BIASNEG:
        case D3DSPSM_SIGNNEG:
        case D3DSPSM_X2NEG:
        case D3DSPSM_ABSNEG:
            _ADDSTR( "-");
            break;
        case D3DSPSM_COMP:
            _ADDSTR( "1-");
            break;
        case D3DSPSM_NOT:
            _ADDSTR( "!" );
            break;
        }
        switch (D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(SrcParam[i]))
        {
        case D3DSPR_TEMP:       _ADDSTR("r"); break;
        case D3DSPR_INPUT:      _ADDSTR("v"); break;
        case D3DSPR_CONST:      _ADDSTR("c"); break;
        case D3DSPR_CONSTINT:   _ADDSTR("i"); break;
        case D3DSPR_CONSTBOOL:  _ADDSTR("b"); break;
        case D3DSPR_SAMPLER:    _ADDSTR("s");break;
        case D3DSPR_LABEL:      _ADDSTR("l"); break;

        case D3DSPR_TEXTURE:
        //   D3DSPR_ADDR:
            if ( __IS_VS(dwVersion) )
            {
                _ADDSTR("a0");
            }
            else
            {
                _ADDSTR("t");
            }
            break;

        case D3DSPR_MISCTYPE:
            switch(D3DSI_GETREGNUM(SrcParam[i]))
            {
            case D3DSMO_POSITION:
                _ADDSTR( "vPos");
                break;
            case D3DSMO_FACE:
                _ADDSTR( "face");
                break;
            }
            break;
        case D3DSPR_LOOP: _ADDSTR( "aL" ); _SHADER_CONV_ASSERT(0==D3DSI_GETREGNUM(SrcParam[i]),"D3DPSInstDisAsm - aL with nonzero regnum is unexpected"); break;
        case D3DSPR_PREDICATE: _ADDSTR( "p" ); _SHADER_CONV_ASSERT(0==D3DSI_GETREGNUM(SrcParam[i]),"D3DPSInstDisAsm - predicate register with nonzero regnum is unexpected"); break;
        default:
            WarpError("D3DPSInstDisAsm - Unexpected source register type");
            break;
        }
        switch (D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(SrcParam[i]))
        {
        case D3DSPR_TEMP:
        case D3DSPR_INPUT:
        case D3DSPR_TEXTURE:
        case D3DSPR_CONST:
        case D3DSPR_CONSTINT:
        case D3DSPR_CONSTBOOL:
        case D3DSPR_SAMPLER:
        case D3DSPR_LABEL:
        case D3DSPR_PREDICATE:
            if( SrcRelAddrToken[i] )
            {
                _ADDSTR( "[");

                if ( 0xffffffff == SrcRelAddrToken[i] )
                {
                    _ADDSTR( "a0");
                }
                else
                {
                    switch(D3DSI_GETREGTYPE(SrcRelAddrToken[i]))
                    {
                    case D3DSPR_LOOP:
                        _ADDSTR( "aL");
                        _SHADER_CONV_ASSERT(0 == D3DSI_GETREGNUM_RESOLVING_CONSTANTS(SrcRelAddrToken[i]),"D3DPSInstDisAsm - Unexpected nonzero register # for aL.");
                        break;
                    case D3DSPR_ADDR:
                        _ADDSTR( "a0");
                        _SHADER_CONV_ASSERT(0 == D3DSI_GETREGNUM_RESOLVING_CONSTANTS(SrcRelAddrToken[i]),"D3DPSInstDisAsm - Unexpected nonzero register # for a0.");
                        break;
                    default:
                        WarpError("D3DPSInstDisAsm - Unexpected relative addressing register type.");
                        break;
                    }

                    GetSwizzleName(SwizzleBuf,SWIZZLE_BUF_SIZE,SrcParam[i] & D3DVS_SWIZZLE_MASK);
                    _ADDSTRP( "%s",SwizzleBuf);
                }

                if(D3DSI_GETREGNUM_RESOLVING_CONSTANTS(SrcParam[i])>0)
                {
                    _ADDSTRP( "+%d",D3DSI_GETREGNUM_RESOLVING_CONSTANTS(SrcParam[i]));
                }
                _ADDSTR( "]");
            }
            else
            {
                _ADDSTRP( "%d", D3DSI_GETREGNUM(SrcParam[i]));
            }
            break;
        case D3DSPR_MISCTYPE:
        case D3DSPR_LOOP:
            break;
        default:
            WarpError("D3DPSInstDisAsm - Unexpected source register type");
            break;
        }
        switch (SrcParam[i] & D3DSP_SRCMOD_MASK)
        {
        default:
        case D3DSPSM_NONE:    break;
        case D3DSPSM_NEG:     break;
        case D3DSPSM_BIAS:
        case D3DSPSM_BIASNEG:
            _ADDSTR( "_bias");
            break;
        case D3DSPSM_SIGN:
        case D3DSPSM_SIGNNEG:
            _ADDSTR( "_bx2");
            break;
        case D3DSPSM_COMP:
            break;
        case D3DSPSM_X2:
        case D3DSPSM_X2NEG:
            _ADDSTR( "_x2");
            break;
        case D3DSPSM_DZ:
            _ADDSTR( "_db");
            break;
        case D3DSPSM_DW:
            _ADDSTR( "_da");
            break;
        case D3DSPSM_ABS:
        case D3DSPSM_ABSNEG:
            _ADDSTR( "_abs");
            break;
        }
        GetSwizzleName(SwizzleBuf,SWIZZLE_BUF_SIZE,SrcParam[i] & D3DVS_SWIZZLE_MASK);
        _ADDSTRP( "%s",SwizzleBuf);
    }

EXIT:

    *ppdwShader = pdwToken;

    return StringCchPrintfA( lpszRet, cchNumChars, "%s%s", pPrefix, pStr );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
Disasm20( const DWORD* pdwShaderAsm, DWORD /*dwFlags*/ )
{
    HRESULT hr;

    char szInstr[256] = { 0 };

    // Get the shader version
    const DWORD dwVersion = *pdwShaderAsm++;
    switch ( dwVersion )
    {
    // Pixel shader

    case D3DPS_VERSION(3,0):
        StringCchPrintfA(szInstr,256,"ps 3.0");
        break;

    case D3DPS_VERSION(2,1):
        StringCchPrintfA(szInstr,256,"ps 2.1");
        break;

    case D3DPS_VERSION(2,0):
        StringCchPrintfA(szInstr,256,"ps 2.0");
        break;

    case D3DPS_VERSION(1,4):
        StringCchPrintfA(szInstr,256,"ps 1.4");
        break;

    case D3DPS_VERSION(1,3):
        StringCchPrintfA(szInstr,256,"ps 1.3");
        break;

    case D3DPS_VERSION(1,2):
        StringCchPrintfA(szInstr,256,"ps 1.2");
        break;

    case D3DPS_VERSION(1,1):
        StringCchPrintfA(szInstr,256,"ps 1.1");
        break;

    case D3DPS_VERSION(1,0):
        StringCchPrintfA(szInstr,256,"ps 1.0");
        break;

    // Vertex shader

    case D3DVS_VERSION(3,0):
        StringCchPrintfA(szInstr,256,"vs 3.0");
        break;

    case D3DVS_VERSION(2,1):
        StringCchPrintfA(szInstr,256,"vs 2.1");
        break;

    case D3DVS_VERSION(2,0):
        StringCchPrintfA(szInstr,256,"vs 2.0");
        break;

    case D3DVS_VERSION(1,4):
        StringCchPrintfA(szInstr,256,"vs 1.4");
        break;

    case D3DVS_VERSION(1,3):
        StringCchPrintfA(szInstr,256,"vs 1.3");
        break;

    case D3DVS_VERSION(1,2):
        StringCchPrintfA(szInstr,256,"vs 1.2");
        break;

    case D3DVS_VERSION(1,1):
        StringCchPrintfA(szInstr,256,"vs 1.1");
        break;

    case D3DVS_VERSION(1,0):
        StringCchPrintfA(szInstr,256,"vs 1.0");
        break;

    default:
        return E_INVALIDARG;
    }

    while ( *pdwShaderAsm != D3DPS_END() )
    {
        hr = DisAsmInstr( szInstr, 256, &pdwShaderAsm, dwVersion );
        if ( FAILED( hr ) )
        {
            return hr;
        }

    }

    return S_OK;
}

} // namespace ShaderConv

#endif //DBG && WARP_INTERNAL
