#include "RenderPCH.h" 
#include "../Layer0/CCryDXPS.hpp"
#include "CCryDXPSRenderDevice.hpp"
#include "CCryDXPSSwapChain.hpp"
#include "Resource/Textures/CCryDXPSTexture1D.hpp"
#include "Resource/Textures/CCryDXPSTexture2D.hpp"
#include "Resource/Textures/CCryDXPSTexture3D.hpp"
//#include <assert.h>
#include <string.h> 
#include <stdio.h>


//redefine instead of including RenderPCH
//#define max(a,b)    (((a) > (b)) ? (a) : (b))
//#define min(a,b)    (((a) < (b)) ? (a) : (b))

extern "C" void *mwprivate2_memalign(size_t, size_t);
#undef min
#undef max


#ifdef CRY_USE_GCM
	#include <sysutil/sysutil_sysparam.h>
#endif

using namespace cell::Gcm;

CCryDXPSRenderDevice::CCryDXPSRenderDevice():
CCryRefAndWeak<CCryDXPSRenderDevice>(this)
{
#ifdef _DEBUG
	m_BreakDrawCall	=	~0;
	m_DrawCounter		=	0;
#endif
//	memset(m_pVSConstBuffer,0,sizeof(m_pVSConstBuffer));
}

#if defined(CELL_GCM_DEBUG)
void DebugCheckCallback(uint32_t* pCmd)
{
	printf("Bad GCM command 0x%x at mem location: 0x%08x\n",*pCmd, (uint32_t)pCmd);
	abort();
}
#endif

#if defined GCM_DEBUG_CMDBUFFER
CellGcmContextCallback chainedGcmContextCallback = NULL;
uint32_t cmdBufferCopy[GCM_CMD_SIZE / 4];
bool cmdBufferCheckEnabled = false;
#if defined GCM_DEBUG_CMDBUFFER_FULL
uint32_t *cmdBufferBase = NULL;
uint32_t *cmdBufferCurrent = NULL;
#else
size_t cmdBufferCopySize = 0;
#endif

static int32_t debugGcmContextCallback(
		CellGcmContextData *data,
		uint32_t cmdSize)
{
	if (!cmdBufferCheckEnabled)
	{
		// If the check is not enabled in the 'launcher.cfg', then this handler
		// will not be installed at all.  However, we might want to turn of the
		// command buffer checking at runtime using the debugger.  Note that this
		// is possible only by accessing 'cmdBufferCheckEnabled' - changing
		// 'gPS3Env' has no effect!
		// Also note: Checking can not be re-activated after disabling at runtime!
		return chainedGcmContextCallback(data, cmdSize);
	}
#if !defined GCM_DEBUG_CMDBUFFER_FULL
	// Simple and quick check.  Only verify a copy of the active command buffer
	// area.  After the chained callback, the buffer copy is re-initialized.
	size_t size = data->current - data->begin;
	assert(size >= cmdBufferCopySize);
	if (size < cmdBufferCopySize)
	{
		printf("GCM error: (callback) cmdbuffer copy size %u < cmdbuffer size %u\n",
				static_cast<unsigned>(cmdBufferCopySize),
				static_cast<unsigned>(size));
		abort();
	}
	if (memcmp(cmdBufferCopy, data->begin, cmdBufferCopySize * 4) != 0)
	{
		printf("GCM error: (callback) cmdbuffer differs from copy\n");
		abort();
	}
	uint32_t result = chainedGcmContextCallback(data, cmdSize);
	cmdBufferCopySize = data->current - data->begin;
	memcpy(cmdBufferCopy, data->begin, cmdBufferCopySize * 4);
#else
	// Re-initialize the buffer copy after the callback.  If the current pointer
	// has moved since the last call to GCM_CHECK_CMDBUFFER, we'll enforce a
	// check (since this is an indication that we missed a command buffer
	// modification, a GCM_CHECK_CMDBUFFER call is missing somewhere).
	if (data->current != cmdBufferCurrent) { GCM_CHECK_CMDBUFFER }
	uint32_t result = chainedGcmContextCallback(data, cmdSize);
	cmdBufferCurrent = data->current;
	memcpy(cmdBufferCopy, cmdBufferBase, GCM_CMD_SIZE);
#endif
	return result;
}

void debugCheckCmdBuffer(void)
{
	if (!cmdBufferCheckEnabled)
		return;
	CellGcmContextData *const data = gCellGcmCurrentContext;
#if !defined GCM_DEBUG_CMDBUFFER_FULL
	// Simple and quick check.  Only verify a copy of the active command buffer
	// area.  Only the already written part is checked.  This will catch command
	// buffer corruptions early, but it will not catch illegal write accesses to
	// unused areas of the command buffer memory.
	size_t size = data->current - data->begin;
	if (size < cmdBufferCopySize)
	{
		printf("GCM error: (check) cmdbuffer size %u < cmdbuffer copy size %u\n",
				static_cast<unsigned>(size),
				static_cast<unsigned>(cmdBufferCopySize));
		abort();
	}
	if (memcmp(cmdBufferCopy, data->begin, cmdBufferCopySize * 4) != 0)
	{
		puts("GCM error: (check) cmdbuffer differs from copy");
		abort();
	}
	memcpy(
			cmdBufferCopy + cmdBufferCopySize,
			data->begin + cmdBufferCopySize,
			(size - cmdBufferCopySize) * 4);
	cmdBufferCopySize = size;
#else
	// Perform a full verification of the command buffer.  We'll keep a copy of
	// the current pointer from the last call, so the area between the last
	// 'current' pointer and the current 'current' pointer can be excluded from
	// the check (this is the memory are where commands have been written to
	// since the last check).
	assert(data->begin >= cmdBufferBase
			&& data->end < cmdBufferBase + (GCM_CMD_SIZE / 4));
	const size_t previousOffset = cmdBufferCurrent - cmdBufferBase;
	if (memcmp(cmdBufferCopy, cmdBufferBase, previousOffset * 4) != 0)
	{
		puts("GCM error: (check) cmdbuffer corruption before current pointer");
		abort();
	}
	size_t offset = data->current - cmdBufferBase;
	if (memcmp(cmdBufferCopy + offset, data->current,
				GCM_CMD_SIZE - offset * 4) != 0)
	{
		puts("GCM error: (check) cmdbuffer corruption after current pointer");
		abort();
	}
	assert(offset >= previousOffset);
	memcpy(cmdBufferCopy + previousOffset, cmdBufferBase + previousOffset,
			(offset - previousOffset) * 4);
	cmdBufferCurrent = data->current;
#endif
}
#endif

CCryDXPSRenderDevice::CCryDXPSRenderDevice(IDXGIAdapter*			pAdapter,
																					 D3D10_DRIVER_TYPE	DriverType,
																					 int								Software,
																					 uint32				Flags,
																					 uint32				SDKVersion):
CCryRefAndWeak<CCryDXPSRenderDevice>(this),
m_Rendering(false)
{
	//PS3HACK
	//#if defined(_DEBUG)
	void *host_addr = mwprivate2_memalign(1024*1024, GCM_CMD_SIZE);
	//#else
	//	void *host_addr = memalign(1024*1024, GCM_CMD_SIZE);
	//#endif
	if(CELL_OK!=cellGcmInit(0x10000, GCM_CMD_SIZE, host_addr))
	{
		CRY_DEBUGOUT(__FUNCTION__);
		CRY_DEBUGOUT(" Failed to initialize GCM (cellGcmInit())\n");
		return;
	}
	else
	{
		CRY_DEBUGOUT(" successfully initialized GCM\n");
	}

#if defined GCM_DEBUG_CMDBUFFER
	cmdBufferCheckEnabled = gPS3Env->bEnableCmdBufferCheck;
	if (cmdBufferCheckEnabled)
	{
		chainedGcmContextCallback = gCellGcmCurrentContext->callback;
		gCellGcmCurrentContext->callback = debugGcmContextCallback;
  #if defined GCM_DEBUG_CMDBUFFER_FULL
		cmdBufferBase = reinterpret_cast<uint32_t *>(host_addr);
		memcpy(cmdBufferCopy, cmdBufferBase, GCM_CMD_SIZE);
		cmdBufferCurrent = gCellGcmCurrentContext->current;
  #endif
		puts("GCM cmdbuffer check enabled");
	}
#endif

#if defined(CRY_USE_GCM_HUD)
	cellGcmHUDInit();
#endif
	cellGcmGetConfiguration(&m_GCMConfig);//returns void
	{
		CRY_DEBUGOUT("GCMConfiguration:\n local: %8x %4dMB %4dMHz\n  main: %8x %4dMB %4dMHz\n",
								(unsigned int)m_GCMConfig.localAddress,m_GCMConfig.localSize/(1024*1024),m_GCMConfig.memoryFrequency/1000000,
								(unsigned int)m_GCMConfig.ioAddress,m_GCMConfig.ioSize/(1024*1024),m_GCMConfig.coreFrequency/1000000);
	}
	CCryLayer0GCM::Instance().Memory().Init(m_GCMConfig.localAddress,m_GCMConfig.localSize);

//	for(size_t a=0;a<sizeof(m_PSConstBuffer)/4;a++)
//		m_PSConstBuffer[a]=0.f;

#if defined(CELL_GCM_DEBUG)
	gCellGcmDebugCheckCallback = DebugCheckCallback;
	cellGcmDebugCheckEnable(CELL_GCM_TRUE);
#endif
}

