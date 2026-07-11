#include "RenderPCH.h"
#include "../../../Layer0/CCryDXPS.hpp"
#include "CCryDXPSShader.hpp"
#include "CCryDXPSShaderReflection.hpp"
#include "CCryDXPSShaderDesc.hpp"


#if defined(_DEBUG)
	extern "C" void *mwprivate2_memalign(size_t, size_t);
#endif


CCryDXPSShader::CCryDXPSShader(const void* pProgram,ECRYDXPSShaderType SType):
CCryDXPSResource(EDXPS_RT_SHADER),
CCryRefAndWeak<CCryDXPSShader>(this)
{
	using namespace cell::Gcm;


	const uint32 ByteSize	=	reinterpret_cast<const uint32*>(pProgram)[0];
	//PS3HACK
#if defined(_DEBUG)
	uint8* pData	=	(uint8*)mwprivate2_memalign(128,(ByteSize+127)&~127);
#else
	uint8* pData	=	(uint8*)memalign(128,(ByteSize+127)&~127);
#endif
	CRY_DEBUGOUT("Shader bytecode-size %d\n",ByteSize);
	memcpy(pData,&reinterpret_cast<const uint8*>(pProgram)[4],ByteSize);

	m_pDesc	=	reinterpret_cast<const CDXPSShaderDesc*>(pData);
	cellGcmCgInitProgram(m_pDesc->Program());

	uint32 uCodeSize;
	cellGcmCgGetUCode(m_pDesc->Program(), &m_puCode, &uCodeSize);
	CRY_DEBUGOUT("Shader ucode-size %d\n",uCodeSize);

	if(m_pDesc->PixelShader())
	{
		m_MemItemIDPixel	=	tdLayer0::Instance().Memory().Allocate(uCodeSize);
		pData	=	reinterpret_cast<uint8*>(tdLayer0::Instance().Memory().ResolveHandle(m_MemItemIDPixel));
		memcpy(pData,m_puCode,uCodeSize);
		m_puCode	=	pData;
	}

//	Reflect();

}

void CCryDXPSShader::Reflect()
{
/*	using namespace cell::Gcm;
	SCryDXPSShaderParam	Params[128];
	m_ParamCounter	=	0;
	m_pParams	=	NULL;
	if(m_ShaderType!=EDXPS_ST_PIXEL)
		return;

	for(uint32 a=0,Idx=0,Size=cellGcmCgGetCountParameter(m_Program);a<Size && m_ParamCounter<128;a++)
	{
		Params[m_ParamCounter].m_Param	=	cellGcmCgGetIndexParameter(m_Program,a);

#ifdef _DEBUG
		Params[m_ParamCounter].m_Referenced	=	cellGcmCgGetParameterReferenced(m_Program,Params[m_ParamCounter].m_Param);
		Params[m_ParamCounter].m_InOut			=	cellGcmCgGetParameterDirection(m_Program,Params[m_ParamCounter].m_Param);
		Params[m_ParamCounter].m_Varia			=	cellGcmCgGetParameterVariability(m_Program,Params[m_ParamCounter].m_Param);
		Params[m_ParamCounter].m_Name				=	cellGcmCgGetParameterName(m_Program,Params[m_ParamCounter].m_Param);
		Params[m_ParamCounter].m_Semantic		=	cellGcmCgGetParameterSemantic(m_Program,Params[m_ParamCounter].m_Param);
		Params[m_ParamCounter].m_Idx				=	cellGcmCgGetParameterResourceIndex(m_Program,Params[m_ParamCounter].m_Param);
#endif
		if(	CG_TRUE			==	cellGcmCgGetParameterReferenced(m_Program,Params[m_ParamCounter].m_Param) &&
				CG_OUT			!=	cellGcmCgGetParameterDirection(m_Program,Params[m_ParamCounter].m_Param) &&
				CG_UNIFORM	==	cellGcmCgGetParameterVariability(m_Program,Params[m_ParamCounter].m_Param))
		{
#if defined(_DEBUG)
			const char* pName	=	cellGcmCgGetParameterName(m_Program,Params[m_ParamCounter].m_Param);
#endif
/*			uint32 L		=	strlen(pName);
			if(']'!=pName[L-1])
			{
				Params[m_ParamCounter].m_Idx	=	Idx<<2;
//							cellGcmSetFragmentProgramParameter(m_Program,Params[ParamCounter].m_Param,&pConsts[Idx<<2],Offset);
				Idx++;
			}
			else
			{
				L-=2;
				int32 Digit=1;
				int32 Add=0;
				while('['!=pName[L] && L>0)
				{
					int32 N=(pName[L]-'0');
					Add+=N*Digit;
					Digit*=10;
					L--;
				}
				//index -1 cause the previous parameter-setting already increased by 1
				//cellGcmSetFragmentProgramParameter(m_Program,Param,&pConsts[(Idx-1+Add)<<2],Offset);
				Params[m_ParamCounter].m_Idx	=	(Idx-1+Add)<<2;
			}*/
	/*
			Params[m_ParamCounter].m_Idx	=	CCryDXPSShaderReflection::IndexOf(m_Program,Params[m_ParamCounter].m_Param,true);
			Params[m_ParamCounter].m_Idx<<=	2;//float4 offset in float array
			m_ParamCounter++;
		}
	}
	if(!m_ParamCounter)
		return;
	m_pParams	=	new	SCryDXPSShaderParam[m_ParamCounter];
	for(uint32 a=0;a<m_ParamCounter;a++)
		m_pParams[a]	=	Params[a];
	*/
}



