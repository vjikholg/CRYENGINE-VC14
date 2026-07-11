#include "RenderPCH.h"
#include "Layer0/CCryDXPS.hpp"
#include "CCryDXPSRenderer.hpp"
#include <string.h>
#include <stdio.h>
#include <cell/fs/cell_fs_file_api.h>
#include <sys/fs_external.h>
#include <Cg/cgc.h>


#define REMOTE_CG_COMPILE


long D3D10CreateDevice(									IDXGIAdapter *pAdapter,
																				D3D10_DRIVER_TYPE DriverType,
																				int Software,
																				uint32 Flags,
																				uint32 SDKVersion,
																				CCryDXPSRenderDevice **ppDevice)
{
	*ppDevice	=	CRY_DXPS_CREATE(CCryDXPSRenderDevice,(pAdapter,DriverType,Software,Flags,SDKVersion));
	return ppDevice?0:-1;
}
long D3D10CreateDeviceAndSwapChain(			IDXGIAdapter *pAdapter,
																				D3D10_DRIVER_TYPE DriverType,
																				int Software,
																				uint32 Flags,
																				uint32 SDKVersion,
																				DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
																				IDXGISwapChain **ppSwapChain,    
																				CCryDXPSRenderDevice **ppDevice)
{
	long RDevice	=	D3D10CreateDevice(pAdapter,DriverType,Software,Flags,SDKVersion,ppDevice);
	if(RDevice<0)
		return RDevice;
	*ppSwapChain	=	CRY_DXPS_CREATE(CCryDXPSSwapChain,(*ppDevice,pSwapChainDesc));
#ifdef PSGL
	cgRTCgcInit();	//initialize CG for runtime shader compilation 
#endif
	return ppSwapChain?0:-1;
}

long D3D10CreateBlob(size_t NumBytes, LPD3D10BLOB *ppBuffer)
{
	*ppBuffer	=	CRY_DXPS_CREATE(CCryDXPSBlob,(NumBytes));//creation with RefCount 1
	if(!(*ppBuffer)->GetBufferPointer())
	{
		delete *ppBuffer;
		*ppBuffer	=	0;
	}
	return ppBuffer?0:-1;
}

