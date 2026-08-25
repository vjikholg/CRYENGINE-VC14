#include "ZlibCompatibilityTests.h"
#include "TestRunner.h"

#include "zlib/zlib.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <process.h>
#include <string>
#include <system_error>
#include <vector>

static const unsigned char kLegacyZlibFixture[] =
{
	0x78, 0x5e, 0xcb, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x28, 0xcf, 0x2f, 0xca,
	0x49, 0x01, 0x00, 0x1a, 0x0b, 0x04, 0x5d
};

static const unsigned char kLegacyRawDeflateFixture[] =
{
	0xcb, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x28, 0xcf, 0x2f, 0xca, 0x49, 0x01,
	0x00
};

static const unsigned char kLegacyGzipFileFixture[] =
{
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
	0xd3, 0x60, 0x64, 0x60, 0x70, 0x2e, 0xaa, 0x74, 0xcd, 0x4b, 0xcf, 0xcc,
	0x4b, 0x55, 0xa8, 0xca, 0xc9, 0x4c, 0x52, 0x48, 0xce, 0xcf, 0x2d, 0x48,
	0x2c, 0xc9, 0x4c, 0xca, 0xcc, 0xc9, 0x2c, 0xa9, 0x54, 0x48, 0xcb, 0xac,
	0x28, 0x29, 0x2d, 0x4a, 0xe5, 0x02, 0x00, 0x32, 0xe0, 0xeb, 0xef, 0x29,
	0x00, 0x00, 0x00
};

static const char kHelloWorld[] = "hello world";
static const char kGzipFixtureText[] = "CryEngine zlib compatibility fixture\n";

static bool InflateBuffer(const unsigned char* input, size_t inputSize, int windowBits, const unsigned char* expected,size_t expectedSize)
{
	if (input == nullptr || inputSize > std::numeric_limits<uInt>::max() ||
		expectedSize > std::numeric_limits<uInt>::max())
	{
		return false;
	}

	std::vector<unsigned char> output(std::max<size_t>(expectedSize, 1));
	z_stream stream = {};
	stream.next_in = const_cast<Bytef*>(input);
	stream.avail_in = static_cast<uInt>(inputSize);
	stream.next_out = output.data();
	stream.avail_out = static_cast<uInt>(expectedSize);

	if (inflateInit2(&stream, windowBits) != Z_OK)
		return false;

	const int result = inflate(&stream, Z_FINISH);
	const bool matches = result == Z_STREAM_END &&
		stream.total_out == expectedSize &&
		std::equal(expected, expected + expectedSize, output.begin());
	inflateEnd(&stream);
	return matches;
}

static bool CompressAndDecompress(const std::vector<unsigned char>& input, int level)
{
	if (input.size() > std::numeric_limits<uLong>::max())
		return false;

	const Bytef* source = input.empty() ? reinterpret_cast<const Bytef*>("") : input.data();
	const uLong sourceSize = static_cast<uLong>(input.size());
	std::vector<unsigned char> compressed(compressBound(sourceSize));
	uLongf compressedSize = static_cast<uLongf>(compressed.size());

	if (compress2(compressed.data(), &compressedSize, source, sourceSize, level) != Z_OK)
		return false;

	std::vector<unsigned char> restored(std::max<size_t>(input.size(), 1));
	uLongf restoredSize = static_cast<uLongf>(input.size());
	if (uncompress(restored.data(), &restoredSize, compressed.data(), compressedSize) != Z_OK)
		return false;

	return restoredSize == input.size() &&
		std::equal(input.begin(), input.end(), restored.begin());
}

static bool CompressRawAndDecompress(const std::vector<unsigned char>& input, int level)
{
	if (input.size() > std::numeric_limits<uInt>::max())
		return false;

	const Bytef* source = input.empty() ? reinterpret_cast<const Bytef*>("") : input.data();
	std::vector<unsigned char> compressed(compressBound(static_cast<uLong>(input.size())));
	z_stream stream = {};
	stream.next_in = const_cast<Bytef*>(source);
	stream.avail_in = static_cast<uInt>(input.size());
	stream.next_out = compressed.data();
	stream.avail_out = static_cast<uInt>(compressed.size());

	if (deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS, 9, Z_DEFAULT_STRATEGY) != Z_OK)
		return false;

	const int result = deflate(&stream, Z_FINISH);
	const int endResult = deflateEnd(&stream);
	if (result != Z_STREAM_END || endResult != Z_OK)
		return false;

	return InflateBuffer(
		compressed.data(),
		static_cast<size_t>(stream.total_out),
		-MAX_WBITS,
		input.empty() ? reinterpret_cast<const unsigned char*>("") : input.data(),
		input.size());
}

