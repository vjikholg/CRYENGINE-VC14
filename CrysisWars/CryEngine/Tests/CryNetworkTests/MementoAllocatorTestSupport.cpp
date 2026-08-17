#include "CryNetworkTestPreamble.h"
#include "MementoAllocatorTestSupport.h"

#include <STLMementoAllocator.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <unordered_map>

struct SMementoAllocationRecord
{
	CMementoMemoryManager* manager;
	std::size_t bytes;
};

static std::unordered_map<const CMementoMemoryManager*, SMementoAllocatorTestStats>
	g_mementoManagerStats;
static std::unordered_map<void*, SMementoAllocationRecord> g_mementoAllocations;
static CMementoMemoryManager* g_currentMementoManager = 0;

void CryAssertTrace(const char*, ...)
{
}

bool CryAssert(const char*, const char*, unsigned int, bool*)
{
	return false;
}

void CryDebugBreak()
{
}

void CryAllocatorSizeFailure(const SCryAllocatorFailureContext& context) noexcept
{
	std::fprintf(
		stderr,
		"Allocator size failure: operation=%s count=%llu elementSize=%llu\n",
		context.operation,
		static_cast<unsigned long long>(context.count),
		static_cast<unsigned long long>(context.elementSize));
	if (context.action == ECryAllocatorFailureAction::WriteDumpAndTerminate)
		std::abort();
}

long CryInterlockedIncrement(int volatile* value)
{
	const int incremented = *value + 1;
	*value = incremented;
	return incremented;
}

long CryInterlockedDecrement(int volatile* value)
{
	const int decremented = *value - 1;
	*value = decremented;
	return decremented;
}

extern "C" void* CryModuleMalloc(size_t size) throw()
{
	return std::malloc(size);
}

extern "C" void* CryModuleCalloc(size_t count, size_t size) throw()
{
	return std::calloc(count, size);
}

extern "C" void* CryModuleRealloc(void* memory, size_t size) throw()
{
	return std::realloc(memory, size);
}

extern "C" void CryModuleFree(void* memory) throw()
{
	std::free(memory);
}

void CryError(const char*, ...)
{
}

CMementoMemoryManager& MMM()
{
	return *g_currentMementoManager;
}

void SetCurrentMementoManagerForTest(CMementoMemoryManager* manager)
{
	g_currentMementoManager = manager;
}

CMementoMemoryManager::CMementoMemoryManager()
	: arith_zeroSizeHdl(InvalidHdl)
	, m_totalAllocations(0)
	, m_handleCookie(NextHandleCookie())
{
	std::memset(pThings, 0, sizeof(pThings));
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

	SMementoAllocatorTestStats stats = {};
	stats.alive = true;
	g_mementoManagerStats[this] = stats;
}

CMementoMemoryManager::~CMementoMemoryManager()
{
	SMementoAllocatorTestStats& stats = g_mementoManagerStats[this];
	stats.destroyedWithOutstandingAllocations = stats.liveAllocationCount != 0;
	stats.alive = false;
	for (int pool = 0; pool < NPOOLS; ++pool)
		delete m_pools[pool];
}

CMementoMemoryManager::Hdl CMementoMemoryManager::AllocHdl(
	size_t size,
	void*)
{
	size_t updatedTotal = 0;
	if (!TryGetUpdatedAllocationTotal(0, size, updatedTotal))
		return InvalidHdl;

	uint32 index = 0;
	if (m_freeHandles.empty())
	{
		if (m_handles.size() >= HandleIndexMask)
			return InvalidHdl;
		index = static_cast<uint32>(m_handles.size());
		m_handles.push_back(SHandleData());
	}
	else
	{
		index = m_freeHandles.back();
		m_freeHandles.pop_back();
	}

	SHandleData& handleData = m_handles[index];
	if (!InitHandleData(handleData, size))
	{
		m_freeHandles.push_back(index);
		return InvalidHdl;
	}
	m_totalAllocations = updatedTotal;
	return EncodeHdl(index, handleData.generation);
}

CMementoMemoryManager::Hdl CMementoMemoryManager::CloneHdl(Hdl handle)
{
	const SHandleData* const source = ResolveHdl(handle);
	if (source == nullptr)
		return InvalidHdl;

	const size_t sourceSize = source->size;
	const void* const sourceMemory = source->p;
	const Hdl clone = AllocHdl(sourceSize);
	if (clone != InvalidHdl)
		std::memcpy(PinHdl(clone), sourceMemory, sourceSize);
	return clone;
}

