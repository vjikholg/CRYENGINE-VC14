#ifndef __CRYDXPSTEXTURE3D__
#define __CRYDXPSTEXTURE3D__

#include "CCryDXPSTextureBase.hpp"

class CCryDXPSTexture3D;
class CCryDXPSTexture3D		:		public CCryDXPSTexture
{
	//should never be called, but of the base-texture
	inline	~CCryDXPSTexture3D(){}
public:
	CCryDXPSTexture3D(uint32 SizeX,uint32 SizeY,uint32 SizeZ,uint32 Mips,DXGI_FORMAT Format);

	//unused yet
//	long						Map(uint32 Subresource,D3D10_MAP MapType,uint32 MapFlags,void** ppData);
//	long						Unmap(uint32 Subresource);
	long						GetDesc(D3D10_TEXTURE3D_DESC *pDesc);
};

typedef CCryDXPSTexture3D ID3D10Texture3D;
typedef CCryAPtrRefCnt<CCryDXPSTexture3D>		APRefTexture3D;
typedef CCryAPtrWeakCnt<CCryDXPSTexture3D>	APWeakTexture3D;

#endif

