#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>

class CLegacyMsvcStdioBridge
{
public:
	static constexpr size_t LegacyFileSize = 0x30;
	static constexpr size_t StandardErrorIndex = 2;

	static FILE* MakeIobBase(FILE* stream, size_t legacyIndex)
	{
		if (stream == nullptr)
		{
			return nullptr;
		}

		const uintptr_t streamAddress = reinterpret_cast<uintptr_t>(stream);
		return reinterpret_cast<FILE*>(
			streamAddress - legacyIndex * LegacyFileSize);
	}

	static FILE* ResolveLegacyStream(FILE* iobBase, size_t legacyIndex)
	{
		if (iobBase == nullptr)
		{
			return nullptr;
		}

		const uintptr_t baseAddress = reinterpret_cast<uintptr_t>(iobBase);
		return reinterpret_cast<FILE*>(
			baseAddress + legacyIndex * LegacyFileSize);
	}
};