CCryDXPSRenderDevice::~CCryDXPSRenderDevice()
{
}


long CCryDXPSRenderDevice::CreateSwapChain(CCryDXPSSwapChain& rSwapChain,DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
{
/*	CellVideoOutConfiguration videocfg;
	memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
	const uint32_t ScrH	=	pSwapChainDesc->BufferDesc.Height;
	videocfg.resolutionId = ScrH>=1080?1:ScrH>=720?2:ScrH>=480?4:5;
	while(videocfg.resolutionId<6 && !cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY,videocfg.resolutionId,0,0))
	{
		videocfg.resolutionId++;
	}
	videocfg.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	videocfg.pitch	=	pSwapChainDesc->BufferDesc.Width*4;
	int Ret=cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0);
	if(CELL_OK!=Ret)
	{
		CRY_DEBUGOUT("Failed on cellVideoOutConfigure(). (0x%x)\n", Ret);
		return -1;
	}
	else
	{
		CRY_DEBUGOUT("VideoOutConfigure resolutionID:%d Pitch:%d\n",videocfg.resolutionId,videocfg.pitch);
	}*/




	return 0;
}

void CCryDXPSRenderDevice::VSSetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer *const *ppConstantBuffers)
{
//	assert(NumBuffers==1);
/*	m_pVSConstBuffer	=	*ppConstantBuffers;
	m_VSCBOffset	=	Offset;
	CRY_ASSERT_MESSAGE(m_VSCBOffset==0,"m_VSCBOffset not 0");
	CRY_ASSERT_MESSAGE(NumBuffers==1,"More than one ConstBuffer not supported yet");
	CRY_ASSERT_MESSAGE(Offset/4+m_pVSConstBuffer->Size()/16<=468,"More 468 consts in VSConstBuffer not supported yet");
	if(Offset/4+m_pVSConstBuffer->Size()/16>468)
	{
		int a=0;
	}
*/
//	cellGcmSetVertexProgramConstants(m_VSCBOffset,m_pVSConstBuffer->Size()/4,reinterpret_cast<const f32*>(m_pVSConstBuffer->RawData()));

//	for(uint32 a=0;a<NumBuffers;a++,Offset+=ppConstantBuffers[a]->Size()/4)
//		if(ppConstantBuffers[a] && ppConstantBuffers[a]->Valid())
//		{
//			const f32* pConsts	=	reinterpret_cast<const f32*>(ppConstantBuffers[a]->RawData());
//			cellGcmSetVertexProgramConstants(Offset,ppConstantBuffers[a]->Size()/4,pConsts);
	//		m_pVShader->Set(m_pVSConstBuffer,m_VSCBOffset);
//		}

//	for(uint32 a=0;a<NumBuffers;a++,Offset+=(*ppConstantBuffers)->Size()/4)
//		cellGcmSetVertexProgramConstants(Offset,(*ppConstantBuffers)->Size()/4,reinterpret_cast<const f32*>((*ppConstantBuffers)->RawData()));

//	for(uint32 a=0;a<NumBuffers;a++,Offset+=ppConstantBuffers[a]->Size()/4)
//		if(ppConstantBuffers[a] && ppConstantBuffers[a]->Valid())
//		{
//		cellGcmSetVertexProgramConstants(Offset,(*ppConstantBuffers)->Size()/4,reinterpret_cast<const f32*>((*ppConstantBuffers)->RawData()));
//			m_pVSConstBuffer[Offset]	=	ppConstantBuffers[a];
//			for(uint32 b=1,Size=ppConstantBuffers[a]->Size()/4;b<Size;b++)
//				m_pVSConstBuffer[Offset+b]	=	0;
//		}

//	for(uint32 a=0;a<NumBuffers;a++,Offset+=(*ppConstantBuffers)->Size()/4)
//		cellGcmSetVertexProgramConstants(Offset,(*ppConstantBuffers)->Size()/4,reinterpret_cast<const f32*>((*ppConstantBuffers)->RawData()));
		for(uint32 a=0;a<NumBuffers && a+Offset<16;a++)
			m_pVSConstBuffer[Offset+a]	=	ppConstantBuffers[a];
}

void CCryDXPSRenderDevice::PSSetShaderResources(uint32	Offset,uint32 NumViews,ID3D10ShaderResourceView *const *ppShaderResourceViews)
{
	for(uint32 a=0;a<NumViews;a++)
	{
		APWeakTexture&	pTex	=	ppShaderResourceViews[a]->Texture();
		if(pTex->Type()==EDXPS_RT_TEX1D)
		{
		}
		else
		if(pTex->Type()==EDXPS_RT_TEX2D)
		{
	#if defined(_DEBUG)
			CRY_ASSERT_MESSAGE(pTex->GcmTexture()->width>0,"(*ppShaderResourceViews)->Texture2D()->GcmTexture()->width>0");
			CRY_ASSERT_MESSAGE(pTex->GcmTexture()->height>0,"(*ppShaderResourceViews)->Texture2D()->GcmTexture()->height>0");
	#endif

			GCM_CHECK_CMDBUFFER
			cellGcmSetTexture(Offset,pTex->GcmTexture());
		}
		else
		if(pTex->Type()==EDXPS_RT_TEX3D)
		{
		}
	}
}

void CCryDXPSRenderDevice::PSSetShader(ID3D10PixelShader *pPixelShader)
{
	m_pPShader	=	pPixelShader;
}

void CCryDXPSRenderDevice::PSSetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState *const *ppSamplers)
{
	for(uint32 a=0;a<NumSamplers;a++)
		ppSamplers[a]->Set(Offset+a);
}

void CCryDXPSRenderDevice::VSSetShader(ID3D10VertexShader *pVertexShader)
{
	m_pVShader	=	pVertexShader;
}

void CCryDXPSRenderDevice::EndScene()
{
}
static const 	uint32 g_Prims[]	=
{
	~0,																	//	D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED					=	0,
	CELL_GCM_PRIMITIVE_POINTS,					//	D3D10_PRIMITIVE_TOPOLOGY_POINTLIST					=	1,
	CELL_GCM_PRIMITIVE_LINES,						//	D3D10_PRIMITIVE_TOPOLOGY_LINELIST						=	2,
	CELL_GCM_PRIMITIVE_LINE_STRIP,				//	D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP					=	3,
	CELL_GCM_PRIMITIVE_TRIANGLES,				//	D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST				=	4,
	CELL_GCM_PRIMITIVE_TRIANGLE_STRIP,	//	D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP			=	5,
	~0,
	~0,
	~0,
	~0,
	~0,																	//	D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ				=	10,
	~0,																	//	D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ			=	11,
	~0,																	//	D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ		=	12,
	~0																	//	D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ	=	13,
};

