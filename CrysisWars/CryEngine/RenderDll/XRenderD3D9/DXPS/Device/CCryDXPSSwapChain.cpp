#include "RenderPCH.h"
#include <sysutil/sysutil_sysparam.h>
#include "../Layer0/CCryDXPS.hpp"
#include "CCryDXPSSwapChain.hpp"
#include "CCryDXPSRenderDevice.hpp"
//#include <assert.h>
#include <stdio.h>

using namespace cell::Gcm;

uint8	CCryDXPSSwapChain::m_CurrentVideoMode=~0;


CCryDXPSSwapChain::CCryDXPSSwapChain(ID3D10Device *pDevice,DXGI_SWAP_CHAIN_DESC *pSwapChainDesc):
CCryRefAndWeak<CCryDXPSSwapChain>(this),
CCryDXPSResource(EDXPS_RT_TEX2D_SWAPCHAIN),
m_Desc(*pSwapChainDesc),
m_pDevice(pDevice)
{
	const uint32_t ScrH	=	pSwapChainDesc->BufferDesc.Height;
	m_VideoMode	=	ScrH>=1080?1:ScrH>=720?2:ScrH>=480?4:5;

	pDevice->CreateSwapChain(*this,pSwapChainDesc);

	//create swap chain
	// texture in requested size
	m_BufferCount	=	pSwapChainDesc->BufferCount<CCRY_MAXBACKBUFFER?pSwapChainDesc->BufferCount+1:CCRY_MAXBACKBUFFER;
	for(uint32 a=0;a<m_BufferCount && a<CCRY_MAXBACKBUFFER;a++)
	{
		uint32 Offset;
		m_pTexture2D[a]	=	CRY_DXPS_CREATE(CCryDXPSTexture2D,(	pSwapChainDesc->BufferDesc.Width,
																													pSwapChainDesc->BufferDesc.Height,
																													1,
																													pSwapChainDesc->BufferDesc.Format,
																													this));
		cellGcmAddressToOffset(m_pTexture2D[a]->RawPointer(),&Offset);
		if(CELL_OK	!=	cellGcmSetDisplayBuffer(a,Offset,	m_pTexture2D[a]->Pitch(0),
																											m_pTexture2D[a]->SizeX(),
																											m_pTexture2D[a]->SizeY()))
		{
			CRY_DEBUGOUT("could not set create/displaybuffer\n");
		}
		else
		{
			CRY_DEBUGOUT("Successfully created displaybuffer with offset:0x%x,Pitch:%d,SizeX%d,SizeY%d\n",Offset,	m_pTexture2D[a]->Pitch(0),
																											m_pTexture2D[a]->SizeX(),
																											m_pTexture2D[a]->SizeY());
		}
	}
	m_CurrentFrameBuffer	=	m_BufferCount-1;
	cellGcmResetFlipStatus();
	cellGcmSetFlip(NextFrameBuffer());
	cellGcmFlush();
	cellGcmSetWaitFlip();
	assert(m_BufferCount>1);
};

