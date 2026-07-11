#ifndef __CRYRENDERER__
#define __CRYRENDERER__

//struct DXGI_SWAP_CHAIN_DESC;

#include "Device/CCryDXPSRenderDevice.hpp"
#include "Device/CCryDXPSSwapChain.hpp"
#include "Device/CCryDXPSBlob.hpp"

//typedef class ID3D10Blob ID3D10Blob;
//typedef ID3D10Blob *LPD3D10BLOB;

long D3D10CreateDevice(							IDXGIAdapter*					pAdapter,
																		D3D10_DRIVER_TYPE 		DriverType,
																		int										Software,
																		uint32					Flags,
																		uint32					SDKVersion,
																		ID3D10Device**				ppDevice);

long D3D10CreateDeviceAndSwapChain(	IDXGIAdapter*					pAdapter,
																		D3D10_DRIVER_TYPE 		DriverType,
																		int										Software,
																		uint32					Flags,
																		uint32					SDKVersion,
																		DXGI_SWAP_CHAIN_DESC*	pSwapChainDesc,
																		IDXGISwapChain**			ppSwapChain,    
																		ID3D10Device**				ppDevice);

long D3D10CreateBlob(								size_t								NumBytes,
										 								LPD3D10BLOB*					ppBuffer);


long D3DX10CompileShaderFromFile(	 const char*								pSrcFile,
																	 const D3D10_SHADER_MACRO*	pDefines,
																	 LPD3D10INCLUDE							pInclude,
																	 const char*								pFunctionName,
																	 const char*								pProfile,
																	 uint32											Flags,
																	 ID3DX10ThreadPump*					pPump,
																	 ID3D10Blob**								ppShader,
																	 ID3D10Blob**								ppErrorMsgs);

long D3DX10CompileFromMemory(
																		const char*								pSrcData,
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
																		long*											pResult);

HRESULT D3D10ReflectShader(					const void*								pShaderBytecode,
																		size_t										BytecodeLength,
																		ID3D10ShaderReflection**	ppReflector);

HRESULT D3D10DisassembleShader(			const void*								pShader,
																		size_t										BytecodeLength,
																		bool											EnableColorCode,
																		const char*								pComments,
																		ID3D10Blob**							ppDisassembly);

HRESULT D3DX10SaveTextureToFile(		ID3D10Resource*						pSrcTexture,
																		D3DX10_IMAGE_FILE_FORMAT	DestFormat,
																		const char*								pDestFile);

HRESULT D3DX10CreateTextureFromMemory(
																		ID3D10Device*							pDevice,
																		const void*								pSrcData,
																		size_t										SrcDataSize,
																		D3DX10_IMAGE_LOAD_INFO*		pLoadInfo,
																		ID3DX10ThreadPump*				pPump,
																		ID3D10Resource**					ppTexture,
																		long*											pResult);
#endif

