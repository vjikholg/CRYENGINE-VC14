#ifndef __BYTESTREAM_H__
#define __BYTESTREAM_H__

#pragma once

#include "MiniQueue.h"
#include "Typelist.h"

#include <limits>

class CByteStreamPacker
{
public:
	static constexpr size_t MaxStreamSize =
		static_cast<size_t>((std::numeric_limits<uint32>::max)());

	CByteStreamPacker()
	{
		for (int i=0; i<3; i++)
			m_data[i] = 0;
	}

	bool TryGetNextOfs(size_t sz, uint32& offset) noexcept;
	uint32 GetNextOfs(size_t sz);

	template <int N>
	ILINE uint32 GetNextOfs_Fixed( NTypelist::Int2Type<N> )
	{
		return GetNextOfs(N);
	}

protected:
	static void GetCurrentOffset(size_t nextSize, size_t& nextOffset) noexcept;
	static bool TryGetSingleByteOffset(size_t& nextSize, size_t& nextOfs1, size_t& nextOffset) noexcept;
	static bool TryGetTwoByteOffset(size_t& nextSize, size_t& nextOfs2, size_t& nextOffset) noexcept;
	static bool TryGetFourByteOffset(size_t& nextSize, size_t& nextOffset) noexcept;
	static bool TryGetAlignedOffset(size_t sz, size_t& nextSize, size_t& nextOffset) noexcept;

	enum EData
	{
		eD_Size = 0,
		eD_Ofs1,
		eD_Ofs2,
	};
	uint32 m_data[3];
	/*
	uint32 m_size;
	uint32 m_ofs1;
	uint32 m_ofs2;
	*/
};

class CByteOutputStream : private CByteStreamPacker
{
public:
	CByteOutputStream( IStreamAllocator* pSA, size_t initSize = 16ull, void* caller = 0 ) :
		m_pSA(pSA),
		m_capacity(initSize),
		m_buffer((uint8*)pSA->Alloc(initSize, caller? caller : UP_STACK_PTR))
	{
		memset(m_buffer, 0, initSize);
	}

	void Put( const void * pWhat, size_t sz )
	{
		uint32 where = GetNextOfs(sz);
		if (m_data[eD_Size] > m_capacity)
			Grow(m_data[eD_Size]);
		memcpy( m_buffer+where, pWhat, sz );
	}

	template <class T>
	ILINE T& PutTyped()
	{
		uint32 where = GetNextOfs_Fixed(NTypelist::Int2Type<sizeof(T)>());
		if (m_data[eD_Size] > m_capacity)
			Grow(m_data[eD_Size]);
		return *reinterpret_cast<T*>(m_buffer+where);
	}

	ILINE void PutByte( uint8 c )
	{
		PutTyped<uint8>() = c;
	}

	ILINE size_t GetSize()
	{
		return m_data[eD_Size];
	}

	const uint8* GetBuffer() const
	{
		return m_buffer;
	}

	void GetMemoryStatistics(ICrySizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CByteOutputStream");

		pSizer->Add(*this);
		if (m_pSA)
			pSizer->Add(m_buffer, m_capacity);
	}

private:
	void Grow( size_t sz );

	IStreamAllocator* m_pSA;
	size_t m_capacity;
	uint8* m_buffer;
};

class CByteInputStream : private CByteStreamPacker
{
public:
	CByteInputStream( const uint8* pData, size_t sz ) : m_pData(pData), m_capacity(sz) 
	{
	}

	~CByteInputStream()
	{
	}

	ILINE void Get( void* pWhere, size_t sz )
	{
		NET_ASSERT(sz <= m_data[eD_Size]);
		memcpy(pWhere, Get(sz), sz);
	}

	ILINE const void* Get( size_t sz )
	{
		uint32 where = GetNextOfs(sz);
		NET_ASSERT(where <= m_capacity && sz <= m_capacity - where);
		return m_pData + where;
	}

	template <class T>
	ILINE const T& GetTyped()
	{
		uint32 where = GetNextOfs_Fixed(NTypelist::Int2Type<sizeof(T)>());
		NET_ASSERT(where <= m_capacity && sizeof(T) <= m_capacity - where);
		return *static_cast<const T*>(static_cast<const void*>(m_pData + where));
	}

	ILINE uint8 GetByte()
	{
		return GetTyped<uint8>();
	}

	void GetMemoryStatistics(ICrySizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CByteInputStream");

		pSizer->Add(*this);
		pSizer->Add(m_pData, m_capacity);
	}

	size_t GetSize() const
	{
		return m_data[eD_Size];
	}

private:
	const uint8* m_pData;
	size_t m_capacity;
};

#endif
