#include "RenderPCH.h"
#include "../../../Layer0/CCryDXPS.hpp"
#include "../../../CCryDXPSMisc.hpp"
#include "CCryDXPSTexture3D.hpp"
//#include <assert.h>

CCryDXPSTexture3D::CCryDXPSTexture3D(uint32 SizeX,uint32 SizeY,uint32 SizeZ,uint32 Mips,DXGI_FORMAT Format):
CCryDXPSTexture(EDXPS_RT_TEX3D)
{
	m_Format	=	Format;

	if(DXGI_FORMAT2GCM[Format]>=255)
	{
#if defined(_DEBUG)
		CRY_DEBUGOUT("unsupported pixelformat %d\n",Format);
#endif
		return;
	}
#ifdef CRY_MM_DEBUG
		CRY_DEBUGOUT("Creating Texture(%d,%d,%d) with Pixelsize of %d bits\n",SizeX,SizeY,SizeZ,GCM_TextureFormatSize(8,DXGI_FORMAT2GCM[Format]));
#endif
	m_MemItemID	=	tdLayer0::Instance().Memory().Allocate(GCM_TextureFormatSize(SizeX*SizeY*SizeZ,DXGI_FORMAT2GCM[Format]));
}
/*
long CCryDXPSTexture3D::Map(uint32 Subresource,D3D10_MAP MapType,uint32 MapFlags,void** ppData)
{
	ppData->pData			=	RawPointer();
	ppData->RowPitch	=	Pitch();
	cellGcmFinish(0);//SLOW
	return 0;
}

long CCryDXPSTexture3D::Unmap(uint32 Subresource)
{
	using namespace cell::Gcm;

	return 0;
}
*/
long CCryDXPSTexture3D::GetDesc(D3D10_TEXTURE3D_DESC *pDesc)
{
	return 0;
}
