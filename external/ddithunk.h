// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*==========================================================================;
 *
 *  Copyright (C) 1994-1999 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       ddithunk.h
 *  Content:    header file used by the NT DDI thunk layer
 *  History:
 *   Date   By  Reason
 *   ====   ==  ======
 *   03-Dec-99  smac    Created it
 *
 ***************************************************************************/

#ifndef __DDITHUNK_INCLUDED__
#define __DDITHUNK_INCLUDED__

// Entire file should not be used in Win9x builds
#ifndef WIN95
#include <d3dkmthk.h>

#define MAX_ZSTENCIL_FORMATS    40

typedef struct _DDSURFHANDLE *PDDSURFHANDLE;
typedef struct _LHSURFHANDLE *PLHSURFHANDLE;
typedef struct _DEFERREDCREATE *PDEFERREDCREATE;

typedef struct _D3DCONTEXTHANDLE
{
    HANDLE                      dwhContext;
    DWORD                       dwFlags;
    struct _DDDEVICEHANDLE*     pDevice;
    DWORD                       dwPID;
    HANDLE                      hDeferHandle;
    struct _DDSURFHANDLE*       pSurface;
    struct _DDSURFHANDLE*       pDDSZ;
} D3DCONTEXTHANDLE, * PD3DCONTEXTHANDLE;

#define D3DCONTEXT_DEFERCREATE  0x00000001

typedef struct _GPUCONTEXTHANDLE *PGPUCONTEXTHANDLE;
typedef struct _GPUCONTEXTHANDLE
{
    D3DKMT_HANDLE                   hContext;
    PVOID                           pCachedCommandBuffer;
    UINT                            CachedCommandBufferSize;
    D3DDDI_ALLOCATIONLIST*          pCachedAllocationList;
    UINT                            CachedAllocationListSize;
    D3DDDI_PATCHLOCATIONLIST*       pCachedPatchLocationList;
    UINT                            CachedPatchLocationListSize;
    D3DGPU_VIRTUAL_ADDRESS          CachedCommandBuffer; // hideyukn_fix: should be unioned with above unused for advsch.
    PGPUCONTEXTHANDLE               pNextContext;
} GPUCONTEXTHANDLE, *PGPUCONTEXTHANDLE;

typedef enum _OVERLAYDWMSTATE
{
    OVERLAYSTATE_UNINITIALIZED = 0,
    OVERLAYSTATE_NODWM         = 1,
    OVERLAYSTATE_DWM           = 2
} OVERLAYDWMSTATE;

typedef struct _CONTENT_PROTECTION
{
    BOOL  ProtectionEnabled;
    BOOL  OverlayOrFullscreenRequired;
    BOOL  RDPDriverActive;
    UINT  DisplayUniqueness;
} CONTENT_PROTECTION;

typedef struct _HANDLE_LIST
{
    HANDLE* pList;
    UINT    Entries;
} HANDLE_LIST;

// Maximum number of cached shared redirection surfaces whose destruction is delayed until a space is needed in the cache
// to store a newer redirection surface, at which point surfaces are disposed of in a LRU fashion.
#define DDMAX_CACHED_REDIRECTION_SURFACES    8

