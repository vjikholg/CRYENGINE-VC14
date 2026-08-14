#include "MemoryAccountingTests.h"
#include "TestRunner.h"

#include <CryMemoryByteCounter.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <atomic>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>

typedef void* (__cdecl* MM04CryMallocFunction)(size_t size, size_t& allocated);
typedef void* (__cdecl* MM04CryReallocFunction)(void* memory, size_t size, size_t& allocated);
typedef size_t (__cdecl* MM04CryFreeFunction)(void* memory);
typedef int (__cdecl* MM04LegacyMemorySizeFunction)();
typedef uint64_t (__cdecl* MM04MemorySize64Function)();

struct MM04RuntimeApi
{
	HMODULE module;
	MM04CryMallocFunction mallocFunction;
	MM04CryReallocFunction reallocFunction;
	MM04CryFreeFunction freeFunction;
	MM04LegacyMemorySizeFunction legacyAllocatedSizeFunction;
	MM04MemorySize64Function allocatedSize64Function;
	MM04MemorySize64Function usedHeapSize64Function;
	MM04MemorySize64Function wastedHeapSize64Function;
};

struct MM04AllocationRecord
{
	void* memory;
	size_t allocatedSize;
};

static_assert(sizeof(void*) == 8, "MM04 tests require the Win64 data model.");
static_assert(sizeof(size_t) == 8, "MM04 byte accounting must accept 64-bit sizes.");
static_assert(sizeof(long) == 4, "Windows x64 uses the LLP64 data model.");

static MM04RuntimeApi* g_mm04RuntimeApi = nullptr;

static void TestMM04ByteCounterExactBalance(CryTestRunner& runner)
{
	CryMemoryByteCounter counter;
	counter.Add(64);
	counter.Add(128);

	CRY_TEST_CHECK(runner, counter.Load() == 192);
	counter.Remove(64);
	CRY_TEST_CHECK(runner, counter.Load() == 128);
	counter.Remove(128);
	CRY_TEST_CHECK(runner, counter.Load() == 0);
}

static void TestMM04ByteCounter64BitRange(CryTestRunner& runner)
{
	CryMemoryByteCounter counter;
	constexpr size_t largeAllocation = (size_t{1} << 32) + 4096;

	counter.Add(largeAllocation);
	CRY_TEST_CHECK(runner, counter.Load() == largeAllocation);
	CRY_TEST_CHECK(runner, counter.LoadLegacyInt() == INT32_MAX);
	CRY_TEST_CHECK(runner, counter.TryRemove(largeAllocation));
	CRY_TEST_CHECK(runner, counter.Load() == 0);
}

static void TestMM04ByteCounterRejectsUnderflow(CryTestRunner& runner)
{
	CryMemoryByteCounter counter;
	counter.Add(16);

	CRY_TEST_CHECK(runner, !counter.TryRemove(17));
	CRY_TEST_CHECK(runner, counter.Load() == 16);
	CRY_TEST_CHECK(runner, counter.TryRemove(16));
	CRY_TEST_CHECK(runner, counter.Load() == 0);
}

static void TestMM04ByteCounterConcurrency(CryTestRunner& runner)
{
	constexpr size_t threadCount = 8;
	constexpr size_t operationsPerThread = 4096;
	constexpr size_t operationSize = 17;
	CryMemoryByteCounter counter;
	std::vector<std::thread> workers;
	workers.reserve(threadCount);

	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
	{
		workers.emplace_back([&counter]() {
			for (size_t operation = 0; operation < operationsPerThread; ++operation)
			{
				counter.Add(operationSize);
			}
		});
	}

	for (std::thread& worker : workers)
	{
		worker.join();
	}

	const uint64_t expected =
		static_cast<uint64_t>(threadCount) * operationsPerThread * operationSize;
	CRY_TEST_CHECK(runner, counter.Load() == expected);

	workers.clear();
	std::atomic<bool> removalFailed{false};
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
	{
		workers.emplace_back([&counter, &removalFailed]() {
			for (size_t operation = 0; operation < operationsPerThread; ++operation)
			{
				if (!counter.TryRemove(operationSize))
				{
					removalFailed.store(true, std::memory_order_relaxed);
				}
			}
		});
	}

	for (std::thread& worker : workers)
	{
		worker.join();
	}

	CRY_TEST_CHECK(runner, !removalFailed.load(std::memory_order_relaxed));
	CRY_TEST_CHECK(runner, counter.Load() == 0);
}

