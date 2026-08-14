#include "MM06Tests.h"
#include "TestRunner.h"

#include <CryAllocatorFailure.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iterator>
#include <limits>

typedef void* (__cdecl* MM06CryCallocFunction)(size_t count, size_t elementSize);
typedef size_t (__cdecl* MM06CryFreeFunction)(void* memory);
typedef size_t (__cdecl* MM06CryGetMemSizeFunction)(void* memory, size_t sourceSize);
typedef void (__cdecl* MM06FailureHandlerFunction)(
	const SCryAllocatorFailureContext* context);
typedef BOOL (WINAPI* MM06ReadDumpStreamFunction)(
	PVOID dumpBase,
	ULONG streamNumber,
	PMINIDUMP_DIRECTORY* directory,
	PVOID* streamPointer,
	ULONG* streamSize);

struct MM06RuntimeApi
{
	HMODULE module;
	MM06CryCallocFunction callocFunction;
	MM06CryFreeFunction freeFunction;
	MM06CryGetMemSizeFunction getMemSizeFunction;
	MM06FailureHandlerFunction failureHandlerFunction;
};

struct MM06MappedDump
{
	HANDLE file;
	HANDLE mapping;
	void* base;
};

static MM06RuntimeApi* g_mm06RuntimeApi = nullptr;
static const wchar_t* g_mm06CrySystemPath = nullptr;

static void TestMM06LegacyMultiplicationWraps(CryTestRunner& runner)
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	constexpr size_t count = maximum / 2 + 1;
	constexpr size_t elementSize = 2;
	const size_t legacyBytes = count * elementSize;

	CRY_TEST_CHECK(runner, count <= maximum);
	CRY_TEST_CHECK(runner, elementSize <= maximum);
	CRY_TEST_CHECK(runner, legacyBytes == 0);
}

static void TestMM06CheckedMultiplicationBoundaries(CryTestRunner& runner)
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	size_t bytes = 99;

	CRY_TEST_CHECK(runner, CryTryMultiplySize(0, maximum, bytes));
	CRY_TEST_CHECK(runner, bytes == 0);
	CRY_TEST_CHECK(runner, CryTryMultiplySize(maximum, 1, bytes));
	CRY_TEST_CHECK(runner, bytes == maximum);
	CRY_TEST_CHECK(runner, CryTryMultiplySize(maximum / 16, 16, bytes));
	CRY_TEST_CHECK(runner, bytes == maximum - 15);
	CRY_TEST_CHECK(runner, !CryTryMultiplySize(maximum / 16 + 1, 16, bytes));
	CRY_TEST_CHECK(runner, bytes == 0);
}

static void TestMM06ZlibOperandsAreWidenedBeforeMultiplication(CryTestRunner& runner)
{
	constexpr unsigned count = 65536u;
	constexpr unsigned elementSize = 65536u;
	const unsigned legacyBytes = count * elementSize;
	size_t checkedBytes = 0;

	CRY_TEST_CHECK(runner, legacyBytes == 0u);
	CRY_TEST_CHECK(
		runner,
		CryTryMultiplySize(
			static_cast<size_t>(count),
			static_cast<size_t>(elementSize),
			checkedBytes));
	CRY_TEST_CHECK(runner, checkedBytes == size_t{1} << 32);
}

static void TestMM06CheckedAdditionForInlineMetadata(CryTestRunner& runner)
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	size_t bytes = 0;

	CRY_TEST_CHECK(runner, CryTryAddSize(maximum - 32, 32, bytes));
	CRY_TEST_CHECK(runner, bytes == maximum);
	CRY_TEST_CHECK(runner, !CryTryAddSize(maximum - 31, 32, bytes));
	CRY_TEST_CHECK(runner, bytes == 0);
}

static void TestMM06PerTypeMaximumElementCounts(CryTestRunner& runner)
{
	struct MM06WideElement
	{
		uint64_t words[8];
	};

	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	CRY_TEST_CHECK(runner, CryAllocatorMaxElementCount<unsigned char> == maximum);
	CRY_TEST_CHECK(
		runner,
		CryAllocatorMaxElementCount<MM06WideElement> ==
			maximum / sizeof(MM06WideElement));
}