typedef struct _DDDEVICEHANDLE
{
    union
    {
        HANDLE                  hDD;        // XDDM DD Local handle (for REF)
        D3DKMT_HANDLE           hDevice;    // LDDM KM device handle
    };
    DWLIST                      SurfaceHandleList;
    char                        szDeviceName[MAX_DRIVER_NAME];
    UINT                        AdapterIndex;
    LPDDRAWI_DIRECTDRAW_LCL     pDD;    // Used by Refrast and RGB HEL
    UINT                        DisplayUniqueness;
    PDDSURFHANDLE               pSurfList;
    PLHSURFHANDLE               pLHSurfList;
    PD3DCONTEXTHANDLE           pContext;
    D3DFORMAT                   DisplayFormatWithoutAlpha;
    D3DFORMAT                   DisplayFormatWithAlpha;
    UINT                        DisplayPitch;
    DWORD                       DriverLevel;
    RECT                        rcMonitor;
    DWORD                       dwVisUnique;    //updated after every SetVisRgn()
    VOID*                       pSwInitFunction;
    DWORD                       PCIID;
    DWORD                       DriverVersionHigh;
    DWORD                       DriverVersionLow;
    DWORD                       ForceFlagsOff;
    DWORD                       ForceFlagsOn;
    DWORD                       dwFlags;
    union
    {
        UINT                        DeviceFlags;        //bitwise flags defined by following booleans
        struct
        {
            UINT    bExclusiveMode          : 1;
            UINT    bDeviceLost             : 1;
            UINT    bIsWhistler             : 1;
            UINT    PresentRedirected       : 1;
            UINT    bLegacyMode             : 1;
            UINT    bRequestVSync           : 1;
            UINT    bRotationAware          : 1;
            UINT    bAsyncCallbacks         : 1; //True when async callbacks are enabled
            UINT    bDDIThreadingEnabled    : 1; //True when the DDI multithreading is enabled in the D3D runtime
            UINT    bAllowDriverMultithreading : 1; //True when the user mode driver is not allowed to use multithreading
            UINT    bMaster                 : 1; // This master device has subordinate devices associated with it
            UINT    PrimaryLockRectValid    : 1;
            UINT    bVideo                  : 1; // Indicates that the device is used for rendering video
            UINT    bCapsOnly               : 1; // Indicates that the device is not used for rendering, so we do not need to
                                                 // call the UMD's CreateDevice call (saves time and resources)
            UINT    bGpuVirtualAddressSupported : 1;
            UINT    bCheckedContentProtection   : 1;
            UINT    bSupportsContentProtection  : 1;
            UINT    bSupportsRestrictedAccess   : 1;
            UINT    bDisableOfferReclaim    : 1;
            UINT    bEmulatedLegacyMode     : 1;
            UINT    bDeferMarkDeviceAsError : 1;
            UINT    bSupportsMaximizedWindowedModeForGameDVR : 1;
        };
    };

    LONG volatile   bDeviceLostAsync;            //True when device is lost and async callbacks are enabled
                                                 //This was pulled out of the above union to avoid false data
                                                 //sharing on multiple threads

    UINT                            DeferredDeviceErrorReason;

    RECTL                           PrimaryLockRect;                // Valid only when PrimaryLockRectValid is true
    D3DKMT_HANDLE                   SharedPrimaryAllocationHandle;  // Device handle of shared primary    
    D3DKMT_DEVICEEXECUTION_STATE    ExecutionState;
    VOID*                           pCurrentFrontBuffer;
    DWORD                           DDCaps;
    DWORD                           SVBCaps;
    HANDLE                          hLibrary;
    PDEFERREDCREATE                 pDeferList;
    D3DDEVTYPE                      DeviceType;
    D3DKMT_PRESENT                  PresentKmtArg;        // Modified by the batch thread in batched present mode
    HANDLE                          hPresentLimitSemaphore;
    UINT64                          PresentLimitSemaphoreID;    
    UINT                            PresentLimitSemaphoreUsers;
    INT                             PresentLimitAdjust;
    BOOL volatile                   bPresentSubmitted;    // TRUE when a present DDI call has been made and the coressponding PresentCB call has not yet been made                   
    HRESULT                         PresentResult;        // Modified by the batch thread in batched present mode
    HRESULT                         SetDisplayModeResult;
    HANDLE                          hUMDrv;
    HANDLE                          hDrvInst;
    D3DDDI_DEVICEFUNCS              UMFunctions;
    D3DDDI_ADAPTERFUNCS             AdapterFunctions;
    D3DDDI_DEVICEFUNCS              CachedDeviceFunctions;
    D3DDDI_ADAPTERFUNCS             CachedAdapterFunctions;
    HANDLE                          hDriverAdapter;
#ifdef DWMREDIRECTION
    BOOL                            PresentRedirectionEnabled;

    // Redirection surface caching.
    struct
    {
        HANDLE  hSharedHandle;
        HANDLE  hRedirectionSurface;
    }
    CachedRedirectionSurfaceHandle[DDMAX_CACHED_REDIRECTION_SURFACES];

    // Video devices: DDMAX_CACHED_REDIRECTION_SURFACES,
    // Otherwise: 1.
    UCHAR                           RedirectionSurfaceCacheSize;
    UCHAR                           NumCachedRedirectionSurfaces;

    UINT                            RedirectionSurfaceCacheHit;
    UINT                            RedirectionSurfaceCacheMiss;    
#endif // DWMREDIRECTION
    SRWLOCK                         TrackedFencesSRWLock;
    TRACKED_FENCE**                 apTrackedFence;
    UINT                            TrackedFences;
    UINT                            TrackedFenceCapacity;
    UINT                            NumVidMemSurfaces;
    UINT                            NumDecodeRenderTargetFormats;
    GUID*                           pDeinterlaceGuids;
    UINT                            NumDeinterlaceGuids;
    D3DFORMAT*                      pDecodeRenderTargetFormats;
    D3DKMT_HANDLE                   hAdapter;
    LUID                            AdapterLuid;
    D3DDDI_VIDEO_PRESENT_SOURCE_ID  VidPnSourceId;
    D3DGAMMARAMP                    OrigGammaRamp;
    ULONGLONG                       TotalTexturePoolSize;
    ULONGLONG                       FreeTexturePoolSize;
    INT                             SchedulingPriority;
    UINT64                          PresentCount;
    ULONGLONG                       VistaBltPresentHistoryToken;
    UINT                            MaxHardwareFlipQueueLength;
    UINT                            MaxSoftwareFlipQueueLength;
    BOOL                            DriverSupportFlipIntervals;
    UINT                            MaxFrameLatency;
    D3DDDI_ROTATION                 DisplayOrientation;
    D3DDDI_VIDEO_SIGNAL_SCANLINE_ORDERING ScanLineOrdering; 
    PGPUCONTEXTHANDLE               DefaultGPUContext;
    SRWLOCK                         GPUContextListSRWLock;
    PGPUCONTEXTHANDLE               GPUContextList;
    HMONITOR                        hMasterMonitor;
    UINT                            DisplayWidth;
    UINT                            DisplayHeight;
    UINT                            DriverVersion;
    BOOL                            m_bSupportsWDDM1_3PerfDDIs;
    CONTENT_PROTECTION              ContentProtection;
    struct _DDDEVICEHANDLE*         pSubordinateDevices[D3DKMDT_MAX_VIDPN_SOURCES];
    HANDLE                          hOverlay;
    RECT                            OverlaySrcRect;
    RECT                            OverlayDstRect;
    RECT                            OverlayClientRect;
    UINT                            OverlayColorkey;
    UINT                            OverlayHardwareColorkey;
    UINT                            OverlayHardwareColorkeyUpper;
    UINT                            OverlayDisplayUniqueness;
    D3DFORMAT                       OverlayDisplayFormat;
    OVERLAYDWMSTATE                 OverlayDWMState;
    HANDLE_LIST                     RestrictedSharedSurfaces;
    HANDLE_LIST                     RestrictedSharedProcesses;
    ULONGLONG                       LastPresentQPCTime;
    D3DKMT_HANDLE                   hKernelOverlay;
    D3DKMT_WDDM_1_2_CAPS            Wddm12Caps;
    D3DKMT_ADAPTERTYPE              AdapterType;
    HANDLE                          hDDHybridChildDevice;
    UINT                            hPresentFence;
    UINT64                          uPresentSignalValue;
    UINT64                          uPresentWaitValue;
    BOOL                            bSingleVendorLda;
    BOOL                            bEverFullscreen;
    BOOL                            bEverAppSetFrameLimit;
    HANDLE                          hTrimNotification;
} DDDEVICEHANDLE, * PDDDEVICEHANDLE;

