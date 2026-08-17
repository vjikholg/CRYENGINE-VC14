#pragma once

#include <cstddef>
#include <malloc.h>

inline size_t CryGetNativeCrtAllocationSize(void* memory) noexcept
{
	if (memory == nullptr)
		return 0;

#if defined(_WIN32)
	return _msize(memory);
#else
	return 0;
#endif
}
