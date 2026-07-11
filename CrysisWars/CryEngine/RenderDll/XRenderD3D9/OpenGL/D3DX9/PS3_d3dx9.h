//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//  File:       d3dx9math.h
//  Content:    D3DX math types and functions
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _PS3_D3DX9_H__
#define _PS3_D3DX9_H__

/*#if defined(PS3) || defined(LINUX)
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#else
#include "../Mygl.h"
#endif

#if defined(PS3)
// Cg stuff already included
#elif defined(LINUX)
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#else
#include "../cg/cg.h"
#include "../cg/cgGL.h"
#endif
*/

#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define PURE                    = 0
#define THIS_
#define THIS                    void
#define interface struct  

#define D3DX_DEFAULT            ((UINT) -1)

#define D3D_SDK_VERSION 10
#define DIRECT3D_VERSION 0x0900

#define D3DADAPTER_DEFAULT                     0

#define _FACD3D  0x876
#define MAKE_D3DHRESULT( code )  MAKE_HRESULT( 1, _FACD3D, code )
#define MAKE_D3DSTATUS( code )  MAKE_HRESULT( 0, _FACD3D, code )

#define D3DERR_WRONGTEXTUREFORMAT               MAKE_D3DHRESULT(2072)
#define D3DERR_UNSUPPORTEDCOLOROPERATION        MAKE_D3DHRESULT(2073)
#define D3DERR_UNSUPPORTEDCOLORARG              MAKE_D3DHRESULT(2074)
#define D3DERR_UNSUPPORTEDALPHAOPERATION        MAKE_D3DHRESULT(2075)
#define D3DERR_UNSUPPORTEDALPHAARG              MAKE_D3DHRESULT(2076)
#define D3DERR_TOOMANYOPERATIONS                MAKE_D3DHRESULT(2077)
#define D3DERR_CONFLICTINGTEXTUREFILTER         MAKE_D3DHRESULT(2078)
#define D3DERR_UNSUPPORTEDFACTORVALUE           MAKE_D3DHRESULT(2079)
#define D3DERR_CONFLICTINGRENDERSTATE           MAKE_D3DHRESULT(2081)
#define D3DERR_UNSUPPORTEDTEXTUREFILTER         MAKE_D3DHRESULT(2082)
#define D3DERR_CONFLICTINGTEXTUREPALETTE        MAKE_D3DHRESULT(2086)
#define D3DERR_DRIVERINTERNALERROR              MAKE_D3DHRESULT(2087)

#define D3DERR_NOTFOUND                         MAKE_D3DHRESULT(2150)
#define D3DERR_MOREDATA                         MAKE_D3DHRESULT(2151)
#define D3DERR_DEVICELOST                       MAKE_D3DHRESULT(2152)
#define D3DERR_DEVICENOTRESET                   MAKE_D3DHRESULT(2153)
#define D3DERR_NOTAVAILABLE                     MAKE_D3DHRESULT(2154)
#define D3DERR_OUTOFVIDEOMEMORY                 MAKE_D3DHRESULT(380)
#define D3DERR_INVALIDDEVICE                    MAKE_D3DHRESULT(2155)
#define D3DERR_INVALIDCALL                      MAKE_D3DHRESULT(2156)
#define D3DERR_DRIVERINVALIDCALL                MAKE_D3DHRESULT(2157)
#define D3DERR_WASSTILLDRAWING                  MAKE_D3DHRESULT(540)
#define D3DOK_NOAUTOGEN                         MAKE_D3DSTATUS(2159)

#define D3DX_FILTER_NONE             (1 << 0)
#define D3DX_FILTER_POINT            (2 << 0)
#define D3DX_FILTER_LINEAR           (3 << 0)
#define D3DX_FILTER_TRIANGLE         (4 << 0)
#define D3DX_FILTER_BOX              (5 << 0)

#define _FACDD  0x876
#define MAKE_DDHRESULT( code )  MAKE_HRESULT( 1, _FACDD, code )

