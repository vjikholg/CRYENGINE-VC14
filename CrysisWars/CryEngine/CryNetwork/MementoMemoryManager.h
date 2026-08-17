#ifndef __MEMENTOMEMORYMANAGER_H__
#define __MEMENTOMEMORYMANAGER_H__

#pragma once

#include <platform.h>
#include <smartptr.h>
#include <CryAllocatorFailure.h>

#include "STLPoolAllocator.h"
#include "Config.h"
#include "StreamAllocator.h"

#include <atomic>
#include <cstdint>
#include <vector>

class ICrySizer;
void CryError(const char*, ...);

struct IMementoManagedThing
{
	virtual void Release() = 0;
};

// handle based memory manager that can repack data to save fragmentation
class CMementoMemoryManager : public CMultiThreadRefCount
{
	friend class CMementoStreamAllocator;

private:
	struct SHandleData;

public:
	static constexpr size_t ALIGNMENT = alignof(std::max_align_t);
	using Hdl = uint64_t;
	static constexpr Hdl InvalidHdl = ~Hdl(0);

	// hack for arithmetic alphabet stuff
	Hdl arith_zeroSizeHdl;
	IMementoManagedThing * pThings[64];

	// who's using pThings:
	//   0 - arith row sym cache
	//   1 - arith row low cache

	CMementoMemoryManager();
	~CMementoMemoryManager();

	void * AllocPtr( size_t sz, void * callerOverride = 0 );
	void FreePtr( void * p, size_t sz );
	Hdl AllocHdl( size_t sz, void * callerOverride = 0 );
	Hdl CloneHdl( Hdl hdl );
	void ResizeHdl( Hdl hdl, size_t sz );
	void FreeHdl( Hdl hdl );
	void AddHdlToSizer( Hdl hdl, ICrySizer * pSizer );
	bool IsValidHdl(Hdl hdl) const noexcept;
	size_t GetLiveAllocationBytes() const noexcept
	{
		return m_totalAllocations;
	}
	ILINE void* PinHdl(Hdl hdl) const;
	ILINE size_t GetHdlSize(Hdl hdl) const;

	void GetMemoryStatistics(ICrySizer* pSizer, bool countingThis = false);

	void DebugDraw( int n );

private:
	static constexpr uint64_t HandleIndexMask = 0xffffffffull;
	static constexpr unsigned HandleGenerationShift = 32;

	static uint32 NextHandleCookie() noexcept;
	Hdl EncodeHdl(uint32 index, uint32 generation) const noexcept;
	SHandleData* ResolveHdl(Hdl hdl, uint32* index = nullptr) noexcept;
	const SHandleData* ResolveHdl(
		Hdl hdl,
		uint32* index = nullptr) const noexcept;

	struct SPoolStats
	{
		SPoolStats() : allocated(0), used(0) {}
		size_t allocated;
		size_t used;
		float GetWastePercent() const
		{
			if (!allocated)
				return 0.0f;
			const float portionUsed = float(used) / float(allocated);
			return 100.0f * (1.0f - portionUsed);
		}
	};
	struct IPool
	{
		virtual void * Allocate( size_t& alloced ) = 0;
		virtual void Free( void * p ) = 0;
		virtual SPoolStats GetPoolStats() { return SPoolStats(); }
		virtual ~IPool() {}
#ifdef CHECK_POOL_ALLOCATED_POINTERS
		virtual void CheckPtr( void * p ) = 0;
#endif
	};
	template <size_t SZ>
	class CPool : public IPool
	{
	public:
		virtual void * Allocate( size_t& alloced )
		{
			alloced = SZ;
			return m_impl.Allocate();
		}
		virtual void Free( void * p )
		{
			m_impl.Deallocate( p );
		}
		virtual SPoolStats GetPoolStats()
		{
			SPoolStats stats;
			stats.allocated = m_impl.GetTotalAllocatedMemory();
			stats.used = m_impl.GetTotalAllocatedNodeSize();
			return stats;
		}
#ifdef CHECK_POOL_ALLOCATED_POINTERS
		virtual void CheckPtr( void * p )
		{
			m_impl.CheckPtr(p);
		}
#endif
	private:
		stl::PoolAllocator<SZ, stl::PoolAllocatorSynchronizationSinglethreaded> m_impl;
	};
	class CLargeObjectPool : public IPool
	{
	public:
		virtual void * Allocate( size_t& ) { CryError("Cannot call Allocate() on this object."); return 0; }
		virtual void Free( void * p ) { CryModuleFree(p); }
		ILINE void * AllocateSized( size_t sz, size_t& alloced ) { alloced = sz; return CryModuleMalloc(sz); }
#ifdef CHECK_POOL_ALLOCATED_POINTERS
		virtual void CheckPtr( void * p )
		{
		}
#endif
	};

	// pool sizes are 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096

