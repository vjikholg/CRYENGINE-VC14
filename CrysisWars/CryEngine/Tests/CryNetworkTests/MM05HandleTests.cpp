#include "CryNetworkTestPreamble.h"
#include "MM05HandleTests.h"
#include "TestRunner.h"

#include <MementoMemoryManager.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

static_assert(sizeof(CMementoMemoryManager::Hdl) == sizeof(uint64_t));
static_assert(std::is_same_v<CMementoMemoryManager::Hdl, uint64_t>);

static void TestMM05ValidHandleRoundTrip(CryTestRunner& runner)
{
	CMementoMemoryManager manager;
	const CMementoMemoryManager::Hdl handle = manager.AllocHdl(64);

	CRY_TEST_CHECK(runner, handle != CMementoMemoryManager::InvalidHdl);
	CRY_TEST_CHECK(runner, manager.IsValidHdl(handle));
	CRY_TEST_CHECK(runner, manager.GetHdlSize(handle) == 64);
	CRY_TEST_CHECK(runner, manager.PinHdl(handle) != nullptr);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 64);

	manager.FreeHdl(handle);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 0);
}

static void TestMM05ForgedIndexIsRejected(CryTestRunner& runner)
{
	CMementoMemoryManager manager;
	const CMementoMemoryManager::Hdl validHandle = manager.AllocHdl(32);
	const CMementoMemoryManager::Hdl forgedHandle =
		(validHandle & 0xffffffff00000000ull) | 0x00ffffffull;

	CRY_TEST_CHECK(runner, !manager.IsValidHdl(forgedHandle));
	CRY_TEST_CHECK(runner, manager.PinHdl(forgedHandle) == nullptr);
	CRY_TEST_CHECK(runner, manager.GetHdlSize(forgedHandle) == 0);

	manager.ResizeHdl(forgedHandle, 4096);
	manager.FreeHdl(forgedHandle);
	CRY_TEST_CHECK(runner, manager.IsValidHdl(validHandle));
	CRY_TEST_CHECK(runner, manager.GetHdlSize(validHandle) == 32);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 32);

	manager.FreeHdl(validHandle);
}

static void TestMM05WrongManagerHandleIsRejected(CryTestRunner& runner)
{
	CMementoMemoryManager owner;
	CMementoMemoryManager other;
	const CMementoMemoryManager::Hdl handle = owner.AllocHdl(48);
	const CMementoMemoryManager::Hdl otherHandle = other.AllocHdl(48);

	CRY_TEST_CHECK(runner, owner.IsValidHdl(handle));
	CRY_TEST_CHECK(runner, other.IsValidHdl(otherHandle));
	CRY_TEST_CHECK(runner, handle != otherHandle);
	CRY_TEST_CHECK(runner, !other.IsValidHdl(handle));
	CRY_TEST_CHECK(runner, other.PinHdl(handle) == nullptr);
	CRY_TEST_CHECK(runner, other.GetHdlSize(handle) == 0);

	other.FreeHdl(handle);
	CRY_TEST_CHECK(runner, owner.GetLiveAllocationBytes() == 48);
	CRY_TEST_CHECK(runner, other.GetLiveAllocationBytes() == 48);

	other.FreeHdl(otherHandle);
	owner.FreeHdl(handle);
}

static void TestMM05FreedHandleCannotReachReusedSlot(CryTestRunner& runner)
{
	CMementoMemoryManager manager;
	const CMementoMemoryManager::Hdl oldHandle = manager.AllocHdl(64);
	void* const oldAddress = manager.PinHdl(oldHandle);
	manager.FreeHdl(oldHandle);

	CRY_TEST_CHECK(runner, !manager.IsValidHdl(oldHandle));
	CRY_TEST_CHECK(runner, manager.PinHdl(oldHandle) == nullptr);
	CRY_TEST_CHECK(runner, manager.GetHdlSize(oldHandle) == 0);

	manager.FreeHdl(oldHandle);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 0);

	const CMementoMemoryManager::Hdl newHandle = manager.AllocHdl(64);
	CRY_TEST_CHECK(runner, newHandle != oldHandle);
	CRY_TEST_CHECK(runner, manager.IsValidHdl(newHandle));
	CRY_TEST_CHECK(runner, !manager.IsValidHdl(oldHandle));
	CRY_TEST_CHECK(runner, manager.PinHdl(newHandle) == oldAddress);

	manager.FreeHdl(newHandle);
}

