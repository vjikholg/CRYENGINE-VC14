#include "../StdTypes.hpp"
#include "../Error.hpp"
#include <vector>
#include "DXPSReflection.hpp"
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <algorithm>
#include <assert.h>

/*
enum EDXPSREF_SEEK
{
	ERS_CBUFFER,
	ERS_OPEN,
	ERS_CLOSE,
};

const char CB[]="cbuffer";
const char OP[]="{";
const char CL[]="};";
const char PO[]="packoffset";

void CDXPSReflection::PrepareCBuffers(std::vector<uint8_t>&	rInOut)
{
	EDXPSREF_SEEK Seek=ERS_CBUFFER;
	for(size_t	a=0,Size=std::max(rInOut.size(),sizeof(CB)-1)-(sizeof(CB)-1);a<Size;a++)
	{
		char* pStr	=	reinterpret_cast<char*>(&rInOut[a]);
		if(strncmp(pStr,PO,sizeof(PO)-1)==0)
		{
			pStr[0]='r';
			pStr[1]='e';
			pStr[2]='g';
			pStr[3]='i';
			pStr[4]='s';
			pStr[5]='t';
			pStr[6]='e';
			pStr[7]='r';
			pStr[8]=' ';
			pStr[9]=' ';
		}
		else
		switch(Seek)
		{
			case ERS_CBUFFER:
				if(strncmp(pStr,CB,sizeof(CB)-1)==0)
				{
					pStr[0]=pStr[1]='/';
					Seek=ERS_OPEN;
				}
			break;
			case ERS_OPEN:
				if(strncmp(pStr,OP,sizeof(OP)-1)==0)
				{
					pStr[0]=' ';
					Seek=ERS_CLOSE;
				}
			break;
			case ERS_CLOSE:
				if(strncmp(pStr,CL,sizeof(CL)-1)==0)
				{
					pStr[0]=pStr[1]=' ';
					Seek=ERS_CBUFFER;
				}
			break;
		}
	}
}
*/

int32_t CDXPSReflection::ScanIndex(const std::string& rIdx,size_t Start)
{
	int32_t	Idx=0;
	const char* pStr=rIdx.c_str();
	size_t a=Start,Size=rIdx.size();
	for(;a<Size && (pStr[a]<'0' ||	pStr[a]>'9');a++);
	for(;a<Size && pStr[a]>='0' &&	pStr[a]<='9';a++)
	{
		if(pStr[a]>='0' &&	pStr[a]<='9')
			Idx	=	Idx*10+(rIdx.c_str()[a]-'0');
	}
	return Idx;
}

int32_t CDXPSReflection::ScanHex(const std::string& rIdx,size_t Start)
{
	int32_t	Idx=0;
	const char* pStr=rIdx.c_str();
	for(size_t a=Start,Size=rIdx.size();a<Size && (Idx==0 || (pStr[a]=='x') || 
																												(pStr[a]>='0' &&	pStr[a]<='9') ||
																												(pStr[a]>='a' &&	pStr[a]<='f'));a++)
	{
		if(pStr[a]>='0' &&	pStr[a]<='9')
			Idx	=	Idx*0x10+(pStr[a]-'0');
		else
		if(pStr[a]>='a' &&	pStr[a]<='f')
			Idx	=	Idx*0x10+(pStr[a]-'a'+0xa);
		else
		if(pStr[a]=='x')
			Idx=0;
	}
	return Idx;
}

