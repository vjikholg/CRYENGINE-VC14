#ifndef __CRYDXPSRENDERTARGETVIEW__
#define __CRYDXPSRENDERTARGETVIEW__

#include "../CCryDXPSResource.hpp"
#include "../../../CCryDXPSMisc.hpp"
#include "../../../Layer0/CCryDXPS.hpp"
#include "../Textures/CCryDXPSTexture2D.hpp"

class CCryDXPSRenderTargetView;
class CCryDXPSRenderTargetView	:	public CCryDXPSResource	,	public CCryRefAndWeak<CCryDXPSRenderTargetView>
{
#ifdef CRY_USE_DX9
	IDirect3DSwapChain9*			m_pSwapChain;
public:
	CCryDXPSRenderTargetView(IDirect3DSwapChain9* pSwapChain):
	CCryRefAndWeak<CCryDXPSRenderTargetView>(this),	m_pSwapChain(pSwapChain)
	{
		if(m_pSwapChain)
			m_pSwapChain->AddRef();
	}
	IDirect3DSwapChain9*			SwapChain(){return m_pSwapChain;}
private:
#elif CRY_USE_OGL
	uint32										m_FBO;
	uint32										m_RBO;
	APWeakTexture2D						m_pTex;
public:
	CCryDXPSRenderTargetView(CCryDXPSTexture2D* pTex):
	CCryRefAndWeak<CCryDXPSRenderTargetView>(this),	m_pTex(pTex)
	{
		glGenFramebuffersCRY(1, &m_FBO);
		glBindFramebufferCRY(GL_FRAMEBUFFER_CRY, m_FBO);
		CRY_GL_CHECKERROR;
		glBindFramebufferCRY(GL_FRAMEBUFFER_CRY, 0);
	}
	uint32	FBO()const{return m_FBO;}
	uint32	RBO()const{return m_RBO;}
	CCryDXPSTexture2D*	Texture(){return m_pTex;}
#elif CRY_USE_GCM
private:
	APWeakTexture2D						m_pTex;
public:
	CCryDXPSRenderTargetView(CCryDXPSTexture2D* pTex):
		CCryDXPSResource(EDXPS_RT_RENDERTARGETVIEW),
		CCryRefAndWeak<CCryDXPSRenderTargetView>(this),	m_pTex(pTex)
	{
	}
	CCryDXPSTexture2D*	Texture(){return m_pTex;}
#else
	//Unsupported DeviceType
#endif
public:
									~CCryDXPSRenderTargetView()
									{
#ifdef CRY_USE_DX9
										if(m_pSwapChain)
											m_pSwapChain->Release();
#elif CRY_USE_OGL
#elif CRY_USE_GCM
#else
										//Unsupported DeviceType
#endif
									}
	inline	void		ReleaseResources()
									{
#ifdef CRY_USE_DX9
										if(m_pSwapChain)
											m_pSwapChain->Release();
										m_pSwapChain	=	0;
#elif CRY_USE_OGL
#elif CRY_USE_GCM
#else
										//Unsupported DeviceType
#endif
									}

	void						GetDesc(D3D10_RENDER_TARGET_VIEW_DESC *pDesc);
	void						GetResource(ID3D10Resource** pResource);
};

typedef CCryDXPSRenderTargetView ID3D10RenderTargetView;

#endif

