#ifndef CRYCOMMON_STLHASHMAP_H
#define CRYCOMMON_STLHASHMAP_H

#include "StlContainerUtils.h"

#include <cctype>
#include <unordered_map>
#include <utility>

namespace stl
{
	template <class Key>
	class hash_simple
	{
	public:
		enum { bucket_size = 4, min_buckets = 8 };

		size_t operator()(const Key& key) const { return size_t(key); }
		bool LessThan(const Key& key1, const Key& key2) const { return key1 < key2; }
		bool Equals(const Key& key1, const Key& key2) const { return key1 == key2; }
	};

	class hash_uint32
	{
	public:
		enum { bucket_size = 4, min_buckets = 8 };

		ILINE size_t operator()(uint32 a) const
		{
			a = (a + 0x7ed55d16) + (a << 12);
			a = (a ^ 0xc761c23c) ^ (a >> 19);
			a = (a + 0x165667b1) + (a << 5);
			a = (a + 0xd3a2646c) ^ (a << 9);
			a = (a + 0xfd7046c5) + (a << 3);
			a = (a ^ 0xb55a4f09) ^ (a >> 16);
			return a;
		}

		ILINE bool LessThan(const uint32& key1, const uint32& key2) const { return key1 < key2; }
		ILINE bool Equals(const uint32& key1, const uint32& key2) const { return key1 == key2; }
	};

	template <class Key>
	class hash_strcmp
	{
	public:
		enum { bucket_size = 4, min_buckets = 8 };

		size_t operator()(const Key& key) const
		{
			unsigned int h = 0;
			const char* s = constchar_cast(key);
			for (; *s; ++s)
				h = 5 * h + *reinterpret_cast<const unsigned char*>(s);
			return size_t(h);
		}

		bool LessThan(const Key& key1, const Key& key2) const
		{
			return strcmp(constchar_cast(key1), constchar_cast(key2)) < 0;
		}

		bool Equals(const Key& key1, const Key& key2) const
		{
			return strcmp(constchar_cast(key1), constchar_cast(key2)) == 0;
		}
	};

	template <class Key>
	class hash_stricmp
	{
	public:
		enum { bucket_size = 4, min_buckets = 8 };

		size_t operator()(const Key& key) const
		{
			unsigned int h = 0;
			const char* s = constchar_cast(key);
			for (; *s; ++s)
				h = 5 * h + tolower(*reinterpret_cast<const unsigned char*>(s));
			return size_t(h);
		}

		bool LessThan(const Key& key1, const Key& key2) const
		{
			return stricmp(constchar_cast(key1), constchar_cast(key2)) < 0;
		}

		bool Equals(const Key& key1, const Key& key2) const
		{
			return stricmp(constchar_cast(key1), constchar_cast(key2)) == 0;
		}
	};

	template <class CryKey, class CryHashFunc>
	struct HashMapEquality
	{
		bool operator()(const CryKey& lhs, const CryKey& rhs) const
			noexcept(noexcept(CryHashFunc{}.Equals(lhs, rhs)))
		{
			return CryHashFunc{}.Equals(lhs, rhs);
		}
	};

	template <
		class Key,
		class Value,
		class HashFunc,
		class Alloc = std::allocator<std::pair<const Key, Value>>>
	using CryHashMap = std::unordered_map<Key, Value, HashFunc, HashMapEquality<Key, HashFunc>, Alloc>;
}

#endif // CRYCOMMON_STLHASHMAP_H
