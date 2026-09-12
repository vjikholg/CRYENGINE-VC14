#include "CryNetworkTestPreamble.h"
#include "MementoAllocatorTestSupport.h"
#include "WireFormatTests.h"
#include "TestRunner.h"

#include <array>
#include <cassert>
#include <bit>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <type_traits>
#include <vector>

#include <windows.h>

#ifndef NET_ASSERT
#define NET_ASSERT assert
#endif

#ifndef UP_STACK_PTR
#define UP_STACK_PTR 0
#endif

#include <ISystem.h>

#include <Config.h>
#include <MementoMemoryManager.h>
#include <StreamAllocator.h>
#include <objcnt.h>
#include <NetLog.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <Compression/ArithModel.h>
#include <Streams/ByteStream.h>
#include <Streams/CommStream.h>

SObjectCounters g_objcnt;
			
class CWireFormatTestAllocator final : public IStreamAllocator
{
public:
	CWireFormatTestAllocator()
		: m_allocation(0)
		, m_allocationCount(0)
		, m_reallocationCount(0)
	{
	}

	~CWireFormatTestAllocator()
	{
		std::free(m_allocation);
	}

	void* Alloc(std::size_t size, void*) override
	{
		++m_allocationCount;
		m_allocation = static_cast<uint8*>(std::malloc(size));
		return m_allocation;
	}

	void* Realloc(void* allocation, std::size_t size) override
	{
		if (allocation != m_allocation)
			return 0;

		uint8* const resizedAllocation =
			static_cast<uint8*>(std::realloc(m_allocation, size));
		if (!resizedAllocation)
			return 0;

		++m_reallocationCount;
		m_allocation = resizedAllocation;
		return resizedAllocation;
	}

	void Free(void* allocation) override
	{
		if (allocation == m_allocation)
		{
			std::free(m_allocation);
			m_allocation = 0;
		}
	}

	std::size_t GetAllocationCount() const
	{
		return m_allocationCount;
	}

	std::size_t GetReallocationCount() const
	{
		return m_reallocationCount;
	}

private:
	uint8* m_allocation;
	std::size_t m_allocationCount;
	std::size_t m_reallocationCount;
};

struct SWireFormatTwelveBytes
{
	uint32 first;
	uint32 second;
	uint32 third;
};

struct SWireFormatSixteenBytes
{
	uint32 first;
	uint32 second;
	uint32 third;
	uint32 fourth;
};

static_assert(sizeof(SWireFormatTwelveBytes) == 12);
static_assert(sizeof(SWireFormatSixteenBytes) == 16);
static_assert(std::is_trivially_copyable_v<SWireFormatTwelveBytes>);
static_assert(std::is_trivially_copyable_v<SWireFormatSixteenBytes>);
static_assert(sizeof(uint16) == 2);
static_assert(std::endian::native == std::endian::little);
static_assert(MAX_UDP_PACKET_SIZE <= (std::numeric_limits<uint16>::max)());
static_assert(
	MAX_UDP_PACKET_SIZE + UDP_HEADER_SIZE <=
		(std::numeric_limits<uint16>::max)());

static void TestMixedByteStreamOffsets(CryTestRunner& runner)
{
	CByteStreamPacker packer;

	CRY_TEST_CHECK(runner, packer.GetNextOfs_Fixed(NTypelist::Int2Type<1>()) == 0u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs(2) == 4u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs(3) == 8u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs_Fixed(NTypelist::Int2Type<4>()) == 12u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs(1) == 1u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs_Fixed(NTypelist::Int2Type<2>()) == 6u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs(4) == 16u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs_Fixed(NTypelist::Int2Type<8>()) == 20u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs_Fixed(NTypelist::Int2Type<12>()) == 28u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs_Fixed(NTypelist::Int2Type<16>()) == 40u);
	CRY_TEST_CHECK(runner, packer.GetNextOfs(0) == 56u);
}