static bool MM06LoadRuntimeApi(const wchar_t* crySystemPath, MM06RuntimeApi& api)
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

	api.callocFunction = reinterpret_cast<MM06CryCallocFunction>(
		GetProcAddress(api.module, "CryModuleCalloc"));
	api.freeFunction = reinterpret_cast<MM06CryFreeFunction>(
		GetProcAddress(api.module, "CryFree"));
	api.getMemSizeFunction = reinterpret_cast<MM06CryGetMemSizeFunction>(
		GetProcAddress(api.module, "CryGetMemSize"));
	api.failureHandlerFunction = reinterpret_cast<MM06FailureHandlerFunction>(
		GetProcAddress(api.module, "CrySystemHandleAllocatorFailure"));

	return true;
}

static void MM06UnloadRuntimeApi(MM06RuntimeApi& api)
{
	if (api.module != nullptr)
	{
		FreeLibrary(api.module);
	}
	api = {};
}

static void TestMM06RuntimeApiIsComplete(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, g_mm06RuntimeApi->callocFunction != nullptr);
	CRY_TEST_CHECK(runner, g_mm06RuntimeApi->freeFunction != nullptr);
	CRY_TEST_CHECK(runner, g_mm06RuntimeApi->getMemSizeFunction != nullptr);
}

static void TestMM06RuntimeCallocReturnsZeroedAllocation(CryTestRunner& runner)
{
	if (g_mm06RuntimeApi->callocFunction == nullptr ||
		g_mm06RuntimeApi->freeFunction == nullptr ||
		g_mm06RuntimeApi->getMemSizeFunction == nullptr)
	{
		CRY_TEST_CHECK(runner, false);
		return;
	}

	constexpr size_t count = 1024 * 1024;
	constexpr size_t elementSize = 64;
	constexpr size_t requestedSize = count * elementSize;
	void* const memory = g_mm06RuntimeApi->callocFunction(count, elementSize);
	CRY_TEST_CHECK(runner, memory != nullptr);
	if (memory == nullptr)
	{
		return;
	}

	const unsigned char* const bytes = static_cast<const unsigned char*>(memory);
	CRY_TEST_CHECK(runner, bytes[0] == 0);
	CRY_TEST_CHECK(runner, bytes[requestedSize / 2] == 0);
	CRY_TEST_CHECK(runner, bytes[requestedSize - 1] == 0);
	CRY_TEST_CHECK(
		runner,
		g_mm06RuntimeApi->getMemSizeFunction(memory, 0) >= requestedSize);
	g_mm06RuntimeApi->freeFunction(memory);
}

static void TestMM06RuntimeCallocRejectsOverflow(CryTestRunner& runner)
{
	if (g_mm06RuntimeApi->callocFunction == nullptr)
	{
		CRY_TEST_CHECK(runner, false);
		return;
	}

	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	void* const memory = g_mm06RuntimeApi->callocFunction(maximum / 2 + 1, 2);
	CRY_TEST_CHECK(runner, memory == nullptr);
	if (memory != nullptr && g_mm06RuntimeApi->freeFunction != nullptr)
	{
		g_mm06RuntimeApi->freeFunction(memory);
	}
}

static std::filesystem::path MM06CreateDumpDirectory()
{
	wchar_t temporaryPath[32768] = {};
	if (GetTempPathW(static_cast<DWORD>(std::size(temporaryPath)), temporaryPath) == 0)
	{
		return {};
	}

	wchar_t directoryName[128] = {};
	_snwprintf_s(
		directoryName,
		std::size(directoryName),
		_TRUNCATE,
		L"CryCommonTests-MM06-%lu-%llu",
		GetCurrentProcessId(),
		static_cast<unsigned long long>(GetTickCount64()));

	const std::filesystem::path directory =
		std::filesystem::path(temporaryPath) / directoryName;
	std::error_code error;
	std::filesystem::create_directories(directory, error);
	return error ? std::filesystem::path{} : directory;
}

static bool MM06RunCrashProbe(
	const std::filesystem::path& dumpDirectory,
	DWORD& exitCode)
{
	wchar_t executablePath[32768] = {};
	if (GetModuleFileNameW(
			nullptr,
			executablePath,
			static_cast<DWORD>(std::size(executablePath))) == 0)
	{
		return false;
	}

	wchar_t commandLine[32768] = {};
	const int written = _snwprintf_s(
		commandLine,
		std::size(commandLine),
		_TRUNCATE,
		L"\"%ls\" --mm06-crash-probe \"%ls\" \"%ls\"",
		executablePath,
		g_mm06CrySystemPath,
		dumpDirectory.c_str());
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
			CREATE_NO_WINDOW,
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
		waitResult == WAIT_OBJECT_0 &&
		GetExitCodeProcess(processInformation.hProcess, &exitCode);
	CloseHandle(processInformation.hThread);
	CloseHandle(processInformation.hProcess);
	return completed;
}

