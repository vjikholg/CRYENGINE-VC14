#include "RenderPCH.h"
#include "../../../Layer0/CCryDXPS.hpp"
#include "../../../CCryDXPSMisc.hpp"
#include "CCryDXPSBuffer.hpp"
//#include <assert.h>

long CCryDXPSBuffer::Map(D3D10_MAP MapType,uint32 MapFlags,void** ppData)
{
	using namespace cell::Gcm;

	(*ppData)	=	m_pBuffer;
	cellGcmFinish(0);//SLOW
	return 0;
}

long CCryDXPSBuffer::Unmap()
{
	return 0;
}

long CCryDXPSBuffer::GetDesc(D3D10_BUFFER_DESC *pDesc)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

