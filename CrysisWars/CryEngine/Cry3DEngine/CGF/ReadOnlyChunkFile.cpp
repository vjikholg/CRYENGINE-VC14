////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2005.
// -------------------------------------------------------------------------
//  File name:   ReadOnlyChunkFile.h
//  Version:     v1.00
//  Created:     15/11/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ReadOnlyChunkFile.h"
#include <CryCheckedArithmetic.h>

#define MAX_CHUNKS_NUM 10000

inline bool ChunkCompareFileOffset( const CReadOnlyChunkFile::ChunkDesc &d1,const CReadOnlyChunkFile::ChunkDesc &d2 )
{
	return d1.hdr.FileOffset < d2.hdr.FileOffset;
}
inline bool ChunkCompareId( const CReadOnlyChunkFile::ChunkDesc &d1,const CReadOnlyChunkFile::ChunkDesc &d2 )
{
	return d1.hdr.ChunkID < d2.hdr.ChunkID;
}

//////////////////////////////////////////////////////////////////////////
// ChunkDesc compare by ChunkId operators.
//////////////////////////////////////////////////////////////////////////
inline bool operator<( const CReadOnlyChunkFile::ChunkDesc &d1,const CReadOnlyChunkFile::ChunkDesc &d2 ) { return d1.hdr.ChunkID < d2.hdr.ChunkID; }
inline bool operator>( const CReadOnlyChunkFile::ChunkDesc &d1,const CReadOnlyChunkFile::ChunkDesc &d2 ) { return d1.hdr.ChunkID > d2.hdr.ChunkID; }
inline bool operator==( const CReadOnlyChunkFile::ChunkDesc &d1,const CReadOnlyChunkFile::ChunkDesc &d2 ) { return d1.hdr.ChunkID == d2.hdr.ChunkID; }
inline bool operator!=( const CReadOnlyChunkFile::ChunkDesc &d1,const CReadOnlyChunkFile::ChunkDesc &d2 ) { return d1.hdr.ChunkID != d2.hdr.ChunkID; }
//////////////////////////////////////////////////////////////////////////


CReadOnlyChunkFile::CReadOnlyChunkFile( bool bNoWarningMode )
{
	m_pFileBuffer = 0;
	m_fileHeader.FileType				= FileType_Geom;
	m_fileHeader.ChunkTableOffset		= -1;
	m_fileHeader.Version				= GeomFileVersion;
	strcpy(m_fileHeader.Signature,FILE_SIGNATURE);

	m_bNoWarningMode = bNoWarningMode;

	m_nLastChunkId = 0;
	m_bOwnFileBuffer = false;

	m_hFile = 0;
}

CReadOnlyChunkFile::~CReadOnlyChunkFile()
{
	FreeBuffer();

	if (m_hFile)
		gEnv->pCryPak->FClose(m_hFile);
	m_hFile = 0;
}

//////////////////////////////////////////////////////////////////////////
void CReadOnlyChunkFile::FreeBuffer()
{
	if (m_pFileBuffer && m_bOwnFileBuffer)
		delete []m_pFileBuffer;
	m_pFileBuffer = 0;
	m_bOwnFileBuffer = 0;
}

// retrieves the raw chunk header, as it appears in the file
const CReadOnlyChunkFile::ChunkHeader& CReadOnlyChunkFile::GetChunkHeader(int nChunkIdx)const
{
	return m_chunks[nChunkIdx].hdr;
}

//////////////////////////////////////////////////////////////////////////
CReadOnlyChunkFile::ChunkDesc* CReadOnlyChunkFile::GetChunk( int nIndex )
{
	assert( nIndex >= 0 && nIndex < (int)m_chunks.size() );
	return &m_chunks[nIndex];
}

// returns the raw data of the i-th chunk
const void* CReadOnlyChunkFile::GetChunkData(int nChunkIdx)const
{
	assert (nChunkIdx >= 0 && nChunkIdx < NumChunks());
	if (nChunkIdx>= 0 && nChunkIdx < NumChunks())
	{
		return m_chunks[nChunkIdx].data;
	}
	else
		return 0;
}

// number of chunks
int CReadOnlyChunkFile::NumChunks()const
{
	return (int)m_chunks.size();
}

// number of chunks of the specified type
int CReadOnlyChunkFile::NumChunksOfType (ChunkTypes nChunkType)const
{
	int nResult = 0;
	for (int i = 0; i < NumChunks(); ++i)
	{
		if (m_chunks[i].hdr.ChunkType == nChunkType)
			++nResult;
	}
	return nResult;
}

//////////////////////////////////////////////////////////////////////////
// calculates the chunk size, based on the very next chunk with greater offset
// or the end of the raw data portion of the file
int CReadOnlyChunkFile::GetChunkSize(int nChunkIdx) const
{
	assert (nChunkIdx >= 0 && nChunkIdx < NumChunks());
	return m_chunks[nChunkIdx].size;
}

