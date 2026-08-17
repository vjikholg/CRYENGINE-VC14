////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   CryMemoryManager_impl.h
//  Version:     v1.00
//  Created:     27/7/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: Provides implementation for CryMemoryManager globally defined functions.
//               This file included only by platform_impl.h, do not include it directly in code!
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
 
#ifndef __CryMemoryManager_impl_h__
#define __CryMemoryManager_impl_h__
#pragma once

#include "CryAllocatorFailure.h"
#include "CryMemoryByteCounter.h"

#include <atomic>
#include <limits>

#ifdef _LIB
	#include "ISystem.h"
#endif

#ifdef XENON
const int CM_MaxMemSize[eCryM_Num] = 
{
  0
};
/*const int CM_MaxMemSize[eCryM_Num] = 
{
  30,           // 3DEngine
  10,           // Action
  40,           // AISystem
  50,           // Animation
  10,           // EntitySystem
  5,            // Font
  1,            // Input
  1,            // Movie
  5,            // Network
  50,           // Physics
  10,           // ScriptSystem
  15,           // SoundSystem
  30,           // System
  30,           // Game
  150,          // Render
  10,           // Launcher
};*/
#else
const int CM_MaxMemSize[eCryM_Num] = 
{
  0
};
#endif

// In debug builds, pass the standard allocator routine in to override alloc behavior.
#if defined(_DEBUG) && !defined(PS3)
	// Standard lib alloc routine (used in debug builds).
	#undef malloc
	#undef calloc
	#undef realloc
	#undef free

	void* StdAlloc(void* pOrig, size_t nNew)
	{
		if (!pOrig)
			return malloc(nNew);
		else if (nNew)
			return realloc(pOrig, nNew);
		else
		{
			free(pOrig);
			return NULL;
		}
	}
	static TPFAlloc CustomAlloc = StdAlloc;
#else
	static TPFAlloc CustomAlloc = NULL;
#endif

#define DEBUG_ALLOC_FLAG	0x80000000

#ifndef XENON
#define DLL_ENTRY_CRYMALLOC "CryMalloc"
#define DLL_ENTRY_CRYFREE "CryFree"
#define DLL_ENTRY_CRYREALLOC "CryRealloc"
#define DLL_ENTRY_CRYGETMEMSIZE "CryGetMemSize"
#define DLL_ENTRY_CRYCRTMALLOC "CrySystemCrtMalloc"
#define DLL_ENTRY_CRYCRTFREE "CrySystemCrtFree"
#define DLL_ENTRY_CRYCRTSIZE "CrySystemCrtSize"
#define DLL_ENTRY_CRYALLOCATORFAILURE CryAllocatorFailureHandlerExportName
#else
#define DLL_ENTRY_CRYMALLOC      (LPCSTR)2
#define DLL_ENTRY_CRYFREE        (LPCSTR)3
#define DLL_ENTRY_CRYREALLOC     (LPCSTR)4
#define DLL_ENTRY_CRYGETMEMSIZE  (LPCSTR)5
#define DLL_ENTRY_CRYCRTMALLOC 	 (LPCSTR)13
#define DLL_ENTRY_CRYCRTFREE		 (LPCSTR)14
#define DLL_ENTRY_CRYCRTSIZE		 (LPCSTR)16
#endif

#ifdef _LIB
  const char *CM_Name[eCryM_Num] = 
  {
    "Cry3DEngine",
    "CryAction",
    "CryAISystem",
    "CryAnimation",
    "CryEntitySystem",
    "CryFont",
    "CryInput",
    "CryMovie",
    "CryNetwork",
    "CryPhysics",
    "CryScriptSystem",
    "CrySoundSystem",
    "CrySystem",
    "CryGame",
    "CryRender",
    "Launcher",
  };
#endif

