#include <CryMemoryAllocator.h>

#include "MemoryAllocatorTests.h"
#include "TestRunner.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

static_assert(sizeof(void*) == 8, "CryCommonTests targets the Win64 runtime data model.");
static_assert(sizeof(long) == 4, "MM01 depends on the Windows LLP64 data model.");
static_assert(sizeof(uintptr_t) == sizeof(void*), "uintptr_t must preserve a complete pointer value.");

using MM01TraceAllocator = node_alloc<eCryDefaultMalloc, true, 64 * 1024>;

using LockedAllocator = node_alloc < eCryDefaultMalloc, true, 64 * 1024>;
using UnlockedAllocator = node_alloc < eCryDefaultMalloc, false, 64 * 1024>;

static void TestWin64PointerDataModel(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, sizeof(void*) == 8);
	CRY_TEST_CHECK(runner, sizeof(long) == 4);
	CRY_TEST_CHECK(runner, sizeof(uintptr_t) == sizeof(void*));
}

void RunMemoryAllocatorDataModelTests(CryTestRunner& runner)
{
	runner.Run("CryMemoryAllocator uses the Win64 LLP64 data model", TestWin64PointerDataModel);
}

int RunMM01AllocatorTrace()
{
	constexpr size_t requestedSize = 33;
	constexpr size_t roundedSize = (requestedSize + _ALIGN - 1) & ~static_cast<size_t>(_ALIGN - 1);
	constexpr size_t freeListIndex = (requestedSize - 1) >> _ALIGN_SHIFT;

	std::printf("MM01 node_alloc trace\n");
	std::printf("  pointer width: %zu bits\n", sizeof(void*) * 8);
	std::printf("  long width:    %zu bits\n", sizeof(long) * 8);
	std::printf("  request:       %zu bytes\n", requestedSize);
	std::printf("  size class:    %zu bytes (free list %zu)\n", roundedSize, freeListIndex);
	std::printf("  exercising the repaired first small-allocation path...\n");
	std::fflush(stdout);


	void* const allocation = MM01TraceAllocator::alloc(requestedSize);
	if (allocation == nullptr)
	{
		std::printf("  allocation returned nullptr\n");
		return 1;
	}

	const uintptr_t address = reinterpret_cast<uintptr_t>(allocation);
	std::printf("  allocation:    %p\n", allocation);
	std::printf("  upper 32 bits: 0x%08llX\n", static_cast<unsigned long long>(address >> 32));

	std::memset(allocation, 0xA5, requestedSize);
	MM01TraceAllocator::dealloc(allocation, requestedSize);
	std::printf("  allocation, write, and deallocation completed\n");
	return 0;
}
