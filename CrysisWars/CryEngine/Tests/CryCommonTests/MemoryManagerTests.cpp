#include "MemoryManagerTests.h"
#include "TestRunner.h"
#include "CryAllocationHeader.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <limits>
#include <new>

struct alignas(16) MM02AlignedPayload
{
	uint64_t first;
	uint64_t second;
};

typedef void* (__cdecl* MM02CryMallocFunction)(size_t size, size_t& allocated);
typedef void* (__cdecl* MM02CryReallocFunction)(void* memory, size_t size, size_t& allocated);
typedef size_t (__cdecl* MM02CryFreeFunction)(void* memory);
typedef size_t (__cdecl* MM02CryGetMemSizeFunction)(void* memory, size_t sourceSize);

struct MM02RuntimeApi
{
	HMODULE module;
	MM02CryMallocFunction mallocFunction;
	MM02CryReallocFunction reallocFunction;
	MM02CryFreeFunction freeFunction;
	MM02CryGetMemSizeFunction getMemSizeFunction;
};

static_assert(sizeof(void*) == 8, "MM02 tests require the Win64 data model.");
static_assert(sizeof(size_t) == sizeof(void*), "MM02 size metadata must be pointer-width.");
static_assert(
	sizeof(SCryAllocationHeader) % alignof(std::max_align_t) == 0,
	"The repaired header must preserve maximum fundamental alignment.");

static MM02RuntimeApi* g_mm02RuntimeApi = nullptr;
static const wchar_t* g_mm02CrySystemPath = nullptr;

static void TestMM02AlignmentDetector(CryTestRunner& runner)
{
	alignas(std::max_align_t) unsigned char storage[64] = {};
	SCryAllocationHeader* const header =
		::new (storage) SCryAllocationHeader{ 24, 24 + sizeof(SCryAllocationHeader) };
	void* const userMemory = CryGetAllocationUserMemory(header);
	const uintptr_t rawAddress = reinterpret_cast<uintptr_t>(header);
	const uintptr_t legacyUserAddress = rawAddress + sizeof(uint32_t);
	const uintptr_t repairedUserAddress = reinterpret_cast<uintptr_t>(userMemory);

	CRY_TEST_CHECK(runner, alignof(std::max_align_t) > sizeof(uint32_t));
	CRY_TEST_CHECK(runner, legacyUserAddress % alignof(std::max_align_t) != 0);
	CRY_TEST_CHECK(runner, repairedUserAddress % alignof(std::max_align_t) == 0);
	CRY_TEST_CHECK(runner, CryGetAllocationHeader(userMemory) == header);
}

static void TestMM02SizeWidthDetector(CryTestRunner& runner)
{
	constexpr size_t largeSize = static_cast<size_t>(UINT32_MAX) + 65;
	const uint32_t truncatedSize = static_cast<uint32_t>(largeSize);
	const SCryAllocationHeader header = {
		largeSize,
		largeSize + sizeof(SCryAllocationHeader)
	};

	CRY_TEST_CHECK(runner, static_cast<size_t>(truncatedSize) != largeSize);
	CRY_TEST_CHECK(runner, header.requestedSize == largeSize);
	CRY_TEST_CHECK(
		runner,
		header.allocationSize == largeSize + sizeof(SCryAllocationHeader));
}

static void TestMM02CheckedAddition(CryTestRunner& runner)
{
	constexpr size_t maximum = std::numeric_limits<size_t>::max();
	constexpr size_t headerSize = sizeof(SCryAllocationHeader);
	size_t allocationSize = 0;

	CRY_TEST_CHECK(runner, CryTryCalculateAllocationSize(maximum - headerSize, allocationSize));
	CRY_TEST_CHECK(runner, allocationSize == maximum);
	CRY_TEST_CHECK(runner, !CryTryCalculateAllocationSize(maximum - headerSize + 1, allocationSize));
	CRY_TEST_CHECK(runner, allocationSize == 0);
}

static bool MM02LoadRuntimeApi(const wchar_t* crySystemPath, MM02RuntimeApi& api)
{
	api = {};
	api.module = LoadLibraryW(crySystemPath);
	if (api.module == nullptr)
	{
		std::printf(
			"Unable to load CrySystem DLL '%ls' (Windows error %lu).\n",
			crySystemPath,
			GetLastError());
		return false;
	}

	api.mallocFunction = reinterpret_cast<MM02CryMallocFunction>(
		GetProcAddress(api.module, "CryMalloc"));
	api.reallocFunction = reinterpret_cast<MM02CryReallocFunction>(
		GetProcAddress(api.module, "CryRealloc"));
	api.freeFunction = reinterpret_cast<MM02CryFreeFunction>(
		GetProcAddress(api.module, "CryFree"));
	api.getMemSizeFunction = reinterpret_cast<MM02CryGetMemSizeFunction>(
		GetProcAddress(api.module, "CryGetMemSize"));

	if (api.mallocFunction == nullptr || api.reallocFunction == nullptr ||
		api.freeFunction == nullptr || api.getMemSizeFunction == nullptr)
	{
		std::printf("CrySystem does not export the complete MM02 allocation API.\n");
		FreeLibrary(api.module);
		api = {};
		return false;
	}

	return true;
}

