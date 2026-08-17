#include "StdAfx.h"
#include "MementoMemoryManager.h"
#include "Network.h"
//#include <IRenderer.h>
#include "ITextModeConsole.h"

CMementoMemoryManager * CMementoMemoryRegion::m_pMMM = 0;

CMementoMemoryManager& MMM()
{
	return CMementoMemoryRegion::Get();
}

CMementoMemoryManager::CMementoMemoryManager()
	: m_totalAllocations(0)
	, m_handleCookie(NextHandleCookie())
{
	ASSERT_GLOBAL_LOCK;
	arith_zeroSizeHdl = InvalidHdl;
	for (int i=0; i<sizeof(pThings)/sizeof(*pThings); i++)
		pThings[i] = 0;
	m_pools[0] = new CPool<8>();
	m_pools[1] = new CPool<16>();
	m_pools[2] = new CPool<32>();
	m_pools[3] = new CPool<64>();
	m_pools[4] = new CPool<128>();
	m_pools[5] = new CPool<256>();
	m_pools[6] = new CPool<512>();
	m_pools[7] = new CPool<1024>();
	m_pools[8] = new CPool<2048>();
	m_pools[9] = new CPool<4096>();
	STATIC_CHECK(NPOOLS == 10, PoolsIncorrectlyInitialized);
}

CMementoMemoryManager::~CMementoMemoryManager()
{
	SCOPED_GLOBAL_LOCK;
	FreeHdl(arith_zeroSizeHdl);
	{
		MMM_REGION(this);
		for (int i=0; i<sizeof(pThings)/sizeof(*pThings); i++)
			SAFE_RELEASE(pThings[i]);
	}
	NET_ASSERT(m_totalAllocations == 0);
	NET_ASSERT(m_handles.size() == m_freeHandles.size());
	for (int i=0; i<NPOOLS; i++)
		delete m_pools[i];
}

CMementoMemoryManager::Hdl CMementoMemoryManager::AllocHdl( size_t sz, void * callerOverride )
{
	ASSERT_GLOBAL_LOCK;
	size_t updatedTotal = 0;
	if (!TryGetUpdatedAllocationTotal(0, sz, updatedTotal))
		return InvalidHdl;

	uint32 index = 0;
	if (m_freeHandles.empty())
	{
		if (m_handles.size() >= HandleIndexMask)
		{
			CryError("CMementoMemoryManager handle table exhausted");
			return InvalidHdl;
		}
		index = static_cast<uint32>(m_handles.size());
		m_handles.push_back(SHandleData());
	}
	else
	{
		index = m_freeHandles.back();
		m_freeHandles.pop_back();
	}

	SHandleData& handleData = m_handles[index];
	if (!InitHandleData(handleData, sz))
	{
		m_freeHandles.push_back(index);
		return InvalidHdl;
	}
	m_totalAllocations = updatedTotal;

#if MMM_CHECK_LEAKS
	void * caller = callerOverride? callerOverride : UP_STACK_PTR;
	m_hdlToAlloc[index] = caller;
	m_allocAmt[caller] += sz;
#endif

	return EncodeHdl(index, handleData.generation);
}

CMementoMemoryManager::Hdl CMementoMemoryManager::CloneHdl( Hdl hdl )
{
	ASSERT_GLOBAL_LOCK;
	const SHandleData* const source = ResolveHdl(hdl);
	if (source == nullptr)
		return InvalidHdl;

	const size_t sourceSize = source->size;
	const void* const sourceMemory = source->p;
	Hdl out = AllocHdl(sourceSize, UP_STACK_PTR);
	if (out == InvalidHdl)
		return InvalidHdl;
	memcpy(PinHdl(out), sourceMemory, sourceSize);
	return out;
}