static int NoDraw=0;
void CCryDXPSRenderDevice::DrawIndexed(uint32 IndexCount,uint32 StartIndexLocation,int BaseVertexLocation)
{
	//PS3HACK
	if(NoDraw)
		return;
	DXPS_PROFILE_FRAME(CCryDXPSRenderDevice::DrawIndexed);

	if(g_Prims[m_Topology]==~0)
	{
			CRY_DEBUGOUT("unsupported primitive/topology type \"DrawIndexed\"\n");
			return;
	}


	if(	!m_pInputLayout ||!m_pInputLayout->Valid()	||
			!m_pVShader || !m_pVShader->Valid()	||
			!m_pPShader || !m_pPShader->Valid()	||
//			!m_pVSConstBuffer || !m_pVSConstBuffer->Valid()	||
			!m_pVertexBuffer[0] || !m_pVertexBuffer[0]->Valid()	||
			!m_pIndexBuffer || !m_pIndexBuffer->Valid())
	{
		if(!m_pInputLayout || !m_pInputLayout->Valid())
			CRY_DEBUGOUT("m_pInputLayout was not valid during \"DrawIndexed\"\n");
		if(!m_pVShader || !m_pVShader->Valid())
			CRY_DEBUGOUT("m_pVShader was not valid during \"DrawIndexed\"\n");
		if(!m_pPShader || !m_pPShader->Valid())
			CRY_DEBUGOUT("m_pPShader was not valid during \"DrawIndexed\"\n");
//		if(!m_pVSConstBuffer || !m_pVSConstBuffer->Valid())
//			CRY_DEBUGOUT("m_pVSConstBuffer was not valid during \"DrawIndexed\"\n");
//		if(!m_pPSConstBuffer || !m_pPSConstBuffer->Valid())
//			CRY_DEBUGOUT("m_pPSConstBuffer was not valid during \"DrawIndexed\"\n");
		if(!m_pVertexBuffer[0] || !m_pVertexBuffer[0]->Valid())
			CRY_DEBUGOUT("m_pVertexBuffer was not valid during \"DrawIndexed\"\n");
		if(!m_pIndexBuffer || !m_pIndexBuffer->Valid())
			CRY_DEBUGOUT("m_pIndexBuffer was not valid during \"DrawIndexed\"\n");
		return;
	}
	uint32_t IBOffset=0;
	if(CELL_OK!=cellGcmAddressToOffset(&m_pIndexBuffer->RawData()[StartIndexLocation*(m_IBFormat==DXGI_FORMAT_R16_UINT?2:4)],&IBOffset))
	{
		CRY_DEBUGOUT("Error in AddressToOffset \"DrawIndexed\"\n");
		return;
	}

#if _DEBUG
	//PS3HACK 
	if(m_DrawCounter==m_BreakDrawCall)
	{
		int a=0;
	}
	m_DrawCounter++;
#endif

	m_pVShader->Set(m_pVSConstBuffer);
	const uint8* pVBArray[16];
	pVBArray[0]	=	&m_pVertexBuffer[0]->RawData()[m_VBOffset[0]+BaseVertexLocation*m_VBStride[0]];
	for(uint32 a=1;a<16;a++)
		pVBArray[a]	=	m_pVertexBuffer[a] && m_pVertexBuffer[a]->Valid()?
									&m_pVertexBuffer[a]->RawData()[m_VBOffset[a]+BaseVertexLocation*m_VBStride[a]]:
									0;
	m_pInputLayout->Set(pVBArray,16,m_VBStride,0);
	m_pPShader->Set(m_pPSConstBuffer);
	GCM_CHECK_CMDBUFFER
	cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
	cellGcmSetInvalidateVertexCache();
	cellGcmSetDrawIndexArray(	g_Prims[m_Topology],
														IndexCount,
														m_IBFormat==DXGI_FORMAT_R16_UINT?	CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16:
																															CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32,
														CELL_GCM_LOCATION_LOCAL,
														IBOffset);
}

void CCryDXPSRenderDevice::Draw(uint32 VertexCount,uint32 StartVertexLocation)
{
	DXPS_PROFILE_FRAME(CCryDXPSRenderDevice::Draw);

	if(g_Prims[m_Topology]==~0)
	{
			CRY_DEBUGOUT("unsupported primitive/topology type \"DrawIndexed\"\n");
			return;
	}

	if(	!m_pInputLayout ||!m_pInputLayout->Valid()	||
			!m_pVShader || !m_pVShader->Valid()	||
			!m_pPShader || !m_pPShader->Valid()	||
			!m_pVertexBuffer[0] || !m_pVertexBuffer[0]->Valid())
	{
		if(!m_pInputLayout || !m_pInputLayout->Valid())
			CRY_DEBUGOUT("m_pInputLayout was not valid during \"Draw\"\n");
		if(!m_pVShader || !m_pVShader->Valid())
			CRY_DEBUGOUT("m_pVShader was not valid during \"Draw\"\n");
		if(!m_pPShader || !m_pPShader->Valid())
			CRY_DEBUGOUT("m_pPShader was not valid during \"Draw\"\n");
		if(!m_pVertexBuffer[0] || !m_pVertexBuffer[0]->Valid())
			CRY_DEBUGOUT("m_pVertexBuffer was not valid during \"Draw\"\n");
		return;
	}

#if _DEBUG
	//PS3HACK 
	if(m_DrawCounter==m_BreakDrawCall)
	{
		int a=0;
	}
	m_DrawCounter++;
#endif

	m_pVShader->Set(m_pVSConstBuffer);
	const uint8* pVBArray[16];
	pVBArray[0]	=	&m_pVertexBuffer[0]->RawData()[m_VBOffset[0]];
	for(uint32 a=1;a<16;a++)
		pVBArray[a]	=	m_pVertexBuffer[a] && m_pVertexBuffer[a]->Valid()?
									&m_pVertexBuffer[a]->RawData()[m_VBOffset[a]]:
									0;
	m_pInputLayout->Set(pVBArray,16,m_VBStride,0);
	m_pPShader->Set(m_pPSConstBuffer);
	GCM_CHECK_CMDBUFFER
	cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
	cellGcmSetInvalidateVertexCache();
	cellGcmSetDrawArrays(g_Prims[m_Topology], StartVertexLocation, VertexCount);
}

void CCryDXPSRenderDevice::PSSetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer *const *ppConstantBuffers)
{
//	assert(NumBuffers==1);
//	m_pPSConstBuffer	=	*ppConstantBuffers;
//	m_PSCBOffset	=	Offset;
//	for(uint32 a=0;a<NumBuffers;a++,Offset+=(*ppConstantBuffers)->Size()/4)
//		memcpy(&m_PSConstBuffer[Offset<<2],reinterpret_cast<const f32*>((*ppConstantBuffers)->RawData()),(*ppConstantBuffers)->Size());
		for(uint32 a=0;a<NumBuffers && a+Offset<16;a++)
			m_pPSConstBuffer[Offset+a]	=	ppConstantBuffers[a];
}

void CCryDXPSRenderDevice::IASetInputLayout(ID3D10InputLayout *pInputLayout)
{
	m_pInputLayout=pInputLayout;
}

void CCryDXPSRenderDevice::IASetVertexBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer *const *ppVertexBuffers,const uint32*pStrides,const uint32*pOffsets)
{
//	CRY_ASSERT_MESSAGE(Offset==0,"Not implemented yet!");
//	CRY_ASSERT_MESSAGE(NumBuffers==1,"Not implemented yet!");
	if(Offset+NumBuffers<=16)
	{
		for(uint32 a=0;a<NumBuffers;a++)
		{
			m_pVertexBuffer[Offset+a]	=	ppVertexBuffers[a];
			m_VBStride[Offset+a]			=	pStrides[a];
			m_VBOffset[Offset+a]			=	pOffsets[a];
		}
	}
	else
	{
		CRY_DEBUGOUT("ERROR: while setting vertexbuffers Offset:%d + NumBuffers:%d exceed 16 for the limit of inputslots",Offset,NumBuffers);
	}
}

void CCryDXPSRenderDevice::IASetIndexBuffer(ID3D10Buffer *pIndexBuffer,DXGI_FORMAT Format,uint32 Offset)
{
	CRY_ASSERT_MESSAGE(Offset==0,"Not implemented yet!");
//	assert(pIndexBuffer);//will be set to 0 every frame once.
	m_pIndexBuffer=	pIndexBuffer;
	m_IBFormat		=	Format;
	m_IBOffset		=	Offset;
}