static void TestByteStreamOffsetLimitIsTransactional(CryTestRunner& runner)
{
	CByteStreamPacker packer;
	uint32 offset = 0;

	CRY_TEST_CHECK(
		runner,
		packer.TryGetNextOfs(CByteStreamPacker::MaxStreamSize - 3u, offset));
	CRY_TEST_CHECK(runner, offset == 0u);
	CRY_TEST_CHECK(
		runner,
		packer.GetNextOfs(0) == CByteStreamPacker::MaxStreamSize - 3u);

	offset = 17u;
	CRY_TEST_CHECK(runner, !packer.TryGetNextOfs(4u, offset));
	CRY_TEST_CHECK(runner, offset == 17u);
	CRY_TEST_CHECK(
		runner,
		packer.GetNextOfs(0) == CByteStreamPacker::MaxStreamSize - 3u);

	CByteStreamPacker paddedOverflow;
	offset = 23u;
	CRY_TEST_CHECK(
		runner,
		!paddedOverflow.TryGetNextOfs(CByteStreamPacker::MaxStreamSize, offset));
	CRY_TEST_CHECK(runner, offset == 23u);
	CRY_TEST_CHECK(runner, paddedOverflow.GetNextOfs(0) == 0u);
}

static void TestMixedByteStreamGoldenLayoutAndRoundTrip(CryTestRunner& runner)
{
	CWireFormatTestAllocator allocator;
	CByteOutputStream output(&allocator, 64);
	const uint8 twoBytes[] = { 0xb1, 0xb2 };
	const uint8 threeBytes[] = { 0xc1, 0xc2, 0xc3 };
	const uint8 oneByte = 0xa2;
	const uint8 fourBytes[] = { 0xe1, 0xe2, 0xe3, 0xe4 };

	output.PutTyped<uint8>() = 0xa1;
	output.Put(twoBytes, sizeof(twoBytes));
	output.Put(threeBytes, sizeof(threeBytes));
	output.PutTyped<uint32>() = 0xd4d3d2d1u;
	output.Put(&oneByte, sizeof(oneByte));
	output.PutTyped<uint16>() = 0xb4b3u;
	output.Put(fourBytes, sizeof(fourBytes));
	output.PutTyped<uint64>() = 0xf8f7f6f5f4f3f2f1ull;
	output.PutTyped<SWireFormatTwelveBytes>() = { 0x14131211u, 0x18171615u, 0x1c1b1a19u };
	output.PutTyped<SWireFormatSixteenBytes>() =
		{ 0x24232221u, 0x28272625u, 0x2c2b2a29u, 0x302f2e2du };

	const std::array<uint8, 56> expected = {
		0xa1, 0xa2, 0x00, 0x00,
		0xb1, 0xb2, 0xb3, 0xb4,
		0xc1, 0xc2, 0xc3, 0x00,
		0xd1, 0xd2, 0xd3, 0xd4,
		0xe1, 0xe2, 0xe3, 0xe4,
		0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
		0x19, 0x1a, 0x1b, 0x1c,
		0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
		0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30
	};

	CRY_TEST_CHECK(runner, allocator.GetAllocationCount() == 1u);
	CRY_TEST_CHECK(runner, allocator.GetReallocationCount() == 0u);
	CRY_TEST_CHECK(runner, output.GetSize() == expected.size());
	CRY_TEST_CHECK(runner, std::memcmp(output.GetBuffer(), expected.data(), expected.size()) == 0);
	CRY_TEST_CHECK(runner, output.GetBuffer()[2] == 0u);
	CRY_TEST_CHECK(runner, output.GetBuffer()[3] == 0u);
	CRY_TEST_CHECK(runner, output.GetBuffer()[11] == 0u);

	CByteInputStream input(output.GetBuffer(), output.GetSize());
	uint8 readTwoBytes[2] = {};
	uint8 readThreeBytes[3] = {};
	uint8 readOneByte = 0;
	uint8 readFourBytes[4] = {};

	CRY_TEST_CHECK(runner, input.GetTyped<uint8>() == 0xa1u);
	input.Get(readTwoBytes, sizeof(readTwoBytes));
	input.Get(readThreeBytes, sizeof(readThreeBytes));
	CRY_TEST_CHECK(runner, input.GetTyped<uint32>() == 0xd4d3d2d1u);
	input.Get(&readOneByte, sizeof(readOneByte));
	CRY_TEST_CHECK(runner, input.GetTyped<uint16>() == 0xb4b3u);
	input.Get(readFourBytes, sizeof(readFourBytes));
	CRY_TEST_CHECK(runner, input.GetTyped<uint64>() == 0xf8f7f6f5f4f3f2f1ull);
	const SWireFormatTwelveBytes readTwelveBytes = input.GetTyped<SWireFormatTwelveBytes>();
	const SWireFormatSixteenBytes readSixteenBytes = input.GetTyped<SWireFormatSixteenBytes>();

	CRY_TEST_CHECK(runner, std::memcmp(readTwoBytes, twoBytes, sizeof(twoBytes)) == 0);
	CRY_TEST_CHECK(runner, std::memcmp(readThreeBytes, threeBytes, sizeof(threeBytes)) == 0);
	CRY_TEST_CHECK(runner, readOneByte == oneByte);
	CRY_TEST_CHECK(runner, std::memcmp(readFourBytes, fourBytes, sizeof(fourBytes)) == 0);
	CRY_TEST_CHECK(runner, readTwelveBytes.first == 0x14131211u);
	CRY_TEST_CHECK(runner, readTwelveBytes.second == 0x18171615u);
	CRY_TEST_CHECK(runner, readTwelveBytes.third == 0x1c1b1a19u);
	CRY_TEST_CHECK(runner, readSixteenBytes.first == 0x24232221u);
	CRY_TEST_CHECK(runner, readSixteenBytes.second == 0x28272625u);
	CRY_TEST_CHECK(runner, readSixteenBytes.third == 0x2c2b2a29u);
	CRY_TEST_CHECK(runner, readSixteenBytes.fourth == 0x302f2e2du);
	CRY_TEST_CHECK(runner, input.GetSize() == expected.size());
}

