//---------------------------------------------------------------------------
// Copyright 2006 Crytek GmbH
// Created by: Michael Smith
//---------------------------------------------------------------------------
#ifndef __POOLALLOCATOR_H__
#define __POOLALLOCATOR_H__

#include "CryAllocatorFailure.h"
#include "PoolAllocatorSynchronization.h"

#include <cstddef>
#include <limits>
#include <new>

// Define to validate that pointers belong to this pool.
//#define CHECK_POOL_ALLOCATED_POINTERS

#ifdef CHECK_POOL_ALLOCATED_POINTERS
#include "CryArray.h"
#endif

namespace stl
{
	template <
		size_t S,
		typename L = PoolAllocatorSynchronizationMultithreaded,
		size_t A = 16>
	class PoolAllocator
	{
	public:
		static constexpr size_t ObjectSize = S;
		static constexpr size_t RequestedAlignment = A;
		static constexpr size_t ObjectAlignment = RequestedAlignment > sizeof(void*) ? RequestedAlignment : sizeof(void*);
		static constexpr size_t ObjectStorageSize = ObjectSize > sizeof(void*) ? ObjectSize : sizeof(void*);
		static constexpr size_t ObjectAllocationSize = (ObjectStorageSize + ObjectAlignment - 1) & ~(ObjectAlignment - 1);
		static constexpr size_t DefaultBucketSize = 128;
		static constexpr size_t BucketAlignment = ( ObjectAlignment > 16 ) ? ObjectAlignment : 16;

		using Synchronization = L;

		static_assert(ObjectSize != 0, "PoolAllocator cannot allocate zero-sized objects.");
		static_assert(
			RequestedAlignment != 0 &&
			(RequestedAlignment & (RequestedAlignment - 1)) == 0,
			"PoolAllocator alignment must be a non-zero power of two.");

		union alignas(ObjectAlignment) ObjectNode
		{
			ObjectNode* pNext;
			unsigned char storage[ObjectAllocationSize];
		};

		struct BucketNode
		{
			BucketNode* pNext;
		};

		static constexpr size_t BucketHeaderSize = (sizeof(BucketNode) + BucketAlignment - 1) & ~(BucketAlignment - 1);

		static_assert(sizeof(ObjectNode) == ObjectAllocationSize);
		static_assert(alignof(ObjectNode) == ObjectAlignment);
		static_assert(BucketHeaderSize % ObjectAlignment == 0);

		explicit PoolAllocator(size_t bucketSize = DefaultBucketSize);
		~PoolAllocator();

		PoolAllocator(const PoolAllocator&) = delete;
		PoolAllocator& operator=(const PoolAllocator&) = delete;

		void* Allocate();
		void Deallocate(void* object);
		void FreeMemory();

		size_t GetTotalAllocatedNodeSize();
		size_t GetTotalAllocatedMemory();
		void CheckPtr(void* pointer, bool allowNull = true);

		static bool TryCalculateBucketBytes(
			size_t bucketSize,
			size_t& bucketBytes) noexcept;

	private:
		static size_t CalculateBucketBytesOrThrow(size_t bucketSize);
		static void* AllocateBucketMemory(size_t bytes);
		static void FreeBucketMemory(void* memory) noexcept;
		static ObjectNode* FirstObject(BucketNode* bucket) noexcept;

		ObjectNode* GrowBucket();
		void CheckPtrUnlocked(void* pointer, bool allowNull) const;

		Synchronization synchronization;
		const size_t nBucketSize;
		const size_t nBucketSizeInBytes;
		BucketNode* pBucketList;
		ObjectNode* pFreeList;

#ifdef CHECK_POOL_ALLOCATED_POINTERS
		using PointerRange = std::pair<void*, void*>;
		using PointerRanges = DynArray<PointerRange>;
		PointerRanges legalPointers;
#endif
	};

	template <size_t S, typename L, size_t A>
	bool PoolAllocator<S, L, A>::TryCalculateBucketBytes(size_t bucketSize, size_t& bucketBytes) noexcept
	{
		bucketBytes = 0;
		if (bucketSize == 0)
			return false;

		size_t objectBytes = 0;
		if (!CryTryMultiplySize(bucketSize, sizeof(ObjectNode), objectBytes))
			return false;

		return CryTryAddSize(BucketHeaderSize, objectBytes, bucketBytes);
	}