void CMementoMemoryManager::ResizeHdl( Hdl hdl, size_t sz )
{
	ASSERT_GLOBAL_LOCK;
	uint32 index = 0;
	SHandleData* const resolved = ResolveHdl(hdl, &index);
	if (resolved == nullptr)
		return;
	SHandleData& hd = *resolved;

#ifdef CHECK_POOL_ALLOCATED_POINTERS
	hd.pPool->CheckPtr(hd.p);
#endif

	size_t updatedTotal = 0;
	if (!TryGetUpdatedAllocationTotal(hd.size, sz, updatedTotal))
		return;
	const size_t previousSize = hd.size;

	if (sz > hd.size) // growing
	{
		if (sz > hd.capacity) // growing and changing pools
		{
			SHandleData hdp;
			if (!InitHandleData(hdp, sz))
				return;
			memcpy( hdp.p, hd.p, hd.size );
			hd.pPool->Free(hd.p);
			hd.p = hdp.p;
			hd.size = hdp.size;
			hd.capacity = hdp.capacity;
			hd.pPool = hdp.pPool;
		}
		else
		{
			hd.size = sz;
		}
	}
	else if (sz < hd.size) // shrinking
	{
		if (sz < hd.capacity/2) // shrinking and changing pools
		{
			SHandleData hdp;
			if (!InitHandleData(hdp, sz))
				return;
			memcpy( hdp.p, hd.p, sz );
			hd.pPool->Free(hd.p);
			hd.p = hdp.p;
			hd.size = hdp.size;
			hd.capacity = hdp.capacity;
			hd.pPool = hdp.pPool;
		}
		else
		{
			hd.size = sz;
		}
	}

	m_totalAllocations = updatedTotal;
#if MMM_CHECK_LEAKS
	const std::map<uint32, void*>::const_iterator handleCaller =
		m_hdlToAlloc.find(index);
	if (handleCaller != m_hdlToAlloc.end())
	{
		const std::map<void*, size_t>::iterator callerAllocation =
			m_allocAmt.find(handleCaller->second);
		if (callerAllocation != m_allocAmt.end() &&
			callerAllocation->second >= previousSize)
		{
			callerAllocation->second =
				callerAllocation->second - previousSize + sz;
		}
	}
#endif
}

void CMementoMemoryManager::FreeHdl( Hdl hdl )
{
	ASSERT_GLOBAL_LOCK;
	if (hdl == InvalidHdl)
		return;

	uint32 index = 0;
	SHandleData* const resolved = ResolveHdl(hdl, &index);
	if (resolved == nullptr)
		return;
	SHandleData& hd = *resolved;
	if (m_totalAllocations >= hd.size)
		m_totalAllocations -= hd.size;
	else
	{
		CryError("CMementoMemoryManager allocation accounting underflow");
		m_totalAllocations = 0;
	}

#if MMM_CHECK_LEAKS
	const std::map<uint32, void*>::iterator handleCaller =
		m_hdlToAlloc.find(index);
	if (handleCaller != m_hdlToAlloc.end())
	{
		const std::map<void*, size_t>::iterator callerAllocation =
			m_allocAmt.find(handleCaller->second);
		if (callerAllocation != m_allocAmt.end())
		{
			if (callerAllocation->second > hd.size)
				callerAllocation->second -= hd.size;
			else
				m_allocAmt.erase(callerAllocation);
		}
		m_hdlToAlloc.erase(handleCaller);
	}
#endif

	hd.pPool->Free(hd.p);
	hd.p = nullptr;
	hd.size = 0;
	hd.capacity = 0;
	hd.pPool = nullptr;
	hd.allocated = false;
	++hd.generation;
	if (hd.generation == 0)
		hd.generation = 1;
	m_freeHandles.push_back(index);
}

int CMementoMemoryManager::GetPoolIndex(size_t size) noexcept
{
	size_t capacity = size_t{1} << FIRST_POOL;
	for (int pool = 0; pool < NPOOLS; ++pool)
	{
		if (size <= capacity)
			return pool;
		capacity <<= 1;
	}
	return NPOOLS;
}

