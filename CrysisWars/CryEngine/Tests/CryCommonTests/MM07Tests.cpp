#include "MM07Tests.h"
#include "TestRunner.h"

#include <CryMemoryCoverage.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

static HMODULE g_mm07CrySystemModule = nullptr;

static void TestMM07LegacyFourGiBWindowCollapsesWin64Addresses(
	CryTestRunner& runner)
{
	constexpr uintptr_t legacyAddressSpace = uintptr_t{1} << 32;
	constexpr size_t unitCount = 8192;
	constexpr uintptr_t bytesPerUnit = legacyAddressSpace / unitCount;
	constexpr uintptr_t firstAddress = uintptr_t{1} << 40;
	constexpr uintptr_t secondAddress = firstAddress + (uintptr_t{1} << 30);

	const size_t firstLegacyUnit = (std::min)(
		static_cast<size_t>(firstAddress / bytesPerUnit),
		unitCount);
	const size_t secondLegacyUnit = (std::min)(
		static_cast<size_t>(secondAddress / bytesPerUnit),
		unitCount);

	CRY_TEST_CHECK(runner, firstLegacyUnit == unitCount);
	CRY_TEST_CHECK(runner, secondLegacyUnit == unitCount);
}

static void TestMM07RelativeCoverageKeepsHighRangesDistinct(
	CryTestRunner& runner)
{
	constexpr uintptr_t firstAddress = uintptr_t{1} << 40;
	constexpr uintptr_t secondAddress = firstAddress + (uintptr_t{1} << 30);
	const std::vector<SCryMemoryCoverageRange> ranges = {
		{ firstAddress, 64 * 1024 },
		{ secondAddress, 64 * 1024 }
	};
	std::vector<bool> coverage;
	uintptr_t coverageBegin = 0;
	uintptr_t coverageEnd = 0;
	size_t bytesPerUnit = 0;

	CRY_TEST_CHECK(
		runner,
		CryBuildMemoryCoverage(
			ranges,
			8192,
			coverage,
			coverageBegin,
			coverageEnd,
			bytesPerUnit));
	CRY_TEST_CHECK(runner, coverageBegin == firstAddress);
	CRY_TEST_CHECK(runner, coverageEnd == secondAddress + 64 * 1024);
	CRY_TEST_CHECK(runner, bytesPerUnit != 0);
	CRY_TEST_CHECK(runner, coverage.front());
	CRY_TEST_CHECK(runner, coverage.back());
	CRY_TEST_CHECK(
		runner,
		std::count(coverage.begin(), coverage.end(), true) >= 2);
}

static void TestMM07CoverageRejectsOverflowingRanges(CryTestRunner& runner)
{
	constexpr uintptr_t maximum = (std::numeric_limits<uintptr_t>::max)();
	const std::vector<SCryMemoryCoverageRange> ranges = {
		{ maximum - 7, 16 },
		{ uintptr_t{1} << 40, 4096 }
	};
	std::vector<bool> coverage;
	uintptr_t coverageBegin = 0;
	uintptr_t coverageEnd = 0;
	size_t bytesPerUnit = 0;

	CRY_TEST_CHECK(
		runner,
		CryBuildMemoryCoverage(
			ranges,
			128,
			coverage,
			coverageBegin,
			coverageEnd,
			bytesPerUnit));
	CRY_TEST_CHECK(runner, coverageBegin == (uintptr_t{1} << 40));
	CRY_TEST_CHECK(runner, coverageEnd == (uintptr_t{1} << 40) + 4096);
	CRY_TEST_CHECK(runner, std::count(coverage.begin(), coverage.end(), true) == 128);
}

static void TestMM07CoverageRejectsEmptyInput(CryTestRunner& runner)
{
	std::vector<bool> coverage;
	uintptr_t coverageBegin = 1;
	uintptr_t coverageEnd = 1;
	size_t bytesPerUnit = 1;

	CRY_TEST_CHECK(
		runner,
		!CryBuildMemoryCoverage(
			{},
			8192,
			coverage,
			coverageBegin,
			coverageEnd,
			bytesPerUnit));
	CRY_TEST_CHECK(runner, coverage.empty());
	CRY_TEST_CHECK(runner, coverageBegin == 0);
	CRY_TEST_CHECK(runner, coverageEnd == 0);
	CRY_TEST_CHECK(runner, bytesPerUnit == 0);
}

static void TestMM07BitmapSizeUsesThreeBytesPerPixel(CryTestRunner& runner)
{
	size_t imageSize = 0;
	CRY_TEST_CHECK(
		runner,
		CryTryCalculateMemoryCoverageBitmapSize(1024, 128, imageSize));
	CRY_TEST_CHECK(runner, imageSize == 1024 * 128 * 3);

	const size_t maximum = (std::numeric_limits<size_t>::max)();
	CRY_TEST_CHECK(
		runner,
		!CryTryCalculateMemoryCoverageBitmapSize(maximum, 2, imageSize));
	CRY_TEST_CHECK(runner, imageSize == 0);
}

