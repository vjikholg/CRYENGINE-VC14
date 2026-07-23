// stl allocator wrapped around CMementoMemoryManager, and using the region system

#ifndef __STLMEMENTOALLOCATOR_H__
#define __STLMEMENTOALLOCATOR_H__

#pragma once

#include "MementoMemoryManager.h"

#include <type_traits>

template <class T> class STLMementoAllocator
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;
	typedef std::false_type propagate_on_container_copy_assignment;
	typedef std::false_type propagate_on_container_move_assignment;
	typedef std::false_type propagate_on_container_swap;
	typedef std::false_type is_always_equal;
	template <class U> struct rebind
	{
		typedef STLMementoAllocator<U> other;
	};

	STLMementoAllocator() noexcept : m_pMMM(&MMM())
	{
	}

	STLMementoAllocator( const CMementoMemoryManagerPtr& pMMM ) noexcept : m_pMMM(pMMM)
	{
	}

	STLMementoAllocator(const STLMementoAllocator& other) noexcept : m_pMMM(other.m_pMMM)
	{
	}

	template <class U> STLMementoAllocator(const STLMementoAllocator<U>& other) noexcept : m_pMMM(other.m_pMMM)
	{
	}

	~STLMementoAllocator() noexcept
	{
	}

	pointer address(reference x) const
	{
		return &x;
	}

	const_pointer address(const_reference x) const
	{
		return &x;
	}

	pointer allocate(size_type n, const void* = 0)
	{
		return static_cast<T*>(m_pMMM->AllocPtr(n*sizeof(T)));
	}

	void deallocate(pointer p, size_type n)
	{
		m_pMMM->FreePtr(p, n*sizeof(T));
	}

	size_type max_size() const noexcept
	{
		return INT_MAX;
	}

	void construct(pointer p, const T& val)
	{
		new(static_cast<void*>(p)) T(val);
	}

	void construct(pointer p)
	{
		new(static_cast<void*>(p)) T();
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	bool operator==(const STLMementoAllocator& other) const noexcept
	{
		return m_pMMM == other.m_pMMM;
	}

	bool operator!=(const STLMementoAllocator& other) const noexcept
	{
		return m_pMMM != other.m_pMMM;
	}

	template <class U>
	bool operator==(const STLMementoAllocator<U>& other) const noexcept
	{
		return m_pMMM == other.m_pMMM;
	}

	template <class U>
	bool operator!=(const STLMementoAllocator<U>& other) const noexcept
	{
		return m_pMMM != other.m_pMMM;
	}

	CMementoMemoryManagerPtr m_pMMM;
};

template <> class STLMementoAllocator<void>
{
public:
	typedef void* pointer;
	typedef const void* const_pointer;
	typedef void value_type;
	typedef std::false_type propagate_on_container_copy_assignment;
	typedef std::false_type propagate_on_container_move_assignment;
	typedef std::false_type propagate_on_container_swap;
	typedef std::false_type is_always_equal;
	template <class U>
	struct rebind { typedef STLMementoAllocator<U> other; };

	STLMementoAllocator() noexcept : m_pMMM(&MMM())
	{
	}

	STLMementoAllocator(const CMementoMemoryManagerPtr& pMMM) noexcept : m_pMMM(pMMM)
	{
	}

	STLMementoAllocator(const STLMementoAllocator& other) noexcept : m_pMMM(other.m_pMMM)
	{
	}

	template <class U>
	STLMementoAllocator(const STLMementoAllocator<U>& other) noexcept : m_pMMM(other.m_pMMM)
	{
	}

	bool operator==(const STLMementoAllocator& other) const noexcept
	{
		return m_pMMM == other.m_pMMM;
	}

	bool operator!=(const STLMementoAllocator& other) const noexcept
	{
		return m_pMMM != other.m_pMMM;
	}

	template <class U>
	bool operator==(const STLMementoAllocator<U>& other) const noexcept
	{
		return m_pMMM == other.m_pMMM;
	}

	template <class U>
	bool operator!=(const STLMementoAllocator<U>& other) const noexcept
	{
		return m_pMMM != other.m_pMMM;
	}

	CMementoMemoryManagerPtr m_pMMM;
};

#endif
