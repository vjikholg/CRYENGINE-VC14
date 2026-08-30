#include "DxtCompatibilityTests.h"

#include "TestRunner.h"

#include <squish.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

struct SDxtFixture
{
	std::uint32_t width;
	std::uint32_t height;
	std::uint32_t fourCc;
	size_t blockOffset;
	size_t blockBytes;
	int squishFlags;
	std::vector<std::uint8_t> ddsBytes;
};

static std::uint32_t MakeDxtFourCc(char a, char b, char c, char d)
{
	return static_cast<std::uint32_t>(static_cast<unsigned char>(a)) |
		(static_cast<std::uint32_t>(static_cast<unsigned char>(b)) << 8) |
		(static_cast<std::uint32_t>(static_cast<unsigned char>(c)) << 16) |
		(static_cast<std::uint32_t>(static_cast<unsigned char>(d)) << 24);
}

static bool ReadBinaryFile(const std::filesystem::path& path, std::vector<std::uint8_t>& bytes)
{
	std::ifstream input(path, std::ios::binary | std::ios::ate);
	if (!input)
		return false;

	const std::streamoff length = input.tellg();
	if (length < 0 || static_cast<std::uintmax_t>(length) > std::numeric_limits<size_t>::max())
		return false;

	bytes.resize(static_cast<size_t>(length));
	input.seekg(0, std::ios::beg);
	return bytes.empty() || input.read(reinterpret_cast<char*>(bytes.data()), length).good();
}

static bool ReadU32(const std::vector<std::uint8_t>& bytes, size_t offset, std::uint32_t& value)
{
	if (offset > bytes.size() || bytes.size() - offset < sizeof(value))
		return false;

	std::memcpy(&value, bytes.data() + offset, sizeof(value));
	return true;
}

static bool LoadDxtFixture(const std::filesystem::path& path, SDxtFixture& fixture)
{
	if (!ReadBinaryFile(path, fixture.ddsBytes) || fixture.ddsBytes.size() < 128)
		return false;

	std::uint32_t magic = 0;
	std::uint32_t headerSize = 0;
	if (!ReadU32(fixture.ddsBytes, 0, magic) ||
		!ReadU32(fixture.ddsBytes, 4, headerSize) ||
		!ReadU32(fixture.ddsBytes, 12, fixture.height) ||
		!ReadU32(fixture.ddsBytes, 16, fixture.width) ||
		!ReadU32(fixture.ddsBytes, 84, fixture.fourCc))
	{
		return false;
	}

	if (magic != MakeDxtFourCc('D', 'D', 'S', ' ') ||
		headerSize != 124 || fixture.width == 0 || fixture.height == 0)
	{
		return false;
	}

	size_t bytesPerBlock = 0;
	if (fixture.fourCc == MakeDxtFourCc('D', 'X', 'T', '1'))
	{
		bytesPerBlock = 8;
		fixture.squishFlags = squish::kDxt1;
	}
	else if (fixture.fourCc == MakeDxtFourCc('D', 'X', 'T', '3'))
	{
		bytesPerBlock = 16;
		fixture.squishFlags = squish::kDxt3;
	}
	else if (fixture.fourCc == MakeDxtFourCc('D', 'X', 'T', '5'))
	{
		bytesPerBlock = 16;
		fixture.squishFlags = squish::kDxt5;
	}
	else
	{
		return false;
	}

	const size_t blocksWide = (static_cast<size_t>(fixture.width) + 3) / 4;
	const size_t blocksHigh = (static_cast<size_t>(fixture.height) + 3) / 4;
	if (blocksWide > std::numeric_limits<size_t>::max() / blocksHigh)
		return false;

	const size_t blockCount = blocksWide * blocksHigh;
	if (blockCount > std::numeric_limits<size_t>::max() / bytesPerBlock)
		return false;

	fixture.blockOffset = 128;
	fixture.blockBytes = blockCount * bytesPerBlock;
	return fixture.blockOffset <= fixture.ddsBytes.size() &&
		fixture.blockBytes <= fixture.ddsBytes.size() - fixture.blockOffset;
}

static std::filesystem::path GetDxtTestDataDirectory()
{
	wchar_t executablePath[MAX_PATH] = {};
	const DWORD length = GetModuleFileNameW(0, executablePath, MAX_PATH);
	if (length == 0 || length == MAX_PATH)
		return {};

	return std::filesystem::path(executablePath).parent_path() / "TestData" / "DxtCompatibility";
}

