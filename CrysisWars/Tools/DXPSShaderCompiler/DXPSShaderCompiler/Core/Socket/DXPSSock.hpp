#ifndef __DXPSSOCK__
#define __DXPSSOCK__

#ifdef UNIX
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#define closesocket close
#endif

#ifdef _MSC_VER
#include <WinSock2.h>
#endif
#include <vector>


enum EDXPSS_TYPE
{
	EDXPSST_ROOT,
	EDXPSST_SERVER,
	EDXPSST_INVALID,
};

class CDXPSSock
{
	static CDXPSSock*		m_pInstance;

	const EDXPSS_TYPE		m_Type;
	SOCKET							m_Socket;


											CDXPSSock();
											CDXPSSock(SOCKET Sock);
											CDXPSSock(const CDXPSSock&):m_Type(EDXPSST_INVALID){};

	void								InitClient();
	void								Release();
	void								ReleaseInstance();
public:

											~CDXPSSock();
	void								InitRoot(uint16_t Port);

	static CDXPSSock&		Instance();

	CDXPSSock*					Accept();

	bool								Recv(std::vector<uint8_t>& rVec);
	void								Send(std::vector<uint8_t>& rVec);
};

#endif
