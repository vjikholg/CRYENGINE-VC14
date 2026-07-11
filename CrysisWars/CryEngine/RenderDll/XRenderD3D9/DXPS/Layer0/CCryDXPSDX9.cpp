#include "RenderPCH.h"
#ifdef CRY_USE_DX9

#include "CCryDXPSDX9.hpp"
//#include <assert.h>

CCryLayer0DX9::CCryLayer0DX9()
{
	m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if(!m_pD3D9)
	{
		assert("Could not create Direct3D");
	}
}

CCryLayer0DX9::~CCryLayer0DX9()
{
	if(m_pD3D9)
		m_pD3D9->Release();
}


CCryLayer0DX9& CCryLayer0DX9::Instance()
{
	static CCryLayer0DX9 g_CryLayer0;
	return g_CryLayer0;
}
#endif
