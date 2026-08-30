#include <windows.h>

#include "dxtlib.h"

#include <cstdio>
#include <vector>

struct SNvDxtOracleReadState
{
	FILE* file;
};

static NV_ERROR_CODE ReadDdsBytes(void* buffer, size_t count, void* userData)
{
	SNvDxtOracleReadState* state = static_cast<SNvDxtOracleReadState*>(userData);
	return std::fread(buffer, 1, count, state->file) == count ? NV_OK : NV_READ_FAILED;
}

static bool ReadDdsDimensions(FILE* file, size_t& width, size_t& height)
{
	unsigned int header[7] = {};
	if (std::fread(header, sizeof(header), 1, file) != 1)
		return false;

	const unsigned int ddsMagic =
		static_cast<unsigned int>('D') |
		(static_cast<unsigned int>('D') << 8) |
		(static_cast<unsigned int>('S') << 16) |
		(static_cast<unsigned int>(' ') << 24);
	if (header[0] != ddsMagic || header[1] != 124)
		return false;

	height = header[3];
	width = header[4];
	return width != 0 && height != 0 && std::fseek(file, 0, SEEK_SET) == 0;
}

static bool WriteRgba(const char* path, const void* pixels, size_t byteCount)
{
	FILE* output = std::fopen(path, "wb");
	if (output == 0)
		return false;

	const bool wroteAll = std::fwrite(pixels, 1, byteCount, output) == byteCount;
	return std::fclose(output) == 0 && wroteAll;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::fprintf(stderr, "Usage: LegacyNvDxtOracle.exe <input.dds> <output.rgba>\n");
		return 2;
	}

	FILE* input = std::fopen(argv[1], "rb");
	if (input == 0)
	{
		std::fprintf(stderr, "Unable to open input DDS: %s\n", argv[1]);
		return 3;
	}

	size_t width = 0;
	size_t height = 0;
	if (!ReadDdsDimensions(input, width, height))
	{
		std::fprintf(stderr, "Invalid DDS header: %s\n", argv[1]);
		std::fclose(input);
		return 4;
	}

	nvImageContainer imageData;
	imageData.rgbaMIPImage.resize(1);
	imageData.rgbaMIPImage[0].realloc(width, height);

	SNvDxtOracleReadState state = { input };
	const NV_ERROR_CODE result = nvDDS::nvDXTdecompress(
		imageData,
		PF_RGBA,
		1,
		ReadDdsBytes,
		&state);
	std::fclose(input);

	if (result != NV_OK || imageData.rgbaMIPImage.numMIPMaps() == 0 || imageData.nPlanes != 4)
	{
		std::fprintf(stderr, "nvDXT decompression failed with code %d\n", result);
		return 5;
	}

	RGBAImage& mip = imageData.rgbaMIPImage[0];
	const size_t outputBytes = imageData.width * imageData.height * imageData.nPlanes;
	if (!WriteRgba(argv[2], mip.pixels(), outputBytes))
	{
		std::fprintf(stderr, "Unable to write output: %s\n", argv[2]);
		return 6;
	}

	std::printf(
		"Decoded %s: %Iu x %Iu, %Iu bytes\n",
		argv[1],
		imageData.width,
		imageData.height,
		outputBytes);
	return 0;
}