#define D3DXERR_CANNOTMODIFYINDEXBUFFER         MAKE_DDHRESULT(2900)
#define D3DXERR_INVALIDMESH                     MAKE_DDHRESULT(2901)
#define D3DXERR_CANNOTATTRSORT                  MAKE_DDHRESULT(2902)
#define D3DXERR_SKINNINGNOTSUPPORTED            MAKE_DDHRESULT(2903)
#define D3DXERR_TOOMANYINFLUENCES               MAKE_DDHRESULT(2904)
#define D3DXERR_INVALIDDATA                     MAKE_DDHRESULT(2905)
#define D3DXERR_LOADEDMESHASNODATA              MAKE_DDHRESULT(2906)
#define D3DXERR_DUPLICATENAMEDFRAGMENT          MAKE_DDHRESULT(2907)
#define D3DXERR_CANNOTREMOVELASTITEM            MAKE_DDHRESULT(2908)

typedef void*  D3DSURFACE_PARAMETERS;
#define BUFFER_OFFSET(i) ((char *) NULL + (i))

#if defined(PS3) || defined(LINUX)

//bits to determine if it is a matrix or not
static const int scParam2x4MatrixBit	= (1 << 31);
static const int scIs2x4Matrix				= 1;
static const int scParam3x4MatrixBit	= (1 << 30);
static const int scIs3x4Matrix				= 2;
static const int scParam4x4MatrixBit	= (1 << 29);
static const int scIs4x4Matrix				= 4;
static const int scParamArrayBit			= (1 << 28);
static const int scIsArray						= 8;
static const int scParamIsMatrixMask	= (scParam2x4MatrixBit | scParam3x4MatrixBit | scParam4x4MatrixBit);
static const int scParamMask	= (scParamArrayBit | scParamIsMatrixMask);
#define STDMETHODCALLTYPE

#define MAKE_HRESULT(sev,fac,code) \
  ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

#define E_FAIL                           0x80004005
#define E_OUTOFMEMORY                    0x80000002
#define E_INVALIDARG                     0x80070057

#define S_OK    ((HRESULT)0x00000000L)
#define S_FALSE ((HRESULT)0x00000001L)

// Severity codes.
#undef SEVERITY_SUCCESS
#define SEVERITY_SUCCESS    0
#undef SEVERITY_ERROR
#define SEVERITY_ERROR      1

// Facility codes.
#undef FACILITY_WINDOWS_CE              
#define FACILITY_WINDOWS_CE              24
#undef FACILITY_WINDOWS                 
#define FACILITY_WINDOWS                 8
#undef FACILITY_URT                     
#define FACILITY_URT                     19
#undef FACILITY_UMI                     
#define FACILITY_UMI                     22
#undef FACILITY_SXS                     
#define FACILITY_SXS                     23
#undef FACILITY_STORAGE                 
#define FACILITY_STORAGE                 3
#undef FACILITY_STATE_MANAGEMENT        
#define FACILITY_STATE_MANAGEMENT        34
#undef FACILITY_SSPI                    
#define FACILITY_SSPI                    9
#undef FACILITY_SCARD                   
#define FACILITY_SCARD                   16
#undef FACILITY_SETUPAPI                
#define FACILITY_SETUPAPI                15
#undef FACILITY_SECURITY                
#define FACILITY_SECURITY                9
#undef FACILITY_RPC                     
#define FACILITY_RPC                     1
#undef FACILITY_WIN32                   
#define FACILITY_WIN32                   7
#undef FACILITY_CONTROL                 
#define FACILITY_CONTROL                 10
#undef FACILITY_NULL                    
#define FACILITY_NULL                    0
#undef FACILITY_METADIRECTORY           
#define FACILITY_METADIRECTORY           35
#undef FACILITY_MSMQ                    
#define FACILITY_MSMQ                    14
#undef FACILITY_MEDIASERVER             
#define FACILITY_MEDIASERVER             13
#undef FACILITY_INTERNET                
#define FACILITY_INTERNET                12
#undef FACILITY_ITF                     
#define FACILITY_ITF                     4
#undef FACILITY_HTTP                    
#define FACILITY_HTTP                    25
#undef FACILITY_DPLAY                   
#define FACILITY_DPLAY                   21
#undef FACILITY_DISPATCH                
#define FACILITY_DISPATCH                2
#undef FACILITY_CONFIGURATION           
#define FACILITY_CONFIGURATION           33
#undef FACILITY_COMPLUS                 
#define FACILITY_COMPLUS                 17
#undef FACILITY_CERT                    
#define FACILITY_CERT                    11
#undef FACILITY_BACKGROUNDCOPY          
#define FACILITY_BACKGROUNDCOPY          32
#undef FACILITY_ACS                     
#define FACILITY_ACS                     20
#undef FACILITY_AAF                     
#define FACILITY_AAF                     18

