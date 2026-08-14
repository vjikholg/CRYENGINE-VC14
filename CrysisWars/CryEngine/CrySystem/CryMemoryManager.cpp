#include "StdAfx.h"

#include "CryAllocationHeader.h"
#include "DebugCallStack.h"

#include <CryMemoryByteCounter.h>
#include <ISystem.h>

#include <cstdlib>
#include <limits>
#include <new>
#include <stdio.h>

#if defined(MM_TRACE_ADDRS)
// Populate this array from the debugger to break when a selected address is
// allocated or freed.
void* CryMM_CheckAddr[16];
int CryMM_nCheckAddr = 0;

void CryMM_CheckAddrBreak(int, void*)
{
	// Set a breakpoint here.
}

static void CryMM_TraceAddress(void* memory)
{
	for (int index = 0; index < CryMM_nCheckAddr; ++index)
	{
		if (memory == CryMM_CheckAddr[index])
			CryMM_CheckAddrBreak(index, memory);
	}
}
#endif

static CryMemoryByteCounter g_totalAllocatedMemory;
static volatile int g_lockMemMan = 0;
static unsigned int g_biggestAllocation = 0;
static volatile bool g_processingAllocationTrace = false;

// CryMemoryManager.h redirects module CRT calls. CrySystem's allocator must
// call the real CRT when obtaining backing storage.
#undef malloc
#undef realloc
#undef free

node_alloc<eCryMallocCryFreeCRTCleanup, true, 524288> g_GlobPageBucketAllocator;

static_assert(
	_ALIGN % alignof(SCryAllocationHeader) == 0,
	"The Win64 node allocator must satisfy the Cry allocation header alignment.");

static void* CryAllocateWithHeader(size_t requestedSize, size_t& allocated)
{
	size_t allocationSize = 0;
	if (!CryTryCalculateAllocationSize(requestedSize, allocationSize))
	{
		allocated = 0;
		return 0;
	}

	void* const rawMemory = g_GlobPageBucketAllocator.alloc(allocationSize);
	if (!rawMemory)
	{
		allocated = 0;
		g_lockMemMan = 0;
		CryError("*** Memory allocation for %zu bytes failed ****", allocationSize);
		return 0;
	}

	SCryAllocationHeader* const header =
		::new (rawMemory) SCryAllocationHeader{requestedSize, allocationSize};
	void* const userMemory = CryGetAllocationUserMemory(header);

	g_totalAllocatedMemory.Add(allocationSize);
	allocated = allocationSize;

#if defined(MM_TRACE_ADDRS)
	CryMM_TraceAddress(userMemory);
#endif

	return userMemory;
}

void* CryMalloc(size_t size, size_t& allocated)
{
	if (g_bTraceAllocations && !g_processingAllocationTrace)
	{
		g_processingAllocationTrace = true;
		CryLogAlways("*** Memory allocation for %zu bytes ****", size);
		DebugCallStack::instance()->LogCallstack();
		g_processingAllocationTrace = false;
	}

	if (g_bProfilerEnabled)
	{
		FUNCTION_PROFILER_FAST(GetISystem(), PROFILE_SYSTEM, g_bProfilerEnabled);
	}

	return CryAllocateWithHeader(size, allocated);
}

size_t CryGetMemSize(void* memory, size_t)
{
	if (!memory)
		return 0;

	return CryGetAllocationHeader(memory)->allocationSize;
}

static void* CryAllocateForReallocation(size_t requestedSize, size_t& allocated)
{
	return CryMalloc(requestedSize, allocated);
}

static void* CryReallocateWithHeader(void* memory, size_t requestedSize, size_t& allocated)
{
	if (!memory)
		return CryAllocateForReallocation(requestedSize, allocated);

	const size_t oldRequestedSize = CryGetAllocationHeader(memory)->requestedSize;
	void* const replacement = CryAllocateForReallocation(requestedSize, allocated);
	if (!replacement)
		return 0;

	const size_t copySize = requestedSize < oldRequestedSize ? requestedSize : oldRequestedSize;
	memcpy(replacement, memory, copySize);
	CryFree(memory);
	return replacement;
}