typedef struct
{
    UINT                            ModeCount;
    D3DKMT_DISPLAYMODE*             pModeList;
} DDMODEINFO;


//
// Synchronization around the members of DDDEVICEHANDLE and LHSURFHANDLE is important
// because this structure can be accessed by 2 threads on multi-processor systems.
// The main application thread can be accessing these members
// while the worker thread is accessing them inside a callback
//
// If a worker thread is inside of a runtime callback it must be the only
// thread in any callback reference a particular device.  Additionally
// DDDEVICEHANDLE::bAsyncCallbacks must be = TRUE
//
// In this case, the only variables that are written to by the worker thread are
// DDDEVICEHANDLE::bDeviceLostAsync and GPUContextList
// The write to bDeviceLostAsync is safe, because it is only read in a call to
// SetAsyncCallbacksCB(FALSE), which must be made only after the worker thread is flushed
// The write to GPUContextList is safe because the main app thread only access this list in the following
// circumstances:
//
// When destroying the device (after the user-mode device has been destroyed, and hence the worker thread has been flushed)
// When setting the GPU thread priority (the worker thread is flushed before this is done)
// 
//
// The worker thread reads DDDEVICEHANDLE::DefaultGPUContext
// and LHSURFHANDLE members that are initialized
// at surface/device creation time, and never changed again (like Format and Width)
// These reads are safe, because the values can't be changed while the worker thread is running
//

