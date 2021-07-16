// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "pch.h"

namespace D3D9on12
{
    _Check_return_ HRESULT APIENTRY CheckCounter(_In_ HANDLE hDevice, _In_ D3DDDIQUERYTYPE, _Out_ D3DDDI_COUNTER_TYPE*, _Out_ UINT*,
        _Out_writes_to_opt_(*pNameLength, *pNameLength) LPSTR,
        _Inout_opt_ UINT* pNameLength,
        _Out_writes_to_opt_(*pUnitsLength, *pUnitsLength) LPSTR,
        _Inout_opt_ UINT* pUnitsLength,
        _Out_writes_to_opt_(*pDescriptionLength, *pDescriptionLength) LPSTR,
        _Inout_opt_ UINT* pDescriptionLength)
    {
        UNREFERENCED_PARAMETER(hDevice);
        UNREFERENCED_PARAMETER(pNameLength);
        UNREFERENCED_PARAMETER(pUnitsLength);
        UNREFERENCED_PARAMETER(pDescriptionLength);

        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    VOID APIENTRY CheckCounterInfo(_In_ HANDLE hDevice, _Out_ D3DDDIARG_COUNTER_INFO* /*pCounterInfo*/)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        D3D9on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetMarker(_In_ HANDLE /*hDevice*/)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY SetMarkerMode(_In_ HANDLE /*hDevice*/, _In_ D3DDDI_MARKERTYPE /*Type*/, /*D3DDDI_SETMARKERMODE*/ UINT /*Flags*/)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY CreateQuery(_In_ HANDLE hDevice, _Inout_ D3DDDIARG_CREATEQUERY* pCreateQuery)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pCreateQuery == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }
        HRESULT hr = S_OK;

