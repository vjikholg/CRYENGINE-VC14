#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <platform.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CrySafeAlloc_impl.h>

#include "MM03BenchmarkEnvironment.h"
#include "SafeAllocTests.h"
#include "TestRunner.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <intrin.h>
#include <limits>
#include <vector>

using MM03SafeAllocFunction = void* (*)(TPFAlloc, void*, size_t, size_t);

struct MM03AllocatorProbe
{
	void* lastPointer;
	size_t lastSize;
	int callCount;
	int allocationCount;
	int reallocationCount;
	int freeCount;
};

struct alignas(16) MM03AlignedValue
{
	uint64_t values[8];
};

struct MM03BenchmarkResult
{
	double averageTicks;
	double deviationTicks;
	double medianTicks;
	double percentile95Ticks;
	double trimmedMeanTicks;
	bool completed;
};

struct MM03BenchmarkClock
{
	double ticksPerSecond;
	double nanosecondsPerTick;
	bool supported;
};

constexpr size_t MM03BenchmarkIterationCount = 1024;
constexpr size_t MM03BenchmarkRoundCount = 24;

struct MM03BenchmarkSeries
{
	std::vector<double> samples;
	std::array<double, MM03BenchmarkRoundCount> roundAverages;
	bool completed;
};

struct MM03BenchmarkComparison
{
	double averageDifferenceTicks;
	double deviationDifferenceTicks;
	double confidenceLowTicks;
	double confidenceHighTicks;
	size_t constexprWinCount;
	bool completed;
};

static MM03AllocatorProbe g_mm03AllocatorA = {};
static MM03AllocatorProbe g_mm03AllocatorB = {};

static void MM03ResetProbe(MM03AllocatorProbe& probe)
{
	probe = MM03AllocatorProbe{};
}

static void* MM03ProbeAllocate(MM03AllocatorProbe& probe, void* pointer, size_t size)
{
	++probe.callCount;
	probe.lastPointer = pointer;
	probe.lastSize = size;

	if (!size)
	{
		if (pointer)
		{
			++probe.freeCount;
			std::free(pointer);
		}
		return nullptr;
	}

	if (pointer)
	{
		++probe.reallocationCount;
		return std::realloc(pointer, size);
	}

	++probe.allocationCount;
	return std::malloc(size);
}

static void* MM03AllocatorA(void* pointer, size_t size)
{
	return MM03ProbeAllocate(g_mm03AllocatorA, pointer, size);
}

static void* MM03AllocatorB(void* pointer, size_t size)
{
	return MM03ProbeAllocate(g_mm03AllocatorB, pointer, size);
}

static bool MM03IsAligned(const void* memory, size_t alignment)
{
	return reinterpret_cast<uintptr_t>(memory) % alignment == 0;
}

static void MM03CheckMetadataPolicy(
	CryTestRunner& runner,
	MM03SafeAllocFunction safeAlloc)
{
	MM03ResetProbe(g_mm03AllocatorA);
	MM03ResetProbe(g_mm03AllocatorB);

	void* const overflow = safeAlloc(
		MM03AllocatorA,
		nullptr,
		(std::numeric_limits<size_t>::max)(),
		alignof(std::max_align_t));
	CRY_TEST_CHECK(runner, overflow == nullptr);
	CRY_TEST_CHECK(runner, g_mm03AllocatorA.callCount == 0);

	void* const invalidAlignment = safeAlloc(MM03AllocatorA, nullptr, 32, 24);
	CRY_TEST_CHECK(runner, invalidAlignment == nullptr);
	CRY_TEST_CHECK(runner, g_mm03AllocatorA.callCount == 0);

	void* memory = safeAlloc(MM03AllocatorA, nullptr, 32, 64);
	CRY_TEST_CHECK(runner, memory != nullptr);
	if (!memory)
		return;

	CRY_TEST_CHECK(runner, MM03IsAligned(memory, 64));
	std::memset(memory, 0x5A, 32);

	void* const replacement = safeAlloc(
		MM03AllocatorB,
		memory,
		64,
		alignof(std::max_align_t));
	CRY_TEST_CHECK(runner, replacement != nullptr);
	if (!replacement)
	{
		safeAlloc(MM03AllocatorA, memory, 0, alignof(std::max_align_t));
		return;
	}

	CRY_TEST_CHECK(runner, MM03IsAligned(replacement, 64));
	bool payloadPreserved = true;
	const auto* const bytes = static_cast<const unsigned char*>(replacement);
	for (size_t index = 0; index < 32; ++index)
		payloadPreserved = payloadPreserved && bytes[index] == 0x5A;

	safeAlloc(MM03AllocatorB, replacement, 0, alignof(std::max_align_t));

	CRY_TEST_CHECK(runner, payloadPreserved);
	CRY_TEST_CHECK(runner, g_mm03AllocatorA.callCount == 4);
	CRY_TEST_CHECK(runner, g_mm03AllocatorA.allocationCount == 2);
	CRY_TEST_CHECK(runner, g_mm03AllocatorA.freeCount == 2);
	CRY_TEST_CHECK(runner, g_mm03AllocatorB.callCount == 0);
}

