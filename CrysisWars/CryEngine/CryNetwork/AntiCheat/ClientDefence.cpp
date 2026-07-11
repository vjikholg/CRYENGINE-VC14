/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2008.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:  common anti-cheat defence data - client side
-------------------------------------------------------------------------
History:
- 10/08/2004   : Created by Craig Tiller, based on CDefenceWall by Timur
- 17/11/2008	 : Refactored slightly by Steve Humphreys
*************************************************************************/

#include "StdAfx.h"
#include "ClientDefence.h"

#ifdef USE_DEFENCE

#include "Protocol/NetChannel.h"
#include "FileProbe.h"
#include "ServerDefence.h"

class CClientDefence::CClientFileProbeResponseMessage : public INetMessage
{
public:
	static CClientFileProbeResponseMessage * Create( uint32 id, uint64 nCode, CMementoMemoryManager& mmm )
	{
		return new (mmm.AllocPtr(sizeof(CClientFileProbeResponseMessage))) CClientFileProbeResponseMessage(id, nCode, mmm);
	}

	EMessageSendResult WritePayload( TSerialize ser, uint32 a, uint32 b )
	{
		ser.Value("id", m_id);
		ser.Value("code", m_nCode);
		return eMSR_SentOk;
	}

	void UpdateState( uint32 seq, ENetSendableStateUpdate state )
	{
	}

	size_t GetSize() { return sizeof(*this); }

private:
	CClientFileProbeResponseMessage( uint32 id, uint64 nCode, CMementoMemoryManager& mmm ) : 
			 INetMessage(CServerDefence::FileCheckMessage), m_id(id), m_nCode(nCode), m_mmm(mmm)
			 {
				 SetGroup('chk');
				 SetPriorityDelta( (cry_rand32()%10)/10.0f - 0.5f );
			 }

			 void DeleteThis()
			 {
				 CMementoMemoryManager * pMMM = &m_mmm;
				 this->~CClientFileProbeResponseMessage();
				 pMMM->FreePtr( this, sizeof(CClientFileProbeResponseMessage) );
			 }

			 uint32 m_id;
			 uint64 m_nCode;
			 CMementoMemoryManager& m_mmm;
};

//
// CClientDefence
//

CClientDefence::CClientDefence( CNetChannel * pParent )
{
	m_pendingRequests = 0;
	m_pParent = pParent;
	m_pMMM = pParent->GetChannelMMM();
}

class CClientDefence::CClientQuery : public CProbeRequest
{
public:
	static CClientQuery * Create( const SDataProbeContext& c, uint32 id, _smart_ptr<CNetChannel> pChannel, CMementoMemoryManager& mmm, volatile int * pRC )
	{
		return new (mmm.AllocPtr(sizeof(CClientQuery))) CClientQuery(c, id, pChannel, mmm, pRC);
	}

	void Complete()
	{
		SCOPED_GLOBAL_LOCK;
		CReleaseRefCount rrc(this);
		m_pChannel->AddSendable( CClientFileProbeResponseMessage::Create(m_id, codes[0], m_mmm), 0, NULL, NULL );
		CMementoMemoryManager * pMMM = &m_mmm;
		this->~CClientQuery();
		pMMM->FreePtr(this, sizeof(CClientQuery));
	}

private:
	CClientQuery( const SDataProbeContext& c, uint32 id, _smart_ptr<CNetChannel> pChannel, CMementoMemoryManager& mmm, volatile int * pRC ) : CProbeRequest(c, false, pRC), m_id(id), m_pChannel(pChannel), m_mmm(mmm) {}

	uint32 m_id;
	_smart_ptr<CNetChannel> m_pChannel;
	CMementoMemoryManager& m_mmm;
};

NET_IMPLEMENT_IMMEDIATE_MESSAGE( CClientDefence, FileCheckMessage, eNRT_ReliableUnordered, 0 )
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_NETWORK);

	uint32 id;
	SDataProbeContext ctx;
	ser.Value("id", id);
	ser.Value("filename", ctx.sFilename);
	ser.Value("offset", ctx.nOffset);
	ser.Value("size", ctx.nSize);
	ser.Value("codeinfo", ctx.nCodeInfo);

	CProbeThread::GetProbeThread().Push(CClientQuery::Create(ctx, id, m_pParent, *m_pMMM, &m_pendingRequests));

	return true;
}

void CClientDefence::DefineProtocol( IProtocolBuilder * pBuilder )
{
	pBuilder->AddMessageSink( this, 
		CServerDefence::GetProtocolDef(), 
		CClientDefence::GetProtocolDef() );
}

void CClientDefence::AddProtectedFile( const CDefenceData::SProtectedFile& )
{
}

void CClientDefence::ClearProtectedFiles()
{
}

bool CClientDefence::HasRemoteDef( const SNetMessageDef * pDef )
{
	return CServerDefence::ClassHasDef( pDef );
}

#endif