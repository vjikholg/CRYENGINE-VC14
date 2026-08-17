#include <CryMemoryAllocator.h>

#include "MemoryAllocatorTests.h"
#include "TestRunner.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <thread>
#include <vector>

static_assert(sizeof(void*) == 8, "CryCommonTests targets the Win64 runtime data model.");
static_assert(sizeof(long) == 4, "MM01 depends on the Windows LLP64 data model.");
static_assert(sizeof(uintptr_t) == sizeof(void*), "uintptr_t must preserve a complete pointer value.");
static_assert(_ALIGN == 16, "MM01 tests expect the active Win64 node allocator alignment.");
static_assert(NFREELISTS * _ALIGN == _MAX_BYTES, "Every Win64 node allocator size class must be represented.");

using MM01LockedSizeClassAllocator = node_alloc<eCryDefaultMalloc, true, 768 * 1024>;
using MM01SingleThreadSizeClassAllocator = node_alloc<eCryDefaultMalloc, false, 832 * 1024>;
using MM01ReuseAllocator = node_alloc<eCryDefaultMalloc, true, 896 * 1024>;
using MM01ConcurrentAllocator = node_alloc<eCryDefaultMalloc, true, 1024 * 1024>;

struct MM01LiveAllocation
{
	void* memory;
	size_t size;
	unsigned char pattern;
};

static bool MM01HasUpperPointerBits(const void* memory)
{
	return (reinterpret_cast<uintptr_t>(memory) >> 32) != 0;
}

static bool MM01IsAligned(const void* memory)
{
	return reinterpret_cast<uintptr_t>(memory) % _ALIGN == 0;
}

static unsigned char MM01MakePattern(size_t sequence, size_t size)
{
	return static_cast<unsigned char>(0x31u + sequence * 29u + size * 17u);
}

static bool MM01VerifyPattern(const void* memory, size_t size, unsigned char pattern)
{
	const unsigned char* const bytes = static_cast<const unsigned char*>(memory);
	for (size_t index = 0; index < size; ++index)
	{
		if (bytes[index] != pattern)
		{
			return false;
		}
	}

	return true;
}

static uint64_t MM01NextRandom(uint64_t& state)
{
	state ^= state << 13;
	state ^= state >> 7;
	state ^= state << 17;
	return state;
}

static void TestWin64PointerDataModel(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, sizeof(void*) == 8);
	CRY_TEST_CHECK(runner, sizeof(long) == 4);
	CRY_TEST_CHECK(runner, sizeof(uintptr_t) == sizeof(void*));
}

template<class Allocator>
static bool MM01ExerciseEverySizeClass()
{
	bool allAllocationsSucceeded = true;
	bool allAddressesAreHigh = true;
	bool allAddressesAreAligned = true;
	bool allPayloadsAreIntact = true;

	for (size_t sizeClass = 0; sizeClass < NFREELISTS; ++sizeClass)
	{
		const std::array<size_t, 2> requestedSizes = {
			sizeClass * _ALIGN + 1,
			(sizeClass + 1) * _ALIGN
		};

		for (size_t variant = 0; variant < requestedSizes.size(); ++variant)
		{
			const size_t requestedSize = requestedSizes[variant];
			void* const memory = Allocator::alloc(requestedSize);
			if (memory == nullptr)
			{
				allAllocationsSucceeded = false;
				continue;
			}

			allAddressesAreHigh = allAddressesAreHigh && MM01HasUpperPointerBits(memory);
			allAddressesAreAligned = allAddressesAreAligned && MM01IsAligned(memory);

			const unsigned char pattern = MM01MakePattern(sizeClass * 2 + variant, requestedSize);
			std::memset(memory, pattern, requestedSize);
			allPayloadsAreIntact = allPayloadsAreIntact &&
				MM01VerifyPattern(memory, requestedSize, pattern);
			Allocator::dealloc(memory, requestedSize);
		}
	}

	return allAllocationsSucceeded &&
		allAddressesAreHigh &&
		allAddressesAreAligned &&
		allPayloadsAreIntact;
}

static void TestMM01LockedPolicySizeClasses(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, MM01ExerciseEverySizeClass<MM01LockedSizeClassAllocator>());
}

static void TestMM01SingleThreadPolicySizeClasses(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, MM01ExerciseEverySizeClass<MM01SingleThreadSizeClassAllocator>());
}

