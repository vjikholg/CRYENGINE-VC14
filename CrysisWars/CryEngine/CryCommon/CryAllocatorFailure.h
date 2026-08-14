#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

enum class ECryAllocatorFailureAction : uint32_t
{
	ReturnNull = 0,
	WriteDumpAndTerminate = 1
};

inline constexpr uint32_t CryAllocatorFailureContextVersion = 1;
inline constexpr uint32_t CryAllocatorFailureExceptionCode = 0xE0060006u;
inline constexpr uint32_t CryAllocatorFailureExitCode = CryAllocatorFailureExceptionCode;
inline constexpr char CryAllocatorFailureHandlerExportName[] =
	"CrySystemHandleAllocatorFailure";

struct SCryAllocatorFailureContext
{
	uint32_t structureSize;
	uint32_t version;
	ECryAllocatorFailureAction action;
	uint32_t sourceLine;
	uint64_t count;
	uint64_t elementSize;
	uint64_t maximumElementCount;
	uint64_t alignment;
	uint64_t additionalBytes;
	char operation[96];
	char function[512];
	char sourceFile[260];
};

constexpr bool CryTryMultiplySize(
	size_t left,
	size_t right,
	size_t& result) noexcept
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	if (left != 0 && right > maximum / left)
	{
		result = 0;
		return false;
	}

	result = left * right;
	return true;
}

constexpr bool CryTryAddSize(
	size_t left,
	size_t right,
	size_t& result) noexcept
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	if (right > maximum - left)
	{
		result = 0;
		return false;
	}

	result = left + right;
	return true;
}

template<class T>
inline constexpr size_t CryAllocatorMaxElementCount =
	(std::numeric_limits<size_t>::max)() / sizeof(T);

inline void CryCopyAllocatorFailureText(
	char* destination,
	size_t destinationSize,
	const char* source) noexcept
{
	if (destinationSize == 0)
	{
		return;
	}

	size_t index = 0;
	if (source != nullptr)
	{
		while (index + 1 < destinationSize && source[index] != '\0')
		{
			destination[index] = source[index];
			++index;
		}
	}

	destination[index] = '\0';
}

inline SCryAllocatorFailureContext CryCreateAllocatorFailureContext(
	const char* operation,
	size_t count,
	size_t elementSize,
	size_t maximumElementCount,
	size_t alignment,
	size_t additionalBytes,
	ECryAllocatorFailureAction action,
	const char* function,
	const char* sourceFile,
	uint32_t sourceLine) noexcept
{
	SCryAllocatorFailureContext context = {};
	context.structureSize = sizeof(context);
	context.version = CryAllocatorFailureContextVersion;
	context.action = action;
	context.sourceLine = sourceLine;
	context.count = static_cast<uint64_t>(count);
	context.elementSize = static_cast<uint64_t>(elementSize);
	context.maximumElementCount = static_cast<uint64_t>(maximumElementCount);
	context.alignment = static_cast<uint64_t>(alignment);
	context.additionalBytes = static_cast<uint64_t>(additionalBytes);
	CryCopyAllocatorFailureText(context.operation, sizeof(context.operation), operation);
	CryCopyAllocatorFailureText(context.function, sizeof(context.function), function);
	CryCopyAllocatorFailureText(context.sourceFile, sizeof(context.sourceFile), sourceFile);
	return context;
}

void CryAllocatorSizeFailure(const SCryAllocatorFailureContext& context) noexcept;

using CryAllocatorFailureHandler = void (*)(
	const SCryAllocatorFailureContext* context);

#if defined(_WIN32) && defined(CRYSYSTEM_EXPORTS)
	#define CRY_ALLOCATOR_FAILURE_API __declspec(dllexport)
#else
	#define CRY_ALLOCATOR_FAILURE_API
#endif

extern "C" CRY_ALLOCATOR_FAILURE_API void CrySystemHandleAllocatorFailure(
	const SCryAllocatorFailureContext* context);

#undef CRY_ALLOCATOR_FAILURE_API

#define CRY_ALLOCATOR_SIZE_FAILURE(operation, count, elementSize, maximumCount, alignment, additionalBytes, action) \
	CryAllocatorSizeFailure(CryCreateAllocatorFailureContext( \
		operation, count, elementSize, maximumCount, alignment, additionalBytes, action, \
		__FUNCSIG__, __FILE__, __LINE__))