static void TestMM02RuntimeAlignmentAndSize(CryTestRunner& runner)
{
	const size_t requestedSizes[] = {
		0, 1, 2, 7, 8, 15, 16, 31, 32, 33, 63, 64, 127, 128,
		255, 256, 495, 496, 497, 508, 509, 511, 512, 513, 1024
	};

	for (size_t requestedSize : requestedSizes)
	{
		size_t allocatedSize = 0;
		void* const memory = g_mm02RuntimeApi->mallocFunction(requestedSize, allocatedSize);
		CRY_TEST_CHECK(runner, memory != nullptr);
		if (memory == nullptr)
		{
			continue;
		}

		const bool isAligned =
			reinterpret_cast<uintptr_t>(memory) % alignof(std::max_align_t) == 0;
		if (!isAligned)
		{
			std::printf("  request %zu returned misaligned address %p\n", requestedSize, memory);
		}
		CRY_TEST_CHECK(runner, isAligned);

		const size_t queriedSize = g_mm02RuntimeApi->getMemSizeFunction(memory, 0);
		CRY_TEST_CHECK(runner, allocatedSize >= requestedSize);
		CRY_TEST_CHECK(runner, queriedSize == allocatedSize);
		if (allocatedSize >= requestedSize)
		{
			CRY_TEST_CHECK(
				runner,
				allocatedSize - requestedSize >= sizeof(SCryAllocationHeader));
		}

		if (requestedSize != 0)
		{
			std::memset(memory, 0xA5, requestedSize);
		}

		const size_t freedSize = g_mm02RuntimeApi->freeFunction(memory);
		CRY_TEST_CHECK(runner, freedSize == allocatedSize);
	}
}

static void TestMM02RuntimeReallocation(CryTestRunner& runner)
{
	constexpr size_t initialSize = 64;
	constexpr size_t grownSize = 256;
	constexpr size_t shrunkSize = 32;

	size_t allocatedSize = 0;
	void* memory = g_mm02RuntimeApi->mallocFunction(initialSize, allocatedSize);
	CRY_TEST_CHECK(runner, memory != nullptr);
	if (memory == nullptr)
	{
		return;
	}

	unsigned char* bytes = static_cast<unsigned char*>(memory);
	for (size_t index = 0; index < initialSize; ++index)
	{
		bytes[index] = static_cast<unsigned char>(index);
	}

	size_t grownAllocationSize = 0;
	void* grownMemory = g_mm02RuntimeApi->reallocFunction(memory, grownSize, grownAllocationSize);
	CRY_TEST_CHECK(runner, grownMemory != nullptr);
	if (grownMemory == nullptr)
	{
		g_mm02RuntimeApi->freeFunction(memory);
		return;
	}

	bytes = static_cast<unsigned char*>(grownMemory);
	bool prefixPreserved = true;
	for (size_t index = 0; index < initialSize; ++index)
	{
		prefixPreserved = prefixPreserved && bytes[index] == static_cast<unsigned char>(index);
	}
	CRY_TEST_CHECK(runner, prefixPreserved);
	CRY_TEST_CHECK(
		runner,
		reinterpret_cast<uintptr_t>(grownMemory) % alignof(std::max_align_t) == 0);
	CRY_TEST_CHECK(
		runner,
		g_mm02RuntimeApi->getMemSizeFunction(grownMemory, 0) == grownAllocationSize);

	size_t shrunkAllocationSize = 0;
	void* shrunkMemory =
		g_mm02RuntimeApi->reallocFunction(grownMemory, shrunkSize, shrunkAllocationSize);
	CRY_TEST_CHECK(runner, shrunkMemory != nullptr);
	if (shrunkMemory == nullptr)
	{
		g_mm02RuntimeApi->freeFunction(grownMemory);
		return;
	}

	bytes = static_cast<unsigned char*>(shrunkMemory);
	prefixPreserved = true;
	for (size_t index = 0; index < shrunkSize; ++index)
	{
		prefixPreserved = prefixPreserved && bytes[index] == static_cast<unsigned char>(index);
	}
	CRY_TEST_CHECK(runner, prefixPreserved);
	CRY_TEST_CHECK(
		runner,
		reinterpret_cast<uintptr_t>(shrunkMemory) % alignof(std::max_align_t) == 0);
	CRY_TEST_CHECK(
		runner,
		g_mm02RuntimeApi->getMemSizeFunction(shrunkMemory, 0) == shrunkAllocationSize);
	g_mm02RuntimeApi->freeFunction(shrunkMemory);
}

