/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2008.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:  common anti-cheat defence data - server side
-------------------------------------------------------------------------
History:
- 10/08/2004   : Created by Craig Tiller, based on CDefenceWall by Timur
- 17/11/2008	 : Refactored slightly by Steve Humphreys
*************************************************************************/

#include "StdAfx.h"
#include "ServerDefence.h"

#ifdef USE_DEFENCE

#include "Protocol/NetChannel.h"
#include "FileProbe.h"
#include "ClientDefence.h"
#include "Context/ContextView.h"

class CServerDefence::CServerFileProbeRequestMessage : public INetMessage
{
public:
	static CServerFileProbeRequestMessage * Create(CServerDefence * pDefence, const SDataProbeContext& ctx, uint32 id, CMementoMemoryManager& mmm)
	{
		return new (mmm.AllocPtr(sizeof(CServerFileProbeRequestMessage))) CServerFileProbeRequestMessage(pDefence, ctx, id, mmm);
	}

	EMessageSendResult WritePayload( TSerialize ser, uint32 a, uint32 b )
	{
		ser.Value("id", m_id);
		ser.Value("filename", m_ctx.sFilename);
		ser.Value("offset", m_ctx.nOffset);
		ser.Value("size", m_ctx.nSize);
		ser.Value("codeinfo", m_ctx.nCodeInfo);
		m_pDefence->SentRequest( m_id, m_ctx );
		return eMSR_SentOk;
	}

	void UpdateState( uint32 seq, ENetSendableStateUpdate state )
	{
	}

	size_t GetSize() { return sizeof(*this); }

private:
	CServerFileProbeRequestMessage( CServerDefence * pDefence, const SDataProbeContext& ctx, uint32 id, CMementoMemoryManager& mmm ) : 
			 INetMessage(CClientDefence::FileCheckMessage), m_pDefence(pDefence), m_ctx(ctx), m_id(id), m_mmm(mmm) 
			 {
				 SetGroup('CHK');
				 SetPriorityDelta( (cry_rand32()%10)/10.0f - 0.5f );
			 }

			 void DeleteThis()
			 {
				 CMementoMemoryManager* pMMM = &m_mmm;
				 this->~CServerFileProbeRequestMessage();
				 pMMM->FreePtr( this, sizeof(CServerFileProbeRequestMessage) );
			 }

			 CServerDefence * m_pDefence;
			 uint32 m_id;
			 SDataProbeContext m_ctx;
			 CMementoMemoryManager& m_mmm;
};



//
// CServerDefence
//

int CServerDefence::m_serverDefenceCount = 0;
int CServerDefence::m_oldLowSpecPak = 0;

CServerDefence::CServerDefence( CNetChannel * pParent )
{
	m_pendingRequests = 0;
	m_pParent = pParent;
	m_pMMM = pParent->GetChannelMMM();
	m_nextID = cry_rand32();
	m_incDefenceCount = false;

	m_timer = TIMER.AddTimer( g_time + 10.0f, TimerCallback, this );
	m_nextSend = TIMER.AddTimer( g_time + 3.5f + (cry_rand32() % 10) / 5.0f, SendNextRequestCallback, this );

	MMM_REGION(m_pMMM);
	m_protectedFiles.reset( new TPFMap );
	m_pending.reset( new TPMap );

	if (gEnv->bMultiplayer)
	{
		m_incDefenceCount = true;
		if (1 == ++m_serverDefenceCount)
		{
			if (ICVar	* pVar = gEnv->pConsole->GetCVar("sys_LowSpecPak"))
			{
				m_oldLowSpecPak = pVar->GetIVal();
				pVar->Set(0);
				pVar->SetFlags(VF_READONLY | pVar->GetFlags());
			}
			gEnv->pCryPak->OpenPacks("_server_lowspec", "Game/Lowspec/*.pak");
		}
	}

	SendNextRequest();
}

CServerDefence::~CServerDefence()
{
	TIMER.CancelTimer( m_timer );
	TIMER.CancelTimer( m_nextSend );

	if (m_incDefenceCount)
	{
		if (0 == --m_serverDefenceCount)
		{
			gEnv->pCryPak->ClosePacks("Game/Lowspec/*.pak");
			if (ICVar	* pVar = gEnv->pConsole->GetCVar("sys_LowSpecPak"))
			{
				pVar->SetFlags(~VF_READONLY & pVar->GetFlags());
				pVar->Set(m_oldLowSpecPak);
			}
		}
	}

	MMM_REGION(m_pMMM);
	m_protectedFiles.reset();
	m_pending.reset();
}

void CServerDefence::AddProtectedFile( const CDefenceData::SProtectedFile& fn )
{
	MMM_REGION(m_pMMM);
	m_protectedFiles->insert(std::make_pair(cry_rand32() % (10+m_protectedFiles->size()), fn));
}

void CServerDefence::ClearProtectedFiles()
{
	MMM_REGION(m_pMMM);
	m_protectedFiles->clear();
}

bool CServerDefence::CheckLevel( const string& level )
{
	if (!level.empty())
		if (m_pParent)
			if (CContextView * pView = m_pParent->GetContextView())
				if (CNetContextState * pState = pView->ContextState())
					if (pState->GetLevelName().compareNoCase(level) != 0)
						return false;

	return true;
}

