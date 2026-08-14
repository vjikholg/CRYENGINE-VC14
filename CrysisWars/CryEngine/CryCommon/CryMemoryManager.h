//////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   CryMemoryManager.h
//  Description: Declares the Win64 CrySystem allocator API and per-module
//               allocation wrappers. CryMemoryManager_impl.h must be
//               included exactly once per module through platform_impl.h.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "platform.h"

#include <cstddef>
#include <malloc.h>
#include <new>

// Kept for legacy call sites that use this as an optional cleanup statement.
#ifndef STLALLOCATOR_CLEANUP
#define STLALLOCATOR_CLEANUP
#endif

#if defined(_DEBUG) && !defined(RELEASE_RUNTIME)
#include <crtdbg.h>

ILINE int IsHeapValid()
{
	return _CrtCheckMemory();
}
#else
ILINE int IsHeapValid()
{
	return true;
}
#endif

struct CryModuleMemoryInfo
{
	uint64 requested;
	uint64 allocated;
	uint64 freed;
	int num_allocations;
	uint64 CryString_allocated;
	uint64 STL_allocated;
	uint64 STL_wasted;
};

// CrySystem owns these functions. Other runtime modules import them.
#if defined(CRYSYSTEM_EXPORTS)
#define CRY_MEMORY_API DLL_EXPORT
#else
#define CRY_MEMORY_API DLL_IMPORT
#endif

// Retained with C++ linkage so existing binaries keep the original decorated
// symbol. New runtime code should use CryMemoryGetAllocatedSize64().
CRY_MEMORY_API int CryMemoryGetAllocatedSize();

extern "C"
{
	CRY_MEMORY_API void* CryMalloc(size_t size, size_t& allocated);
	CRY_MEMORY_API void* CryRealloc(void* memory, size_t size, size_t& allocated);
	CRY_MEMORY_API size_t CryFree(void* memory);
	CRY_MEMORY_API size_t CryGetMemSize(void* memory, size_t sourceSize);
	CRY_MEMORY_API int CryStats(char* buffer);
	CRY_MEMORY_API void CryFlushAll();
	CRY_MEMORY_API void CryCleanup();
	CRY_MEMORY_API int CryGetUsedHeapSize();
	CRY_MEMORY_API int CryGetWastedHeapSize();
	CRY_MEMORY_API uint64 CryMemoryGetAllocatedSize64();
	CRY_MEMORY_API uint64 CryGetUsedHeapSize64();
	CRY_MEMORY_API uint64 CryGetWastedHeapSize64();
	CRY_MEMORY_API void* CrySystemCrtMalloc(size_t size);
	CRY_MEMORY_API void CrySystemCrtFree(void* memory);
	CRY_MEMORY_API size_t CrySystemCrtSize(void* memory);

	// Implemented locally by CryMemoryManager_impl.h in every engine module.
	void CryGetMemoryInfoForModule(CryModuleMemoryInfo* memoryInfo);
}

#undef CRY_MEMORY_API

// platform_impl.h uses this marker to include the per-module implementation.
#define USING_CRY_MEMORY_MANAGER

extern "C"
{
	void* CryModuleMalloc(size_t size) throw();
	DLL_EXPORT void* CryModuleCalloc(size_t count, size_t size) throw();
	void* CryModuleRealloc(void* memory, size_t size) throw();
	void CryModuleFree(void* memory) throw();
}

// Every runtime DLL exports its own accounting snapshot for CrySystem.
extern "C" DLL_EXPORT void CryModuleGetMemoryInfo(CryModuleMemoryInfo* memoryInfo);

// Redirect module CRT allocations except in debug builds and modules that
// explicitly opt out of Cry's allocator.
#if !defined(_DEBUG) && !defined(NOT_USE_CRY_MEMORY_MANAGER)
	#undef malloc
	#undef calloc
	#undef realloc
	#undef free
	#undef memalign

	#define malloc CryModuleMalloc
	#define calloc CryModuleCalloc
	#define realloc CryModuleRealloc
	#define free CryModuleFree
#endif

// Forces Windows to commit physical pages until an allocation fails. This is
// intentionally retained as a legacy diagnostic command.
#define MEMORY_DEBUG_POINT(pSystem, no) \
	{ assert(no); static ICVar* pVar = pSystem->GetIConsole()->GetCVar("sys_memory_debug"); assert(pVar); \
		if (pVar && pVar->GetIVal() == no) { \
			void* pMem = 0; uint32 dwSumMB = 0, dwSizeMB = 10; \
			while (pMem = malloc(dwSizeMB * 1024 * 1024)) \
			{ dwSumMB += dwSizeMB; memset(pMem, no, dwSizeMB * 1024 * 1024); } \
			if (pMem) free(pMem); \
			pSystem->GetILog()->Log("MEMORY_DEBUG_POINT %d activated, %d MB", no, dwSumMB); } }

// Local wrappers used by the allocator to avoid recursive module allocation.
void* CryCrtMalloc(size_t size);
void CryCrtFree(void* memory);
size_t CryCrtSize(void* memory);

#if !defined(NOT_USE_CRY_MEMORY_MANAGER)
#include "CryMemoryAllocator.h"
#endif

typedef void* (*TPFAlloc)(void* memory, size_t size);

ILINE void* ModuleAlloc(void* memory, size_t size)
{
	if (size)
		return memory ? realloc(memory, size) : malloc(size);
	if (memory)
		free(memory);
	return 0;
}

void* SafeAlloc(
	TPFAlloc allocator,
	void* memory,
	size_t size,
	size_t alignment = alignof(std::max_align_t));
