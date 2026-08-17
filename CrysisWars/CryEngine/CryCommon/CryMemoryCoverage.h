#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

struct SCryMemoryCoverageRange
{
	uintptr_t address;
	size_t size;
};

inline bool CryTryGetMemoryCoverageRangeEnd(
	uintptr_t address,
	size_t size,
	uintptr_t& endAddress) noexcept
{
	const uintptr_t maximumAddress = (std::numeric_limits<uintptr_t>::max)();
	if (size == 0 || size > maximumAddress - address)
	{
		endAddress = 0;
		return false;
	}

	endAddress = address + size;
	return true;
}

inline bool CryTryCalculateMemoryCoverageBitmapSize(
	size_t width,
	size_t height,
	size_t& imageSize) noexcept
{
	constexpr size_t bytesPerPixel = 3;
	const size_t maximum = (std::numeric_limits<size_t>::max)();
	if (height != 0 && width > maximum / height)
	{
		imageSize = 0;
		return false;
	}

	const size_t pixelCount = width * height;
	if (pixelCount > maximum / bytesPerPixel)
	{
		imageSize = 0;
		return false;
	}

	imageSize = pixelCount * bytesPerPixel;
	return true;
}

inline bool CryTryCalculateMemoryCoverageRowOffset(
	size_t width,
	size_t height,
	size_t line,
	size_t headerSize,
	size_t& rowOffset) noexcept
{
	rowOffset = 0;
	if (line >= height)
		return false;

	size_t imageSize = 0;
	if (!CryTryCalculateMemoryCoverageBitmapSize(width, height - 1 - line, imageSize))
		return false;

	const size_t maximum = (std::numeric_limits<size_t>::max)();
	if (imageSize > maximum - headerSize)
		return false;

	rowOffset = headerSize + imageSize;
	return true;
}

inline char CryGetMemoryCoverageLabel(size_t coveredUnits, size_t unitCount) noexcept
{
	if (coveredUnits == 0)
		return '.';
	return coveredUnits >= unitCount ? '#' : '+';
}

inline bool CryBuildMemoryCoverage(
	const std::vector<SCryMemoryCoverageRange>& ranges,
	size_t unitCount,
	std::vector<bool>& coverage,
	uintptr_t& coverageBegin,
	uintptr_t& coverageEnd,
	size_t& bytesPerUnit)
{
	coverage.clear();
	coverageBegin = 0;
	coverageEnd = 0;
	bytesPerUnit = 0;
	if (unitCount == 0)
		return false;

	uintptr_t firstAddress = (std::numeric_limits<uintptr_t>::max)();
	uintptr_t lastAddress = 0;
	for (const SCryMemoryCoverageRange& range : ranges)
	{
		uintptr_t rangeEnd = 0;
		if (!CryTryGetMemoryCoverageRangeEnd(range.address, range.size, rangeEnd))
			continue;

		if (range.address < firstAddress)
			firstAddress = range.address;
		if (rangeEnd > lastAddress)
			lastAddress = rangeEnd;
	}

	if (firstAddress == (std::numeric_limits<uintptr_t>::max)() ||
		lastAddress <= firstAddress)
	{
		return false;
	}

	const uintptr_t span = lastAddress - firstAddress;
	bytesPerUnit = static_cast<size_t>(span / unitCount);
	if (span % unitCount != 0)
		++bytesPerUnit;
	if (bytesPerUnit == 0)
		bytesPerUnit = 1;

	coverage.assign(unitCount, false);
	coverageBegin = firstAddress;
	coverageEnd = lastAddress;

	for (const SCryMemoryCoverageRange& range : ranges)
	{
		uintptr_t rangeEnd = 0;
		if (!CryTryGetMemoryCoverageRangeEnd(range.address, range.size, rangeEnd))
			continue;

		const uintptr_t clippedBegin = range.address < coverageBegin
			? coverageBegin
			: range.address;
		const uintptr_t clippedEnd = rangeEnd > coverageEnd
			? coverageEnd
			: rangeEnd;
		if (clippedEnd <= clippedBegin)
			continue;

		size_t beginUnit = static_cast<size_t>(
			(clippedBegin - coverageBegin) / bytesPerUnit);
		const uintptr_t endOffset = clippedEnd - coverageBegin;
		size_t endUnit = static_cast<size_t>(endOffset / bytesPerUnit);
		if (endOffset % bytesPerUnit != 0)
			++endUnit;

		if (beginUnit > unitCount)
			beginUnit = unitCount;
		if (endUnit > unitCount)
			endUnit = unitCount;
		for (size_t unit = beginUnit; unit < endUnit; ++unit)
			coverage[unit] = true;
	}

	return true;
}
