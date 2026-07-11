#ifdef CRY_USE_GCM
#include "RenderPCH.h"
#include <sysutil/sysutil_sysparam.h>
#include "CCryDXPS.hpp"
#include "CCryDXPSGCM.hpp"
#include "../Device/CCryDXPSBlob.hpp"
#include "../Device/CCryDXPSRenderDevice.hpp"
#include "../Device/CCryDXPSSwapChain.hpp"
#include "../Device/Resource/Buffer/CCryDXPSSamplerState.hpp"
#include "../Device/Resource/States/CCryDXPSBlendState.hpp"
#include "../Device/Resource/States/CCryDXPSRasterizerState.hpp"
#include "../Device/Resource/States/CCryDXPSDepthStencilState.hpp"

CCryLayer0GCM& CCryLayer0GCM::Instance()
{
	static CCryLayer0GCM g_CryLayer0;
	return g_CryLayer0;
}

CCryDXPSRenderDevice*	CCryLayer0GCM::CreateCCryDXPSRenderDevice(
																								IDXGIAdapter* pAdapter,D3D10_DRIVER_TYPE DriverType,
																								int Software,uint32 Flags,
																								uint32 SDKVersion)
{
	return new CCryDXPSRenderDevice(pAdapter,DriverType,Software,Flags,SDKVersion);
}

CCryDXPSSwapChain* CCryLayer0GCM::CreateCCryDXPSSwapChain(CCryDXPSRenderDevice*& pDevice,DXGI_SWAP_CHAIN_DESC*& pSwapChainDesc)
{
	return new CCryDXPSSwapChain(pDevice,pSwapChainDesc);
}

CCryDXPSInputLayout* CCryLayer0GCM::CreateCCryDXPSInputLayout(const D3D10_INPUT_ELEMENT_DESC *pInputElementDescs,uint32 NumElements,const void *pShaderBytecodeWithInputSignature)
{
	return new CCryDXPSInputLayout(pInputElementDescs,NumElements,pShaderBytecodeWithInputSignature);
}

CCryDXPSBlob* CCryLayer0GCM::CreateCCryDXPSBlob(std::size_t Size)
{
	return new CCryDXPSBlob(Size);
}

CCryDXPSBlob* CCryLayer0GCM::CreateCCryDXPSBlob(const char* pProgram,uint32 Size)
{
	return new CCryDXPSBlob(pProgram,Size);
}


CCryDXPSTexture1D* CCryLayer0GCM::CreateCCryDXPSTexture1D(uint32 SizeX,uint32 Mips,DXGI_FORMAT Format)
{
	return new CCryDXPSTexture1D(SizeX,Mips,Format);
}

CCryDXPSTexture2D* CCryLayer0GCM::CreateCCryDXPSTexture2D(uint32 SizeX,uint32 SizeY,uint32 Mips,DXGI_FORMAT Format,CCryDXPSSwapChain* pSwapChain)
{
	return new CCryDXPSTexture2D(SizeX,SizeY,Mips,Format,pSwapChain);
}

CCryDXPSTexture3D* CCryLayer0GCM::CreateCCryDXPSTexture3D(uint32 SizeX,uint32 SizeY,uint32 SizeZ,uint32 Mips,DXGI_FORMAT Format)
{
	return new CCryDXPSTexture3D(SizeX,SizeY,SizeZ,Mips,Format);
}

CCryDXPSRenderTargetView*	CCryLayer0GCM::CreateCCryDXPSRenderTargetView(CCryDXPSTexture2D* pTex)
{
	return new CCryDXPSRenderTargetView(pTex);
}

CCryDXPSDepthStencilView*	CCryLayer0GCM::CreateCCryDXPSDepthStencilView(CCryDXPSTexture2D* pTex)
{
	return new CCryDXPSDepthStencilView(pTex);
}

CCryDXPSShaderResourceView*	CCryLayer0GCM::CreateCCryDXPSShaderResourceView(	CCryDXPSResource* pTex,
																																							const D3D10_SHADER_RESOURCE_VIEW_DESC* pDesc)
{
	return new CCryDXPSShaderResourceView(reinterpret_cast<CCryDXPSTexture2D*>(pTex),pDesc);
}

CCryDXPSBuffer*	CCryLayer0GCM::CreateCCryDXPSBuffer(const void* pVertices,uint32 Size,CCRYDXPSResType Type)
{
	return new CCryDXPSBuffer(pVertices,Size,Type);
}

CCryDXPSShader* CCryLayer0GCM::CreateCCryDXPSShader(const void* pProgram,uint32 SType)
{
	return new CCryDXPSShader(pProgram,(ECRYDXPSShaderType)SType);
}

CCryDXPSShaderReflection* CCryLayer0GCM::CreateCCryDXPSShaderReflection(const char* pProgram)
{
	return new CCryDXPSShaderReflection(pProgram);
}

CCryDXPSSamplerState*	CCryLayer0GCM::CreateCCryDXPSSamplerState(const D3D10_SAMPLER_DESC *pSamplerDesc)
{
	return new CCryDXPSSamplerState(*pSamplerDesc);
}

CCryDXPSBlendState*	CCryLayer0GCM::CreateCCryDXPSBlendState(const D3D10_BLEND_DESC *pBlendDesc)
{
	return new CCryDXPSBlendState(*pBlendDesc);
}

CCryDXPSRasterizerState*	CCryLayer0GCM::CreateCCryDXPSRasterizerState(const D3D10_RASTERIZER_DESC *pRasterizerDesc)
{
	return new CCryDXPSRasterizerState(*pRasterizerDesc);
}

CCryDXPSDepthStencilState*	CCryLayer0GCM::CreateCCryDXPSDepthStencilState(const D3D10_DEPTH_STENCIL_DESC *pDepthStencilDesc)
{
	return new CCryDXPSDepthStencilState(*pDepthStencilDesc);
}


void CCryLayer0GCM::ScreenSize(int& rSizeX,int& rSizeY)
{
	CellVideoOutState videoState;
	if(0==cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState))
	{
		CellVideoOutResolution resolution;
		cellVideoOutGetResolution(videoState.displayMode.resolutionId, &resolution);
		rSizeX	= resolution.width;
		rSizeY	= resolution.height;
		CRY_DEBUGOUT("--resolution %d %d\n",resolution.width,resolution.height);
	}
}

#endif