static void TestMM03StructMetadataPolicy(CryTestRunner& runner)
{
	MM03CheckMetadataPolicy(runner, SafeAllocUsingStructMetadata);
}

static void TestMM03ConstexprMetadataPolicy(CryTestRunner& runner)
{
	MM03CheckMetadataPolicy(runner, SafeAllocUsingConstexprMetadata);
}

static void TestMM03DefaultSafeAllocAPI(CryTestRunner& runner)
{
	void* const memory = SafeAlloc(ModuleAlloc, nullptr, sizeof(std::max_align_t));
	CRY_TEST_CHECK(runner, memory != nullptr);
	if (!memory)
		return;

	CRY_TEST_CHECK(
		runner,
		MM03IsAligned(memory, alignof(std::max_align_t)));
	SafeAlloc(ModuleAlloc, memory, 0);
}

static void TestMM03SafeDynArrayComposition(CryTestRunner& runner)
{
	DynArray<uint64_t, FSafeModuleAlloc> values;
	values.resize(4);

	uint64_t* const elements = values.begin();
	CRY_TEST_CHECK(runner, values.size() == 4);
	CRY_TEST_CHECK(runner, MM03IsAligned(elements, alignof(uint64_t)));
	values[0] = 0x1122334455667788ull;
	CRY_TEST_CHECK(runner, values[0] == 0x1122334455667788ull);
}

static void TestMM03DynArray16Alignment(CryTestRunner& runner)
{
	DynArray16<MM03AlignedValue> values;
	values.resize(4);

	MM03AlignedValue* const elements = values.begin();
	CRY_TEST_CHECK(runner, values.size() == 4);
	CRY_TEST_CHECK(runner, MM03IsAligned(elements, 16));
	values[0].values[0] = 0x1020304050607080ull;
	CRY_TEST_CHECK(runner, values[0].values[0] == 0x1020304050607080ull);
}

static bool MM03HasInvariantTscAndRdtscp()
{
	int registers[4] = {};
	__cpuid(registers, static_cast<int>(0x80000000u));
	const unsigned int maximumExtendedLeaf = static_cast<unsigned int>(registers[0]);
	if (maximumExtendedLeaf < 0x80000007u)
		return false;

	__cpuid(registers, static_cast<int>(0x80000001u));
	const bool hasRdtscp = (static_cast<unsigned int>(registers[3]) & (1u << 27)) != 0;
	__cpuid(registers, static_cast<int>(0x80000007u));
	const bool hasInvariantTsc =
		(static_cast<unsigned int>(registers[3]) & (1u << 8)) != 0;
	return hasRdtscp && hasInvariantTsc;
}

static uint64_t MM03BeginTicks()
{
	_ReadWriteBarrier();
	_mm_lfence();
	const uint64_t ticks = __rdtsc();
	_mm_lfence();
	_ReadWriteBarrier();
	return ticks;
}

static uint64_t MM03EndTicks()
{
	unsigned int processorId = 0;
	_ReadWriteBarrier();
	const uint64_t ticks = __rdtscp(&processorId);
	_mm_lfence();
	_ReadWriteBarrier();
	return ticks;
}

