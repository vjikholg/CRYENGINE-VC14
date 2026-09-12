#include "StdAfx.h"
#include "ByteStream.h"

#include <CryAllocatorFailure.h>

void CByteStreamPacker::GetCurrentOffset(size_t nextSize, size_t& nextOffset) noexcept
{
	nextOffset = nextSize;
}

bool CByteStreamPacker::TryGetSingleByteOffset(size_t& nextSize, size_t& nextOfs1, size_t& nextOffset) noexcept
{
	if (nextOfs1 % 4 == 0)
	{
		nextOfs1 = nextSize;
		if (nextSize > MaxStreamSize - 4)
			return false;
		nextSize += 4;
	}

	nextOffset = nextOfs1;
	if (nextOfs1 == MaxStreamSize)
		return false;
	++nextOfs1;
	return true;
}

bool CByteStreamPacker::TryGetTwoByteOffset(size_t& nextSize, size_t& nextOfs2, size_t& nextOffset) noexcept
{
	if (nextOfs2 % 4 == 0)
	{
		nextOfs2 = nextSize;
		if (nextSize > MaxStreamSize - 4)
			return false;
		nextSize += 4;
	}

	nextOffset = nextOfs2;
	if (nextOfs2 > MaxStreamSize - 2)
		return false;
	nextOfs2 += 2;
	return true;
}

bool CByteStreamPacker::TryGetFourByteOffset(size_t& nextSize, size_t& nextOffset) noexcept
{
	nextOffset = nextSize;
	if (nextSize > MaxStreamSize - 4)
		return false;
	nextSize += 4;
	return true;
}

bool CByteStreamPacker::TryGetAlignedOffset(size_t sz, size_t& nextSize, size_t& nextOffset) noexcept
{
	const size_t remainder = sz % 4;
	const size_t padding = remainder == 0 ? 0 : 4 - remainder;
	if (sz > MaxStreamSize - padding)
		return false;

	const size_t paddedSize = sz + padding;
	nextOffset = nextSize;
	if (nextSize > MaxStreamSize - paddedSize)
		return false;
	nextSize += paddedSize;
	return true;
}

bool CByteStreamPacker::TryGetNextOfs(size_t sz, uint32& offset) noexcept
{
	size_t nextSize = m_data[eD_Size];
	size_t nextOfs1 = m_data[eD_Ofs1];
	size_t nextOfs2 = m_data[eD_Ofs2];
	size_t nextOffset = 0;

	switch (sz)
	{
	case 0:
		GetCurrentOffset(nextSize, nextOffset);
		break;
	case 1:
		if (!TryGetSingleByteOffset(nextSize, nextOfs1, nextOffset))
			return false;
		break;
	case 2:
		if (!TryGetTwoByteOffset(nextSize, nextOfs2, nextOffset))
			return false;
		break;
	case 3:
	case 4:
		if (!TryGetFourByteOffset(nextSize, nextOffset))
			return false;
		break;
	default:
		if (!TryGetAlignedOffset(sz, nextSize, nextOffset))
			return false;
		break;
	}

	offset = static_cast<uint32>(nextOffset);
	m_data[eD_Size] = static_cast<uint32>(nextSize);
	m_data[eD_Ofs1] = static_cast<uint32>(nextOfs1);
	m_data[eD_Ofs2] = static_cast<uint32>(nextOfs2);
	return true;
}

uint32 CByteStreamPacker::GetNextOfs(size_t sz)
{
	uint32 offset = 0;
	if (TryGetNextOfs(sz, offset))
		return offset;

	CRY_ALLOCATOR_SIZE_FAILURE(
		"CByteStreamPacker::GetNextOfs",
		sz,
		1,
		MaxStreamSize,
		4,
		m_data[eD_Size],
		ECryAllocatorFailureAction::WriteDumpAndTerminate);
	return 0;
}

void CByteOutputStream::Grow( size_t sz )
{
	const size_t oldCapacity = m_capacity;
	size_t newCapacity = m_capacity == 0 ? 1 : m_capacity;
	while (newCapacity < sz)
	{
		if (newCapacity > MaxStreamSize / 2)
		{
			newCapacity = sz;
			break;
		}
		newCapacity *= 2;
	}

	void* const resizedBuffer = m_pSA->Realloc(m_buffer, newCapacity);
	if (resizedBuffer == 0)
	{
		CRY_ALLOCATOR_SIZE_FAILURE(
			"CByteOutputStream::Grow",
			newCapacity,
			1,
			MaxStreamSize,
			4,
			0,
			ECryAllocatorFailureAction::WriteDumpAndTerminate);
		return;
	}

	m_buffer = static_cast<uint8*>(resizedBuffer);
	m_capacity = newCapacity;
	memset(m_buffer + oldCapacity, 0, m_capacity - oldCapacity);
}