#define DDDEVICE_SUPPORTD3DBUF        0x00000001    // this device has D3DBuf callbacks
#define DDDEVICE_DP2ERROR             0x00000002    // A DP2 call failed
#define DDDEVICE_SUPPORTSUBVOLUMELOCK 0x00000004    // this device supports sub-volume texture lock
#define DDDEVICE_CANCREATESURFACE     0x00000008    // OK to call CanCreateSurface with this device
#define DDDEVICE_READY                0x00000010    // All vidmem surfs have been destroyed for this device
#define DDDEVICE_GETDRIVERINFO2       0x00000020    // Driver support the GetDriverInfo2 call
#define DDDEVICE_INITIALIZED          0x00000040    // The device has been initialized
#define DDDEVICE_GAMMASET             0x00000080    // A gamma ramp is currently set for this device
#define DDDEVICE_MPEG2IDCT            0x00000100    // The device supports MPEG2 IDCT
#define DDDEVICE_MPEG2VLD             0x00000200    // The device supports MPEG2 VLD
#define DDDEVICE_GETCOMPRESSEDCALLED  0x00000400    // DXVA GetCompressedBufferInfo has been called
#define DDDEVICE_QUERIEDPROCAMP       0x00000800    // Indicates we have queried for ProcAmp support
#define DDDEVICE_PROCAMPSUPPORTED     0x00001000    // Indicates that ProcAmp is supported

#ifdef DWMREDIRECTION
#define ISPRESENTREDIRECTED(x)  (((PDDDEVICEHANDLE)(x))->PresentRedirectionEnabled)    // Present needs to redirect
#else
#define ISPRESENTREDIRECTED(x)  NULL
#endif
#define DDHANDLE(x)  \
    (((PDDDEVICEHANDLE)(x))->hDD)

typedef struct _LHSURFHANDLE
{
    HANDLE                      hUMDriverResourceHandle;    // User mode driver returned handle
    UINT                        Index;      //sub resource index
    UINT                        MipCount;
    D3DKMT_HANDLE               hSurface;
    HANDLE                      hSharedHandle; // Cross process surface handle for redirection
    
    // Runtime resource data. Note that enumerants use D3D types, not DDI ones.
    // This data shadows the runtime information in this DDI accessible struct. Used by several functions:
    // a) Width and Height used by DWM redirected present, in DdBltLH
    // b) All of them by D3D9ResetDevice, when recreating the sysmem surfaces in the new device
    // c) All of them in AllocSurfaceWhenLost (when the surface is moved to sysmem while the device is lost)
    // d) All of them in DdLockLH (when the dummy sysmem surface is locked)
    D3DFORMAT                   Format;
    D3DRESOURCETYPE             ResourceType;
    UINT                        Width;
    UINT                        Height;
    UINT                        Depth;
    UINT                        FVF;            
    // These two are only shadowed for validation purposes when a shared surface is opened in-API
    D3DMULTISAMPLE_TYPE         MultiSampleType;
    UINT                        MultiSampleQuality;
    DWORD                       Usage;

    UINT                        LockRefCnt;
    D3DDDI_RESOURCEFLAGS        Caps;
    D3DDDI_POOL                 Pool;
    PDDDEVICEHANDLE             pDevice;
    UINT                        Flags;
    union
    {
        UINT                    ResourceFlags;  //bitwise flags defined by following booleans
        struct
        {
            UINT    bIsGdiPrimary           : 1;
            UINT    bMappedCompressedBuffer : 1;
            UINT    bRawDisplayModePrimary  : 1;
            UINT    bRotated                : 1;
            UINT    bAsyncLockable          : 1; //TRUE if a surface can be locked with asynchrounous lock flags
            UINT    bAsyncLocked            : 1; //TRUE if the surf is currently locked with the AsyncLock DDI    
            UINT    bReadOnly               : 1; //TRUE if surface is read only
            UINT    bRestrictedAccess       : 1; 
            UINT    bUserMemory             : 1; 
            UINT    bCrossAdapter           : 1;
        };
    };
    VOID*                           pSysmem;
    UINT                            SysmemPitch;    // Required for handling Locks/Creations when the device is removed
    UINT                            VidMemSize;
    UINT                            RawDisplayModeWidth;     // Due to NOAUTOROTATE feature, display mode width & Resource width
    UINT                            RawDisplayModeHeight;    // are not always equal. Therefore, they must be stored here for matching
    UINT                            RawDisplayModeFrequency; // against changing the display mode to the current display mode.
    HANDLE                          ApiSurfPointer; // API-visible surface pointer.  Used for event tracing only.
    HDC                             hDC;
    HBITMAP                         hBitmap;
    D3DDDI_VIDEO_PRESENT_SOURCE_ID  VidPnSourceIdOverride; // Used if LHSURF_HIDDENPRIMARY is set.

    struct _LHSURFHANDLE*       pNext;
    struct _LHSURFHANDLE*       pPrevious;
} LHSURFHANDLE, * PLHSURFHANDLE;

