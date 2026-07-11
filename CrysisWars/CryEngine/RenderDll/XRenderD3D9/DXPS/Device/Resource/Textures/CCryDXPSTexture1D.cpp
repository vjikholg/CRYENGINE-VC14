#include "RenderPCH.h"
#include "../../../Layer0/CCryDXPS.hpp"
#include "../../../CCryDXPSMisc.hpp"
#include "CCryDXPSTexture1D.hpp"
//#include <assert.h>

CCryDXPSTexture1D::CCryDXPSTexture1D(uint32 SizeX,uint32 Mips,DXGI_FORMAT Format):
CCryDXPSTexture(EDXPS_RT_TEX1D)
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
		CRY_DEBUGOUT("Creating Texture(%d) with Pixelsize of %d Bits\n",SizeX,GCM_TextureFormatSize(8,DXGI_FORMAT2GCM[Format]));
#endif
	m_MemItemID	=	tdLayer0::Instance().Memory().Allocate(GCM_TextureFormatSize(SizeX,DXGI_FORMAT2GCM[Format]));
}
/*
long CCryDXPSTexture1D::Map(uint32 Subresource,D3D10_MAP MapType,uint32 MapFlags,void** ppData)
{
	ppData->pData			=	RawPointer();
	ppData->RowPitch	=	Pitch();
	cellGcmFinish(0);//SLOW
	return 0;
}

long CCryDXPSTexture1D::Unmap(uint32 Subresource)
{
	return 0;
}
*/
long CCryDXPSTexture1D::GetDesc(D3D10_TEXTURE1D_DESC *pDesc)
{
	return 0;
}