static void CompareSquishToNvDxt(
	CryTestRunner& runner,
	const char* fixtureName,
	unsigned int allowedMaximumDelta)
{
	const std::filesystem::path dataDirectory = GetDxtTestDataDirectory();
	const std::filesystem::path ddsPath = dataDirectory / (std::string(fixtureName) + ".dds");
	const std::filesystem::path goldenPath = dataDirectory / (std::string(fixtureName) + ".nvdxt.rgba");

	SDxtFixture fixture = {};
	const bool loadedFixture = LoadDxtFixture(ddsPath, fixture);
	CRY_TEST_CHECK(runner, loadedFixture);
	if (!loadedFixture)
		return;

	std::vector<std::uint8_t> nvDxtPixels;
	const bool loadedGolden = ReadBinaryFile(goldenPath, nvDxtPixels);
	CRY_TEST_CHECK(runner, loadedGolden);
	if (!loadedGolden)
		return;

	const size_t pixelCount = static_cast<size_t>(fixture.width) * fixture.height;
	CRY_TEST_CHECK(runner, pixelCount <= std::numeric_limits<size_t>::max() / 4);
	if (pixelCount > std::numeric_limits<size_t>::max() / 4)
		return;

	std::vector<std::uint8_t> squishPixels(pixelCount * 4);
	CRY_TEST_CHECK(runner, nvDxtPixels.size() == squishPixels.size());
	if (nvDxtPixels.size() != squishPixels.size())
		return;

	squish::DecompressImage(
		squishPixels.data(),
		static_cast<int>(fixture.width),
		static_cast<int>(fixture.height),
		fixture.ddsBytes.data() + fixture.blockOffset,
		fixture.squishFlags);

	size_t mismatchedBytes = 0;
	unsigned int maximumDelta = 0;
	size_t channelMismatches[4] = {};
	unsigned int channelMaximumDeltas[4] = {};
	for (size_t i = 0; i < squishPixels.size(); ++i)
	{
		const unsigned int lhs = squishPixels[i];
		const unsigned int rhs = nvDxtPixels[i];
		const unsigned int delta = lhs > rhs ? lhs - rhs : rhs - lhs;
		maximumDelta = (std::max)(maximumDelta, delta);
		mismatchedBytes += delta != 0 ? 1 : 0;
		const size_t channel = i % 4;
		channelMismatches[channel] += delta != 0 ? 1 : 0;
		channelMaximumDeltas[channel] =
			(std::max)(channelMaximumDeltas[channel], delta);
	}

	if (mismatchedBytes != 0)
	{
		std::printf(
			"  %s: %zu of %zu channel bytes differ; maximum delta=%u; "
			"RGBA mismatches=%zu/%zu/%zu/%zu; RGBA maximum deltas=%u/%u/%u/%u\n",
			fixtureName,
			mismatchedBytes,
			squishPixels.size(),
			maximumDelta,
			channelMismatches[0],
			channelMismatches[1],
			channelMismatches[2],
			channelMismatches[3],
			channelMaximumDeltas[0],
			channelMaximumDeltas[1],
			channelMaximumDeltas[2],
			channelMaximumDeltas[3]);
	}

	CRY_TEST_CHECK(runner, maximumDelta <= allowedMaximumDelta);
	if (allowedMaximumDelta == 0)
		CRY_TEST_CHECK(runner, mismatchedBytes == 0);
}

static void TestMwllDxt1AgainstNvDxt(CryTestRunner& runner)
{
	CompareSquishToNvDxt(runner, "mwll_tankcolor_dxt1", 0);
}

static void TestMwllDxt3AgainstNvDxt(CryTestRunner& runner)
{
	CompareSquishToNvDxt(runner, "mwll_dot_dxt3", 0);
}

static void TestMwllDxt5AgainstNvDxt(CryTestRunner& runner)
{
	CompareSquishToNvDxt(runner, "mwll_light_color_dxt5", 1);
}

void RunDxtCompatibilityTests(CryTestRunner& runner)
{
	runner.Run("MWLL DXT1: Squish matches nvDXT", TestMwllDxt1AgainstNvDxt);
	runner.Run("MWLL DXT3: Squish matches nvDXT", TestMwllDxt3AgainstNvDxt);
	runner.Run("MWLL DXT5: Squish matches nvDXT within one LSB", TestMwllDxt5AgainstNvDxt);
}
