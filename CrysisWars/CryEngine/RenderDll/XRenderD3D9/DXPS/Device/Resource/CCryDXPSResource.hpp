#ifndef __CRYDXPSRESOURCE__
#define __CRYDXPSRESOURCE__

enum	CCRYDXPSResType
{
	EDXPS_RT_NONE=0,
	EDXPS_RT_TEX1D,
	EDXPS_RT_TEX2D,
	EDXPS_RT_TEX2D_SWAPCHAIN,
	EDXPS_RT_TEX3D,
	EDXPS_RT_INPUTLAYOUT,
	EDXPS_RT_RENDERTARGETVIEW,
	EDXPS_RT_DEPTHSTENCILVIEW,
	EDXPS_RT_SHADERRESOURCEVIEW,
	EDXPS_RT_SHADER,
	EDXPS_RT_SHADERREFLECTION,
	EDXPS_RT_BLOB,
	EDXPS_RT_VERTEXBUFFER,
	EDXPS_RT_SAMPLERSTATE,
	EDXPS_RT_INDEXBUFFER,
	EDXPS_RT_CONSTBUFFER,
	EDXPS_RT_BLENDSTATE,
	EDXPS_RT_RASTERIZERSTATE,
	EDXPS_RT_DEPTHSTENCILSTATE,
	EDXPS_RT_UNUSED=~0,
};

class CCryDXPSResource
{
	CCRYDXPSResType							m_Type;
protected:
															CCryDXPSResource(CCRYDXPSResType T):
															m_Type(T){}
public:
	//we try to run non-virtual if possible to avoid pipeline stalls on non-ooo CPU that handle
	//virtual calls like mispredicted conditional jumps

/*
	virtual void STDMETHODCALLTYPE GetType(D3D10_RESOURCE_DIMENSION *rType) = 0;
	virtual void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) = 0;
	virtual UINT STDMETHODCALLTYPE GetEvictionPriority( void) = 0;
*/
	inline CCRYDXPSResType			Type()const{return m_Type;}
	unsigned int								Release();
};

typedef CCryDXPSResource ID3D10Resource;

#endif

