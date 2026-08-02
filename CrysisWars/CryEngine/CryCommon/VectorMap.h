////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2006.
// -------------------------------------------------------------------------
//  File name:   VectorMap.h
//  Version:     v1.00
//  Created:     20/7/2006 by MichaelS.
//  Compilers:   Visual Studio.NET 2005
//  Description: std::map replacement implemented using sorted vector.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __SORTEDVECTOR_H__
#define __SORTEDVECTOR_H__
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

//--------------------------------------------------------------------------
// VectorMap
//
// Usage Notes:
// This class is designed to be an (almost, see below) drop-in replacement
// for std::map. It features an almost identical interface, but it is
// implemented using a sorted vector rather than a tree. This is in most
// cases more efficient, as there is less dynamic memory allocation and
// pointer dereferencing.
//
// *************************************************************************
// PLEASE NOTE: There is one vital difference between std::map and VectorMap
// that you will need to note before trying to replace std::map. Since
// VectorMap is implemented using a vector, iterators can and will be
// invalidated by many operations, such as insertions and deletions, and
// due to sorting potentially even normal lookups. Please Please PLEASE make
// sure that you are not storing any iterators to this class.
// *************************************************************************
//
// Performance Notes:
//
// This class uses [[no_unique_address]] to allow comparison predicate
// objects that have no state to take up no additional space in the object.
//
// In addition to the normal map interface, this class provides the
// following members that can be used to manage memory requirements:
//
// * void reserve(size_type count);
//   Allocate enough space for count elements (see vector::reserve()).
//
// * size_type capacity() const;
//   Report how many elements can be stored without reallocating (see
//   vector::capacity()).
//
//--------------------------------------------------------------------------

template<
	typename K, 
	typename V, 
	typename Comparator = std::less<K>, 
	typename Allocator = std::allocator<std::pair <K, V> >>
class VectorMap // Empty base optimization
{
public:
	using key_type = K;
	using mapped_type = V;
	using allocator_type = Allocator;

	using value_type = std::pair<key_type, mapped_type>;

	using key_compare = Comparator;

	using container_type = std::vector<value_type, allocator_type>;
	static_assert(
		std::is_same_v<
		typename std::allocator_traits<allocator_type>::value_type,
		value_type>, 
		"VectorMap allocator must allocate VectorMap::value_type");


	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef typename container_type::reverse_iterator reverse_iterator;
	typedef typename container_type::const_reverse_iterator const_reverse_iterator;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::difference_type difference_type;

	VectorMap();
	explicit VectorMap(const key_compare& comp);
	explicit VectorMap(const key_compare& comp, const allocator_type& alloc);
	VectorMap(const VectorMap& right);
	template <class InputIterator> VectorMap(InputIterator first, InputIterator last);
	template <class InputIterator> VectorMap(InputIterator first, InputIterator last, const key_compare& comp);
	template <class InputIterator> VectorMap(InputIterator first, InputIterator last, const key_compare& comp, const allocator_type& alloc);
	void SwapElementsWithVector(container_type& elementVector);
	iterator begin();
	const_iterator begin() const;
	size_type capacity() const;
	void clear();
	size_type count(const key_type& key) const;
	bool empty() const;
	iterator end();
	const_iterator end() const;
	std::pair<iterator, iterator> equal_range(const key_type& key);
	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const;
	iterator erase(iterator where);
	iterator erase(iterator first, iterator last);
	size_type erase(const key_type& key);
	iterator find(const key_type& key);
	const_iterator find(const key_type& key) const;
	allocator_type get_allocator() const;
	std::pair<iterator, bool> insert(const value_type& val);
	iterator insert(iterator where, const value_type& val);
	template <class InputIterator> void insert(InputIterator first, InputIterator last);
	key_compare key_comp() const;
	iterator lower_bound(const key_type& key);
	const_iterator lower_bound(const key_type& key) const;
	size_type max_size() const;
	reverse_iterator rbegin();
	const_reverse_iterator rbegin() const;
	reverse_iterator rend();
	const_reverse_iterator rend() const;
	void reserve(size_type count);
	size_type size() const;
	void swap(VectorMap& other);
	iterator upper_bound(const key_type& key);
	const_iterator upper_bound(const key_type& key) const;
	mapped_type& operator[](const key_type& key);

private:
	bool KeysEquivalent(const key_type& left, const key_type& right) const;
	void SortAndMakeUnique();

	[[no_unique_address]] key_compare m_compare{};
	container_type m_entries;
};