static void TestMM04SignedLiveByteBoundary(CryTestRunner& runner)
{
	constexpr uint32_t initialBits = 0x7ffff000u;
	constexpr uint32_t allocationSize = 0x2000u;
	constexpr uint32_t wrappedBits = initialBits + allocationSize;
	constexpr int32_t legacyResult = std::bit_cast<int32_t>(wrappedBits);
	constexpr uint64_t expectedResult =
		static_cast<uint64_t>(initialBits) + allocationSize;

	CRY_TEST_CHECK(runner, expectedResult > static_cast<uint64_t>(INT32_MAX));
	CRY_TEST_CHECK(runner, legacyResult < 0);
	CRY_TEST_CHECK(runner, static_cast<uint64_t>(wrappedBits) == expectedResult);
}

static void TestMM04SizeDeltaNarrowing(CryTestRunner& runner)
{
	constexpr uint64_t allocationSize = (uint64_t{1} << 32) + 4096;
	constexpr uint32_t narrowedAllocationSize = static_cast<uint32_t>(allocationSize);

	CRY_TEST_CHECK(runner, narrowedAllocationSize == 4096);
	CRY_TEST_CHECK(runner, static_cast<uint64_t>(narrowedAllocationSize) != allocationSize);
}

static void TestMM04FreeDeltaNegationBoundary(CryTestRunner& runner)
{
	constexpr uint32_t allocationBits = 0x80000000u;
	constexpr int32_t narrowedAllocationSize = std::bit_cast<int32_t>(allocationBits);
	constexpr int64_t desiredFreeDelta = -static_cast<int64_t>(allocationBits);
	constexpr int64_t negatedNarrowedValue = -static_cast<int64_t>(narrowedAllocationSize);

	CRY_TEST_CHECK(runner, narrowedAllocationSize == INT32_MIN);
	CRY_TEST_CHECK(runner, desiredFreeDelta == INT32_MIN);
	CRY_TEST_CHECK(runner, negatedNarrowedValue > INT32_MAX);
	CRY_TEST_CHECK(runner, negatedNarrowedValue != desiredFreeDelta);
}

static void TestMM04LegacyStatisticsRange(CryTestRunner& runner)
{
	constexpr uint64_t threeGiB = uint64_t{3} * 1024 * 1024 * 1024;
	constexpr uint64_t fiveGiB = uint64_t{5} * 1024 * 1024 * 1024;
	constexpr int32_t signedThreeGiB =
		std::bit_cast<int32_t>(static_cast<uint32_t>(threeGiB));
	constexpr uint32_t unsignedFiveGiB = static_cast<uint32_t>(fiveGiB);

	CRY_TEST_CHECK(runner, signedThreeGiB < 0);
	CRY_TEST_CHECK(runner, static_cast<uint64_t>(unsignedFiveGiB) != fiveGiB);
	CRY_TEST_CHECK(
		runner,
		static_cast<uint64_t>(INT32_MAX) < threeGiB);
}

static void TestMM04NodeMetricSignedConversion(CryTestRunner& runner)
{
	constexpr uint64_t expectedHeapSize = uint64_t{2} * 1024 * 1024 * 1024;
	constexpr int32_t wrappedNodeMetric =
		std::bit_cast<int32_t>(static_cast<uint32_t>(expectedHeapSize));
	constexpr size_t reportedHeapSize = static_cast<size_t>(wrappedNodeMetric);

	CRY_TEST_CHECK(runner, wrappedNodeMetric == INT32_MIN);
	CRY_TEST_CHECK(runner, reportedHeapSize != expectedHeapSize);
	CRY_TEST_CHECK(runner, reportedHeapSize > UINT32_MAX);
}

static void TestMM04NonAtomicLostUpdateModel(CryTestRunner& runner)
{
	constexpr uint64_t firstAllocation = 64;
	constexpr uint64_t secondAllocation = 128;
	uint64_t sharedCounter = 0;

	const uint64_t firstThreadRead = sharedCounter;
	const uint64_t secondThreadRead = sharedCounter;
	sharedCounter = firstThreadRead + firstAllocation;
	sharedCounter = secondThreadRead + secondAllocation;

	CRY_TEST_CHECK(runner, sharedCounter == secondAllocation);
	CRY_TEST_CHECK(runner, sharedCounter != firstAllocation + secondAllocation);
}