void CCryDXPSRenderDevice::DrawIndexedInstanced( uint32 IndexCountPerInstance,uint32 InstanceCount,uint32 StartIndexLocation,int BaseVertexLocation,uint32 StartInstanceLocation)
{
	DXPS_PROFILE_FRAME(CCryDXPSRenderDevice::DrawIndexed);

	if(g_Prims[m_Topology]==~0)
	{
			CRY_DEBUGOUT("unsupported primitive/topology type \"DrawIndexed\"\n");
			return;
	}


	if(	!m_pInputLayout ||!m_pInputLayout->Valid()	||
			!m_pVShader || !m_pVShader->Valid()	||
			!m_pPShader || !m_pPShader->Valid()	||
			!m_pVertexBuffer[0] || !m_pVertexBuffer[0]->Valid()	||
			!m_pIndexBuffer || !m_pIndexBuffer->Valid())
	{
		if(!m_pInputLayout || !m_pInputLayout->Valid())
			CRY_DEBUGOUT("m_pInputLayout was not valid during \"DrawIndexed\"\n");
		if(!m_pVShader || !m_pVShader->Valid())
			CRY_DEBUGOUT("m_pVShader was not valid during \"DrawIndexed\"\n");
		if(!m_pPShader || !m_pPShader->Valid())
			CRY_DEBUGOUT("m_pPShader was not valid during \"DrawIndexed\"\n");
		if(!m_pVertexBuffer[0] || !m_pVertexBuffer[0]->Valid())
			CRY_DEBUGOUT("m_pVertexBuffer was not valid during \"DrawIndexed\"\n");
		if(!m_pIndexBuffer || !m_pIndexBuffer->Valid())
			CRY_DEBUGOUT("m_pIndexBuffer was not valid during \"DrawIndexed\"\n");
		return;
	}
	uint32_t IBOffset=0;
	if(CELL_OK!=cellGcmAddressToOffset(&m_pIndexBuffer->RawData()[StartIndexLocation*(m_IBFormat==DXGI_FORMAT_R16_UINT?2:4)],&IBOffset))
	{
		CRY_DEBUGOUT("Error in AddressToOffset \"DrawIndexed\"\n");
		return;
	}

#if _DEBUG
	//PS3HACK 
	if(m_DrawCounter==m_BreakDrawCall)
	{
		int a=0;
	}
	m_DrawCounter++;
#endif

	m_pVShader->Set(m_pVSConstBuffer);
	const uint8* pVBArray[16];
	for(uint32 a=0;a<16;a++)
		pVBArray[a]	=	m_pVertexBuffer[a] && m_pVertexBuffer[a]->Valid()?
									&m_pVertexBuffer[a]->RawData()[m_VBOffset[a]+BaseVertexLocation*m_VBStride[a]]:
									0;
	m_pInputLayout->Set(pVBArray,16,m_VBStride,IndexCountPerInstance);
	m_pPShader->Set(m_pPSConstBuffer);
	cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
	cellGcmSetInvalidateVertexCache();


	uint32	IBFormat	=	m_IBFormat==DXGI_FORMAT_R16_UINT?	CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16:
																												CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32;
	cellGcmSetDrawIndexArray(	g_Prims[m_Topology],
														IndexCountPerInstance,
														IBFormat,
														CELL_GCM_LOCATION_LOCAL,
														IBOffset);

	for(uint32 a=1;a<InstanceCount;a++)
	{
		m_pInputLayout->SetUpdateInstance(pVBArray,16,m_VBStride,IndexCountPerInstance,a);
		cellGcmSetInvalidateVertexCache();
		cellGcmSetDrawIndexArray(	g_Prims[m_Topology],
															IndexCountPerInstance,
															IBFormat,
															CELL_GCM_LOCATION_LOCAL,
															IBOffset);
	}
}

void CCryDXPSRenderDevice::DrawInstanced( uint32 VertexCountPerInstance,uint32 InstanceCount,uint32 StartVertexLocation,uint32 StartInstanceLocation)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GSSetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer *const *ppConstantBuffers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GSSetShader(ID3D10GeometryShader *pShader)
{
	CRY_ASSERT_MESSAGE(!pShader,"Not implemented yet!");
}

void CCryDXPSRenderDevice::IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY Topology)
{
	m_Topology	=	Topology;
}

void CCryDXPSRenderDevice::VSSetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView *const *ppShaderResourceViews)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::VSSetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState *const *ppSamplers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::SetPredication(ID3D10Predicate *pPredicate,uint32 PredicateValue)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GSSetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView *const *ppShaderResourceViews)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GSSetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState *const *ppSamplers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::OMSetRenderTargets(uint32 NumViews,ID3D10RenderTargetView *const *ppRenderTargetViews,ID3D10DepthStencilView *pDepthStencilView)
{
	if(NumViews>CRYDXPSDEVICE_MAXRENDERTARGETS)
		NumViews	=	CRYDXPSDEVICE_MAXRENDERTARGETS;
	m_pRenderTargetViews[0]	=	NumViews>0?ppRenderTargetViews[0]->Texture():0;
	m_pRenderTargetViews[1]	=	NumViews>1?ppRenderTargetViews[1]->Texture():0;
	m_pRenderTargetViews[2]	=	NumViews>2?ppRenderTargetViews[2]->Texture():0;
	m_pDepthStencilView	=	pDepthStencilView?pDepthStencilView->Texture():0;

	//CRY_DEBUGOUT("Number of views%d\n",NumViews);
	UpdateRendertargets();
}

void CCryDXPSRenderDevice::UpdateRendertargets()
{
	CellGcmSurface sf;
	//basic initialisation
	sf.colorLocation[0]	= 
	sf.colorLocation[1]	= 
	sf.colorLocation[2]	= 
	sf.colorLocation[3]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorOffset[0]	=
	sf.colorOffset[1]	=
	sf.colorOffset[2]	=
	sf.colorOffset[3]	=	0;
	sf.colorPitch[0]	=
	sf.colorPitch[1]	=
	sf.colorPitch[2]	=
	sf.colorPitch[3]	= 64;//needs to be at least 64 regarding to the rsx doc

	m_ClearableFormat	=	true;
	for(uint32 a=0;m_pRenderTargetViews[a];a++)
	{
		m_ClearableFormat	&=	DXGI_FORMAT2GCM[m_pRenderTargetViews[a]->Format()]==CELL_GCM_TEXTURE_A8R8G8B8;
		sf.colorFormat 	= GCM_TextureFormat2SurfaceFormat(DXGI_FORMAT2GCM[m_pRenderTargetViews[a]->Format()]);
		sf.colorTarget	= CELL_GCM_SURFACE_TARGET_0;
		sf.colorLocation[a]	= CELL_GCM_LOCATION_LOCAL;
		if(CELL_OK!=cellGcmAddressToOffset(m_pRenderTargetViews[a]->RawPointer(),&sf.colorOffset[a]))
		{
			CRY_DEBUGOUT("Setting View failed on AddressToOffset of View %d\nppRenderTargetView: %x\nColorOffset %d\n",a,(unsigned int)m_pRenderTargetViews[a]->RawPointer(),(int)sf.colorOffset[a]);
		}
		sf.colorPitch[a] 		= m_pRenderTargetViews[a]->Pitch(0);
	}

	if(m_pDepthStencilView)
	{
		sf.depthFormat 	= GCM_TextureFormat2SurfaceFormat(DXGI_FORMAT2GCM[m_pDepthStencilView->Format()]);
		sf.depthLocation	= CELL_GCM_LOCATION_LOCAL;
		if(CELL_OK!=cellGcmAddressToOffset(m_pDepthStencilView->RawPointer(),&sf.depthOffset))
		{
			CRY_DEBUGOUT("Setting depthstencil failed on AddressToOffset \nppDepthStencilView: %x\nColorOffset %d\n",(unsigned int)m_pDepthStencilView->RawPointer(),(int)sf.colorOffset);
		}
		sf.depthPitch 	= m_pDepthStencilView->Pitch(0);
	}
	else
	{
		sf.depthOffset	= 0;
		sf.depthPitch 	= 64;
		sf.depthFormat 	= CELL_GCM_SURFACE_Z24S8;
		sf.depthLocation	= CELL_GCM_LOCATION_LOCAL;
	}

	sf.type		= CELL_GCM_SURFACE_PITCH;
	sf.antialias	= CELL_GCM_SURFACE_CENTER_1;

	sf.width 		= m_pRenderTargetViews[0]?m_pRenderTargetViews[0]->SizeX():0;
	sf.height 	= m_pRenderTargetViews[0]?m_pRenderTargetViews[0]->SizeY():0;
	sf.x 		= 0;
	sf.y 		= 0;

	GCM_CHECK_CMDBUFFER
	cellGcmSetSurface(&sf);

	cellGcmSetColorMaskMrt(	CELL_GCM_COLOR_MASK_MRT1_A|CELL_GCM_COLOR_MASK_MRT1_R|CELL_GCM_COLOR_MASK_MRT1_G|CELL_GCM_COLOR_MASK_MRT1_B|
													CELL_GCM_COLOR_MASK_MRT2_A|CELL_GCM_COLOR_MASK_MRT2_R|CELL_GCM_COLOR_MASK_MRT2_G|CELL_GCM_COLOR_MASK_MRT2_B|
													CELL_GCM_COLOR_MASK_MRT3_A|CELL_GCM_COLOR_MASK_MRT3_R|CELL_GCM_COLOR_MASK_MRT3_G|CELL_GCM_COLOR_MASK_MRT3_B);
	float scale[4],offset[4];
	scale[0] = m_GCMViewPort.Width * 0.5f;
	scale[1] = m_GCMViewPort.Height * -0.5f;
	scale[2] = 0.5f;
	scale[3] = 0.0f;
	offset[0] = m_GCMViewPort.TopLeftX + scale[0];
	offset[1] = m_GCMViewPort.Height - m_GCMViewPort.TopLeftY + scale[1];
	offset[2] = 0.5f;
	offset[3] = 0.0f;
	cellGcmSetViewport(	m_GCMViewPort.TopLeftX,
											m_GCMViewPort.TopLeftY,
											m_GCMViewPort.Width,
											m_GCMViewPort.Height, 
											0.f, 
											1.f, scale, offset);
//	cellGcmSetDepthTestEnable(CELL_GCM_TRUE);
//	cellGcmSetDepthTestEnable(CELL_GCM_FALSE);
//	cellGcmSetDepthFunc(CELL_GCM_LESS);

}