template <typename K, typename V, typename T, typename A>
VectorMap<K, V, T, A>::VectorMap()
{
}

template <typename K, typename V, typename T, typename A>
VectorMap<K, V, T, A>::VectorMap(const key_compare& comp)
:	m_compare(comp)
{
}

template <typename K, typename V, typename T, typename A>
VectorMap<K, V, T, A>::VectorMap(const key_compare& comp, const allocator_type& alloc)
:	m_compare(comp),
	m_entries(alloc)
{
}

template <typename K, typename V, typename T, typename A>
VectorMap<K, V, T, A>::VectorMap(const VectorMap& right)
:	m_compare(right.m_compare),
	m_entries(right.m_entries)
{
}

template <typename K, typename V, typename T, typename A>
template <class InputIterator> VectorMap<K, V, T, A>::VectorMap(InputIterator first, InputIterator last)
{
	for (; first != last; ++first)
		m_entries.push_back(*first);
	SortAndMakeUnique();
}

template <typename K, typename V, typename T, typename A>
template <class InputIterator> VectorMap<K, V, T, A>::VectorMap(InputIterator first, InputIterator last, const key_compare& comp)
:	m_compare(comp)
{
	for (; first != last; ++first)
		m_entries.push_back(*first);
	SortAndMakeUnique();
}

template <typename K, typename V, typename T, typename A>
template <class InputIterator> VectorMap<K, V, T, A>::VectorMap(InputIterator first, InputIterator last, const key_compare& comp, const allocator_type& alloc)
:	m_compare(comp),
	m_entries(alloc)
{
	for (; first != last; ++first)
		m_entries.push_back(*first);
	SortAndMakeUnique();
}

