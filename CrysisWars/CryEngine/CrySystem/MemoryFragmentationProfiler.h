#ifndef __MEMORYFRAGMENTATIONPROFILER_H__
#define __MEMORYFRAGMENTATIONPROFILER_H__

#pragma once

#include <CryMemoryCoverage.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <vector>

#if defined(WIN32) || defined(WIN64)

class CMemoryFragmentationProfiler
{
public:
	CMemoryFragmentationProfiler()
		: m_dwLine(UninitializedLine)
	{
	}

	void DumpMemoryCoverage()
	{
		if (m_dwLine == UninitializedLine)
			Init();
		if (m_dwLine == UninitializedLine || m_dwLine >= m_nLineCount)
			return;

		std::vector<SCryMemoryCoverageRange> committedRanges;
		size_t availableMemory = 0;
		size_t usedMemory = 0;
		CollectMemoryRanges(committedRanges, availableMemory, usedMemory);

		std::vector<bool> coverage;
		uintptr_t coverageBegin = 0;
		uintptr_t coverageEnd = 0;
		size_t bytesPerUnit = 0;
		if (!CryBuildMemoryCoverage(
				committedRanges,
				CoverageUnitCount,
				coverage,
				coverageBegin,
				coverageEnd,
				bytesPerUnit))
		{
			CryLog("CMemoryFragmentationProfiler: no committed memory ranges found");
			return;
		}

		CryLog(
			"CMemoryFragmentationProfiler Y=%u, available memory=%zu MB, used memory=%zu MB, range=[%p,%p), bytes/unit=%zu",
			m_dwLine,
			BytesToMegabytesRoundedUp(availableMemory),
			BytesToMegabytesRoundedUp(usedMemory),
			reinterpret_cast<void*>(coverageBegin),
			reinterpret_cast<void*>(coverageEnd),
			bytesPerUnit);

		LogCoverage(coverage);
		DumpToRAWCoverage(coverage);
	}

private:
	static constexpr unsigned int UninitializedLine = 0xffffffffu;
	static constexpr size_t CoverageUnitCount = 1024 * 8;
	static constexpr size_t m_nPixelsPerLine = 1024;
	static constexpr size_t m_nLineCount = 128;

	static size_t BytesToMegabytesRoundedUp(size_t bytes) noexcept
	{
		constexpr size_t bytesPerMegabyte = 1024 * 1024;
		return bytes / bytesPerMegabyte + (bytes % bytesPerMegabyte != 0 ? 1 : 0);
	}

	static void AddSaturated(size_t value, size_t& total) noexcept
	{
		const size_t maximum = (std::numeric_limits<size_t>::max)();
		total = value > maximum - total ? maximum : total + value;
	}

	static void CollectMemoryRanges(
		std::vector<SCryMemoryCoverageRange>& committedRanges,
		size_t& availableMemory,
		size_t& usedMemory)
	{
		SYSTEM_INFO systemInfo = {};
		GetSystemInfo(&systemInfo);
		uintptr_t address = reinterpret_cast<uintptr_t>(
			systemInfo.lpMinimumApplicationAddress);
		const uintptr_t maximumAddress = reinterpret_cast<uintptr_t>(
			systemInfo.lpMaximumApplicationAddress);

		while (address < maximumAddress)
		{
			MEMORY_BASIC_INFORMATION memoryInfo = {};
			if (VirtualQuery(
					reinterpret_cast<const void*>(address),
					&memoryInfo,
					sizeof(memoryInfo)) == 0)
			{
				break;
			}

			const uintptr_t rangeAddress = reinterpret_cast<uintptr_t>(
				memoryInfo.BaseAddress);
			const size_t rangeSize = memoryInfo.RegionSize;
			if (memoryInfo.State == MEM_COMMIT)
			{
				committedRanges.push_back({ rangeAddress, rangeSize });
				AddSaturated(rangeSize, usedMemory);
			}
			else if (memoryInfo.State == MEM_FREE)
			{
				AddSaturated(rangeSize, availableMemory);
			}

			uintptr_t nextAddress = 0;
			if (!CryTryGetMemoryCoverageRangeEnd(
					rangeAddress,
					rangeSize,
					nextAddress) ||
				nextAddress <= address)
			{
				break;
			}
			address = nextAddress;
		}
	}