	template <size_t S, typename L, size_t A>
	size_t PoolAllocator<S, L, A>::CalculateBucketBytesOrThrow(size_t bucketSize)
	{
		size_t bucketBytes = 0;
		if (TryCalculateBucketBytes(bucketSize, bucketBytes))
			return bucketBytes;

		CRY_ALLOCATOR_SIZE_FAILURE(
			"PoolAllocator::PoolAllocator",
			bucketSize,
			sizeof(ObjectNode),
			((std::numeric_limits<size_t>::max)() - BucketHeaderSize) /
				sizeof(ObjectNode),
			BucketAlignment,
			BucketHeaderSize,
			ECryAllocatorFailureAction::ReturnNull);
		throw std::bad_array_new_length();
	}

	template <size_t S, typename L, size_t A>
	PoolAllocator<S, L, A>::PoolAllocator(size_t bucketSize)
		: nBucketSize(bucketSize)
		, nBucketSizeInBytes(CalculateBucketBytesOrThrow(bucketSize))
		, pBucketList(nullptr)
		, pFreeList(nullptr)
	{
	}

	template <size_t S, typename L, size_t A>
	PoolAllocator<S, L, A>::~PoolAllocator()
	{
		FreeMemory();
	}

	template <size_t S, typename L, size_t A>
	void* PoolAllocator<S, L, A>::AllocateBucketMemory(size_t bytes)
	{
		return ::operator new(bytes, std::align_val_t(BucketAlignment));
	}

	template <size_t S, typename L, size_t A>
	void PoolAllocator<S, L, A>::FreeBucketMemory(void* memory) noexcept
	{
		::operator delete(memory, std::align_val_t(BucketAlignment));
	}

	template <size_t S, typename L, size_t A>
	typename PoolAllocator<S, L, A>::ObjectNode*
	PoolAllocator<S, L, A>::FirstObject(BucketNode* bucket) noexcept
	{
		return reinterpret_cast<ObjectNode*>(
			reinterpret_cast<unsigned char*>(bucket) + BucketHeaderSize);
	}

	template <size_t S, typename L, size_t A>
	void PoolAllocator<S, L, A>::CheckPtrUnlocked(
		void* pointer,
		bool allowNull) const
	{
		assert(allowNull || pointer != nullptr);
		assert(
			pointer == nullptr ||
			reinterpret_cast<uintptr_t>(pointer) % ObjectAlignment == 0);
		assert(
			pointer == nullptr ||
			reinterpret_cast<uintptr_t>(pointer) > 4095);
		(void)pointer;
		(void)allowNull;

#ifdef CHECK_POOL_ALLOCATED_POINTERS
		if (pointer != nullptr)
		{
			for (typename PointerRanges::const_iterator it = legalPointers.begin();
				it != legalPointers.end(); ++it)
			{
				if (pointer >= it->first && pointer < it->second)
					return;
			}
			assert(!"pointer not allocated by this pool allocator");
		}
#endif
	}

	template <size_t S, typename L, size_t A>
	void PoolAllocator<S, L, A>::CheckPtr(void* pointer, bool allowNull)
	{
		typename Synchronization::CLock lock(synchronization);
		CheckPtrUnlocked(pointer, allowNull);
	}

	template <size_t S, typename L, size_t A>
	void* PoolAllocator<S, L, A>::Allocate()
	{
		typename Synchronization::CLock lock(synchronization);

		if (pFreeList == nullptr)
			GrowBucket();

		ObjectNode* const node = pFreeList;
		pFreeList = node->pNext;
		CheckPtrUnlocked(node, false);
		return node;
	}

	template <size_t S, typename L, size_t A>
	void PoolAllocator<S, L, A>::Deallocate(void* object)
	{
		if (object == nullptr)
			return;

		typename Synchronization::CLock lock(synchronization);
		CheckPtrUnlocked(object, false);

		ObjectNode* const node = ::new (object) ObjectNode;
		node->pNext = pFreeList;
		pFreeList = node;
	}