#undef FAILED
#undef SUCCEEDED
#define FAILED(Status) ((HRESULT)(Status)<0)
#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0)

#define CONST const

typedef struct tagPALETTEENTRY {
  BYTE        peRed;
  BYTE        peGreen;
  BYTE        peBlue;
  BYTE        peFlags;
} PALETTEENTRY, *PPALETTEENTRY;

typedef BYTE *LPBYTE;
typedef void* RGNDATA;

#define TEXT(quote) quote

typedef struct SDevicemode
{
  DWORD  dmSize;
  DWORD  dmBitsPerPel;
  DWORD  dmPelsWidth;
  DWORD  dmPelsHeight;
  DWORD  dmDisplayFrequency;
} DEVMODE;

#endif // PS3 || LINUX

#define D3D_OK                              S_OK

/****************************************************************************
*
* Flags for IDirect3D9::CreateDevice's BehaviorFlags
*
****************************************************************************/
#define D3DCREATE_FPU_PRESERVE                  0x00000002L
#define D3DCREATE_MULTITHREADED                 0x00000004L

#define D3DCREATE_PUREDEVICE                    0x00000010L
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING     0x00000020L
#define D3DCREATE_HARDWARE_VERTEXPROCESSING     0x00000040L
#define D3DCREATE_MIXED_VERTEXPROCESSING        0x00000080L

#define D3DCREATE_DISABLE_DRIVER_MANAGEMENT     0x00000100L
#define D3DCREATE_ADAPTERGROUP_DEVICE           0x00000200L
#define D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX  0x00000400L

#include "PS3_d3d9types.h"
#include "PS3_d3d9caps.h"
#include "PS3_d3dx9math.h"

//===========================================================================
// D3D9 objects

struct IPS3Unknown
{
  ULONG m_nRef;

  IPS3Unknown()
  {
    m_nRef = 1;
  }
  virtual ~IPS3Unknown() {}
  virtual ULONG AddRef() { m_nRef++; return m_nRef; };
  virtual ULONG Release()
  {
    m_nRef--;
    if (m_nRef)
      return m_nRef;
    delete this;
    return 0;
  }
};

struct IDirect3DResource9 : public IPS3Unknown
{
  D3DRESOURCETYPE m_nType;

  STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS)
  {
    return m_nType;
  }
};

struct IDirect3DSurface9 : public IDirect3DResource9
{
  D3DFORMAT m_Format;
  int m_Width;
  int m_Height;
  D3DPOOL m_Pool;
  int m_Usage;
  int m_nLevel;
  struct IDirect3DBaseTexture9 *m_pTex;
  void *m_pLockedData;

	// The name of the renderbuffer.  0 if the framebuffer is 0.
  int m_nID;

  IDirect3DSurface9();
  virtual ~IDirect3DSurface9();

  STDMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc);
  STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
  STDMETHOD(UnlockRect)(THIS);
};

struct SGLTexState
{
  int nAddressU;
  int nAddressV;
  int nAddressW;

  int nD3DMinFilter;
  int nD3DMagFilter;
  int nD3DMipFilter;

  int nAnisotropy;

  IDirect3DBaseTexture9 *m_pTex;
};


struct IDirect3DBaseTexture9 : public IDirect3DResource9
{
  DWORD m_nLevelCount;
  DWORD m_nWidth;
  DWORD m_nHeight;
  uint m_nID;
  D3DPOOL m_Pool;
  int m_Usage;
  D3DTEXTUREFILTERTYPE m_nFilterType;
  D3DFORMAT m_Format;
  ETEX_Type m_eTT;
  int m_nGLtarget;
  SGLTexState m_CurState;
  int m_nLod;