static bool MM03WarmCpu(DWORD durationMilliseconds)
{
	LARGE_INTEGER frequency = {};
	LARGE_INTEGER start = {};
	if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&start))
		return false;

	const LONGLONG targetTicks =
		frequency.QuadPart * static_cast<LONGLONG>(durationMilliseconds) / 1000;
	volatile uint64_t accumulator = 0x9E3779B97F4A7C15ull;
	LARGE_INTEGER current = start;
	while (current.QuadPart - start.QuadPart < targetTicks)
	{
		for (size_t iteration = 0; iteration < 4096; ++iteration)
		{
			accumulator = accumulator * 6364136223846793005ull + 1442695040888963407ull;
		}
		if (!QueryPerformanceCounter(&current))
			return false;
	}

	return true;
}

static MM03BenchmarkClock MM03CalibrateBenchmarkClock()
{
	MM03BenchmarkClock clock = {};
	if (!MM03HasInvariantTscAndRdtscp())
		return clock;

	LARGE_INTEGER qpcFrequency = {};
	LARGE_INTEGER qpcStart = {};
	LARGE_INTEGER qpcFinish = {};
	if (!QueryPerformanceFrequency(&qpcFrequency) || !QueryPerformanceCounter(&qpcStart))
		return clock;

	const uint64_t tickStart = MM03BeginTicks();
	if (!MM03WarmCpu(200))
		return clock;
	const uint64_t tickFinish = MM03EndTicks();
	if (!QueryPerformanceCounter(&qpcFinish) || tickFinish <= tickStart)
		return clock;

	const LONGLONG elapsedQpc = qpcFinish.QuadPart - qpcStart.QuadPart;
	if (elapsedQpc <= 0)
		return clock;

	clock.ticksPerSecond =
		static_cast<double>(tickFinish - tickStart) *
		static_cast<double>(qpcFrequency.QuadPart) /
		static_cast<double>(elapsedQpc);
	clock.nanosecondsPerTick = 1.0e9 / clock.ticksPerSecond;
	clock.supported = clock.ticksPerSecond > 0.0;
	return clock;
}

static bool MM03WarmAllocator(
	MM03SafeAllocFunction safeAlloc,
	size_t allocationSize,
	size_t warmupCount)
{
	for (size_t iteration = 0; iteration < warmupCount; ++iteration)
	{
		void* const memory = safeAlloc(
			ModuleAlloc,
			nullptr,
			allocationSize,
			alignof(std::max_align_t));
		if (!memory)
			return false;

		static_cast<volatile unsigned char*>(memory)[0] =
			static_cast<unsigned char>(iteration);
		safeAlloc(ModuleAlloc, memory, 0, alignof(std::max_align_t));
	}

	return true;
}

static bool MM03CollectBenchmarkRound(
	MM03SafeAllocFunction safeAlloc,
	size_t allocationSize,
	MM03BenchmarkSeries& series,
	size_t roundIndex)
{
	constexpr size_t roundWarmupCount = 64;
	if (!MM03WarmAllocator(safeAlloc, allocationSize, roundWarmupCount))
		return false;

	double roundTotal = 0.0;
	for (size_t iteration = 0; iteration < MM03BenchmarkIterationCount; ++iteration)
	{
		const uint64_t start = MM03BeginTicks();
		void* const memory = safeAlloc(
			ModuleAlloc,
			nullptr,
			allocationSize,
			alignof(std::max_align_t));
		const uint64_t finish = MM03EndTicks();
		if (!memory)
			return false;
		if (finish <= start)
		{
			safeAlloc(ModuleAlloc, memory, 0, alignof(std::max_align_t));
			return false;
		}

		const double elapsedTicks = static_cast<double>(finish - start);
		series.samples.push_back(elapsedTicks);
		roundTotal += elapsedTicks;
		static_cast<volatile unsigned char*>(memory)[0] =
			static_cast<unsigned char>(iteration);
		safeAlloc(ModuleAlloc, memory, 0, alignof(std::max_align_t));
	}
	series.roundAverages[roundIndex] =
		roundTotal / static_cast<double>(MM03BenchmarkIterationCount);
	return true;
}

