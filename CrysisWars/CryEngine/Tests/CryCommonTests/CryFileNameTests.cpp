#include <platform.h>

#include "CryFileNameTests.h"
#include "TestRunner.h"

#include <CryFile.h>
#include <CryName.h>

#include <cstring>
#include <limits>

static void TestCryFileExtensionBoundaries(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, std::strcmp(CryGetExt(""), "") == 0);
	CRY_TEST_CHECK(runner, std::strcmp(CryGetExt("Objects/vehicle"), "") == 0);
	CRY_TEST_CHECK(runner, std::strcmp(CryGetExt("Objects/vehicle.cgf"), "cgf") == 0);
	CRY_TEST_CHECK(runner, std::strcmp(CryGetExt("Objects/archive.tar.gz"), "gz") == 0);
	CRY_TEST_CHECK(runner, std::strcmp(CryGetExt("Objects.old/vehicle"), "") == 0);
	CRY_TEST_CHECK(runner, std::strcmp(CryGetExt("Objects/vehicle."), "") == 0);
	CRY_TEST_CHECK(runner, std::strcmp(CryGetExt("C:\\Game\\Objects\\vehicle.CGF"), "CGF") == 0);
	CRY_TEST_CHECK(runner, IsCharacterFile("Characters/soldier.CHR"));
	CRY_TEST_CHECK(runner, IsCharacterFile("Characters/soldier.cdf"));
	CRY_TEST_CHECK(runner, IsStatObjFile("Objects/vehicle.CGF"));
	CRY_TEST_CHECK(runner, !IsStatObjFile("Objects/vehicle.cga"));
}

static void TestCryNameEntryAllocationBounds(CryTestRunner& runner)
{
	using SNameEntry = INameTable::SNameEntry;

	size_t allocationSize = 0;
	CRY_TEST_CHECK(runner, SNameEntry::TryGetAllocationSize(0, allocationSize));
	CRY_TEST_CHECK(runner, allocationSize == sizeof(SNameEntry) + 1);

	const size_t maximumNameLength = SNameEntry::MaximumNameLength();
	CRY_TEST_CHECK(
		runner,
		SNameEntry::TryGetAllocationSize(maximumNameLength, allocationSize));
	CRY_TEST_CHECK(
		runner,
		allocationSize == static_cast<size_t>((std::numeric_limits<int>::max)()));

	CRY_TEST_CHECK(
		runner,
		!SNameEntry::TryGetAllocationSize(maximumNameLength + 1, allocationSize));
	CRY_TEST_CHECK(runner, allocationSize == 0);
}

static void TestCryNameTableEntryInvariants(CryTestRunner& runner)
{
	CNameTable table;
	const int emptyMemoryUsage = table.GetMemoryUsage();

	INameTable::SNameEntry* const first = table.GetEntry("Vehicle");
	CRY_TEST_CHECK(runner, first != nullptr);
	if (first == nullptr)
		return;

	CRY_TEST_CHECK(runner, std::strcmp(first->GetStr(), "Vehicle") == 0);
	CRY_TEST_CHECK(runner, first->nLength == 7);
	CRY_TEST_CHECK(
		runner,
		first->nAllocSize == static_cast<int>(sizeof(INameTable::SNameEntry) + 8));
	CRY_TEST_CHECK(
		runner,
		first->GetMemoryUsage() == sizeof(INameTable::SNameEntry) + 7);

	INameTable::SNameEntry* const duplicate = table.GetEntry("vehicle");
	CRY_TEST_CHECK(runner, duplicate == first);
	CRY_TEST_CHECK(runner, table.FindEntry("VEHICLE") == first);

	const int populatedMemoryUsage = table.GetMemoryUsage();
	CRY_TEST_CHECK(runner, populatedMemoryUsage > emptyMemoryUsage);
	CRY_TEST_CHECK(runner, table.GetMemoryUsage() == populatedMemoryUsage);

	table.Release(first);
	CRY_TEST_CHECK(runner, table.FindEntry("vehicle") == nullptr);
	CRY_TEST_CHECK(runner, table.GetMemoryUsage() == emptyMemoryUsage);
}

void RunCryFileNameTests(CryTestRunner& runner)
{
	runner.Run("CryFile extension parsing boundaries", TestCryFileExtensionBoundaries);
	runner.Run("CryName entry allocation bounds", TestCryNameEntryAllocationBounds);
	runner.Run("CryName table entry invariants", TestCryNameTableEntryInvariants);
}