#define LHSURF_CREATESHARED             0x00000002      // Surface is created as shared
#define LHSURF_PICPARAMS                0x00000004      // Needs to stay in sync with DXVAMAPPER flag
#define LHSURF_MBCONTROLBUFF            0x00000008      // Needs to stay in sync with DXVAMAPPER flag
#define LHSURF_RESIDUALDIFFBUFF         0x00000010      // Needs to stay in sync with DXVAMAPPER flag
#define LHSURF_DEBLOCKBUFF              0x00000020      // Needs to stay in sync with DXVAMAPPER flag
#define LHSURF_INVERSEQUANTBUFF         0x00000040      // Needs to stay in sync with DXVAMAPPER flag
#define LHSURF_SLICEBUFF                0x00000080      // Needs to stay in sync with DXVAMAPPER flag
#define LHSURF_BITSTREAMBUFF            0x00000100      // Needs to stay in sync with DXVAMAPPER flag
#define LHSURF_SYSMEMALLOCATED          0x00000200      // Fake vidmem surface created while device lost
#define LHSURF_TREATASOUSTANDINGVIDMEM  0x00000400      // Used for DoVidmemSurfacesExist check
#define LHSURF_CREATEDWHILEDEVICELOST   0x00000800      // Surface created while device is lost
#define LHSURF_NONSECURE                0x00001000      // Allow surface to be opened by non-secure processes
#define LHSURF_DESTROYEDINRESETDEVICE   0x00002000      // Track which surfaces were invalidated due to our device lost handling
#define LHSURF_NTSHARING                0x00004000      // NT Handle sharing (introduced for Flip Model backbuffers)
#define LHSURF_DEFERREDFENCEWAITS       0x00008000      // Used for Flip Model backbuffers which can be used with deferred fence waits
#define LHSURF_UNSYNCHRONIZEDFLIPS      0x00010000      // Used to indicate that this resource can be flipped immediately in dFlip or iFlip
#define LHSURF_HIDDENPRIMARY            0x00020000      // Kernel needs to see primary flag but not the UMD.

typedef struct _DDSURFHANDLE
{
    DWORD                       dwCookie;   // CreateSurfaceEx handle
    HANDLE                      hSurface;   // Kernel mode surface handle
    HANDLE                      hSharedHandle; // Cross process surface handle for redirection
    D3DPOOL                     Pool;       // Location of surface
    D3DFORMAT                   Format;
    D3DRESOURCETYPE             Type;       // What kind of surface it is
    ULONG_PTR                   fpVidMem;
    DWORD                       dwLinearSize;
    LONG                        lPitch;
    LPDDRAWI_DDRAWSURFACE_LCL   pLcl;
    PDDDEVICEHANDLE             pDevice;
    PDDDEVICEHANDLE             pDeviceMaster; // Used only with additional primary surface
    DWORD                       dwFlags;
    DWORD                       dwWidth;
    DWORD                       dwHeight;
    DWORD                       dwDepth;
    LONG                        lSlicePitch; // Offset to next slice for volume texture
    struct _DDSURFHANDLE*       pNext;
    struct _DDSURFHANDLE*       pPrevious;
    UINT                        LockRefCnt;
    HDC                         hDC;
    HBITMAP                     hBitmap;

#ifdef DEBUG
    DWORD                       dwUsage;
#endif // DEBUG
} DDSURFHANDLE, * PDDSURFHANDLE;

