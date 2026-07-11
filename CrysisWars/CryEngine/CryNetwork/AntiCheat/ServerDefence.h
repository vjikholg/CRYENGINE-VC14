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

#ifndef __SERVERDEFENCE_H__
#define __SERVERDEFENCE_H__

#pragma once

#include "Config.h"
#include "Network.h"

#if USE_DEFENCE

#include "DefenceContext.h"
#include "NetHelpers.h"
#include "STLMementoAllocator.h"

class CServerDefence : public CNetMessageSinkHelper<CServerDefence, IDefenceContext>
{
public:
	CServerDefence( CNetChannel * );
	~CServerDefence();
	void DefineProtocol( IProtocolBuilder * pBuilder );
	virtual void AddProtectedFile( const CDefenceData::SProtectedFile& );
	virtual void ClearProtectedFiles();
	virtual bool HasRemoteDef( const SNetMessageDef * pDef );
	virtual bool CanRemove() { return m_pendingRequests == 0; }

	NET_DECLARE_IMMEDIATE_MESSAGE(FileCheckMessage);

private:
	void SendNextRequest();
	void SentRequest( uint32 id, const SDataProbeContext& ctx );
	bool CheckLevel( const string& level );

	NetTimerId m_timer;
	NetTimerId m_nextSend;
	static void TimerCallback(NetTimerId, void* p, CTimeValue now);
	static void SendNextRequestCallback(NetTimerId, void* p, CTimeValue now);

	CNetChannel * m_pParent;
	CMementoMemoryManagerPtr m_pMMM;

	typedef std::multimap<uint32, CDefenceData::SProtectedFile, std::less<uint32>, STLMementoAllocator< std::pair<uint32, string> > > TPFMap;
	std::auto_ptr<TPFMap> m_protectedFiles;
	struct SPending
	{
		SPending( const SDataProbeContext& c ) : ctx(c), when(g_time) {}
		SDataProbeContext ctx;
		CTimeValue when;
	};
	typedef std::map<uint32, SPending, std::less<uint32>, STLMementoAllocator< std::pair<uint32, SPending> > > TPMap;
	std::auto_ptr<TPMap> m_pending;
	uint32 m_nextID;

	class CServerFileProbeRequestMessage;
	class CServerValidate;

	// refcount for number of defence contexts - so we can tear up/down our fake lowspec directory
	static int m_serverDefenceCount;
	// were low spec paks enabled
	static int m_oldLowSpecPak;

	bool m_incDefenceCount;
	volatile int m_pendingRequests;
};

#endif	// USE_DEFENCE

#endif	// ndef __SERVERDEFENCE_H__