static std::filesystem::path MM06FindDump(
	const std::filesystem::path& dumpDirectory)
{
	std::error_code error;
	for (const std::filesystem::directory_entry& entry :
		 std::filesystem::directory_iterator(dumpDirectory, error))
	{
		if (entry.is_regular_file() && entry.path().extension() == L".dmp")
		{
			return entry.path();
		}
	}

	return {};
}

static bool MM06MapDump(
	const std::filesystem::path& dumpPath,
	MM06MappedDump& mappedDump)
{
	mappedDump = {};
	mappedDump.file = CreateFileW(
		dumpPath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);
	if (mappedDump.file == INVALID_HANDLE_VALUE)
	{
		mappedDump.file = nullptr;
		return false;
	}

	mappedDump.mapping = CreateFileMappingW(
		mappedDump.file,
		nullptr,
		PAGE_READONLY,
		0,
		0,
		nullptr);
	if (mappedDump.mapping == nullptr)
	{
		CloseHandle(mappedDump.file);
		mappedDump = {};
		return false;
	}

	mappedDump.base = MapViewOfFile(mappedDump.mapping, FILE_MAP_READ, 0, 0, 0);
	if (mappedDump.base == nullptr)
	{
		CloseHandle(mappedDump.mapping);
		CloseHandle(mappedDump.file);
		mappedDump = {};
		return false;
	}

	return true;
}

static void MM06UnmapDump(MM06MappedDump& mappedDump)
{
	if (mappedDump.base != nullptr)
	{
		UnmapViewOfFile(mappedDump.base);
	}
	if (mappedDump.mapping != nullptr)
	{
		CloseHandle(mappedDump.mapping);
	}
	if (mappedDump.file != nullptr)
	{
		CloseHandle(mappedDump.file);
	}
	mappedDump = {};
}

static bool MM06InspectDump(
	const std::filesystem::path& dumpPath,
	bool& hasException,
	bool& hasThreads,
	bool& hasDiagnosticContext)
{
	hasException = false;
	hasThreads = false;
	hasDiagnosticContext = false;

	MM06MappedDump mappedDump = {};
	if (!MM06MapDump(dumpPath, mappedDump))
	{
		return false;
	}

	HMODULE dbgHelp = LoadLibraryW(L"dbghelp.dll");
	if (dbgHelp == nullptr)
	{
		MM06UnmapDump(mappedDump);
		return false;
	}

	MM06ReadDumpStreamFunction readDumpStream =
		reinterpret_cast<MM06ReadDumpStreamFunction>(
			GetProcAddress(dbgHelp, "MiniDumpReadDumpStream"));
	if (readDumpStream == nullptr)
	{
		FreeLibrary(dbgHelp);
		MM06UnmapDump(mappedDump);
		return false;
	}

	PMINIDUMP_DIRECTORY directory = nullptr;
	void* stream = nullptr;
	ULONG streamSize = 0;
	if (readDumpStream(
			mappedDump.base,
			ExceptionStream,
			&directory,
			&stream,
			&streamSize) &&
		streamSize >= sizeof(MINIDUMP_EXCEPTION_STREAM))
	{
		const MINIDUMP_EXCEPTION_STREAM* const exceptionStream =
			static_cast<const MINIDUMP_EXCEPTION_STREAM*>(stream);
		hasException =
			exceptionStream->ExceptionRecord.ExceptionCode ==
			CryAllocatorFailureExceptionCode;
	}

	directory = nullptr;
	stream = nullptr;
	streamSize = 0;
	hasThreads = readDumpStream(
		mappedDump.base,
		ThreadListStream,
		&directory,
		&stream,
		&streamSize) != FALSE;

	directory = nullptr;
	stream = nullptr;
	streamSize = 0;
	if (readDumpStream(
			mappedDump.base,
			CommentStreamA,
			&directory,
			&stream,
			&streamSize) &&
		stream != nullptr && streamSize != 0)
	{
		const char* const comment = static_cast<const char*>(stream);
		hasDiagnosticContext =
			std::strstr(comment, "MM06IntentionalArrayOverflowProbe") != nullptr &&
			std::strstr(comment, "count=18446744073709551615") != nullptr &&
			std::strstr(comment, "elementSize=4096") != nullptr &&
			std::strstr(comment, "MM06Tests.cpp") != nullptr;
	}

	FreeLibrary(dbgHelp);
	MM06UnmapDump(mappedDump);
	return true;
}

