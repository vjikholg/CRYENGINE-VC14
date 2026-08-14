#include "StdAfx.h"

#include <CryAllocatorFailure.h>
#include <ISystem.h>

#include <dbghelp.h>

#include <cstdio>
#include <cstring>

typedef BOOL (WINAPI* CryMiniDumpWriteDumpFunction)(
	HANDLE process,
	DWORD processId,
	HANDLE file,
	MINIDUMP_TYPE dumpType,
	const MINIDUMP_EXCEPTION_INFORMATION* exceptionInformation,
	const MINIDUMP_USER_STREAM_INFORMATION* userStreamInformation,
	const MINIDUMP_CALLBACK_INFORMATION* callbackInformation);

static void CryAppendPathSeparator(wchar_t* path, size_t pathCapacity) noexcept
{
	const size_t length = wcsnlen_s(path, pathCapacity);
	if (length == 0 || length + 1 >= pathCapacity)
		return;

	if (path[length - 1] != L'\\' && path[length - 1] != L'/')
		wcscat_s(path, pathCapacity, L"\\");
}

static bool CryGetAllocatorDumpDirectory(
	wchar_t* path,
	size_t pathCapacity) noexcept
{
	const DWORD environmentLength = GetEnvironmentVariableW(
		L"CRY_ALLOCATOR_DUMP_DIRECTORY",
		path,
		static_cast<DWORD>(pathCapacity));
	if (environmentLength != 0 && environmentLength < pathCapacity)
	{
		CreateDirectoryW(path, nullptr);
		return true;
	}

	const DWORD temporaryPathLength = GetTempPathW(
		static_cast<DWORD>(pathCapacity),
		path);
	if (temporaryPathLength == 0 || temporaryPathLength >= pathCapacity)
		return false;

	CryAppendPathSeparator(path, pathCapacity);
	if (wcscat_s(path, pathCapacity, L"CryEngineDumps") != 0)
		return false;

	return CreateDirectoryW(path, nullptr) != FALSE ||
		GetLastError() == ERROR_ALREADY_EXISTS;
}

static bool CryBuildAllocatorDumpPath(
	wchar_t* path,
	size_t pathCapacity) noexcept
{
	if (!CryGetAllocatorDumpDirectory(path, pathCapacity))
		return false;

	CryAppendPathSeparator(path, pathCapacity);
	const size_t directoryLength = wcsnlen_s(path, pathCapacity);
	if (directoryLength >= pathCapacity)
		return false;

	SYSTEMTIME time = {};
	GetSystemTime(&time);
	const int written = _snwprintf_s(
		path + directoryLength,
		pathCapacity - directoryLength,
		_TRUNCATE,
		L"AllocatorFailure-%04u%02u%02u-%02u%02u%02u-%03u-p%lu-t%lu.dmp",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond,
		time.wMilliseconds,
		GetCurrentProcessId(),
		GetCurrentThreadId());
	return written >= 0;
}

static void CryFormatAllocatorFailureComment(
	const SCryAllocatorFailureContext& context,
	char* comment,
	size_t commentCapacity) noexcept
{
	_snprintf_s(
		comment,
		commentCapacity,
		_TRUNCATE,
		"CryAllocatorFailure\n"
		"contextVersion=%u\n"
		"action=%u\n"
		"operation=%s\n"
		"count=%llu\n"
		"elementSize=%llu\n"
		"maximumElementCount=%llu\n"
		"alignment=%llu\n"
		"additionalBytes=%llu\n"
		"function=%s\n"
		"sourceFile=%s\n"
		"sourceLine=%u\n",
		context.version,
		static_cast<unsigned>(context.action),
		context.operation,
		static_cast<unsigned long long>(context.count),
		static_cast<unsigned long long>(context.elementSize),
		static_cast<unsigned long long>(context.maximumElementCount),
		static_cast<unsigned long long>(context.alignment),
		static_cast<unsigned long long>(context.additionalBytes),
		context.function,
		context.sourceFile,
		context.sourceLine);
}

static void CryReportAllocatorFailure(
	const SCryAllocatorFailureContext& context) noexcept
{
	char message[2048] = {};
	CryFormatAllocatorFailureComment(context, message, sizeof(message));
	OutputDebugStringA(message);

	static thread_local bool reportingToCryLog = false;
	if (!reportingToCryLog &&
		gEnv != nullptr &&
		gEnv->pSystem != nullptr &&
		gEnv->pLog != nullptr)
	{
		reportingToCryLog = true;
		CryLogAlways("%s", message);
		reportingToCryLog = false;
	}
}

