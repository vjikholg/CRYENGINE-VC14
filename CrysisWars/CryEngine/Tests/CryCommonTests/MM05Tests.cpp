#include "MM05Tests.h"
#include "TestRunner.h"

#include <CryAllocatorFailure.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

static_assert(sizeof(void*) == 8, "MM05 tests require the Win64 data model.");
static_assert(sizeof(size_t) == 8, "MM05 tests require pointer-width size accounting.");

struct MM05LegacyHandleCodec
{
	static constexpr uint32_t InvalidHandle = UINT32_MAX;

	static constexpr uint32_t ProtectionKey(uintptr_t managerAddress)
	{
		return static_cast<uint32_t>(managerAddress);
	}

	static constexpr uint32_t Protect(uint32_t rawHandle, uintptr_t managerAddress)
	{
		return (rawHandle + 0x80u) ^ ProtectionKey(managerAddress);
	}

	static constexpr uint32_t Unprotect(uint32_t protectedHandle, uintptr_t managerAddress)
	{
		return (protectedHandle ^ ProtectionKey(managerAddress)) - 0x80u;
	}
};

struct MM05ReservedDomainCodec
{
	static constexpr uint32_t InvalidHandle = UINT32_MAX;
	static constexpr uint64_t HandleDomain = uint64_t{InvalidHandle};

	uint32_t protectionOffset;

	constexpr uint32_t Protect(uint32_t rawHandle) const
	{
		return static_cast<uint32_t>(
			(uint64_t{rawHandle} + protectionOffset) % HandleDomain);
	}

	constexpr uint32_t Unprotect(uint32_t protectedHandle) const
	{
		return static_cast<uint32_t>(
			(uint64_t{protectedHandle} + HandleDomain - protectionOffset) %
			HandleDomain);
	}

	constexpr bool TryResolve(
		uint32_t protectedHandle,
		size_t handleCount,
		uint32_t& rawHandle) const
	{
		if (protectedHandle == InvalidHandle)
			return false;

		rawHandle = Unprotect(protectedHandle);
		return rawHandle < handleCount;
	}
};

static void TestMM05LegacyCodecPreservesOrdinaryRoundTrips(CryTestRunner& runner)
{
	constexpr uintptr_t managerAddress = 0x000001AA12340000ull;
	constexpr std::array<uint32_t, 6> handles = {
		0u,
		1u,
		127u,
		128u,
		0x00ffffffu,
		UINT32_MAX - 1u
	};

	for (uint32_t handle : handles)
	{
		const uint32_t protectedHandle =
			MM05LegacyHandleCodec::Protect(handle, managerAddress);
		CRY_TEST_CHECK(
			runner,
			MM05LegacyHandleCodec::Unprotect(protectedHandle, managerAddress) ==
				handle);
	}
}

static void TestMM05PointerDerivedKeysCollide(CryTestRunner& runner)
{
	constexpr uintptr_t firstManager = 0x000001AA12340000ull;
	constexpr uintptr_t secondManager = 0x000002BB12340000ull;
	constexpr uint32_t rawHandle = 37;

	CRY_TEST_CHECK(
		runner,
		MM05LegacyHandleCodec::ProtectionKey(firstManager) ==
			MM05LegacyHandleCodec::ProtectionKey(secondManager));
	CRY_TEST_CHECK(
		runner,
		MM05LegacyHandleCodec::Protect(rawHandle, firstManager) ==
			MM05LegacyHandleCodec::Protect(rawHandle, secondManager));
}

static void TestMM05LegacyCodecCanProduceInvalidSentinel(CryTestRunner& runner)
{
	constexpr uintptr_t managerAddress = 0x000001AA12340000ull;
	constexpr uint32_t key =
		MM05LegacyHandleCodec::ProtectionKey(managerAddress);
	constexpr uint32_t faultingRawHandle =
		(MM05LegacyHandleCodec::InvalidHandle ^ key) - 0x80u;

	CRY_TEST_CHECK(
		runner,
		faultingRawHandle != MM05LegacyHandleCodec::InvalidHandle);
	CRY_TEST_CHECK(
		runner,
		MM05LegacyHandleCodec::Protect(faultingRawHandle, managerAddress) ==
			MM05LegacyHandleCodec::InvalidHandle);
}

static void TestMM05HandleCountNarrowingAliasesIndexZero(CryTestRunner& runner)
{
	constexpr size_t firstUnrepresentableHandleCount =
		static_cast<size_t>(UINT32_MAX) + 1;
	constexpr uint32_t narrowedHandle =
		static_cast<uint32_t>(firstUnrepresentableHandleCount);

	CRY_TEST_CHECK(runner, narrowedHandle == 0);
	CRY_TEST_CHECK(
		runner,
		static_cast<size_t>(narrowedHandle) != firstUnrepresentableHandleCount);
}