void CCryDXPSSwapChain::SwitchVideoMode()
{
	//all work needs to be finished on display-mode-switch
	cellGcmSetWaitFlip();
	cellGcmFinish(0);

	CellVideoOutConfiguration videocfg;
	memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
	videocfg.resolutionId = m_VideoMode;
	while(videocfg.resolutionId<6 && !cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY,videocfg.resolutionId,0,0))
	{
		videocfg.resolutionId++;
	}
	videocfg.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	videocfg.pitch	=	m_pTexture2D[0]->Pitch(0);//pSwapChainDesc->BufferDesc.Width*4;
	int Ret=cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0);
	if(CELL_OK!=Ret)
	{
		CRY_DEBUGOUT("Failed on cellVideoOutConfigure(). (0x%x)\n", Ret);
	}
	else
	{
		CRY_DEBUGOUT("VideoOutConfigure resolutionID:%d Pitch:%d\n",videocfg.resolutionId,videocfg.pitch);
	}
	for(uint32 a=0;a<m_BufferCount && a<CCRY_MAXBACKBUFFER;a++)
	{
		uint32 Offset;
		cellGcmAddressToOffset(m_pTexture2D[a]->RawPointer(),&Offset);
		if(CELL_OK	!=	cellGcmSetDisplayBuffer(a,Offset,	m_pTexture2D[a]->Pitch(0),
																											m_pTexture2D[a]->SizeX(),
																											m_pTexture2D[a]->SizeY()))
		{
			CRY_DEBUGOUT("could not set create/displaybuffer\n");
		}
		else
		{
			CRY_DEBUGOUT("Successfully created displaybuffer with offset:0x%x,Pitch:%d,SizeX%d,SizeY%d\n",Offset,	m_pTexture2D[a]->Pitch(0),
																											m_pTexture2D[a]->SizeX(),
																											m_pTexture2D[a]->SizeY());
		}
	}

	CellVideoOutDeviceInfo Info;
	cellVideoOutGetDeviceInfo(CELL_VIDEO_OUT_PRIMARY,0,&Info);
	CellVideoOutResolution Resolution;
	cellVideoOutGetResolution(videocfg.resolutionId,&Resolution);


	m_CurrentVideoMode	=	m_VideoMode;
}

long CCryDXPSSwapChain::Present(uint32 SyncInterval,uint32 Flags)
{
	if(m_VideoMode!=m_CurrentVideoMode)
		SwitchVideoMode();

	int a=100000;
	while(cellGcmGetFlipStatus()!=0 && --a)
	{
		sys_timer_usleep(300);
	}
	if(!a)
	{
		cellGcmResetFlipStatus();
		CRY_DEBUGOUT("- endless loop while waiting for flip - \n");
	}

	if(cellGcmSetFlip(NextFrameBuffer()) != CELL_OK)
	{
		CRY_DEBUGOUT("Error on flipping Ln%d\n",__LINE__);
		return -1;
	}
	cellGcmFlush();

	cellGcmSetWaitFlip();

//	CRY_DEBUGOUT("CCRY_MAXBACKBUFFER %d    m_CurrentFrameBuffer %d\n",CCRY_MAXBACKBUFFER,m_CurrentFrameBuffer);
	for(uint32 a=1;a<CCRY_MAXBACKBUFFER;a++)
		if(m_pTexture2D[a])	//check weakness
			m_pTexture2D[a]->Swap(*m_pTexture2D[a-1]);
	m_pDevice->UpdateRendertargets();
#ifdef _DEBUG
	m_pDevice->m_DrawCounter	=	0;
#endif

	return 0;
}

long CCryDXPSSwapChain::GetBuffer(uint32 Buffer,ECryGUID riid,void **ppSurface)
{
#ifdef CRY_USE_DX9
	assert(riid==ID3D10Texture2D__GUID);
	if(riid==ID3D10Texture2D__GUID)
	{
		if(!m_pTexture2D)
			m_pTexture2D	=	CRY_DXPS_CREATE(CCryDXPSTexture2D,(m_pSwapChain));
		else
			m_pTexture2D->IncRef();
		*ppSurface	=	m_pTexture2D;
		return 0;
	}
#elif CRY_USE_OGL
	m_pTexture2D->IncRef();
	*ppSurface	=	m_pTexture2D;
	return *ppSurface?0:-1;
#elif CRY_USE_GCM
	m_pTexture2D[0]->IncRef();
	*ppSurface	=	m_pTexture2D[0];
	return 0;
#else
//Unsupported DeviceType
#endif
}

long CCryDXPSSwapChain::SetFullscreenState(int Fullscreen,IDXGIOutput *pTarget)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSSwapChain::GetFullscreenState(int *pFullscreen,IDXGIOutput **ppTarget)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSSwapChain::GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSSwapChain::ResizeBuffers(uint32 BufferCount,uint32 Width,uint32 Height,DXGI_FORMAT NewFormat,uint32 SwapChainFlags)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSSwapChain::ResizeTarget(const DXGI_MODE_DESC *pNewTargetParameters)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSSwapChain::GetContainingOutput(IDXGIOutput** ppOutput)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSSwapChain::GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSSwapChain::GetLastPresentCount(uint32*	pLastPresentCount)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}
