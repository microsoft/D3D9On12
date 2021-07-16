// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  Implementation for the Shader Translator
*
****************************************************************************/

#include "pch.h"
#include "shaderconv.hpp"

namespace ShaderConv
{

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
///---------------------------------------------------------------------------
HRESULT
CreateTranslator( UINT runtimeVersion, ITranslator** ppTranslator )
{
    SHADER_CONV_ASSERT( ppTranslator );
    return CTranslator::Create( runtimeVersion, ppTranslator );
}

//////////////////////////////////////////////////////////////////////////////

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
CTranslator::CTranslator( UINT runtimeVersion )
{
    m_runtimeVersion = runtimeVersion;
    m_pShaderAsm = NULL;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
CTranslator::~CTranslator()
{
    __safeDelete( m_pShaderAsm );
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTranslator::Create( UINT runtimeVersion, ITranslator** ppTranslator )
{
    HRESULT hr;

    if ( NULL == ppTranslator )
    {
        return E_INVALIDARG;
    }

    CTranslator* pTranslator = new CTranslator( runtimeVersion );
    if ( NULL == pTranslator )
    {
        return E_OUTOFMEMORY;
    }

    pTranslator->AddRef();

    hr = pTranslator->Initialize();
    if ( FAILED( hr ) )
    {
        pTranslator->Release();
        SHADER_CONV_ASSERT(!"CTranslator::Initialize() failed, hr = %d\n");
        return hr;
    }

    *ppTranslator = pTranslator;

    return S_OK;
}

///---------------------------------------------------------------------------
/// <summary>
/// </summary>
///---------------------------------------------------------------------------
HRESULT
CTranslator::Initialize()
{
    // Create the shader assembler
    m_pShaderAsm = new CShaderAsmWrapper();
    if ( NULL == m_pShaderAsm )
    {
        SHADER_CONV_ASSERT(!"CShaderAsm() allocation failed, out of memory\n");
        return E_OUTOFMEMORY;;
    }

    if (FAILED(m_pShaderAsm->Init()))
    {
        SHADER_CONV_ASSERT(!"Init() failed, out of memory\n");
        return E_OUTOFMEMORY;;
    }

    return S_OK;
}

} // namespace ShaderConv