void CServerDefence::SendNextRequest()
{
	if (!CNetCVars::Get().CheatProtectionLevel || !gEnv->bMultiplayer || m_pParent->IsSuicidal())
		return;

	if(m_pParent->IsLocal())
		return;

	MMM_REGION(m_pMMM);

	IDataProbe * pProbe = gEnv->pSystem->GetIDataProbe();

	if (m_protectedFiles->empty())
		return;
	CDefenceData::SProtectedFile file;
	do 
	{
		file = m_protectedFiles->begin()->second;

		uint32 oldPrio = m_protectedFiles->begin()->first;
		m_protectedFiles->erase( m_protectedFiles->begin() );
		uint32 newPrio = (uint32) (oldPrio + (0.7f + 0.3f*cry_frand())*m_protectedFiles->size());
		//		NetLog("new priority: %u (was %u)", newPrio, oldPrio);
		m_protectedFiles->insert( std::make_pair( newPrio, file ) );
	} 
	while (!CheckLevel(file.level));

	SDataProbeContext probe;
	probe.nCodeInfo = rand()%3;
	probe.nOffset = 0;
	probe.nSize = 0xFFFFFFFF; // all file.
	probe.sFilename = file.name;
	if (!GetISystem()->GetIDataProbe()->GetRandomFileProbe( probe, false ))
	{
		SendNextRequest();
		return;
	}

	uint32 id = m_nextID++;

	//NetLogAlways("checkfile: %s %f", probe.sFilename);
	m_pParent->NetAddSendable( CServerFileProbeRequestMessage::Create(this, probe, id, *m_pMMM), 0, NULL, NULL );
}

void CServerDefence::SentRequest( uint32 id, const SDataProbeContext& ctx )
{
	MMM_REGION(m_pMMM);
	m_pending->insert(std::make_pair(id, ctx));
}

void CServerDefence::DefineProtocol( IProtocolBuilder * pBuilder )
{
	pBuilder->AddMessageSink( this, 
		CClientDefence::GetProtocolDef(), 
		CServerDefence::GetProtocolDef() );
}

bool CServerDefence::HasRemoteDef( const SNetMessageDef * pDef )
{
	return CClientDefence::ClassHasDef( pDef );
}

void CServerDefence::TimerCallback(NetTimerId, void* p, CTimeValue now)
{
	CServerDefence * pThis = static_cast<CServerDefence*>(p);
	MMM_REGION(pThis->m_pMMM);
	for (TPMap::iterator it = pThis->m_pending->begin(); it != pThis->m_pending->end(); ++it)
	{
		if ((it->second.when - g_time).GetSeconds() > 60.0f)
		{
			string err;
			err.Format("Client %s took too long replying to server file check! (profileid=%d); file was %s",
				pThis->m_pParent->GetRemoteAddressString().c_str(), pThis->m_pParent->GetProfileId(), it->second.ctx.sFilename.c_str());

			NetLogAlways_Secret( err.c_str() );
			pThis->m_pParent->DisconnectThroughPB( "Too long waiting for server file check!", err.c_str());
		}
	}
	pThis->m_timer = TIMER.AddTimer( now + 2.0f, TimerCallback, p );
}

void CServerDefence::SendNextRequestCallback(NetTimerId, void* p, CTimeValue now)
{
	CServerDefence * pThis = static_cast<CServerDefence*>(p);
	pThis->SendNextRequest();
	pThis->m_nextSend = TIMER.AddTimer( g_time + (cry_rand32() % 10) / 10.0f, SendNextRequestCallback, p );
}

class CServerDefence::CServerValidate : public CProbeRequest
{
public:
	static CServerValidate * Create(const SDataProbeContext& c, uint64 nCode, _smart_ptr<CNetChannel> pChannel, CMementoMemoryManager& mmm, volatile int * pRC)
	{
		return new (mmm.AllocPtr(sizeof(CServerValidate))) CServerValidate(c, nCode, pChannel, mmm, pRC);
	}

	void Complete()
	{
		CReleaseRefCount rrc(this);
		bool anyMatches = false;
		for (int i=0; i<ncodes; i++)
			if (codes[i] == m_nCode)
				anyMatches = true;
		if (!anyMatches)
		{
			string err;
			err.Format("Client hash mismatch from %s (profileid=%d); file was %s", m_pChannel->GetRemoteAddressString().c_str(), m_pChannel->GetProfileId(), ctx.sFilename.c_str());

			NetLogAlways_Secret( err.c_str() );

			m_pChannel->DisconnectThroughPB( "Server probe hash mismatch (probable cheat)", err.c_str() );
		}
		CMementoMemoryManager * pMMM = &m_mmm;
		this->~CServerValidate();
		SCOPED_GLOBAL_LOCK;
		pMMM->FreePtr(this, sizeof(CServerValidate));
	}

private:
	CServerValidate( const SDataProbeContext& c, uint64 nCode, _smart_ptr<CNetChannel> pChannel, CMementoMemoryManager& mmm, volatile int * pRC ) : CProbeRequest(c, true, pRC), m_nCode(nCode), m_pChannel(pChannel), m_mmm(mmm) {}

	uint64 m_nCode;
	_smart_ptr<CNetChannel> m_pChannel;
	CMementoMemoryManager& m_mmm;
};

NET_IMPLEMENT_IMMEDIATE_MESSAGE( CServerDefence, FileCheckMessage, eNRT_ReliableUnordered, 0 )
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_NETWORK);

	MMM_REGION(m_pMMM);

	uint32 id;
	ser.Value("id", id);
	TPMap::iterator it = m_pending->find(id);
	if (it == m_pending->end())
		return false;
	if ((it->second.when - g_time).GetSeconds() > 60.0f)
	{
		m_pending->erase(it);
		return false;
	}
	uint64 nCode;
	ser.Value("code", nCode);

	CProbeThread::GetProbeThread().Push(CServerValidate::Create(it->second.ctx, nCode, m_pParent, *m_pMMM, &m_pendingRequests) );
	m_pending->erase(it);

	return true;
}

#endif	// USE_DEFENCE