bool CMementoMemoryManager::TryGetUpdatedAllocationTotal(
	size_t previousSize,
	size_t newSize,
	size_t& updatedTotal) const noexcept
{
	if (previousSize > m_totalAllocations)
	{
		CryError("CMementoMemoryManager allocation accounting underflow");
		updatedTotal = m_totalAllocations;
		return false;
	}

	const size_t retainedTotal = m_totalAllocations - previousSize;
	if (CryTryAddSize(retainedTotal, newSize, updatedTotal))
		return true;

	CRY_ALLOCATOR_SIZE_FAILURE(
		"CMementoMemoryManager allocation accounting",
		retainedTotal,
		1,
		(std::numeric_limits<size_t>::max)(),
		ALIGNMENT,
		newSize,
		ECryAllocatorFailureAction::ReturnNull);
	return false;
}

bool CMementoMemoryManager::InitHandleData(SHandleData& hd, size_t sz)
{
	ASSERT_GLOBAL_LOCK;
	const int pool = GetPoolIndex(sz);
	IPool* selectedPool = nullptr;
	void* memory = nullptr;
	size_t capacity = 0;
	if (pool < NPOOLS)
	{
		selectedPool = m_pools[pool];
		memory = selectedPool->Allocate(capacity);
	}
	else
	{
		selectedPool = &m_largeObjectPool;
		memory = m_largeObjectPool.AllocateSized(sz, capacity);
	}
	if (memory == nullptr)
	{
		CryError("CMementoMemoryManager failed to allocate %zu bytes", sz);
		return false;
	}

	hd.p = memory;
	hd.size = sz;
	hd.capacity = capacity;
	hd.pPool = selectedPool;
	hd.allocated = true;
	NET_ASSERT(hd.capacity >= sz);

#ifdef CHECK_POOL_ALLOCATED_POINTERS
	hd.pPool->CheckPtr(hd.p);
#endif
	return true;
}

void * CMementoMemoryManager::AllocPtr( size_t sz, void * callerOverride )
{
	ASSERT_GLOBAL_LOCK;
	SHandleData hd;
	size_t updatedTotal = 0;
	if (!TryGetUpdatedAllocationTotal(0, sz, updatedTotal) ||
		!InitHandleData(hd, sz))
	{
		return nullptr;
	}
	m_totalAllocations = updatedTotal;

#if MMM_CHECK_LEAKS
	void * caller = callerOverride? callerOverride : UP_STACK_PTR;
	m_ptrToAlloc[hd.p] = caller;
	m_allocAmt[caller] += sz;
#endif

	return hd.p;
}

void CMementoMemoryManager::FreePtr( void * p, size_t sz )
{
	ASSERT_GLOBAL_LOCK;
	if (p == nullptr)
		return;
	const int pool = GetPoolIndex(sz);
	if (pool < NPOOLS)
		m_pools[pool]->Free(p);
	else
		m_largeObjectPool.Free(p);

#if MMM_CHECK_LEAKS
	const std::map<void*, void*>::iterator pointerCaller = m_ptrToAlloc.find(p);
	if (pointerCaller != m_ptrToAlloc.end())
	{
		const std::map<void*, size_t>::iterator callerAllocation =
			m_allocAmt.find(pointerCaller->second);
		if (callerAllocation != m_allocAmt.end())
		{
			if (callerAllocation->second > sz)
				callerAllocation->second -= sz;
			else
				m_allocAmt.erase(callerAllocation);
		}
		m_ptrToAlloc.erase(pointerCaller);
	}
#endif

	if (m_totalAllocations >= sz)
		m_totalAllocations -= sz;
	else
	{
		CryError("CMementoMemoryManager allocation accounting underflow");
		m_totalAllocations = 0;
	}
}

void CMementoMemoryManager::AddHdlToSizer( Hdl hdl, ICrySizer * pSizer )
{
	const SHandleData* const handleData = ResolveHdl(hdl);
	if (handleData != nullptr)
		pSizer->AddObject(handleData->p, handleData->size);
}