R_CG_RESOURCE CDXPSReflection::IdentifyResource(const std::string& rType)
{
	if(rType.size()==R_CG_CONST_UNDEFINED)
		return R_CG_CONST_UNDEFINED;
	else
	if(rType=="ATTR0")
			return R_CG_ATTR0;
	else
	if(rType=="ATTR1")
			return R_CG_ATTR1;
	else
	if(rType=="ATTR2")
			return R_CG_ATTR2;
	else
	if(rType=="ATTR3")
			return R_CG_ATTR3;
	else
	if(rType=="ATTR4")
			return R_CG_ATTR4;
	else
	if(rType=="ATTR5")
			return R_CG_ATTR5;
	else
	if(rType=="ATTR6")
			return R_CG_ATTR6;
	else
	if(rType=="ATTR7")
			return R_CG_ATTR7;
	else
	if(rType=="ATTR8")
			return R_CG_ATTR8;
	else
	if(rType=="ATTR9")
			return R_CG_ATTR9;
	else
	if(rType=="ATTR10")
			return R_CG_ATTR10;
	else
	if(rType=="ATTR11")
			return R_CG_ATTR11;
	else
	if(rType=="ATTR12")
			return R_CG_ATTR12;
	else
	if(rType=="ATTR13")
			return R_CG_ATTR14;
	else
	if(rType=="ATTR14")
			return R_CG_ATTR14;
	else
	if(rType=="ATTR15")
			return R_CG_ATTR15;
	else
	if(rType=="TEXUNIT0")
			return R_CG_SAMPLER0;
	else
	if(rType=="TEXUNIT1")
			return R_CG_SAMPLER1;
	else
	if(rType=="TEXUNIT2")
			return R_CG_SAMPLER2;
	else
	if(rType=="TEXUNIT3")
			return R_CG_SAMPLER3;
	else
	if(rType=="TEXUNIT4")
			return R_CG_SAMPLER4;
	else
	if(rType=="TEXUNIT5")
			return R_CG_SAMPLER5;
	else
	if(rType=="TEXUNIT6")
			return R_CG_SAMPLER6;
	else
	if(rType=="TEXUNIT7")
			return R_CG_SAMPLER7;
	else
	if(rType=="TEXUNIT8")
			return R_CG_SAMPLER8;
	else
	if(rType=="TEXUNIT9")
			return R_CG_SAMPLER9;
	else
	if(rType=="TEXUNIT10")
			return R_CG_SAMPLER10;
	else
	if(rType=="TEXUNIT11")
			return R_CG_SAMPLER11;
	else
	if(rType=="TEXUNIT12")
			return R_CG_SAMPLER12;
	else
	if(rType=="TEXUNIT13")
			return R_CG_SAMPLER13;
	else
	if(rType=="TEXUNIT14")
			return R_CG_SAMPLER15;
	else
	if(rType=="TEXUNIT15")
			return R_CG_SAMPLER15;
	else
	if(rType.c_str()[0]=='C' && rType.c_str()[1]=='[')
	{
		const int32_t CIdx	=	ScanIndex(rType);
		return (R_CG_RESOURCE)(R_CG_C0+CIdx);
	}

	return R_CG_CONST_UNDEFINED;
}

tdTypeSize CDXPSReflection::IdentifyType(const std::string& rType)
{
	if(rType=="half")
		return tdTypeSize(R_CG_HALF,1);
	else
	if(rType=="float")
			return tdTypeSize(R_CG_FLOAT,1);
	else
	if(rType=="bool")
			return tdTypeSize(R_CG_BOOL,1);
	else
	if(rType=="bool2")
			return tdTypeSize(R_CG_BOOL2,1);
	else
	if(rType=="bool3")
			return tdTypeSize(R_CG_BOOL3,1);
	else
	if(rType=="bool4")
			return tdTypeSize(R_CG_BOOL4,1);
	else
	if(rType=="half2")
			return tdTypeSize(R_CG_HALF2,1);
	else
	if(rType=="half3")
			return tdTypeSize(R_CG_HALF3,1);
	else
	if(rType=="half4")
			return tdTypeSize(R_CG_HALF4,1);
	else
	if(rType=="float2")
			return tdTypeSize(R_CG_FLOAT2,1);
	else
	if(rType=="float3")
			return tdTypeSize(R_CG_FLOAT3,1);
	else
	if(rType=="float4")
			return tdTypeSize(R_CG_FLOAT4,1);
	else
	if(rType=="bool2x2")
			return tdTypeSize(R_CG_BOOL2x2,2);
	else
	if(rType=="bool3x2")
			return tdTypeSize(R_CG_BOOL3x2,3);
	else
	if(rType=="bool4x2")
			return tdTypeSize(R_CG_BOOL4x2,4);
	else
	if(rType=="bool2x3")
			return tdTypeSize(R_CG_BOOL2x3,2);
	else
	if(rType=="bool3x3")
			return tdTypeSize(R_CG_BOOL3x3,3);
	else
	if(rType=="bool4x3")
			return tdTypeSize(R_CG_BOOL4x3,4);
	else
	if(rType=="bool2x4")
			return tdTypeSize(R_CG_BOOL2x4,2);
	else
	if(rType=="bool3x4")
			return tdTypeSize(R_CG_BOOL3x4,3);
	else
	if(rType=="bool4x4")
			return tdTypeSize(R_CG_BOOL4x4,4);
	else
	if(rType=="half2x2")
			return tdTypeSize(R_CG_HALF2x2,2);
	else
	if(rType=="half3x2")
			return tdTypeSize(R_CG_HALF3x2,3);
	else
	if(rType=="half4x2")
			return tdTypeSize(R_CG_HALF4x2,4);
	else
	if(rType=="half2x3")
			return tdTypeSize(R_CG_HALF2x3,2);
	else
	if(rType=="half3x3")
			return tdTypeSize(R_CG_HALF3x3,3);
	else
	if(rType=="half4x3")
			return tdTypeSize(R_CG_HALF4x3,4);
	else
	if(rType=="half2x4")
			return tdTypeSize(R_CG_HALF2x4,2);
	else
	if(rType=="half3x4")
			return tdTypeSize(R_CG_HALF3x4,3);
	else
	if(rType=="half4x4")
			return tdTypeSize(R_CG_HALF4x4,4);
	else
	if(rType=="float2x2")
			return tdTypeSize(R_CG_FLOAT2x2,2);
	else
	if(rType=="float3x2")
			return tdTypeSize(R_CG_FLOAT3x2,3);
	else
	if(rType=="float4x2")
			return tdTypeSize(R_CG_FLOAT4x2,4);
	else
	if(rType=="float2x3")
			return tdTypeSize(R_CG_FLOAT2x3,2);
	else
	if(rType=="float3x3")
			return tdTypeSize(R_CG_FLOAT3x3,3);
	else
	if(rType=="float4x3")
			return tdTypeSize(R_CG_FLOAT4x3,4);
	else
	if(rType=="float2x4")
			return tdTypeSize(R_CG_FLOAT2x4,2);
	else
	if(rType=="float3x4")
			return tdTypeSize(R_CG_FLOAT3x4,3);
	else
	if(rType=="float4x4")
			return tdTypeSize(R_CG_FLOAT4x4,4);
	else
	if(rType=="sampler1D")
			return tdTypeSize(R_CG_SAMPLER1D,1);
	else
	if(rType=="sampler2D")
			return tdTypeSize(R_CG_SAMPLER2D,1);
	else
	if(rType=="sampler3D")
			return tdTypeSize(R_CG_SAMPLER3D,1);
	else
	if(rType=="samplerRECT")
			return tdTypeSize(R_CG_SAMPLERRECT,1);
	else
	if(rType=="samplerCube")
			return tdTypeSize(R_CG_SAMPLERCUBE,1);

	DXPS_LOG(std::string("unknown input-type: ")+rType);

	return tdTypeSize(R_CG_INVALIDE,0);
}