template <typename K, typename V, typename T, typename A>
void VectorMap<K, V, T, A>::SwapElementsWithVector(typename VectorMap<K, V, T, A>::container_type& elementVector)
{
	m_entries.swap(elementVector);
	SortAndMakeUnique();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::iterator VectorMap<K, V, T, A>::begin()
{
	return m_entries.begin();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::const_iterator VectorMap<K, V, T, A>::begin() const
{
	return m_entries.begin();
}

template <typename K, typename V, typename T, typename A>
void VectorMap<K, V, T, A>::clear()
{
	m_entries.clear();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::size_type VectorMap<K, V, T, A>::capacity() const
{
	return m_entries.capacity();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::size_type VectorMap<K, V, T, A>::count(const key_type& key) const
{
	return find(key) == end() ? size_type(0) : size_type(1);
}

template <typename K, typename V, typename T, typename A>
bool VectorMap<K, V, T, A>::empty() const
{
	return m_entries.empty();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::iterator VectorMap<K, V, T, A>::end()
{
	return m_entries.end();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::const_iterator VectorMap<K, V, T, A>::end() const
{
	return m_entries.end();
}

template <typename K, typename V, typename T, typename A>
std::pair<typename VectorMap<K, V, T, A>::iterator, typename VectorMap<K, V, T, A>::iterator> VectorMap<K, V, T, A>::equal_range(const key_type& key)
{
	iterator lower = lower_bound(key);
	return std::make_pair(lower, upper_bound(key));
}

template <typename K, typename V, typename T, typename A>
std::pair<typename VectorMap<K, V, T, A>::const_iterator, typename VectorMap<K, V, T, A>::const_iterator> VectorMap<K, V, T, A>::equal_range(const key_type& key) const
{
	const_iterator lower = lower_bound(key);
	return std::make_pair(lower, upper_bound(key));
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::iterator VectorMap<K, V, T, A>::erase(iterator where)
{
	return m_entries.erase(where);
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::iterator VectorMap<K, V, T, A>::erase(iterator first, iterator last)
{
	return m_entries.erase(first, last);
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::size_type VectorMap<K, V, T, A>::erase(const key_type& key)
{
	iterator where = find(key);

	if (where == m_entries.end())
		return size_type(0);

	m_entries.erase(where);
	return size_type(1);
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::iterator VectorMap<K, V, T, A>::find(const key_type& key)
{
	iterator it = lower_bound(key);
	if (it != m_entries.end() && !KeysEquivalent(key, it->first))
		it = m_entries.end();
	return it;
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::const_iterator VectorMap<K, V, T, A>::find(const key_type& key) const
{
	const_iterator it = lower_bound(key);
	if (it != m_entries.end() && !KeysEquivalent(key, it->first))
		it = m_entries.end();
	return it;
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::allocator_type VectorMap<K, V, T, A>::get_allocator() const
{
	return m_entries.get_allocator();
}

template <typename K, typename V, typename T, typename A>
std::pair<typename VectorMap<K, V, T, A>::iterator, bool> VectorMap<K, V, T, A>::insert(const value_type& val)
{
	iterator it = lower_bound(val.first);
	bool insertionMade = false;
	if (it == m_entries.end() || !KeysEquivalent(val.first, it->first))
		it = m_entries.insert(it, val), insertionMade = true;
	return std::make_pair(it, insertionMade);
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::iterator VectorMap<K, V, T, A>::insert(iterator where, const value_type& val)
{
	(void)where;
	return insert(val);
}

template <typename K, typename V, typename T, typename A>
template <class InputIterator> void VectorMap<K, V, T, A>::insert(InputIterator first, InputIterator last)
{
	for (; first != last; ++first)
		m_entries.push_back(*first);
	SortAndMakeUnique();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::key_compare VectorMap<K, V, T, A>::key_comp() const
{
	return m_compare;
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::iterator VectorMap<K, V, T, A>::lower_bound(const key_type& key)
{
	return std::lower_bound(
		m_entries.begin(),
		m_entries.end(),
		key,
		[this](const value_type& entry, const key_type& value)
		{
			return m_compare(entry.first, value);
		});
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::const_iterator VectorMap<K, V, T, A>::lower_bound(const key_type& key) const
{
	return std::lower_bound(
		m_entries.begin(),
		m_entries.end(),
		key,
		[this](const value_type& entry, const key_type& value)
		{
			return m_compare(entry.first, value);
		});
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::size_type VectorMap<K, V, T, A>::max_size() const
{
	return m_entries.max_size();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::reverse_iterator VectorMap<K, V, T, A>::rbegin()
{
	return m_entries.rbegin();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::const_reverse_iterator VectorMap<K, V, T, A>::rbegin() const
{
	return m_entries.rbegin();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::reverse_iterator VectorMap<K, V, T, A>::rend()
{
	return m_entries.rend();
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::const_reverse_iterator VectorMap<K, V, T, A>::rend() const
{
	return m_entries.rend();
}

template <typename K, typename V, typename T, typename A>
void VectorMap<K, V, T, A>::reserve(size_type count)
{
	m_entries.reserve(count);
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::size_type VectorMap<K, V, T, A>::size() const
{
	return m_entries.size();
}

template <typename K, typename V, typename T, typename A>
void VectorMap<K, V, T, A>::swap(VectorMap& other)
{
	m_entries.swap(other.m_entries);
	std::swap(m_compare, other.m_compare);
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::iterator VectorMap<K, V, T, A>::upper_bound(const key_type& key)
{
	return std::upper_bound(
		m_entries.begin(),
		m_entries.end(),
		key,
		[this](const key_type& value, const value_type& entry)
		{
			return m_compare(value, entry.first);
		});
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::const_iterator VectorMap<K, V, T, A>::upper_bound(const key_type& key) const
{
	return std::upper_bound(
		m_entries.begin(),
		m_entries.end(),
		key,
		[this](const key_type& value, const value_type& entry)
		{
			return m_compare(value, entry.first);
		});
}

template <typename K, typename V, typename T, typename A>
bool VectorMap<K, V, T, A>::KeysEquivalent(const key_type& left, const key_type& right) const
{
	return !m_compare(left, right) && !m_compare(right, left);
}

template <typename K, typename V, typename T, typename A>
void VectorMap<K, V, T, A>::SortAndMakeUnique()
{
	std::stable_sort(
		m_entries.begin(),
		m_entries.end(),
		[this](const value_type& left, const value_type& right)
		{
			return m_compare(left.first, right.first);
		});

	const iterator newEnd = std::unique(
		m_entries.begin(),
		m_entries.end(),
		[this](const value_type& left, const value_type& right)
		{
			return KeysEquivalent(left.first, right.first);
		});
	m_entries.erase(newEnd, m_entries.end());
}

template <typename K, typename V, typename T, typename A>
typename VectorMap<K, V, T, A>::mapped_type& VectorMap<K, V, T, A>::operator[](const key_type& key)
{
	iterator it = find(key);
	if (it == m_entries.end())
		it = insert(value_type(key, mapped_type())).first;
	return (*it).second;
}

#endif //__SORTEDVECTOR_H__
