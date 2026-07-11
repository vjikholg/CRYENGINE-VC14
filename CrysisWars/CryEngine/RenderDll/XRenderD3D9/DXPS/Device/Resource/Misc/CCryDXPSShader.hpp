#ifndef __CRYDXPSSHADER__
#define __CRYDXPSSHADER__

#include "../CCryDXPSResource.hpp"
#include "../../../CCryDXPSAPtr.hpp"
#include "../../../Layer0/CCryDXPS.hpp"
#include "../Buffer/CCryDXPSBuffer.hpp"

enum	ECRYDXPSShaderType
{
	EDXPS_ST_GEOMETRY,
	EDXPS_ST_PIXEL,
	EDXPS_ST_VERTEX,
};

class CDXPSShaderDesc;

struct SCryDXPSShaderParam
{
	CGparameter m_Param;
	union
	{
		uint32			m_Idx;
		uint32			m_InputOffset;
	};
#if defined(_DEBUG)
	std::string	m_Name;
	std::string	m_Semantic;
	CGbool			m_Referenced;
	CGenum			m_InOut;
	CGenum			m_Varia;
#endif
};

class CCryDXPSShader;
class CCryDXPSShader	:	public CCryDXPSResource	,	public	CCryRefAndWeak<CCryDXPSShader>
{
	const CDXPSShaderDesc*		m_pDesc;
//	ECRYDXPSShaderType				m_ShaderType;
	void*											m_puCode;
//	CGprogram									m_Program;
	uint16										m_MemItemIDPixel;
//	SCryDXPSShaderParam*			m_pParams;
//	uint32										m_ParamCounter;


	void											Reflect();
public:
	CCryDXPSShader(const void* pProgram,ECRYDXPSShaderType SType);
//	void Set(float* pConsts=0);
	void Set(const APWeakBuffer* pConstBuffer);


	inline	void		ReleaseResources();

	ECRYDXPSShaderType ShaderType()const;
};

/*
class CCryDXPSGeometryShader
{
public:
	unsigned long		AddRef(){};
	unsigned long		Release(){};
};

class CCryDXPSPixelShader
{
public:
	unsigned long		AddRef(){};
	unsigned long		Release(){};
};

class CCryDXPSVertexShader
{
public:
	unsigned long		AddRef(){};
	unsigned long		Release(){};
};*/

typedef CCryDXPSShader ID3D10GeometryShader;
typedef CCryDXPSShader ID3D10PixelShader;
typedef CCryDXPSShader ID3D10VertexShader;
typedef CCryAPtrWeakCnt<CCryDXPSShader>	APWeakShader;

#endif