static void TestMM05UncheckedIndexRequiresRangeValidation(CryTestRunner& runner)
{
	constexpr uintptr_t managerAddress = 0x000001AA12340000ull;
	constexpr size_t handleCount = 8;
	constexpr uint32_t outOfRangeRawHandle = 4096;
	constexpr uint32_t protectedHandle =
		MM05LegacyHandleCodec::Protect(outOfRangeRawHandle, managerAddress);
	constexpr uint32_t decodedHandle =
		MM05LegacyHandleCodec::Unprotect(protectedHandle, managerAddress);

	CRY_TEST_CHECK(runner, decodedHandle == outOfRangeRawHandle);
	CRY_TEST_CHECK(runner, decodedHandle >= handleCount);

	constexpr MM05ReservedDomainCodec repairedCodec{0x12345u};
	uint32_t resolvedHandle = 0;
	const uint32_t repairedProtectedHandle =
		repairedCodec.Protect(outOfRangeRawHandle);
	CRY_TEST_CHECK(
		runner,
		!repairedCodec.TryResolve(
			repairedProtectedHandle,
			handleCount,
			resolvedHandle));
}

static void TestMM05LeakAccountingWrapsAtFourGiB(CryTestRunner& runner)
{
	constexpr uint64_t firstAllocation = uint64_t{3} * 1024 * 1024 * 1024;
	constexpr uint64_t secondAllocation = uint64_t{1} * 1024 * 1024 * 1024 + 4096;
	constexpr uint64_t expectedTotal = firstAllocation + secondAllocation;
	constexpr uint32_t legacyTotal =
		static_cast<uint32_t>(firstAllocation) +
		static_cast<uint32_t>(secondAllocation);

	CRY_TEST_CHECK(runner, expectedTotal > UINT32_MAX);
	CRY_TEST_CHECK(runner, legacyTotal == 4096);
	CRY_TEST_CHECK(runner, static_cast<uint64_t>(legacyTotal) != expectedTotal);
}

static void TestMM05ReservedDomainPreservesRequiredBehavior(CryTestRunner& runner)
{
	constexpr MM05ReservedDomainCodec codec{0x12345u};
	constexpr std::array<uint32_t, 6> handles = {
		0u,
		1u,
		127u,
		128u,
		0x00ffffffu,
		UINT32_MAX - 1u
	};

	for (uint32_t handle : handles)
	{
		const uint32_t protectedHandle = codec.Protect(handle);
		CRY_TEST_CHECK(
			runner,
			protectedHandle != MM05ReservedDomainCodec::InvalidHandle);
		CRY_TEST_CHECK(runner, codec.Unprotect(protectedHandle) == handle);
	}

	uint32_t rawHandle = 0;
	CRY_TEST_CHECK(
		runner,
		!codec.TryResolve(
			MM05ReservedDomainCodec::InvalidHandle,
			handles.size(),
			rawHandle));
}

static void TestMM05CheckedAccountingRejectsOverflow(CryTestRunner& runner)
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	size_t updatedTotal = 0;

	CRY_TEST_CHECK(runner, CryTryAddSize(maximum - 64, 64, updatedTotal));
	CRY_TEST_CHECK(runner, updatedTotal == maximum);
	CRY_TEST_CHECK(runner, !CryTryAddSize(maximum - 63, 64, updatedTotal));
	CRY_TEST_CHECK(runner, updatedTotal == 0);
}

void RunMM05ModelTests(CryTestRunner& runner)
{
	runner.Run("MM05 legacy codec preserves ordinary round trips", TestMM05LegacyCodecPreservesOrdinaryRoundTrips);
	runner.Run("MM05 pointer-derived manager keys collide on Win64", TestMM05PointerDerivedKeysCollide);
	runner.Run("MM05 legacy codec can produce InvalidHdl", TestMM05LegacyCodecCanProduceInvalidSentinel);
	runner.Run("MM05 handle count narrowing aliases index zero", TestMM05HandleCountNarrowingAliasesIndexZero);
	runner.Run("MM05 handle resolution requires a range check", TestMM05UncheckedIndexRequiresRangeValidation);
	runner.Run("MM05 legacy leak accounting wraps above four GiB", TestMM05LeakAccountingWrapsAtFourGiB);
	runner.Run("MM05 reserved-domain codec preserves handle invariants", TestMM05ReservedDomainPreservesRequiredBehavior);
	runner.Run("MM05 checked accounting rejects overflow", TestMM05CheckedAccountingRejectsOverflow);
}

int RunMM05Trace()
{
	CryTestRunner runner;
	RunMM05ModelTests(runner);
	return runner.ExitCode();
}