  IDirect3DBaseTexture9();

  DWORD GetLevelCount() { return m_nLevelCount; }
  HRESULT SetAutoGenFilterType (D3DTEXTUREFILTERTYPE FilterType);
  void SetLOD(DWORD LodNew) { m_nLod = LodNew; }
};

struct IDirect3DTexture9 : public IDirect3DBaseTexture9
{
  byte **m_pLockedData;

  IDirect3DTexture9()
  {
    m_pLockedData = NULL;
  }

  virtual ~IDirect3DTexture9();

  STDMETHOD(GetLevelDesc)(THIS_ UINT Level,D3DSURFACE_DESC *pDesc);
  STDMETHOD(LockRect)(THIS_ UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
  STDMETHOD(UnlockRect)(THIS_ UINT Level);
  STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level,IDirect3DSurface9** ppSurfaceLevel);
};

struct IDirect3DCubeTexture9 : public IDirect3DBaseTexture9
{
  byte **m_pLockedData;

  IDirect3DCubeTexture9()
  {
    m_pLockedData = NULL;
  }

  virtual ~IDirect3DCubeTexture9();

  STDMETHOD(GetLevelDesc)(THIS_ UINT Level,D3DSURFACE_DESC *pDesc);
  STDMETHOD(LockRect)(THIS_ D3DCUBEMAP_FACES FaceType,UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
  STDMETHOD(UnlockRect)(THIS_ D3DCUBEMAP_FACES FaceType,UINT Level);
  STDMETHOD(GetCubeMapSurface)(THIS_ D3DCUBEMAP_FACES FaceType,UINT Level,IDirect3DSurface9** ppCubeMapSurface);
};

struct IDirect3DVolumeTexture9 : public IDirect3DBaseTexture9
{
  byte **m_pLockedData;
  int m_nDepth;

  IDirect3DVolumeTexture9()
  {
    m_nDepth = 0;
    m_pLockedData = NULL;
  }

  virtual ~IDirect3DVolumeTexture9();

  STDMETHOD(LockBox)(THIS_ UINT Level,D3DLOCKED_BOX* pLockedVolume,CONST D3DBOX* pBox,DWORD Flags);
  STDMETHOD(UnlockBox)(THIS_ UINT Level);
};

#define FSM_POSITION     0
#define FSM_BLENDWEIGHT  1
#define FSM_NORMAL       2
#define FSM_COLOR0       3
#define FSM_COLOR1       4
#define FSM_TESSFACTOR   5
#define FSM_PSIZE        6
#define FSM_BLENDINDICES 7
#define FSM_TEXCOORD0    8
#define FSM_TEXCOORD1    9
#define FSM_TEXCOORD2    10
#define FSM_TEXCOORD3    11
#define FSM_TEXCOORD4    12
#define FSM_TEXCOORD5    13
#define FSM_TEXCOORD6    14	
#define FSM_TEXCOORD7    15 
//take special care for tangent and binormal stream for now
//they map to a stream index outside 0..15 but are converted for each relevant call
#define FSM_TANGENT      16	//add a special define for usage index otherwise we dont know anymore what is what
#define FSM_BINORMAL     17
#define FSM_TANGENT_USAGE_INDEX 14	//pay attention possible conflict with FSM_TEXCOORD6 here
#define FSM_BINORMAL_USAGE_INDEX 15 //pay attention possible conflict with FSM_TEXCOORD7 here

#define FSM_MAX_STREAM_INDEX 17


struct IDirect3DVertexBuffer9 : public IDirect3DResource9
{
  uint m_nID;
  int m_nLength;
  int m_Usage;
  int m_Pool;
  int m_nStreamMask;
  bool m_bLocked;

  IDirect3DVertexBuffer9()
  {
    m_nID = 0;
    m_nLength = 0;
    m_Usage = 0;
    m_Pool = 0;
    m_nStreamMask = FSM_POSITION;
    m_bLocked = false;
  }
  virtual ~IDirect3DVertexBuffer9();