void CMementoMemoryManager::GetMemoryStatistics(ICrySizer* pSizer, bool countingThis /* = false */)
{
	SIZER_COMPONENT_NAME(pSizer, "CMementoMemoryManager");

	if (countingThis)
		pSizer->Add(*this);
	pSizer->AddContainer(m_handles);
	pSizer->AddContainer(m_freeHandles);
}

void CMementoMemoryManager::DrawDebugLine( int grp, int x, int y, const char * fmt, ... )
{
	char buffer[512];

	va_list args;
	va_start( args, fmt );
	vsprintf_s( buffer, sizeof(buffer), fmt, args );
	va_end( args );

	float white[] = {1,1,1,1};

	gEnv->pRenderer->Draw2dLabel( x*10+10, (y+grp*12)*10+10, 1, white, false, "%s", buffer );
	if (ITextModeConsole * pC = gEnv->pSystem->GetITextModeConsole())
		pC->PutText( x, y+grp*13, buffer );
}

void CMementoMemoryManager::DebugDraw( int n )
{
#if ENABLE_DEBUG_KIT
	if ((CVARS.MemInfo & eDMM_Mementos) == 0)
		return;

	static const char * names[] = {
		"Memento data",
		"Packet data",
		"Object data",
		"Alphabet soup"
	};

	const char * name = "<unknown manager>";
	if (n >= 0 && n < (sizeof(names)/sizeof(*names)))
		name = names[n];

	int y = 0;
	DrawDebugLine( n, 0, y++, "Memento memory for %s", name );

	SPoolStats aggregateStats;
	for (int i=0; i<NPOOLS; i++)
	{
		SPoolStats stats = m_pools[i]->GetPoolStats();

		if (!stats.allocated)
			continue;

		DrawDebugLine( n, 2, y++, "Pool %d (%d byte blocks): alloc=%zu, used=%zu, waste=%.1f%%", i, 1<<(i+FIRST_POOL), stats.allocated, stats.used, stats.GetWastePercent() );

		aggregateStats.allocated += stats.allocated;
		aggregateStats.used += stats.used;
	}

	float liveWaste = 0;
	if (aggregateStats.allocated)
		liveWaste = 100.0f * (1.0f - float(m_totalAllocations) / float(aggregateStats.allocated));
	float poolWaste = 0;
	if (aggregateStats.used)
		poolWaste = 100.0f * (1.0f - float(m_totalAllocations) / float(aggregateStats.used));

	DrawDebugLine( n, 2, y++, "Total: live=%zu, alloc=%zu, used=%zu, live-waste=%.1f%%, alloc-waste=%.1f%%, pool-waste=%.1f%%", m_totalAllocations, aggregateStats.allocated, aggregateStats.used, liveWaste, aggregateStats.GetWastePercent(), poolWaste );
#endif
}

/*
 * CMementoStreamAllocator
 */

CMementoStreamAllocator::CMementoStreamAllocator( const CMementoMemoryManagerPtr& mmm ) : m_hdl(CMementoMemoryManager::InvalidHdl), m_pPin(0), m_mmm(mmm)
{
}

void * CMementoStreamAllocator::Alloc( size_t sz, void * callerOverride )
{
	NET_ASSERT( m_hdl == CMementoMemoryManager::InvalidHdl );
	m_hdl = m_mmm->AllocHdl(sz, callerOverride? callerOverride : UP_STACK_PTR);

	return m_pPin = m_mmm->PinHdl(m_hdl);
}

void * CMementoStreamAllocator::Realloc( void * old, size_t sz )
{
	NET_ASSERT(m_pPin == old && m_pPin);
	m_mmm->ResizeHdl( m_hdl, sz );
	if (m_mmm->GetHdlSize(m_hdl) != sz)
		return nullptr;

	return m_pPin = m_mmm->PinHdl(m_hdl);
}

void CMementoStreamAllocator::Free( void *old )
{
}
