#include "MM08Tests.h"
#include "TestRunner.h"

#include <platform.h>
#include <PoolAllocator.h>
#include <STLPoolAllocator.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <mutex>
#include <new>
#include <thread>
#include <unordered_set>
#include <vector>

struct alignas(32) MM08OverAlignedPayload
{
	std::array<uint64_t, 4> words;
};

struct MM08ConcurrentPayload
{
	std::array<uint64_t, 8> words;
};

struct alignas(64) MM08ConcurrentStlPayload
{
	std::array<uint64_t, 16> words;
};

static bool MM08IsAligned(const void* pointer, size_t alignment)
{
	return reinterpret_cast<uintptr_t>(pointer) % alignment == 0;
}

static uint32_t MM08NextRandom(uint32_t& state)
{
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}

static void TestMM08DefaultPoolGuaranteesSixteenByteNodes(
	CryTestRunner& runner)
{
	using Allocator = stl::PoolAllocator<
		24,
		stl::PoolAllocatorSynchronizationSinglethreaded>;

	CRY_TEST_CHECK(runner, Allocator::ObjectAlignment == 16);
	CRY_TEST_CHECK(runner, Allocator::ObjectAllocationSize == 32);

	Allocator allocator(3);
	std::vector<void*> allocations;
	for (size_t index = 0; index < 12; ++index)
	{
		void* const memory = allocator.Allocate();
		CRY_TEST_CHECK(runner, memory != nullptr);
		CRY_TEST_CHECK(runner, MM08IsAligned(memory, 16));
		allocations.push_back(memory);
	}

	for (void* memory : allocations)
		allocator.Deallocate(memory);
}

static void TestMM08TypedPoolHonorsOverAlignment(CryTestRunner& runner)
{
	stl::STLPoolAllocator<
		MM08OverAlignedPayload,
		stl::PoolAllocatorSynchronizationSinglethreaded> allocator;
	std::vector<MM08OverAlignedPayload*> allocations;

	for (size_t index = 0; index < 256; ++index)
	{
		MM08OverAlignedPayload* const payload = allocator.allocate(1);
		CRY_TEST_CHECK(runner, payload != nullptr);
		CRY_TEST_CHECK(
			runner,
			MM08IsAligned(payload, alignof(MM08OverAlignedPayload)));

		allocator.construct(payload);
		payload->words = {
			index,
			index + 1,
			index + 2,
			index + 3
		};
		CRY_TEST_CHECK(runner, payload->words[3] == index + 3);
		allocations.push_back(payload);
	}

	for (MM08OverAlignedPayload* payload : allocations)
	{
		allocator.destroy(payload);
		allocator.deallocate(payload, 1);
	}
}

static void TestMM08BucketArithmeticRejectsInvalidCounts(
	CryTestRunner& runner)
{
	using Allocator = stl::PoolAllocator<
		64,
		stl::PoolAllocatorSynchronizationSinglethreaded>;

	size_t bucketBytes = 0;
	CRY_TEST_CHECK(runner, Allocator::TryCalculateBucketBytes(128, bucketBytes));
	CRY_TEST_CHECK(
		runner,
		bucketBytes ==
			Allocator::BucketHeaderSize +
				128 * Allocator::ObjectAllocationSize);

	CRY_TEST_CHECK(runner, !Allocator::TryCalculateBucketBytes(0, bucketBytes));
	CRY_TEST_CHECK(runner, bucketBytes == 0);
	CRY_TEST_CHECK(
		runner,
		!Allocator::TryCalculateBucketBytes(
			(std::numeric_limits<size_t>::max)(),
			bucketBytes));
	CRY_TEST_CHECK(runner, bucketBytes == 0);

	bool rejectedZero = false;
	try
	{
		Allocator invalidAllocator(0);
	}
	catch (const std::bad_array_new_length&)
	{
		rejectedZero = true;
	}
	CRY_TEST_CHECK(runner, rejectedZero);
}

