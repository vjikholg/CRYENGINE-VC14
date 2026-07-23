#ifndef __STREAMALLOCATOR_H__
#define __STREAMALLOCATOR_H__

#pragma once

#include <cstddef>

struct IStreamAllocator
{
	virtual void* Alloc(std::size_t size, void* callerOverride = 0) = 0;
	virtual void* Realloc(void* old, std::size_t size) = 0;
	virtual void Free(void* old) = 0;
};

#endif