static bool MM03RunPairedBenchmark(
	size_t allocationSize,
	MM03BenchmarkSeries& structSeries,
	MM03BenchmarkSeries& constexprSeries)
{
	constexpr size_t initialWarmupCount = 1024;
	constexpr DWORD roundCpuWarmupMilliseconds = 10;
	structSeries = MM03BenchmarkSeries{};
	constexprSeries = MM03BenchmarkSeries{};
	structSeries.samples.reserve(MM03BenchmarkRoundCount * MM03BenchmarkIterationCount);
	constexprSeries.samples.reserve(MM03BenchmarkRoundCount * MM03BenchmarkIterationCount);

	if (!MM03WarmCpu(100) ||
		!MM03WarmAllocator(
			SafeAllocUsingStructMetadata, allocationSize, initialWarmupCount) ||
		!MM03WarmAllocator(
			SafeAllocUsingConstexprMetadata, allocationSize, initialWarmupCount))
	{
		return false;
	}

	for (size_t round = 0; round < MM03BenchmarkRoundCount; ++round)
	{
		if (!MM03WarmCpu(roundCpuWarmupMilliseconds))
			return false;

		const bool structFirst = (round % 2) == 0;
		if (structFirst)
		{
			if (!MM03CollectBenchmarkRound(
					SafeAllocUsingStructMetadata, allocationSize, structSeries, round) ||
				!MM03CollectBenchmarkRound(
					SafeAllocUsingConstexprMetadata, allocationSize, constexprSeries, round))
			{
				return false;
			}
		}
		else
		{
			if (!MM03CollectBenchmarkRound(
					SafeAllocUsingConstexprMetadata, allocationSize, constexprSeries, round) ||
				!MM03CollectBenchmarkRound(
					SafeAllocUsingStructMetadata, allocationSize, structSeries, round))
			{
				return false;
			}
		}
	}

	structSeries.completed = true;
	constexprSeries.completed = true;
	return true;
}

static MM03BenchmarkResult MM03CalculateBenchmarkResult(
	const MM03BenchmarkSeries& series)
{
	MM03BenchmarkResult result = {};
	if (!series.completed || series.samples.empty())
		return result;

	double total = 0.0;
	for (double sample : series.samples)
		total += sample;
	result.averageTicks = total / static_cast<double>(series.samples.size());

	double squaredDifferenceTotal = 0.0;
	for (double sample : series.samples)
	{
		const double difference = sample - result.averageTicks;
		squaredDifferenceTotal += difference * difference;
	}
	result.deviationTicks = std::sqrt(
		squaredDifferenceTotal / static_cast<double>(series.samples.size()));

	std::vector<double> sortedSamples = series.samples;
	std::sort(sortedSamples.begin(), sortedSamples.end());
	const size_t middle = sortedSamples.size() / 2;
	result.medianTicks = sortedSamples.size() % 2
		? sortedSamples[middle]
		: (sortedSamples[middle - 1] + sortedSamples[middle]) * 0.5;
	const size_t percentile95Index =
		(95 * sortedSamples.size() + 99) / 100 - 1;
	result.percentile95Ticks = sortedSamples[percentile95Index];

	const size_t trimCount = sortedSamples.size() / 20;
	double trimmedTotal = 0.0;
	for (size_t index = trimCount; index < sortedSamples.size() - trimCount; ++index)
		trimmedTotal += sortedSamples[index];
	result.trimmedMeanTicks = trimmedTotal /
		static_cast<double>(sortedSamples.size() - 2 * trimCount);
	result.completed = true;
	return result;
}

static MM03BenchmarkComparison MM03CompareBenchmarkSeries(
	const MM03BenchmarkSeries& structSeries,
	const MM03BenchmarkSeries& constexprSeries)
{
	MM03BenchmarkComparison comparison = {};
	if (!structSeries.completed || !constexprSeries.completed)
		return comparison;

	std::array<double, MM03BenchmarkRoundCount> differences = {};
	for (size_t round = 0; round < MM03BenchmarkRoundCount; ++round)
	{
		differences[round] =
			structSeries.roundAverages[round] - constexprSeries.roundAverages[round];
		comparison.averageDifferenceTicks += differences[round];
		if (differences[round] > 0.0)
			++comparison.constexprWinCount;
	}
	comparison.averageDifferenceTicks /= static_cast<double>(MM03BenchmarkRoundCount);

	double squaredDifferenceTotal = 0.0;
	for (double difference : differences)
	{
		const double distance = difference - comparison.averageDifferenceTicks;
		squaredDifferenceTotal += distance * distance;
	}
	comparison.deviationDifferenceTicks = std::sqrt(
		squaredDifferenceTotal / static_cast<double>(MM03BenchmarkRoundCount - 1));

	constexpr double studentTCritical95For23DegreesOfFreedom = 2.068657610;
	const double confidenceMargin =
		studentTCritical95For23DegreesOfFreedom * comparison.deviationDifferenceTicks /
		std::sqrt(static_cast<double>(MM03BenchmarkRoundCount));
	comparison.confidenceLowTicks = comparison.averageDifferenceTicks - confidenceMargin;
	comparison.confidenceHighTicks = comparison.averageDifferenceTicks + confidenceMargin;
	comparison.completed = true;
	return comparison;
}