bool CDXPSReflection::AddVSConst(const std::string& rToks)
{
	tdEntryVec ConstToks;
	CDXPSHelper::Tokenize(ConstToks,rToks," ");
	const size_t Count=ConstToks.size();
//	OutputDebugString(rToks.c_str());
	if(Count!=6)
		return false;

	SVSConst Const;
	Const.m_Index		=	ScanIndex(ConstToks[1]);
	Const.m_Value[0]=	ScanHex(ConstToks[2]);
	Const.m_Value[1]=	ScanHex(ConstToks[3]);
	Const.m_Value[2]=	ScanHex(ConstToks[4]);
	Const.m_Value[3]=	ScanHex(ConstToks[5]);
	m_VSConsts.push_back(Const);

	return true;
}


bool CDXPSReflection::AddParam(const tdEntryVec& rToks)
{
/*	for(uint32_t a=0,Size=rToks.size();a<Size;a++)
	{
		OutputDebugString(rToks[a].c_str());
		OutputDebugString(":");
	}
	OutputDebugString("\n");
*/
	const int32_t	Idx	=	ScanIndex(rToks[0]);
	//InputParameter?
	if(Idx<0)
		return false;

	const	tdTypeSize	Type	=	IdentifyType(rToks[1]);
	if(Type.first==R_CG_INVALIDE)
		return false;

	const std::string Name	=	rToks[2];
	if(Name=="")
		return false;
#ifdef _DEBUG
	if(Name=="IN.InstMatrix")
	{
		int a=0;
	}
#endif

	const std::string Semantic	=	rToks[3];
//	if(Semantic=="")
//		return false;

	const R_CG_RESOURCE	Resource	=	IdentifyResource(rToks[4]);
	if(Resource==R_CG_CONST_UNDEFINED && m_ShaderType==EST_VERTEX)
		return false;

	m_InputVec.resize(m_InputVec.size()+1);
	SDXPSInputParam& rIParam	=	m_InputVec[m_InputVec.size()-1];
	rIParam.m_Index			=	Idx;
	rIParam.m_Type			=	Type.first;
	rIParam.m_Size			=	Type.second;
	if(Resource>=R_CG_ATTR0 && Resource<=R_CG_ATTR15)
		rIParam.m_Resource	=	(R_CG_RESOURCE)(Resource	-	rIParam.m_Size+1);
	else
		rIParam.m_Resource	=	Resource;
	rIParam.m_Name			=	Name;
	rIParam.m_Semantic	=	Semantic;


	if(m_ShaderType==EST_PIXEL && rToks.size()==7)
	{
			std::string Out;
			tdEntryVec OffsetToks;
			CDXPSHelper::Remove(Out,rToks[6],' ');
			CDXPSHelper::Tokenize(OffsetToks,Out,",");
			for(size_t a=0,Size=OffsetToks.size();a<Size;a++)
			{
				uint32_t C=ScanIndex(OffsetToks[a]);
				rIParam.m_Offsets.push_back(tdPatchOffsetPair(0,C));
			}
	}

//	const uint32_t a=rToks.size();

	return true;
}

