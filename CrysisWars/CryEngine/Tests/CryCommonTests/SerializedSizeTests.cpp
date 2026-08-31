#include "SerializedSizeTests.h"
#include "TestRunner.h"

#include <CryCheckedArithmetic.h>
#include <CryHeaders.h>

#include <cstdint>
#include <limits>

static void TestSerializedSizeCheckedNarrowing(CryTestRunner& runner)
{
	uint32_t count32 = 99;
	int32_t signedCount = 99;
	uint16_t index16 = 99;

	CRY_TEST_CHECK(runner, CryTryNarrowSize(size_t{0}, count32));
	CRY_TEST_CHECK(runner, count32 == 0);
	CRY_TEST_CHECK(runner, CryTryNarrowSize(UINT32_MAX, count32));
	CRY_TEST_CHECK(runner, count32 == UINT32_MAX);
	CRY_TEST_CHECK(runner, !CryTryNarrowSize(size_t{UINT32_MAX} + 1, count32));
	CRY_TEST_CHECK(runner, count32 == 0);

	CRY_TEST_CHECK(runner, CryTryNarrowSize(size_t{INT32_MAX}, signedCount));
	CRY_TEST_CHECK(runner, signedCount == INT32_MAX);
	CRY_TEST_CHECK(runner, !CryTryNarrowSize(size_t{INT32_MAX} + 1, signedCount));
	CRY_TEST_CHECK(runner, signedCount == 0);

	CRY_TEST_CHECK(runner, CryTryNarrowSize(size_t{UINT16_MAX}, index16));
	CRY_TEST_CHECK(runner, index16 == UINT16_MAX);
	CRY_TEST_CHECK(runner, !CryTryNarrowSize(size_t{UINT16_MAX} + 1, index16));
	CRY_TEST_CHECK(runner, index16 == 0);
	CRY_TEST_CHECK(runner, CrySaturateSize<int32_t>(size_t{INT32_MAX} + 1) == INT32_MAX);
}

static void TestSerializedSizeCheckedRecordArithmetic(CryTestRunner& runner)
{
	size_t payloadBytes = 0;
	size_t totalBytes = 0;
	constexpr size_t headerBytes = 64;

	CRY_TEST_CHECK(runner, CryTryMultiplySize(128, 32, payloadBytes));
	CRY_TEST_CHECK(runner, payloadBytes == 4096);
	CRY_TEST_CHECK(runner, CryTryAddSize(headerBytes, payloadBytes, totalBytes));
	CRY_TEST_CHECK(runner, totalBytes == 4160);

	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	CRY_TEST_CHECK(runner, !CryTryMultiplySize(maximum / 32 + 1, 32, payloadBytes));
	CRY_TEST_CHECK(runner, !CryTryAddSize(maximum - 31, 32, totalBytes));
}

static void TestSerializedSizeRangeValidation(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, CryIsRangeWithinSize(1024, 0, 1024));
	CRY_TEST_CHECK(runner, CryIsRangeWithinSize(1024, 512, 512));
	CRY_TEST_CHECK(runner, CryIsRangeWithinSize(1024, 1024, 0));
	CRY_TEST_CHECK(runner, !CryIsRangeWithinSize(1024, 1025, 0));
	CRY_TEST_CHECK(runner, !CryIsRangeWithinSize(1024, 512, 513));

	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	CRY_TEST_CHECK(runner, CryIsRangeWithinSize(maximum, maximum - 7, 7));
	CRY_TEST_CHECK(runner, !CryIsRangeWithinSize(maximum, maximum - 7, 8));
}

static void TestSerializedSizeArrayRangeValidation(CryTestRunner& runner)
{
	size_t rangeEnd = 0;
	CRY_TEST_CHECK(runner, CryTryGetArrayRangeEnd(100, 20, 10, 8, rangeEnd));
	CRY_TEST_CHECK(runner, rangeEnd == 100);

	CRY_TEST_CHECK(runner, !CryTryGetArrayRangeEnd(99, 20, 10, 8, rangeEnd));
	CRY_TEST_CHECK(runner, rangeEnd == 0);
	CRY_TEST_CHECK(runner, !CryTryGetArrayRangeEnd(
		(std::numeric_limits<size_t>::max)(),
		0,
		(std::numeric_limits<size_t>::max)(),
		2,
		rangeEnd));
}

static void TestBonePhysicsSeparatesSerializedIdFromRuntimePointer(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, sizeof(BONE_PHYSICS_COMP) == 104);
	CRY_TEST_CHECK(runner, sizeof(CryBoneDescData_Comp) == 584);

	BONE_PHYSICS_COMP serialized = {};
	serialized.nPhysGeom = 37;
	serialized.flags = 0x1234;

	BONE_PHYSICS runtime = {};
	CopyPhysInfo(runtime, serialized);

	CRY_TEST_CHECK(runner, runtime.nPhysGeomId == 37);
	CRY_TEST_CHECK(runner, runtime.pPhysGeom == nullptr);
	CRY_TEST_CHECK(runner, runtime.flags == serialized.flags);

	BONE_PHYSICS_COMP roundTrip = {};
	runtime.pPhysGeom = reinterpret_cast<phys_geometry*>(uintptr_t{0x10000});
	CopyPhysInfo(roundTrip, runtime);

	CRY_TEST_CHECK(runner, roundTrip.nPhysGeom == 37);
	CRY_TEST_CHECK(runner, roundTrip.flags == serialized.flags);
}

void RunSerializedSizeTests(CryTestRunner& runner)
{
	runner.Run("Serialized sizes: checked narrowing", TestSerializedSizeCheckedNarrowing);
	runner.Run("Serialized sizes: checked record arithmetic", TestSerializedSizeCheckedRecordArithmetic);
	runner.Run("Serialized sizes: range validation", TestSerializedSizeRangeValidation);
	runner.Run("Serialized sizes: array range validation", TestSerializedSizeArrayRangeValidation);
	runner.Run("Serialized sizes: bone physics id/pointer separation", TestBonePhysicsSeparatesSerializedIdFromRuntimePointer);
}
