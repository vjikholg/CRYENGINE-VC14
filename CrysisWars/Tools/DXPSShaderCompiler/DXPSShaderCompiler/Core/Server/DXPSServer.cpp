#include "../StdTypes.hpp"
#ifdef _MSC_VER
#include <process.h>
#include <direct.h>
#endif
#ifdef UNIX
#include <pthread.h>
#endif
#include "../Error.hpp"
#include "DXPSServer.hpp"
#include "../Socket/DXPSSock.hpp"
#include "../Reflect/DXPSReflect.hpp"
#include "../DXPSHelper.hpp"


#ifdef _MSC_VER
const static std::string SHADER_STRIPPER					=	"sce-cgcstrip.exe";
const static std::string SHADER_COMPILER					=	"sce-cgc.exe";
const static std::string SHADER_DISASSEMBLER			=	"sce-cgcdisasm.exe";
#endif
#ifdef UNIX
const static std::string SHADER_STRIPPER					=	"sce-cgcstrip";
const static std::string SHADER_COMPILER					=	"sce-cgc";
const static std::string SHADER_DISASSEMBLER			=	"sce-cgcdisasm";
#endif
const static std::string SHADER_PATH_SOURCE				=	"Source";
const static std::string SHADER_PATH_BINARY				=	"Binary";
const static std::string SHADER_PATH_HALFSTRIPPED	=	"HalfStripped";
const static std::string SHADER_PATH_DISASSEMBLED	=	"DisAsm";
const static std::string SHADER_PATH_STRIPPPED		=	"Stripped";
const static std::string SHADER_PATH_CACHE				=	"Cache";


SEnviropment&	SEnviropment::Instance()
{
	static SEnviropment g_Enviropment;
	return g_Enviropment;
}

class CThreadData
{
	uint32_t		m_Counter;
	CDXPSSock*	m_pSock;
public:
							CThreadData(uint32_t		Counter,CDXPSSock*	pSock):
							m_Counter(Counter),
							m_pSock(pSock){}

							~CThreadData(){delete m_pSock;}

	CDXPSSock*	Socket(){return m_pSock;}
	uint32_t		ID()const{return m_Counter;}

};

uint32_t __stdcall Compile(void* pData)
{
	DXPS_SECURE_START
		CThreadData* pThreadData	=	reinterpret_cast<CThreadData*>(pData);
		std::vector<uint8_t> Vec;
		if(pThreadData->Socket()->Recv(Vec))
		{
			delete new CDXPSReflect(Vec,pThreadData->ID());
			pThreadData->Socket()->Send(Vec);
		}
		delete pThreadData;
	DXPS_SECURE_END
	return 0;
}

CDXPSServer::CDXPSServer(uint32_t Port)
{
	DXPS_SECURE_START

		Init();
		CDXPSSock::Instance().InitRoot(Port);

		uint32_t JobCounter=0;
		while(1)
		{
			CThreadData* pData	=	new CThreadData(JobCounter++,CDXPSSock::Instance().Accept());
#if defined _MSC_VER
			uint32_t Ret;
			CloseHandle(reinterpret_cast<HANDLE>(_beginthreadex(0,0,Compile,(void*)pData,0,&Ret)));
#elif defined UNIX
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_t thread;
			if (pthread_create(&thread, &attr,
						reinterpret_cast<void *(*)(void *)>(Compile),
						reinterpret_cast<void *>(pData)) != 0)
				DXPS_ERROR("thread creation failed");
#else
#error "not implemented"
#endif

		}
	DXPS_SECURE_END
}