void CMementoMemoryManager::ResizeHdl(Hdl handle, size_t size)
{
	SHandleData* const resolved = ResolveHdl(handle);
	if (resolved == nullptr)
		return;
	SHandleData& handleData = *resolved;

	size_t updatedTotal = 0;
	if (!TryGetUpdatedAllocationTotal(handleData.size, size, updatedTotal))
		return;
	if (size > handleData.capacity || size < handleData.capacity / 2)
	{
		SHandleData replacement;
		if (!InitHandleData(replacement, size))
			return;
		std::memcpy(
			replacement.p,
			handleData.p,
			size < handleData.size ? size : handleData.size);
		handleData.pPool->Free(handleData.p);
		handleData.p = replacement.p;
		handleData.size = replacement.size;
		handleData.capacity = replacement.capacity;
		handleData.pPool = replacement.pPool;
	}
	else
	{
		handleData.size = size;
	}
	m_totalAllocations = updatedTotal;
}

void CMementoMemoryManager::FreeHdl(Hdl handle)
{
	uint32 index = 0;
	SHandleData* const resolved = ResolveHdl(handle, &index);
	if (resolved == nullptr)
		return;

	SHandleData& handleData = *resolved;
	if (m_totalAllocations >= handleData.size)
		m_totalAllocations -= handleData.size;
	else
		m_totalAllocations = 0;
	handleData.pPool->Free(handleData.p);
	handleData.p = nullptr;
	handleData.size = 0;
	handleData.capacity = 0;
	handleData.pPool = nullptr;
	handleData.allocated = false;
	++handleData.generation;
	if (handleData.generation == 0)
		handleData.generation = 1;
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
		updatedTotal = m_totalAllocations;
		return false;
	}
	return CryTryAddSize(m_totalAllocations - previousSize, newSize, updatedTotal);
}

bool CMementoMemoryManager::InitHandleData(
	SHandleData& handleData,
	size_t size)
{
	const int pool = GetPoolIndex(size);
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
		memory = m_largeObjectPool.AllocateSized(size, capacity);
	}
	if (memory == nullptr)
		return false;

	handleData.p = memory;
	handleData.size = size;
	handleData.capacity = capacity;
	handleData.pPool = selectedPool;
	handleData.allocated = true;
	return true;
}

void* CMementoMemoryManager::AllocPtr(size_t size, void*)
{
	void* memory = std::malloc(size == 0 ? 1 : size);
	if (!memory)
	{
		throw std::bad_alloc();
	}

	g_mementoAllocations[memory] = {this, size};
	SMementoAllocatorTestStats& stats = g_mementoManagerStats[this];
	++stats.allocationCalls;
	++stats.liveAllocationCount;
	stats.liveBytes += size;
	m_totalAllocations += size;
	return memory;
}

void CMementoMemoryManager::FreePtr(void* memory, size_t size)
{
	SMementoAllocatorTestStats& deallocatingManagerStats = g_mementoManagerStats[this];
	++deallocatingManagerStats.deallocationCalls;

	const std::unordered_map<void*, SMementoAllocationRecord>::iterator allocation =
		g_mementoAllocations.find(memory);
	if (allocation == g_mementoAllocations.end())
	{
		++deallocatingManagerStats.invalidDeallocationCalls;
		return;
	}

	const SMementoAllocationRecord record = allocation->second;
	if (record.manager != this || record.bytes != size)
	{
		++deallocatingManagerStats.invalidDeallocationCalls;
	}

	SMementoAllocatorTestStats& owningManagerStats =
		g_mementoManagerStats[record.manager];
	--owningManagerStats.liveAllocationCount;
	owningManagerStats.liveBytes -= record.bytes;
	record.manager->m_totalAllocations -= record.bytes;
	g_mementoAllocations.erase(allocation);
	std::free(memory);
}

SMementoAllocatorTestStats GetMementoAllocatorTestStats(
	const CMementoMemoryManager* manager)
{
	const std::unordered_map<const CMementoMemoryManager*, SMementoAllocatorTestStats>::const_iterator
		stats = g_mementoManagerStats.find(manager);
	if (stats == g_mementoManagerStats.end())
	{
		return SMementoAllocatorTestStats{};
	}

	return stats->second;
}
