#ifndef __CRYDXPSBUFFER__
#define __CRYDXPSBUFFER__

#include "../CCryDXPSResource.hpp"
#include "../../../Layer0/CCryDXPS.hpp"

class CCryDXPSBuffer;
class CCryDXPSBuffer		:		public CCryDXPSResource	,	public CCryRefAndWeak<CCryDXPSBuffer>
{
	uint8*									m_pBuffer;
	uint32									m_Size;
	uint16									m_MemItemID;
#ifdef CRY_USE_DX9
public:
													CCryDXPSBuffer(IDirect3DVertexBuffer9* pVBuffer):
													CCryRefAndWeak<CCryDXPSBuffer>(this),
													CCryDXPSResource(EDXPS_RT_VERTEXBUFFER),
													m_pBuffer(pVBuffer),
													m_Size(0)
													{
														if(pVBuffer)
															pVBuffer->AddRef();
													}
													CCryDXPSBuffer(IDirect3DIndexBuffer9* pIBuffer):
													CCryRefAndWeak<CCryDXPSBuffer>(this),
													CCryDXPSResource(EDXPS_RT_INDEXBUFFER),
													m_pBuffer(pIBuffer),
													m_Size(0)
													{
														if(pIBuffer)
															pIBuffer->AddRef();
													}

	IDirect3DVertexBuffer9*	VBuffer(){return Type()==EDXPS_RT_VERTEXBUFFER?reinterpret_cast<IDirect3DVertexBuffer9*>(m_pBuffer):0;}
	IDirect3DIndexBuffer9*	IBuffer(){return Type()==EDXPS_RT_INDEXBUFFER?reinterpret_cast<IDirect3DIndexBuffer9*>(m_pBuffer):0;}
private:
#elif CRY_USE_OGL
	unsigned	int						m_BufferID;
public:
													CCryDXPSBuffer(const void* pVertices,uint32 Size):
														CCryDXPSResource(EDXPS_RT_VERTEXBUFFER),
														CCryRefAndWeak<CCryDXPSBuffer>(this)
													{
														glGenBuffers(1,&m_BufferID);
														CRY_GL_CHECKERROR;
														glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
														CRY_GL_CHECKERROR;
														glBufferData(GL_ARRAY_BUFFER, Size,pVertices,GL_STATIC_DRAW);
														CRY_GL_CHECKERROR;
													}

	uint32						VBO()const{return m_BufferID;}
#elif CRY_USE_GCM
#else
	//Unsupported DeviceType
#endif

public:
		CCryDXPSBuffer(const void* pData,uint32 Size,CCRYDXPSResType T):
			CCryDXPSResource(T), CCryRefAndWeak<CCryDXPSBuffer>(this),
			m_Size(Size)
		{
			//PS3OPTIMIZE move indexbuffer to mainmemory 'cause it might be read be cellgcmdrawindexed via ppu from vram  ->slow
			if(Type()==EDXPS_RT_INDEXBUFFER || Type()==EDXPS_RT_VERTEXBUFFER)
			{
				m_MemItemID	=	tdLayer0::Instance().Memory().Allocate(Size);
				m_pBuffer		=	reinterpret_cast<uint8*>(tdLayer0::Instance().Memory().ResolveHandle(m_MemItemID));
			}
			else
			if(Type()==EDXPS_RT_CONSTBUFFER)
			{
				CRY_ASSERT_MESSAGE(Size<468*16,"Size of const buffer exceeds 468 float4");
				m_pBuffer		=	CRY_DXPS_NEWARRAY(uint8,Size);
			}
			else
			{
				m_pBuffer		=	CRY_DXPS_NEWARRAY(uint8,Size);
			}
			if(pData)
				memcpy(m_pBuffer,pData,Size);
//				for(uint32 a=0;a<Size;a++)
//					m_pBuffer[a]	=	reinterpret_cast<const uint8*>(pData)[a];
		}
		inline	~CCryDXPSBuffer()
		{
#ifdef CRY_USE_DX9				
		IDirect3DVertexBuffer9* pVBuffer	=	VBuffer();
		IDirect3DIndexBuffer9* pIBuffer	=	IBuffer();
		if(pVBuffer)
			pVBuffer->Release();
		if(pIBuffer)
			pIBuffer->Release();
#elif CRY_USE_OGL
		glDeleteBuffers(1,&m_BufferID);
#elif CRY_USE_GCM
			ReleaseResources();
#else
		//Unsupported DeviceType
#endif
	}
	inline	void	ReleaseResources()
	{
		if(m_pBuffer)
		if(Type()==EDXPS_RT_INDEXBUFFER || Type()==EDXPS_RT_VERTEXBUFFER)
		{
			tdLayer0::Instance().Memory().Free(m_MemItemID);
#if defined(_DEBUG)
			m_MemItemID	=	0;
#endif
		}
		else
			CRY_DXPS_DELETEARRAY(m_pBuffer);
		m_pBuffer=0;
	}
	long	Map(D3D10_MAP MapType,uint32 MapFlags,void** ppData);
	long	Unmap();
	long	GetDesc(D3D10_BUFFER_DESC *pDesc);
	uint32	Size()const{return m_Size;}
//	void*	CBuffer(){return Type()==EDXPS_RT_CONSTBUFFER?m_pBuffer:0;}
	uint8*	RawData(){return m_pBuffer;}
	const uint8*	RawData()const{return m_pBuffer;}
	unsigned long		AddRef(){return IncRef();}
	unsigned long		Release(){return DecRef();}

};

typedef CCryDXPSBuffer ID3D10Buffer;
typedef CCryAPtrWeakCnt<CCryDXPSBuffer>	APWeakBuffer;

#endif