void CCryDXPSRenderDevice::OMSetBlendState(ID3D10BlendState *pBlendState,const float BlendFactor[4],uint32 SampleMask)
{
	if(!pBlendState)
	{
		GCM_CHECK_CMDBUFFER
		cellGcmSetBlendEnable(CELL_GCM_FALSE);
	}

	pBlendState->Set();
	if(BlendFactor)
	{
		int R=static_cast<int>(BlendFactor[0]*255.f);
		int G=static_cast<int>(BlendFactor[1]*255.f);
		int B=static_cast<int>(BlendFactor[2]*255.f);
		int A=static_cast<int>(BlendFactor[3]*255.f);
		R	=	R>255?255:(R<0)?0:R;
		G	=	G>255?255:(G<0)?0:G;
		B	=	B>255?255:(B<0)?0:B;
		A	=	A>255?255:(A<0)?0:A;
		GCM_CHECK_CMDBUFFER
		cellGcmSetBlendColor((B|(G<<8)|(R<<16)|(A<<24)),0);
	}
}

void CCryDXPSRenderDevice::OMSetDepthStencilState(ID3D10DepthStencilState *pDepthStencilState,uint32 StencilRef)
{
	pDepthStencilState->Set(StencilRef);
}

void CCryDXPSRenderDevice::SOSetTargets(uint32 NumBuffers,ID3D10Buffer *const *ppSOTargets,uint32 *pOffsets)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::DrawAuto( void)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::RSSetState(ID3D10RasterizerState *pRasterizerState)
{
	pRasterizerState->Set();
}

void CCryDXPSRenderDevice::RSSetViewports(uint32 NumViewports,const D3D10_VIEWPORT *pViewports)
{
//	CRY_DEBUGOUT(__FUNCTION__);
	m_GCMViewPort	=	*pViewports;
//	CRY_DEBUGOUT("\nViewPort %d %d %d %d\n",pViewports->TopLeftX,pViewports->TopLeftY,pViewports->Width,pViewports->Height);

}

void CCryDXPSRenderDevice::RSSetScissorRects(uint32 NumRects,const D3D10_RECT *pRects)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::CopyResource(ID3D10Resource *pDstResource,ID3D10Resource *pSrcResource)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}


inline uint32_t cellUtilCountLeadingZero(uint32_t x)
{
	uint32_t y;
	uint32_t n = 32;

	y = x >> 16; if (y != 0) { n = n - 16; x = y; }
	y = x >>  8; if (y != 0) { n = n -  8; x = y; }
	y = x >>  4; if (y != 0) { n = n -  4; x = y; }
	y = x >>  2; if (y != 0) { n = n -  2; x = y; }
	y = x >>  1; if (y != 0) { return n - 2; }
	return n - x;
}
 
inline uint32_t cellUtilLog2(uint32_t x)
{
	return 31 - cellUtilCountLeadingZero(x);
}


void convertSwizzle(uint8*& dst,const uint8*&	src,
										const uint32_t SrcPitch, const uint32_t depth,
										const uint32_t xpos, const uint32_t ypos,
										const uint32_t SciX1,const uint32_t SciY1,
										const uint32_t SciX2,const uint32_t SciY2,
										const uint32_t level)
{
	if (level == 1)
	{
		switch(depth)
		{
		case 16:
			if(xpos>=SciX1 && xpos<SciX2 && ypos>=SciY1 && ypos<SciY2)
			{
				//					*((uint32_t*&)dst)++ = ((uint32_t*)src)[ypos * width + xpos];
				//					*((uint32_t*&)dst)++ = ((uint32_t*)src)[ypos * width + xpos+1];
				//					*((uint32_t*&)dst)++ = ((uint32_t*)src)[ypos * width + xpos+2];
				//					*((uint32_t*&)dst)++ = ((uint32_t*)src)[ypos * width + xpos+3];
				*((uint32_t*&)dst)++ = *((uint32_t*)(src+(ypos * SrcPitch + xpos*16)));
				*((uint32_t*&)dst)++ = *((uint32_t*)(src+(ypos * SrcPitch + xpos*16+4)));
				*((uint32_t*&)dst)++ = *((uint32_t*)(src+(ypos * SrcPitch + xpos*16+8)));
				*((uint32_t*&)dst)++ = *((uint32_t*)(src+(ypos * SrcPitch + xpos*16+12)));
			}
			else
				((uint32_t*&)dst)+=4;
			break;
		case 8:
			if(xpos>=SciX1 && xpos<SciX2 && ypos>=SciY1 && ypos<SciY2)
			{
				*((uint32_t*&)dst)++ = *((uint32_t*)(src+(ypos * SrcPitch + xpos*8)));
				*((uint32_t*&)dst)++ = *((uint32_t*)(src+(ypos * SrcPitch + xpos*8+4)));
			}
			else
				((uint32_t*&)dst)+=2;
			break;
		case 4:
			if(xpos>=SciX1 && xpos<SciX2 && ypos>=SciY1 && ypos<SciY2)
				*((uint32_t*&)dst)++ = *((uint32_t*)(src+(ypos * SrcPitch + xpos)));
			else
				((uint32_t*&)dst)++;
			break;
		case 3:
			if(xpos>=SciX1 && xpos<SciX2 && ypos>=SciY1 && ypos<SciY2)
			{
				*dst++ = src[ypos * SrcPitch + xpos * depth];
				*dst++ = src[ypos * SrcPitch + xpos * depth + 1];
				*dst++ = src[ypos * SrcPitch + xpos * depth + 2];
			}
			else
				dst+=3;
			break;
		case 1:
			if(xpos>=SciX1 && xpos<SciX2 && ypos>=SciY1 && ypos<SciY2)
				*dst++ = src[ypos * SrcPitch + xpos * depth];
			else
				dst++;
			break;
		}
		return;
	}
	else
	{
		convertSwizzle(dst, src, SrcPitch, depth, xpos, ypos,SciX1,SciY1,SciX2,SciY2, level - 1);
		convertSwizzle(dst, src, SrcPitch, depth,xpos + (1U << (level - 2)), ypos,SciX1,SciY1,SciX2,SciY2, level - 1);
		convertSwizzle(dst, src, SrcPitch, depth,xpos, ypos + (1U << (level - 2)),SciX1,SciY1,SciX2,SciY2, level - 1);
		convertSwizzle(dst, src, SrcPitch, depth, xpos + (1U << (level - 2)),ypos + (1U << (level - 2)),SciX1,SciY1,SciX2,SciY2, level - 1);
	}
}

void Linear2Swizzle(		uint8*	 dst,
										const uint8*	 src,
										const uint32_t SrcPitch,
										const uint32_t width,
										const uint32_t height,
										const uint32_t depth,
										const uint32_t SciX1,const uint32_t SciY1,
										const uint32_t SciX2,const uint32_t SciY2)
{
	//PS3HACK 
	if(width==0 || height==0)
	{
		while(true)
		{
			CRY_ASSERT_MESSAGE(width==0 || height==0,"width==0 || height==0");
			int a=0;
		}
	}


	/*	cellGcmConvertSwizzleFormat(dst,src,
	0,0,0,width,height,0,
	0,0,0,width,height,0,width,height,0,
	depth,depth,CELL_GCM_FALSE,2,NULL);

	return;*/

	src-=SciY1*SrcPitch+SciX1*depth;
	if (width == height)
		convertSwizzle(dst, src, SrcPitch, depth, 0, 0,SciX1,SciY1,SciX2,SciY2, cellUtilLog2(width) + 1);
	else
		if (width > height)
		{
			uint32_t baseLevel	= cellUtilLog2(width)-(cellUtilLog2(width)-cellUtilLog2(height));
			for (uint32_t i = 0;i < (1UL << (cellUtilLog2(width)-cellUtilLog2(height))); i++)
				convertSwizzle(dst, src, SrcPitch, depth,(1U << baseLevel)*i, 0,SciX1,SciY1,SciX2,SciY2, baseLevel + 1);
		}
		else
			//	if (width < height)//wtf
		{
			uint32_t baseLevel	= cellUtilLog2(height)-(cellUtilLog2(height)-cellUtilLog2(width));
			for (uint32_t i = 0;i < (1UL << (cellUtilLog2(height)-cellUtilLog2(width))); i++)
				convertSwizzle(dst, src, SrcPitch, depth, 0,(1U << baseLevel)*i,SciX1,SciY1,SciX2,SciY2, baseLevel + 1);
		}
}