static void TestMM08LiveAccountingTracksBucketsAndNodes(
	CryTestRunner& runner)
{
	using Allocator = stl::PoolAllocator<
		64,
		stl::PoolAllocatorSynchronizationSinglethreaded>;
	Allocator allocator(4);
	std::array<void*, 5> allocations = {};

	for (void*& allocation : allocations)
		allocation = allocator.Allocate();

	const size_t expectedBucketBytes =
		2 * (Allocator::BucketHeaderSize +
			4 * Allocator::ObjectAllocationSize);
	CRY_TEST_CHECK(
		runner,
		allocator.GetTotalAllocatedMemory() == expectedBucketBytes);
	CRY_TEST_CHECK(
		runner,
		allocator.GetTotalAllocatedNodeSize() ==
			allocations.size() * Allocator::ObjectAllocationSize);

	for (void* allocation : allocations)
		allocator.Deallocate(allocation);

	CRY_TEST_CHECK(runner, allocator.GetTotalAllocatedNodeSize() == 0);
}

static void TestMM08PoolReusesFreedNodes(CryTestRunner& runner)
{
	using Allocator = stl::PoolAllocator<
		64,
		stl::PoolAllocatorSynchronizationSinglethreaded>;
	Allocator allocator(8);
	void* const first = allocator.Allocate();
	void* const second = allocator.Allocate();

	CRY_TEST_CHECK(runner, first != nullptr);
	CRY_TEST_CHECK(runner, second != nullptr);
	CRY_TEST_CHECK(runner, first != second);

	allocator.Deallocate(first);
	void* const reused = allocator.Allocate();
	CRY_TEST_CHECK(runner, reused == first);

	allocator.Deallocate(reused);
	allocator.Deallocate(second);
}

static void TestMM08ConcurrentPoolMaintainsExclusiveOwnership(
	CryTestRunner& runner)
{
	using Allocator = stl::PoolAllocator<sizeof(MM08ConcurrentPayload)>;
	constexpr size_t ThreadCount = 6;
	constexpr size_t OperationsPerThread = 30000;

	Allocator allocator(64);
	std::mutex livePointersMutex;
	std::unordered_set<void*> livePointers;
	std::atomic<bool> failed{false};
	std::vector<std::thread> threads;
	threads.reserve(ThreadCount);

	for (size_t threadIndex = 0; threadIndex < ThreadCount; ++threadIndex)
	{
		threads.emplace_back([&, threadIndex]()
		{
			uint32_t randomState =
				0x9e3779b9u ^ static_cast<uint32_t>(threadIndex * 0x85ebca6bu);
			std::vector<MM08ConcurrentPayload*> owned;
			owned.reserve(128);

			for (size_t operation = 0;
				operation < OperationsPerThread && !failed.load();
				++operation)
			{
				const bool allocate =
					owned.empty() ||
					(owned.size() < 128 && (MM08NextRandom(randomState) & 1u) != 0);
				if (allocate)
				{
					auto* const payload = static_cast<MM08ConcurrentPayload*>(
						allocator.Allocate());
					{
						std::lock_guard<std::mutex> lock(livePointersMutex);
						if (!livePointers.insert(payload).second)
							failed.store(true);
					}

					const uint64_t marker =
						(static_cast<uint64_t>(threadIndex) << 56) | operation;
					payload->words.fill(marker);
					owned.push_back(payload);
				}
				else
				{
					const size_t selected = MM08NextRandom(randomState) % owned.size();
					MM08ConcurrentPayload* const payload = owned[selected];
					const uint64_t marker = payload->words[0];
					for (uint64_t word : payload->words)
					{
						if (word != marker)
							failed.store(true);
					}

					{
						std::lock_guard<std::mutex> lock(livePointersMutex);
						if (livePointers.erase(payload) != 1)
							failed.store(true);
					}
					allocator.Deallocate(payload);
					owned[selected] = owned.back();
					owned.pop_back();
				}
			}

			for (MM08ConcurrentPayload* payload : owned)
			{
				{
					std::lock_guard<std::mutex> lock(livePointersMutex);
					if (livePointers.erase(payload) != 1)
						failed.store(true);
				}
				allocator.Deallocate(payload);
			}
		});
	}

	for (std::thread& thread : threads)
		thread.join();

	CRY_TEST_CHECK(runner, !failed.load());
	CRY_TEST_CHECK(runner, livePointers.empty());
	CRY_TEST_CHECK(runner, allocator.GetTotalAllocatedNodeSize() == 0);
}

