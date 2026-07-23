#ifndef CRYCOMMON_STLCONTAINERUTILS_H
#define CRYCOMMON_STLCONTAINERUTILS_H

#include "platform.h"

#include <cstring>
#include <vector>

template <class T>
class CAutoClear
{
public:
	CAutoClear(T* p) : m_p(p) {}
	~CAutoClear() { m_p->clear(); }

protected:
	T* m_p;
};

template <class Container>
unsigned sizeofArray(const Container& arr)
{
	return static_cast<unsigned>(sizeof(typename Container::value_type) * arr.size());
}

template <class Container>
unsigned sizeofVector(const Container& arr)
{
	return static_cast<unsigned>(sizeof(typename Container::value_type) * arr.capacity());
}

template <class Container>
unsigned sizeofArray(const Container& arr, unsigned nSize)
{
	return arr.empty() ? 0u : static_cast<unsigned>(sizeof(typename Container::value_type) * nSize);
}

template <class Container>
unsigned capacityofArray(const Container& arr)
{
	return static_cast<unsigned>(arr.capacity() * sizeof(arr[0]));
}

template <class T>
unsigned countElements(const std::vector<T>& arrT, const T& x)
{
	unsigned nSum = 0;
	for (typename std::vector<T>::const_iterator iter = arrT.begin(); iter != arrT.end(); ++iter)
	{
		if (x == *iter)
			++nSum;
	}
	return nSum;
}

namespace stl
{
	struct container_object_deleter
	{
		template <typename T>
		void operator()(const T* ptr) const
		{
			delete ptr;
		}
	};

	template <class Type>
	inline const char* constchar_cast(const Type& type)
	{
		return type;
	}

	template <>
	inline const char* constchar_cast(const string& type)
	{
		return type.c_str();
	}

	template <class Type>
	struct less_strcmp
	{
		bool operator()(const Type& left, const Type& right) const
		{
			return strcmp(constchar_cast(left), constchar_cast(right)) < 0;
		}
	};

	template <class Type>
	struct less_stricmp
	{
		bool operator()(const Type& left, const Type& right) const
		{
			return stricmp(constchar_cast(left), constchar_cast(right)) < 0;
		}
	};
}

#endif // CRYCOMMON_STLCONTAINERUTILS_H