void CCryDXPSRenderDevice::CopySubresourceRegion(ID3D10Resource *pDstResource,uint32 DstSubresource,uint32 DstX,uint32 DstY,uint32 DstZ,ID3D10Resource *pSrcResource,uint32 SrcSubresource,const D3D10_BOX *pSrcBox)
{
	//SLOW could be implemented as gpu transfer
	if(pDstResource && pSrcResource)
	{
		if(pDstResource->Type()==EDXPS_RT_TEX2D	&&	pSrcResource->Type()==EDXPS_RT_TEX2D)
		{
			CCryDXPSTexture2D* pDst	=	(CCryDXPSTexture2D*)pDstResource;
			CCryDXPSTexture2D* pSrc	=	(CCryDXPSTexture2D*)pSrcResource;
			if(!pSrcBox && !DstSubresource && !DstX && !DstY && !DstZ)
			{
//				const uint8* pSrcData;//	=	reinterpret_cast<uint8*>(pSrc->RawPointer());
//				uint8* pDstData;//	=	reinterpret_cast<uint8*>(pDst->RawPointer());
				const uint32	SrcPitch	=	pSrc->Pitch(DstSubresource);
				const uint32	DstPitch	=	pDst->Pitch(DstSubresource);
				const uint32	LineLen		=	SrcPitch<=DstPitch?SrcPitch:DstPitch;
				D3D10_MAPPED_TEXTURE2D pDstPtr;
				D3D10_MAPPED_TEXTURE2D pSrcPtr;
				pDst->Map(DstSubresource,D3D10_MAP_VOID,0,&pDstPtr);
				pSrc->Map(DstSubresource,D3D10_MAP_VOID,0,&pSrcPtr);
				for(uint32 y=0,SizeY=pSrc->SizeY();y<SizeY;y++)
//					memset(&reinterpret_cast<uint8*>(pDstPtr.pData)[y*DstPitch],0x7f,LineLen);
					memcpy(&reinterpret_cast<uint8*>(pDstPtr.pData)[y*DstPitch],&reinterpret_cast<uint8*>(pSrcPtr.pData)[y*SrcPitch],LineLen);

/*				uint32 Format	=	pDst->GcmTexture()->format;

				if(Format==CELL_GCM_TEXTURE_COMPRESSED_DXT1)
					Linear2Swizzle(reinterpret_cast<uint8*>(pDstPtr.pData),reinterpret_cast<uint8*>(pSrcPtr.pData),pDst->SizeX()>>2,pDst->SizeY()>>2,8,0,0,pSrc->SizeX()>>2,pSrc->SizeY()>>2);
				else
				if(	Format==CELL_GCM_TEXTURE_COMPRESSED_DXT23 ||
						Format==CELL_GCM_TEXTURE_COMPRESSED_DXT45)
					Linear2Swizzle(reinterpret_cast<uint8*>(pDstPtr.pData),reinterpret_cast<uint8*>(pSrcPtr.pData),pDst->SizeX()>>2,pDst->SizeY()>>2,16,0,0,pSrc->SizeX()>>2,pSrc->SizeY()>>2);
				else
					Linear2Swizzle(reinterpret_cast<uint8*>(pDstPtr.pData),reinterpret_cast<uint8*>(pSrcPtr.pData),pDst->SizeX(),pDst->SizeY(),GCM_TextureFormatSize(1,pDst->GcmTexture()->format),0,0,pSrc->SizeX(),pSrc->SizeY());
*/

				pDst->Unmap(DstSubresource);
				pSrc->Unmap(DstSubresource);
			}
			else
			{
				CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
				return;
			}
		}
		else
		if((pDstResource->Type()==EDXPS_RT_VERTEXBUFFER || pDstResource->Type()==EDXPS_RT_INDEXBUFFER || pDstResource->Type()==EDXPS_RT_CONSTBUFFER)
//					&&	(pSrcResource->Type()==EDXPS_RT_VERTEXBUFFER || pSrcResource->Type()==EDXPS_RT_INDEXBUFFER || pSrcResource->Type()==EDXPS_RT_CONSTBUFFER)
				)
		{
			CCryDXPSBuffer* pDst	=	(CCryDXPSBuffer*)pDstResource;
			CCryDXPSBuffer* pSrc	=	(CCryDXPSBuffer*)pSrcResource;
			const int SrcOffset	=	pSrcBox?pSrcBox->left:0;
			const int SrcSize		=	(pSrcBox?pSrcBox->right:pSrc->Size())-SrcOffset;
			const int	DstOffset	=	DstX;
			const int	DstSize		=	pDst->Size()-DstX;
			const int TransSize	=	DstSize<SrcSize?DstSize:SrcSize;
			const uint8*	pSrcData	=	&(pSrc->RawData()[SrcOffset]);
			uint8*	pDstData;//	=	&(pDst->RawData()[DstOffset]);
			pDst->Map(D3D10_MAP_VOID,0,(void**)&pDstData);
			pDstData+=DstOffset;
	//		for(uint32 a=0,Size=DstSize<SrcSize?DstSize:SrcSize;a<Size;a++)//SLOW
	//			pDstData[a]	=	pSrcData[a];
			memcpy(pDstData,pSrcData,TransSize);
			pDst->Unmap();
		}
		else
		{
			CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
			return;
		}

	}
}

void CCryDXPSRenderDevice::UpdateSubresource(ID3D10Resource *pDstResource,uint32 DstSubresource,const D3D10_BOX *pDstBox,const void *pSrc,uint32 SrcRowPitch,uint32 SrcDepthPitch)
{
	if(pDstResource->Type()==EDXPS_RT_CONSTBUFFER  && !pDstBox)
	{
		if(DstSubresource)
		{
				CRY_ASSERT_MESSAGE(0,"DstSubresource is not zero");
				return;
		}

		CCryDXPSBuffer* pDst	=	(CCryDXPSBuffer*)pDstResource;
		const uint8* pSrcData	=	reinterpret_cast<const uint8*>(pSrc);
		uint8*	pDstData;//			=	pDst->RawData();
		pDst->Map(D3D10_MAP_VOID,0,(void**)&pDstData);
		for(uint32 a=0,Size=pDst->Size();a<Size;a++)
			pDstData[a]	=	pSrcData[a];
		pDst->Unmap();
	}
	else
	if(pDstResource->Type()==EDXPS_RT_TEX2D)
	{
		CCryDXPSTexture2D* pDst	=	(CCryDXPSTexture2D*)pDstResource;
		if(pDst && pSrc)
		{
			const int	DstStartX	=	std::max(pDstBox?pDstBox->left:(unsigned int)0,(unsigned int)0)>>DstSubresource;
			const int	DstEndX		=	std::max(std::min(pDstBox?pDstBox->right:(unsigned int)0,pDst->SizeX())>>DstSubresource,1u);
			const int	DstStartY	=	std::max(pDstBox?pDstBox->top:(unsigned int)0,(unsigned int)0)>>DstSubresource;
			const int	DstEndY		=	std::max(std::min(pDstBox?pDstBox->bottom:(unsigned int)0,pDst->SizeY())>>DstSubresource,1u);
			uint32 Format	=	pDst->GcmTexture()->format;

			D3D10_MAPPED_TEXTURE2D pDstPtr;
			if(pDst->Map(DstSubresource,D3D10_MAP_VOID,0,&pDstPtr)<0)
				return;

			if((Format&CELL_GCM_TEXTURE_LN))
			{
				for(uint32 y=DstStartY;y<DstEndY;y++)//SLOW
				{
					const uint8*	pSrcData	=	&reinterpret_cast<const uint8*>(pSrc)[y*SrcRowPitch];
					uint8*	pDstData	=	&reinterpret_cast<uint8*>(pDstPtr.pData)[y*pDstPtr.RowPitch];
					for(uint32	x		=GCM_TextureFormatSize(DstStartX,pDst->GcmTexture()->format),
											xend=GCM_TextureFormatSize(DstEndX,pDst->GcmTexture()->format);x<xend;x++)
							pDstData[x]	=	pSrcData[x];
				}
			}
			else
			{
				Format&~(CELL_GCM_TEXTURE_LN|CELL_GCM_TEXTURE_UN);
				if(Format==CELL_GCM_TEXTURE_COMPRESSED_DXT1)
					memcpy(pDstPtr.pData,pSrc,std::max(pDst->SizeX()>>DstSubresource,1u)*std::max(pDst->SizeY()>>DstSubresource,1u)/2);
					//Linear2Swizzle((uint8*)pDst->RawPointer(),(const uint8*)pSrc,pDst->SizeX()>>2,pDst->SizeY()>>2,8,DstStartX>>2,DstStartY>>2,DstEndX>>2,DstEndY>>2);
				else
				if(	Format==CELL_GCM_TEXTURE_COMPRESSED_DXT23 ||
						Format==CELL_GCM_TEXTURE_COMPRESSED_DXT45)
/*				{
					for(uint a=0,S=pDst->SizeX()*pDst->SizeY()/4;a<S;a++)
					{
						((uint32*)pDst->RawPointer())[a]	=	((uint32*)pSrc)[a^1];
					}
				}*/
					memcpy(pDstPtr.pData,pSrc,std::max(pDst->SizeX()>>DstSubresource,1u)*std::max(pDst->SizeY()>>DstSubresource,1u));
//					Linear2Swizzle((uint8*)pDst->RawPointer(),(const uint8*)pSrc,pDst->SizeX()>>2,pDst->SizeY()>>2,16,DstStartX>>2,DstStartY>>2,DstEndX>>2,DstEndY>>2);
				else
					Linear2Swizzle(reinterpret_cast<uint8*>(pDstPtr.pData),(const uint8*)pSrc,SrcRowPitch,std::max(pDst->SizeX()>>DstSubresource,1u),std::max(pDst->SizeY()>>DstSubresource,1u),GCM_TextureFormatSize(1,pDst->GcmTexture()->format),DstStartX,DstStartY,DstEndX,DstEndY);
			}
			pDst->Unmap(DstSubresource);
		}
	}
	else
	{
		CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	}
}

