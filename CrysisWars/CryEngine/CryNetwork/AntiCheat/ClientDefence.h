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

#ifndef __CLIENTDEFENCE_H__
#define __CLIENTDEFENCE_H__

#pragma once

#include "Config.h"
#include "Network.h"

#if USE_DEFENCE

#include "DefenceContext.h"
#include "NetHelpers.h"
#include "STLMementoAllocator.h"

class CClientDefence : public CNetMessageSinkHelper<CClientDefence, IDefenceContext>
{
public:
	CClientDefence( CNetChannel * );
	void DefineProtocol( IProtocolBuilder * pBuilder );
	virtual void AddProtectedFile( const CDefenceData::SProtectedFile& );
	virtual void ClearProtectedFiles();
	virtual bool HasRemoteDef( const SNetMessageDef * pDef );
	virtual bool CanRemove() { return m_pendingRequests == 0; }

	NET_DECLARE_IMMEDIATE_MESSAGE(FileCheckMessage);

private:
	CNetChannel * m_pParent;
	CMementoMemoryManagerPtr m_pMMM;
	volatile int m_pendingRequests;

	class CClientFileProbeResponseMessage;
	class CClientQuery;
};

#endif	// USE_DEFENCE

#endif	// ndef __CLIENTDEFENCE_H__