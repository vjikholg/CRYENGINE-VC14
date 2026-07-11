#ifndef __DXPSREFLECT__
#define __DXPSREFLECT__

#include <vector>
#include <string>
#include "DXPSReflection.hpp"

const uint32_t	COMPILER_VERSION	=	0xDeadFace;

struct SCompileData
{
	uint32_t		m_Version;
	uint8_t			m_VertexShader;//false==pixelshader
	char				m_Entry[256-1-8];
	uint32_t		m_ShaderSize;

	void				EndianSwizzle()
	{
		CDXPSHelper::EndianSwizzleU32(m_Version);
		CDXPSHelper::EndianSwizzleU32(m_ShaderSize);
	}

	void				ValidateVersion()
	{
		if(COMPILER_VERSION!=m_Version)
		{
			DXPS_ERROR("Version conflict of server and shader compile-job");
		}
	}
};


class CDXPSReflect
{
	std::string			m_FileID;
	std::string			m_ShaderFileName;
	std::string			m_BinaryOutput;
	std::string			m_ShaderHalfStripped;
	std::string			m_ShaderDisassembled;
	std::string			m_ShaderStripped;
	std::string			m_ShaderCache;

	SCompileData		m_CompileData;

	CDXPSReflection	m_Reflection;

	void						ToFile(const std::string& rFileName,const std::vector<uint8_t>&	rOut);
	void						FromFile(const std::string& rFileName,std::vector<uint8_t>&	rIn);
	bool						Execute(const std::string& rFileName,const std::string& rParams);

	bool						Compile();
	bool						HalfStrip();
	bool						Disasseble();
	bool						Strip();
	bool						Reflect(std::vector<uint8_t>& rProgram);

public:
									CDXPSReflect(std::vector<uint8_t>&	rInOut,uint32_t ID);
};

#endif
