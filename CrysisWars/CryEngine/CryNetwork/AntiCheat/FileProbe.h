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

#ifndef __FILEPROBE_H__
#define __FILEPROBE_H__

#pragma once

#include "Config.h"
#include "Network.h"

#if USE_DEFENCE

#include "TimeValue.h"
#include "IDataProbe.h"
#include "CryThread.h"

class CProbeRequest
{
public:
	CProbeRequest( const SDataProbeContext& c, bool sv, volatile int * pRefCount ) : ctx(c), ncodes(0), isServer(sv), m_pRefCount(pRefCount) 
	{
		SCOPED_GLOBAL_LOCK;
		CryInterlockedIncrement(m_pRefCount);
	}
	SDataProbeContext ctx;
	// maximum probes done by the probe thread
	static const int NUM_CODES = 2;
	uint64 codes[NUM_CODES];
	int ncodes;
	bool isServer;

	virtual void Complete() = 0;

protected:
	// must be instantiated at the start of Complete()
	class CReleaseRefCount
	{
	public:
		CReleaseRefCount( CProbeRequest * pReq )
		{
			m_pRefCount = pReq->m_pRefCount;
		}

		~CReleaseRefCount()
		{
			CryInterlockedDecrement(m_pRefCount);
		}

	private:
		volatile int * m_pRefCount;
	};

private:
	// pointer to an owner refcount - for implementing IDefenceContext::CanRemove
	volatile int * m_pRefCount;
};

class CProbeThread : public CrySimpleThread<>
{
public:
	CProbeThread() {}

	void Run();
	void Cancel() { Push(NULL); }

	void Push(CProbeRequest*);

	static CProbeThread& GetProbeThread();

private:
	typedef CryCondLock<CRYLOCK_RECURSIVE> TLock;
	typedef CryCond<TLock> TCond;
	TLock m_mtx;
	TCond m_cond;
	std::queue<CProbeRequest*> m_incoming;
};

#endif // defined(USE_DEFENCE)

#endif	// defined(__FILEPROBE_H__)