static void TestMM08ConcurrentStlFirstUseMaintainsExclusiveOwnership(
	CryTestRunner& runner)
{
	using Allocator = stl::STLPoolAllocator<MM08ConcurrentStlPayload>;
	constexpr size_t ThreadCount = 8;
	constexpr size_t OperationsPerThread = 10000;

	std::mutex livePointersMutex;
	std::unordered_set<void*> livePointers;
	std::atomic<size_t> readyThreadCount{0};
	std::atomic<bool> start{false};
	std::atomic<bool> failed{false};
	std::vector<std::thread> threads;
	threads.reserve(ThreadCount);

	for (size_t threadIndex = 0; threadIndex < ThreadCount; ++threadIndex)
	{
		threads.emplace_back([&, threadIndex]()
		{
			Allocator allocator;
			uint32_t randomState =
				0x243f6a88u ^ static_cast<uint32_t>(threadIndex * 0x9e3779b9u);
			std::vector<MM08ConcurrentStlPayload*> owned;
			owned.reserve(64);

			readyThreadCount.fetch_add(1, std::memory_order_release);
			while (!start.load(std::memory_order_acquire))
				std::this_thread::yield();

			for (size_t operation = 0;
				operation < OperationsPerThread && !failed.load();
				++operation)
			{
				const bool allocate =
					owned.empty() ||
					(owned.size() < 64 &&
						(MM08NextRandom(randomState) & 1u) != 0);
				if (allocate)
				{
					MM08ConcurrentStlPayload* const payload =
						allocator.allocate(1);
					bool ownsUniquePointer = false;
					{
						std::lock_guard<std::mutex> lock(livePointersMutex);
						ownsUniquePointer = livePointers.insert(payload).second;
					}
					if (!ownsUniquePointer ||
						!MM08IsAligned(payload, alignof(MM08ConcurrentStlPayload)))
					{
						failed.store(true);
						break;
					}

					allocator.construct(payload);
					const uint64_t marker =
						(static_cast<uint64_t>(threadIndex) << 56) | operation;
					payload->words.fill(marker);
					owned.push_back(payload);
				}
				else
				{
					const size_t selected =
						MM08NextRandom(randomState) % owned.size();
					MM08ConcurrentStlPayload* const payload = owned[selected];
					const uint64_t marker = payload->words[0];
					for (uint64_t word : payload->words)
					{
						if (word != marker)
							failed.store(true);
					}

					{
						std::lock_guard<std::mutex> lock(livePointersMutex);
						if (livePointers.erase(payload) != 1)
							failed.store(true);
					}
					allocator.destroy(payload);
					allocator.deallocate(payload, 1);
					owned[selected] = owned.back();
					owned.pop_back();
				}
			}

			for (MM08ConcurrentStlPayload* payload : owned)
			{
				{
					std::lock_guard<std::mutex> lock(livePointersMutex);
					if (livePointers.erase(payload) != 1)
						failed.store(true);
				}
				allocator.destroy(payload);
				allocator.deallocate(payload, 1);
			}
		});
	}

	while (readyThreadCount.load(std::memory_order_acquire) != ThreadCount)
		std::this_thread::yield();
	start.store(true, std::memory_order_release);

	for (std::thread& thread : threads)
		thread.join();

	CRY_TEST_CHECK(runner, !failed.load());
	CRY_TEST_CHECK(runner, livePointers.empty());
}

void RunMM08PoolAllocatorTests(CryTestRunner& runner)
{
	runner.Run("MM08 default pool guarantees 16-byte nodes", TestMM08DefaultPoolGuaranteesSixteenByteNodes);
	runner.Run("MM08 typed pool honors over-alignment", TestMM08TypedPoolHonorsOverAlignment);
	runner.Run("MM08 bucket arithmetic rejects invalid counts", TestMM08BucketArithmeticRejectsInvalidCounts);
	runner.Run("MM08 live accounting tracks buckets and nodes", TestMM08LiveAccountingTracksBucketsAndNodes);
	runner.Run("MM08 pool reuses freed nodes", TestMM08PoolReusesFreedNodes);
	runner.Run("MM08 concurrent pool maintains exclusive ownership", TestMM08ConcurrentPoolMaintainsExclusiveOwnership);
	runner.Run("MM08 concurrent STL first use maintains exclusive ownership", TestMM08ConcurrentStlFirstUseMaintainsExclusiveOwnership);
}

int RunMM08Trace()
{
	CryTestRunner runner;
	RunMM08PoolAllocatorTests(runner);
	return runner.ExitCode();
}