	static const int FIRST_POOL = 3; // == 8bytes
	static const int LAST_POOL = 12; // == 4096bytes
	static const int NPOOLS = LAST_POOL - FIRST_POOL + 1;

	IPool * m_pools[NPOOLS];
	CLargeObjectPool m_largeObjectPool;

	struct SHandleData
	{
		SHandleData()
			: p(nullptr)
			, size(0)
			, capacity(0)
			, pPool(nullptr)
			, generation(1)
			, allocated(false)
		{
		}

		void* p;
		size_t size;
		size_t capacity;
		IPool* pPool;
		uint32 generation;
		bool allocated;
	};

	std::vector<SHandleData> m_handles;
	std::vector<uint32> m_freeHandles;
	size_t m_totalAllocations;
	const uint32 m_handleCookie;

	static int GetPoolIndex(size_t size) noexcept;
	bool TryGetUpdatedAllocationTotal(
		size_t previousSize,
		size_t newSize,
		size_t& updatedTotal) const noexcept;
	bool InitHandleData(SHandleData& handleData, size_t size);
	void DrawDebugLine( int grp, int x, int y, const char * fmt, ... );

#if MMM_CHECK_LEAKS
	std::map<void*, void*> m_ptrToAlloc;
	std::map<uint32, void*> m_hdlToAlloc;
	std::map<void*, size_t> m_allocAmt;
#endif
};

inline uint32 CMementoMemoryManager::NextHandleCookie() noexcept
{
	static std::atomic<uint32> nextCookie{1};
	uint32 cookie = nextCookie.fetch_add(1, std::memory_order_relaxed);
	if (cookie == 0)
		cookie = nextCookie.fetch_add(1, std::memory_order_relaxed);
	return cookie;
}

inline void* CMementoMemoryManager::PinHdl(Hdl hdl) const
{
	const SHandleData* const handleData = ResolveHdl(hdl);
	if (handleData == nullptr)
		return nullptr;
#ifdef CHECK_POOL_ALLOCATED_POINTERS
	handleData->pPool->CheckPtr(handleData->p);
#endif
	return handleData->p;
}

inline size_t CMementoMemoryManager::GetHdlSize(Hdl hdl) const
{
	const SHandleData* const handleData = ResolveHdl(hdl);
	if (handleData == nullptr)
		return 0;
#ifdef CHECK_POOL_ALLOCATED_POINTERS
	handleData->pPool->CheckPtr(handleData->p);
#endif
	return handleData->size;
}

inline CMementoMemoryManager::Hdl CMementoMemoryManager::EncodeHdl(
	uint32 index,
	uint32 generation) const noexcept
{
	const uint64_t generationToken = generation ^ m_handleCookie;
	return (generationToken << HandleGenerationShift) | index;
}

inline const CMementoMemoryManager::SHandleData*
CMementoMemoryManager::ResolveHdl(
	Hdl hdl,
	uint32* resolvedIndex) const noexcept
{
	if (hdl == InvalidHdl)
		return nullptr;

	const uint64_t wideIndex = hdl & HandleIndexMask;
	if (wideIndex >= m_handles.size())
		return nullptr;

	const uint32 index = static_cast<uint32>(wideIndex);
	const SHandleData& handleData = m_handles[index];
	const uint32 generationToken =
		static_cast<uint32>(hdl >> HandleGenerationShift);
	if (!handleData.allocated ||
		generationToken != (handleData.generation ^ m_handleCookie))
	{
		return nullptr;
	}

	if (resolvedIndex != nullptr)
		*resolvedIndex = index;
	return &handleData;
}

inline CMementoMemoryManager::SHandleData* CMementoMemoryManager::ResolveHdl(
	Hdl hdl,
	uint32* resolvedIndex) noexcept
{
	return const_cast<SHandleData*>(
		static_cast<const CMementoMemoryManager*>(this)->ResolveHdl(
			hdl,
			resolvedIndex));
}

inline bool CMementoMemoryManager::IsValidHdl(Hdl hdl) const noexcept
{
	return ResolveHdl(hdl) != nullptr;
}

typedef CMementoMemoryManager::Hdl TMemHdl;

typedef _smart_ptr<CMementoMemoryManager> CMementoMemoryManagerPtr;

CMementoMemoryManager& MMM();

class CMementoStreamAllocator : public IStreamAllocator
{
public:
	CMementoStreamAllocator(const CMementoMemoryManagerPtr& mmm);

	void * Alloc( size_t sz, void * callerOverride );
	void * Realloc( void * old, size_t sz );
	void Free( void *old ); // WARNING: no-op (calling code uses GetHdl() to grab this...)

	TMemHdl GetHdl() const { return m_hdl; }

private:
	TMemHdl m_hdl;
	void * m_pPin;
	CMementoMemoryManagerPtr m_mmm;
};

#endif