bool CDXPSReflection::ParseInputParams()
{
	tdEntryVec VerToks;
	for(size_t	a=0,Size=m_DisassemblyTokens.size();a<Size;a++)
	{
		VerToks.clear();
		//skip line if the first two chars dont match typical param lines
		if(*m_DisassemblyTokens[a].c_str()=='#' && m_DisassemblyTokens[a].c_str()[1]!=' ')
		{
			std::string Out;
//			OutputDebugString(m_DisassemblyTokens[a].c_str());
//			OutputDebugString("\n");
			CDXPSHelper::Remove(Out,m_DisassemblyTokens[a],' ');
//			OutputDebugString(m_DisassemblyTokens[a].c_str());
//			OutputDebugString("\n");
//			OutputDebugString(Out.c_str());
//			OutputDebugString("\n");
			CDXPSHelper::Tokenize(VerToks,Out,":");
			if(VerToks.size()>=5)
				AddParam(VerToks);
			else//constant? just in case it's a vertexshader, pixelshaders already have hardcoded constants
			if(m_ShaderType==EST_VERTEX &&	VerToks.size()==1 &&	VerToks[0].size()>=sizeof("#constC[")-1 && 
																			strncmp(VerToks[0].c_str(),"#constC[",sizeof("#constC[")-1)==0)
				AddVSConst(m_DisassemblyTokens[a]);
		}
	}
	return true;
}

bool CDXPSReflection::IdentifyShadertype()
{
	tdEntryVec VerToks;
	CDXPSHelper::Tokenize(VerToks,m_DisassemblyTokens[0]," ");
	if(VerToks.size()!=3)
	{
		if(VerToks.size()>3)
		{
			DXPS_LOG("tokenized versionstring has more than 3 tokens");
		}
		else
		{
			DXPS_LOG("tokenized versionstring has less than 3 tokens");
		}
		return false;
	}
	if(VerToks[2]=="sce_vp_rsx")
		m_ShaderType	=	EST_VERTEX;
	else
	if(VerToks[2]=="sce_fp_rsx")
		m_ShaderType	=	EST_PIXEL;
	else
	{
		DXPS_LOG(std::string("unknown shader profile token: ")+VerToks[2]);
		return false;
	}
	return true;
}

bool CDXPSReflection::ParseAssembly(const std::string& rShaderDisassembled)
{
	std::vector<uint8_t> Vec;

	CDXPSHelper::FromFile(rShaderDisassembled,Vec);
	const std::string DisAssembly(reinterpret_cast<char*>(&Vec[0]),Vec.size());
	CDXPSHelper::Tokenize(m_DisassemblyTokens,DisAssembly,"\n");
#ifdef _DEBUG
	for(uint32_t a=0;a<m_DisassemblyTokens.size();a++)
	{
		OutputDebugString(m_DisassemblyTokens[a].c_str());
		OutputDebugString("\n");
	}
#endif

//	CDXPSHelper::Splitizer(TokenList,Tokens,"=");
	return true;
}

class IsVSConstUsed: public std::unary_function<SDXPSInputParam, bool> 
{
	const tdIPVec&			m_InputVec;
	const tdVSConstVec&	m_VSConsts;
public:
		IsVSConstUsed(const tdIPVec& rInputVec,const tdVSConstVec&	rVSConsts):m_InputVec(rInputVec),m_VSConsts(rVSConsts){}
    bool operator()(const SDXPSInputParam& rParam) const 
    { 
			//remove all default cg constants
			for(size_t a=0,Size=m_VSConsts.size();a<Size;a++)
				if(m_VSConsts[a].m_Index==rParam.m_Resource)
					return true;
			for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
				if(&m_InputVec[a]!=&rParam && m_InputVec[a].m_Name==rParam.m_Name &&
					(m_InputVec[a].m_Size>rParam.m_Size || (m_InputVec[a].m_Size==rParam.m_Size && m_InputVec[a].m_Resource<rParam.m_Resource)))
					return true;
			return false;
		}
};

class IsPSConstUsed: public std::unary_function<SDXPSInputParam, bool> 
{
public:
    bool operator()(const SDXPSInputParam& rParam) const 
    { 
			return	rParam.m_Offsets.size()==0 &&
							!(rParam.m_Type>=R_CG_SAMPLER1D && rParam.m_Type<=R_CG_SAMPLERCUBE && rParam.m_Resource!=R_CG_CONST_UNDEFINED) ||
							(rParam.m_Semantic.size()>2 && rParam.m_Semantic.c_str()[0]=='i' && rParam.m_Semantic.c_str()[1]=='n' && rParam.m_Semantic.c_str()[2]=='.');
		}
};

bool CDXPSReflection::RemoveUnusedParams()
{
	if(m_ShaderType==EST_VERTEX)
	{
		m_InputVec.erase(std::remove_if(m_InputVec.begin(),m_InputVec.end(),IsVSConstUsed(m_InputVec,m_VSConsts)),m_InputVec.end());
		return true;
	}
	else
	if(m_ShaderType==EST_PIXEL)
	{
		m_InputVec.erase(std::remove_if(m_InputVec.begin(),m_InputVec.end(),IsPSConstUsed()),m_InputVec.end());
		return true;
	}
	return false;
}