static void TestMM06IntentionalFailureWritesDiagnosticDump(CryTestRunner& runner)
{
	const std::filesystem::path dumpDirectory = MM06CreateDumpDirectory();
	CRY_TEST_CHECK(runner, !dumpDirectory.empty());
	if (dumpDirectory.empty())
	{
		return;
	}

	DWORD exitCode = 0;
	const bool completed = MM06RunCrashProbe(dumpDirectory, exitCode);
	const std::filesystem::path dumpPath = MM06FindDump(dumpDirectory);
	bool hasException = false;
	bool hasThreads = false;
	bool hasDiagnosticContext = false;
	const bool inspected = !dumpPath.empty() && MM06InspectDump(
		dumpPath,
		hasException,
		hasThreads,
		hasDiagnosticContext);

	std::printf("  MM06 dump: %ls\n", dumpPath.empty() ? L"<missing>" : dumpPath.c_str());
	CRY_TEST_CHECK(runner, completed);
	CRY_TEST_CHECK(runner, exitCode == CryAllocatorFailureExitCode);
	CRY_TEST_CHECK(runner, !dumpPath.empty());
	CRY_TEST_CHECK(runner, inspected);
	CRY_TEST_CHECK(runner, hasException);
	CRY_TEST_CHECK(runner, hasThreads);
	CRY_TEST_CHECK(runner, hasDiagnosticContext);
}

void RunMM06SizeArithmeticTests(CryTestRunner& runner)
{
	runner.Run("MM06 legacy multiplication demonstrates wrapping", TestMM06LegacyMultiplicationWraps);
	runner.Run("MM06 checked multiplication protects boundaries", TestMM06CheckedMultiplicationBoundaries);
	runner.Run("MM06 zlib operands widen before multiplication", TestMM06ZlibOperandsAreWidenedBeforeMultiplication);
	runner.Run("MM06 metadata addition protects boundaries", TestMM06CheckedAdditionForInlineMetadata);
	runner.Run("MM06 per-type maximum element counts", TestMM06PerTypeMaximumElementCounts);
}

int RunMM06RuntimeTrace(const wchar_t* crySystemPath)
{
	MM06RuntimeApi api = {};
	if (crySystemPath == nullptr || !MM06LoadRuntimeApi(crySystemPath, api))
	{
		return 3;
	}

	g_mm06RuntimeApi = &api;
	g_mm06CrySystemPath = crySystemPath;
	CryTestRunner runner;
	RunMM06SizeArithmeticTests(runner);
	runner.Run("MM06 runtime API exports required probes", TestMM06RuntimeApiIsComplete);
	runner.Run("MM06 runtime calloc returns a large zeroed allocation", TestMM06RuntimeCallocReturnsZeroedAllocation);
	runner.Run("MM06 runtime calloc rejects an overflowing product", TestMM06RuntimeCallocRejectsOverflow);
	runner.Run("MM06 intentional failure writes a diagnostic dump", TestMM06IntentionalFailureWritesDiagnosticDump);
	const int result = runner.ExitCode();
	g_mm06RuntimeApi = nullptr;
	g_mm06CrySystemPath = nullptr;
	MM06UnloadRuntimeApi(api);
	return result;
}

int RunMM06CrashProbe(
	const wchar_t* crySystemPath,
	const wchar_t* dumpDirectory)
{
	if (crySystemPath == nullptr || dumpDirectory == nullptr)
	{
		return 3;
	}

	SetEnvironmentVariableW(L"CRY_ALLOCATOR_DUMP_DIRECTORY", dumpDirectory);
	MM06RuntimeApi api = {};
	if (!MM06LoadRuntimeApi(crySystemPath, api) ||
		api.failureHandlerFunction == nullptr)
	{
		MM06UnloadRuntimeApi(api);
		return 3;
	}

	const SCryAllocatorFailureContext context = CryCreateAllocatorFailureContext(
		"MM06IntentionalArrayOverflowProbe",
		(std::numeric_limits<size_t>::max)(),
		4096,
		(std::numeric_limits<size_t>::max)() / 4096,
		alignof(std::max_align_t),
		64,
		ECryAllocatorFailureAction::WriteDumpAndTerminate,
		__FUNCSIG__,
		__FILE__,
		__LINE__);
	CryAllocatorSizeFailure(context);
	MM06UnloadRuntimeApi(api);
	return 1;
}