static void TestMM02RuntimePoolBoundary(CryTestRunner& runner)
{
	const size_t requestedSizes[] = { 480, 495, 496, 497, 508, 509, 511, 512, 513 };
	bool allAllocationsValid = true;

	for (size_t iteration = 0; iteration < 64; ++iteration)
	{
		for (size_t requestedSize : requestedSizes)
		{
			size_t allocatedSize = 0;
			void* const memory = g_mm02RuntimeApi->mallocFunction(requestedSize, allocatedSize);
			if (memory == nullptr)
			{
				allAllocationsValid = false;
				continue;
			}

			const bool isAligned =
				reinterpret_cast<uintptr_t>(memory) % alignof(std::max_align_t) == 0;
			allAllocationsValid = allAllocationsValid && isAligned;
			std::memset(memory, static_cast<int>(iteration), requestedSize);
			allAllocationsValid = allAllocationsValid &&
				g_mm02RuntimeApi->freeFunction(memory) == allocatedSize;
		}
	}

	CRY_TEST_CHECK(runner, allAllocationsValid);
}

static void TestMM02RuntimeAlignedObject(CryTestRunner& runner)
{
	size_t allocatedSize = 0;
	void* const memory =
		g_mm02RuntimeApi->mallocFunction(sizeof(MM02AlignedPayload), allocatedSize);
	CRY_TEST_CHECK(runner, memory != nullptr);
	if (memory == nullptr)
	{
		return;
	}

	const bool isAligned = reinterpret_cast<uintptr_t>(memory) % alignof(MM02AlignedPayload) == 0;
	CRY_TEST_CHECK(runner, isAligned);
	if (isAligned)
	{
		MM02AlignedPayload* const payload = new (memory) MM02AlignedPayload{ 0x1122334455667788ull, 0x8877665544332211ull };
		CRY_TEST_CHECK(runner, payload->first == 0x1122334455667788ull);
		CRY_TEST_CHECK(runner, payload->second == 0x8877665544332211ull);
		payload->~MM02AlignedPayload();
	}

	CRY_TEST_CHECK(runner, g_mm02RuntimeApi->freeFunction(memory) == allocatedSize);
}

static bool MM02RunChildProbe(const wchar_t* probeArgument, DWORD& exitCode)
{
	wchar_t executablePath[32768] = {};
	if (GetModuleFileNameW(nullptr, executablePath, static_cast<DWORD>(std::size(executablePath))) == 0)
	{
		return false;
	}

	wchar_t commandLine[32768] = {};
	const int written = _snwprintf_s(
		commandLine,
		std::size(commandLine),
		_TRUNCATE,
		L"\"%ls\" %ls \"%ls\"",
		executablePath,
		probeArgument,
		g_mm02CrySystemPath);
	if (written < 0)
	{
		return false;
	}

	STARTUPINFOW startupInfo = {};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInformation = {};
	if (!CreateProcessW(
		nullptr,
		commandLine,
		nullptr,
		nullptr,
		FALSE,
		0,
		nullptr,
		nullptr,
		&startupInfo,
		&processInformation))
	{
		return false;
	}

	const DWORD waitResult = WaitForSingleObject(processInformation.hProcess, 30000);
	if (waitResult == WAIT_TIMEOUT)
	{
		TerminateProcess(processInformation.hProcess, 4);
		WaitForSingleObject(processInformation.hProcess, INFINITE);
	}

	const bool completed =
		waitResult == WAIT_OBJECT_0 && GetExitCodeProcess(processInformation.hProcess, &exitCode);
	CloseHandle(processInformation.hThread);
	CloseHandle(processInformation.hProcess);
	return completed;
}

static void TestMM02RuntimeOverflowRejection(CryTestRunner& runner)
{
	DWORD exitCode = 0;
	const bool completed = MM02RunChildProbe(L"--mm02-overflow-probe", exitCode);
	if (completed && exitCode != 0)
	{
		std::printf("  overflow probe exited with code %lu\n", exitCode);
	}
	CRY_TEST_CHECK(runner, completed);
	CRY_TEST_CHECK(runner, exitCode == 0);
}

