#include "StdAfx.h"
#include "IAnimationStateNode.h"

bool AG_FILE::SetFile(bool read, FILE* f)
{
	file = f;
	m_curpos = 0;
	m_readFailed = false;
	if (!read)
		return true;

	m_buffer.resize(0);
	ICryPak* pak = gEnv->pCryPak;
	const long cursor = pak->FTell(file);
	const size_t size = pak->FGetSize(file);
	if (cursor < 0 || size_t(cursor) > size || size > INT_MAX ||
	    pak->FSeek(file, 0, SEEK_SET) != 0)
	{
		m_readFailed = true;
		return false;
	}

	m_buffer.resize(static_cast<int>(size));
	if (size && pak->FReadRaw(m_buffer.begin(), 1, size, file) != size)
	{
		m_buffer.resize(0);
		m_readFailed = true;
		return false;
	}
	m_curpos = static_cast<size_t>(cursor);
	return true;
}

void FileSerializationHelper::Init(bool read, AG_FILE* f)
{
	reading = read;
	ag_file = f;
	file = f->file;
	pak = gEnv->pCryPak;
#ifdef NEED_ENDIAN_SWAP
	swappingEndian = true;
#else
	swappingEndian = false;
#endif
}

bool FileSerializationHelper::StringValue(string* s)
{
	if (!file)
		return false;
	if (!reading)
	{
		if (s->length() >= INT_MAX)
			return Fail();
		int size = 1 + s->length();
		pak->FWrite(&size, 1, file);
		pak->FWrite(s->c_str(), size, file);
		return true;
	}

	const size_t start = ag_file->m_curpos;
	int size = 0;
	if (!ReadFromBuffer(&size))
		return false;
	if (size <= 0 || size == INT_MAX || !CheckCount(size, 1))
	{
		ag_file->m_curpos = start;
		return Fail();
	}

	const char* source = reinterpret_cast<const char*>(ag_file->m_buffer.begin() + ag_file->m_curpos);
	if (memchr(source, '\0', size) != source + size - 1)
	{
		ag_file->m_curpos = start;
		return Fail();
	}

	ag_file->m_strBuffer.resize(size + 1);
	memcpy(ag_file->m_strBuffer.begin(), source, size);
	ag_file->m_strBuffer[size] = 0;
	s->assign(source, size - 1);
	ag_file->m_curpos += size;
	return true;
}
