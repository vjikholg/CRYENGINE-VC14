#ifndef __NETTIMER_H__
#define __NETTIMER_H__

#pragma once

#include "TimeValue.h"
#include <queue>
#include <memory>
//#include "VectorMap.h"
#include "STLPoolAllocator.h"
#include "Config.h"

typedef int NetTimerId;
typedef void (*NetTimerCallback)(NetTimerId, void*, CTimeValue);

static const int TIMER_HERTZ = 250;

class CNetTimer
{
public:
	CNetTimer();
	NetTimerId AddTimer( CTimeValue when, NetTimerCallback callback, void * pUserData );
	void * CancelTimer( NetTimerId& id );
	CTimeValue Update();

	void GetMemoryStatistics(ICrySizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetTimer");
		if (countingThis)
			pSizer->Add(*this);
		pSizer->AddContainer(m_callbacks);
	}

private:
	static const int TIMER_SLOTS = 32; // 0.128 seconds
	int m_timerCallbacks[TIMER_SLOTS];
	int m_curSlot;
	using SlowCallbacksValue = std::pair<const CTimeValue, int>;
#if USE_SYSTEM_ALLOCATOR
	using SlowCallbacksAllocator = std::allocator<SlowCallbacksValue>;
#else
	using SlowCallbacksAllocator = stl::STLPoolAllocator<
		SlowCallbacksValue,
		stl::PoolAllocatorSynchronizationSinglethreaded>;
#endif
	using SlowCallbacks = std::multimap<
		CTimeValue,
		int,
		std::less<CTimeValue>,
		SlowCallbacksAllocator>;
	SlowCallbacks m_slowCallbacks;

	struct SCallbackInfo
	{
		SCallbackInfo( NetTimerCallback callback, void * pUserData )
		{
			this->callback = callback;
			this->pUserData = pUserData;
			next = -1;
			cancelled = true;
		}
		SCallbackInfo()
		{
			this->callback = 0;
			this->pUserData = 0;
			next = -1;
			cancelled = true;
		}
		NetTimerCallback callback;
		void * pUserData;
		int next;
		bool cancelled;
#if TIMER_DEBUG
		CTimeValue schedTime;
		int slotsInFuture;
		int curSlotWhenScheduled;
#endif
	};

	std::vector<SCallbackInfo> m_callbacks;
	std::vector<int> m_freeCallbacks;
	std::vector<int> m_execCallbacks;
	CTimeValue m_wakeup;
	CTimeValue m_lastExec;
#if TIMER_DEBUG
	CTimeValue m_epoch; // just for debugging
#endif
};

#endif
