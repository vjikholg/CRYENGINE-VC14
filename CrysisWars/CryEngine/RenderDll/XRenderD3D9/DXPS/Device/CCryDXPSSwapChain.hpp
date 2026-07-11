#ifndef __CRYDXPSSWAPCHAIN__
#define __CRYDXPSSWAPCHAIN__

#include "Resource/CCryDXPSResource.hpp"
#include "../CCryDXPSMisc.hpp"
#include "../CCryDXPSDummy.hpp"
#include "../Layer0/CCryDXPS.hpp"
#include "CCryDXPSRenderDevice.hpp"

enum{CCRY_MAXBACKBUFFER	=	1+1};//the additional one is the current framebuffer on PS3

class CCryDXPSSwapChain;
class CCryDXPSSwapChain	:		public CCryDXPSResource	,	public	CCryRefAndWeak<CCryDXPSSwapChain>
{
	friend class CCryDXPSRenderDevice;
	APRefTexture2D					m_pTexture2D[CCRY_MAXBACKBUFFER];
	uint8										m_BufferCount;
	uint8										m_CurrentFrameBuffer;
	uint8										m_VideoMode;
	static uint8						m_CurrentVideoMode;

	void										SwitchVideoMode();
public:
	uint32	BufferCount()const{return m_BufferCount;}
	uint32	NextFrameBuffer()
	{
		m_CurrentFrameBuffer=(m_CurrentFrameBuffer+1)%m_BufferCount;
		return m_CurrentFrameBuffer^1;
	}

	DXGI_SWAP_CHAIN_DESC	m_Desc;
	CCryDXPSRenderDevice*	m_pDevice;


public:
													CCryDXPSSwapChain(ID3D10Device* pDevice,DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
				inline	void			ReleaseResources(){}

				long  						Present(uint32 SyncInterval,uint32 Flags);
        long  						GetBuffer(uint32 Buffer,ECryGUID riid,void **ppSurface);
        long  						SetFullscreenState(int Fullscreen,IDXGIOutput *pTarget);
        long  						GetFullscreenState(int *pFullscreen,IDXGIOutput **ppTarget);
        long  						GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc);
        long  						ResizeBuffers(uint32 BufferCount,uint32 Width,uint32 Height,DXGI_FORMAT NewFormat,uint32 SwapChainFlags);
        long  						ResizeTarget(const DXGI_MODE_DESC *pNewTargetParameters);
        long  						GetContainingOutput(IDXGIOutput** ppOutput);
        long  						GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats);
        long  						GetLastPresentCount(uint32*	pLastPresentCount);
};

typedef CCryDXPSSwapChain IDXGISwapChain;
typedef CCryAPtrRefCnt<CCryDXPSSwapChain> LPDXGISWAPCHAIN;

#endif

