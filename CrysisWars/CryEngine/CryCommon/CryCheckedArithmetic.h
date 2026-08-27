#pragma once
#include <cstddef>
#include <limits>

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

template<class T>
inline constexpr size_t CryAllocatorMaxElementCount =
	(std::numeric_limits<size_t>::max)() / sizeof(T);