static bool MM04LoadRuntimeApi(const wchar_t* crySystemPath, MM04RuntimeApi& api)
{
	api = {};
	api.module = LoadLibraryW(crySystemPath);
	if (api.module == nullptr)
	{
		std::printf(
			"Unable to load CrySystem DLL '%ls' (Windows error %lu).\n",
			crySystemPath,
			GetLastError());
		return false;
	}

	api.mallocFunction = reinterpret_cast<MM04CryMallocFunction>(
		GetProcAddress(api.module, "CryMalloc"));
	api.reallocFunction = reinterpret_cast<MM04CryReallocFunction>(
		GetProcAddress(api.module, "CryRealloc"));
	api.freeFunction = reinterpret_cast<MM04CryFreeFunction>(
		GetProcAddress(api.module, "CryFree"));
	api.legacyAllocatedSizeFunction = reinterpret_cast<MM04LegacyMemorySizeFunction>(
		GetProcAddress(api.module, "CryMemoryGetAllocatedSize"));
	if (api.legacyAllocatedSizeFunction == nullptr)
	{
		api.legacyAllocatedSizeFunction = reinterpret_cast<MM04LegacyMemorySizeFunction>(
			GetProcAddress(api.module, "?CryMemoryGetAllocatedSize@@YAHXZ"));
	}
	api.allocatedSize64Function = reinterpret_cast<MM04MemorySize64Function>(
		GetProcAddress(api.module, "CryMemoryGetAllocatedSize64"));
	api.usedHeapSize64Function = reinterpret_cast<MM04MemorySize64Function>(
		GetProcAddress(api.module, "CryGetUsedHeapSize64"));
	api.wastedHeapSize64Function = reinterpret_cast<MM04MemorySize64Function>(
		GetProcAddress(api.module, "CryGetWastedHeapSize64"));

	if (api.mallocFunction == nullptr || api.reallocFunction == nullptr ||
		api.freeFunction == nullptr)
	{
		std::printf("CrySystem does not export the base MM04 allocation API.\n");
		FreeLibrary(api.module);
		api = {};
		return false;
	}

	return true;
}

static bool MM04Has64BitAccountingApi(const MM04RuntimeApi& api)
{
	return api.allocatedSize64Function != nullptr &&
		api.usedHeapSize64Function != nullptr &&
		api.wastedHeapSize64Function != nullptr;
}

static void TestMM04Runtime64BitExports(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, g_mm04RuntimeApi->allocatedSize64Function != nullptr);
	CRY_TEST_CHECK(runner, g_mm04RuntimeApi->usedHeapSize64Function != nullptr);
	CRY_TEST_CHECK(runner, g_mm04RuntimeApi->wastedHeapSize64Function != nullptr);
}

static void TestMM04RuntimeAllocationDelta(CryTestRunner& runner)
{
	constexpr size_t requestedSize = 4096;
	const uint64_t baseline = g_mm04RuntimeApi->allocatedSize64Function();
	size_t allocatedSize = 0;
	void* const memory = g_mm04RuntimeApi->mallocFunction(requestedSize, allocatedSize);

	CRY_TEST_CHECK(runner, memory != nullptr);
	if (memory == nullptr)
	{
		return;
	}

	CRY_TEST_CHECK(
		runner,
		g_mm04RuntimeApi->allocatedSize64Function() == baseline + allocatedSize);
	CRY_TEST_CHECK(runner, g_mm04RuntimeApi->freeFunction(memory) == allocatedSize);
	CRY_TEST_CHECK(runner, g_mm04RuntimeApi->allocatedSize64Function() == baseline);
}

