#ifndef __CRYDXPSINPUTLAYOUT__
#define __CRYDXPSINPUTLAYOUT__

#include "../CCryDXPSResource.hpp"
#include "../../../CCryDXPSAPtr.hpp"
#include "../../../Layer0/CCryDXPS.hpp"

class CCryDXPSInputLayout;
class CCryDXPSInputLayout	:	public CCryDXPSResource	,	public CCryRefAndWeak<CCryDXPSInputLayout>
{

public:
	struct SLayout
	{
	public:
		//debug not possible, out of mem
#if defined(_DEBUG)
		char													m_SemanticName[64];
#endif
		uint16												m_Frequency;
		uint8													m_ByteOffset;
		uint8													m_AttribCount;
		uint8													m_AttribPitch;
		uint8													m_InputOffset:4;
		uint8													m_InputSlot:4;
		uint8													m_Type:4;
		uint8													m_Size:4;
	};
	SLayout													m_Layout[16];
	uint32													m_Count;
	uint32													m_FrequencyDivider;
public:
	CCryDXPSInputLayout(const D3D10_INPUT_ELEMENT_DESC *pInputElementDescs,uint32 NumElements,const void *pShaderBytecodeWithInputSignature);
	void Set(const uint8** pVBufferArray,uint32 BufferCount,uint32* pStride,uint32 IndexCount);
	void SetUpdateInstance(const uint8** pVBufferArray,uint32 BufferCount,uint32* pStride,uint32 IndexCount,uint32 instance);

public:
	inline	void	ReleaseResources()
	{
	}
};

typedef CCryDXPSInputLayout ID3D10InputLayout;
typedef CCryAPtrWeakCnt<CCryDXPSInputLayout>	APWeakInputLayout;

#endif

