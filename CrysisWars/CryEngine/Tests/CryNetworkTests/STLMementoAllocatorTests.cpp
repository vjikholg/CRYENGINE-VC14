#include "CryNetworkTestPreamble.h"
#include "MementoAllocatorTestSupport.h"
#include "STLMementoAllocatorTests.h"
#include "TestRunner.h"

#include <CryCommon/StlHashMap.h>
#include <STLMementoAllocator.h>

#include <memory>
#include <type_traits>
#include <utility>

struct SMementoRebindProbe
{
	uint64 words[3];
};

typedef std::pair<const uint32, uint32> TMementoMapValue;
typedef STLMementoAllocator<TMementoMapValue> TMementoMapAllocator;
typedef std::allocator_traits<TMementoMapAllocator> TMementoMapAllocatorTraits;
typedef TMementoMapAllocatorTraits::rebind_alloc<SMementoRebindProbe>
	TMementoReboundAllocator;
typedef TMementoMapAllocatorTraits::rebind_alloc<void> TMementoVoidAllocator;
typedef stl::CryHashMap<
	uint32,
	uint32,
	stl::hash_uint32,
	TMementoMapAllocator> TMementoMap;

static_assert(
	std::is_same<TMementoReboundAllocator, STLMementoAllocator<SMementoRebindProbe>>::value,
	"allocator_traits must rebind STLMementoAllocator to the requested element type");
static_assert(
	std::is_same<TMementoVoidAllocator, STLMementoAllocator<void>>::value,
	"allocator_traits must retain the STLMementoAllocator family when rebinding to void");
static_assert(
	std::is_nothrow_constructible<
		TMementoReboundAllocator,
		const TMementoMapAllocator&>::value,
	"typed allocator rebinding must be non-throwing");
static_assert(
	std::is_nothrow_constructible<
		TMementoMapAllocator,
		const TMementoReboundAllocator&>::value,
	"round-trip allocator rebinding must be non-throwing");
static_assert(
	std::is_nothrow_constructible<
		TMementoVoidAllocator,
		const TMementoMapAllocator&>::value,
	"void allocator rebinding must be non-throwing");
static_assert(!TMementoMapAllocatorTraits::is_always_equal::value);
static_assert(!TMementoMapAllocatorTraits::propagate_on_container_copy_assignment::value);
static_assert(!TMementoMapAllocatorTraits::propagate_on_container_move_assignment::value);
static_assert(!TMementoMapAllocatorTraits::propagate_on_container_swap::value);

static CMementoMemoryManagerPtr CreateMementoManager()
{
	return CMementoMemoryManagerPtr(new CMementoMemoryManager());
}

static std::unique_ptr<TMementoMap> CreateMementoMap(
	const CMementoMemoryManagerPtr& manager)
{
	return std::unique_ptr<TMementoMap>(new TMementoMap(
		0,
		stl::hash_uint32(),
		TMementoMap::key_equal(),
		TMementoMapAllocator(manager)));
}

static void TestRebindProducesTheAllocatorFamily(CryTestRunner& runner)
{
	CRY_TEST_CHECK(
		runner,
		(std::is_same<
			TMementoReboundAllocator,
			STLMementoAllocator<SMementoRebindProbe>>::value));
	CRY_TEST_CHECK(
		runner,
		(std::is_same<TMementoVoidAllocator, STLMementoAllocator<void>>::value));
}

static void TestConvertingConstructorsAreNonThrowing(CryTestRunner& runner)
{
	CRY_TEST_CHECK(
		runner,
		(std::is_nothrow_constructible<
			TMementoReboundAllocator,
			const TMementoMapAllocator&>::value));
	CRY_TEST_CHECK(
		runner,
		(std::is_nothrow_constructible<
			TMementoMapAllocator,
			const TMementoReboundAllocator&>::value));
	CRY_TEST_CHECK(
		runner,
		(std::is_nothrow_constructible<
			TMementoVoidAllocator,
			const TMementoMapAllocator&>::value));
}

static void TestRebindPreservesTheManager(CryTestRunner& runner)
{
	CMementoMemoryManagerPtr manager = CreateMementoManager();
	CMementoMemoryManager* const expectedManager = manager.get();
	const TMementoMapAllocator mapAllocator(manager);
	const TMementoReboundAllocator reboundAllocator(mapAllocator);
	const TMementoMapAllocator roundTripAllocator(reboundAllocator);
	const TMementoVoidAllocator voidAllocator(mapAllocator);
	const TMementoMapAllocator voidRoundTripAllocator(voidAllocator);

	CRY_TEST_CHECK(runner, mapAllocator.m_pMMM.get() == expectedManager);
	CRY_TEST_CHECK(runner, reboundAllocator.m_pMMM.get() == expectedManager);
	CRY_TEST_CHECK(runner, roundTripAllocator.m_pMMM.get() == expectedManager);
	CRY_TEST_CHECK(runner, voidAllocator.m_pMMM.get() == expectedManager);
	CRY_TEST_CHECK(runner, voidRoundTripAllocator.m_pMMM.get() == expectedManager);
}