#if defined(PS3)
// On PS3, CryMalloc and friends may be exported from a PRX (unless PS3_NOPRX
// is defined, indicating that the entire solution is linked statically).  PRX
// exported functions may _not_ be called through a function pointer, so we
// need a wrapper to call them.
#if !defined(PS3_NOPRX)
void *WrappedCryMalloc(size_t size, size_t &allocated)
{
	return CryMalloc(size, allocated);
}
void *WrappedCryRealloc(void *memblock, size_t size, size_t &allocated)
{
	return CryRealloc(memblock, size, allocated);
}
size_t WrappedCryFree(void *memblock)
{
	return CryFree(memblock);
}
size_t WrappedCryGetMemSize(void *memblock, size_t sourceSize)
{
	return CryGetMemSize(memblock, sourceSize);
}
void *WrappedCrySystemCrtMalloc(size_t size)
{
	return CrySystemCrtMalloc(size);
}
void WrappedCrySystemCrtFree(void *memblock)
{
	CrySystemCrtFree(memblock);
}
size_t WrappedCrySystemCrtSize(void *memblock)
{
	return CrySystemCrtSize(memblock);
}
#else
#define WrappedCryMalloc CryMalloc
#define WrappedCryRealloc CryRealloc
#define WrappedCryFree CryFree
#define WrappedCryGetMemSize CryGetMemSize
#define WrappedCrySystemCrtMalloc CrySystemCrtMalloc
#define WrappedCrySystemCrtFree CrySystemCrtFree
#define WrappedCrySystemCrtSize CrySystemCrtSize
#endif
#endif

//////////////////////////////////////////////////////////////////////////
// _PoolHelper definition.
//////////////////////////////////////////////////////////////////////////
struct _CryMemoryManagerPoolHelper
{

  typedef void *(*FNC_CryMalloc)(size_t size, size_t& allocated);
	typedef void *(*FNC_CryCrtMalloc)(size_t size);
  typedef void *(*FNC_CryRealloc)(void *memblock, size_t size, size_t& allocated);
  typedef size_t (*FNC_CryFree)(void *p);

#ifndef _LIB
	static CryMemoryByteCounter allocatedMemory;
	static CryMemoryByteCounter freedMemory;
	static CryMemoryByteCounter requestedMemory;
	static std::atomic<uint64> numAllocations;
#else
	static CryMemoryByteCounter allocatedMemory[eCryM_Num];
	static CryMemoryByteCounter freedMemory[eCryM_Num];
	static CryMemoryByteCounter requestedMemory[eCryM_Num];
	static std::atomic<uint64> numAllocations[eCryM_Num];
#endif

//	typedef size_t (*FNC_CryFree)(void);
	typedef size_t (*FNC_CryGetMemSize)(void *p, size_t);
	typedef int (*FNC_CryStats)(char *buf);
	
	static FNC_CryMalloc _CryMalloc;
	static FNC_CryRealloc _CryRealloc;
	static FNC_CryFree _CryFree;
	static FNC_CryGetMemSize _CryGetMemSize;
	static FNC_CryCrtMalloc _CryCrtMalloc;
	static FNC_CryFree _CryCrtFree;
	static FNC_CryFree _CryCrtSize;
	static CryAllocatorFailureHandler _CryAllocatorFailure;