static void MM03PrintTickMetric(
	const char* metricName,
	double ticks,
	const MM03BenchmarkClock& clock)
{
	std::printf(
		"    %-15s %10.2f ticks / %8.2f ns\n",
		metricName,
		ticks,
		ticks * clock.nanosecondsPerTick);
}

static void MM03PrintBenchmarkResult(
	const char* policyName,
	const char* allocationName,
	size_t allocationSize,
	const MM03BenchmarkResult& result,
	const MM03BenchmarkClock& clock)
{
	if (!result.completed)
	{
		std::printf(
			"  %s %s (%zu bytes): allocation failed\n",
			policyName,
			allocationName,
			allocationSize);
		return;
	}

	std::printf("  %s %s (%zu bytes)\n", policyName, allocationName, allocationSize);
	MM03PrintTickMetric("average", result.averageTicks, clock);
	MM03PrintTickMetric("deviation", result.deviationTicks, clock);
	MM03PrintTickMetric("median", result.medianTicks, clock);
	MM03PrintTickMetric("p95", result.percentile95Ticks, clock);
	MM03PrintTickMetric("5% trimmed", result.trimmedMeanTicks, clock);
}

static void MM03PrintBenchmarkComparison(
	const char* allocationName,
	const MM03BenchmarkComparison& comparison,
	const MM03BenchmarkClock& clock)
{
	if (!comparison.completed)
		return;

	const char* conclusion = "inconclusive";
	if (comparison.confidenceLowTicks > 0.0)
		conclusion = "constexpr faster";
	else if (comparison.confidenceHighTicks < 0.0)
		conclusion = "struct faster";

	std::printf(
		"  %s paired difference (struct - constexpr)\n"
		"    average          %+10.2f ticks / %+8.2f ns\n"
		"    deviation        %10.2f ticks / %8.2f ns\n"
		"    95%% confidence   [%+.2f, %+.2f] ticks / [%+.2f, %+.2f] ns\n"
		"    constexpr wins   %zu/%zu rounds; %s\n",
		allocationName,
		comparison.averageDifferenceTicks,
		comparison.averageDifferenceTicks * clock.nanosecondsPerTick,
		comparison.deviationDifferenceTicks,
		comparison.deviationDifferenceTicks * clock.nanosecondsPerTick,
		comparison.confidenceLowTicks,
		comparison.confidenceHighTicks,
		comparison.confidenceLowTicks * clock.nanosecondsPerTick,
		comparison.confidenceHighTicks * clock.nanosecondsPerTick,
		comparison.constexprWinCount,
		MM03BenchmarkRoundCount,
		conclusion);
}

