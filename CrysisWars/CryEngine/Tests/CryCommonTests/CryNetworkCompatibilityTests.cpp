#include "CryNetworkCompatibilityTests.h"
#include "TestRunner.h"

#include <platform.h>
#include <CryCheckedArithmetic.h>
#include <CrySizer.h>
#include <ISerialize.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

#ifndef NET_ASSERT
#define NET_ASSERT assert
#endif

#include <Context/ChangeList.h>
#include <Cryptography/rijndael.h>

static void TestNetworkWidthBoundaries(CryTestRunner& runner)
{
	uint16 narrow16 = 0;
	CRY_TEST_CHECK(runner, CryTryNarrowSize((std::numeric_limits<uint16>::max)(), narrow16));
	CRY_TEST_CHECK(runner, narrow16 == (std::numeric_limits<uint16>::max)());
	CRY_TEST_CHECK(
		runner,
		!CryTryNarrowSize(static_cast<size_t>((std::numeric_limits<uint16>::max)()) + 1, narrow16));
	CRY_TEST_CHECK(runner, narrow16 == 0);

	uint32 narrow32 = 0;
	CRY_TEST_CHECK(runner, CryTryNarrowSize((std::numeric_limits<uint32>::max)(), narrow32));
	CRY_TEST_CHECK(runner, narrow32 == (std::numeric_limits<uint32>::max)());
#if SIZE_MAX > UINT32_MAX
	CRY_TEST_CHECK(
		runner,
		!CryTryNarrowSize(static_cast<size_t>((std::numeric_limits<uint32>::max)()) + 1, narrow32));
	CRY_TEST_CHECK(runner, narrow32 == 0);
#endif
}

static void TestChangeListIndexInvariants(CryTestRunner& runner)
{
	CChangeList<uint32> changes;
	const SNetObjectID first(7, 1);
	const SNetObjectID second(13, 2);
	const SNetObjectID largestValid(SNetObjectID::InvalidId - 1, 3);

	CRY_TEST_CHECK(runner, !changes.AddChange(SNetObjectID(), 1));
	CRY_TEST_CHECK(runner, changes.AddChange(first, 1));
	CRY_TEST_CHECK(runner, !changes.AddChange(first, 2));
	CRY_TEST_CHECK(runner, changes.IsChanged(first, 1));
	CRY_TEST_CHECK(runner, changes.IsChanged(first, 2));
	CRY_TEST_CHECK(runner, changes.AddChange(second, 4));
	CRY_TEST_CHECK(runner, changes.AddChange(largestValid, 8));

	changes.Remove(first);
	CRY_TEST_CHECK(runner, !changes.IsChanged(first, 1));
	CRY_TEST_CHECK(runner, changes.IsChanged(second, 4));
	CRY_TEST_CHECK(runner, changes.IsChanged(largestValid, 8));

	changes.Flush();
	CRY_TEST_CHECK(runner, changes.GetObjects().empty());
	CRY_TEST_CHECK(runner, !changes.IsChanged(second, 4));
}

static void TestRijndaelBlockLengthConversion(CryTestRunner& runner)
{
	int bitCount = -1;
	CRY_TEST_CHECK(runner, Rijndael::TryGetBlockBitCount(0, bitCount));
	CRY_TEST_CHECK(runner, bitCount == 0);
	CRY_TEST_CHECK(runner, Rijndael::TryGetBlockBitCount(16, bitCount));
	CRY_TEST_CHECK(runner, bitCount == 128);
	CRY_TEST_CHECK(runner, !Rijndael::TryGetBlockBitCount(15, bitCount));
	CRY_TEST_CHECK(runner, bitCount == 0);

	const size_t largestAligned =
		(static_cast<size_t>((std::numeric_limits<int>::max)()) / Rijndael::BitsPerByte) &
		~(Rijndael::BlockSizeBytes - 1);
	CRY_TEST_CHECK(runner, Rijndael::TryGetBlockBitCount(largestAligned, bitCount));
	CRY_TEST_CHECK(runner, bitCount == static_cast<int>(largestAligned * Rijndael::BitsPerByte));
	CRY_TEST_CHECK(
		runner,
		!Rijndael::TryGetBlockBitCount(largestAligned + Rijndael::BlockSizeBytes, bitCount));
	CRY_TEST_CHECK(runner, bitCount == 0);
}

static void TestRijndaelAes256KnownVector(CryTestRunner& runner)
{
	const std::array<UINT8, 32> key = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
	};
	const std::array<UINT8, 16> plaintext = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
	};
	const std::array<UINT8, 16> expectedCiphertext = {
		0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
		0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89
	};
	std::array<UINT8, 16> ciphertext = {};
	std::array<UINT8, 16> decrypted = {};

	Rijndael encryptor;
	CRY_TEST_CHECK(
		runner,
		encryptor.init(Rijndael::ECB, Rijndael::Encrypt, key.data(), Rijndael::Key32Bytes) == RIJNDAEL_SUCCESS);
	CRY_TEST_CHECK(
		runner,
		encryptor.blockEncrypt(plaintext.data(), 128, ciphertext.data()) == 128);
	CRY_TEST_CHECK(runner, ciphertext == expectedCiphertext);

	Rijndael decryptor;
	CRY_TEST_CHECK(
		runner,
		decryptor.init(Rijndael::ECB, Rijndael::Decrypt, key.data(), Rijndael::Key32Bytes) == RIJNDAEL_SUCCESS);
	CRY_TEST_CHECK(
		runner,
		decryptor.blockDecrypt(ciphertext.data(), 128, decrypted.data()) == 128);
	CRY_TEST_CHECK(runner, decrypted == plaintext);
}

void RunCryNetworkCompatibilityTests(CryTestRunner& runner)
{
	runner.Run("CryNetwork fixed-width conversion boundaries", TestNetworkWidthBoundaries);
	runner.Run("CryNetwork change-list index invariants", TestChangeListIndexInvariants);
	runner.Run("Rijndael block byte-to-bit conversion", TestRijndaelBlockLengthConversion);
	runner.Run("Rijndael AES-256 known vector", TestRijndaelAes256KnownVector);
}
