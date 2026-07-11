#ifndef __CRYDXPSTEXTURE2D__
#define __CRYDXPSTEXTURE2D__

#include "CCryDXPSTextureBase.hpp"

class CCryDXPSSwapChain;

class CCryDXPSTexture2D;
class CCryDXPSTexture2D		:		public CCryDXPSTexture
{
	//should never be called, but of the base-texture
	inline	~CCryDXPSTexture2D(){}
private:

public:
	CCryDXPSTexture2D(uint32 SizeX,uint32 SizeY,uint32 Mips,DXGI_FORMAT Format,CCryDXPSSwapChain* pSwapChain);

	long	Map(uint32 Subresource,D3D10_MAP MapType,uint32 MapFlags,D3D10_MAPPED_TEXTURE2D* ppData);
	long	Unmap(uint32 Subresource);
	long	GetDesc(D3D10_TEXTURE2D_DESC *pDesc);
	inline uint32	Pitch(uint32 SubResource)const
	{
		return (GCM_TextureFormatSize(SizeX(),DXGI_FORMAT2GCM[m_Format]))>>((m_GCMTexture.format&CELL_GCM_TEXTURE_LN)?0:SubResource);
	}
	inline void Swap(CCryDXPSTexture2D& rTex)
	{
		const CellGcmTexture GCMTexture	=	m_GCMTexture;
		const DXGI_FORMAT	Format=	m_Format;
		const uint16 MemItemID	=	m_MemItemID;
		m_GCMTexture	=	rTex.m_GCMTexture;
		m_Format		=	rTex.m_Format;
		m_MemItemID	=	rTex.m_MemItemID;
		rTex.m_Format			=	Format;
		rTex.m_GCMTexture	=	GCMTexture;
		rTex.m_MemItemID	=	MemItemID;
	}
};

typedef CCryDXPSTexture2D ID3D10Texture2D;
typedef CCryAPtrRefCnt<CCryDXPSTexture2D>		APRefTexture2D;
typedef CCryAPtrWeakCnt<CCryDXPSTexture2D>	APWeakTexture2D;

#endif