long D3DX10CompileShaderFromFile(const char*								pSrcFile,
																 const D3D10_SHADER_MACRO*	pDefines,
																 LPD3D10INCLUDE							pInclude,
																 const char*								pFunctionName,
																 const char*								pProfile,
																 uint32											Flags,
																 ID3DX10ThreadPump*					pPump,
																 ID3D10Blob**								ppShader,
																 ID3D10Blob**								ppErrorMsgs)
{
	if(pDefines!=0 || pInclude!=0 || pPump!=0 || ppErrorMsgs==0)
		return -1;
#ifdef CRY_USE_DX9
	char FileName[1024];
	Cry_strcpy_s(FileName,sizeof(FileName),pSrcFile);
	Cry_strcat_s(FileName,sizeof(FileName),"9");
	const char* pShaderVersion	=	pProfile[3]<='3'?pProfile:*pProfile=='v'?"vs_3_0":"ps_3_0";
	ID3DXBuffer* pShader;
	ID3DXBuffer* pErrorMsgs;
	long Ret	= D3DXCompileShaderFromFile(FileName,0,0,pFunctionName,pShaderVersion,0,&pShader,&pErrorMsgs,0);
	if(Ret>=0)
	{
		(*ppShader)	=	CRY_DXPS_CREATE(CCryDXPSBlob,(pShader));
		pShader->Release();
	}
#ifdef _DEBUG
	else
	{
		const char* pBuffer=(const char*)pErrorMsgs->GetBufferPointer();
		OutputDebugString(pBuffer);
	}	
#endif
	return Ret;
#elif CRY_USE_OGL
	char FileName[1024];
	Cry_strcpy_s(FileName,sizeof(FileName),pSrcFile);
	Cry_strcat_s(FileName,sizeof(FileName),"9");
	CGprogram	cgProgram;
	cgProgram = cgCreateProgramFromFile(CCryLayer0OGL::Instance().ShaderContext(), CG_SOURCE,FileName, 
																			*pProfile=='v'?	CCryLayer0OGL::Instance().VertexProfile():
																											CCryLayer0OGL::Instance().FragmentProfile(), pFunctionName, 0);
#ifdef PSGL
	CRY_DEBUGOUT("\n\nShadername: ");
	CRY_DEBUGOUT(FileName);
	CRY_DEBUGOUT("\n\n");
#endif
	if(cgProgram!=0)
	{
//			OutputDebugString(cgGetProgramString(cgProgram, CG_COMPILED_PROGRAM));
		(*ppShader)	=	CRY_DXPS_CREATE(CCryDXPSBlob,(cgProgram));
		return 0;
	}
#elif CRY_USE_GCM
	char FileName[1024];
	Cry_strcpy_s(FileName,sizeof(FileName),pSrcFile);
	Cry_strcat_s(FileName,sizeof(FileName),"9");
	int32 FileID;
	if(CELL_FS_SUCCEEDED!=cellFsOpen(FileName,CELL_FS_O_RDONLY,&FileID,0,0))
	{
		CRY_DEBUGOUT("Could not load file to compile as shader: %s\n",FileName);
		return -1;
	}

	CellFsStat FsStat;
	int Err;
	if(CELL_FS_SUCCEEDED!=(Err=cellFsFstat(FileID,&FsStat)))
	{
		CRY_DEBUGOUT("Could not get filesize of %s in shadercompiler, error:%d\n",FileName,Err);
		cellFsClose(FileID);
		return -1;
	}
	//try first to alloc via scratchpad to avoid minimal memblock which cause memory fragmentation
	char* pData	=	reinterpret_cast<char*>(tdLayer0::Instance().AllocScratchPad(FsStat.st_size+1));
	char* pNewedMem	=	0;
	if(!pData)//scratchpad not available ?
	{
		CRY_DEBUGOUT("Not using scratchpad mem, but allocating %dbytes\n",(int)FsStat.st_size+1);
		pData	=	pNewedMem	=	new	char[FsStat.st_size+1];
	}
	else
	{
		CRY_DEBUGOUT("Using scratchpad mem\n");
	}
	

	uint64_t nRead;
	if(CELL_FS_SUCCEEDED!=cellFsRead(FileID,pData,FsStat.st_size,&nRead) || nRead!=FsStat.st_size)
	{
		CRY_DEBUGOUT("Could not read file: %s to compile as shader\n",FileName);
		if(pNewedMem)
			delete[]	pNewedMem;
		else
			tdLayer0::Instance().FreeScratchPad();
		return -1;
	}

	pData[FsStat.st_size]=0;

	cellFsClose(FileID);

	char* pLocalShader=0;
	const char* args[]={"-fastmath",0};
	const int Ret=compile_program_from_string(pData,
																						*pProfile=='v'?"sce_vp_rsx":"sce_fp_rsx",
																						pFunctionName,
																						args,
																						&pLocalShader);
	if(Ret)
	{
		CRY_DEBUGOUT("Error 0x%x on compiling : %s \n",Ret,FileName);
		if(pNewedMem)
			delete[]	pNewedMem;
		else
			tdLayer0::Instance().FreeScratchPad();
		return -1;
	}
	else
	{
		CRY_DEBUGOUT("Successfully compiled %s of %s\n",pFunctionName,FileName);
	}

	if(pNewedMem)
		delete[]	pNewedMem;
	else
		tdLayer0::Instance().FreeScratchPad();

	const uint32 Size	=	cellGcmCgGetTotalBinarySize((CGprogram)pLocalShader);
	(*ppShader)	=	CRY_DXPS_CREATE(CCryDXPSBlob,(pLocalShader,Size));
	free_compiled_program(pLocalShader);

	return 0;
#else
		//Unsupported DeviceType
#endif
		return -1;
}

size_t cgSrvCompile(const bool VertexShader,
										const char *program,
										const char *entry,
										const char *comment,
										const char **options,
										char *buffer,
										size_t bufferSize);
size_t cgSrvCompile2(const bool VertexShader,
										const char *program,
										const char *entry,
										const char *comment,
										const char **options,
										char *buffer,
										size_t bufferSize);