static void TestMM04RuntimeReallocationDelta(CryTestRunner& runner)
{
	constexpr size_t initialSize = 64;
	constexpr size_t grownSize = 8192;
	constexpr size_t shrunkSize = 32;
	const uint64_t baseline = g_mm04RuntimeApi->allocatedSize64Function();
	size_t initialAllocatedSize = 0;
	void* memory = g_mm04RuntimeApi->mallocFunction(initialSize, initialAllocatedSize);

	CRY_TEST_CHECK(runner, memory != nullptr);
	if (memory == nullptr)
	{
		return;
	}

	size_t grownAllocatedSize = 0;
	void* const grownMemory = g_mm04RuntimeApi->reallocFunction(
		memory,
		grownSize,
		grownAllocatedSize);
	CRY_TEST_CHECK(runner, grownMemory != nullptr);
	if (grownMemory == nullptr)
	{
		g_mm04RuntimeApi->freeFunction(memory);
		return;
	}

	CRY_TEST_CHECK(
		runner,
		g_mm04RuntimeApi->allocatedSize64Function() ==
			baseline + grownAllocatedSize);

	size_t shrunkAllocatedSize = 0;
	void* const shrunkMemory = g_mm04RuntimeApi->reallocFunction(
		grownMemory,
		shrunkSize,
		shrunkAllocatedSize);
	CRY_TEST_CHECK(runner, shrunkMemory != nullptr);
	if (shrunkMemory == nullptr)
	{
		g_mm04RuntimeApi->freeFunction(grownMemory);
		return;
	}

	CRY_TEST_CHECK(
		runner,
		g_mm04RuntimeApi->allocatedSize64Function() ==
			baseline + shrunkAllocatedSize);
	g_mm04RuntimeApi->freeFunction(shrunkMemory);
	CRY_TEST_CHECK(runner, g_mm04RuntimeApi->allocatedSize64Function() == baseline);
}

static void TestMM04RuntimeConcurrentAccounting(CryTestRunner& runner)
{
	constexpr size_t threadCount = 8;
	constexpr size_t allocationsPerThread = 512;
	const uint64_t baseline = g_mm04RuntimeApi->allocatedSize64Function();
	std::vector<MM04AllocationRecord> allocations(
		threadCount * allocationsPerThread,
		MM04AllocationRecord{ nullptr, 0 });
	std::atomic<bool> allocationFailed{ false };
	std::vector<std::thread> workers;
	workers.reserve(threadCount);

	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
	{
		workers.emplace_back([threadIndex, &allocations, &allocationFailed]() {
			const size_t firstRecord = threadIndex * allocationsPerThread;
			for (size_t index = 0; index < allocationsPerThread; ++index)
			{
				MM04AllocationRecord& record = allocations[firstRecord + index];
				const size_t requestedSize = 32 + ((threadIndex + index) % 257);
				record.memory =
					g_mm04RuntimeApi->mallocFunction(requestedSize, record.allocatedSize);
				if (record.memory == nullptr)
				{
					allocationFailed.store(true, std::memory_order_relaxed);
				}
			}
		});
	}

	for (std::thread& worker : workers)
	{
		worker.join();
	}

	uint64_t expectedAllocationDelta = 0;
	for (const MM04AllocationRecord& record : allocations)
	{
		expectedAllocationDelta += record.allocatedSize;
	}

	CRY_TEST_CHECK(runner, !allocationFailed.load(std::memory_order_relaxed));
	CRY_TEST_CHECK(
		runner,
		g_mm04RuntimeApi->allocatedSize64Function() ==
			baseline + expectedAllocationDelta);

	workers.clear();
	std::atomic<bool> freeSizeMismatch{ false };
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
	{
		workers.emplace_back([threadIndex, &allocations, &freeSizeMismatch]() {
			const size_t firstRecord = threadIndex * allocationsPerThread;
			for (size_t index = 0; index < allocationsPerThread; ++index)
			{
				const MM04AllocationRecord& record = allocations[firstRecord + index];
				if (record.memory != nullptr &&
					g_mm04RuntimeApi->freeFunction(record.memory) != record.allocatedSize)
				{
					freeSizeMismatch.store(true, std::memory_order_relaxed);
				}
			}
		});
	}

	for (std::thread& worker : workers)
	{
		worker.join();
	}

	CRY_TEST_CHECK(runner, !freeSizeMismatch.load(std::memory_order_relaxed));
	CRY_TEST_CHECK(runner, g_mm04RuntimeApi->allocatedSize64Function() == baseline);
}

static void TestMM04RuntimeLegacyCompatibility(CryTestRunner& runner)
{
	const uint64_t modernValue = g_mm04RuntimeApi->allocatedSize64Function();
	const int legacyValue = g_mm04RuntimeApi->legacyAllocatedSizeFunction();
	const int expectedLegacyValue = modernValue > static_cast<uint64_t>(INT32_MAX)
		? INT32_MAX
		: static_cast<int>(modernValue);

	CRY_TEST_CHECK(runner, legacyValue == expectedLegacyValue);
}

