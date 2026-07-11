#include "../StdTypes.hpp"
#include "../Error.hpp"
#include "DXPSReflect.hpp"
#include "../Server/DXPSServer.hpp"
#include "../DXPSHelper.hpp"
#ifdef _MSC_VER
#include <io.h>
#include <Windows.h>
#endif



CDXPSReflect::CDXPSReflect(std::vector<uint8_t>&	rInOut,uint32_t ID)
{
	char Text[1024];
	sprintf(Text,"%d",ID);
	m_FileID=Text;
	m_ShaderFileName			=	SEnviropment::Instance().m_ShaderSourcePath				+"/"+m_FileID+".cg";
	m_BinaryOutput				=	SEnviropment::Instance().m_ShaderBinaryPath				+"/"+m_FileID+".bin";
	m_ShaderHalfStripped	=	SEnviropment::Instance().m_ShaderHalfStrippedPath	+"/"+m_FileID+".bin";
	m_ShaderDisassembled	=	SEnviropment::Instance().m_ShaderDisassembledPath	+"/"+m_FileID+".asm";
	m_ShaderStripped			=	SEnviropment::Instance().m_ShaderStrippedPath			+"/"+m_FileID+".bin";
	m_ShaderCache					=	SEnviropment::Instance().m_ShaderCache						+"/"+m_FileID+".dps";

	if(rInOut.size()<sizeof(SCompileData))
	{
		rInOut.clear();
		DXPS_LOG("sizeof compile-job not even the header-size");
		return;
	}

	m_CompileData	=	*reinterpret_cast<SCompileData*>(&rInOut[0]);
	m_CompileData.EndianSwizzle();
	m_CompileData.ValidateVersion();
	std::vector<uint8_t> Program(&rInOut[sizeof(SCompileData)],&rInOut[rInOut.size()]);

	CDXPSHelper::ToFile(m_ShaderFileName,Program);

	rInOut.clear();

	if(	Compile()	&&
			HalfStrip()	&&
			Disasseble()	&&
			Strip()	&&
			Reflect(Program))
		CDXPSHelper::FromFile(m_ShaderCache,rInOut);
}


bool CDXPSReflect::Execute(const std::string& rFileName,const std::string& rParams)
{
#ifdef _MSC_VER
	const std::string Caller	=	rFileName+" "+rParams;
	bool	Ret	= false;
	DWORD	ExitCode	=	0;

	char* pCaller = new char[Caller.size() + 1];
	memcpy(pCaller,Caller.c_str(),Caller.size());
	pCaller[Caller.size()]=0;

#ifdef _DEBUG
	OutputDebugString(pCaller);
	OutputDebugString("\n");
#endif

	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	memset(&ProcessInfo, 0, sizeof(ProcessInfo));
	StartupInfo.cb = sizeof(StartupInfo);

	if(CreateProcess(rFileName.c_str(),pCaller,0,0, false,CREATE_DEFAULT_ERROR_MODE,0,0,&StartupInfo,&ProcessInfo) != false)
	{
		WaitForSingleObject(ProcessInfo.hProcess,INFINITE);
		GetExitCodeProcess(ProcessInfo.hProcess,&ExitCode);
/*		while(dwExitCode == STILL_ACTIVE && SecondsToWait != 0)
		{
			 GetExitCodeProcess(piProcessInfo.hProcess, &dwExitCode);
			 Sleep(500);
			 iMyCounter += 500;

			 if (iMyCounter > (SecondsToWait * 1000))
			 {
					dwExitCode = 0;
			 }
		}*/
		if(ExitCode)
		{
			Ret	=	false;
		}
		else
		{
			Ret	=	true;
		}

		CloseHandle(ProcessInfo.hProcess);
		CloseHandle(ProcessInfo.hThread);
	}
	else
	{
		Ret = false;
	}


	/* Free memory */
	delete[]	pCaller;

	return Ret;
#endif

#ifdef UNIX
	return system((rFileName + " " + rParams).c_str()) == 0;
#endif
}

bool CDXPSReflect::Compile()
{
	std::string Param;
	if(m_CompileData.m_VertexShader)
		Param	=	" -p sce_vp_rsx";
	else
		Param	=	" -p sce_fp_rsx";

	Param +=	" -e ";
	Param +=	m_CompileData.m_Entry;

	Param +=	" -o ";
	Param	+=	m_BinaryOutput;

	Param +=	" ";
	Param	+=	m_ShaderFileName;

	return Execute(SEnviropment::Instance().m_CompilerFileName,Param);
}

bool CDXPSReflect::HalfStrip()
{
	std::string Param;
	Param =	" -param -o ";
	Param	+=	m_ShaderHalfStripped;

	Param +=	" ";
	Param	+=	m_BinaryOutput;

	return Execute(SEnviropment::Instance().m_StripperFileName,Param);
}

bool CDXPSReflect::Disasseble()
{
#ifdef _DEBUG
	{
		std::string Param;
		Param =	" -o ";
		Param	+=	m_ShaderDisassembled+".dbg";

		Param +=	" ";
		
		Param	+=	m_BinaryOutput;

		Execute(SEnviropment::Instance().m_DisassemblerFileName,Param);
	}
#endif
	std::string Param;
	Param =	" -o ";
	Param	+=	m_ShaderDisassembled;

	Param +=	" ";
	
//	Param	+=	m_BinaryOutput;
	Param	+=	m_ShaderHalfStripped;

	return Execute(SEnviropment::Instance().m_DisassemblerFileName,Param);
}

bool CDXPSReflect::Strip()
{
	std::string Param;
	Param =	" -o ";
	Param	+=	m_ShaderStripped;

	Param +=	" ";
	Param	+=	m_BinaryOutput;

	return Execute(SEnviropment::Instance().m_StripperFileName,Param);
}

bool CDXPSReflect::Reflect(std::vector<uint8_t>& rProgram)
{
	m_Reflection.Reflect(m_ShaderCache,m_BinaryOutput,m_ShaderDisassembled,rProgram);
//	m_Reflection.Reflect(m_ShaderCache,m_ShaderStripped,m_ShaderDisassembled,rProgram);
	return true;
}

