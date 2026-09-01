#pragma once
#include <cstddef>
#include <limits>
#include <type_traits>

constexpr bool CryTryAddSize(
	size_t left,
	size_t right,
	size_t& result) noexcept
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	if (right > maximum - left)
	{
		result = 0;
		return false;
	}

	result = left + right;
	return true;
}

constexpr bool CryTryMultiplySize(
	size_t left,
	size_t right,
	size_t& result) noexcept
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	if (left != 0 && right > maximum / left)
	{
		result = 0;
		return false;
	}

	result = left * right;
	return true;
}

template<class Destination>
constexpr bool CryTryNarrowSize( size_t value, Destination& result) noexcept
{
	static_assert(std::is_integral_v<Destination>);
	constexpr size_t maximum = static_cast<size_t>((std::numeric_limits<Destination>::max)());
	if (value > maximum)
	{
		result = 0;
		return false;
	}

	result = static_cast<Destination>(value);
	return true;
}

template<class Destination>
constexpr Destination CrySaturateSize(size_t value) noexcept
{
	Destination result = 0;
	return CryTryNarrowSize(value, result)
		? result
		: (std::numeric_limits<Destination>::max)();
}

constexpr bool CryIsRangeWithinSize(
	size_t totalSize,
	size_t offset,
	size_t length) noexcept
{
	return offset <= totalSize && length <= totalSize - offset;
}

constexpr bool CryTryGetArrayRangeEnd(
	size_t totalSize,
	size_t offset,
	size_t count,
	size_t elementSize,
	size_t& rangeEnd) noexcept
{
	size_t byteCount = 0;
	if (!CryTryMultiplySize(count, elementSize, byteCount) ||
		!CryIsRangeWithinSize(totalSize, offset, byteCount))
	{
		rangeEnd = 0;
		return false;
	}

	return CryTryAddSize(offset, byteCount, rangeEnd);
}

template<class T>
inline constexpr size_t CryAllocatorMaxElementCount =
	(std::numeric_limits<size_t>::max)() / sizeof(T);