static void TestMM05ResizeAndClonePreservePayload(CryTestRunner& runner)
{
	CMementoMemoryManager manager;
	const CMementoMemoryManager::Hdl handle = manager.AllocHdl(32);
	auto* bytes = static_cast<std::byte*>(manager.PinHdl(handle));
	for (size_t index = 0; index < 32; ++index)
		bytes[index] = static_cast<std::byte>(index ^ 0x5a);

	manager.ResizeHdl(handle, 512);
	CRY_TEST_CHECK(runner, manager.IsValidHdl(handle));
	CRY_TEST_CHECK(runner, manager.GetHdlSize(handle) == 512);
	bytes = static_cast<std::byte*>(manager.PinHdl(handle));
	for (size_t index = 0; index < 32; ++index)
	{
		CRY_TEST_CHECK(
			runner,
			bytes[index] == static_cast<std::byte>(index ^ 0x5a));
	}

	const CMementoMemoryManager::Hdl clone = manager.CloneHdl(handle);
	CRY_TEST_CHECK(runner, manager.IsValidHdl(clone));
	CRY_TEST_CHECK(runner, manager.GetHdlSize(clone) == 512);
	CRY_TEST_CHECK(
		runner,
		std::memcmp(manager.PinHdl(handle), manager.PinHdl(clone), 512) == 0);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 1024);

	manager.FreeHdl(clone);
	manager.FreeHdl(handle);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 0);
}

static void TestMM05InvalidSentinelIsAlwaysRejected(CryTestRunner& runner)
{
	CMementoMemoryManager manager;
	CRY_TEST_CHECK(
		runner,
		!manager.IsValidHdl(CMementoMemoryManager::InvalidHdl));
	CRY_TEST_CHECK(
		runner,
		manager.PinHdl(CMementoMemoryManager::InvalidHdl) == nullptr);
	CRY_TEST_CHECK(
		runner,
		manager.GetHdlSize(CMementoMemoryManager::InvalidHdl) == 0);
	manager.FreeHdl(CMementoMemoryManager::InvalidHdl);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 0);
}

static void TestMM05FailedAllocationRollsBackHandleSlot(CryTestRunner& runner)
{
	CMementoMemoryManager manager;
	const CMementoMemoryManager::Hdl failedHandle = manager.AllocHdl(
		(std::numeric_limits<size_t>::max)());

	CRY_TEST_CHECK(runner, failedHandle == CMementoMemoryManager::InvalidHdl);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 0);

	const CMementoMemoryManager::Hdl validHandle = manager.AllocHdl(64);
	CRY_TEST_CHECK(runner, validHandle != CMementoMemoryManager::InvalidHdl);
	CRY_TEST_CHECK(runner, manager.IsValidHdl(validHandle));
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 64);
	manager.FreeHdl(validHandle);
}

static void TestMM05FailedResizePreservesOriginalAllocation(
	CryTestRunner& runner)
{
	CMementoMemoryManager manager;
	const CMementoMemoryManager::Hdl handle = manager.AllocHdl(64);
	auto* const originalMemory = static_cast<unsigned char*>(manager.PinHdl(handle));
	CRY_TEST_CHECK(runner, originalMemory != nullptr);
	if (originalMemory == nullptr)
		return;

	for (size_t index = 0; index < 64; ++index)
		originalMemory[index] = static_cast<unsigned char>(index ^ 0xa5);

	manager.ResizeHdl(handle, (std::numeric_limits<size_t>::max)());
	CRY_TEST_CHECK(runner, manager.IsValidHdl(handle));
	CRY_TEST_CHECK(runner, manager.PinHdl(handle) == originalMemory);
	CRY_TEST_CHECK(runner, manager.GetHdlSize(handle) == 64);
	CRY_TEST_CHECK(runner, manager.GetLiveAllocationBytes() == 64);
	for (size_t index = 0; index < 64; ++index)
	{
		CRY_TEST_CHECK(
			runner,
			originalMemory[index] == static_cast<unsigned char>(index ^ 0xa5));
	}

	manager.FreeHdl(handle);
}

void RunMM05HandleTests(CryTestRunner& runner)
{
	runner.Run("MM05 valid handle round trip", TestMM05ValidHandleRoundTrip);
	runner.Run("MM05 forged index is rejected", TestMM05ForgedIndexIsRejected);
	runner.Run("MM05 wrong-manager handle is rejected", TestMM05WrongManagerHandleIsRejected);
	runner.Run("MM05 freed handle cannot reach reused slot", TestMM05FreedHandleCannotReachReusedSlot);
	runner.Run("MM05 resize and clone preserve payload", TestMM05ResizeAndClonePreservePayload);
	runner.Run("MM05 invalid sentinel is always rejected", TestMM05InvalidSentinelIsAlwaysRejected);
	runner.Run(
		"MM05 failed allocation rolls back handle slot",
		TestMM05FailedAllocationRollsBackHandleSlot);
	runner.Run(
		"MM05 failed resize preserves original allocation",
		TestMM05FailedResizePreservesOriginalAllocation);
}