static void TestMM03AllocationBenchmarks(CryTestRunner& runner)
{
	constexpr size_t smallAllocationSize = 64;
	constexpr size_t largeAllocationSize = 1024 * 1024;
	MM03BenchmarkEnvironment environment;
	const MM03BenchmarkClock clock = MM03CalibrateBenchmarkClock();

	std::printf(
		"\nMM03 benchmark environment: group %u, logical CPU %u, "
		"affinity %s, process priority %s, thread priority %s, boost control %s\n",
		static_cast<unsigned int>(environment.ProcessorGroup()),
		static_cast<unsigned int>(environment.LogicalProcessorIndex()),
		environment.IsAffinityLocked() ? "locked" : "failed",
		environment.IsProcessPriorityRaised() ? "high" : "failed",
		environment.IsThreadPriorityRaised() ? "highest" : "failed",
		environment.IsPriorityBoostDisabled() ? "disabled" : "failed");
	if (clock.supported)
	{
		std::printf(
			"MM03 benchmark clock: invariant TSC + RDTSCP, calibrated %.3f MHz\n",
			clock.ticksPerSecond / 1.0e6);
	}
	else
	{
		std::printf("MM03 benchmark clock: invariant TSC or RDTSCP unavailable\n");
	}

	CRY_TEST_CHECK(runner, environment.IsReady());
	CRY_TEST_CHECK(runner, clock.supported);
	if (!environment.IsReady() || !clock.supported)
		return;

	MM03BenchmarkSeries structSmallSeries = {};
	MM03BenchmarkSeries constexprSmallSeries = {};
	MM03BenchmarkSeries structLargeSeries = {};
	MM03BenchmarkSeries constexprLargeSeries = {};
	const bool smallCompleted = MM03RunPairedBenchmark(
		smallAllocationSize, structSmallSeries, constexprSmallSeries);
	const bool largeCompleted = MM03RunPairedBenchmark(
		largeAllocationSize, structLargeSeries, constexprLargeSeries);

	const MM03BenchmarkResult structSmall =
		MM03CalculateBenchmarkResult(structSmallSeries);
	const MM03BenchmarkResult constexprSmall =
		MM03CalculateBenchmarkResult(constexprSmallSeries);
	const MM03BenchmarkResult structLarge =
		MM03CalculateBenchmarkResult(structLargeSeries);
	const MM03BenchmarkResult constexprLarge =
		MM03CalculateBenchmarkResult(constexprLargeSeries);
	const MM03BenchmarkComparison smallComparison =
		MM03CompareBenchmarkSeries(structSmallSeries, constexprSmallSeries);
	const MM03BenchmarkComparison largeComparison =
		MM03CompareBenchmarkSeries(structLargeSeries, constexprLargeSeries);

	std::printf(
		"MM03 allocation benchmark (%zu paired ABBA rounds x %zu allocations, "
		"%zu measured per implementation/case)\n",
		MM03BenchmarkRoundCount,
		MM03BenchmarkIterationCount,
		MM03BenchmarkRoundCount * MM03BenchmarkIterationCount);
	MM03PrintBenchmarkResult(
		"struct metadata",
		"small",
		smallAllocationSize,
		structSmall,
		clock);
	MM03PrintBenchmarkResult(
		"struct metadata",
		"large",
		largeAllocationSize,
		structLarge,
		clock);
	MM03PrintBenchmarkResult(
		"constexpr metadata",
		"small",
		smallAllocationSize,
		constexprSmall,
		clock);
	MM03PrintBenchmarkResult(
		"constexpr metadata",
		"large",
		largeAllocationSize,
		constexprLarge,
		clock);
	MM03PrintBenchmarkComparison("small", smallComparison, clock);
	MM03PrintBenchmarkComparison("large", largeComparison, clock);

	CRY_TEST_CHECK(runner, smallCompleted);
	CRY_TEST_CHECK(runner, largeCompleted);
	CRY_TEST_CHECK(runner, structSmall.completed);
	CRY_TEST_CHECK(runner, structLarge.completed);
	CRY_TEST_CHECK(runner, constexprSmall.completed);
	CRY_TEST_CHECK(runner, constexprLarge.completed);
	CRY_TEST_CHECK(runner, smallComparison.completed);
	CRY_TEST_CHECK(runner, largeComparison.completed);
}

void RunSafeAllocTests(CryTestRunner& runner)
{
	runner.Run("MM03 struct metadata policy", TestMM03StructMetadataPolicy);
	runner.Run("MM03 constexpr metadata policy", TestMM03ConstexprMetadataPolicy);
	runner.Run("MM03 default SafeAlloc alignment API", TestMM03DefaultSafeAllocAPI);
	runner.Run("MM03 FSafeModuleAlloc DynArray composition", TestMM03SafeDynArrayComposition);
	runner.Run("MM03 DynArray16 preserves 16-byte alignment", TestMM03DynArray16Alignment);
	runner.Run("MM03 allocation benchmarks", TestMM03AllocationBenchmarks);
}

int RunMM03Trace()
{
	CryTestRunner runner;
	RunSafeAllocTests(runner);
	return runner.ExitCode();
}