typedef struct _DEFERREDCREATE
{
    D3D9_CREATESURFACEDATA      CreateData;
    BOOL                        bLightWeight;
    struct _DEFERREDCREATE     *pNext;
} DEFERREDCREATE, *PDEFERREDCREATE;

#define DDSURF_SYSMEMALLOCATED      0x00000001
#define DDSURF_DEFERCREATEEX        0x00000002
#define DDSURF_HAL                  0x00000004
#define DDSURF_SOFTWARE             0x00000008
#define DDSURF_CREATECOMPLETE       0x00000010
#define DDSURF_TREATASVIDMEM        0x00000020      // Flag to indicate that surf should
                                                    // be treated as vid-mem for the
                                                    // "do vid-mem surfaces exist" case
#define DDSURF_ADDITIONALPRIMARY    0x00000040
#define DDSURF_LHHANDLE             0x00000100

#define IS_SOFTWARE_DRIVER(x)                                       \
    (((PDDDEVICEHANDLE)(x))->pDD != NULL)

#define IS_SOFTWARE_DRIVER_SURFACE(x)                               \
    (((PDDSURFHANDLE)(x))->dwFlags & DDSURF_SOFTWARE)

#define IS_SURFACE_LOOSABLE(x)                                      \
    (!IS_SOFTWARE_DRIVER_SURFACE(x) &&                              \
    ((((PDDSURFHANDLE)(x))->Pool == D3DPOOL_LOCALVIDMEM) ||        \
    (((PDDSURFHANDLE)(x))->Pool == D3DPOOL_NONLOCALVIDMEM)))

__inline HANDLE GetSurfHandle(HANDLE hSurface)
{
    if(hSurface)
    {
        return(((PDDSURFHANDLE)hSurface)->hSurface);
    }
    return NULL;
}

__inline D3DRESOURCETYPE GetSurfType(HANDLE hSurface)
{
    if(hSurface)
    {
        return(((PDDSURFHANDLE)hSurface)->Type);
    }
    return (D3DRESOURCETYPE) 0;
}

typedef enum _DXVATYPE {
    DXVATYPE_DECODE         = 1,
    DXVATYPE_DEINTERLACE    = 2,
    DXVATYPE_PROCAMP        = 3,
    DXVATYPE_CONTAINER      = 4,
    DXVATYPE_EXTENSION      = 5,
    DXVATYPE_FORCE_DWORD    = 0x7fffffff, /* force 32-bit size enum */
} DXVATYPE;

typedef struct _DXVAMAPPER
{
    UINT                          Flags;
    DXVATYPE                      Type;
    HANDLE                        hDxva;
    HRESULT                       PreviousError;
    GUID                          Guid;
    UINT                          Width;
    UINT                          Height;
    D3DFORMAT                     Format;
    DXVADDI_CONFIGPICTUREDECODE   LockedConfig;
    DXVADDI_DECODEBUFFERDESC      PictureParamsHeader;
    HANDLE                        hPictureParamsResource;
    UINT                          PictureParamsResourceIndex;
    DXVADDI_DECODEBUFFERDESC      MBControlHeader;
    HANDLE                        hMBControlResource;
    UINT                          MBControlSubResourceIndex;
    DXVADDI_DECODEBUFFERDESC      ResidualDiffHeader;
    HANDLE                        hResidualDiffResource;
    UINT                          ResidualDiffSubResourceIndex;
    DXVADDI_DECODEBUFFERDESC      DeblockingControlHeader;
    HANDLE                        hDeblockingControlResource;
    UINT                          DeblockingControlSubResourceIndex;
    DXVADDI_DECODEBUFFERDESC      InverseQuantizationHeader;
    HANDLE                        hInverseQuantizationResource;
    UINT                          InverseQuantizationSubResourceIndex;
    DXVADDI_DECODEBUFFERDESC      SliceControlHeader;
    HANDLE                        hSliceControlResource;
    UINT                          SliceControlSubResourceIndex;
    DXVADDI_DECODEBUFFERDESC      BitStreamHeader;
    HANDLE                        hBitStreamResource;
    UINT                          BitStreamSubResourceIndex;
    DXVADDI_VIDEOSAMPLE*          pDeinterlaceSampleBuffer;
    UINT                          DeinterlaceSampleBufferSize;
} DXVAMAPPER;