bool CDXPSReflection::ParamFusion()
{
	//remove indices like [0],[1] etc.
	for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
	{
		std::string::size_type Pt;
		if((Pt=m_InputVec[a].m_Name.find("[",0))!=std::string::npos)
		{
			const std::string StrIndex	=	m_InputVec[a].m_Name.substr(Pt,m_InputVec[a].m_Name.size());
			const uint32_t	Index	=	ScanIndex(StrIndex);
			m_InputVec[a].m_Name=m_InputVec[a].m_Name.substr(0,Pt);
			if(Index+1>m_InputVec[a].m_Size)
				m_InputVec[a].m_Size	=	Index+1;
			for(size_t b=0;b<m_InputVec[a].m_Offsets.size();b++)
				m_InputVec[a].m_Offsets[b].first=Index;
			if(m_InputVec[a].m_Resource>=R_CG_ATTR0 &&	m_InputVec[a].m_Resource<=R_CG_ATTR15)
				m_InputVec[a].m_Resource=(R_CG_RESOURCE)(m_InputVec[a].m_Resource-m_InputVec[a].m_Size+1);

		}
	}

	//on PS patchoffset-vec fusion
	if(m_ShaderType==EST_PIXEL)
	{
		for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
			for(size_t b=a+1;b<Size;b++)
				if(m_InputVec[a].m_Name==m_InputVec[b].m_Name)
				{
					for(size_t c=0;c<m_InputVec[b].m_Offsets.size();c++)
						m_InputVec[a].m_Offsets.push_back(m_InputVec[b].m_Offsets[c]);
					m_InputVec[b].m_Offsets.clear();
				}
	}
	return true;
}

bool CDXPSReflection::IdentifyConstBuffers(const std::vector<uint8_t>& rProgram)
{
	const std::string Program((const char*)&rProgram[0],rProgram.size());
	for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
	{
		std::string::size_type Pt;
		if(m_InputVec[a].m_Resource<=R_CG_C468 || m_InputVec[a].m_Resource==R_CG_CONST_UNDEFINED)
		{
#ifdef _DEBUG
			const std::string Name=m_InputVec[a].m_Name;
#endif
			if(	(Pt = Program.find(m_InputVec[a].m_Name,0)) != std::string::npos &&
					(Pt = Program.find("//CBUFFER(",Pt)) != std::string::npos)
			{
				uint32_t Idx	=	ScanIndex(Program,Pt);
				m_InputVec[a].m_CBuffer	=	Idx;
			}
			else
				return false;
		}
	}
	return true;
}

bool CDXPSReflection::AssigneInvalideConsts()
{
	for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
	{
		if(m_InputVec[a].m_Resource==R_CG_CONST_UNDEFINED && m_InputVec[a].m_Type<R_CG_SAMPLER1D)
		{
			//find an empty slot, start at const32 as the renderer reservs lower const hardcoded
			for(size_t b=32;b<468;b++)
			{
				bool Found=true;
				for(size_t c=0;c<Size;c++)
					if(	m_InputVec[a].m_CBuffer==m_InputVec[c].m_CBuffer &&
							m_InputVec[c].m_Resource>=(int)b && 
							m_InputVec[c].m_Resource<(int)(b+m_InputVec[c].m_Size))
					{
						Found=false;
						break;
					}
				if(Found)
				{
					m_InputVec[a].m_Resource=(R_CG_RESOURCE)b;
					break;
				}
			}
			if(m_InputVec[a].m_Resource==R_CG_CONST_UNDEFINED)
			{
				DXPS_LOG(std::string("Could not assing constindex to pixelshader constvar:")+m_InputVec[a].m_Name);
			}
		}
	}
	return true;
}

uint32_t CDXPSReflection::AddName(const std::string& rName)
{
	size_t Idx=m_NameTable.size();
	m_NameTable.resize(m_NameTable.size()+rName.size()+1);
	memcpy(&m_NameTable[Idx],rName.c_str(),rName.size());
	m_NameTable[m_NameTable.size()-1]	=0;
	return static_cast<uint32_t>(Idx);
}

bool CDXPSReflection::ReflectVertexAttributes(tdVertexAttributeReflection&	rVAR)
{
	rVAR.clear();
	for(uint32_t Attr=R_CG_ATTR0;Attr<=R_CG_ATTR15;Attr++)
	{
		for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
		{
			if(m_InputVec[a].m_Resource==Attr)
			{
				SRefVertexAttribute RAttr;

				std::string Semantic	=	m_InputVec[a].m_Semantic;
				if(Semantic.size()>=3 && Semantic.c_str()[0]=='i' && Semantic.c_str()[1]=='n' && Semantic.c_str()[2]=='.')
					Semantic	=	std::string(&Semantic.c_str()[3],Semantic.size()-3);

				RAttr.m_SemanticNameIndex	=	AddName(Semantic);	//32Bit
				RAttr.m_InputOffset	=	Attr-R_CG_ATTR0;			//32Bit
				RAttr.m_Size				=	m_InputVec[a].m_Size;			//32Bit

				CDXPSHelper::EndianSwizzleU32(RAttr.m_InputOffset);
				CDXPSHelper::EndianSwizzleU32(RAttr.m_SemanticNameIndex);
				CDXPSHelper::EndianSwizzleU32(RAttr.m_Size);

				rVAR.push_back(RAttr);
			}
		}
	}
	return true;
}

