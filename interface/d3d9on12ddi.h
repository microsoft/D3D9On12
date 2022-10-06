// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
*
*  Copyright (C) Microsoft Corporation.  All Rights Reserved.
*
*  File:   d3d9on12DDI.h
*  Content:    Private DDI specific for 9on12
*
****************************************************************************/

#ifndef _D3D9ON12DDI_H_
#define _D3D9ON12DDI_H_

#define D3D9ON12_CURRENT_INTERFACE_VERSION 2

typedef struct _D3D9ON12_PRESENTCB_ARGS
{
    _In_ ID3D12CommandQueue* pGraphicsCommandQueue;
    _In_ ID3D12CommandList* pGraphicsCommandList;
    _In_reads_(numSrcSurfaces) const D3DDDIARG_PRESENTSURFACE* pSrcSurfaces;
    UINT numSrcSurfaces;
    _In_opt_ HANDLE hDestResource;
    UINT flipInterval;
    UINT vidPnSourceId;
    _In_ D3DKMT_PRESENT* pKMTPresent;
} D3D9ON12_PRESENTCB_ARGS;


typedef HRESULT(CALLBACK* PFND3D9ON12_PRESENTCB)(_In_ HANDLE, _In_ D3D9ON12_PRESENTCB_ARGS*);

typedef struct _D3D9ON12_PRIVATE_CALLBACKS
{
    PFND3D9ON12_PRESENTCB pfnPresentCB;
} D3D9ON12_PRIVATE_CALLBACKS;

typedef struct _D3D9ON12_CREATE_DEVICE_ARGS
{
    IUnknown *pD3D12Device;
    IUnknown **ppD3D12Queues;
    UINT NumQueues;
    UINT NodeMask;
    UINT D3D9On12InterfaceVersion;
    D3D9ON12_PRIVATE_CALLBACKS* pPrivateCallbacks;

} D3D9ON12_CREATE_DEVICE_ARGS;

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_OPENADAPTER)(
    _Inout_ D3DDDIARG_OPENADAPTER* pOpenAdapter, _In_ LUID *pLUID, _In_opt_ D3D9ON12_CREATE_DEVICE_ARGS *pArgs);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_GETSHAREDGDIHANDLE)(
    _In_ HANDLE hDevice, HANDLE DriverResource, HANDLE *pSharedHandle);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_CREATESHAREDNTHANDLE)(
    _In_ HANDLE hDevice, HANDLE DriverResource, __in_opt SECURITY_DESCRIPTOR* pSD, HANDLE *pSharedHandle);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_GETDEVICESTATE)(
    _In_ HANDLE hDevice);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_KMTPRESENT)(
    _In_ HANDLE hDevice, D3DKMT_PRESENT *pKMTArgs);
    
#define D3D9ON12USAGE_CROSSADAPTER 0x1
#define D3D9ON12USAGE_DEFERREDFENCEWAITS 0x2
#define D3D9ON12USAGE_MONITOREDFENCE 0x8

// The first four members of all the _DESC structs are 
// always the same (even for buffers). 
// We use that fact to create a common _DESC in order
// to differentiate the kind of resource
typedef struct {
    D3DMULTISAMPLE_TYPE MultiSampleType;
    DWORD               MultiSampleQuality;
    UINT Levels;
    UINT Depth;
} D3D9ON12_TEXTURE_DESC;

typedef struct {
    DWORD FVF;
    UINT  Size;
} D3D9ON12_BUFFER_DESC;

typedef struct _D3D9ON12_RESOURCE_INFO
{
    UINT                Width;
    UINT                Height;
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    D3DPOOL             Pool;
    BOOL bNtHandle;
    BOOL bAllocatedBy9on12;

    union
    {
        // D3DRTYPE_SURFACE, D3DRTYPE_VOLUME, D3DRTYPE_TEXTURE, D3DRTYPE_VOLUMETEXTURE, D3DRTYPE_CUBETEXTURE
        D3D9ON12_TEXTURE_DESC TextureDesc;

        // D3DRTYPE_INDEXBUFFER, D3DRTYPE_VERTEXBUFFER
        D3D9ON12_BUFFER_DESC BufferDesc;
    };
} D3D9ON12_RESOURCE_INFO;