        Query* pQuery = new Query(pCreateQuery->QueryType);
        if (pQuery != nullptr)
        {
            hr = pQuery->Init(*pDevice);

            if (SUCCEEDED(hr))
            {
                pCreateQuery->hQuery = Query::GetHandleFromQuery(pQuery);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        CHECK_HR(hr);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY DestroyQuery(_In_ HANDLE hDevice, _In_ CONST HANDLE handle)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        Query* pQuery = Query::GetQueryFromHandle(handle);
        if (pDevice == nullptr || pQuery == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        delete(pQuery);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    _Check_return_ HRESULT APIENTRY IssueQuery(_In_ HANDLE hDevice, _In_ CONST D3DDDIARG_ISSUEQUERY* pIssueQuery)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pIssueQuery == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Query* pQuery = Query::GetQueryFromHandle(pIssueQuery->hQuery);
        if (pQuery == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = pQuery->Issue(*pDevice, pIssueQuery->Flags);

        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    _Check_return_ HRESULT APIENTRY GetQueryData(_In_ HANDLE hDevice, _Inout_ CONST D3DDDIARG_GETQUERYDATA* pGetQueryData)
    {
        D3D9on12_DDI_ENTRYPOINT_START(TRUE);
        Device* pDevice = Device::GetDeviceFromHandle(hDevice);
        if (pDevice == nullptr || pGetQueryData == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        Query* pQuery = Query::GetQueryFromHandle(pGetQueryData->hQuery);
        if (pQuery == nullptr)
        {
            RETURN_E_INVALIDARG_AND_CHECK();
        }

        HRESULT hr = pQuery->GetData(*pDevice, pGetQueryData->pData);
        D3D9on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    Query::Query(D3DDDIQUERYTYPE queryType) : 
        m_type(queryType),
        m_eventFenceValue(m_cUnitializedFenceValue),
        m_pUnderlyingQuery(nullptr)
    {};


    HRESULT Query::Init(Device& device)
    {
        HRESULT hr = S_OK;

        switch (m_type)
        {
        case D3DDDIQUERYTYPE_EVENT:
            m_pUnderlyingQuery.reset(new D3D12TranslationLayer::EventQuery(&device.GetContext(), D3D12TranslationLayer::COMMAND_LIST_TYPE_GRAPHICS_MASK));
            break;
        case D3DDDIQUERYTYPE_OCCLUSION:
            m_pUnderlyingQuery.reset(new D3D12TranslationLayer::Query(&device.GetContext(), D3D12TranslationLayer::e_QUERY_OCCLUSION, D3D12TranslationLayer::COMMAND_LIST_TYPE_GRAPHICS_MASK));
            break;
        case D3DDDIQUERYTYPE_TIMESTAMP:
            m_pUnderlyingQuery.reset(new D3D12TranslationLayer::Query(&device.GetContext(), D3D12TranslationLayer::e_QUERY_TIMESTAMP, D3D12TranslationLayer::COMMAND_LIST_TYPE_GRAPHICS_MASK));
            break;
        case D3DDDIQUERYTYPE_TIMESTAMPFREQ:
        case D3DDDIQUERYTYPE_TIMESTAMPDISJOINT:
            m_pUnderlyingQuery.reset(new D3D12TranslationLayer::TimestampDisjointQuery(&device.GetContext(), D3D12TranslationLayer::COMMAND_LIST_TYPE_GRAPHICS_MASK));
            break;
        default:
            break;
        }

        if (m_pUnderlyingQuery)
        {
            m_pUnderlyingQuery->Initialize();
        }

        return hr;
    }

    HRESULT Query::Issue(Device& /*device*/, D3DDDI_ISSUEQUERYFLAGS flags)
    {
        HRESULT hr = S_OK;

        switch (m_type)
        {
        case D3DDDIQUERYTYPE_EVENT:
        case D3DDDIQUERYTYPE_OCCLUSION:
        case D3DDDIQUERYTYPE_TIMESTAMP:
        case D3DDDIQUERYTYPE_TIMESTAMPDISJOINT:
        case D3DDDIQUERYTYPE_TIMESTAMPFREQ:
        {
            if (flags.Begin)
            {
                m_pUnderlyingQuery->Begin();
            }
            else if (flags.End)
            {
                m_pUnderlyingQuery->End();
            }
            else
            {
                Check9on12(false);
            }
            return S_OK;
        }
        case D3DDDIQUERYTYPE_VCACHE:
            return S_OK;

        case D3DDDIQUERYTYPE_RESOURCEMANAGER:
        case D3DDDIQUERYTYPE_VERTEXSTATS:
        case D3DDDIQUERYTYPE_DDISTATS:
        case D3DDDIQUERYTYPE_PIPELINETIMINGS:
        case D3DDDIQUERYTYPE_INTERFACETIMINGS:
        case D3DDDIQUERYTYPE_VERTEXTIMINGS:
        case D3DDDIQUERYTYPE_PIXELTIMINGS:
        case D3DDDIQUERYTYPE_BANDWIDTHTIMINGS:
        case D3DDDIQUERYTYPE_CACHEUTILIZATION:
        case D3DDDIQUERYTYPE_COUNTER_DEVICE_DEPENDENT:
        default:
            Check9on12(false);
            hr = E_NOTIMPL;
        }

        return hr;
    }

    HRESULT Query::GetData(Device& /*device*/, VOID* pData)
    {
        HRESULT hr = S_FALSE;

        switch (m_type)
        {
        case D3DDDIQUERYTYPE_EVENT:
        {
            BOOL finished = false;

            if (m_pUnderlyingQuery->GetData(&finished, sizeof(finished), true, false))
            {
                *reinterpret_cast<BOOL*>(pData) = finished;
                return S_OK;
            }
            else
            {
                return S_FALSE;
            }
        }        
        case D3DDDIQUERYTYPE_OCCLUSION:
        {
            UINT64 occluded = 0;
            if (m_pUnderlyingQuery->GetData(&occluded, sizeof(occluded), true, false))
            {
                *reinterpret_cast<DWORD*>(pData) = DWORD(occluded);
                return S_OK;
            }
            else
            {
                return S_FALSE;
            }
        }
        case D3DDDIQUERYTYPE_TIMESTAMP:
        {
            UINT64 timestamp = 0;
            if (m_pUnderlyingQuery->GetData(&timestamp, sizeof(timestamp), true, false))
            {
                *reinterpret_cast<DWORD*>(pData) = DWORD(timestamp);
                return S_OK;
            }
            else
            {
                return S_FALSE;
            }
        }
        case D3DDDIQUERYTYPE_TIMESTAMPFREQ:
        case D3DDDIQUERYTYPE_TIMESTAMPDISJOINT:
        {
            D3D12TranslationLayer::QUERY_DATA_TIMESTAMP_DISJOINT disjointResult = {};
            if (m_pUnderlyingQuery->GetData(&disjointResult, sizeof(disjointResult), true, false))
            {
                BOOL isDisjoint = disjointResult.Disjoint;
                if (m_type == D3DDDIQUERYTYPE_TIMESTAMPDISJOINT)
                {
                    *reinterpret_cast<DWORD*>(pData) = DWORD(isDisjoint);
                    return S_OK;
                }
                else
                {
                    Check9on12(m_type == D3DDDIQUERYTYPE_TIMESTAMPFREQ);
                    *reinterpret_cast<UINT64*>(pData) = isDisjoint ? 0 : disjointResult.Frequency;
                    return S_OK;
                }
            }
            else
            {
                return S_FALSE;
            }
        }
        case D3DDDIQUERYTYPE_VCACHE:
        {
            D3DDEVINFO_VCACHE Data;
            Data.Pattern = MAKEFOURCC('C', 'A', 'C', 'H');
            Data.OptMethod = 0;
            Data.CacheSize = 0;
            Data.MagicNumber = 0;
            *reinterpret_cast<D3DDEVINFO_VCACHE*>(pData) = Data;
            return S_OK;
        }
        case D3DDDIQUERYTYPE_RESOURCEMANAGER:
        case D3DDDIQUERYTYPE_VERTEXSTATS:
        case D3DDDIQUERYTYPE_DDISTATS:
        case D3DDDIQUERYTYPE_PIPELINETIMINGS:
        case D3DDDIQUERYTYPE_INTERFACETIMINGS:
        case D3DDDIQUERYTYPE_VERTEXTIMINGS:
        case D3DDDIQUERYTYPE_PIXELTIMINGS:
        case D3DDDIQUERYTYPE_BANDWIDTHTIMINGS:
        case D3DDDIQUERYTYPE_CACHEUTILIZATION:
        case D3DDDIQUERYTYPE_COUNTER_DEVICE_DEPENDENT:
        default:
            Check9on12(false);
            hr = E_NOTIMPL;
        }

        return hr;
    }
};