void CCryDXPSRenderDevice::ClearRenderTargetView( ID3D10RenderTargetView *pRenderTargetView,const float ColorRGBA[ 4 ])
{
	if(!m_ClearableFormat)
		return;
	int R=static_cast<int>(ColorRGBA[0]*255.f);
	int G=static_cast<int>(ColorRGBA[1]*255.f);
	int B=static_cast<int>(ColorRGBA[2]*255.f);
	int A=static_cast<int>(ColorRGBA[3]*255.f);
	R	=	R>255?255:(R<0)?0:R;
	G	=	G>255?255:(G<0)?0:G;
	B	=	B>255?255:(B<0)?0:B;
	A	=	A>255?255:(A<0)?0:A;
//	static CC=0xffe;
//	CC^=CC<<17;
//	CC+=5464561;
//	cellGcmSetClearColor((B|(G<<8)|(R<<16)|(A<<24))^CC);
  GCM_CHECK_CMDBUFFER
	cellGcmSetClearColor((B|(G<<8)|(R<<16)|(A<<24)));
	cellGcmSetClearSurface(CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A);

}

void CCryDXPSRenderDevice::ClearDepthStencilView( ID3D10DepthStencilView *pDepthStencilView,uint32 ClearFlags,float Depth,uint8 Stencil)
{
	
	int32 ClearValue=static_cast<int32>(Depth*static_cast<float>(1<<24));
	ClearValue	=	((ClearValue>0xffffff?0xffffff:ClearValue<0?0:ClearValue)<<8)|Stencil;

	uint32 Mask=(ClearFlags&(0x01L))?CELL_GCM_CLEAR_Z:0;
	if(ClearFlags&(0x02L))//D3D10_DDI_CLEAR_STENCIL)
		Mask	|=	CELL_GCM_CLEAR_S;
	if(Mask)
	{
		GCM_CHECK_CMDBUFFER
		cellGcmSetClearDepthStencil(ClearValue);
		cellGcmSetClearSurface(Mask);
	}
	else
	{
		CRY_ASSERT_MESSAGE(0,"Nothing cleared in ClearDepthStencilView!");
	}
}

void CCryDXPSRenderDevice::GenerateMips(ID3D10ShaderResourceView *pShaderResourceView)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::ResolveSubresource(ID3D10Resource *pDstResource,uint32 DstSubresource,ID3D10Resource *pSrcResource,uint32 SrcSubresource,DXGI_FORMAT Format)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::VSGetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer **ppConstantBuffers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::PSGetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView **ppShaderResourceViews)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::PSGetShader(ID3D10PixelShader **ppPixelShader)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::PSGetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState **ppSamplers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::VSGetShader(ID3D10VertexShader **ppVertexShader)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::PSGetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer **ppConstantBuffers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::IAGetInputLayout(ID3D10InputLayout **ppInputLayout)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::IAGetVertexBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer **ppVertexBuffers,uint32*pStrides,uint32 *pOffsets)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::IAGetIndexBuffer(ID3D10Buffer **pIndexBuffer,DXGI_FORMAT *Format,uint32*Offset)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GSGetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer **ppConstantBuffers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GSGetShader(ID3D10GeometryShader **ppGeometryShader)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::IAGetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY *pTopology)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::VSGetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView **ppShaderResourceViews)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::VSGetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState **ppSamplers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GetPredication(ID3D10Predicate **ppPredicate,BOOL *pPredicateValue)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GSGetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView **ppShaderResourceViews)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GSGetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState **ppSamplers)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::OMGetRenderTargets(uint32 NumViews,ID3D10RenderTargetView **ppRenderTargetViews,ID3D10DepthStencilView **ppDepthStencilView)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::OMGetBlendState(ID3D10BlendState **ppBlendState,float BlendFactor[ 4 ],uint32*pSampleMask)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::OMGetDepthStencilState(ID3D10DepthStencilState **ppDepthStencilState,uint32*pStencilRef)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::SOGetTargets(uint32 NumBuffers,ID3D10Buffer **ppSOTargets,uint32* pOffsets)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::RSGetState(ID3D10RasterizerState **ppRasterizerState)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::RSGetViewports(uint32* NumViewports,D3D10_VIEWPORT *pViewports)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::RSGetScissorRects(uint32* NumRects,D3D10_RECT *pRects)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

long CCryDXPSRenderDevice::GetDeviceRemovedReason( void)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::SetExceptionMode(uint32 intRaiseFlags)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

uint32 CCryDXPSRenderDevice::GetExceptionMode( void)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return 0;
}

long CCryDXPSRenderDevice::GetPrivateData(const ECryGUID & guid,uint32*pDataSize,void *pData)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::SetPrivateData(const ECryGUID & guid,uint32 DataSize,const void *pData)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::SetPrivateDataInterface( const ECryGUID & guid,const IUnknown *pData)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}
void CCryDXPSRenderDevice::SetTextFilterSize(unsigned int Width,unsigned int Height)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::GetTextFilterSize(unsigned int* Width,unsigned int* Height)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}


void CCryDXPSRenderDevice::ClearState( void)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

void CCryDXPSRenderDevice::Flush( void)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

long CCryDXPSRenderDevice::CreateBuffer(const D3D10_BUFFER_DESC *pDesc,const D3D10_SUBRESOURCE_DATA *pInitialData,ID3D10Buffer **ppBuffer)
{
	CCRYDXPSResType Type=EDXPS_RT_UNUSED;
	if(pDesc->BindFlags==D3D10_BIND_VERTEX_BUFFER)
	{
		Type	=	EDXPS_RT_VERTEXBUFFER;
	}
	else
	if(pDesc->BindFlags==D3D10_BIND_INDEX_BUFFER)
	{
		Type	=	EDXPS_RT_INDEXBUFFER;
	}
	else
	if(pDesc->BindFlags==D3D10_BIND_CONSTANT_BUFFER)
	{
		Type	=	EDXPS_RT_CONSTBUFFER;
	}

	*ppBuffer	=	CRY_DXPS_CREATE(CCryDXPSBuffer,(pInitialData?pInitialData->pSysMem:0,pDesc->ByteWidth,Type));
	return 0;
}

long CCryDXPSRenderDevice::CreateTexture1D(const D3D10_TEXTURE1D_DESC *pDesc,const D3D10_SUBRESOURCE_DATA *pInitialData,ID3D10Texture1D **ppTexture)
{
	*ppTexture	=	CRY_DXPS_CREATE(CCryDXPSTexture1D,(	pDesc->Width,
																										pDesc->MipLevels,
																										pDesc->Format));
	return (*ppTexture!=0)-1;
}

long CCryDXPSRenderDevice::CreateTexture2D(const D3D10_TEXTURE2D_DESC *pDesc,const D3D10_SUBRESOURCE_DATA *pInitialData,ID3D10Texture2D **ppTexture)
{
	*ppTexture	=	CRY_DXPS_CREATE(CCryDXPSTexture2D,(	pDesc->Width,
																										pDesc->Height,
																										pDesc->MipLevels,
																										pDesc->Format,0));

	if(pInitialData)
	{
		D3D10_BOX Box;
		Box.back	=	
		Box.front	=
		Box.left	=
		Box.top		=	0;

		for(uint a=0;a<pDesc->MipLevels;a++)
		{
			Box.right	=	pDesc->Width>>a;
			Box.bottom=	pDesc->Height>>a;
			if(!Box.right)Box.right=1;
			if(!Box.bottom)Box.bottom=1;
			UpdateSubresource(*ppTexture,a,&Box,pInitialData[a].pSysMem,pInitialData[a].SysMemPitch,0);
		}
	}
	return (*ppTexture!=0)-1;
}

long CCryDXPSRenderDevice::CreateTexture3D(const D3D10_TEXTURE3D_DESC *pDesc,const D3D10_SUBRESOURCE_DATA *pInitialData,ID3D10Texture3D **ppTexture)
{
	*ppTexture	=	CRY_DXPS_CREATE(CCryDXPSTexture3D,(	pDesc->Width,
																										pDesc->Height,
																										pDesc->Depth,
																										pDesc->MipLevels,
																										pDesc->Format));
	return (*ppTexture!=0)-1;
}

