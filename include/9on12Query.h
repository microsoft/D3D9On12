// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    static const D3DQUERYTYPE g_cSupportedQueryTypes[] =
    {
        D3DQUERYTYPE_VCACHE,
        D3DQUERYTYPE_EVENT,
        D3DQUERYTYPE_OCCLUSION,
        D3DQUERYTYPE_TIMESTAMP,
        D3DQUERYTYPE_TIMESTAMPDISJOINT,
        D3DQUERYTYPE_TIMESTAMPFREQ,
    };

    class Query
    {
    public:
        Query(D3DDDIQUERYTYPE queryType);

        static FORCEINLINE HANDLE GetHandleFromQuery(Query* pQuery){ return static_cast<HANDLE>(pQuery); }
        static FORCEINLINE Query* GetQueryFromHandle(HANDLE hQuery){ return static_cast<Query*>(hQuery); }

        HRESULT Init(Device& device);
        HRESULT Issue(Device& device, D3DDDI_ISSUEQUERYFLAGS flags);
        HRESULT GetData(Device& device, VOID* pData);

    private:
        D3DDDIQUERYTYPE m_type;

        std::unique_ptr<D3D12TranslationLayer::Async> m_pUnderlyingQuery;

        static const UINT64 m_cUnitializedFenceValue = _UI64_MAX;
        UINT64 m_eventFenceValue;
    };
};