typedef struct _D3D9ON12_OPEN_RESOURCE_ARGS
{
    D3DFORMAT           Format;
} D3D9ON12_OPEN_RESOURCE_ARGS;

typedef enum D3D9ON12_COMMANDLIST_TYPE
{
    D3D9ON12_COMMANDLIST_TYPE_GRAPHICS = 0,
    D3D9ON12_COMMANDLIST_TYPE_VIDEO_DECODE = 1,
    D3D9ON12_COMMANDLIST_TYPE_VIDEO_PROCESS = 2,
    D3D9ON12_COMMANDLIST_TYPE_MAX_VALID = 3,
    D3D9ON12_COMMANDLIST_TYPE_UNKNOWN = D3D9ON12_COMMANDLIST_TYPE_MAX_VALID,
} D3D9ON12_COMMANDLIST_TYPE;

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_CREATEFENCE)(
    HANDLE hDD, UINT64 InitialValue, UINT Flags, __out HANDLE* phFence);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_OPENFENCE)(
    HANDLE hDD, HANDLE hSharedHandle, __out BOOL* pbMonitored, __out HANDLE* phFence);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_SHAREFENCE)(
    HANDLE hFence, __in_opt SECURITY_DESCRIPTOR*, __out HANDLE*);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_WAITFORFENCE)(
    HANDLE hFence, UINT64 NewFenceValue);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_SIGNALFENCE)(
    HANDLE hFence, UINT64 NewFenceValue);

typedef UINT64(APIENTRY *PFND3D9ON12_GETCOMPLETEDFENCEVALUE)(
    HANDLE hFence);

typedef void(APIENTRY *PFND3D9ON12_DESTROYTRACKEDFENCE)(
    HANDLE hFence);

typedef UINT (APIENTRY *PFND3D9ON12_QUERYRESOURCEPRIVATEDRIVERDATASIZE)(HANDLE hDD);

typedef _Check_return_ HRESULT (APIENTRY *PFND3D9ON12_QUERYRESOURCEINFOFROMKMTHANDLE)(
    HANDLE hDD, D3DKMT_HANDLE hKMResource, _Out_ D3D9ON12_RESOURCE_INFO *pResourceInfo);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_CREATEKMTHANDLE)(
    HANDLE hDD, _In_ HANDLE resourceHandle, _Out_ D3DKMT_HANDLE *pHKMResource, _Out_writes_bytes_(dataSize) void* pResourcePrivateDriverData, UINT dataSize);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_DESTROYKMTHANDLE)(
    HANDLE hDD, D3DKMT_HANDLE hKMResource);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_OPENRESOURCE)(
    HANDLE hDD, _Inout_ D3DDDIARG_OPENRESOURCE* hOpenResource, _In_ D3D9ON12_OPEN_RESOURCE_ARGS *pOpenResourceArgs);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_CREATERESOURCEWRAPPINGHANDLE)(
    HANDLE hDD, _In_ IUnknown* pD3D12Resource, _Out_ HANDLE* phWrappingHandle);

typedef void(APIENTRY *PFND3D9ON12_DESTROYRESOURCEWRAPPINGHANDLE)(HANDLE hDD, HANDLE hWrappingHandle);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_GETD3D12DEVICE)(HANDLE hDD, REFIID riid, void** ppv);

typedef void(APIENTRY *PFND3D9ON12_RESOURCECHANGESTATE)(HANDLE hDD, HANDLE hResource, UINT State);

typedef void(APIENTRY *PFND3D9ON12_SETMAXIMUMFRAMELATENCY)(HANDLE hDD, UINT MaxFrameLatency);
typedef BOOL(APIENTRY *PFND3D9ON12_ISMAXIMUMFRAMELATENCYREACHED)(HANDLE hDD);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_GETD3D12RESOURCE)(HANDLE hResource, REFIID riid, void** ppv);
typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_ADDRESOURCEWAITSTOQUEUE)(HANDLE hResource, ID3D12CommandQueue* pCommmandQueue);
typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_ADDDEFERREDWAITSTORESOURCE)(HANDLE hResource, UINT NumSync, UINT64* pSignalValues, ID3D12Fence** ppFences);

typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_CLOSEANDSUBMITGRAPHICSCOMMANDLISTFORPRESENT)(
    _In_ HANDLE hDevice, BOOL commandsAdded, _In_reads_(numSrcSurfaces) const D3DDDIARG_PRESENTSURFACE* pSrcSurfaces, UINT numSrcSurfaces, _In_opt_ HANDLE hDestResource, _In_ D3DKMT_PRESENT* pKMTPresent);
typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_PREEXECUTECOMMANDQUEUECOMMAND)(_In_ HANDLE hDevice, D3D9ON12_COMMANDLIST_TYPE commandListType);
typedef _Check_return_ HRESULT(APIENTRY *PFND3D9ON12_POSTEXECUTECOMMANDQUEUECOMMAND)(_In_ HANDLE hDevice, D3D9ON12_COMMANDLIST_TYPE commandListType);

typedef struct _D3D9ON12_PRIVATE_DDI_TABLE
{
    PFND3D9ON12_OPENADAPTER pfnOpenAdapter;

    PFND3D9ON12_GETSHAREDGDIHANDLE pfnGetSharedGDIHandle;
    PFND3D9ON12_CREATESHAREDNTHANDLE pfnCreateSharedNTHandle;
    PFND3D9ON12_GETDEVICESTATE pfnGetDeviceState;
    PFND3D9ON12_KMTPRESENT pfnKMTPresent;

    PFND3D9ON12_CREATEFENCE pfnCreateFence;
    PFND3D9ON12_OPENFENCE pfnOpenFence;
    PFND3D9ON12_SHAREFENCE pfnShareFence;
    PFND3D9ON12_WAITFORFENCE pfnWaitForFence;
    PFND3D9ON12_SIGNALFENCE pfnSignalFence;

    PFND3D9ON12_GETCOMPLETEDFENCEVALUE pfnGetCompletedFenceValue;
    PFND3D9ON12_DESTROYTRACKEDFENCE pfnDestroyTrackedFence;

    PFND3D9ON12_QUERYRESOURCEPRIVATEDRIVERDATASIZE pfnQueryResourcePrivateDriverDataSize;
    PFND3D9ON12_OPENRESOURCE pfnOpenResource;

    PFND3D9ON12_CREATEKMTHANDLE pfnCreateKMTHandle;
    PFND3D9ON12_QUERYRESOURCEINFOFROMKMTHANDLE pfnQueryResourceInfoFromKMTHandle;
    PFND3D9ON12_DESTROYKMTHANDLE pfnDestroyKMTHandle;

    PFND3D9ON12_CREATERESOURCEWRAPPINGHANDLE pfnCreateResourceWrappingHandle;
    PFND3D9ON12_DESTROYRESOURCEWRAPPINGHANDLE pfnDestroyResourceWrappingHandle;

    PFND3D9ON12_GETD3D12DEVICE pfnGetD3D12Device;
    PFND3D9ON12_RESOURCECHANGESTATE pfnTransitionResource;
    PFND3D9ON12_RESOURCECHANGESTATE pfnSetCurrentResourceState;

    PFND3D9ON12_SETMAXIMUMFRAMELATENCY pfnSetMaximumFrameLatency;
    PFND3D9ON12_ISMAXIMUMFRAMELATENCYREACHED pfnIsMaxmimumFrameLatencyReached;
    PFND3D9ON12_GETD3D12RESOURCE pfnGetD3D12Resource;
    PFND3D9ON12_ADDRESOURCEWAITSTOQUEUE pfnAddResourceWaitsToQueue;
    PFND3D9ON12_ADDDEFERREDWAITSTORESOURCE pfnAddDeferredWaitsToResource;

    //New DDIs should be added to D3D9ON12_PRIVATE_DDI_TABLE2 only
} D3D9ON12_PRIVATE_DDI_TABLE;