bool CDXPSReflection::ReflectSamplers(tdSamplerReflection&	rSR)
{
	rSR.clear();
	for(uint32_t Sampler=R_CG_SAMPLER0;Sampler<=R_CG_SAMPLER15;Sampler++)
	{
		for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
		{
			if(m_InputVec[a].m_Resource==Sampler)
			{
				SRefSampler RSampler;

				RSampler.m_SamplerIndex	=	Sampler-R_CG_SAMPLER0;//32bit
				RSampler.m_NameIndex		=	AddName(m_InputVec[a].m_Name);//32bit

				CDXPSHelper::EndianSwizzleU32(RSampler.m_SamplerIndex);
				CDXPSHelper::EndianSwizzleU32(RSampler.m_NameIndex);

				rSR.push_back(RSampler);
			}
		}
	}
	return true;
}

bool CDXPSReflection::ReflectConstants(tdConstantReflection&	rCR)
{
	rCR.clear();
	for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
	{
		if(m_InputVec[a].m_Resource<R_CG_C468)
		{
			SRefConstant RConst;

			RConst.m_NameIndex			=	AddName(m_InputVec[a].m_Name);//32bit
			RConst.m_Type						=	m_InputVec[a].m_Type;//can be 8bit
			RConst.m_Size						=	m_InputVec[a].m_Size;//can be 8bit
			RConst.m_ConstRegister	=	m_InputVec[a].m_Resource;//can be 16bit
			RConst.m_ConstSlot			=	m_InputVec[a].m_CBuffer;

			CDXPSHelper::EndianSwizzleU32(RConst.m_NameIndex);
			CDXPSHelper::EndianSwizzleU32(RConst.m_Type);
			CDXPSHelper::EndianSwizzleU32(RConst.m_Size);
			CDXPSHelper::EndianSwizzleU32(RConst.m_ConstRegister);
			CDXPSHelper::EndianSwizzleU32(RConst.m_ConstSlot);

			rCR.push_back(RConst);
		}
	}
	return true;
}

bool CDXPSReflection::ReflectPatchOffsets(tdPatchReflection&	rPR)
{
	rPR.clear();
	for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
	{
		if(m_InputVec[a].m_Resource<R_CG_C468)
		{
			for(size_t b=0,SizeB=m_InputVec[a].m_Offsets.size();b<SizeB;b++)
			{
				SRefPatch RPatch;

				RPatch.m_ConstSlot	=	m_InputVec[a].m_CBuffer;//16bit
				RPatch.m_ConstReg		=	m_InputVec[a].m_Resource+m_InputVec[a].m_Offsets[b].first;//16bit
				RPatch.m_Offset			=	m_InputVec[a].m_Offsets[b].second;//32bit

				CDXPSHelper::EndianSwizzleU16(RPatch.m_ConstSlot);
				CDXPSHelper::EndianSwizzleU16(RPatch.m_ConstReg);
				CDXPSHelper::EndianSwizzleU32(RPatch.m_Offset);

				rPR.push_back(RPatch);
			}
		}
	}
	return true;
}