static std::filesystem::path MakeTemporaryPath(const char* suffix)
{
	static unsigned int counter = 0;
	std::error_code error;
	std::filesystem::path directory = std::filesystem::temp_directory_path(error);
	if (error)
		directory = std::filesystem::current_path(error);

	return directory /
		("CryCommonTests-zlib-" + std::to_string(_getpid()) + "-" +
		 std::to_string(++counter) + suffix);
}

static bool WriteBinaryFile(
	const std::filesystem::path& path,
	const unsigned char* data,
	size_t size)
{
	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (!file)
		return false;

	file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
	return file.good();
}

static void RemoveTemporaryFile(const std::filesystem::path& path)
{
	std::error_code error;
	std::filesystem::remove(path, error);
}

static void TestLegacyZlibFixture(CryTestRunner& runner)
{
	CRY_TEST_CHECK(
		runner,
		InflateBuffer(
			kLegacyZlibFixture,
			sizeof(kLegacyZlibFixture),
			MAX_WBITS,
			reinterpret_cast<const unsigned char*>(kHelloWorld),
			sizeof(kHelloWorld) - 1));
}

static void TestLegacyRawDeflateFixture(CryTestRunner& runner)
{
	CRY_TEST_CHECK(
		runner,
		InflateBuffer(
			kLegacyRawDeflateFixture,
			sizeof(kLegacyRawDeflateFixture),
			-MAX_WBITS,
			reinterpret_cast<const unsigned char*>(kHelloWorld),
			sizeof(kHelloWorld) - 1));
}

static void TestLegacyGzipFileEnvelope(CryTestRunner& runner)
{
	const std::filesystem::path path = MakeTemporaryPath(".crygzip");
	const bool written = WriteBinaryFile(
		path,
		kLegacyGzipFileFixture,
		sizeof(kLegacyGzipFileFixture));
	CRY_TEST_CHECK(runner, written);
	if (!written)
	{
		RemoveTemporaryFile(path);
		return;
	}

	gzFile file = gzopen(path.string().c_str(), "rb");
	CRY_TEST_CHECK(runner, file != Z_NULL);
	if (file != Z_NULL)
	{
		unsigned int bitLength = 0;
		std::array<unsigned char, sizeof(kGzipFixtureText) - 1> payload = {};
		const int headerBytes = gzread(file, &bitLength, sizeof(bitLength));
		const int payloadBytes = gzread(file, payload.data(), static_cast<unsigned>(payload.size()));
		const int closeResult = gzclose(file);

		CRY_TEST_CHECK(runner, headerBytes == sizeof(bitLength));
		CRY_TEST_CHECK(runner, payloadBytes == static_cast<int>(payload.size()));
		CRY_TEST_CHECK(runner, bitLength == (payload.size() * 8));
		CRY_TEST_CHECK(
			runner,
			std::equal(
				payload.begin(),
				payload.end(),
				reinterpret_cast<const unsigned char*>(kGzipFixtureText)));
		CRY_TEST_CHECK(runner, closeResult == Z_OK);
	}

	RemoveTemporaryFile(path);
}