	static int m_bInitialized;

#if defined PS3_PRX
	// For a PRX build, the Init() method must be visible for the *_PRX.o, so it
	// may not be inlined.  With 'noinline', GCC will place the Init() method
	// into a linkonce section, which is OK.
	__attribute__ ((noinline))
#endif
	static void Init()
	{
		if(m_bInitialized)
			return;

		m_bInitialized = 1;
		
#ifndef _LIB
		allocatedMemory.Reset();
		freedMemory.Reset();
		requestedMemory.Reset();
		numAllocations.store(0, std::memory_order_relaxed);

#if defined(LINUX)
		_CryMalloc=CryMalloc;
		_CryRealloc=CryRealloc;
		_CryFree=CryFree;
		_CryGetMemSize=CryGetMemSize;
		_CryCrtMalloc=CrySystemCrtMalloc;
		_CryCrtFree=(FNC_CryFree)CrySystemCrtFree;
		_CryCrtSize=(FNC_CryFree)CrySystemCrtSize;
#elif defined(PS3)
		_CryMalloc=WrappedCryMalloc;
		_CryRealloc=WrappedCryRealloc;
		_CryFree=WrappedCryFree;
		_CryGetMemSize=WrappedCryGetMemSize;
		_CryCrtMalloc=WrappedCrySystemCrtMalloc;
		_CryCrtFree=(FNC_CryFree)WrappedCrySystemCrtFree;
		_CryCrtSize=(FNC_CryFree)WrappedCrySystemCrtSize;
#else
		HMODULE hMod;
		int iter;
		for(iter=0,hMod=GetModuleHandle(0); hMod; iter++)
		{
			_CryMalloc=(FNC_CryMalloc)GetProcAddress((HINSTANCE)hMod,DLL_ENTRY_CRYMALLOC);
			_CryRealloc=(FNC_CryRealloc)GetProcAddress((HINSTANCE)hMod,DLL_ENTRY_CRYREALLOC);
			_CryFree=(FNC_CryFree)GetProcAddress((HINSTANCE)hMod,DLL_ENTRY_CRYFREE);
			_CryGetMemSize=(FNC_CryGetMemSize)GetProcAddress((HINSTANCE)hMod,DLL_ENTRY_CRYGETMEMSIZE);
			_CryCrtMalloc=(FNC_CryCrtMalloc)GetProcAddress((HINSTANCE)hMod,DLL_ENTRY_CRYCRTMALLOC);
			_CryCrtFree=(FNC_CryFree)GetProcAddress((HINSTANCE)hMod,DLL_ENTRY_CRYCRTFREE);
			_CryCrtSize=(FNC_CryFree)GetProcAddress((HINSTANCE)hMod,DLL_ENTRY_CRYCRTSIZE);
			_CryAllocatorFailure=reinterpret_cast<CryAllocatorFailureHandler>(
				GetProcAddress((HINSTANCE)hMod,DLL_ENTRY_CRYALLOCATORFAILURE));
			if ((_CryMalloc && _CryRealloc && _CryFree && _CryGetMemSize && _CryCrtMalloc && _CryCrtFree && _CryCrtSize) || iter==1)
				break;
      hMod = ::LoadLibrary("CrySystem.dll"); 
		}
		if (!hMod || !_CryMalloc || !_CryRealloc || !_CryFree || !_CryGetMemSize ||! _CryCrtMalloc || !_CryCrtFree || !_CryCrtSize)
		{
#ifndef AVOID_MEMORY_ERROR
			#ifdef WIN32
				MessageBox(NULL, "Could not access CrySystem.dll (check working directory)", "Memory Manager", MB_OK);
      #else
      if (!hMod)
        OutputDebugString("Could not access CrySystem.dll (check working directory)");
      else
        OutputDebugString("Could not get Memory Functions in CrySystem.dll");
			#endif
			exit(1);
#endif
		};
#endif // LINUX || PS3
#else
    for (int i=0; i<eCryM_Num; i++)
    {
		allocatedMemory[i].Reset();
		freedMemory[i].Reset();
		requestedMemory[i].Reset();
		numAllocations[i].store(0, std::memory_order_relaxed);
    }
#if defined(PS3)
		_CryMalloc=WrappedCryMalloc;
		_CryRealloc=WrappedCryRealloc;
		_CryFree=WrappedCryFree;
		_CryGetMemSize=WrappedCryGetMemSize;
		_CryCrtMalloc=WrappedCrySystemCrtMalloc;
		_CryCrtFree=(FNC_CryFree)WrappedCrySystemCrtFree;
		_CryCrtSize=(FNC_CryFree)WrappedCrySystemCrtSize;
#else
		_CryMalloc=CryMalloc;
		_CryRealloc=CryRealloc;
		_CryFree=CryFree;
		_CryGetMemSize=CryGetMemSize;
		_CryCrtMalloc=CrySystemCrtMalloc;
		_CryCrtFree=(FNC_CryFree)CrySystemCrtFree;
		_CryCrtSize=(FNC_CryFree)CrySystemCrtSize;
#endif
#endif
	}
 
#ifndef _LIB
	static void GetMemoryInfo( CryModuleMemoryInfo *pMmemInfo )
	{
		pMmemInfo->allocated = allocatedMemory.Load();
		pMmemInfo->freed = freedMemory.Load();
		pMmemInfo->requested = requestedMemory.Load();
		const uint64 allocationCount = numAllocations.load(std::memory_order_relaxed);
		pMmemInfo->num_allocations = allocationCount > static_cast<uint64>(std::numeric_limits<int>::max())
			? std::numeric_limits<int>::max()
			: static_cast<int>(allocationCount);
#ifdef CRY_STRING
		pMmemInfo->CryString_allocated = string::_usedMemory(0) + wstring::_usedMemory(0);
#endif
		pMmemInfo->STL_allocated = 0;
		pMmemInfo->STL_wasted = 0;
#if defined(CRY_STL_ALLOC) && defined(STLPORT)
		std::vector<int> dummy;
		pMmemInfo->STL_allocated = dummy.get_allocator().get_heap_size();
		pMmemInfo->STL_wasted = dummy.get_allocator().get_wasted_in_allocation() + dummy.get_allocator().get_wasted_in_blocks();
#endif
	}
#else
  static void GetMemoryInfo( CryModuleMemoryInfo *pMmemInfo, ECryModule eCM )
  {
		pMmemInfo->allocated = allocatedMemory[eCM].Load();
		pMmemInfo->freed = freedMemory[eCM].Load();
		pMmemInfo->requested = requestedMemory[eCM].Load();
		const uint64 allocationCount = numAllocations[eCM].load(std::memory_order_relaxed);
		pMmemInfo->num_allocations = allocationCount > static_cast<uint64>(std::numeric_limits<int>::max())
			? std::numeric_limits<int>::max()
			: static_cast<int>(allocationCount);
  }
#endif