  STDMETHOD(Lock)(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags);
  STDMETHOD(Unlock)(THIS);
  STDMETHOD(GetDesc)(THIS_ D3DVERTEXBUFFER_DESC *pDesc);
};

struct IDirect3DIndexBuffer9 : public IDirect3DResource9
{
  uint m_nID;
  int m_nLength;
  int m_Usage;
  bool m_bLocked;

  IDirect3DIndexBuffer9()
  {
    m_nID = 0;
    m_nLength = 0;
    m_Usage = 0;
    m_bLocked = false;
  }
  virtual ~IDirect3DIndexBuffer9();

  STDMETHOD(Lock)(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags);
  STDMETHOD(Unlock)(THIS);
  STDMETHOD(GetDesc)(THIS_ D3DINDEXBUFFER_DESC *pDesc);
};

struct IDirect3DVertexDeclaration9 : public IPS3Unknown
{
  int m_nStreamMask;
  TArray<D3DVERTEXELEMENT9> m_Declaration;
};

struct IDirect3DBaseShader9 : public IDirect3DResource9
{
  uint m_nID;
  IDirect3DBaseShader9()
  {
    m_nID = 0;
  }

  virtual ~IDirect3DBaseShader9();
};

struct IDirect3DVertexShader9 : public IDirect3DBaseShader9
{
	int m_streamId[16];
  IDirect3DVertexShader9()
  {
		for (int i = 0; i < 16; ++i)
			m_streamId[i] = -1;
  }
  virtual ~IDirect3DVertexShader9();
};

struct IDirect3DPixelShader9 : public IDirect3DBaseShader9
{
  IDirect3DPixelShader9()
  {
  }
  virtual ~IDirect3DPixelShader9();
};

struct IDirect3DQuery9 : public IPS3Unknown
{
  virtual ~IDirect3DQuery9();

  STDMETHOD(Issue)(THIS_ DWORD dwIssueFlags);
  STDMETHOD(GetData)(THIS_ void* pData,DWORD dwSize,DWORD dwGetDataFlags);
};

struct IDirect3DSwapChain9
{
};

struct IDirect3DStateBlock9
{
};

enum eD3DXType
{
  eD3DXBufType_Unknown,
  eD3DXBufType_CGProg,
  eD3DXBufType_Buf,
};

struct ID3DXBuffer : public IPS3Unknown
{
  eD3DXType m_eType;
  void *m_pBuf;
  int m_nSize;

  ID3DXBuffer()
  {
    m_pBuf = NULL;
    m_nSize = 0;
    m_eType = eD3DXBufType_Buf;
  }
  virtual ~ID3DXBuffer();

  STDMETHOD_(LPVOID, GetBufferPointer)(THIS)
  {
    return m_pBuf;
  }
  STDMETHOD_(DWORD, GetBufferSize)(THIS)
  {
    return m_nSize;
  }
};

struct IDirect3D9
{
  friend class CRenderer;

public:
  HMODULE m_hLibHandle;
  HMODULE m_hLibHandleGDI;
  const char *m_szVendorName;
  const char *m_szRendererName;
  const char *m_szVersionName;
  const char *m_szExtensionsName;

  TArray<SRendContext *> m_RContexts;
  SRendContext *m_pCurrContext;
	RECT m_CurScissorRect;
  TArray<DEVMODE> m_VidModes;

  int m_nNumAdapters;
  D3DADAPTER_IDENTIFIER9 m_AdapterIdent;
  D3DCAPS9 m_DeviceCaps;

  bool CreateRContext(SRendContext *rc, D3DFORMAT fmtBack, D3DFORMAT fmtZ, bool bAllowFSAA);

#if defined(PS3)
  bool PS3_SetupWindow(int nFSAA);
  bool PS3_MakeNewContext(int nFSAA);
  bool PS3_ResetContext(int nFSAA);
#elif defined(LINUX)
  bool LINUX_SetupWindow();
#else
  bool SetupPixelFormat(D3DFORMAT fmtBack, D3DFORMAT fmtZ, SRendContext *rc);
  bool LoadOpenGLLibrary();
  bool FreeOpenGLLibrary();
#endif

#if !defined(PS3)
  bool CheckOGLExtensions();
  bool CheckGammaSupport();