	void Init()
	{
		FILE* const output = fopen("MemoryCoverage.bmp", "wb");
		if (output == nullptr)
			return;

		BITMAPFILEHEADER fileHeader = {};
		BITMAPINFOHEADER infoHeader = {};
		size_t imageSize = 0;
		if (!CryTryCalculateMemoryCoverageBitmapSize(
				m_nPixelsPerLine,
				m_nLineCount,
				imageSize) ||
			imageSize > (std::numeric_limits<DWORD>::max)())
		{
			fclose(output);
			return;
		}

		fileHeader.bfType = 0x4D42;
		fileHeader.bfSize = static_cast<DWORD>(
			sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize);
		fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		infoHeader.biSize = sizeof(BITMAPINFOHEADER);
		infoHeader.biWidth = static_cast<LONG>(m_nPixelsPerLine);
		infoHeader.biHeight = static_cast<LONG>(m_nLineCount);
		infoHeader.biPlanes = 1;
		infoHeader.biBitCount = 24;
		infoHeader.biCompression = BI_RGB;
		infoHeader.biSizeImage = static_cast<DWORD>(imageSize);

		fwrite(&fileHeader, 1, sizeof(fileHeader), output);
		fwrite(&infoHeader, 1, sizeof(infoHeader), output);
		for (size_t pixel = 0; pixel < m_nPixelsPerLine * m_nLineCount; ++pixel)
		{
			putc(0, output);
			putc(0, output);
			putc(0, output);
		}

		fclose(output);
		m_dwLine = 0;
	}

	void LogCoverage(const std::vector<bool>& coverage)
	{
		constexpr size_t charactersPerLine = 128;
		char result[charactersPerLine + 1] = {};
		const size_t unitsPerCharacter = coverage.size() / charactersPerLine;
		for (size_t character = 0; character < charactersPerLine; ++character)
		{
			const size_t begin = character * unitsPerCharacter;
			const size_t end = begin + unitsPerCharacter;
			const size_t covered = static_cast<size_t>(
				std::count(coverage.begin() + begin, coverage.begin() + end, true));
			result[character] = CryGetMemoryCoverageLabel(
				covered,
				unitsPerCharacter);
		}
		CryLog("         Coverage=%s", result);
	}

	void DumpToRAWCoverage(const std::vector<bool>& coverage)
	{
		if (m_dwLine >= m_nLineCount)
			return;

		FILE* const output = fopen("MemoryCoverage.bmp", "rb+");
		if (output == nullptr)
			return;

		size_t wideOffset = 0;
		if (!CryTryCalculateMemoryCoverageRowOffset(
				m_nPixelsPerLine,
				m_nLineCount,
				m_dwLine,
				sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER),
				wideOffset) ||
			wideOffset > (std::numeric_limits<long>::max)())
		{
			fclose(output);
			return;
		}
		const long offset = static_cast<long>(wideOffset);
		if (fseek(output, offset, SEEK_SET) != 0)
		{
			fclose(output);
			return;
		}

		const size_t unitsPerPixel = coverage.size() / m_nPixelsPerLine;
		for (size_t pixel = 0; pixel < m_nPixelsPerLine; ++pixel)
		{
			const size_t begin = pixel * unitsPerPixel;
			const size_t end = begin + unitsPerPixel;
			const size_t covered = static_cast<size_t>(
				std::count(coverage.begin() + begin, coverage.begin() + end, true));
			size_t value = 256 - (256 * covered) / unitsPerPixel;
			if (value > 0)
				value = 127 + value / 2;
			const unsigned char color = static_cast<unsigned char>(value);
			putc(color, output);
			putc(color, output);
			putc(color, output);
		}

		fclose(output);
		++m_dwLine;
	}

	unsigned int m_dwLine;
};

#else

class CMemoryFragmentationProfiler
{
public:
	void DumpMemoryCoverage() {}
};

#endif

#endif
