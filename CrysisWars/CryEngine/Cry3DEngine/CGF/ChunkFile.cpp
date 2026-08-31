////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2005.
// -------------------------------------------------------------------------
//  File name:   ChunkFile.h
//  Version:     v1.00
//  Created:     15/11/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ChunkFile.h"
#include <CryCheckedArithmetic.h>

#define MAX_CHUNKS_NUM 10000

inline bool ChunkLess( CChunkFile::ChunkDesc *d1,CChunkFile::ChunkDesc *d2 )
{
	return d1->hdr.FileOffset < d2->hdr.FileOffset;
}


CChunkFile::CChunkFile()
{
	m_fileHeader.FileType				= FileType_Geom;
	m_fileHeader.ChunkTableOffset		= -1;
	m_fileHeader.Version				= GeomFileVersion;
	strcpy(m_fileHeader.Signature,FILE_SIGNATURE);

	m_nLastChunkId = 0;
	m_pInternalData = NULL;
}

CChunkFile::~CChunkFile()
{
	ReleaseChunks();
	if (m_pInternalData)
		free(m_pInternalData);
}

// retrieves the raw chunk header, as it appears in the file
const CChunkFile::ChunkHeader& CChunkFile::GetChunkHeader(int nChunkIdx)const
{
	return m_chunks[nChunkIdx]->hdr;
}

//////////////////////////////////////////////////////////////////////////
CChunkFile::ChunkDesc* CChunkFile::GetChunk( int nIndex )
{
	assert( nIndex >= 0 && nIndex < (int)m_chunks.size() );
	return m_chunks[nIndex];
}

// returns the raw data of the i-th chunk
const void* CChunkFile::GetChunkData(int nChunkIdx)const
{
	assert (nChunkIdx >= 0 && nChunkIdx < NumChunks());
	if (nChunkIdx>= 0 && nChunkIdx < NumChunks())
	{
		return m_chunks[nChunkIdx]->data;
	}
	else
		return 0;
}

// number of chunks
int CChunkFile::NumChunks()const
{
	return (int)m_chunks.size();
}

// number of chunks of the specified type
int CChunkFile::NumChunksOfType (ChunkTypes nChunkType)const
{
	int nResult = 0;
	for (int i = 0; i < NumChunks(); ++i)
	{
		if (m_chunks[i]->hdr.ChunkType == nChunkType)
			++nResult;
	}
	return nResult;
}

//////////////////////////////////////////////////////////////////////////
// calculates the chunk size, based on the very next chunk with greater offset
// or the end of the raw data portion of the file
int CChunkFile::GetChunkSize(int nChunkIdx) const
{
	assert (nChunkIdx >= 0 && nChunkIdx < NumChunks());
	return m_chunks[nChunkIdx]->size;
}

//////////////////////////////////////////////////////////////////////////
int CChunkFile::AddChunk( const CHUNK_HEADER &hdr,void *chunkData,int chunkSize )
{
	ChunkDesc *chunk = new ChunkDesc;
	chunk->hdr = hdr;
	chunk->data = new char[chunkSize];
	chunk->size = chunkSize;
	memcpy( chunk->data,chunkData,chunkSize );

	int chunkID = ++m_nLastChunkId;
	chunk->hdr.ChunkID = chunkID;
	m_chunks.push_back( chunk );
	m_chunkIdMap[chunkID] = chunk;
	return chunkID;
}

//////////////////////////////////////////////////////////////////////////
void CChunkFile::SetChunkData( int nChunkId,void *chunkData,int chunkSize )
{
	ChunkDesc *pChunk = FindChunkById(nChunkId);
	delete [](char *)pChunk->data;
	pChunk->data = new char[chunkSize];
	pChunk->size = chunkSize;
	memcpy( pChunk->data,chunkData,chunkSize );
}

//////////////////////////////////////////////////////////////////////////
void CChunkFile::DeleteChunkId( int nChunkId )
{
	for (unsigned int i = 0; i < m_chunks.size(); i++)
	{
		if (m_chunks[i]->hdr.ChunkID == nChunkId) 
		{
			if (m_chunks[i]->data)
				delete [] (char *)m_chunks[i]->data;
			m_chunks.erase( m_chunks.begin()+i );
			return;
		}
	}
	m_chunkIdMap.erase(nChunkId);
}