void* CryRealloc(void* memory, size_t size, size_t& allocated)
{
#if defined(MM_TRACE_ADDRS)
	CryMM_TraceAddress(memory);
#endif

	if (g_bProfilerEnabled)
	{
		FUNCTION_PROFILER_FAST(GetISystem(), PROFILE_SYSTEM, g_bProfilerEnabled);
	}

	return CryReallocateWithHeader(memory, size, allocated);
}

size_t CryFree(void* memory)
{
#if defined(MM_TRACE_ADDRS)
	CryMM_TraceAddress(memory);
#endif

	if (g_bProfilerEnabled)
	{
		FUNCTION_PROFILER_FAST(GetISystem(), PROFILE_SYSTEM, g_bProfilerEnabled);
	}

	if (!memory)
		return 0;

	SCryAllocationHeader* const header = CryGetAllocationHeader(memory);
	const size_t size = header->allocationSize;
	g_totalAllocatedMemory.Remove(size);
	g_GlobPageBucketAllocator.dealloc(header, size);
	return size;
}

void CryFlushAll()
{
	// The node allocator owns process-lifetime pools. Resetting this counter
	// while allocations remain live corrupts the next free operation.
	CRY_ASSERT_MESSAGE(
		g_totalAllocatedMemory.Load() == 0,
		"CryFlushAll cannot discard accounting for live allocations");
}

DLL_EXPORT int CryMemoryGetAllocatedSize()
{
	return g_totalAllocatedMemory.LoadLegacyInt();
}

uint64 CryMemoryGetAllocatedSize64()
{
	return g_totalAllocatedMemory.Load();
}

DLL_EXPORT int CryMemoryGetPoolSize()
{
	ReadLock lock(g_lockMemMan);
	return 0;
}

int CryStats(char* buffer)
{
	ReadLock lock(g_lockMemMan);

	const int currentAllocated = 0;
	const int totalFree = 0;
	const int maximumFree = 0;
	const int allocationCount = 0;
	const int freeCount = 0;
	const int poolSize = CryMemoryGetPoolSize();

	if (buffer)
	{
		sprintf(
			buffer,
			"Memory Allocated = %d K, totfree = %d K , maxfree = %d K, "
			"nmalloc = %d, nfree = %d, biggestalloc = %u, Pool Size = %d K",
			currentAllocated / 1024,
			totalFree / 1024,
			maximumFree / 1024,
			allocationCount,
			freeCount,
			g_biggestAllocation,
			poolSize / 1024);
	}

	return currentAllocated / 1024;
}

uint64 CryGetUsedHeapSize64()
{
	return static_cast<uint64>(g_GlobPageBucketAllocator.get_heap_size());
}

uint64 CryGetWastedHeapSize64()
{
	return static_cast<uint64>(g_GlobPageBucketAllocator.get_wasted_in_allocation());
}

int CryGetUsedHeapSize()
{
	const uint64 bytes = CryGetUsedHeapSize64();
	const uint64 maximum = static_cast<uint64>(std::numeric_limits<int>::max());
	return bytes > maximum ? std::numeric_limits<int>::max() : static_cast<int>(bytes);
}

int CryGetWastedHeapSize()
{
	const uint64 bytes = CryGetWastedHeapSize64();
	const uint64 maximum = static_cast<uint64>(std::numeric_limits<int>::max());
	return bytes > maximum ? std::numeric_limits<int>::max() : static_cast<int>(bytes);
}

void CryCleanup()
{
	WriteLock lock(g_lockMemMan);
	g_GlobPageBucketAllocator.cleanup();
}

void* CrySystemCrtMalloc(size_t size)
{
	return ::malloc(size);
}

void CrySystemCrtFree(void* memory)
{
	::free(memory);
}

size_t CrySystemCrtSize(void* memory)
{
	return _msize(memory);
}