bool CDXPSReflection::Reflect(const std::string&					rShaderCache,
															const std::string& 					rShaderStripped,
															const std::string& 					rShaderDisassembled,
															const std::vector<uint8_t>& rProgram)
{
	m_InputVec.clear();

	if(!ParseAssembly(rShaderDisassembled))
		return false;

	if(!IdentifyShadertype())
		return false;

	if(!ParseInputParams())
		return false;

	ParamFusion();

	RemoveUnusedParams();

	IdentifyConstBuffers(rProgram);

	if(m_ShaderType==EST_PIXEL)
		AssigneInvalideConsts();


#ifdef _DEBUG
	if(m_ShaderType==EST_VERTEX)
		OutputDebugString("Identified Vertexshader\n");
	else
	if(m_ShaderType==EST_PIXEL)
		OutputDebugString("Identified Pixelshader\n");
	else
		OutputDebugString("could not identify shadertype\n");

	if(m_VSConsts.size())
		OutputDebugString("VSConstants\n");
	for(size_t a=0,Size=m_VSConsts.size();a<Size;a++)
	{
		char Text[1024];
		sprintf(Text,"C%d={%f,%f,%f,%f};\n",m_VSConsts[a].m_Index,*(float*)&m_VSConsts[a].m_Value[0],
																															*(float*)&m_VSConsts[a].m_Value[1],
																															*(float*)&m_VSConsts[a].m_Value[2],
																															*(float*)&m_VSConsts[a].m_Value[3]);
		OutputDebugString(Text);
	}
	if(m_InputVec.size())
		OutputDebugString("InputParams\n");
	for(size_t a=0,Size=m_InputVec.size();a<Size;a++)
	{
		char Text[1024];
		sprintf(Text,"Idx:%d;Name=\"%s\";Semantic=\"%s\";Type=%d;Size:%d;CBuffer:%d;",
																											m_InputVec[a].m_Index,
																											m_InputVec[a].m_Name.c_str(),
																											m_InputVec[a].m_Semantic!=""?m_InputVec[a].m_Semantic.c_str():"No",
																											m_InputVec[a].m_Type,m_InputVec[a].m_Size,m_InputVec[a].m_CBuffer);
		OutputDebugString(Text);
		if(m_InputVec[a].m_Resource<=R_CG_C468)
		{
			sprintf(Text,"C[%d];",m_InputVec[a].m_Resource);
			OutputDebugString(Text);
			if(m_InputVec[a].m_CBuffer>16)
			{
				int g=0;
			}
		}
		else
		switch(m_InputVec[a].m_Resource)
		{
			case R_CG_ATTR0:OutputDebugString("ATTR0;");break;
			case R_CG_ATTR1:OutputDebugString("ATTR1;");break;
			case R_CG_ATTR2:OutputDebugString("ATTR2;");break;
			case R_CG_ATTR3:OutputDebugString("ATTR3;");break;
			case R_CG_ATTR4:OutputDebugString("ATTR4;");break;
			case R_CG_ATTR5:OutputDebugString("ATTR5;");break;
			case R_CG_ATTR6:OutputDebugString("ATTR6;");break;
			case R_CG_ATTR7:OutputDebugString("ATTR7;");break;
			case R_CG_ATTR8:OutputDebugString("ATTR8;");break;
			case R_CG_ATTR9:OutputDebugString("ATTR9;");break;
			case R_CG_ATTR10:OutputDebugString("ATTR10;");break;
			case R_CG_ATTR11:OutputDebugString("ATTR11;");break;
			case R_CG_ATTR12:OutputDebugString("ATTR12;");break;
			case R_CG_ATTR13:OutputDebugString("ATTR13;");break;
			case R_CG_ATTR14:OutputDebugString("ATTR14;");break;
			case R_CG_ATTR15:OutputDebugString("ATTR15;");break;
			case R_CG_SAMPLER0:OutputDebugString("Sampler0;");break;
			case R_CG_SAMPLER1:OutputDebugString("Sampler1;");break;
			case R_CG_SAMPLER2:OutputDebugString("Sampler2;");break;
			case R_CG_SAMPLER3:OutputDebugString("Sampler3;");break;
			case R_CG_SAMPLER4:OutputDebugString("Sampler4;");break;
			case R_CG_SAMPLER5:OutputDebugString("Sampler5;");break;
			case R_CG_SAMPLER6:OutputDebugString("Sampler6;");break;
			case R_CG_SAMPLER7:OutputDebugString("Sampler7;");break;
			default:OutputDebugString("Invalide Attribute;");
		}
		if(m_InputVec[a].m_Offsets.size())
			OutputDebugString(";Offsets:");
		for(size_t b=0,Size=m_InputVec[a].m_Offsets.size();b<Size;b++)
		{
			sprintf(Text,"[%d|%d],",m_InputVec[a].m_Offsets[b].first,m_InputVec[a].m_Offsets[b].second);
			OutputDebugString(Text);
		}
		OutputDebugString("\n");
	}

	
#endif

	tdVertexAttributeReflection	VAR;
	tdSamplerReflection	SR;
	tdConstantReflection	CR;
	tdPatchReflection	PR;

	m_NameTable.clear();

	if(m_ShaderType==EST_VERTEX)
		ReflectVertexAttributes(VAR);
	if(m_ShaderType==EST_PIXEL)
	{
		ReflectSamplers(SR);
		ReflectPatchOffsets(PR);
	}
	ReflectConstants(CR);

	std::vector<uint8_t> StrippedShader;
	CDXPSHelper::FromFile(rShaderStripped,StrippedShader);


	std::vector<uint8_t> ShaderCache;

	SShaderDesc ShaderDesc;

	const uint8_t*	pShaderDesc			=	reinterpret_cast<const uint8_t*>(&ShaderDesc);
	const uint8_t*	pVAR						=	reinterpret_cast<const uint8_t*>(&VAR[0]);
	const uint8_t*	pSR							=	reinterpret_cast<const uint8_t*>(&SR[0]);
	const uint8_t*	pCR							=	reinterpret_cast<const uint8_t*>(&CR[0]);
	const uint8_t*	pVSC						=	reinterpret_cast<const uint8_t*>(&m_VSConsts[0]);
	const uint8_t*	pPR							=	reinterpret_cast<const uint8_t*>(&PR[0]);
	const uint8_t*	pStrippedShader	=	reinterpret_cast<const uint8_t*>(&StrippedShader[0]);
	const uint8_t*	pNameTable			=	reinterpret_cast<const uint8_t*>(&m_NameTable[0]);

	//swizzle VSConsts now, cannot do it before as they're needed to remove internal-constants of the CR
	for(size_t a=0,Size=m_VSConsts.size();a<Size;a++)
	{
		CDXPSHelper::EndianSwizzleU32(m_VSConsts[a].m_Index);
		CDXPSHelper::EndianSwizzleU32(m_VSConsts[a].m_Value[0]);
		CDXPSHelper::EndianSwizzleU32(m_VSConsts[a].m_Value[1]);
		CDXPSHelper::EndianSwizzleU32(m_VSConsts[a].m_Value[2]);
		CDXPSHelper::EndianSwizzleU32(m_VSConsts[a].m_Value[3]);
	}


	ShaderDesc.m_Version			=	DXPS_REF_SHADER_VERSION;
	ShaderDesc.m_PixelShader	=	m_ShaderType;

	ShaderDesc.m_VertexAttributeOffset	=	sizeof(SShaderDesc);
	ShaderDesc.m_VertexAttributeCount		=	(uint32_t)VAR.size();

	ShaderDesc.m_SamplerOffset	=	ShaderDesc.m_VertexAttributeOffset+VAR.size()*sizeof(SRefVertexAttribute);
	ShaderDesc.m_SamplerCount		=	(uint32_t)SR.size();

	ShaderDesc.m_ConstOffset	=	ShaderDesc.m_SamplerOffset+SR.size()*sizeof(SRefSampler);
	ShaderDesc.m_ConstCount		=	(uint32_t)CR.size();

	ShaderDesc.m_VSConstOffset	=	ShaderDesc.m_ConstOffset+CR.size()*sizeof(SRefConstant);
	ShaderDesc.m_VSConstCount		=	(uint32_t)m_VSConsts.size();

	ShaderDesc.m_PatchOffset	=	ShaderDesc.m_VSConstOffset+m_VSConsts.size()*sizeof(SVSConst);
	ShaderDesc.m_PatchCount		=	(uint32_t)PR.size();

	ShaderDesc.m_ShaderBinaryOffset	=	ShaderDesc.m_PatchOffset+PR.size()*sizeof(SRefPatch);
	ShaderDesc.m_ShaderBinarySize		=	(uint32_t)StrippedShader.size();
	ShaderDesc.m_ShaderBinaryOffset	=	(ShaderDesc.m_ShaderBinaryOffset+127)&~127;

	ShaderDesc.m_NameTableOffset	=	ShaderDesc.m_ShaderBinaryOffset+StrippedShader.size();
	ShaderDesc.m_NameTableSize		=	(uint32_t)m_NameTable.size();

	ShaderCache.resize(ShaderDesc.m_NameTableOffset+ShaderDesc.m_NameTableSize);

	memcpy(&ShaderCache[ShaderDesc.m_VertexAttributeOffset],pVAR,VAR.size()*sizeof(SRefVertexAttribute));
	memcpy(&ShaderCache[ShaderDesc.m_SamplerOffset],pSR,SR.size()*sizeof(SRefSampler));
	memcpy(&ShaderCache[ShaderDesc.m_ConstOffset],pCR,CR.size()*sizeof(SRefConstant));
	memcpy(&ShaderCache[ShaderDesc.m_VSConstOffset],pVSC,m_VSConsts.size()*sizeof(SVSConst));
	memcpy(&ShaderCache[ShaderDesc.m_PatchOffset],pPR,PR.size()*sizeof(SRefPatch));
	memcpy(&ShaderCache[ShaderDesc.m_ShaderBinaryOffset],pStrippedShader,StrippedShader.size());
	memcpy(&ShaderCache[ShaderDesc.m_NameTableOffset],pNameTable,m_NameTable.size());

	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_Version);
	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_PixelShader);

	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_VertexAttributeOffset);
	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_VertexAttributeCount);

	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_SamplerOffset);
	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_SamplerCount);

	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_ConstOffset);
	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_ConstCount);

	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_VSConstOffset);
	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_VSConstCount);

	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_PatchOffset);
	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_PatchCount);

	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_ShaderBinaryOffset);
	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_ShaderBinarySize);

	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_NameTableOffset);
	CDXPSHelper::EndianSwizzleU32(ShaderDesc.m_NameTableSize);

	memcpy(&ShaderCache[0],pShaderDesc,sizeof(SShaderDesc));

	CDXPSHelper::ToFile(rShaderCache,ShaderCache);

	return false;
}


