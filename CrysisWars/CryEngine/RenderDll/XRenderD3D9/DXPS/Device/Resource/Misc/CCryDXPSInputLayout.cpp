#include "RenderPCH.h"
#include "../../../Layer0/CCryDXPS.hpp"
#include "CCryDXPSInputLayout.hpp"
#include "CCryDXPSShader.hpp"
#include "CCryDXPSShaderDesc.hpp"
//#include <assert.h>
#include <string.h>

const SRefVertexAttribute* GetParameterBySemantic(const CDXPSShaderDesc*	pShaderDesc,const char* pSemantic,uint SemanticIndex)
{
	const uint32 SemLen	=	strlen(pSemantic);
	for(uint32 a=0,Size=pShaderDesc->VertexAttributeCount();a<Size;a++)
	{
		const char* pAttributeSemName	=	&pShaderDesc->NameTable()[pShaderDesc->VertexAttribute()[a].m_SemanticNameIndex];
		if(!strncmp(pAttributeSemName,pSemantic,SemLen))
		{
				const uint32 ParamLen=strlen(pAttributeSemName);
				if(ParamLen==SemLen)
					return &pShaderDesc->VertexAttribute()[a];
					if(ParamLen==SemLen+1)
					{
						const uint32 SIndex=pAttributeSemName[ParamLen-1]-'0';
						if(SIndex==SemanticIndex)
							return &pShaderDesc->VertexAttribute()[a];
					}
		}

	}
	return 0;
/*	CGparameter Param=cellGcmCgGetFirstLeafParameter(Prog,CG_PROGRAM);
	if(!Param)
		CRY_DEBUGOUT("No Params\n");
	else
	{
		const uint32 SemLen	=	strlen(pSemantic);
		do
		{
			const	uint32 InputOffset	=	cellGcmCgGetParameterResource(Prog,Param) - CG_ATTR0;
			if(InputOffset<16)
			{
				const char* pSem=cellGcmCgGetParameterSemantic(Prog,Param);
				if(!strncmp(pSem,pSemantic,SemLen))
				{
					const uint32 ParamLen=strlen(pSem);
					if(ParamLen==SemLen)
						return Param;
					if(ParamLen==SemLen+1)
					{
						const uint32 SIndex=pSem[ParamLen-1]-'0';
						if(SIndex==SemanticIndex)
							return Param;
					}
				}
			}
			Param	=	cellGcmCgGetNextLeafParameter(Prog,Param);
		}while(Param);
	}
	return 0;*/
}




