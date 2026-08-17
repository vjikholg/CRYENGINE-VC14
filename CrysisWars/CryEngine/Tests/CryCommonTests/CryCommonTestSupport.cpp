#include <CryAllocatorFailure.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <intrin.h>

long CryInterlockedExchangeAdd(long volatile* addend, long value)
{
	return _InterlockedExchangeAdd(addend, value);
}

void* CryInterlockedCompareExchangePointer(
	void* volatile* destination,
	void* exchange,
	void* comparand)
{
	return _InterlockedCompareExchangePointer(destination, exchange, comparand);
}

void CryAssertTrace(const char* format, ...)
{
	if (format == nullptr)
	{
		return;
	}

	va_list arguments;
	va_start(arguments, format);
	std::vfprintf(stderr, format, arguments);
	std::fputc('\n', stderr);
	va_end(arguments);
}

bool CryAssert(const char*, const char*, unsigned int, bool*)
{
	return false;
}

void CryDebugBreak()
{
}

void CryAllocatorSizeFailure(const SCryAllocatorFailureContext& context) noexcept
{
	HMODULE const crySystem = GetModuleHandleW(L"CrySystem.dll");
	if (crySystem != nullptr)
	{
		CryAllocatorFailureHandler const handler =
			reinterpret_cast<CryAllocatorFailureHandler>(
				GetProcAddress(
					crySystem,
					CryAllocatorFailureHandlerExportName));
		if (handler != nullptr)
		{
			handler(&context);
			if (context.action == ECryAllocatorFailureAction::ReturnNull)
				return;
		}
	}

	std::fprintf(
		stderr,
		"Allocator size failure: operation=%s count=%llu elementSize=%llu function=%s\n",
		context.operation,
		static_cast<unsigned long long>(context.count),
		static_cast<unsigned long long>(context.elementSize),
		context.function);

	if (context.action == ECryAllocatorFailureAction::WriteDumpAndTerminate)
	{
		std::abort();
	}
}
