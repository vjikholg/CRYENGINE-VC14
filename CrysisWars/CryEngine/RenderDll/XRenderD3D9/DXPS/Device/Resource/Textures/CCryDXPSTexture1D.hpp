#ifndef __CRYDXPSTEXTURE1D__
#define __CRYDXPSTEXTURE1D__

#include "CCryDXPSTextureBase.hpp"

class CCryDXPSTexture1D;
class CCryDXPSTexture1D		:		public CCryDXPSTexture
{
	//should never be called, but of the base-texture
	inline	~CCryDXPSTexture1D(){}
public:
									CCryDXPSTexture1D(uint32 SizeX,uint32 Mips,DXGI_FORMAT Format);

	//unused yet
//	long						Map(uint32 Subresource,D3D10_MAP MapType,uint32 MapFlags,void** ppData);
//	long						Unmap(uint32 Subresource);
	long						GetDesc(D3D10_TEXTURE1D_DESC *pDesc);
};

typedef CCryDXPSTexture1D ID3D10Texture1D;
typedef CCryAPtrRefCnt<CCryDXPSTexture1D>		APRefTexture1D;
typedef CCryAPtrWeakCnt<CCryDXPSTexture1D>	APWeakTexture1D;

#endif