CCryDXPSInputLayout::CCryDXPSInputLayout(const D3D10_INPUT_ELEMENT_DESC *pInputElementDescs,uint32 NumElements,const void *pShaderBytecodeWithInputSignature):
CCryDXPSResource(EDXPS_RT_INPUTLAYOUT),
CCryRefAndWeak<CCryDXPSInputLayout>(this)
{
//	const uint32 ByteSize	=	reinterpret_cast<const uint32*>(pShaderBytecodeWithInputSignature)[0];
//	CGprogram	Prog	=	(CGprogram)&((uint8*)pShaderBytecodeWithInputSignature)[4];
	const CDXPSShaderDesc*	pShaderDesc	=	(const CDXPSShaderDesc*)&((const uint8*)pShaderBytecodeWithInputSignature)[4];

	m_Count=0;
	m_FrequencyDivider	=	0;

	for(uint32 a=0;a<NumElements;a++)
	{
		uint32 Size=0,Type=0,AttribPitch;
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R32_FLOAT)
		{
			Type				=	CELL_GCM_VERTEX_F;
			Size				=	1;
			AttribPitch	=	4;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R16G16_SINT)
		{
			Type				=	CELL_GCM_VERTEX_S32K;
			Size				=	2;
			AttribPitch	=	4;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R32G32_FLOAT)
		{
			Type				=	CELL_GCM_VERTEX_F;
			Size				=	2;
			AttribPitch	=	8;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R32G32B32_FLOAT)
		{
			Type				=	CELL_GCM_VERTEX_F;
			Size				=	3;
			AttribPitch	=	12;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R32G32B32A32_FLOAT)
		{
			Type				=	CELL_GCM_VERTEX_F;
			Size				=	4;
			AttribPitch	=	16;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			Type				=	CELL_GCM_VERTEX_UB;
			Size				=	4;
			AttribPitch	=	4;
		}
		else
		if(pInputElementDescs[a].Format==DXGI_FORMAT_R16G16B16A16_SNORM)
		{
			Type				=	CELL_GCM_VERTEX_S1;
			Size				=	4;
			AttribPitch	=	8;
		}
		else
		{
			CRY_DEBUGOUT("Unknown Format in InputLayout %d\n",pInputElementDescs[a].Format);
		}




#if defined(_DEBUG)
		m_Layout[m_Count].m_SemanticName[0]	=	0;
		assert(strlen(pInputElementDescs[a].SemanticName)+1<=sizeof(m_Layout[m_Count].m_SemanticName));
		if(strlen(pInputElementDescs[a].SemanticName)+1>sizeof(m_Layout[m_Count].m_SemanticName))
		{
			CRY_DEBUGOUT("Size of Semanticname %s exceeds the limit of %d bytes\n",pInputElementDescs[a].SemanticName,sizeof(m_Layout[m_Count].m_SemanticName));
			break;
		}
		strcpy(m_Layout[m_Count].m_SemanticName,pInputElementDescs[a].SemanticName);
#endif




		const SRefVertexAttribute* pAttr = GetParameterBySemantic(pShaderDesc,pInputElementDescs[a].SemanticName,pInputElementDescs[a].SemanticIndex);
		if(!pAttr)
			continue;
/*		uint32 InputOffsetAdd=0;
		if(!Param)
			for(int32 b=((int32)pInputElementDescs[a].SemanticIndex)-1;!Param && b>=0;b--,InputOffsetAdd++)
			{
				Param = GetParameterBySemantic(Prog,pInputElementDescs[a].SemanticName,b);
			}
		if(!Param)
		{
			continue;
		}*/

//		const	uint32 InputOffset	=	cellGcmCgGetParameterResource(Prog,Param) - CG_ATTR0 + InputOffsetAdd;

		//useless, returns always just one row
/*			const CGtype T		=	cellGcmCgGetParameterType(Prog,Param);
...*/

		//already tested for valid InputOffset in GetParameterBySemantic
		CRY_ASSERT_MESSAGE(pInputElementDescs[a].AlignedByteOffset<256 && (pInputElementDescs[a].AlignedByteOffset&3)==0,"InputLayout fails due to AlignedByteOffset>=128 or not 4byte alligned"); 
		CRY_ASSERT_MESSAGE(pInputElementDescs[a].InstanceDataStepRate<65536,"InputLayout fails due to InstanceDataStepRate>=65536"); 
		CRY_ASSERT_MESSAGE(Type<8,"InputLayout fails due to Type>=8"); 
		CRY_ASSERT_MESSAGE(Size>0 && Size<5,"InputLayout fails due to Size==0 or Size>4"); 

#if defined(_DEBUG)
		while(Size>4 || Size==0 || Type==0)
		{
			int a=0;
		}
#endif
//		m_Layout[m_Count].m_InputOffset = InputOffset;
		m_Layout[m_Count].m_InputOffset = pAttr->m_InputOffset;
		m_Layout[m_Count].m_AttribCount	= pAttr->m_Size;
		m_Layout[m_Count].m_AttribPitch	= AttribPitch;
		m_FrequencyDivider	|=	(pInputElementDescs[a].InstanceDataStepRate==0?1:0)<<pAttr->m_InputOffset;
		m_Layout[m_Count].m_ByteOffset=	pInputElementDescs[a].AlignedByteOffset;
		m_Layout[m_Count].m_Frequency	=	pInputElementDescs[a].InstanceDataStepRate;
		m_Layout[m_Count].m_Type			=	Type;
		m_Layout[m_Count].m_Size			=	Size;
		m_Layout[m_Count].m_InputSlot	=	pInputElementDescs[a].InputSlot;
		m_Count++;

	}

}