static void TestEqualityAndPropagationPolicy(CryTestRunner& runner)
{
	CMementoMemoryManagerPtr managerA = CreateMementoManager();
	CMementoMemoryManagerPtr managerB = CreateMementoManager();
	const TMementoMapAllocator allocatorA(managerA);
	const TMementoMapAllocator equivalentAllocatorA(managerA);
	const TMementoMapAllocator allocatorB(managerB);
	const TMementoReboundAllocator reboundAllocatorA(allocatorA);

	CRY_TEST_CHECK(runner, allocatorA == equivalentAllocatorA);
	CRY_TEST_CHECK(runner, allocatorA != allocatorB);
	CRY_TEST_CHECK(runner, allocatorA == reboundAllocatorA);
	CRY_TEST_CHECK(runner, !(allocatorA != reboundAllocatorA));
	CRY_TEST_CHECK(runner, !TMementoMapAllocatorTraits::is_always_equal::value);
	CRY_TEST_CHECK(
		runner,
		!TMementoMapAllocatorTraits::propagate_on_container_copy_assignment::value);
	CRY_TEST_CHECK(
		runner,
		!TMementoMapAllocatorTraits::propagate_on_container_move_assignment::value);
	CRY_TEST_CHECK(
		runner,
		!TMementoMapAllocatorTraits::propagate_on_container_swap::value);

	TMementoMap source(
		0,
		stl::hash_uint32(),
		TMementoMap::key_equal(),
		allocatorA);
	TMementoMap destination(
		0,
		stl::hash_uint32(),
		TMementoMap::key_equal(),
		allocatorB);
	source.emplace(7u, 70u);
	destination.emplace(9u, 90u);
	const TMementoMap copied(source);
	CRY_TEST_CHECK(runner, copied.get_allocator().m_pMMM.get() == managerA.get());
	CRY_TEST_CHECK(runner, copied.at(7u) == 70u);

	destination = source;
	CRY_TEST_CHECK(runner, destination.get_allocator().m_pMMM.get() == managerB.get());
	CRY_TEST_CHECK(runner, destination.at(7u) == 70u);

	TMementoMap moveSource(
		0,
		stl::hash_uint32(),
		TMementoMap::key_equal(),
		allocatorA);
	moveSource.emplace(11u, 110u);
	destination = std::move(moveSource);
	CRY_TEST_CHECK(runner, destination.get_allocator().m_pMMM.get() == managerB.get());
	CRY_TEST_CHECK(runner, destination.at(11u) == 110u);
}

static void TestDirectReboundAllocationUsesTheOriginalManager(CryTestRunner& runner)
{
	CMementoMemoryManagerPtr manager = CreateMementoManager();
	CMementoMemoryManager* const rawManager = manager.get();
	TMementoReboundAllocator allocator{TMementoMapAllocator(manager)};

	SMementoRebindProbe* const allocation =
		std::allocator_traits<TMementoReboundAllocator>::allocate(allocator, 3);
	SMementoAllocatorTestStats stats = GetMementoAllocatorTestStats(rawManager);
	CRY_TEST_CHECK(runner, stats.allocationCalls == 1u);
	CRY_TEST_CHECK(runner, stats.liveAllocationCount == 1u);
	CRY_TEST_CHECK(runner, stats.liveBytes == 3u * sizeof(SMementoRebindProbe));
	CRY_TEST_CHECK(runner, stats.invalidDeallocationCalls == 0u);

	std::allocator_traits<TMementoReboundAllocator>::deallocate(allocator, allocation, 3);
	stats = GetMementoAllocatorTestStats(rawManager);
	CRY_TEST_CHECK(runner, stats.deallocationCalls == 1u);
	CRY_TEST_CHECK(runner, stats.liveAllocationCount == 0u);
	CRY_TEST_CHECK(runner, stats.liveBytes == 0u);
	CRY_TEST_CHECK(runner, stats.invalidDeallocationCalls == 0u);
}