typedef struct _D3D9ON12_PRIVATE_DDI_TABLE_VERSIONED
{
    PFND3D9ON12_OPENADAPTER pfnOpenAdapter;

    PFND3D9ON12_GETSHAREDGDIHANDLE pfnGetSharedGDIHandle;
    PFND3D9ON12_CREATESHAREDNTHANDLE pfnCreateSharedNTHandle;
    PFND3D9ON12_GETDEVICESTATE pfnGetDeviceState;
    PFND3D9ON12_KMTPRESENT pfnKMTPresent;

    PFND3D9ON12_CREATEFENCE pfnCreateFence;
    PFND3D9ON12_OPENFENCE pfnOpenFence;
    PFND3D9ON12_SHAREFENCE pfnShareFence;
    PFND3D9ON12_WAITFORFENCE pfnWaitForFence;
    PFND3D9ON12_SIGNALFENCE pfnSignalFence;

    PFND3D9ON12_GETCOMPLETEDFENCEVALUE pfnGetCompletedFenceValue;
    PFND3D9ON12_DESTROYTRACKEDFENCE pfnDestroyTrackedFence;

    PFND3D9ON12_QUERYRESOURCEPRIVATEDRIVERDATASIZE pfnQueryResourcePrivateDriverDataSize;
    PFND3D9ON12_OPENRESOURCE pfnOpenResource;

    PFND3D9ON12_CREATEKMTHANDLE pfnCreateKMTHandle;
    PFND3D9ON12_QUERYRESOURCEINFOFROMKMTHANDLE pfnQueryResourceInfoFromKMTHandle;
    PFND3D9ON12_DESTROYKMTHANDLE pfnDestroyKMTHandle;

    PFND3D9ON12_CREATERESOURCEWRAPPINGHANDLE pfnCreateResourceWrappingHandle;
    PFND3D9ON12_DESTROYRESOURCEWRAPPINGHANDLE pfnDestroyResourceWrappingHandle;

    PFND3D9ON12_GETD3D12DEVICE pfnGetD3D12Device;
    PFND3D9ON12_RESOURCECHANGESTATE pfnTransitionResource;
    PFND3D9ON12_RESOURCECHANGESTATE pfnSetCurrentResourceState;

    PFND3D9ON12_SETMAXIMUMFRAMELATENCY pfnSetMaximumFrameLatency;
    PFND3D9ON12_ISMAXIMUMFRAMELATENCYREACHED pfnIsMaxmimumFrameLatencyReached;
    PFND3D9ON12_GETD3D12RESOURCE pfnGetD3D12Resource;
    PFND3D9ON12_ADDRESOURCEWAITSTOQUEUE pfnAddResourceWaitsToQueue;
    PFND3D9ON12_ADDDEFERREDWAITSTORESOURCE pfnAddDeferredWaitsToResource;

    // Entries beyond this point only valid when D3D11On12InterfaceVersion >= 2
    PFND3D9ON12_CLOSEANDSUBMITGRAPHICSCOMMANDLISTFORPRESENT pfnCloseAndSubmitGraphicsCommandListForPresent;
    PFND3D9ON12_PREEXECUTECOMMANDQUEUECOMMAND pfnPreExecuteCommandQueueCommand;
    PFND3D9ON12_POSTEXECUTECOMMANDQUEUECOMMAND pfnPostExecuteCommandQueueCommand;
} D3D9ON12_PRIVATE_DDI_TABLE_VERSIONED;

typedef void (APIENTRY *PFND3D9ON12_GETPRIVATEDDITABLE)(
    D3D9ON12_PRIVATE_DDI_TABLE *pPrivateDDITable);
typedef HRESULT (APIENTRY* PFND3D9ON12_GETPRIVATEDDITABLEVERSIONED)(
    _Inout_updates_bytes_(ddiTableSize) void* pPrivateDDITableVersioned, UINT ddiTableSize);

#define D3D9ON12_PIXEL_SHADER_MASK 0x1
#define D3D9ON12_VERTEX_SHADER_MASK 0x2

typedef struct _D3D9ON12_APP_COMPAT_INFO
{
    DWORD AnythingTimes0Equals0ShaderMask;
    DWORD PSOCacheTrimLimitSize;  // PSO cache will be trimmed if its size is greater then this limit
    DWORD PSOCacheTrimLimitAge;   // We will only trim PSOs that are older then this age (age in command list age)
    DWORD MaxAllocatedUploadHeapSpacePerCommandList; // Maximum amount of storage allocated for upload operations per command list before flush
    DWORD MaxSRVHeapSize; // Maximum number of entries in the sharer resource view decriptor heap
} D3D9ON12_APP_COMPAT_INFO;

typedef void (APIENTRY *PFND3D9ON12_SETAPPCOMPATDATA)(
    const D3D9ON12_APP_COMPAT_INFO *pAppCompatData);
#endif /* _D3D9ON12DDI_H_ */