static void TestMM02RuntimeReallocFailure(CryTestRunner& runner)
{
	DWORD exitCode = 0;
	const bool completed =
		MM02RunChildProbe(L"--mm02-realloc-failure-probe", exitCode);
	if (completed && exitCode != 0)
	{
		std::printf("  realloc failure probe exited with code %lu\n", exitCode);
	}
	CRY_TEST_CHECK(runner, completed);
	CRY_TEST_CHECK(runner, exitCode == 0);
}

void RunMemoryManagerLayoutTests(CryTestRunner& runner)
{
	runner.Run("MM02 production header preserves Win64 alignment", TestMM02AlignmentDetector);
	runner.Run("MM02 production header preserves 64-bit sizes", TestMM02SizeWidthDetector);
	runner.Run("MM02 production size calculation rejects overflow", TestMM02CheckedAddition);
}

int RunMM02MemoryManagementTrace()
{
	CryTestRunner runner;
	RunMemoryManagerLayoutTests(runner);
	return runner.ExitCode();
}

int RunMM02RuntimeTrace(const wchar_t* crySystemPath)
{
	if (crySystemPath == nullptr || crySystemPath[0] == L'\0')
	{
		std::printf("MM02 runtime trace requires a CrySystem DLL path.\n");
		return 3;
	}

	MM02RuntimeApi api = {};
	if (!MM02LoadRuntimeApi(crySystemPath, api))
	{
		return 3;
	}

	g_mm02RuntimeApi = &api;
	g_mm02CrySystemPath = crySystemPath;

	CryTestRunner runner;
	runner.Run("MM02 runtime alignment and size metadata", TestMM02RuntimeAlignmentAndSize);
	runner.Run("MM02 runtime realloc preserves payload", TestMM02RuntimeReallocation);
	runner.Run("MM02 runtime survives the node allocator boundary", TestMM02RuntimePoolBoundary);
	runner.Run("MM02 runtime constructs a 16-byte aligned object", TestMM02RuntimeAlignedObject);
	runner.Run("MM02 runtime rejects allocation-size overflow", TestMM02RuntimeOverflowRejection);
	runner.Run("MM02 failed realloc preserves the old allocation", TestMM02RuntimeReallocFailure);
	const int exitCode = runner.ExitCode();

	g_mm02RuntimeApi = nullptr;
	g_mm02CrySystemPath = nullptr;
	FreeLibrary(api.module);
	return exitCode;
}

int RunMM02OverflowProbe(const wchar_t* crySystemPath)
{
	MM02RuntimeApi api = {};
	if (!MM02LoadRuntimeApi(crySystemPath, api))
	{
		return 3;
	}

	size_t allocatedSize = std::numeric_limits<size_t>::max();
	void* const memory =
		api.mallocFunction(std::numeric_limits<size_t>::max(), allocatedSize);
	if (memory != nullptr)
	{
		std::printf(
			"MM02 overflow probe: CryMalloc accepted SIZE_MAX and returned %p (%zu bytes).\n",
			memory,
			allocatedSize);
		return 1;
	}

	const bool passed = allocatedSize == 0;
	FreeLibrary(api.module);
	return passed ? 0 : 1;
}

int RunMM02ReallocFailureProbe(const wchar_t* crySystemPath)
{
	MM02RuntimeApi api = {};
	if (!MM02LoadRuntimeApi(crySystemPath, api))
	{
		return 3;
	}

	constexpr size_t oldSize = 64;
	size_t oldAllocationSize = 0;
	void* const oldMemory = api.mallocFunction(oldSize, oldAllocationSize);
	if (oldMemory == nullptr)
	{
		FreeLibrary(api.module);
		return 1;
	}

	std::memset(oldMemory, 0x5A, oldSize);
	size_t newAllocationSize = std::numeric_limits<size_t>::max();
	void* const replacement = api.reallocFunction(
		oldMemory,
		std::numeric_limits<size_t>::max(),
		newAllocationSize);
	if (replacement != nullptr)
	{
		std::printf(
			"MM02 realloc probe: CryRealloc accepted SIZE_MAX and returned %p.\n",
			replacement);
		return 1;
	}

	bool oldPayloadPreserved = newAllocationSize == 0;
	const unsigned char* const bytes = static_cast<const unsigned char*>(oldMemory);
	for (size_t index = 0; index < oldSize; ++index)
	{
		oldPayloadPreserved = oldPayloadPreserved && bytes[index] == 0x5A;
	}

	if (oldPayloadPreserved)
	{
		oldPayloadPreserved = api.freeFunction(oldMemory) == oldAllocationSize;
	}

	FreeLibrary(api.module);
	return oldPayloadPreserved ? 0 : 1;
}