long CCryDXPSRenderDevice::CreateShaderResourceView( CCryDXPSResource *pResource,const D3D10_SHADER_RESOURCE_VIEW_DESC *pDesc,ID3D10ShaderResourceView **ppSRView)
{
	*ppSRView	=	CRY_DXPS_CREATE(CCryDXPSShaderResourceView,(pResource,pDesc));
	return (*ppSRView!=0)-1;
}

long CCryDXPSRenderDevice::CreateRenderTargetView(ID3D10Resource *pResource,const D3D10_RENDER_TARGET_VIEW_DESC *pDesc,ID3D10RenderTargetView **ppRTView)
{
//	assert(!pDesc);
	assert(pResource->Type()==EDXPS_RT_TEX2D);
	if(/*!pDesc &&*/ pResource->Type()==EDXPS_RT_TEX2D)
	{
		CCryDXPSTexture2D* pTex	=	reinterpret_cast<CCryDXPSTexture2D*>(pResource);
		(*ppRTView)	=	CRY_DXPS_CREATE(CCryDXPSRenderTargetView,(pTex));
		return 0;
	}
	return -1;
}

long CCryDXPSRenderDevice::CreateDepthStencilView(ID3D10Resource *pResource,const D3D10_DEPTH_STENCIL_VIEW_DESC *pDesc,ID3D10DepthStencilView **ppDepthStencilView)
{
	*ppDepthStencilView 	=	pResource->Type()==EDXPS_RT_TEX2D?CRY_DXPS_CREATE(CCryDXPSDepthStencilView,(reinterpret_cast<CCryDXPSTexture2D*>(pResource))):0;
	return (ppDepthStencilView!=0)-1;	
}

HRESULT	CCryDXPSRenderDevice::QueryInterface(REFIID riid,void **ppvObject)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::CreateInputLayout(const D3D10_INPUT_ELEMENT_DESC *pInputElementDescs,uint32 NumElements,const void *pShaderBytecodeWithInputSignature,size_t Size,ID3D10InputLayout **ppInputLayout)
{
/*	CCryDXPSInputLayout::SLayout  Descriptor[16];//16 is the max input count regarding to gcm_reference
	assert(NumElements<=16);
	uint32 a=0;
	for(;a<NumElements;a++)
	{
		Descriptor[a].m_Offset	=	pInputElementDescs[a].AlignedByteOffset;
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R32_FLOAT)
		{
			Descriptor[a].m_Type				=	CELL_GCM_VERTEX_F;
			Descriptor[a].m_Size				=	1;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R16G16_SINT)
		{
			Descriptor[a].m_Type				=	CELL_GCM_VERTEX_S32K;
			Descriptor[a].m_Size				=	2;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R32G32_FLOAT)
		{
			Descriptor[a].m_Type				=	CELL_GCM_VERTEX_F;
			Descriptor[a].m_Size				=	2;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R32G32B32_FLOAT)
		{
			Descriptor[a].m_Type				=	CELL_GCM_VERTEX_F;
			Descriptor[a].m_Size				=	3;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R32G32B32A32_FLOAT)
		{
			Descriptor[a].m_Type				=	CELL_GCM_VERTEX_F;
			Descriptor[a].m_Size				=	4;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			Descriptor[a].m_Type				=	CELL_GCM_VERTEX_UB;
			Descriptor[a].m_Size				=	4;
		}
		else
		{
			CRY_DEBUGOUT("Unknown Format in InputLayout %d\n",pInputElementDescs[a].Format);
			return -1;
		}

		Descriptor[a].m_SemanticName[0]	=	0;

		assert(strlen(pInputElementDescs[a].SemanticName)+1<=sizeof(Descriptor[a].m_SemanticName));
		if(strlen(pInputElementDescs[a].SemanticName)+1>sizeof(Descriptor[a].m_SemanticName))
		{
			CRY_DEBUGOUT("Size of Semanticname %s exceeds the limit of %d bytes\n",pInputElementDescs[a].SemanticName,sizeof(Descriptor[a].m_SemanticName));
			return -1;
		}
		strcpy(Descriptor[a].m_SemanticName,pInputElementDescs[a].SemanticName);
	}
	(*ppInputLayout)	=	CRY_DXPS_CREATE(CCryDXPSInputLayout,(Descriptor,NumElements));
*/
	(*ppInputLayout)	=	CRY_DXPS_CREATE(CCryDXPSInputLayout,(pInputElementDescs,NumElements,pShaderBytecodeWithInputSignature));
	return 0;
}

long CCryDXPSRenderDevice::CreateVertexShader(const void *pShaderBytecode,size_t Size,ID3D10VertexShader **ppVertexShader)
{
//	CRY_DEBUGOUT("ByteCodePtr: %x\n",(uint32)pShaderBytecode);
	*ppVertexShader	=	CRY_DXPS_CREATE(CCryDXPSShader,(pShaderBytecode,EDXPS_ST_VERTEX));
	return 0;
}

long CCryDXPSRenderDevice::CreateGeometryShader(const void *pShaderBytecode,size_t Size,ID3D10GeometryShader **ppGeometryShader)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::CreateGeometryShaderWithStreamOutput(const void *pShaderBytecode,size_t BytecondeLength,const D3D10_SO_DECLARATION_ENTRY *pSODeclaration,uint32 NumEntries,uint32 OutputStreamStride,ID3D10GeometryShader **ppGeometryShader)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::CreatePixelShader(const void *pShaderBytecode,size_t Size,ID3D10PixelShader **ppPixelShader)
{
	*ppPixelShader	=	CRY_DXPS_CREATE(CCryDXPSShader,(pShaderBytecode,EDXPS_ST_PIXEL));
	return 0;
}

long CCryDXPSRenderDevice::CreateBlendState(const D3D10_BLEND_DESC *pBlendStateDesc,ID3D10BlendState **ppBlendState)
{
	*ppBlendState	=	CRY_DXPS_CREATE(CCryDXPSBlendState,(pBlendStateDesc));
	return 0;
}

long CCryDXPSRenderDevice::CreateDepthStencilState(const D3D10_DEPTH_STENCIL_DESC *pDepthStencilDesc,ID3D10DepthStencilState **ppDepthStencilState)
{
	*ppDepthStencilState	=	CRY_DXPS_CREATE(CCryDXPSDepthStencilState,(pDepthStencilDesc));
	return 0;
}

long CCryDXPSRenderDevice::CreateRasterizerState(const D3D10_RASTERIZER_DESC *pRasterizerDesc,ID3D10RasterizerState **ppRasterizerState)
{
	*ppRasterizerState	=	CRY_DXPS_CREATE(CCryDXPSRasterizerState,(pRasterizerDesc));
	
	return 0;
}

long CCryDXPSRenderDevice::CreateSamplerState(const D3D10_SAMPLER_DESC *pSamplerDesc,ID3D10SamplerState **ppSamplerState)
{
	*ppSamplerState 	=	CRY_DXPS_CREATE(CCryDXPSSamplerState,(pSamplerDesc));
	return ppSamplerState?0:-1;	
}

long CCryDXPSRenderDevice::CreateQuery(const D3D10_QUERY_DESC *pQueryDesc,ID3D10Query **ppQuery)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::CreatePredicate(const D3D10_QUERY_DESC *pPredicateDesc,ID3D10Predicate **ppPredicate)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::CreateCounter(const D3D10_COUNTER_DESC *pCounterDesc,ID3D10Counter **ppCounter)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

long CCryDXPSRenderDevice::CheckFormatSupport(DXGI_FORMAT Format,uint32*pFormatSupport)
{
	switch(Format)
	{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_R16_TYPELESS:
			*pFormatSupport=D3D10_FORMAT_SUPPORT_TEXTURE2D;
			return 0;
	}
	return -1;
}

long CCryDXPSRenderDevice::CheckMultisampleQualityLevels(DXGI_FORMAT Format,uint32 SampleCount,uint32*pNumQualityLevels)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

void CCryDXPSRenderDevice::CheckCounterInfo(D3D10_COUNTER_INFO *pCounterInfo)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
}

long CCryDXPSRenderDevice::CheckCounter(const D3D10_COUNTER_DESC *pDesc,D3D10_COUNTER_TYPE *pType,uint32*pActiveCounters,const char* szName,uint32 *pNameLength,const char* szUnits,uint32 *pUnitsLength,const char* szDescription,uint32 *pDescriptionLength)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

uint32 CCryDXPSRenderDevice::GetCreationFlags( void)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return 0;
}

long CCryDXPSRenderDevice::OpenSharedResource(int hResource,const CRYIID & ReturnedInterface,void **ppResource)
{
	CRY_ASSERT_MESSAGE(0,"Not implemented yet!");
	return -1;
}

