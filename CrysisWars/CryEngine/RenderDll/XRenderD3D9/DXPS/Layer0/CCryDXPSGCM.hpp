#ifndef __CRYDXPSGCM__
#define __CRYDXPSGCM__

//use libgcm command buffer debug mode in debug
#include <sys/process.h>
#include <sys/spu_initialize.h>
#include <sys/paths.h>

#ifdef PSGL
	#include <PSGL/psgl.h>
	#include <PSGL/psglu.h>
#endif

#include <cell/dbgfont.h>

#include <stdarg.h>
#include <cell/dbgfont.h>
#include <cell/sysmodule.h>
#if defined(CRY_USE_GCM_HUD)
	#include <cell/gcm_hud.h>
#else
	#include <cell/gcm.h>
#endif
#include "../CCryDXPSMisc.hpp"

#include "GCM/CCryDXPSGCM_MemMan.hpp"
#include "GCM/CCryDXPSGCM_Formats.hpp"
#include "GCM/CCryDXPSGCM_Scratchpad.hpp"
//#include "../Device/Resource/Misc/CCryDXPSInputLayout.hpp"

#include "../Device/Resource/CCryDXPSResource.hpp"

#ifdef GCM_DEBUG_CMDBUFFER
void debugCheckCmdBuffer(void);
#define GCM_CHECK_CMDBUFFER (debugCheckCmdBuffer());
#else
#define GCM_CHECK_CMDBUFFER
#endif

class CCryDXPSRenderDevice;
class CCryDXPSSwapChain;
class CCryDXPSBlob;
class CCryDXPSBuffer;
class CCryDXPSSamplerState;
class CCryDXPSShader;
class CCryDXPSShaderReflection;
class CCryDXPSTexture3D;
class CCryDXPSTexture1D;
class CCryDXPSTexture2D;
class CCryDXPSTexture3D;
class CCryDXPSBlendState;
class CCryDXPSRasterizerState;
class CCryDXPSDepthStencilState;
class CCryDXPSInputLayout;
class CCryDXPSRenderTargetView;
class CCryDXPSDepthStencilView;
class CCryDXPSShaderResourceView;
typedef class CCryDXPSGIAdapter IDXGIAdapter;

class CCryLayer0GCM
{
	CCryDXPSGCMMemMan							m_Memory;
	CCryDXPSScratchPad						m_ScratchPad;
public:
	static CCryLayer0GCM&	Instance();


	// Mem management
	CCryDXPSRenderDevice*	CreateCCryDXPSRenderDevice(
																IDXGIAdapter* pAdapter,D3D10_DRIVER_TYPE DriverType,
																int Software,uint32 Flags,
																uint32 SDKVersion);

	CCryDXPSSwapChain*	CreateCCryDXPSSwapChain(CCryDXPSRenderDevice*&, DXGI_SWAP_CHAIN_DESC*&);

	CCryDXPSBlob*	CreateCCryDXPSBlob(std::size_t);
	CCryDXPSBlob*	CreateCCryDXPSBlob(const char* pProgram,uint32 Size);

	CCryDXPSBuffer*	CreateCCryDXPSBuffer(const void* pVertices,uint32 Size,CCRYDXPSResType Type);

	CCryDXPSSamplerState*	CreateCCryDXPSSamplerState(const D3D10_SAMPLER_DESC *pSamplerDesc);
	CCryDXPSBlendState*	CreateCCryDXPSBlendState(const D3D10_BLEND_DESC *pBlendDesc);
	CCryDXPSRasterizerState*	CreateCCryDXPSRasterizerState(const D3D10_RASTERIZER_DESC *pRasterizerDesc);
	CCryDXPSDepthStencilState*	CreateCCryDXPSDepthStencilState(const D3D10_DEPTH_STENCIL_DESC *pDepthStencilDesc);

	CCryDXPSTexture1D*	CreateCCryDXPSTexture1D(uint32 SizeX,uint32 Mips,DXGI_FORMAT Format);
	CCryDXPSTexture2D*	CreateCCryDXPSTexture2D(uint32 SizeX,uint32 SizeY,uint32 Mips,DXGI_FORMAT Format,CCryDXPSSwapChain* pSwapChain);
	CCryDXPSTexture3D*	CreateCCryDXPSTexture3D(uint32 SizeX,uint32 SizeY,uint32 SizeZ,uint32 Mips,DXGI_FORMAT Format);

	CCryDXPSRenderTargetView*	CreateCCryDXPSRenderTargetView(CCryDXPSTexture2D* pTex);
	CCryDXPSDepthStencilView*	CreateCCryDXPSDepthStencilView(CCryDXPSTexture2D* pTex);
	CCryDXPSShaderResourceView*	CreateCCryDXPSShaderResourceView(CCryDXPSResource* pTex,const D3D10_SHADER_RESOURCE_VIEW_DESC* pDesc);

	void	ScreenSize(int& rSizeX,int& rSizeY);

	CCryDXPSShader* CreateCCryDXPSShader(const void* pProgram,uint32 SType);
	CCryDXPSShaderReflection* CreateCCryDXPSShaderReflection(const char* pProgram);

	CCryDXPSInputLayout* CreateCCryDXPSInputLayout(const D3D10_INPUT_ELEMENT_DESC *pInputElementDescs,uint32 NumElements,const void *pShaderBytecodeWithInputSignature);

	template<class T>
	T*	New()
	{
		return new T;
	}
	template<class T>
	T*	New(uint32 S)
	{
		return new T[S];
	}

	template<class T>
	void	Release(T* pObject)
	{
		pObject->Release();
	}
	template<class T>
	void	Delete(T*	pObject)
	{
		delete pObject;
	}
	template<class T>
	void	DeleteArray(T* pObject)
	{
		delete[] pObject;
	}


	inline CCryDXPSGCMMemMan&	Memory(){return m_Memory;}

	uint8* AllocScratchPad(uint32 Size)
	{
		if(!m_ScratchPad.InUse() && Size<=m_ScratchPad.Size())
		{
			m_ScratchPad.InUse(1);
			return m_ScratchPad.Data();
		}
		return 0;
	}
	void FreeScratchPad(){m_ScratchPad.InUse(0);}


};
typedef CCryLayer0GCM				tdLayer0;
#endif