#define DXVAFLAG_NODEVICE        0x00000001
#define DXVAFLAG_PICPARAMSSET    LHSURF_PICPARAMS
#define DXVAFLAG_MBCONTROLSET    LHSURF_MBCONTROLBUFF
#define DXVAFLAG_RESIDUALDIFFSET LHSURF_RESIDUALDIFFBUFF
#define DXVAFLAG_DEBLOCKSET      LHSURF_DEBLOCKBUFF
#define DXVAFLAG_INVERSEQUANTSET LHSURF_INVERSEQUANTBUFF
#define DXVAFLAG_SLICESET        LHSURF_SLICEBUFF
#define DXVAFLAG_BITSTREAMSET    LHSURF_BITSTREAMBUFF
#define DXVAFLAG_CONFIGLOCKED    0x00000200

#define DXVAFLAG_BUFFERMASK      (DXVAFLAG_PICPARAMSSET | DXVAFLAG_MBCONTROLSET | DXVAFLAG_RESIDUALDIFFSET | \
                                  DXVAFLAG_DEBLOCKSET | DXVAFLAG_INVERSEQUANTSET | DXVAFLAG_SLICESET | DXVAFLAG_BITSTREAMSET)

#define _FACD3D  0x876
#define MAKE_D3DHRESULT( code )  MAKE_HRESULT( 1, _FACD3D, code )
#define MAKE_D3DSTATUS( code )  MAKE_HRESULT( 0, _FACD3D, code )

//
// !!! Make sure that these values are the same as in d3d9.w !!!
//

#define D3DERR_WASSTILLDRAWING                  MAKE_D3DHRESULT(540)
#define D3DERR_DEVICELOST                       MAKE_D3DHRESULT(2152)
#define D3DERR_DEVICEREMOVED                    MAKE_D3DHRESULT(2160)
#define D3DERR_DRIVERINTERNALERROR              MAKE_D3DHRESULT(2087)
#define D3DERR_NOTAVAILABLE                     MAKE_D3DHRESULT(2154)
#define D3DERR_OUTOFVIDEOMEMORY                 MAKE_D3DHRESULT(380)
#define D3DERR_DEFERRED_DP2ERROR                MAKE_D3DHRESULT(2158)
#define S_PRESENT_MODE_CHANGED                  MAKE_D3DSTATUS(2167)
#define S_PRESENT_OCCLUDED                      MAKE_D3DSTATUS(2168)
#define D3DERR_DEVICEHUNG                       MAKE_D3DHRESULT(2164)

// Function protoptypes