void CCryDXPSShader::ReleaseResources()
{
		if(m_pDesc->PixelShader())
					tdLayer0::Instance().Memory().Free(m_MemItemIDPixel);
		free((uint8*)m_pDesc);
}

ECRYDXPSShaderType CCryDXPSShader::ShaderType()const
{
	return m_pDesc->PixelShader()?EDXPS_ST_PIXEL:EDXPS_ST_VERTEX;
}


void CCryDXPSShader::Set(const APWeakBuffer* pConstBuffer)
{
	using namespace cell::Gcm;
	if(m_pDesc->PixelShader())
	{
		DXPS_PROFILE_FRAME(DXPS_SetPixelshader);
		uint32 Offset;
		if(CELL_OK!=cellGcmAddressToOffset(m_puCode,&Offset))
		{
			CRY_DEBUGOUT("Could not translate fragment-ucode-address to offset\n");
			return;
		}

		cellGcmFinish(0);//SLOW
//		static uint32 Delay	=	1000;
//		sys_timer_usleep(Delay);

		//SLOW, SLOOOOOW, need to get it working first
		for(uint32 a=0,Size=m_pDesc->PatchCount();a<Size;a++)
		{
			const SRefPatch&	rPatch	=	m_pDesc->Patch()[a];
			if(pConstBuffer[rPatch.m_ConstSlot]->Valid())
			{
				uint16* pDst	=	reinterpret_cast<uint16*>(&reinterpret_cast<uint8*>(m_puCode)[rPatch.m_Offset]);
				const uint16* pSrc	=	reinterpret_cast<const uint16*>(&reinterpret_cast<const uint8*>(pConstBuffer[rPatch.m_ConstSlot]->RawData())[rPatch.m_ConstReg<<4]);
				pDst[0]	=	pSrc[1];
				pDst[1]	=	pSrc[0];
				pDst[2]	=	pSrc[3];
				pDst[3]	=	pSrc[2];

				pDst[4]	=	pSrc[5];
				pDst[5]	=	pSrc[4];
				pDst[6]	=	pSrc[7];
				pDst[7]	=	pSrc[6];
			}
		}

//		if(pConsts)
//			for(uint32 a=0;a<m_ParamCounter;a++)
//				cellGcmSetFragmentProgramParameter(m_Program,m_pParams[a].m_Param,&pConsts[m_pParams[a].m_Idx],Offset);
/*
#if defined(_DEBUG)
		if(!m_ParamCounter)
		{
			//debug
			if(m_ParamCounter>256)
			{
				Reflect();
			}
			for(uint32 a=0,Idx=0,Size=cellGcmCgGetCountParameter(m_Program);a<Size;a++)
			{

				CGparameter Param	=	cellGcmCgGetIndexParameter(m_Program,a);

//<debug>
				const CGbool Referenced	=	cellGcmCgGetParameterReferenced(m_Program,Param);
				const CGenum InOut			=	cellGcmCgGetParameterDirection(m_Program,Param);
				const bool In						=	(CG_OUT			!=	InOut);
				const CGenum Varia			=	cellGcmCgGetParameterVariability(m_Program,Param);
				const bool Variability	=	(CG_UNIFORM	==	Varia);

				if(!strncmp("cBitmapColorTransform",cellGcmCgGetParameterName(m_Program,Param),sizeof("cBitmapColorTransform")-1))
				{
					int a=0;
				}

				char Semantic[1024];
				const char* pName	=	cellGcmCgGetParameterName(m_Program,Param);
				strcpy(Semantic,cellGcmCgGetParameterSemantic(m_Program,Param));
//</debug>
				if(	CG_TRUE			==	cellGcmCgGetParameterReferenced(m_Program,Param) &&
						CG_OUT			!=	cellGcmCgGetParameterDirection(m_Program,Param) &&
						CG_UNIFORM	==	cellGcmCgGetParameterVariability(m_Program,Param))
				{
					const char* pName	=	cellGcmCgGetParameterName(m_Program,Param);
					uint32 L		=	strlen(pName);
					if(']'!=pName[L-1])
					{
						cellGcmSetFragmentProgramParameter(m_Program,Param,&pConsts[Idx<<2],Offset);
						Idx++;
					}
					else
					{
						L-=2;
						int32 Digit=1;
						int32 Add=0;
						while('['!=pName[L] && L>0)
						{
							int32 N=(pName[L]-'0');
							Add+=N*Digit;
							Digit*=10;
							L--;
						}
						//index -1 cause the previous parameter-setting already increased by 1
						cellGcmSetFragmentProgramParameter(m_Program,Param,&pConsts[(Idx-1+Add)<<2],Offset);
					}
				}
			}
		}
#endif

		cellGcmSetUpdateFragmentProgramParameter(Offset);*/
		cellGcmSetFragmentProgram(m_pDesc->Program(),Offset);
	}
	else
	{
		DXPS_PROFILE_FRAME(DXPS_SetVertexshader);

		for(uint32 a=0,Size=m_pDesc->ConstantCount();a<Size;a++)
		{
			const SRefConstant& rConstant	=	m_pDesc->Constant()[a];
#ifdef _DEBUG
			const char* pName = &m_pDesc->NameTable()[rConstant.m_NameIndex];
#endif
			if(pConstBuffer[rConstant.m_ConstSlot]->Valid())
				cellGcmSetVertexProgramConstants(	rConstant.m_ConstRegister,
																					rConstant.m_Size<<2,
																					&reinterpret_cast<const f32*>(pConstBuffer[rConstant.m_ConstSlot]->RawData())[rConstant.m_ConstRegister<<2]);
		}
		for(uint32 a=0,Size=m_pDesc->VSConstantCount();a<Size;a++)
		{
			const SVSConst& rConstant	=	m_pDesc->VSConstant()[a];
			cellGcmSetVertexProgramConstants(	rConstant.m_Index,
																				4,
																				reinterpret_cast<const f32*>(&rConstant.m_Value[0]));
		}
		static CGprogram LastVertexShader	=	(CGprogram)~0;
		if(LastVertexShader!=m_pDesc->Program())
		{
			cellGcmSetVertexProgram(m_pDesc->Program(),m_puCode);
			LastVertexShader=m_pDesc->Program();
		}
	}
}
