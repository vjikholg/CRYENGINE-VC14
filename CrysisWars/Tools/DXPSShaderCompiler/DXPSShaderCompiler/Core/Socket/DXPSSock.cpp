#include "../StdTypes.hpp"
#include "DXPSSock.hpp"
#include "../Error.hpp"

CDXPSSock* CDXPSSock::m_pInstance=0;


CDXPSSock::CDXPSSock():
m_Type(EDXPSST_ROOT)
{
}

CDXPSSock::CDXPSSock(SOCKET Sock):
m_Type(EDXPSST_SERVER),
m_Socket(Sock)
{
	InitClient();
}


CDXPSSock::~CDXPSSock()
{
	Release();
}

void CDXPSSock::InitRoot(uint16_t Port)
{
#ifdef _MSC_VER
	WSADATA Data;
	m_Socket	=	INVALID_SOCKET;
	if(WSAStartup(MAKEWORD(2,0),&Data))
	{
			DXPS_ERROR("Could not init root socket");
			return;
	}
#endif

	sockaddr_in SockAddr;
	memset(&SockAddr,0,sizeof(sockaddr_in));

	SockAddr.sin_family	=	PF_INET;
	SockAddr.sin_port		=	htons(Port);
	m_Socket	=	socket(AF_INET,SOCK_STREAM,0);
	if(INVALID_SOCKET==m_Socket)
	{
		DXPS_ERROR("Could not initialize basic server due to invalid socket");
		return;
	}
#ifdef UNIX
	int arg = 1;
	setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, &arg, sizeof arg);
	arg = 6;
	setsockopt(m_Socket, SOL_SOCKET, SO_PRIORITY, &arg, sizeof arg);
	arg = 1;
	setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof arg);
#endif

	if(bind(m_Socket,(sockaddr*)&SockAddr,sizeof(sockaddr_in))==SOCKET_ERROR)
	{
		closesocket(m_Socket);
		DXPS_ERROR("Could not bind server socket");
		return;
	}
	listen(m_Socket,5);
}

void CDXPSSock::InitClient()
{
}

void CDXPSSock::Release()
{
	if(m_Socket!=INVALID_SOCKET)
		closesocket(m_Socket);

#ifdef _MSC_VER
	switch(m_Type)
	{
		case	EDXPSST_ROOT:WSACleanup();break;
		case	EDXPSST_SERVER:;break;
		default:
			DXPS_ERROR("unknown SocketType Released");
	}
#endif
}

CDXPSSock& CDXPSSock::Instance()
{
	if(!m_pInstance)
		m_pInstance	=	new	CDXPSSock();
	return *m_pInstance;
}

void CDXPSSock::ReleaseInstance()
{
	delete m_pInstance;
	m_pInstance=0;
}

CDXPSSock* CDXPSSock::Accept()
{
	if(m_Type!=EDXPSST_ROOT)
	{
		DXPS_ERROR("called Accept on non root socket");
		return 0;
	}

	while (true)
	{
		SOCKET Sock=accept(m_Socket,0,0);
		if(Sock==INVALID_SOCKET)
		{
#if defined _MSC_VER
			int Error=WSAGetLastError();
			DXPS_ERROR("Accept recived invalid socket");
#elif defined UNIX
			if (errno == EINTR
  #ifdef LINUX
					|| errno == 512 /* ERESTARTSYS */
	#endif
	#ifdef ETIMEDOUT
					|| errno == ETIMEDOUT
	#endif
					)
				continue;
			char errbuf[256];
			snprintf(errbuf, sizeof errbuf, "accept(): %s", strerror(errno));
			errbuf[sizeof errbuf - 1] = 0;
			DXPS_ERROR(errbuf);
#endif
			return 0;
		}
		return new CDXPSSock(Sock);
	}
}

union DXPSRecvSize
{
	uint8_t		m_Data8[4];
	uint32_t	m_Data32;
};

bool CDXPSSock::Recv(std::vector<uint8_t>& rVec)
{
	DXPSRecvSize Size;
	//it shouldn't happen, but it's possible that tcp-packages are splitted
	//bruteforce solution: read atomar units for size + endian conversion
	recv(m_Socket,reinterpret_cast<char*>(&Size.m_Data8[3]),1,0);
	recv(m_Socket,reinterpret_cast<char*>(&Size.m_Data8[2]),1,0);
	recv(m_Socket,reinterpret_cast<char*>(&Size.m_Data8[1]),1,0);
	recv(m_Socket,reinterpret_cast<char*>(&Size.m_Data8[0]),1,0);

	if(Size.m_Data32==0)
		return false;

	rVec.clear();
	rVec.resize(Size.m_Data32);

	for(uint32_t a=0;a<Size.m_Data32;)
	{
		int Read=recv(m_Socket,reinterpret_cast<char*>(&rVec[a]),Size.m_Data32-a,0);
		if(Read<=0)
		{
			DXPS_ERROR("Error while reciving tcp-data");
			return false;
		}
		a+=Read;
	}
	return true;
}

void CDXPSSock::Send(std::vector<uint8_t>& rVec)
{
	DXPSRecvSize Size;
	Size.m_Data32=static_cast<int>(rVec.size());
	if(Size.m_Data32)
	{
//		send(m_Socket,reinterpret_cast<char*>(&Size),4,0);
		send(m_Socket,reinterpret_cast<char*>(&Size.m_Data8[3]),1,0);
		send(m_Socket,reinterpret_cast<char*>(&Size.m_Data8[2]),1,0);
		send(m_Socket,reinterpret_cast<char*>(&Size.m_Data8[1]),1,0);
		send(m_Socket,reinterpret_cast<char*>(&Size.m_Data8[0]),1,0);

		char* pData	=	reinterpret_cast<char*>(&rVec[0]);
		send(m_Socket,pData,Size.m_Data32,0);
	}
}