void CDXPSServer::Init()
{

	std::vector<uint8_t> Config;

	SEnviropment::Instance().m_StripperFileName				=	SHADER_STRIPPER;
	SEnviropment::Instance().m_CompilerFileName				=	SHADER_COMPILER;
	SEnviropment::Instance().m_DisassemblerFileName		=	SHADER_DISASSEMBLER;
	SEnviropment::Instance().m_ShaderSourcePath				=	SHADER_PATH_SOURCE;
	SEnviropment::Instance().m_ShaderBinaryPath				=	SHADER_PATH_BINARY;
	SEnviropment::Instance().m_ShaderHalfStrippedPath	=	SHADER_PATH_HALFSTRIPPED;
	SEnviropment::Instance().m_ShaderDisassembledPath	=	SHADER_PATH_DISASSEMBLED;
	SEnviropment::Instance().m_ShaderStrippedPath			=	SHADER_PATH_STRIPPPED;
	SEnviropment::Instance().m_ShaderCache						=	SHADER_PATH_CACHE;


	if(CDXPSHelper::FromFile("config.cfg",Config))
	{
		std::string Con;
		tdEntryVec Tokens;
		tdTokenList	TokenList;
//		std::copy(Config.begin(),Config.end(),Con.begin());
//		Con.push_back(0);
		Con	=	std::string(reinterpret_cast<char*>(&Config[0]),Config.size());
#ifdef _DEBUG
		OutputDebugString(Con.c_str());
		OutputDebugString("\n");
#endif
		CDXPSHelper::Tokenize(Tokens,Con,"\n");
		CDXPSHelper::Splitizer(TokenList,Tokens,"=");
		for(tdTokenList::iterator it=TokenList.begin();it!=TokenList.end();++it)
		{
#ifdef _DEBUG
			OutputDebugString(it->first.c_str());
			OutputDebugString("|");
			OutputDebugString(it->second.c_str());
			OutputDebugString("\n");
#endif
			if(it->first=="Rootpath")
			{
				std::string Path=it->second;
				CDXPSHelper::Trim(Path,13);
				DXPS_LOG(std::string("Setting tmp-file-path to ") + it->second);
				SEnviropment::Instance().m_ShaderSourcePath				=Path+SHADER_PATH_SOURCE;
				SEnviropment::Instance().m_ShaderBinaryPath				=Path+SHADER_PATH_BINARY;
				SEnviropment::Instance().m_ShaderHalfStrippedPath	=Path+SHADER_PATH_HALFSTRIPPED;
				SEnviropment::Instance().m_ShaderDisassembledPath	=Path+SHADER_PATH_DISASSEMBLED;
				SEnviropment::Instance().m_ShaderStrippedPath			=Path+SHADER_PATH_STRIPPPED;
				SEnviropment::Instance().m_ShaderCache						=Path+SHADER_PATH_CACHE;
			}
			else
			if(it->first=="Compilerpath")
			{
				std::string Path=it->second;
				CDXPSHelper::Trim(Path,13);
				DXPS_LOG(std::string("Setting cg-binary-path to ") + Path);
				//ugly hack as + and append does not work, very weird bug
				char Text[1024];
				strcpy(Text,Path.c_str());
				strcat(Text,SHADER_STRIPPER.c_str());
				SEnviropment::Instance().m_StripperFileName			= Text;
				strcpy(Text,Path.c_str());
				strcat(Text,SHADER_COMPILER.c_str());
				SEnviropment::Instance().m_CompilerFileName			= Text;
				strcpy(Text,Path.c_str());
				strcat(Text,SHADER_DISASSEMBLER.c_str());
				SEnviropment::Instance().m_DisassemblerFileName	= Text;
#ifdef _DEBUG
			OutputDebugString("\n");
			OutputDebugString(SHADER_COMPILER.c_str());
			OutputDebugString("\n");
			OutputDebugString(SHADER_DISASSEMBLER.c_str());
			OutputDebugString("\n");
			OutputDebugString(SEnviropment::Instance().m_CompilerFileName.c_str());
			OutputDebugString("\n");
			OutputDebugString(SEnviropment::Instance().m_DisassemblerFileName.c_str());
			OutputDebugString("\n");
#endif
			}
		}
	}

#ifdef _DEBUG
	OutputDebugString(SEnviropment::Instance().m_ShaderSourcePath.c_str());
	OutputDebugString("\n");
	OutputDebugString(SEnviropment::Instance().m_ShaderBinaryPath.c_str());
	OutputDebugString("\n");
	OutputDebugString(SEnviropment::Instance().m_ShaderHalfStrippedPath.c_str());
	OutputDebugString("\n");
	OutputDebugString(SEnviropment::Instance().m_ShaderDisassembledPath.c_str());
	OutputDebugString("\n");
	OutputDebugString(SEnviropment::Instance().m_ShaderStrippedPath.c_str());
	OutputDebugString("\n");
	OutputDebugString(SEnviropment::Instance().m_ShaderCache.c_str());
	OutputDebugString("\n");
#endif

	_mkdir(SEnviropment::Instance().m_ShaderSourcePath.c_str());
	_mkdir(SEnviropment::Instance().m_ShaderBinaryPath.c_str());
	_mkdir(SEnviropment::Instance().m_ShaderHalfStrippedPath.c_str());
	_mkdir(SEnviropment::Instance().m_ShaderDisassembledPath.c_str());
	_mkdir(SEnviropment::Instance().m_ShaderStrippedPath.c_str());
	_mkdir(SEnviropment::Instance().m_ShaderCache.c_str());


}



