#include "MM09Tests.h"
#include "TestRunner.h"

#define MEM_MAN_ADD_SIZE_BLOCK 1
#include <CryCrtAllocationSize.h>
#undef MEM_MAN_ADD_SIZE_BLOCK

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdlib>
#include <limits>
#include <type_traits>

typedef void* (__cdecl* MM09CrySystemCrtMallocFunction)(size_t size);
typedef void (__cdecl* MM09CrySystemCrtFreeFunction)(void* memory);
typedef size_t (__cdecl* MM09CrySystemCrtSizeFunction)(void* memory);

static MM09CrySystemCrtMallocFunction g_mm09MallocFunction = nullptr;
static MM09CrySystemCrtFreeFunction g_mm09FreeFunction = nullptr;
static MM09CrySystemCrtSizeFunction g_mm09SizeFunction = nullptr;

static_assert(
	std::is_same_v<
		decltype(CryGetNativeCrtAllocationSize(static_cast<void*>(nullptr))),
		size_t>,
	"MM09 native CRT size queries must preserve pointer-width sizes.");

static void TestMM09LegacyMetadataTruncatesLargeSizes(CryTestRunner& runner)
{
	constexpr size_t largeSize = static_cast<size_t>(UINT32_MAX) + 4096;
	const uint32_t legacySize = static_cast<uint32_t>(largeSize);

	CRY_TEST_CHECK(runner, static_cast<size_t>(legacySize) != largeSize);
	CRY_TEST_CHECK(runner, sizeof(size_t) == sizeof(void*));
}

static void TestMM09NativeCrtSizeUsesTheAllocationBase(CryTestRunner& runner)
{
	constexpr size_t requestedSize = 4096;
	void* const memory = std::malloc(requestedSize);
	CRY_TEST_CHECK(runner, memory != nullptr);
	if (memory == nullptr)
		return;

	const size_t measuredSize = CryGetNativeCrtAllocationSize(memory);
	CRY_TEST_CHECK(runner, measuredSize >= requestedSize);
	std::free(memory);
}

static void TestMM09NullCrtSizeIsZero(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, CryGetNativeCrtAllocationSize(nullptr) == 0);
}

void RunMM09CrtSizeTests(CryTestRunner& runner)
{
	runner.Run(
		"MM09 legacy metadata truncates large sizes",
		TestMM09LegacyMetadataTruncatesLargeSizes);
	runner.Run(
		"MM09 native CRT size uses the allocation base",
		TestMM09NativeCrtSizeUsesTheAllocationBase);
	runner.Run("MM09 null CRT size is zero", TestMM09NullCrtSizeIsZero);
}

static void TestMM09RuntimeCrtExportsAreComplete(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, g_mm09MallocFunction != nullptr);
	CRY_TEST_CHECK(runner, g_mm09FreeFunction != nullptr);
	CRY_TEST_CHECK(runner, g_mm09SizeFunction != nullptr);
}

static void TestMM09RuntimeCrtSizeRoundTrip(CryTestRunner& runner)
{
	if (g_mm09MallocFunction == nullptr ||
		g_mm09FreeFunction == nullptr ||
		g_mm09SizeFunction == nullptr)
	{
		CRY_TEST_CHECK(runner, false);
		return;
	}

	constexpr size_t requestedSize = 8192;
	void* const memory = g_mm09MallocFunction(requestedSize);
	CRY_TEST_CHECK(runner, memory != nullptr);
	if (memory != nullptr)
	{
		CRY_TEST_CHECK(runner, g_mm09SizeFunction(memory) >= requestedSize);
		g_mm09FreeFunction(memory);
	}
}

int RunMM09RuntimeTrace(const wchar_t* crySystemPath)
{
	if (crySystemPath == nullptr)
		return 3;

	HMODULE const crySystem = LoadLibraryW(crySystemPath);
	if (crySystem == nullptr)
		return 3;

	g_mm09MallocFunction = reinterpret_cast<MM09CrySystemCrtMallocFunction>(
		GetProcAddress(crySystem, "CrySystemCrtMalloc"));
	g_mm09FreeFunction = reinterpret_cast<MM09CrySystemCrtFreeFunction>(
		GetProcAddress(crySystem, "CrySystemCrtFree"));
	g_mm09SizeFunction = reinterpret_cast<MM09CrySystemCrtSizeFunction>(
		GetProcAddress(crySystem, "CrySystemCrtSize"));

	CryTestRunner runner;
	runner.Run(
		"MM09 runtime CRT exports are complete",
		TestMM09RuntimeCrtExportsAreComplete);
	runner.Run(
		"MM09 runtime CRT size round trip",
		TestMM09RuntimeCrtSizeRoundTrip);

	const int result = runner.ExitCode();
	g_mm09MallocFunction = nullptr;
	g_mm09FreeFunction = nullptr;
	g_mm09SizeFunction = nullptr;
	FreeLibrary(crySystem);
	return result;
}
