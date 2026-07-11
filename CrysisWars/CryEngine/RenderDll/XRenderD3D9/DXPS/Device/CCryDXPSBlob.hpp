#ifndef __CRYDXPSBLOB__
#define __CRYDXPSBLOB__

#include "Resource/CCryDXPSResource.hpp"
#include "../CCryDXPSMisc.hpp"
#include "../CCryDXPSDummy.hpp"
#include "../Layer0/CCryDXPS.hpp"

enum	CCRYDXPSBlobType
{
	EDXPS_BT_DATA,
	EDXPS_BT_SHADER,
};

class CCryDXPSBlob;
class CCryDXPSBlob	:	public CCryDXPSResource	,	public	CCryRefAndWeak<CCryDXPSBlob>
{
#ifdef CRY_USE_DX9
	ID3DXBuffer*								m_pShader;
public:
	CCryDXPSBlob(ID3DXBuffer* pShader):
		CCryRefAndWeak<CCryDXPSBlob>(this),
		m_Type(EDXPS_BT_SHADER)	,	m_pShader(pShader)
	{
		if(m_pShader)
			m_pShader->AddRef();
	}
	ID3DXBuffer*	Shader(){return m_pShader;}
private:
#elif CRY_USE_OGL
	CGprogram										m_Program;
public:
	CCryDXPSBlob(CGprogram	cgProgram):
		CCryRefAndWeak<CCryDXPSBlob>(this),
		m_Program(cgProgram)	,	m_Type(EDXPS_BT_SHADER)
	{
	}
	CGprogram	Program()const
	{
		return m_Program;
	}
private:
#elif CRY_USE_GCM
	uint8*										m_pProgram;
public:
	CCryDXPSBlob(const char* pProgram,uint32 Size):
		CCryDXPSResource(EDXPS_RT_BLOB),
		CCryRefAndWeak<CCryDXPSBlob>(this),
		m_pProgram(reinterpret_cast<uint8*>(CRY_DXPS_NEWARRAY(uint32,(Size+4+3)>>2))),
		m_Size(Size+4),m_Type(EDXPS_BT_SHADER)
	{
		*reinterpret_cast<uint32*>(m_pProgram)	=	Size;
		for(uint32 a=0;a<Size;a++)
			m_pProgram[a+4]=pProgram[a];
	}
	uint8*	Program()const
	{
		return m_pProgram;
	}
#else
	//Unsupported DeviceType
#endif
	CCRYDXPSBlobType							m_Type;
	CCryAPtrScope<uint8>					m_pData;
	size_t												m_Size;
public:
	inline CCryDXPSBlob(size_t Size):
		CCryDXPSResource(EDXPS_RT_BLOB),
		CCryRefAndWeak<CCryDXPSBlob>(this)	,		m_Type(EDXPS_BT_DATA)	,
		m_pData(CRY_DXPS_NEWARRAY(uint8,Size))	,	m_Size(Size)
	{
	}
	~CCryDXPSBlob()
	{
#ifdef CRY_USE_DX9
		if(m_pShader)
			m_pShader->Release();
#elif CRY_USE_OGL
#elif CRY_USE_GCM
		delete[] m_pProgram;
#else
		//Unsupported DeviceType
#endif
 }

	inline void ReleaseResources()
	{
		m_pData.Release();
#ifdef CRY_USE_DX9
		if(m_pShader)
			m_pShader->Release();
		m_pShader	=	0;
#elif CRY_USE_OGL
#elif CRY_USE_GCM
		CRY_DXPS_DELETEARRAY(m_pProgram);
		m_pProgram	=	0;
#else
		//Unsupported DeviceType
#endif
	}

	inline	void* GetBufferPointer()
	{
	#ifdef CRY_USE_DX9
			if(m_Type==EDXPS_BT_SHADER)
				return m_pShader;
	#elif CRY_USE_OGL
			if(m_Type==EDXPS_BT_SHADER)
				return &m_Program;
	#elif CRY_USE_GCM
			if(m_Type==EDXPS_BT_SHADER)
				return m_pProgram;
	#else
			//Unsupported DeviceType
	#endif
		return m_pData;
	}
	inline	size_t GetBufferSize()
	{
		return m_Size;
	}

	inline	CCRYDXPSBlobType Type()const
	{
		return m_Type;
	}
};

typedef CCryDXPSBlob ID3D10Blob;
typedef ID3D10Blob* LPD3D10BLOB;

#endif