extern LPDDRAWI_DIRECTDRAW_LCL SwDDICreateDirectDraw( void);
extern void SwDDIMungeCaps (HINSTANCE hLibrary, HANDLE hDD, PD3D9_DRIVERCAPS pDriverCaps, PD3D9_CALLBACKS pCallbacks, LPDDSURFACEDESC, UINT*, D3DQUERYTYPE*, UINT*, VOID* pSwInitFunction);
extern LPDDRAWI_DDRAWSURFACE_LCL SwDDIBuildHeavyWeightSurface (LPDDRAWI_DIRECTDRAW_LCL, PD3D9_CREATESURFACEDATA pCreateSurface, DD_SURFACE_LOCAL* pSurfaceLocal, DD_SURFACE_GLOBAL* pSurfaceGlobal, DD_SURFACE_MORE* pSurfaceMore, DWORD index);
extern void SwDDICreateSurfaceEx(LPDDRAWI_DIRECTDRAW_LCL pDrv, LPDDRAWI_DDRAWSURFACE_LCL pLcl);
extern void SwDDIAttachSurfaces (LPDDRAWI_DDRAWSURFACE_LCL pFrom, LPDDRAWI_DDRAWSURFACE_LCL pTo);
extern HRESULT SwDDICreateSurface( PD3D9_CREATESURFACEDATA pCreateSurface, DD_SURFACE_LOCAL* pDDSurfaceLocal, DD_SURFACE_GLOBAL* pDDSurfaceGlobal, DD_SURFACE_MORE*  pDDSurfaceMore);
extern void AddUnknownZFormats( UINT NumFormats, DDPIXELFORMAT* pFormats, UINT* pNumUnknownFormats, D3DFORMAT* pUnknownFormats);
extern DWORD SwDDILock( HANDLE hDD, PDDSURFHANDLE   pSurf, DD_LOCKDATA* pLockData);
extern DWORD SwDDIUnlock( HANDLE hDD, PDDSURFHANDLE   pSurf, DD_UNLOCKDATA* pUnlockData);
extern DWORD SwDDIDestroySurface( HANDLE hDD, PDDSURFHANDLE pSurf);
extern HRESULT MapLegacyResult(HRESULT hr);
extern HRESULT CreateDeviceLHDDI(HDC hDC, PDDDEVICEHANDLE pDeviceHandle, BOOL bDeferTrimRegistration);
extern void DestroyDeviceLHDDI(PDDDEVICEHANDLE pDevice, BOOL bDDIOnly);
extern BOOL QueryLHDDICaps(__inout PDDDEVICEHANDLE         pDevice,
                           HINSTANCE                       hLibrary,
                           __inout PD3D9_DRIVERCAPS        pDriverCaps,
                           __inout PD3D9_CALLBACKS         pCallbacks,
                           __in LPSTR                      pDeviceName,
                           __out_opt LPDDSURFACEDESC       pTextureFormats,
                           __out_opt D3DDISPLAYMODE*       pExtendedModeFormats,
                           __out_opt D3DQUERYTYPE*         pQueries,
                           __inout UINT*                   pcTextureFormats,
                           __inout UINT*                   pcExtendedModeFormats,
                           __inout UINT*                   pcQueries,
                           BOOL                            bUpdateDisplayModeOnly);
extern DWORD GetDriverInfo2LH(DWORD* pdwDrvRet, PDDDEVICEHANDLE pDevice, DWORD Type, DWORD Size, void* pBuffer);
extern HRESULT BuildModeTableLH(PD3D9_DEVICEDATA pDD, D3DDISPLAYMODEEX** ppModeTable, DWORD* pNumEntries, DWORD* pNumUnprunedEntries);
extern HRESULT APIENTRY DdCreateSurfaceLH(PD3D9_CREATESURFACEDATA pCreateSurface);
extern HRESULT APIENTRY DdDestroySurfaceLH(PD3D8_DESTROYSURFACEDATA pDestroySurface);
extern DWORD APIENTRY DdBltLH(PD3D8_BLTDATA pBlt);
extern HRESULT APIENTRY DdLockLH(PD3D8_LOCKDATA pLock);
extern HRESULT APIENTRY DdUnlockLH(PD3D8_UNLOCKDATA pUnlock);
extern DWORD APIENTRY DdFlipLH(PD3D8_FLIPDATA pFlip);
extern VOID LoseDeviceLH(PDDDEVICEHANDLE pDeviceHandle);
extern VOID LoseDeviceWithReasonLH(PDDDEVICEHANDLE pDeviceHandle, D3DKMT_DEVICE_ERROR_REASON Reason);
extern BOOL CanRestoreNowLH(PDDDEVICEHANDLE pDeviceHandle);
extern VOID RestoreDeviceLH(PDDDEVICEHANDLE pDevice);
extern BOOL DdSetGammaRampLH(PDDDEVICEHANDLE pDevice, PDDDEVICEHANDLE pMaster, LPVOID pGammaRamp);
extern HRESULT APIENTRY D3D9SetDisplayModeLH(HANDLE hDevice, HANDLE hMaster, HANDLE hPrimarySurf);

extern HRESULT APIENTRY DdCreateSurface(PD3D9_CREATESURFACEDATA pCreateSurface);
extern DWORD APIENTRY DdDestroySurface(PD3D8_DESTROYSURFACEDATA pDestroySurface);
extern HRESULT APIENTRY DdLock(PD3D8_LOCKDATA pLock);
extern HRESULT APIENTRY DdUnlock(PD3D8_UNLOCKDATA pUnlock);
extern DWORD WINAPI DdBlt(PD3D8_BLTDATA pBlt);
extern DWORD APIENTRY DdFlip(PD3D8_FLIPDATA pFlip);

extern BOOL CheckForDeviceLost (HANDLE hDD);

#endif // !WIN95

#endif // __DDITHUNK_INCLUDED__