static void TestGzipEnvelopeRoundTrip(CryTestRunner& runner)
{
	const std::filesystem::path path = MakeTemporaryPath(".crygzip");
	const unsigned int bitLength = static_cast<unsigned int>((sizeof(kGzipFixtureText) - 1) * 8);
	const unsigned int payloadSize = static_cast<unsigned int>(sizeof(kGzipFixtureText) - 1);

	gzFile writer = gzopen(path.string().c_str(), "wb9");
	CRY_TEST_CHECK(runner, writer != Z_NULL);
	if (writer == Z_NULL)
	{
		RemoveTemporaryFile(path);
		return;
	}

	const int headerBytes = gzwrite(writer, &bitLength, sizeof(bitLength));
	const int payloadBytes = gzwrite(writer, kGzipFixtureText, payloadSize);
	const int closeResult = gzclose(writer);
	CRY_TEST_CHECK(runner, headerBytes == sizeof(bitLength));
	CRY_TEST_CHECK(runner, payloadBytes == static_cast<int>(payloadSize));
	CRY_TEST_CHECK(runner, closeResult == Z_OK);

	gzFile reader = gzopen(path.string().c_str(), "rb");
	CRY_TEST_CHECK(runner, reader != Z_NULL);
	if (reader != Z_NULL)
	{
		unsigned int restoredBitLength = 0;
		std::array<char, sizeof(kGzipFixtureText) - 1> restored = {};
		const int restoredHeaderBytes = gzread(reader, &restoredBitLength, sizeof(restoredBitLength));
		const int restoredPayloadBytes = gzread(reader, restored.data(), payloadSize);
		const int restoredCloseResult = gzclose(reader);

		CRY_TEST_CHECK(runner, restoredHeaderBytes == sizeof(restoredBitLength));
		CRY_TEST_CHECK(runner, restoredPayloadBytes == static_cast<int>(payloadSize));
		CRY_TEST_CHECK(runner, restoredBitLength == bitLength);
		CRY_TEST_CHECK(
			runner,
			std::memcmp(restored.data(), kGzipFixtureText, payloadSize) == 0);
		CRY_TEST_CHECK(runner, restoredCloseResult == Z_OK);
	}

	RemoveTemporaryFile(path);
}

static void TestDeterministicRoundTrips(CryTestRunner& runner)
{
	const std::array<size_t, 8> sizes = {0, 1, 7, 255, 256, 1023, 1024, 65536};
	const std::array<int, 5> levels = {0, 1, 3, 6, 9};

	for (size_t size : sizes)
	{
		std::vector<unsigned char> input(size);
		for (size_t index = 0; index < input.size(); ++index)
			input[index] = static_cast<unsigned char>((index * 31 + size) & 0xff);

		for (int level : levels)
		{
			CRY_TEST_CHECK(runner, CompressAndDecompress(input, level));
			CRY_TEST_CHECK(runner, CompressRawAndDecompress(input, level));
		}
	}
}

static void TestMalformedStreamsAreRejected(CryTestRunner& runner)
{
	std::array<unsigned char, sizeof(kLegacyZlibFixture)> corruptedZlib = {};
	std::copy(std::begin(kLegacyZlibFixture), std::end(kLegacyZlibFixture), corruptedZlib.begin());
	corruptedZlib.back() ^= 0x01;

	std::vector<unsigned char> output(sizeof(kHelloWorld) - 1);
	uLongf outputSize = static_cast<uLongf>(output.size());
	const int zlibResult = uncompress(
		output.data(),
		&outputSize,
		corruptedZlib.data(),
		static_cast<uLong>(corruptedZlib.size()));
	CRY_TEST_CHECK(runner, zlibResult != Z_OK);

	CRY_TEST_CHECK(
		runner,
		!InflateBuffer(
			kLegacyRawDeflateFixture,
			sizeof(kLegacyRawDeflateFixture) - 1,
			-MAX_WBITS,
			reinterpret_cast<const unsigned char*>(kHelloWorld),
			sizeof(kHelloWorld) - 1));
}

static void TestZlibWidthAssumptions(CryTestRunner& runner)
{
	CRY_TEST_CHECK(runner, sizeof(size_t) == sizeof(void*));
	CRY_TEST_CHECK(runner, sizeof(uInt) == sizeof(uint32_t));
	CRY_TEST_CHECK(runner, sizeof(uLong) == sizeof(uint32_t));
	CRY_TEST_CHECK(runner, sizeof(size_t) >= sizeof(uLong));
}

void RunZlibCompatibilityTests(CryTestRunner& runner)
{
	runner.Run("zlib reads legacy zlib fixture", TestLegacyZlibFixture);
	runner.Run("zlib reads legacy raw DEFLATE fixture", TestLegacyRawDeflateFixture);
	runner.Run("zlib reads CryEngine gzip envelope fixture", TestLegacyGzipFileEnvelope);
	runner.Run("zlib preserves CryEngine gzip envelope round trip", TestGzipEnvelopeRoundTrip);
	runner.Run("zlib deterministic round trips", TestDeterministicRoundTrips);
	runner.Run("zlib rejects malformed streams", TestMalformedStreamsAreRejected);
	runner.Run("zlib width assumptions are explicit", TestZlibWidthAssumptions);
}