static void TestMM07BitmapRowsAreBoundedAndChecked(CryTestRunner& runner)
{
	constexpr size_t headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	size_t rowOffset = 0;
	CRY_TEST_CHECK(
		runner,
		CryTryCalculateMemoryCoverageRowOffset(1024, 128, 0, headerSize, rowOffset));
	CRY_TEST_CHECK(runner, rowOffset == headerSize + 127 * 1024 * 3);
	CRY_TEST_CHECK(
		runner,
		CryTryCalculateMemoryCoverageRowOffset(1024, 128, 127, headerSize, rowOffset));
	CRY_TEST_CHECK(runner, rowOffset == headerSize);
	CRY_TEST_CHECK(
		runner,
		!CryTryCalculateMemoryCoverageRowOffset(1024, 128, 128, headerSize, rowOffset));
}

static void TestMM07CoverageLabelsMatchCommittedState(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, CryGetMemoryCoverageLabel(0, 8) == '.');
	CRY_TEST_CHECK(runner, CryGetMemoryCoverageLabel(4, 8) == '+');
	CRY_TEST_CHECK(runner, CryGetMemoryCoverageLabel(8, 8) == '#');
}

void RunMM07MemoryCoverageTests(CryTestRunner& runner)
{
	runner.Run(
		"MM07 legacy 4 GiB window collapses Win64 addresses",
		TestMM07LegacyFourGiBWindowCollapsesWin64Addresses);
	runner.Run(
		"MM07 relative coverage keeps high ranges distinct",
		TestMM07RelativeCoverageKeepsHighRangesDistinct);
	runner.Run(
		"MM07 coverage rejects overflowing ranges",
		TestMM07CoverageRejectsOverflowingRanges);
	runner.Run(
		"MM07 coverage rejects empty input",
		TestMM07CoverageRejectsEmptyInput);
	runner.Run(
		"MM07 bitmap size uses three bytes per pixel",
		TestMM07BitmapSizeUsesThreeBytesPerPixel);
	runner.Run(
		"MM07 bitmap rows are bounded and checked",
		TestMM07BitmapRowsAreBoundedAndChecked);
	runner.Run(
		"MM07 labels distinguish free and committed coverage",
		TestMM07CoverageLabelsMatchCommittedState);
}

static void TestMM07RuntimeModuleRangeCanBeMapped(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, g_mm07CrySystemModule != nullptr);
	if (g_mm07CrySystemModule == nullptr)
		return;

	MEMORY_BASIC_INFORMATION memoryInfo = {};
	CRY_TEST_CHECK(
		runner,
		VirtualQuery(
			g_mm07CrySystemModule,
			&memoryInfo,
			sizeof(memoryInfo)) == sizeof(memoryInfo));
	if (memoryInfo.RegionSize == 0)
		return;

	const std::vector<SCryMemoryCoverageRange> ranges = {
		{
			reinterpret_cast<uintptr_t>(memoryInfo.BaseAddress),
			memoryInfo.RegionSize
		}
	};
	std::vector<bool> coverage;
	uintptr_t coverageBegin = 0;
	uintptr_t coverageEnd = 0;
	size_t bytesPerUnit = 0;
	CRY_TEST_CHECK(
		runner,
		CryBuildMemoryCoverage(
			ranges,
			128,
			coverage,
			coverageBegin,
			coverageEnd,
			bytesPerUnit));
	CRY_TEST_CHECK(
		runner,
		coverageBegin == reinterpret_cast<uintptr_t>(memoryInfo.BaseAddress));
	CRY_TEST_CHECK(runner, coverageEnd > coverageBegin);
	CRY_TEST_CHECK(runner, bytesPerUnit != 0);
	CRY_TEST_CHECK(runner, std::count(coverage.begin(), coverage.end(), true) == 128);
}

int RunMM07RuntimeTrace(const wchar_t* crySystemPath)
{
	if (crySystemPath == nullptr)
		return 3;

	g_mm07CrySystemModule = LoadLibraryW(crySystemPath);
	if (g_mm07CrySystemModule == nullptr)
		return 3;

	CryTestRunner runner;
	runner.Run(
		"MM07 runtime CrySystem range uses checked coverage mapping",
		TestMM07RuntimeModuleRangeCanBeMapped);
	const int result = runner.ExitCode();
	FreeLibrary(g_mm07CrySystemModule);
	g_mm07CrySystemModule = nullptr;
	return result;
}