static void TestDefaultAllocatorCapturesItsRegion(CryTestRunner& runner)
{
	CMementoMemoryManagerPtr managerA = CreateMementoManager();
	CMementoMemoryManagerPtr managerB = CreateMementoManager();
	std::unique_ptr<TMementoMap> values;

	SetCurrentMementoManagerForTest(managerA.get());
	values.reset(new TMementoMap());

	CRY_TEST_CHECK(runner, values->get_allocator().m_pMMM.get() == managerA.get());
	SetCurrentMementoManagerForTest(managerB.get());
	for (uint32 value = 0; value < 256; ++value)
	{
		values->emplace(value, value * 10u);
	}
	SetCurrentMementoManagerForTest(0);

	const SMementoAllocatorTestStats managerAStats =
		GetMementoAllocatorTestStats(managerA.get());
	const SMementoAllocatorTestStats managerBStats =
		GetMementoAllocatorTestStats(managerB.get());
	CRY_TEST_CHECK(runner, managerAStats.allocationCalls != 0u);
	CRY_TEST_CHECK(runner, managerAStats.liveAllocationCount != 0u);
	CRY_TEST_CHECK(runner, managerBStats.allocationCalls == 0u);
	CRY_TEST_CHECK(runner, managerBStats.liveAllocationCount == 0u);

	values.reset();
	const SMementoAllocatorTestStats releasedStats =
		GetMementoAllocatorTestStats(managerA.get());
	CRY_TEST_CHECK(runner, releasedStats.liveAllocationCount == 0u);
	CRY_TEST_CHECK(runner, releasedStats.liveBytes == 0u);
	CRY_TEST_CHECK(runner, releasedStats.invalidDeallocationCalls == 0u);
}

static void TestRehashAndDestructionUseTheCapturedManager(CryTestRunner& runner)
{
	CMementoMemoryManagerPtr manager = CreateMementoManager();
	CMementoMemoryManager* const rawManager = manager.get();
	std::unique_ptr<TMementoMap> values = CreateMementoMap(manager);
	values->max_load_factor(0.35f);

	for (uint32 value = 0; value < 2048; ++value)
	{
		values->emplace(value, value ^ 0x5a5a5a5au);
	}

	const std::size_t allocationsBeforeExplicitRehash =
		GetMementoAllocatorTestStats(rawManager).allocationCalls;
	const std::size_t bucketsBeforeExplicitRehash = values->bucket_count();
	values->rehash(bucketsBeforeExplicitRehash * 2u + 1u);

	CRY_TEST_CHECK(runner, values->bucket_count() > bucketsBeforeExplicitRehash);
	CRY_TEST_CHECK(
		runner,
		GetMementoAllocatorTestStats(rawManager).allocationCalls >
			allocationsBeforeExplicitRehash);
	for (uint32 value = 0; value < 2048; value += 97)
	{
		CRY_TEST_CHECK(runner, values->at(value) == (value ^ 0x5a5a5a5au));
	}

	values.reset();
	const SMementoAllocatorTestStats stats =
		GetMementoAllocatorTestStats(rawManager);
	CRY_TEST_CHECK(runner, stats.allocationCalls == stats.deallocationCalls);
	CRY_TEST_CHECK(runner, stats.liveAllocationCount == 0u);
	CRY_TEST_CHECK(runner, stats.liveBytes == 0u);
	CRY_TEST_CHECK(runner, stats.invalidDeallocationCalls == 0u);
}

static void TestContainerOwnsTheManagerThroughDestruction(CryTestRunner& runner)
{
	CMementoMemoryManagerPtr manager = CreateMementoManager();
	CMementoMemoryManager* const rawManager = manager.get();
	std::unique_ptr<TMementoMap> values = CreateMementoMap(manager);
	values->emplace(1u, 10u);
	values->emplace(2u, 20u);

	manager = 0;
	SMementoAllocatorTestStats stats = GetMementoAllocatorTestStats(rawManager);
	CRY_TEST_CHECK(runner, stats.alive);
	CRY_TEST_CHECK(runner, stats.liveAllocationCount != 0u);

	values.reset();
	stats = GetMementoAllocatorTestStats(rawManager);
	CRY_TEST_CHECK(runner, !stats.alive);
	CRY_TEST_CHECK(runner, !stats.destroyedWithOutstandingAllocations);
	CRY_TEST_CHECK(runner, stats.liveAllocationCount == 0u);
	CRY_TEST_CHECK(runner, stats.liveBytes == 0u);
	CRY_TEST_CHECK(runner, stats.invalidDeallocationCalls == 0u);
}

void RunSTLMementoAllocatorTests(CryTestRunner& runner)
{
	runner.Run("STLMementoAllocator rebind type", TestRebindProducesTheAllocatorFamily);
	runner.Run("STLMementoAllocator converting constructors", TestConvertingConstructorsAreNonThrowing);
	runner.Run("STLMementoAllocator manager preservation", TestRebindPreservesTheManager);
	runner.Run("STLMementoAllocator equality and propagation", TestEqualityAndPropagationPolicy);
	runner.Run("STLMementoAllocator rebound allocation routing", TestDirectReboundAllocationUsesTheOriginalManager);
	runner.Run("STLMementoAllocator region capture", TestDefaultAllocatorCapturesItsRegion);
	runner.Run("STLMementoAllocator rehash and destruction", TestRehashAndDestructionUseTheCapturedManager);
	runner.Run("STLMementoAllocator manager lifetime", TestContainerOwnsTheManagerThroughDestruction);
}