  bool FindExt(const char* Name);
  void FindProc (void*& ProcAddress, char* Name, char* SupportName, byte& Supports, bool bAllowExt);
  void FindProcs(bool bAllowExt);
#endif

  bool OpenGLExtensions2D3D9Caps();

public:
  ~IDirect3D9();
  int Release();

  STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter,D3DDISPLAYMODE* pMode);
  STDMETHOD(CreateDevice)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,struct IDirect3DDevice9** ppReturnedDeviceInterface);
  STDMETHOD_(UINT, GetAdapterCount)(THIS);
  STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels);
  STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat);
  STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier);
  STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter,D3DFORMAT Format);
  STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode);
  STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps);
  STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed);
  STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat);
};

struct SGLStreamData
{
  IDirect3DVertexBuffer9 *m_pBuf;
  int m_nOffset;
  int m_nStride;
  byte m_bDirty   : 1;
  byte m_bTouched : 1;
};

struct IDirect3DDevice9 : public IPS3Unknown
{
  IDirect3D9* m_pD3D;
  D3DPRESENT_PARAMETERS m_PresentParams;

  int m_nCurTMU;
  float m_fAT_ReqVal;
  int m_nAT_ReqOp;

  int m_nAB_Src;
  int m_nAB_Dst;

  int m_nCP_Mask;

  int m_nSTENC_ref;
  int m_nSTENC_mask;
  int m_nSTENC_func;
  int m_nSTENC_OpFail;
  int m_nSTENC_OpZFail;
  int m_nSTENC_OpPass;

  IDirect3DVertexDeclaration9 *m_pCurVDeclaration;
  IDirect3DIndexBuffer9 *m_pCurIB;
  int m_nLastVertexIndex;
  int m_nCurStreamMask;
  SGLStreamData m_CurStream[MAX_STREAMS];
  int m_nCurrentVB;
  int m_nCurrentIB;

	IDirect3DVertexShader9 *m_pCurVShader;

  SGLTexState m_ReqTS[MAX_TMU];

	GLuint m_nCurFBO; // current framebuffer object
	GLuint m_nCurFBOZ;
	IDirect3DSurface9 *m_pCurTarget[4];
	RECT m_CurScissorRect;
	IDirect3DSurface9 *m_pCurZStencil;
	IDirect3DSurface9 *m_pBackBuffer;

  int m_bAT_Dirty : 1;
  int m_bAB_Dirty : 1;
  int m_bAT_Enabled : 1;
  int m_bAB_Enabled : 1;
  int m_bSTENC_Dirty : 1;
	int m_bVA_Enabled : 1;
	int m_bTCA_Enabled : 1;
	int m_bNA_Enabled : 1;
	int m_bCA_Enabled : 1;
	int m_bDT_Enabled : 1;

	D3DVIEWPORT9 m_CurViewport;

  IDirect3DDevice9();
  virtual ~IDirect3DDevice9();
  void SelectTMU(int nTMU);
  HRESULT Commit(int nFirstVertex);
  void SetVertexAttrib(int i, int nMask);


