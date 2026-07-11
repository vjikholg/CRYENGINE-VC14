/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2008.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:  common anti-cheat defence data
-------------------------------------------------------------------------
History:
- 10/08/2004   : Created by Craig Tiller, based on CDefenceWall by Timur
- 17/11/2008	 : Refactored slightly by Steve Humphreys
*************************************************************************/

#include "StdAfx.h"
#include "FileProbe.h"

//
// probe thread
//

void CProbeThread::Run()
{
	CryThreadSetName( -1, "ServerProbe" );

	while (true)
	{
		CProbeRequest* p = NULL;
		{
			CryAutoLock<TLock> lk(m_mtx);
			while (m_incoming.empty())
				m_cond.Wait( m_mtx );
			p = m_incoming.front();
			m_incoming.pop();
		}

		if (!p)
			break;

		FUNCTION_PROFILER(gEnv->pSystem, PROFILE_NETWORK);
		static SDataProbeContext ctx;
		ctx = p->ctx;
		gEnv->pSystem->GetIDataProbe()->GetCode( ctx );
		p->codes[p->ncodes++] = ctx.nCode;

		// hack so that dds files on high/low spec machines can be checked
		if (p->isServer && PathUtil::MatchWildcard(ctx.sFilename, "*.dds") && ctx.sFilename.length() > 5 && 0 == stricmp(ctx.sFilename.substr(0,5).c_str(), "game/"))
		{
			ctx = p->ctx;
			ctx.sFilename = "game/_server_lowspec/" + ctx.sFilename.substr(5);
			gEnv->pSystem->GetIDataProbe()->GetCode( ctx );
			p->codes[p->ncodes++] = ctx.nCode;
		}

		// similar hack for downloaded maps... they might be in a different location on clients
		if(!p->isServer && p->codes[0] == 0 && ctx.sFilename.length() > 12 && 0 == stricmp(ctx.sFilename.substr(0,12).c_str(), "game/levels/"))
		{
			ctx.sFilename = "%USER%/Downloads/Levels/" + ctx.sFilename.substr(12);
			gEnv->pSystem->GetIDataProbe()->GetCode(ctx);
			p->codes[0] = ctx.nCode;
		}

		p->Complete();
	}
}

void CProbeThread::Push(CProbeRequest* p)
{
	CryAutoLock<TLock> lk(m_mtx);
	m_incoming.push(p);
	m_cond.Notify();
}

CProbeThread& CProbeThread::GetProbeThread()
{
	static CProbeThread * pThread = 0;
	if (!pThread)
	{
		pThread = new CProbeThread;
		pThread->Start();
	}
	return *pThread;
}