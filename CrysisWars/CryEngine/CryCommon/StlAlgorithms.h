#ifndef CRYCOMMON_STLALGORITHMS_H
#define CRYCOMMON_STLALGORITHMS_H

#include <algorithm>

namespace stl
{
	template <typename Map>
	inline typename Map::mapped_type find_in_map(const Map& mapKeyToValue, const typename Map::key_type& key, typename Map::mapped_type valueDefault)
	{
		typename Map::const_iterator it = mapKeyToValue.find(key);
		return it == mapKeyToValue.end() ? valueDefault : it->second;
	}

	template <typename Map>
	inline typename Map::mapped_type& find_in_map_ref(Map& mapKeyToValue, const typename Map::key_type& key, typename Map::mapped_type& valueDefault)
	{
		typename Map::iterator it = mapKeyToValue.find(key);
		return it == mapKeyToValue.end() ? valueDefault : it->second;
	}

	template <class Map, class Vector>
	inline void map_to_vector(const Map& theMap, Vector& array)
	{
		array.resize(0);
		array.reserve(theMap.size());
		for (typename Map::const_iterator it = theMap.begin(); it != theMap.end(); ++it)
			array.push_back(it->second);
	}

	template <class Set, class Vector>
	inline void set_to_vector(const Set& theSet, Vector& array)
	{
		array.resize(0);
		array.reserve(theSet.size());
		for (typename Set::const_iterator it = theSet.begin(); it != theSet.end(); ++it)
			array.push_back(*it);
	}

	template <class Container, class Value>
	inline bool find_and_erase(Container& container, const Value& value)
	{
		typename Container::iterator it = std::find(container.begin(), container.end(), value);
		if (it == container.end())
			return false;
		container.erase(it);
		return true;
	}

	template <class Container, class Value>
	inline bool push_back_unique(Container& container, const Value& value)
	{
		if (std::find(container.begin(), container.end(), value) != container.end())
			return false;
		container.push_back(value);
		return true;
	}

	template <class Container, class Iter>
	inline void push_back_range(Container& container, Iter begin, Iter end)
	{
		for (Iter it = begin; it != end; ++it)
			container.push_back(*it);
	}

	template <class Container, class Value>
	inline bool find(Container& container, const Value& value)
	{
		return std::find(container.begin(), container.end(), value) != container.end();
	}

	template <class Iterator, class T>
	inline Iterator binary_find(Iterator first, Iterator last, const T& value)
	{
		Iterator it = std::lower_bound(first, last, value);
		return it == last || value != *it ? last : it;
	}

	template <class Container, class Value>
	inline bool binary_insert_unique(Container& container, const Value& value)
	{
		typename Container::iterator it = std::lower_bound(container.begin(), container.end(), value);
		if (it != container.end())
		{
			if (*it == value)
				return false;
			container.insert(it, value);
		}
		else
		{
			container.insert(container.end(), value);
		}
		return true;
	}

	template <class Container, class Value>
	inline bool binary_erase(Container& container, const Value& value)
	{
		typename Container::iterator it = std::lower_bound(container.begin(), container.end(), value);
		if (it == container.end() || *it != value)
			return false;
		container.erase(it);
		return true;
	}
}

#endif // CRYCOMMON_STLALGORITHMS_H