static bool CryWriteAllocatorFailureDump(
	EXCEPTION_POINTERS* exceptionPointers,
	const SCryAllocatorFailureContext& context,
	wchar_t* dumpPath,
	size_t dumpPathCapacity) noexcept
{
	if (!CryBuildAllocatorDumpPath(dumpPath, dumpPathCapacity))
		return false;

	HMODULE const dbgHelp = LoadLibraryW(L"dbghelp.dll");
	if (dbgHelp == nullptr)
		return false;

	CryMiniDumpWriteDumpFunction const writeDump =
		reinterpret_cast<CryMiniDumpWriteDumpFunction>(
			GetProcAddress(dbgHelp, "MiniDumpWriteDump"));
	if (writeDump == nullptr)
	{
		FreeLibrary(dbgHelp);
		return false;
	}

	HANDLE const dumpFile = CreateFileW(
		dumpPath,
		GENERIC_WRITE,
		0,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);
	if (dumpFile == INVALID_HANDLE_VALUE)
	{
		FreeLibrary(dbgHelp);
		return false;
	}

	char diagnosticComment[2048] = {};
	CryFormatAllocatorFailureComment(
		context,
		diagnosticComment,
		sizeof(diagnosticComment));
	MINIDUMP_USER_STREAM commentStream = {};
	commentStream.Type = CommentStreamA;
	commentStream.BufferSize =
		static_cast<ULONG>(std::strlen(diagnosticComment) + 1);
	commentStream.Buffer = diagnosticComment;
	MINIDUMP_USER_STREAM_INFORMATION userStreams = {};
	userStreams.UserStreamCount = 1;
	userStreams.UserStreamArray = &commentStream;

	MINIDUMP_EXCEPTION_INFORMATION exceptionInformation = {};
	exceptionInformation.ThreadId = GetCurrentThreadId();
	exceptionInformation.ExceptionPointers = exceptionPointers;
	exceptionInformation.ClientPointers = FALSE;

	const MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(
		MiniDumpNormal |
		MiniDumpWithThreadInfo |
		MiniDumpWithUnloadedModules |
		MiniDumpWithIndirectlyReferencedMemory);
	const bool written = writeDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		dumpFile,
		dumpType,
		&exceptionInformation,
		&userStreams,
		nullptr) != FALSE;

	CloseHandle(dumpFile);
	FreeLibrary(dbgHelp);
	return written;
}

static LONG CryAllocatorFailureExceptionFilter(
	EXCEPTION_POINTERS* exceptionPointers,
	const SCryAllocatorFailureContext* context) noexcept
{
	wchar_t dumpPath[32768] = {};
	const bool written = context != nullptr && CryWriteAllocatorFailureDump(
		exceptionPointers,
		*context,
		dumpPath,
		std::size(dumpPath));

	wchar_t message[32768] = {};
	if (written)
	{
		_snwprintf_s(
			message,
			std::size(message),
			_TRUNCATE,
			L"Allocator failure dump written to %ls\n",
			dumpPath);
	}
	else
	{
		_snwprintf_s(
			message,
			std::size(message),
			_TRUNCATE,
			L"Allocator failure dump could not be written (Windows error %lu)\n",
			GetLastError());
	}
	OutputDebugStringW(message);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void CryRaiseAllocatorFailureException(
	const SCryAllocatorFailureContext* context) noexcept
{
	__try
	{
		RaiseException(
			CryAllocatorFailureExceptionCode,
			EXCEPTION_NONCONTINUABLE,
			0,
			nullptr);
	}
	__except(CryAllocatorFailureExceptionFilter(
		GetExceptionInformation(),
		context))
	{
	}

	TerminateProcess(GetCurrentProcess(), CryAllocatorFailureExitCode);
}

void CrySystemHandleAllocatorFailure(
	const SCryAllocatorFailureContext* context)
{
	if (context == nullptr ||
		context->structureSize != sizeof(SCryAllocatorFailureContext) ||
		context->version != CryAllocatorFailureContextVersion)
	{
		const SCryAllocatorFailureContext invalidContext =
			CryCreateAllocatorFailureContext(
				"InvalidAllocatorFailureContext",
				0,
				0,
				0,
				alignof(std::max_align_t),
				0,
				ECryAllocatorFailureAction::WriteDumpAndTerminate,
				__FUNCSIG__,
				__FILE__,
				__LINE__);
		CryRaiseAllocatorFailureException(&invalidContext);
		return;
	}

	if (context->action == ECryAllocatorFailureAction::WriteDumpAndTerminate)
	{
		CryReportAllocatorFailure(*context);
		CryRaiseAllocatorFailureException(context);
		return;
	}

	CryReportAllocatorFailure(*context);
}