	//////////////////////////////////////////////////////////////////////////
	// Local version of allocations, does memory counting per module.
	//////////////////////////////////////////////////////////////////////////
#ifndef _LIB
	static __forceinline void*	Malloc(size_t size)
#else
  static __forceinline void*	Malloc(size_t size, ECryModule eCM)
#endif
	{
		if (!m_bInitialized)
			Init();
		size_t allocated;
#ifndef __MEMORY_VALIDATOR_HACK
		void * p = _CryMalloc( size, allocated );
# ifndef _LIB
		allocatedMemory.Add(allocated);
# else
		allocatedMemory[eCM].Add(allocated);
# endif
#else
		void * p = _CryMalloc( size);
		allocatedMemory.Add(_CryGetMemSize(p));
#endif

#ifndef _LIB
		requestedMemory.Add(size);
		numAllocations.fetch_add(1, std::memory_order_relaxed);
#else
		requestedMemory[eCM].Add(size);
		numAllocations[eCM].fetch_add(1, std::memory_order_relaxed);
		if (CM_MaxMemSize[eCM]>0 && requestedMemory[eCM].Load()/(1024*1024) > CM_MaxMemSize[eCM])
    {
      CryError("Memory usage for module '%s' exceed limit (%d Mb)", CM_Name[eCM], CM_MaxMemSize[eCM]);
    }
#endif

		/*
		if (CustomAlloc || !_CryMalloc)
		{
			// Use and remember custom allocation routine.
			// Flag memory block by setting size negative.
			size_t* p = (size_t*)CustomAlloc(NULL, size + 2*sizeof(size_t));
			*p++ = (size_t)CustomAlloc;
			*p++ = size | DEBUG_ALLOC_FLAG;
			return p;
		}
		*/
		return p;
	}
	//////////////////////////////////////////////////////////////////////////
	// Local version of allocations, does memory counting per module.
	//////////////////////////////////////////////////////////////////////////
#ifndef _LIB
	static __forceinline void*	Calloc(size_t num,size_t size)
	{
		size_t allocationSize = 0;
		if (!CryTryMultiplySize(num, size, allocationSize))
		{
			CRY_ALLOCATOR_SIZE_FAILURE(
				"CryModuleCalloc",
				num,
				size,
				size == 0 ? (std::numeric_limits<size_t>::max)() :
					(std::numeric_limits<size_t>::max)() / size,
				alignof(std::max_align_t),
				0,
				ECryAllocatorFailureAction::ReturnNull);
			return nullptr;
		}

		void* const p = Malloc(allocationSize);
		if (p != nullptr)
			memset(p, 0, allocationSize);
		return p;
	}
#else
  static __forceinline void*	Calloc(size_t num,size_t size, ECryModule eCM)
  {
		size_t allocationSize = 0;
		if (!CryTryMultiplySize(num, size, allocationSize))
		{
			CRY_ALLOCATOR_SIZE_FAILURE(
				"CryModuleCalloc",
				num,
				size,
				size == 0 ? (std::numeric_limits<size_t>::max)() :
					(std::numeric_limits<size_t>::max)() / size,
				alignof(std::max_align_t),
				0,
				ECryAllocatorFailureAction::ReturnNull);
			return nullptr;
		}

		void* const p = Malloc(allocationSize, eCM);
		if (p != nullptr)
			memset(p, 0, allocationSize);
		return p;
  }
#endif

