#include "StdAfx.h"
#include "StringTable.h"
#include "ArithModel.h"
#include <limits>
#include <memory>


static constexpr uint32 LOG2_NUM_STRING_IDS = 16;
static constexpr uint32 NUM_STRING_IDS = 1 << LOG2_NUM_STRING_IDS;

CStringTable::CStringTable() : m_pRep(CStringTableRep::Create())
{
	string emptyStr;
	m_pRep->m_strToId[emptyStr] = 0;
	m_pRep->m_strings.push_back(emptyStr);
}

CStringTable::~CStringTable()
{
}

CStringTable::CStringTable( const CStringTable& other ) : m_pRep(other.m_pRep)
{
}

CStringTable& CStringTable::operator =( const CStringTable& other )
{
	m_pRep = other.m_pRep;
	return *this;
}

bool CStringTable::WriteString(CCommOutputStream& out, const string& s, CArithModel* pModel)
{
	auto existing = m_pRep->m_strToId.find(s);

	if (existing != m_pRep->m_strToId.end())
	{
		NET_ASSERT(existing->second < NUM_STRING_IDS);
		out.WriteBits(existing->second, LOG2_NUM_STRING_IDS);
		return true;
	}

	uint16 stringId;
	if (!TryGetStringID(m_pRep->m_strings.size(), stringId))
		return false;

	if (!pModel->CanWriteString(s))
		return false;

	// detach shared table if necessary
	if (m_pRep->IsUnique())
		m_pRep = CStringTableRep::Create(*m_pRep);
	NET_ASSERT(m_pRep->IsUnique());

	// write bits
	out.WriteBits(stringId, LOG2_NUM_STRING_IDS);
	const bool wroteString = pModel->WriteString(out, s);
	NET_ASSERT(wroteString);
	if (!wroteString)
		return false;

	// insert into dict
	m_pRep->m_strToId.insert(std::make_pair(s, stringId));
	m_pRep->m_strings.push_back(s);

	return true;
}


void CStringTable::ReadString( CCommInputStream& in, string& s, CArithModel * pModel )
{
	uint32 id = in.ReadBits(LOG2_NUM_STRING_IDS);
	if (id >= m_pRep->m_strings.size() || (id && m_pRep->m_strings[id].empty()))
	{
		if (m_pRep.use_count() != 1)
			m_pRep = CStringTableRep::Create(*m_pRep);
		NET_ASSERT(m_pRep->IsUnique());
		pModel->ReadString( in, s );
		m_pRep->m_strToId[s] = id;
		if (id >= m_pRep->m_strings.size())
			m_pRep->m_strings.resize(id+1);
		m_pRep->m_strings[id] = s;
	}
	else
	{
		s = m_pRep->m_strings[id];
	}
}

bool CStringTable::TryGetStringID(size_t sz, StringID& out) const
{
	if (sz > static_cast<size_t>((std::numeric_limits<StringID>::max)()))
	{
		NetWarning("String table exhausted");
		return false;
	}

	out = static_cast<StringID>(sz);
	return true;
}