static void TestByteStreamGrowthPreservesDataAndPadding(CryTestRunner& runner)
{
	CWireFormatTestAllocator allocator;
	CByteOutputStream output(&allocator, 4);
	const uint8 threeBytes[] = { 0xde, 0xad, 0xbe };
	const std::array<uint8, 8> beforeSecondGrowth = {
		0x5a, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44
	};
	const std::array<uint8, 16> expected = {
		0x5a, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44,
		0xde, 0xad, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	output.PutTyped<uint8>() = 0x5a;
	output.PutTyped<uint32>() = 0x44332211u;
	CRY_TEST_CHECK(runner, output.GetSize() == beforeSecondGrowth.size());
	CRY_TEST_CHECK(
		runner,
		std::memcmp(output.GetBuffer(), beforeSecondGrowth.data(), beforeSecondGrowth.size()) == 0);

	output.Put(threeBytes, sizeof(threeBytes));
	CRY_TEST_CHECK(runner, allocator.GetAllocationCount() == 1u);
	CRY_TEST_CHECK(runner, allocator.GetReallocationCount() == 2u);
	CRY_TEST_CHECK(runner, output.GetSize() == 12u);
	CRY_TEST_CHECK(runner, std::memcmp(output.GetBuffer(), expected.data(), expected.size()) == 0);
}

static void TestUint32SerializedFieldBoundary(CryTestRunner& runner)
{
	CWireFormatTestAllocator allocator;
	CByteOutputStream output(&allocator, 4);
	const uint32 maximumLength = (std::numeric_limits<uint32>::max)();

	output.PutTyped<uint32>() = maximumLength;
	CRY_TEST_CHECK(runner, output.GetSize() == sizeof(maximumLength));

	CByteInputStream input(output.GetBuffer(), output.GetSize());
	CRY_TEST_CHECK(runner, input.GetTyped<uint32>() == maximumLength);
	CRY_TEST_CHECK(runner, input.GetSize() == sizeof(maximumLength));
}

static void TestArithModelTwelveBitStringBoundary(CryTestRunner& runner)
{
	static const uint32 maxStringLength = (1u << 12) - 1u;
	string value('x', maxStringLength);
	CMementoMemoryManager mementoManager;
	SetCurrentMementoManagerForTest(&mementoManager);

	{
		CWireFormatTestAllocator allocator;
		CArithModel writer;
		CCommOutputStream output(&allocator, 8192, 0x7e);
		writer.WriteString(output, value);
		const size_t encodedSize = output.Flush();
		CRY_TEST_CHECK(runner, encodedSize <= output.GetOutputCapacity());

		CArithModel reader;
		CCommInputStream input(output.GetPtr(), encodedSize);
		string readValue;
		CRY_TEST_CHECK(runner, input.GetBonus() == 0x7eu);
		CRY_TEST_CHECK(runner, reader.ReadString(input, readValue));
		CRY_TEST_CHECK(runner, readValue.size() == maxStringLength);
		CRY_TEST_CHECK(runner, readValue == value);
	}

	SetCurrentMementoManagerForTest(nullptr);
}

static void TestStringTableCopyOnWrite(CryTestRunner& runner)
{
	CMementoMemoryManager mementoManager;
	SetCurrentMementoManagerForTest(&mementoManager);

	{
		CArithModel firstWriter;
		CArithModel secondWriter(firstWriter);
		CWireFormatTestAllocator firstAllocator;
		CWireFormatTestAllocator secondAllocator;
		CCommOutputStream firstOutput(&firstAllocator, 1024, 0);
		CCommOutputStream secondOutput(&secondAllocator, 1024, 0);

		CRY_TEST_CHECK(runner, firstWriter.TableWriteString(firstOutput, "first"));
		CRY_TEST_CHECK(runner, secondWriter.TableWriteString(secondOutput, "second"));

		const size_t firstSize = firstOutput.Flush();
		const size_t secondSize = secondOutput.Flush();
		CArithModel firstReader;
		CArithModel secondReader;
		CCommInputStream firstInput(firstOutput.GetPtr(), firstSize);
		CCommInputStream secondInput(secondOutput.GetPtr(), secondSize);
		string firstValue;
		string secondValue;

		firstReader.TableReadString(firstInput, firstValue);
		secondReader.TableReadString(secondInput, secondValue);
		CRY_TEST_CHECK(runner, firstValue == "first");
		CRY_TEST_CHECK(runner, secondValue == "second");
	}

	SetCurrentMementoManagerForTest(nullptr);
}

static void TestPacketSizeAndUint16AccountingInvariants(CryTestRunner& runner)
{
	const size_t packetSize = MAX_UDP_PACKET_SIZE;
	const size_t packetSizeWithUdpHeader = packetSize + UDP_HEADER_SIZE;
	const uint16 packetSizeAsUint16 = static_cast<uint16>(packetSize);
	const uint16 packetSizeWithHeaderAsUint16 =
		static_cast<uint16>(packetSizeWithUdpHeader);

	CRY_TEST_CHECK(runner, packetSize == 4096u);
	CRY_TEST_CHECK(runner, packetSize == packetSizeAsUint16);
	CRY_TEST_CHECK(runner, packetSizeWithUdpHeader == packetSizeWithHeaderAsUint16);
	CRY_TEST_CHECK(runner, packetSize < (std::numeric_limits<uint16>::max)());
	CRY_TEST_CHECK(runner, packetSizeWithUdpHeader <= (std::numeric_limits<uint16>::max)());
}

void RunCryNetworkWireFormatTests(CryTestRunner& runner)
{
	runner.Run("ByteStream mixed offsets", TestMixedByteStreamOffsets);
	runner.Run(
		"ByteStream offset limit rejects without mutation",
		TestByteStreamOffsetLimitIsTransactional);
	runner.Run("ByteStream mixed golden layout and round trip", TestMixedByteStreamGoldenLayoutAndRoundTrip);
	runner.Run("ByteStream growth preserves data and padding", TestByteStreamGrowthPreservesDataAndPadding);
	runner.Run("ByteStream uint32 serialized field boundary", TestUint32SerializedFieldBoundary);
	runner.Run("ArithModel twelve-bit string boundary", TestArithModelTwelveBitStringBoundary);
	runner.Run("StringTable copy-on-write independence", TestStringTableCopyOnWrite);
	runner.Run("Packet size uint16 accounting invariants", TestPacketSizeAndUint16AccountingInvariants);
}