long D3DX10CompileFromMemory(				const char*								pSrcData,
																		size_t										SrcDataLen,
																		const char*								pSrcFile,
																		const D3D10_SHADER_MACRO*	pDefines,
																		LPD3D10INCLUDE						pInclude,
																		const char*								pFunctionName,
																		const char*								pProfile,
																		uint32										Flags1,
																		uint32										Flags2,
																		ID3DX10ThreadPump*				pPump,
																		ID3D10Blob**							ppShader,
																		ID3D10Blob**							ppErrorMsgs,
																		long*											pResult)
{
	*pResult	=	0;
#ifdef REMOTE_CG_COMPILE
	if(pDefines!=0 || pInclude!=0 || pPump!=0 || ppErrorMsgs==0)
		return -1;

	const size_t bufferSize = 0x40000;
	char *pLocalShader = new char[bufferSize];
/*	char *pLocalShader2 = new char[bufferSize];
	const char* args[]={"-fastmath",0};
	const int32 Size2	=	cgSrvCompile(	*pProfile=='v',//?"sce_vp_rsx":"sce_fp_rsx",
																		pSrcData,
																		pFunctionName,
																		0,
																		args,
																		pLocalShader2,
																		bufferSize);*/
	const int32 Size	=	cgSrvCompile2(*pProfile=='v',//?"sce_vp_rsx":"sce_fp_rsx",
																		pSrcData,
																		pFunctionName,
																		0,
																		0,
																		pLocalShader,
																		bufferSize);
/*	while(Size!=Size2)
	{
		int a=0;
	}
	for(uint32 a=0;a<Size;a++)
	{
		while(pLocalShader[a]!=pLocalShader2[a])
		{
			a--;
		}
	}
	delete[]	pLocalShader2;*/
	const int Ret=Size;
	if(Ret<0)
	{
		CRY_DEBUGOUT("Error 0x%x on compiling : %s \n",Ret,pFunctionName);
		delete[]	pLocalShader;
		return -1;
	}
	else
	{
		CRY_DEBUGOUT("Successfully compiled %s of %s\n",pFunctionName,pSrcFile);
	}
	pLocalShader[Size] = 0;

	(*ppShader)	=	CRY_DXPS_CREATE(CCryDXPSBlob,(&pLocalShader[4],Size+1-4));
	delete[]	pLocalShader;

	return 0;
#else
	char *pLocalShader = 0;
	const char* args[]={"-fastmath -fastprecision -contalloc",0};
	const int Ret=compile_program_from_string(pSrcData,
																						*pProfile=='v'?"sce_vp_rsx":"sce_fp_rsx",
																						pFunctionName,
																						args,
																						&pLocalShader);

	if(Ret)
	{
		CRY_DEBUGOUT("Error 0x%x on compiling : %s \n",Ret,pFunctionName);
		delete[]	pLocalShader;
		return -1;
	}
	else
	{
		CRY_DEBUGOUT("Successfully compiled %s of %s\n",pFunctionName,pSrcFile);
	}

	const uint32 Size	=	cellGcmCgGetTotalBinarySize((CGprogram)pLocalShader);
	(*ppShader)	=	CRY_DXPS_CREATE(CCryDXPSBlob,(pLocalShader,Size+1));
	free_compiled_program(pLocalShader);
	return 0;
#endif
}

HRESULT D3D10ReflectShader(					const void*								pShaderBytecode,
																		size_t										BytecodeLength,
																		ID3D10ShaderReflection**	ppReflector)
{
	*ppReflector=	CRY_DXPS_CREATE(CCryDXPSShaderReflection,((const char*)pShaderBytecode));
	return 0;
}

HRESULT D3D10DisassembleShader(			const void*								pShader,
																		size_t										BytecodeLength,
																		bool											EnableColorCode,
																		const char*								pComments,
																		ID3D10Blob**							ppDisassembly)
{
	assert("D3D10DisassembleShader not implemented yet");
	return -1;
}

HRESULT D3DX10CreateTextureFromMemory(
																		ID3D10Device*							pDevice,
																		const void*								pSrcData,
																		size_t										SrcDataSize,
																		D3DX10_IMAGE_LOAD_INFO*		pLoadInfo,
																		ID3DX10ThreadPump*				pPump,
																		ID3D10Resource**					ppTexture,
																		long*											pResult)
{
	*pResult	=	0;
	if(pLoadInfo->pSrcInfo->ResourceDimension==D3D10_RESOURCE_DIMENSION_TEXTURE2D)
	{
		D3D10_TEXTURE2D_DESC Data;
		Data.Format	=	pLoadInfo->pSrcInfo->Format;
		Data.Height	=	pLoadInfo->pSrcInfo->Height;
		Data.Width	=	pLoadInfo->pSrcInfo->Width;
		if(-1==pDevice->CreateTexture2D(&Data,0,reinterpret_cast<CCryDXPSTexture2D**>(ppTexture)))
			return -1;

		D3D10_MAPPED_TEXTURE2D ResData;
		if(-1==(*reinterpret_cast<CCryDXPSTexture2D**>(ppTexture))->Map(0,D3D10_MAP(),0,&ResData))
			return -1;
		/*
		for(uint32 y=0;y<Data.Height;y++)
			for(uint32 x=0;x<Data.Width && x<ResData.RowPitch;y++)
				reinterpret_cast<uint8*>(ResData.pData)[x+y*ResData.RowPitch]	=	
				reinterpret_cast<const uint8*>(pSrcData)[x+y*Data.Width];
				*/
		//this function is just used for weird runtime texture conversion so we output just debug noise
//		for(uint32 y=0;y<Data.Height;y++)
//			for(uint32 x=0;x<Data.Width && x<ResData.RowPitch;y++)
//				reinterpret_cast<uint32*>(ResData.pData)[(x+y*ResData.RowPitch)/4]	=	x+y*Data.Width;

		return 0;
	}
	return -1;
}

HRESULT D3DX10SaveTextureToFile(		ID3D10Resource*						pSrcTexture,
																D3DX10_IMAGE_FILE_FORMAT	DestFormat,
																const char*								pDestFile)
{
	assert("D3DX10SaveTextureToFile not implemented yet");
	return -1;
}