	template <size_t S, typename L, size_t A>
	size_t PoolAllocator<S, L, A>::GetTotalAllocatedNodeSize()
	{
		typename Synchronization::CLock lock(synchronization);

		size_t bucketCount = 0;
		for (BucketNode* bucket = pBucketList; bucket != nullptr;
			bucket = bucket->pNext)
		{
			++bucketCount;
		}

		size_t freeNodeCount = 0;
		for (ObjectNode* node = pFreeList; node != nullptr; node = node->pNext)
			++freeNodeCount;

		return bucketCount * nBucketSize * sizeof(ObjectNode) -
			freeNodeCount * sizeof(ObjectNode);
	}

	template <size_t S, typename L, size_t A>
	size_t PoolAllocator<S, L, A>::GetTotalAllocatedMemory()
	{
		typename Synchronization::CLock lock(synchronization);

		size_t bucketCount = 0;
		for (BucketNode* bucket = pBucketList; bucket != nullptr;
			bucket = bucket->pNext)
		{
			++bucketCount;
		}
		return bucketCount * nBucketSizeInBytes;
	}

	template <size_t S, typename L, size_t A>
	typename PoolAllocator<S, L, A>::ObjectNode*
	PoolAllocator<S, L, A>::GrowBucket()
	{
		void* const memory = AllocateBucketMemory(nBucketSizeInBytes);
		BucketNode* const bucket = ::new (memory) BucketNode;
		bucket->pNext = pBucketList;
		pBucketList = bucket;

		ObjectNode* const first = FirstObject(bucket);
		ObjectNode* newFreeList = nullptr;
		ObjectNode* newFreeListTail = first;

		unsigned char* const firstObjectMemory =
			reinterpret_cast<unsigned char*>(first);
		for (size_t index = 0; index < nBucketSize; ++index)
		{
			ObjectNode* const node = ::new (
				firstObjectMemory + index * sizeof(ObjectNode)) ObjectNode;
			node->pNext = newFreeList;
			newFreeList = node;
		}

		newFreeListTail->pNext = pFreeList;
		pFreeList = newFreeList;

#ifdef CHECK_POOL_ALLOCATED_POINTERS
		legalPointers.push_back(PointerRange(
			first,
			reinterpret_cast<unsigned char*>(first) +
				nBucketSize * sizeof(ObjectNode)));
#endif

		return newFreeList;
	}

	template <size_t S, typename L, size_t A>
	void PoolAllocator<S, L, A>::FreeMemory()
	{
		typename Synchronization::CLock lock(synchronization);

		BucketNode* bucket = pBucketList;
		pBucketList = nullptr;
		pFreeList = nullptr;

		while (bucket != nullptr)
		{
			BucketNode* const next = bucket->pNext;
			bucket->~BucketNode();
			FreeBucketMemory(bucket);
			bucket = next;
		}

#ifdef CHECK_POOL_ALLOCATED_POINTERS
		legalPointers.clear();
#endif
	}

	template <size_t S>
	class PoolAllocatorNoMT
		: public PoolAllocator<S, PoolAllocatorSynchronizationSinglethreaded>
	{
	public:
		explicit PoolAllocatorNoMT(
			size_t bucketSize = PoolAllocator<S>::DefaultBucketSize)
			: PoolAllocator<S, PoolAllocatorSynchronizationSinglethreaded>(
				bucketSize)
		{
		}
	};

	template<class T>
	class TPoolAllocator
		: public PoolAllocator<
			sizeof(T),
			PoolAllocatorSynchronizationMultithreaded,
			alignof(T)>
	{
		using Base = PoolAllocator<
			sizeof(T),
			PoolAllocatorSynchronizationMultithreaded,
			alignof(T)>;

	public:
		explicit TPoolAllocator(size_t bucketSize = Base::DefaultBucketSize)
			: Base(bucketSize)
		{
		}

		void Delete(T* pointer)
		{
			if (pointer != nullptr)
			{
				pointer->~T();
				this->Deallocate(pointer);
			}
		}
	};
}

template<class T>
void* operator new(size_t size, stl::TPoolAllocator<T>& pool)
{
	assert(size == sizeof(T));
	return pool.Allocate();
}

template<class T>
void operator delete(void* pointer, stl::TPoolAllocator<T>& pool)
{
	pool.Deallocate(pointer);
}

#endif // __POOLALLOCATOR_H__
