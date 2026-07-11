#include "RenderPCH.h"
#include "../../../Layer0/CCryDXPS.hpp"
#include "../../../CCryDXPSMisc.hpp"
#include "CCryDXPSTexture2D.hpp"
//#include <assert.h>

template<class T>
bool Po2(T x){return ((x & -x) == x);}


CCryDXPSTexture2D::CCryDXPSTexture2D(uint32 SizeX,uint32 SizeY,uint32 Mips,DXGI_FORMAT Format,CCryDXPSSwapChain* pSwapChain):
CCryDXPSTexture(EDXPS_RT_TEX2D)

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
		CRY_DEBUGOUT("Creating Texture(%d,%d) with Pixelsize of %d Bits\n",SizeX,SizeY,GCM_TextureFormatSize(8,DXGI_FORMAT2GCM[Format]));
#endif
	m_GCMTexture.format	=	DXGI_FORMAT2GCM[Format];
/*	if(	m_GCMTexture.format!=CELL_GCM_TEXTURE_COMPRESSED_DXT1 &&
			m_GCMTexture.format!=CELL_GCM_TEXTURE_COMPRESSED_DXT23 &&
			m_GCMTexture.format!=CELL_GCM_TEXTURE_COMPRESSED_DXT45)
		m_GCMTexture.format|=CELL_GCM_TEXTURE_LN;//|CELL_GCM_TEXTURE_UN;
	else
		m_GCMTexture.format|=CELL_GCM_TEXTURE_SZ;*/

	//needed for  Pitch()
	m_GCMTexture.width	=	SizeX;
	m_GCMTexture.height	=	SizeY;
	m_GCMTexture.mipmap	=	Mips;


	if(Po2(SizeX) && Po2(SizeY) && m_GCMTexture.format!=CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT)
	{
		m_GCMTexture.format|=CELL_GCM_TEXTURE_SZ;
		m_GCMTexture.pitch	=	0;
		m_MemItemID	=	tdLayer0::Instance().Memory().Allocate(GCM_TextureFormatSize((Mips>1?SizeX*SizeY*4/3:SizeX*SizeY),DXGI_FORMAT2GCM[Format]));
	}
	else
	{
		m_GCMTexture.format|=CELL_GCM_TEXTURE_LN;
		m_GCMTexture.pitch	=	Pitch(0);
		//OPTIMIZE TODO PS3HACK memory calculation could be optimized slightly
		m_MemItemID	=	tdLayer0::Instance().Memory().Allocate(GCM_TextureFormatSize((SizeX*SizeY*(Mips>1?2:1)),DXGI_FORMAT2GCM[Format]));
	}

	m_GCMTexture.dimension	=	CELL_GCM_TEXTURE_DIMENSION_2;
	m_GCMTexture.cubemap	=	CELL_GCM_FALSE;
	m_GCMTexture.depth	=	1;
	m_GCMTexture.location	=	CELL_GCM_LOCATION_LOCAL;
	if(DXGI_FORMAT2GCM[Format]==CELL_GCM_TEXTURE_B8)
	{
		m_GCMTexture.remap =	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
													CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
													CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
													CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
													CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
													CELL_GCM_TEXTURE_REMAP_FROM_B << 4 |
													CELL_GCM_TEXTURE_REMAP_FROM_B << 2 |
													CELL_GCM_TEXTURE_REMAP_FROM_B;
	}
	else
	{
		m_GCMTexture.remap =	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
													CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
													CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
													CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
													CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
													CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
													CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
													CELL_GCM_TEXTURE_REMAP_FROM_A;
	}
	cellGcmAddressToOffset(RawPointer(),&m_GCMTexture.offset);
}


long CCryDXPSTexture2D::Map(uint32 SubResource,D3D10_MAP MapType,uint32 MapFlags,D3D10_MAPPED_TEXTURE2D* ppData)
{
	using namespace cell::Gcm;
	ppData->pData			=	RawPointer();

	if((SizeY()>>(SubResource>0?SubResource-1:0))<4)
		return -1;

	for(uint32 a=0;a<SubResource;a++)
		ppData->pData	+=	Pitch(a)*SizeY()>>a;
	ppData->RowPitch	=	Pitch(SubResource);
	cellGcmFinish(0);//SLOW
	return 0;
}

long CCryDXPSTexture2D::Unmap(uint32 Subresource)
{
	return 0;
}

long CCryDXPSTexture2D::GetDesc(D3D10_TEXTURE2D_DESC *pDesc)
{

	pDesc->Width							=	SizeX();
	pDesc->Height							=	SizeY();
	pDesc->MipLevels					=	1;
	pDesc->ArraySize					=	1;
	pDesc->Format							=	m_Format;
	pDesc->SampleDesc.Count		=	1;
	pDesc->SampleDesc.Quality	=	0;
	pDesc->Usage							=	D3D10_USAGE_DEFAULT;
	pDesc->CPUAccessFlags			=	D3D10_CPU_ACCESS_WRITE |D3D10_CPU_ACCESS_READ;
	pDesc->MiscFlags					=	0;
	return 0;
}