  /*** IDirect3DDevice9 methods ***/
  STDMETHOD(TestCooperativeLevel)(THIS);
  STDMETHOD_(UINT, GetAvailableTextureMem)(THIS);
  STDMETHOD(EvictManagedResources)(THIS);
  STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9);
  STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps);
  STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode);
  STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters);
  STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap);

  STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
  STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion);
  STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);
  STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus);
  STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp);
  STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp);
  STDMETHOD(CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
  STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle);
  STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle);
  STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
  STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
  STDMETHOD(CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,D3DSURFACE_PARAMETERS* pSharedHandle);
  STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,D3DSURFACE_PARAMETERS* pSharedHandle);
  STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint);
  STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture);

  STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface);
  STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain,IDirect3DSurface9* pDestSurface);
  STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
  STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color);
  STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
  STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
  STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget);
  STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil);
  STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface);
  STDMETHOD(BeginScene)(THIS);
  STDMETHOD(EndScene)(THIS);
  STDMETHOD(Clear)(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
  STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
  STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport);
  STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport);
  STDMETHOD(SetClipPlane)(THIS_ DWORD Index,CONST float* pPlane);
  STDMETHOD(GetClipPlane)(THIS_ DWORD Index,float* pPlane);
  STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value);
  STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue);
  STDMETHOD(GetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9** ppTexture);
  STDMETHOD(SetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9* pTexture);
  STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
  STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue);
  STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value);
  STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect);
  STDMETHOD(GetScissorRect)(THIS_ RECT* pRect);
  STDMETHOD(SetNPatchMode)(THIS_ float nSegments);
  STDMETHOD_(float, GetNPatchMode)(THIS);
  STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
  STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount);
  STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
  STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
  STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl);
  STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl);
  STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl);
  STDMETHOD(SetFVF)(THIS_ DWORD FVF);
  STDMETHOD(GetFVF)(THIS_ DWORD* pFVF);
  STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader);
  STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader);
  STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader);
  STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount,CONST UINT cOffset=0);
  STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount);
  STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
  STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount);
  STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
  STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
  STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride);
  STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride);
  STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT Divider);
  STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT* Divider);
  STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData);
  STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData);
  STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader);
  STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader);
  STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader);
  STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount,CONST UINT cOffset=0);
  STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount);
  STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
  STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount);
  STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
  STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
  STDMETHOD(DrawRectPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo);
  STDMETHOD(DrawTriPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo);
  STDMETHOD(DeletePatch)(THIS_ UINT Handle);
  STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery);
};

IDirect3D9 *Direct3DCreate9(UINT SDKVersion);

typedef struct IDirect3D9 *LPDIRECT3D9, *PDIRECT3D9;
typedef struct IDirect3DDevice9 *LPDIRECT3DDEVICE9, *PDIRECT3DDEVICE9;
typedef struct IDirect3DBaseTexture9 *LPDIRECT3DBASETEXTURE9, *PDIRECT3DBASETEXTURE9;
typedef struct IDirect3DTexture9 *LPDIRECT3DTEXTURE9, *PDIRECT3DTEXTURE9;
typedef struct IDirect3DCubeTexture9 *LPDIRECT3DCUBETEXTURE9, *PDIRECT3DCUBETEXTURE9;
typedef struct IDirect3DVolumeTexture9 *LPDIRECT3DVOLUMETEXTURE9, *PDIRECT3DVOLUMETEXTURE9;
typedef struct IDirect3DSurface9 *LPDIRECT3DSURFACE9, *PDIRECT3DSURFACE9;
typedef struct IDirect3DVertexDeclaration9 *LPDIRECT3DVERTEXDECLARATION9, *PDIRECT3DVERTEXDECLARATION9;
typedef struct IDirect3DVertexBuffer9 *LPDIRECT3DVERTEXBUFFER9, *PDIRECT3DVERTEXBUFFER9;
typedef struct IDirect3DIndexBuffer9 *LPDIRECT3DINDEXBUFFER9, *PDIRECT3DINDEXBUFFER9;
typedef struct IDirect3DVertexShader9 *LPDIRECT3DVERTEXSHADER9, *PDIRECT3DVERTEXSHADER9;
typedef struct IDirect3DPixelShader9 *LPDIRECT3DPIXELSHADER9, *PDIRECT3DPIXELSHADER9;

typedef interface ID3DXBuffer *LPD3DXBUFFER;
typedef interface ID3DXConstantTable ID3DXConstantTable;
typedef interface ID3DXConstantTable *LPD3DXCONSTANTTABLE;
typedef struct IDirect3DQuery9 *LPDIRECT3DQUERY9, *PDIRECT3DQUERY9;


typedef VOID (WINAPI * LPD3DXFILL2D)(D3DXVECTOR4 *pOut, 
                                    CONST D3DXVECTOR2 *pTexCoord, CONST D3DXVECTOR2 *pTexelSize, LPVOID pData);