//////////////////////////////////////////////////////////////////////////
void CChunkFile::ReleaseChunks()
{
	for (unsigned int i = 0; i < m_chunks.size(); i++)
	{
		if (m_chunks[i]->data)
			delete [](char *) m_chunks[i]->data;
		delete m_chunks[i];
	}
	m_chunks.clear();
	m_chunkIdMap.clear();
}

//////////////////////////////////////////////////////////////////////////
CChunkFile::ChunkDesc* CChunkFile::FindChunkByType( int type )
{
	for (unsigned int i = 0; i < m_chunks.size(); i++)
	{
		if (m_chunks[i]->hdr.ChunkType == type) 
		{
			return m_chunks[i];
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CChunkFile::ChunkDesc* CChunkFile::FindChunkById( int id )
{
	ChunkIdMap::iterator it = m_chunkIdMap.find(id);
	if (it != m_chunkIdMap.end())
	{
		return it->second;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CChunkFile::Write( const char *filename )
{
	FILE *file = fopen( filename,"wb" );
	if (!file)
	{
		m_LastError.Format( "File %s failed to open for writing",filename );
		return false;
	}

	unsigned int i;

	uint32 numChunks = 0;
	if (!CryTryNarrowSize(m_chunks.size(), numChunks))
	{
		m_LastError = "Too many chunks for the 32-bit CGF format";
		fclose(file);
		return false;
	}

	// Update chunk offsets.
	int nChunkTableOffset = sizeof(m_fileHeader);
	size_t chunkHeaderBytes = 0;
	size_t chunksOffsetSize = 0;
	int nChunksOffset = 0;
	if (!CryTryMultiplySize(numChunks, sizeof(CHUNK_HEADER), chunkHeaderBytes) ||
		!CryTryAddSize(nChunkTableOffset + sizeof(numChunks), chunkHeaderBytes, chunksOffsetSize) ||
		!CryTryNarrowSize(chunksOffsetSize, nChunksOffset))
	{
		m_LastError = "CGF chunk table exceeds the 32-bit file format";
		fclose(file);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Update offsets of chunks.
	//////////////////////////////////////////////////////////////////////////
	size_t nCurrOffset = 0;
	for (i = 0; i < m_chunks.size(); i++)
	{
		size_t fileOffset = 0;
		if (m_chunks[i]->size < 0 ||
			!CryTryAddSize(chunksOffsetSize, nCurrOffset, fileOffset) ||
			!CryTryNarrowSize(fileOffset, m_chunks[i]->hdr.FileOffset) ||
			!CryTryAddSize(nCurrOffset, static_cast<size_t>(m_chunks[i]->size), nCurrOffset))
		{
			m_LastError = "CGF chunk data exceeds the 32-bit file format";
			fclose(file);
			return false;
		}
	}

	//=======================
	//Write File Header.
	//=======================
	m_fileHeader.ChunkTableOffset = nChunkTableOffset;
	if (fwrite(&m_fileHeader,sizeof(m_fileHeader),1,file) != 1)
	{
		fclose(file);
		return false;
	}

	//=======================
	//Write Number of Chunks
	//=======================
	assert( ftell(file) == nChunkTableOffset );
	if (fwrite(&numChunks,sizeof(numChunks),1,file) != 1)
	{
		fclose(file);
		return false;
	}

	//=======================
	//Write Chunk List
	//=======================
	for(i = 0; i < numChunks; i++)
	{
		CHUNK_HEADER &hdr = m_chunks[i]->hdr;
		if (fwrite(&hdr,sizeof(hdr),1,file)!=1)
		{
			fclose(file);
			return false;
		}
	}

	//=======================
	// Write Chunks.
	//=======================
	assert( ftell(file) == nChunksOffset );
	for (i = 0; i < m_chunks.size(); i++)
	{
		ChunkDesc &ch = *m_chunks[i];

		// Copy header.
		switch (ch.hdr.ChunkType)
		{
			// Hack for incompatible chunk types
		case ChunkType_Controller:
			if (ch.hdr.ChunkVersion == CONTROLLER_CHUNK_DESC_0827::VERSION ||
					ch.hdr.ChunkVersion == CONTROLLER_BSPLINE_DATA_0826::VERSION)
				break;
		case ChunkType_BoneNameList:
			if (ch.hdr.ChunkVersion == BONENAMELIST_CHUNK_DESC_0745::VERSION)
				break;
		case ChunkType_MeshMorphTarget:
			if (ch.hdr.ChunkVersion == MESHMORPHTARGET_CHUNK_DESC_0001::VERSION)
				break;
		case ChunkType_BoneInitialPos:
			if (ch.hdr.ChunkVersion == BONEINITIALPOS_CHUNK_DESC_0001::VERSION)
				break;
			default:
				memcpy( ch.data,&ch.hdr,sizeof(ch.hdr) );
		}

		// write data.
		if (fwrite(ch.data,ch.size,1,file) != 1)
		{
			fclose(file);
			return false;
		}
	}

	fclose(file);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CChunkFile::WriteToMemory( void **pData,int *nSize )
{
	*pData = nullptr;
	*nSize = 0;
	size_t i = 0;
	
	// Update chunk offsets.
	uint32 numChunks = 0;
	if (!CryTryNarrowSize(m_chunks.size(), numChunks))
		return;
	const size_t nChunkTableOffset = sizeof(m_fileHeader);
	size_t chunkHeaderBytes = 0;
	size_t nChunksOffset = 0;
	if (!CryTryMultiplySize(numChunks, sizeof(CHUNK_HEADER), chunkHeaderBytes) ||
		!CryTryAddSize(nChunkTableOffset + sizeof(numChunks), chunkHeaderBytes, nChunksOffset))
	{
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Update offsets of chunks.
	//////////////////////////////////////////////////////////////////////////
	size_t nCurrOffset = 0;
	for (i = 0; i < numChunks; i++)
	{
		ChunkDesc &ch = *(m_chunks[i]);
		size_t fileOffset = 0;
		if (ch.size < 0 ||
			!CryTryAddSize(nChunksOffset, nCurrOffset, fileOffset) ||
			!CryTryNarrowSize(fileOffset, ch.hdr.FileOffset) ||
			!CryTryAddSize(nCurrOffset, static_cast<size_t>(ch.size), nCurrOffset))
		{
			return;
		}
	}
	
	size_t size = 0;
	int outputSize = 0;
	int chunkTableOffset32 = 0;
	if (!CryTryAddSize(nChunksOffset, nCurrOffset, size) ||
		!CryTryNarrowSize(size, outputSize) ||
		!CryTryNarrowSize(nChunkTableOffset, chunkTableOffset32))
	{
		return;
	}
	if (m_pInternalData)
		free(m_pInternalData);
	m_pInternalData = (char*)malloc( size );
	if (!m_pInternalData)
		return;

	char *pBuf = m_pInternalData;
	size_t ofs = nChunkTableOffset;

	//////////////////////////////////////////////////////////////////////////
	// Write File Header.
	//////////////////////////////////////////////////////////////////////////
	m_fileHeader.ChunkTableOffset = chunkTableOffset32;
	memcpy( pBuf,&m_fileHeader,sizeof(m_fileHeader) );

	//////////////////////////////////////////////////////////////////////////
	// Write Number of Chunks
	//////////////////////////////////////////////////////////////////////////
	memcpy( &pBuf[ofs],&numChunks,sizeof(numChunks) );
	ofs += sizeof(numChunks);

	//////////////////////////////////////////////////////////////////////////
	// Write Chunk List
	//////////////////////////////////////////////////////////////////////////
	for(i = 0; i < numChunks;i++)
	{
		CHUNK_HEADER &hdr = m_chunks[i]->hdr;
		memcpy( &pBuf[ofs],&hdr,sizeof(hdr) );
		ofs += sizeof(hdr);
	}

	//////////////////////////////////////////////////////////////////////////
	// Write Chunks.
	//////////////////////////////////////////////////////////////////////////
	for(i = 0; i < numChunks; i++)
	{
		// write header.
		ChunkDesc &ch = *(m_chunks[i]);

		switch (ch.hdr.ChunkType)
		{
			// Hack for incompatible chunk types
		case ChunkType_Controller:
			if (ch.hdr.ChunkVersion == CONTROLLER_CHUNK_DESC_0827::VERSION ||
				ch.hdr.ChunkVersion == CONTROLLER_BSPLINE_DATA_0826::VERSION)
				break;
		case ChunkType_BoneNameList:
			if (ch.hdr.ChunkVersion == BONENAMELIST_CHUNK_DESC_0745::VERSION)
				break;
		case ChunkType_MeshMorphTarget:
			if (ch.hdr.ChunkVersion == MESHMORPHTARGET_CHUNK_DESC_0001::VERSION)
				break;
		case ChunkType_BoneInitialPos:
			if (ch.hdr.ChunkVersion == BONEINITIALPOS_CHUNK_DESC_0001::VERSION)
				break;
		default:
			memcpy( ch.data,&ch.hdr,sizeof(ch.hdr) );
		}

		// write data.
		memcpy( &pBuf[ch.hdr.FileOffset],ch.data,ch.size );
	}

	*pData = m_pInternalData;
	*nSize = outputSize;
}

//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadChunkTable( CCryFile &file,int nChunkTableOffset )
{
	size_t res = 0;
	size_t i = 0;

	ReleaseChunks();
	if (nChunkTableOffset < 0)
		return false;

	size_t nEndOfChunks = static_cast<size_t>(nChunkTableOffset);
	if (nChunkTableOffset == sizeof(m_fileHeader))
	{
		nEndOfChunks = file.GetLength();
	}
	
	file.Seek(nChunkTableOffset,SEEK_SET);
  
	int n_chunks;
	res = file.ReadType(&n_chunks);
	if(res!=sizeof(n_chunks) || n_chunks < 0 || n_chunks > MAX_CHUNKS_NUM)
  {
    return false;
  }

	m_nLastChunkId = 0;

	std::vector<ChunkDesc*> m_sortedChunks;

	CHUNK_HEADER *chunks = new CHUNK_HEADER[n_chunks];
	assert( chunks );
  res = file.ReadType( chunks,n_chunks );
	if(res != sizeof(CHUNK_HEADER)*n_chunks)
  {
		delete []chunks;
    return false;
  }

	m_chunks.clear();
	m_chunks.resize(n_chunks);
	m_sortedChunks.resize(n_chunks);

	for (i = 0; i < m_chunks.size(); i++)
	{
		m_chunks[i] = new ChunkDesc;
		m_chunks[i]->hdr = chunks[i];
		m_sortedChunks[i] = m_chunks[i];

		// Add chunk to chunkid map.
		m_chunkIdMap[m_chunks[i]->hdr.ChunkID] = m_chunks[i];
		// Find last chunk id.
		if (m_chunks[i]->hdr.ChunkID > m_nLastChunkId)
			m_nLastChunkId = m_chunks[i]->hdr.ChunkID;
	}

	std::sort( m_sortedChunks.begin(),m_sortedChunks.end(),ChunkLess );
		

	for (i = 0; i < m_sortedChunks.size(); i++)
	{
		size_t nextFileOffset = 0;
		if (i+1 < m_sortedChunks.size())
		{
			if (m_sortedChunks[i+1]->hdr.FileOffset < 0)
				return false;
			nextFileOffset = static_cast<size_t>(m_sortedChunks[i+1]->hdr.FileOffset);
		}
		else
			nextFileOffset = nEndOfChunks;

		ChunkDesc &cd = *m_sortedChunks[i];
		if (cd.hdr.FileOffset < 0)
			return false;
		const size_t fileOffset = static_cast<size_t>(cd.hdr.FileOffset);
		if (nextFileOffset < fileOffset || nextFileOffset > file.GetLength())
			return false;
		const size_t chunkSize = nextFileOffset - fileOffset;
		if (!CryTryNarrowSize(chunkSize, cd.size))
			return false;

		cd.data = new char[chunkSize];
		file.Seek( cd.hdr.FileOffset,SEEK_SET );

		const size_t readres = file.ReadRaw( cd.data,chunkSize );
		if (readres != chunkSize)
		{
			return false;
		}
	}

	delete []chunks;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CChunkFile::Read( const char *filename )
{
	CCryFile file;
	if (!file.Open(filename,"rb"))
	{
		m_LastError.Format( "File %s failed to open for reading",filename );
		return false;
	}
 
	if (!file.ReadType( &m_fileHeader))//,sizeof(m_fileHeader) ) != sizeof(m_fileHeader))
	{
		return false;
	}

	if (!ReadChunkTable( file,m_fileHeader.ChunkTableOffset ))
	{
		return false;
	}

	return true;
}
