#include "CryNetworkTestPreamble.h"
#include "NetworkIdAndIndexTests.h"
#include "TestRunner.h"

#include <StdAfx.h>
#include <INetwork.h>
#include <Context/ChangeList.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

static_assert(sizeof(SNetObjectID) == sizeof(uint32));
static_assert(std::is_same_v<decltype(SNetObjectID::id), uint16>);
static_assert(std::is_same_v<decltype(SNetObjectID::salt), uint16>);
static_assert(offsetof(SNetObjectID, id) == 0);
static_assert(offsetof(SNetObjectID, salt) == sizeof(uint16));
static_assert(sizeof(SSendableHandle) == sizeof(uint64));
static_assert(std::is_same_v<decltype(SSendableHandle::id), uint32>);
static_assert(std::is_same_v<decltype(SSendableHandle::salt), uint32>);
static_assert(offsetof(SSendableHandle, id) == 0);
static_assert(offsetof(SSendableHandle, salt) == sizeof(uint32));

static void TestNetObjectIdSentinelValueAndSaltSemantics(CryTestRunner& runner)
{
	const SNetObjectID sentinel;
	const SNetObjectID zeroValue(0, 0);
	const SNetObjectID minimumValue(0, 1);
	const SNetObjectID maximumValue(
		static_cast<uint16>(SNetObjectID::InvalidId - 1),
		(std::numeric_limits<uint16>::max)());
	const SNetObjectID sentinelWithSalt(SNetObjectID::InvalidId, 7);
	SNetObjectID assignedValue;
	assignedValue = maximumValue;
	char text[64];

	CRY_TEST_CHECK(runner, !sentinel);
	CRY_TEST_CHECK(runner, !static_cast<bool>(sentinel));
	CRY_TEST_CHECK(runner, sentinel.id == SNetObjectID::InvalidId);
	CRY_TEST_CHECK(runner, sentinel.salt == 0);
	CRY_TEST_CHECK(runner, !sentinel.IsLegal());
	CRY_TEST_CHECK(runner, sentinel.GetAsUint32() == 0x0000ffffu);
	CRY_TEST_CHECK(runner, std::strcmp(sentinel.GetText(text), "<nil>") == 0);

	CRY_TEST_CHECK(runner, static_cast<bool>(zeroValue));
	CRY_TEST_CHECK(runner, !zeroValue.IsLegal());
	CRY_TEST_CHECK(runner, zeroValue.GetAsUint32() == 0u);
	CRY_TEST_CHECK(runner, std::strcmp(zeroValue.GetText(text), "illegal:0:0") == 0);
	CRY_TEST_CHECK(runner, static_cast<bool>(minimumValue));
	CRY_TEST_CHECK(runner, minimumValue.IsLegal());
	CRY_TEST_CHECK(runner, minimumValue.GetAsUint32() == 0x00010000u);
	CRY_TEST_CHECK(runner, std::strcmp(minimumValue.GetText(text), "0:1") == 0);

	CRY_TEST_CHECK(runner, !sentinelWithSalt);
	CRY_TEST_CHECK(runner, sentinelWithSalt.IsLegal());
	CRY_TEST_CHECK(runner, maximumValue.IsLegal());
	CRY_TEST_CHECK(runner, maximumValue.GetAsUint32() == 0xfffffffeu);
	CRY_TEST_CHECK(runner, assignedValue == maximumValue);
	CRY_TEST_CHECK(runner, std::strcmp(maximumValue.GetText(text), "65534:65535") == 0);
	CRY_TEST_CHECK(runner, minimumValue < maximumValue);
	CRY_TEST_CHECK(runner, maximumValue > minimumValue);
	CRY_TEST_CHECK(runner, maximumValue < sentinelWithSalt);
}

static void TestSendableHandleRepresentationAndSemantics(CryTestRunner& runner)
{
	const SSendableHandle sentinel;
	SSendableHandle idOnly;
	idOnly.id = 1;
	SSendableHandle saltOnly;
	saltOnly.salt = 1;
	SSendableHandle lower;
	lower.id = 7;
	lower.salt = (std::numeric_limits<uint32>::max)();
	SSendableHandle higher;
	higher.id = 8;
	SSendableHandle assigned;
	assigned = lower;
	char text[64];

	CRY_TEST_CHECK(runner, !sentinel);
	CRY_TEST_CHECK(runner, !static_cast<bool>(sentinel));
	CRY_TEST_CHECK(runner, static_cast<bool>(idOnly));
	CRY_TEST_CHECK(runner, static_cast<bool>(saltOnly));
	CRY_TEST_CHECK(runner, assigned == lower);
	CRY_TEST_CHECK(runner, assigned != higher);
	CRY_TEST_CHECK(runner, lower < higher);
	CRY_TEST_CHECK(runner, !(higher < lower));
	CRY_TEST_CHECK(
		runner,
		std::strcmp(lower.GetText(text), "00000007:ffffffff") == 0);
}

