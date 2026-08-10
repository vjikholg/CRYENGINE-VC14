#pragma once

#include <cstddef>
#include <limits>

struct alignas(std::max_align_t) SCryAllocationHeader
{
	size_t requestedSize;
	size_t allocationSize;
};

static_assert(
	sizeof(SCryAllocationHeader) % alignof(SCryAllocationHeader) == 0,
	"The Cry allocation header must preserve the backing allocation alignment.");

inline bool CryTryCalculateAllocationSize(size_t requestedSize, size_t& allocationSize)
{
	const size_t maximumSize = (std::numeric_limits<size_t>::max)();
	if (requestedSize > maximumSize - sizeof(SCryAllocationHeader))
	{
		allocationSize = 0;
		return false;
	}

	allocationSize = requestedSize + sizeof(SCryAllocationHeader);
	return true;
}

inline void* CryGetAllocationUserMemory(SCryAllocationHeader* header)
{
	return header + 1;
}

inline SCryAllocationHeader* CryGetAllocationHeader(void* memory)
{
	return static_cast<SCryAllocationHeader*>(memory) - 1;
}
