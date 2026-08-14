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
{
	std::memset(pThings, 0, sizeof(pThings));
	std::memset(m_pools, 0, sizeof(m_pools));

	SMementoAllocatorTestStats stats = {};
	stats.alive = true;
	g_mementoManagerStats[this] = stats;
}

CMementoMemoryManager::~CMementoMemoryManager()
{
	SMementoAllocatorTestStats& stats = g_mementoManagerStats[this];
	stats.destroyedWithOutstandingAllocations = stats.liveAllocationCount != 0;
	stats.alive = false;
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
