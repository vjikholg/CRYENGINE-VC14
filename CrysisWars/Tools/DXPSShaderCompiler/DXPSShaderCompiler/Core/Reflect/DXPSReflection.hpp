#ifndef __DXPSREFLECTION__
#define __DXPSREFLECTION__

#include "../DXPSHelper.hpp"
#include "DXPSReflectionShared.hpp"

enum ERShaderType
{
	EST_VERTEX,
	EST_PIXEL
};


typedef std::vector<SRefVertexAttribute>		tdVertexAttributeReflection;
typedef std::vector<SRefSampler>						tdSamplerReflection;
typedef std::vector<SRefConstant>						tdConstantReflection;
typedef std::vector<SRefPatch>							tdPatchReflection;
typedef std::vector<SVSConst>								tdVSConstVec;
typedef std::vector<char>										m_NameTabel;

//const array index,offset
typedef std::pair<uint32_t,uint32_t>		tdPatchOffsetPair;
typedef std::vector<tdPatchOffsetPair>	tdPatchOffsets;

struct SDXPSInputParam
{
	uint32_t				m_Index;
	uint32_t				m_Size;
	R_CG_TYPE				m_Type;
	R_CG_RESOURCE		m_Resource;
	std::string			m_Name;
	std::string			m_Semantic;
	tdPatchOffsets	m_Offsets;
	uint32_t				m_CBuffer;
};


typedef std::vector<SDXPSInputParam>	tdIPVec;
typedef std::pair<R_CG_TYPE,int>			tdTypeSize;

class CDXPSReflection
{
private:
	tdVSConstVec	m_VSConsts;
	tdEntryVec		m_DisassemblyTokens;
	ERShaderType	m_ShaderType;
	tdIPVec				m_InputVec;
	m_NameTabel		m_NameTable;

	int32_t				ScanHex(const std::string& rIdx,size_t Start=0);
	int32_t				ScanIndex(const std::string& rIdx,size_t Start=0);
	tdTypeSize		IdentifyType(const std::string& rType);
	R_CG_RESOURCE	IdentifyResource(const std::string& rType);

	uint32_t			AddName(const std::string& rName);
	bool					AddParam(const tdEntryVec& rToks);
	bool					AddVSConst(const std::string& rToks);

	bool					AssigneInvalideConsts();
	bool					IdentifyConstBuffers(const std::vector<uint8_t>& rProgram);
	bool					RemoveUnusedParams();
	bool					ParamFusion();
	bool					ParseInputParams();
	bool					IdentifyShadertype();
	bool					ParseAssembly(const std::string& rShaderDisassembled);


	bool					ReflectVertexAttributes(tdVertexAttributeReflection&	rVAR);
	bool					ReflectSamplers(tdSamplerReflection&	rSR);
	bool					ReflectConstants(tdConstantReflection&	rCR);
	bool					ReflectPatchOffsets(tdPatchReflection&	rPR);
public:
//	void				PrepareCBuffers(std::vector<uint8_t>&	rInOut);
	bool					Reflect(const std::string& rShaderCache,
												const std::string& rShaderStripped,
												const std::string& rShaderDisassembled,
												const std::vector<uint8_t>& rProgram);
};

#endif