	//////////////////////////////////////////////////////////////////////////
#ifndef _LIB
	static __forceinline void*	Realloc(void *memblock,size_t size)
#else
  static __forceinline void*	Realloc(void *memblock,size_t size, ECryModule eCM)
#endif
	{
		if (!m_bInitialized)
			Init();
		if (memblock == NULL)
		{
			//numAllocations++;
			//allocatedMemory += size;
#ifndef _LIB
			return Malloc(size);
#else
      return Malloc(size, eCM);
#endif
		}

//		int* t = (int*)memblock;
		size_t oldsize = _CryGetMemSize(memblock, size);//*--t;	

		/*
		size_t* p = NULL;
		if (oldsize & DEBUG_ALLOC_FLAG)
		{
			// Allocated with custom allocator.
			TPFAlloc opa = (TPFAlloc)*--t;
			if (opa == CustomAlloc)
			{
				// Same allocator, use its realloc.
				p = (size_t*)CustomAlloc(t, size + 2*sizeof(size_t));
				*p++ = (size_t)CustomAlloc;
				*p++ = size | DEBUG_ALLOC_FLAG;
				return p;
			}
			else
			{
				// Separate malloc and free.
				p = (size_t*)Malloc(size);
				memcpy(p, memblock, oldsize);
				Free(memblock);
			}
		}
*/
		size_t allocated;
#ifndef ___MEMORY_VALIDATOR_HACK
		void * p=  _CryRealloc( memblock, size, allocated );
		if (p == nullptr && size != 0)
			return nullptr;
# ifndef _LIB
		allocatedMemory.Add(allocated);
# else
		allocatedMemory[eCM].Add(allocated);
# endif
#else
		void * p=  _CryRealloc( memblock, size);
		if (p == nullptr && size != 0)
			return nullptr;
		allocatedMemory.Add(_CryGetMemSize(p));
#endif

#ifndef _LIB
		numAllocations.fetch_add(1, std::memory_order_relaxed);
		requestedMemory.Add(size);
		oldsize &= ~DEBUG_ALLOC_FLAG;
		freedMemory.Add(oldsize);
#else
		numAllocations[eCM].fetch_add(1, std::memory_order_relaxed);
		requestedMemory[eCM].Add(size);
		oldsize &= ~DEBUG_ALLOC_FLAG;
		freedMemory[eCM].Add(oldsize);
		if (CM_MaxMemSize[eCM]>0 && requestedMemory[eCM].Load()/(1024*1024) > CM_MaxMemSize[eCM])
    {
      CryError("Memory usage for module '%s' exceed limit (%d Mb)", CM_Name[eCM], CM_MaxMemSize[eCM]);
    }
#endif
/*
		if (p)
			return p;
		else
		*/
		return p;
			
	}
	//////////////////////////////////////////////////////////////////////////
#ifndef _LIB
	static __forceinline void Free( void *memblock )
#else
  static __forceinline void Free( void *memblock, ECryModule eCM )
#endif
	{
		if (!m_bInitialized)
			Init();
		if (memblock != 0)
		{
			/*
			if (size & DEBUG_ALLOC_FLAG)
			{
				// Free using custom allocator.
				size &= ~DEBUG_ALLOC_FLAG;
				TPFAlloc pa = (TPFAlloc)*--t;
				pa(t, 0);
			}
			else
			*/
#ifndef _LIB				
			freedMemory.Add(_CryFree(memblock));
#else
			freedMemory[eCM].Add(_CryFree(memblock));
#endif
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// Static variables.
//////////////////////////////////////////////////////////////////////////
#ifndef _LIB
CryMemoryByteCounter _CryMemoryManagerPoolHelper::allocatedMemory;
CryMemoryByteCounter _CryMemoryManagerPoolHelper::freedMemory;
CryMemoryByteCounter _CryMemoryManagerPoolHelper::requestedMemory;
std::atomic<uint64> _CryMemoryManagerPoolHelper::numAllocations{0};
#else
CryMemoryByteCounter _CryMemoryManagerPoolHelper::allocatedMemory[eCryM_Num];
CryMemoryByteCounter _CryMemoryManagerPoolHelper::freedMemory[eCryM_Num];
CryMemoryByteCounter _CryMemoryManagerPoolHelper::requestedMemory[eCryM_Num];
std::atomic<uint64> _CryMemoryManagerPoolHelper::numAllocations[eCryM_Num] = {};
#endif
_CryMemoryManagerPoolHelper::FNC_CryMalloc _CryMemoryManagerPoolHelper::_CryMalloc = NULL;
_CryMemoryManagerPoolHelper::FNC_CryGetMemSize _CryMemoryManagerPoolHelper::_CryGetMemSize = NULL;
_CryMemoryManagerPoolHelper::FNC_CryRealloc _CryMemoryManagerPoolHelper::_CryRealloc = NULL;
_CryMemoryManagerPoolHelper::FNC_CryFree _CryMemoryManagerPoolHelper::_CryFree = NULL;
_CryMemoryManagerPoolHelper::FNC_CryCrtMalloc _CryMemoryManagerPoolHelper::_CryCrtMalloc = NULL;
_CryMemoryManagerPoolHelper::FNC_CryFree _CryMemoryManagerPoolHelper::_CryCrtFree = NULL;
_CryMemoryManagerPoolHelper::FNC_CryFree _CryMemoryManagerPoolHelper::_CryCrtSize = NULL;
CryAllocatorFailureHandler _CryMemoryManagerPoolHelper::_CryAllocatorFailure = nullptr;
int _CryMemoryManagerPoolHelper::m_bInitialized = 0;
//////////////////////////////////////////////////////////////////////////

void CryAllocatorSizeFailure(const SCryAllocatorFailureContext& context) noexcept
{
	if (!_CryMemoryManagerPoolHelper::m_bInitialized)
		_CryMemoryManagerPoolHelper::Init();

	if (_CryMemoryManagerPoolHelper::_CryAllocatorFailure != nullptr)
	{
		_CryMemoryManagerPoolHelper::_CryAllocatorFailure(&context);
		if (context.action == ECryAllocatorFailureAction::ReturnNull)
			return;
	}

	char message[1024] = {};
	_snprintf_s(
		message,
		sizeof(message),
		_TRUNCATE,
		"Allocator size failure: operation=%s count=%llu elementSize=%llu maxCount=%llu function=%s\n",
		context.operation,
		static_cast<unsigned long long>(context.count),
		static_cast<unsigned long long>(context.elementSize),
		static_cast<unsigned long long>(context.maximumElementCount),
		context.function);
	OutputDebugStringA(message);

	if (context.action == ECryAllocatorFailureAction::WriteDumpAndTerminate)
	{
		RaiseFailFastException(nullptr, nullptr, 0);
		TerminateProcess(GetCurrentProcess(), CryAllocatorFailureExitCode);
	}
}

#ifndef _LIB
//////////////////////////////////////////////////////////////////////////
void* CryModuleMalloc( size_t size ) throw()
{
	return _CryMemoryManagerPoolHelper::Malloc(size);
};
//////////////////////////////////////////////////////////////////////////
void* CryModuleCalloc( size_t num,size_t size ) throw()
{
	return _CryMemoryManagerPoolHelper::Calloc(num,size);
};
//////////////////////////////////////////////////////////////////////////
void* CryModuleRealloc( void *ptr,size_t size )  throw()
{
	return _CryMemoryManagerPoolHelper::Realloc(ptr,size);
};
//////////////////////////////////////////////////////////////////////////
void  CryModuleFree( void *ptr ) throw()
{
	_CryMemoryManagerPoolHelper::Free(ptr);
};
//////////////////////////////////////////////////////////////////////////
extern "C" DLL_EXPORT void CryModuleGetMemoryInfo( CryModuleMemoryInfo *pMemInfo )
{
	_CryMemoryManagerPoolHelper::GetMemoryInfo(pMemInfo);
};

void CryGetMemoryInfoForModule(CryModuleMemoryInfo * pInfo)
{
	_CryMemoryManagerPoolHelper::GetMemoryInfo(pInfo);
};



#else

//////////////////////////////////////////////////////////////////////////
void* CryModuleMalloc( size_t size, ECryModule eCM ) throw()
{
  return _CryMemoryManagerPoolHelper::Malloc(size, eCM);
};
//////////////////////////////////////////////////////////////////////////
void* CryModuleCalloc( size_t num,size_t size, ECryModule eCM ) throw()
{
  return _CryMemoryManagerPoolHelper::Calloc(num,size,eCM);
};
//////////////////////////////////////////////////////////////////////////
void* CryModuleRealloc( void *ptr,size_t size, ECryModule eCM )  throw()
{
  return _CryMemoryManagerPoolHelper::Realloc(ptr,size, eCM);
};
//////////////////////////////////////////////////////////////////////////
void  CryModuleFree( void *ptr, ECryModule eCM ) throw()
{
  _CryMemoryManagerPoolHelper::Free(ptr, eCM);
};
//////////////////////////////////////////////////////////////////////////
extern "C" DLL_EXPORT void CryModuleGetMemoryInfo( CryModuleMemoryInfo *pMemInfo, ECryModule eCM )
{
  _CryMemoryManagerPoolHelper::GetMemoryInfo(pMemInfo, eCM);
};
//////////////////////////////////////////////////////////////////////////
void CryGetMemoryInfoForModule(CryModuleMemoryInfo * pInfo)
{
	_CryMemoryManagerPoolHelper::GetMemoryInfo(pInfo, eCryModule);
};
#endif

void *CryCrtMalloc(size_t size)
{
#if defined(NOT_USE_CRY_MEMORY_MANAGER) || defined(_DEBUG)
	return malloc(size);
#else
	_CryMemoryManagerPoolHelper::Init();	// make sure we're initialized!
	return _CryMemoryManagerPoolHelper::_CryCrtMalloc(size);
#endif
}

void CryCrtFree(void *p)
{
#if defined(NOT_USE_CRY_MEMORY_MANAGER) || defined(_DEBUG)
	free(p);
#else
	_CryMemoryManagerPoolHelper::_CryCrtFree(p);
#endif
};

size_t CryCrtSize(void *p)
{
#if defined(NOT_USE_CRY_MEMORY_MANAGER) || (defined(_DEBUG) && !defined(PS3))
#if LINUX
	assert(false && "need _msize");
	return 0;
#else
	return CryGetNativeCrtAllocationSize(p);
#endif
#else
	return _CryMemoryManagerPoolHelper::_CryCrtSize(p);
#endif
};


// If using CryMemoryManager, redefine new & delete for entire module.
#if (!defined(_DEBUG) || defined(PS3)) && !defined(NOT_USE_CRY_MEMORY_MANAGER) && !defined(__SPU__)
	#ifndef _LIB
		void * __cdecl operator new   (size_t size) { return CryModuleMalloc(size); } 
		void * __cdecl operator new[] (size_t size) { return CryModuleMalloc(size); }; 
		void __cdecl operator delete  (void *p) { CryModuleFree(p); };
		void __cdecl operator delete[](void *p) { CryModuleFree(p); };
	#endif//_LIB
#endif //!defined(_DEBUG) && !defined(NOT_USE_CRY_MEMORY_MANAGER) && !defined(__SPU__)

//////////////////////////////////////////////////////////////////////////

#include "CrySafeAlloc_impl.h"

#endif // __CryMemoryManager_impl_h__