void CCryDXPSInputLayout::Set(const uint8** pVBufferArray,uint32 BufferCount,uint32* pStride,uint32 IndexCount)
{
	using namespace cell::Gcm;
	DXPS_PROFILE_FRAME(DXPS_SetVertexLayout);

	cellGcmSetFrequencyDividerOperation(m_FrequencyDivider);	//dividers set to division or modulo

	for(uint32 a=0;a<m_Count;a++)
	{
		uint32 ElementOffset;
		if(m_Layout[a].m_InputSlot<BufferCount)
		{
			const uint8* pVBuffer	=	pVBufferArray[m_Layout[a].m_InputSlot];
			if(pVBuffer)
			{
				cellGcmAddressToOffset(&pVBuffer[m_Layout[a].m_ByteOffset],&ElementOffset);
				for(uint32 ac=0;ac<m_Layout[a].m_AttribCount;ac++)
				{
					cellGcmSetVertexDataArray(
														m_Layout[a].m_InputOffset+ac,
														m_Layout[a].m_Frequency*IndexCount,
														pStride[m_Layout[a].m_InputSlot],
														m_Layout[a].m_Size,
														m_Layout[a].m_Type,
														CELL_GCM_LOCATION_LOCAL,
														ElementOffset+ac*m_Layout[a].m_AttribPitch);
				}
			}
			else
			{
				CRY_DEBUGOUT("ERROR: InputVertexBuffer indexing %d is a NULL-ptr\n",m_Layout[a].m_InputSlot); 
			}
		}
		else
		{
			CRY_DEBUGOUT("ERROR: indexing InputVertexBuffer #%d, but set %d InputVertexBuffers\n",m_Layout[a].m_InputSlot,BufferCount); 
		}
	}
}

void CCryDXPSInputLayout::SetUpdateInstance(const uint8** pVBufferArray,uint32 BufferCount,uint32* pStride,uint32 IndexCount,uint32 Instance)
{
	using namespace cell::Gcm;
	DXPS_PROFILE_FRAME(DXPS_SetVertexLayoutInstance);

	for(uint32 a=0;a<m_Count;a++)
	{
		uint32 ElementOffset;
		if(m_Layout[a].m_InputSlot<BufferCount)
		{
			if(!m_Layout[a].m_Frequency)
				continue;

			const uint8* pVBuffer	=	pVBufferArray[m_Layout[a].m_InputSlot];
			if(pVBuffer)//just updated instancing data
			{
/*				cellGcmAddressToOffset(&pVBuffer[m_Layout[a].m_ByteOffset+instance*pStride[m_Layout[a].m_InputSlot]],&ElementOffset);
				GCM_CHECK_CMDBUFFER
				cellGcmSetVertexDataArray(
													m_Layout[a].m_InputOffset,
													m_Layout[a].m_Frequency*IndexCount,
													pStride[m_Layout[a].m_InputSlot],
													m_Layout[a].m_Size,
													m_Layout[a].m_Type,
													CELL_GCM_LOCATION_LOCAL,
													ElementOffset);*/
	//			cellGcmAddressToOffset(&pVBuffer[m_Layout[a].m_ByteOffset],&ElementOffset);
				cellGcmAddressToOffset(&pVBuffer[m_Layout[a].m_ByteOffset+Instance*pStride[m_Layout[a].m_InputSlot]],&ElementOffset);
				for(uint32 ac=0;ac<m_Layout[a].m_AttribCount;ac++)
				{
					cellGcmSetVertexDataArray(
														m_Layout[a].m_InputOffset+ac,
														m_Layout[a].m_Frequency*IndexCount,
														pStride[m_Layout[a].m_InputSlot],
														m_Layout[a].m_Size,
														m_Layout[a].m_Type,
														CELL_GCM_LOCATION_LOCAL,
														ElementOffset+ac*m_Layout[a].m_AttribPitch);
				}
			}
			else
			{
				CRY_DEBUGOUT("ERROR: InputVertexBuffer indexing %d is a NULL-ptr\n",m_Layout[a].m_InputSlot); 
			}
		}
		else
		{
			CRY_DEBUGOUT("ERROR: indexing InputVertexBuffer #%d, but set %d InputVertexBuffers\n",m_Layout[a].m_InputSlot,BufferCount); 
		}
	}
}


