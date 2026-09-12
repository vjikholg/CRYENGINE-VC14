#ifndef __STRINGTABLE_H__
#define __STRINGTABLE_H__

#pragma once

#include "Streams/CommStream.h"

#include "StlHashMap.h"
#include <memory>

class CArithModel;

class CStringTableRep;
using CStringTableRepPtr = std::shared_ptr<CStringTableRep>;


class CStringTable
{
public:
	using StringID = uint16;

	CStringTable();
	~CStringTable();
	CStringTable( const CStringTable& );
	CStringTable& operator=( const CStringTable& );

	bool WriteString( CCommOutputStream& out, const string& s, CArithModel * pModel );
	void ReadString( CCommInputStream& in, string& s, CArithModel * pModel );
	[[nodiscard]] bool TryGetStringID(size_t sz, StringID& out) const;

private:
	CStringTableRepPtr m_pRep;
};

class CStringTableRep : public std::enable_shared_from_this<CStringTableRep>
{
public:
	inline static CStringTableRepPtr Create() { return std::make_shared<CStringTableRep>(); }
	inline static CStringTableRepPtr Create(const CStringTableRep& r) { return std::make_shared<CStringTableRep>(r); }

	inline bool IsUnique() const noexcept
	{
		return weak_from_this().use_count() == 1;
	}

	using StrToIdMap = stl::CryHashMap<string, CStringTable::StringID, stl::hash_strcmp<string> >;
	typedef std::vector<string> StrVec;
	StrToIdMap m_strToId;
	StrVec m_strings;

	CStringTableRep() {}
	CStringTableRep(const CStringTableRep& r) : m_strToId(r.m_strToId), m_strings(r.m_strings) {}
	~CStringTableRep() {}
};

#endif