static void TestMM04RuntimeHeapStatistics(CryTestRunner& runner)
{
	const uint64_t usedBefore = g_mm04RuntimeApi->usedHeapSize64Function();
	size_t allocatedSize = 0;
	void* const memory = g_mm04RuntimeApi->mallocFunction(1024, allocatedSize);

	CRY_TEST_CHECK(runner, memory != nullptr);
	if (memory == nullptr)
	{
		return;
	}

	const uint64_t usedDuring = g_mm04RuntimeApi->usedHeapSize64Function();
	const uint64_t wastedDuring = g_mm04RuntimeApi->wastedHeapSize64Function();
	CRY_TEST_CHECK(runner, usedDuring >= usedBefore);
	CRY_TEST_CHECK(runner, wastedDuring <= usedDuring || usedDuring == 0);

	g_mm04RuntimeApi->freeFunction(memory);
	const uint64_t usedAfter = g_mm04RuntimeApi->usedHeapSize64Function();
	const uint64_t wastedAfter = g_mm04RuntimeApi->wastedHeapSize64Function();
	CRY_TEST_CHECK(runner, usedAfter <= usedDuring);
	CRY_TEST_CHECK(runner, wastedAfter <= usedAfter || usedAfter == 0);
}

void RunMM04DataModelTests(CryTestRunner& runner)
{
	runner.Run("MM04 byte counter keeps exact balances", TestMM04ByteCounterExactBalance);
	runner.Run("MM04 byte counter supports 64-bit totals", TestMM04ByteCounter64BitRange);
	runner.Run("MM04 byte counter rejects underflow", TestMM04ByteCounterRejectsUnderflow);
	runner.Run("MM04 byte counter is concurrency-safe", TestMM04ByteCounterConcurrency);
	runner.Run("MM04 model detects signed live-byte overflow", TestMM04SignedLiveByteBoundary);
	runner.Run("MM04 model detects size_t delta truncation", TestMM04SizeDeltaNarrowing);
	runner.Run("MM04 model detects INT32_MIN free negation", TestMM04FreeDeltaNegationBoundary);
	runner.Run("MM04 model detects legacy statistics range", TestMM04LegacyStatisticsRange);
	runner.Run("MM04 model detects node metric sign extension", TestMM04NodeMetricSignedConversion);
	runner.Run("MM04 model detects non-atomic lost update", TestMM04NonAtomicLostUpdateModel);
}

int RunMM04DataModelTrace()
{
	CryTestRunner runner;
	RunMM04DataModelTests(runner);
	return runner.ExitCode();
}

int RunMM04AccountingTrace(const wchar_t* crySystemPath)
{
	if (crySystemPath == nullptr || crySystemPath[0] == L'\0')
	{
		std::printf("MM04 runtime trace requires a CrySystem DLL path.\n");
		return 3;
	}

	MM04RuntimeApi api = {};
	if (!MM04LoadRuntimeApi(crySystemPath, api))
	{
		return 3;
	}

	g_mm04RuntimeApi = &api;
	CryTestRunner runner;
	RunMM04DataModelTests(runner);
	runner.Run("MM04 runtime exposes 64-bit accounting", TestMM04Runtime64BitExports);

	if (MM04Has64BitAccountingApi(api))
	{
		runner.Run("MM04 runtime allocation/free delta", TestMM04RuntimeAllocationDelta);
		runner.Run("MM04 runtime reallocation delta", TestMM04RuntimeReallocationDelta);
		runner.Run("MM04 runtime concurrent accounting", TestMM04RuntimeConcurrentAccounting);
		if (api.legacyAllocatedSizeFunction != nullptr)
		{
			runner.Run("MM04 legacy accounting API compatibility", TestMM04RuntimeLegacyCompatibility);
		}
		else
		{
			std::printf("  Legacy allocated-size export not found; compatibility check skipped.\n");
		}
		runner.Run("MM04 runtime 64-bit heap statistics", TestMM04RuntimeHeapStatistics);
	}
	else
	{
		std::printf(
			"  Runtime accounting checks skipped until CrySystem exports the MM04 64-bit API.\n");
	}

	const int exitCode = runner.ExitCode();
	g_mm04RuntimeApi = nullptr;
	FreeLibrary(api.module);
	return exitCode;
}