//////////////////////////////////////////////////////////////////////////
CReadOnlyChunkFile::ChunkDesc* CReadOnlyChunkFile::FindChunkByType( int type )
{
	const size_t nchunks = m_chunks.size();
	if (nchunks > 0)
	{
		ChunkDesc *chunks = &m_chunks[0];
		for (size_t i = 0; i < nchunks; i++)
		{
			if (chunks[i].hdr.ChunkType == type)
			{
				return &chunks[i];
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CReadOnlyChunkFile::ChunkDesc* CReadOnlyChunkFile::FindChunkById( int id )
{
	ChunkDesc chunkToFind;
	chunkToFind.hdr.ChunkID = id;
	std::vector<ChunkDesc>::iterator it = stl::binary_find( m_chunks.begin(),m_chunks.end(),chunkToFind );
	if (it != m_chunks.end())
	{
		ChunkDesc* pChunk = &*it;
		return pChunk;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CReadOnlyChunkFile::ReadChunkTableFromBuffer( void *pBuffer,size_t nBufferSize )
{
	FreeBuffer();
	m_chunks.clear();
	if (pBuffer == nullptr || !CryIsRangeWithinSize(nBufferSize, 0, sizeof(m_fileHeader)))
		return false;

	m_pFileBuffer = static_cast<char*>(pBuffer);
	m_bOwnFileBuffer = false;

	memcpy(&m_fileHeader, m_pFileBuffer, sizeof(m_fileHeader));
	SwapEndian(m_fileHeader);
	if (m_fileHeader.ChunkTableOffset < 0)
		return false;

	const size_t chunkTableOffset = static_cast<size_t>(m_fileHeader.ChunkTableOffset);
	if (!CryIsRangeWithinSize(nBufferSize, chunkTableOffset, sizeof(int)))
		return false;

	int chunkCount = 0;
	memcpy(&chunkCount, m_pFileBuffer + chunkTableOffset, sizeof(chunkCount));
	SwapEndian(chunkCount);
	if (chunkCount < 0 || chunkCount > MAX_CHUNKS_NUM)
		return false;

	const size_t chunkCountSize = static_cast<size_t>(chunkCount);
	size_t chunkHeadersOffset = 0;
	if (!CryTryAddSize(chunkTableOffset, sizeof(chunkCount), chunkHeadersOffset))
		return false;

	size_t chunkHeadersEnd = 0;
	if (!CryTryGetArrayRangeEnd(
		nBufferSize,
		chunkHeadersOffset,
		chunkCountSize,
		sizeof(CHUNK_HEADER),
		chunkHeadersEnd))
	{
		return false;
	}
	const bool tablePrecedesChunks = chunkTableOffset == sizeof(m_fileHeader);
	const size_t chunkDataBegin = tablePrecedesChunks
		? chunkHeadersEnd
		: sizeof(m_fileHeader);
	const size_t chunkDataEnd = tablePrecedesChunks
		? nBufferSize
		: chunkTableOffset;

	m_nLastChunkId = 0;
	m_chunks.resize(chunkCountSize);
	for (size_t i = 0; i < chunkCountSize; ++i)
	{
		ChunkDesc& chunk = m_chunks[i];
		memcpy(
			&chunk.hdr,
			m_pFileBuffer + chunkHeadersOffset + i * sizeof(CHUNK_HEADER),
			sizeof(CHUNK_HEADER));
		SwapEndian(chunk.hdr);
		if (chunk.hdr.FileOffset < 0 || chunk.hdr.ChunkID < 0)
			return false;
		if (chunk.hdr.ChunkID > m_nLastChunkId)
			m_nLastChunkId = chunk.hdr.ChunkID;
	}

	std::sort(m_chunks.begin(), m_chunks.end(), ChunkCompareFileOffset);
	for (size_t i = 0; i < chunkCountSize; ++i)
	{
		ChunkDesc& chunk = m_chunks[i];
		const size_t chunkOffset = static_cast<size_t>(chunk.hdr.FileOffset);
		const size_t nextOffset = i + 1 < chunkCountSize
			? static_cast<size_t>(m_chunks[i + 1].hdr.FileOffset)
			: chunkDataEnd;
		if (chunkOffset < chunkDataBegin ||
			nextOffset < chunkOffset ||
			nextOffset > chunkDataEnd)
			return false;

		const size_t chunkSize = nextOffset - chunkOffset;
		if (!CryTryNarrowSize(chunkSize, chunk.size))
			return false;
		chunk.data = m_pFileBuffer + chunkOffset;
	}

	std::sort(m_chunks.begin(), m_chunks.end(), ChunkCompareId);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CReadOnlyChunkFile::Read( const char *filename )
{
	if (m_hFile)
		gEnv->pCryPak->FClose(m_hFile);
	m_hFile = 0;
	FreeBuffer();

	int nOpenFlags = 0;
	if (m_bNoWarningMode)
		nOpenFlags |= ICryPak::FOPEN_HINT_QUIET;

	m_hFile = gEnv->pCryPak->FOpen(filename,"rb",nOpenFlags );
	if (!m_hFile)
		return false;

	size_t nFileSize = 0;
	m_pFileBuffer = (char*)gEnv->pCryPak->FGetCachedFileData(m_hFile,nFileSize);
	m_bOwnFileBuffer = false;
	if (!m_pFileBuffer)
		return false;

	if (!ReadChunkTableFromBuffer( m_pFileBuffer,nFileSize ))
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CReadOnlyChunkFile::ReadFromMemBlock( CMemoryBlock * pMemBlock )
{
  if (!ReadChunkTableFromBuffer( pMemBlock->GetData(),pMemBlock->GetSize() ))
  {
    return false;
  }
  return true;
}