typedef VOID (WINAPI * LPD3DXFILL3D)(D3DXVECTOR4 *pOut, 
                                    CONST D3DXVECTOR3 *pTexCoord, CONST D3DXVECTOR3 *pTexelSize, LPVOID pData);

typedef enum _D3DXIMAGE_FILEFORMAT
{
  D3DXIFF_BMP         = 0,
  D3DXIFF_JPG         = 1,
  D3DXIFF_TGA         = 2,
  D3DXIFF_PNG         = 3,
  D3DXIFF_DDS         = 4,
  D3DXIFF_PPM         = 5,
  D3DXIFF_DIB         = 6,
  D3DXIFF_HDR         = 7,       //high dynamic range formats
  D3DXIFF_PFM         = 8,       //
  D3DXIFF_FORCE_DWORD = 0x7fffffff

} D3DXIMAGE_FILEFORMAT;

//===========================================================================
// D3DX functions
HRESULT D3DXCreateTexture(
                          LPDIRECT3DDEVICE9         pDevice,
                          UINT                      Width,
                          UINT                      Height,
                          UINT                      MipLevels,
                          DWORD                     Usage,
                          D3DFORMAT                 Format,
                          D3DPOOL                   Pool,
                          LPDIRECT3DTEXTURE9*       ppTexture);


HRESULT D3DXLoadSurfaceFromSurface(
                           LPDIRECT3DSURFACE9        pDestSurface,
                           CONST PALETTEENTRY*       pDestPalette,
                           CONST RECT*               pDestRect,
                           LPDIRECT3DSURFACE9        pSrcSurface,
                           CONST PALETTEENTRY*       pSrcPalette,
                           CONST RECT*               pSrcRect,
                           DWORD                     Filter,
                           D3DCOLOR                  ColorKey);

HRESULT D3DXCreateCubeTexture(
                      LPDIRECT3DDEVICE9         pDevice,
                      UINT                      Size,
                      UINT                      MipLevels,
                      DWORD                     Usage,
                      D3DFORMAT                 Format,
                      D3DPOOL                   Pool,
                      LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

HRESULT D3DXCreateVolumeTexture(
                        LPDIRECT3DDEVICE9         pDevice,
                        UINT                      Width,
                        UINT                      Height,
                        UINT                      Depth,
                        UINT                      MipLevels,
                        DWORD                     Usage,
                        D3DFORMAT                 Format,
                        D3DPOOL                   Pool,
                        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

HRESULT D3DXLoadSurfaceFromMemory(
                          LPDIRECT3DSURFACE9        pDestSurface,
                          CONST PALETTEENTRY*       pDestPalette,
                          CONST RECT*               pDestRect,
                          LPVOID                   pSrcMemory,
                          D3DFORMAT                 SrcFormat,
                          UINT                      SrcPitch,
                          CONST PALETTEENTRY*       pSrcPalette,
                          CONST RECT*               pSrcRect,
                          DWORD                     Filter,
                          D3DCOLOR                  ColorKey);

HRESULT D3DXFillTexture(
                LPDIRECT3DTEXTURE9        pTexture,
                LPD3DXFILL2D              pFunction,
                LPVOID                    pData);

HRESULT D3DXFillVolumeTexture(
                      LPDIRECT3DVOLUMETEXTURE9  pVolumeTexture,
                      LPD3DXFILL3D              pFunction,
                      LPVOID                    pData);

HRESULT D3DXSaveSurfaceToFile(
                       LPCSTR                    pDestFile,
                       D3DXIMAGE_FILEFORMAT      DestFormat,
                       LPDIRECT3DSURFACE9        pSrcSurface,
                       CONST PALETTEENTRY*       pSrcPalette,
                       CONST RECT*               pSrcRect);

HRESULT D3DXFilterTexture(
                  LPDIRECT3DBASETEXTURE9    pBaseTexture,
                  CONST PALETTEENTRY*       pPalette,
                  UINT                      SrcLevel,
                  DWORD                     Filter);

#define D3DXFilterCubeTexture D3DXFilterTexture
#define D3DXFilterVolumeTexture D3DXFilterTexture


#include "PS3_d3dx9shader.h"

#endif // _PS3_D3DX9_H__