static void TestMM01FreeListReusePreservesPointers(CryTestRunner& runner)
{
	constexpr size_t requestedSize = 33;
	constexpr size_t allocationCount = 256;

	std::vector<void*> firstPass;
	firstPass.reserve(allocationCount);

	bool firstPassSucceeded = true;
	for (size_t index = 0; index < allocationCount; ++index)
	{
		void* const memory = MM01ReuseAllocator::alloc(requestedSize);
		if (memory == nullptr)
		{
			firstPassSucceeded = false;
			break;
		}

		const unsigned char pattern = MM01MakePattern(index, requestedSize);
		std::memset(memory, pattern, requestedSize);
		firstPassSucceeded = firstPassSucceeded &&
			MM01HasUpperPointerBits(memory) &&
			MM01IsAligned(memory) &&
			MM01VerifyPattern(memory, requestedSize, pattern);
		firstPass.push_back(memory);
	}

	for (auto allocation = firstPass.rbegin(); allocation != firstPass.rend(); ++allocation)
	{
		MM01ReuseAllocator::dealloc(*allocation, requestedSize);
	}

	std::vector<void*> secondPass;
	secondPass.reserve(firstPass.size());
	bool onlyFreedPointersWereReused = firstPassSucceeded && firstPass.size() == allocationCount;
	bool noPointerWasReturnedTwice = true;

	for (size_t index = 0; index < firstPass.size(); ++index)
	{
		void* const memory = MM01ReuseAllocator::alloc(requestedSize);
		if (memory == nullptr)
		{
			onlyFreedPointersWereReused = false;
			break;
		}

		onlyFreedPointersWereReused = onlyFreedPointersWereReused &&
			std::find(firstPass.begin(), firstPass.end(), memory) != firstPass.end();
		noPointerWasReturnedTwice = noPointerWasReturnedTwice &&
			std::find(secondPass.begin(), secondPass.end(), memory) == secondPass.end();
		secondPass.push_back(memory);
	}

	for (void* memory : secondPass)
	{
		MM01ReuseAllocator::dealloc(memory, requestedSize);
	}

	CRY_TEST_CHECK(runner, firstPassSucceeded);
	CRY_TEST_CHECK(runner, firstPass.size() == allocationCount);
	CRY_TEST_CHECK(runner, secondPass.size() == allocationCount);
	CRY_TEST_CHECK(runner, onlyFreedPointersWereReused);
	CRY_TEST_CHECK(runner, noPointerWasReturnedTwice);
}

static void MM01RunConcurrentWorker(
	size_t workerIndex,
	std::atomic<bool>& succeeded,
	std::atomic<bool>& observedHighAddress)
{
	constexpr size_t activeAllocationCount = 64;
	constexpr size_t iterationCount = 20000;

	std::array<MM01LiveAllocation, activeAllocationCount> allocations = {};
	uint64_t randomState = 0x9E3779B97F4A7C15ull ^
		(static_cast<uint64_t>(workerIndex + 1) * 0xBF58476D1CE4E5B9ull);

	for (size_t iteration = 0; iteration < iterationCount; ++iteration)
	{
		const size_t slot = static_cast<size_t>(MM01NextRandom(randomState) % allocations.size());
		MM01LiveAllocation& live = allocations[slot];

		if (live.memory != nullptr)
		{
			if (!MM01VerifyPattern(live.memory, live.size, live.pattern))
			{
				succeeded.store(false, std::memory_order_relaxed);
			}

			MM01ConcurrentAllocator::dealloc(live.memory, live.size);
			live = {};
		}

		const size_t requestedSize =
			1 + static_cast<size_t>(MM01NextRandom(randomState) % _MAX_BYTES);
		void* const memory = MM01ConcurrentAllocator::alloc(requestedSize);
		if (memory == nullptr)
		{
			succeeded.store(false, std::memory_order_relaxed);
			break;
		}

		if (!MM01IsAligned(memory))
		{
			succeeded.store(false, std::memory_order_relaxed);
		}
		if (MM01HasUpperPointerBits(memory))
		{
			observedHighAddress.store(true, std::memory_order_relaxed);
		}

		const unsigned char pattern = MM01MakePattern(
			workerIndex * iterationCount + iteration,
			requestedSize);
		std::memset(memory, pattern, requestedSize);
		live = { memory, requestedSize, pattern };
	}

	for (MM01LiveAllocation& live : allocations)
	{
		if (live.memory == nullptr)
		{
			continue;
		}

		if (!MM01VerifyPattern(live.memory, live.size, live.pattern))
		{
			succeeded.store(false, std::memory_order_relaxed);
		}
		MM01ConcurrentAllocator::dealloc(live.memory, live.size);
	}
}

static void TestMM01ConcurrentPayloadIntegrity(CryTestRunner& runner)
{
	constexpr size_t workerCount = 8;
	std::atomic<bool> succeeded(true);
	std::atomic<bool> observedHighAddress(false);
	std::array<std::thread, workerCount> workers;

	for (size_t workerIndex = 0; workerIndex < workers.size(); ++workerIndex)
	{
		workers[workerIndex] = std::thread(
			MM01RunConcurrentWorker,
			workerIndex,
			std::ref(succeeded),
			std::ref(observedHighAddress));
	}

	for (std::thread& worker : workers)
	{
		worker.join();
	}

	CRY_TEST_CHECK(runner, succeeded.load(std::memory_order_relaxed));
	CRY_TEST_CHECK(runner, observedHighAddress.load(std::memory_order_relaxed));
}

void RunMemoryAllocatorDataModelTests(CryTestRunner& runner)
{
	runner.Run("MM01 uses the Win64 LLP64 data model", TestWin64PointerDataModel);
	runner.Run("MM01 locked policy covers every small-allocation size class", TestMM01LockedPolicySizeClasses);
	runner.Run("MM01 single-thread policy covers every small-allocation size class", TestMM01SingleThreadPolicySizeClasses);
	runner.Run("MM01 free-list reuse preserves complete pointer values", TestMM01FreeListReusePreservesPointers);
	runner.Run("MM01 concurrent allocation preserves payloads and pointers", TestMM01ConcurrentPayloadIntegrity);
}

int RunMM01AllocatorTrace()
{
	std::printf("MM01 Win64 node allocator validation\n");
	std::printf("  pointer width: %zu bits\n", sizeof(void*) * 8);
	std::printf("  long width:    %zu bits\n", sizeof(long) * 8);
	std::printf("  size classes:  %d (%d-byte alignment, up to %d bytes)\n", NFREELISTS, _ALIGN, _MAX_BYTES);
	std::printf("  stress load:   8 workers x 20000 allocation cycles\n\n");

	CryTestRunner runner;
	RunMemoryAllocatorDataModelTests(runner);
	return runner.ExitCode();
}
