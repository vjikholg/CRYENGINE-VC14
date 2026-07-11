#ifndef __DXPSSERVER__
#define __DXPSSERVER__

#include <string>

class	SEnviropment
{
public:
	std::string		m_StripperFileName;
	std::string		m_CompilerFileName;
	std::string		m_DisassemblerFileName;
	std::string		m_ShaderSourcePath;
	std::string		m_ShaderBinaryPath;
	std::string		m_ShaderHalfStrippedPath;
	std::string		m_ShaderDisassembledPath;
	std::string		m_ShaderStrippedPath;
	std::string		m_ShaderCache;

	static SEnviropment&	Instance();
};

class CDXPSServer
{
	void			Init();
public:
						CDXPSServer(uint32_t Port);
};

#endif