static void TestChangeListAddUpdateRemoveAndFlush(CryTestRunner& runner)
{
	CChangeList<uint32> changes;
	const SNetObjectID first(7, 1);
	const SNetObjectID second(11, 2);
	const SNetObjectID third(23, 3);

	CRY_TEST_CHECK(runner, !changes.IsChanged(SNetObjectID(), 0x01u));
	CRY_TEST_CHECK(runner, changes.AddChange(first, 0x01u));
	CRY_TEST_CHECK(runner, !changes.AddChange(first, 0x04u));
	CRY_TEST_CHECK(runner, changes.AddChange(second, 0x10u));
	CRY_TEST_CHECK(runner, changes.GetObjects().size() == 2);
	CRY_TEST_CHECK(runner, changes.GetObjects()[0].first == first);
	CRY_TEST_CHECK(runner, changes.GetObjects()[0].second == 0x05u);
	CRY_TEST_CHECK(runner, changes.GetObjects()[1].first == second);
	CRY_TEST_CHECK(runner, changes.GetObjects()[1].second == 0x10u);
	CRY_TEST_CHECK(runner, changes.IsChanged(first, 0x01u));
	CRY_TEST_CHECK(runner, changes.IsChanged(first, 0x04u));
	CRY_TEST_CHECK(runner, !changes.IsChanged(first, 0x02u));

	changes.Remove(first);
	CRY_TEST_CHECK(runner, changes.GetObjects().size() == 1);
	CRY_TEST_CHECK(runner, changes.GetObjects()[0].first == second);
	CRY_TEST_CHECK(runner, changes.IsChanged(second, 0x10u));
	CRY_TEST_CHECK(runner, !changes.AddChange(second, 0x20u));
	CRY_TEST_CHECK(runner, changes.GetObjects()[0].second == 0x30u);

	CRY_TEST_CHECK(runner, changes.AddChange(third, 0x08u));
	CRY_TEST_CHECK(runner, changes.GetObjects().size() == 2);
	changes.Remove(second);
	CRY_TEST_CHECK(runner, changes.GetObjects().size() == 1);
	CRY_TEST_CHECK(runner, changes.GetObjects()[0].first == third);
	CRY_TEST_CHECK(runner, changes.IsChanged(third, 0x08u));
	CRY_TEST_CHECK(runner, !changes.AddChange(third, 0x04u));
	CRY_TEST_CHECK(runner, changes.GetObjects()[0].second == 0x0cu);

	changes.Remove(third);
	CRY_TEST_CHECK(runner, changes.GetObjects().empty());
	changes.Flush();
	CRY_TEST_CHECK(runner, changes.GetObjects().empty());
	CRY_TEST_CHECK(runner, !changes.IsChanged(second, 0x30u));
	CRY_TEST_CHECK(runner, changes.AddChange(second, 0x40u));
	CRY_TEST_CHECK(runner, changes.GetObjects().size() == 1);
	CRY_TEST_CHECK(runner, changes.GetObjects()[0].first == second);
	CRY_TEST_CHECK(runner, changes.GetObjects()[0].second == 0x40u);
}

static void TestChangeListAcceptsEveryRepresentableObjectSlot(CryTestRunner& runner)
{
	CChangeList<uint32> changes;
	const uint32 representableSlotCount = SNetObjectID::InvalidId;

	for (uint32 slot = 0; slot < representableSlotCount; ++slot)
	{
		const SNetObjectID id(static_cast<uint16>(slot), 1);
		CRY_TEST_CHECK(runner, changes.AddChange(id, 0x01u));
	}

	const SNetObjectID first(0, 1);
	const SNetObjectID last(
		static_cast<uint16>(representableSlotCount - 1),
		1);
	CRY_TEST_CHECK(runner, changes.GetObjects().size() == representableSlotCount);
	CRY_TEST_CHECK(runner, changes.GetObjects().front().first == first);
	CRY_TEST_CHECK(runner, changes.GetObjects().back().first == last);
	CRY_TEST_CHECK(runner, changes.IsChanged(first, 0x01u));
	CRY_TEST_CHECK(runner, changes.IsChanged(last, 0x01u));
	CRY_TEST_CHECK(runner, !changes.IsChanged(SNetObjectID(), 0x01u));

	changes.Flush();
	CRY_TEST_CHECK(runner, changes.GetObjects().empty());
	CRY_TEST_CHECK(runner, changes.AddChange(last, 0x02u));
	CRY_TEST_CHECK(runner, changes.GetObjects().size() == 1);
	CRY_TEST_CHECK(runner, changes.GetObjects().front().first == last);
	CRY_TEST_CHECK(runner, changes.GetObjects().front().second == 0x02u);
}

void RunNetworkIdAndIndexTests(CryTestRunner& runner)
{
	runner.Run(
		"SNetObjectID sentinel, value, and salt semantics",
		TestNetObjectIdSentinelValueAndSaltSemantics);
	runner.Run(
		"SSendableHandle representation and sentinel semantics",
		TestSendableHandleRepresentationAndSemantics);
	runner.Run(
		"ChangeList add, update, remove, and flush mappings",
		TestChangeListAddUpdateRemoveAndFlush);
	runner.Run(
		"ChangeList accepts every representable object slot",
		TestChangeListAcceptsEveryRepresentableObjectSlot);
}
