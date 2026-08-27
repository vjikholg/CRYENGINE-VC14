#include "CryStringCompatibilityTests.h"
#include "TestRunner.h"

#include <platform.h>
#include <CryCheckedArithmetic.h>
#include <bit>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <limits>
#include <type_traits>

struct CryStringLegacyHeaderLayout
{
	int nRefCount;
	int nLength;
	int nAllocSize;
};

struct CryStringWideHeaderLayout
{
	int nRefCount;
	size_t nLength;
	size_t nAllocSize;
};

static_assert(sizeof(void*) == 8, "CryString compatibility tests require Win64.");
static_assert(std::is_same_v<string::size_type, size_t>);
static_assert(std::is_same_v<wstring::size_type, size_t>);
static_assert(std::is_same_v<decltype(string::npos), const string::size_type>);
static_assert(sizeof(CryStringLegacyHeaderLayout) == 12);
static_assert(sizeof(CryStringWideHeaderLayout) == 24);

static bool CryStringTryAllocationSizeModel(
	size_t length,
	size_t elementSize,
	size_t headerSize,
	size_t& allocationSize)
{
	allocationSize = 0;
	size_t characterCount = 0;
	size_t characterBytes = 0;
	return CryTryAddSize(length, size_t{ 1 }, characterCount) &&
		CryTryMultiplySize(characterCount, elementSize, characterBytes) &&
		CryTryAddSize(headerSize, characterBytes, allocationSize);
}

static void TestCryStringCopyOnWrite(CryTestRunner& runner)
{
	string original("alpha");
	string copy(original);

	CRY_TEST_CHECK(runner, std::strcmp(original.c_str(), "alpha") == 0);
	CRY_TEST_CHECK(runner, original.c_str() == copy.c_str());

	copy += "-beta";
	CRY_TEST_CHECK(runner, std::strcmp(original.c_str(), "alpha") == 0);
	CRY_TEST_CHECK(runner, std::strcmp(copy.c_str(), "alpha-beta") == 0);
	CRY_TEST_CHECK(runner, original.c_str() != copy.c_str());
}

static void TestCryStringCoreMutationInvariants(CryTestRunner& runner)
{
	string value("abcdef");
	value.reserve(16);
	value.erase(2, 2);
	CRY_TEST_CHECK(runner, std::strcmp(value.c_str(), "abef") == 0);

	value.insert(2, 2, 'X');
	CRY_TEST_CHECK(runner, std::strcmp(value.c_str(), "abXXef") == 0);

	value.insert(2, "cd", 2);
	CRY_TEST_CHECK(runner, std::strcmp(value.c_str(), "abcdXXef") == 0);

	value.replace(2, 2, "Q", 1);
	CRY_TEST_CHECK(runner, std::strcmp(value.c_str(), "abQXXef") == 0);

	value.replace("XX", "wxyz");
	CRY_TEST_CHECK(runner, std::strcmp(value.c_str(), "abQwxyzef") == 0);

	string whitespace("  alpha  ");
	whitespace.Trim();
	CRY_TEST_CHECK(runner, std::strcmp(whitespace.c_str(), "alpha") == 0);
}

static void TestCryStringStorageInvariants(CryTestRunner& runner)
{
	string value;
	CRY_TEST_CHECK(runner, value.empty());
	CRY_TEST_CHECK(runner, value.size() == 0);
	CRY_TEST_CHECK(runner, value.c_str()[0] == '\0');
	CRY_TEST_CHECK(runner, value.GetAllocatedMemory() == 0);

	value.reserve(64);
	CRY_TEST_CHECK(runner, value.capacity() >= 64);
	CRY_TEST_CHECK(runner, value.GetAllocatedMemory() >= value.capacity() + 1);

	value = "cat";
	value.reserve(0);
	CRY_TEST_CHECK(runner, value.size() == 3);
	CRY_TEST_CHECK(runner, value.capacity() == value.size());
	CRY_TEST_CHECK(runner, value.GetAllocatedMemory() >= value.size() + 1);
	CRY_TEST_CHECK(
		runner,
		string::npos == (std::numeric_limits<string::size_type>::max)());
}

static void TestCryStringConcatenationInvariants(CryTestRunner& runner)
{
	string inPlace("alpha");
	inPlace.reserve(64);
	const char* const originalBuffer = inPlace.c_str();
	inPlace += "-beta";

	CRY_TEST_CHECK(runner, std::strcmp(inPlace.c_str(), "alpha-beta") == 0);
	CRY_TEST_CHECK(runner, inPlace.c_str() == originalBuffer);
	CRY_TEST_CHECK(runner, inPlace.size() <= inPlace.capacity());

	string grown("12345678");
	grown += "-abcdefghijklmnop";
	CRY_TEST_CHECK(
		runner,
		std::strcmp(grown.c_str(), "12345678-abcdefghijklmnop") == 0);
	CRY_TEST_CHECK(runner, grown.size() <= grown.capacity());
	CRY_TEST_CHECK(runner, grown.c_str()[grown.size()] == '\0');

	string self("repeat");
	self += self;
	CRY_TEST_CHECK(runner, std::strcmp(self.c_str(), "repeatrepeat") == 0);
	CRY_TEST_CHECK(runner, self.size() == 12);
}

static void TestCryStringAliasedMutationInvariants(CryTestRunner& runner)
{
	string inserted("abcdef");
	inserted.insert(3, inserted.c_str() + 1, 3);
	CRY_TEST_CHECK(runner, std::strcmp(inserted.c_str(), "abcbcddef") == 0);
	CRY_TEST_CHECK(runner, inserted.c_str()[inserted.length()] == '\0');

	string selfInserted("repeat");
	selfInserted.insert(3, selfInserted.c_str(), selfInserted.length());
	CRY_TEST_CHECK(runner, std::strcmp(selfInserted.c_str(), "reprepeateat") == 0);

	string positionalReplace("abcdef");
	positionalReplace.replace(1, 3, positionalReplace.c_str() + 2, 3);
	CRY_TEST_CHECK(runner, std::strcmp(positionalReplace.c_str(), "acdeef") == 0);
}

static void TestCryStringReplacementInvariants(CryTestRunner& runner)
{
	string shrinking("aaaa");
	shrinking.replace("aa", "x");
	CRY_TEST_CHECK(runner, std::strcmp(shrinking.c_str(), "xx") == 0);
	CRY_TEST_CHECK(runner, shrinking.length() == 2);

	string growing("aba");
	growing.replace("a", "expanded");
	CRY_TEST_CHECK(runner, std::strcmp(growing.c_str(), "expandedbexpanded") == 0);
	CRY_TEST_CHECK(runner, growing.length() == std::strlen("expandedbexpanded"));

	string aliasedReplacement("ab");
	aliasedReplacement.replace("a", aliasedReplacement.c_str());
	CRY_TEST_CHECK(runner, std::strcmp(aliasedReplacement.c_str(), "abb") == 0);
}

static void TestCryStringRangeInvariants(CryTestRunner& runner)
{
	string value("expanded");
	char copied[16] = {};
	CRY_TEST_CHECK(runner, value.copy(copied, 4, value.length()) == 0);
	CRY_TEST_CHECK(runner, value.copy(copied, 4, 0) == 4);
	CRY_TEST_CHECK(runner, std::memcmp(copied, "expa", 4) == 0);

	CRY_TEST_CHECK(runner, value.substr(3, string::npos) == "anded");
	value.erase(value.length() + 10, 4);
	CRY_TEST_CHECK(runner, value == "expanded");
	CRY_TEST_CHECK(runner, value.rfind('d', value.length()) == 7);
	CRY_TEST_CHECK(runner, value.rfind('q') == string::npos);
	CRY_TEST_CHECK(runner, value.compare(value.length(), string::npos, "", 0) == 0);
	CRY_TEST_CHECK(runner, value.compareNoCase(value.length(), string::npos, "", 0) == 0);

	int tokenStart = 100;
	CRY_TEST_CHECK(runner, value.Tokenize(" ", tokenStart).empty());
	CRY_TEST_CHECK(runner, tokenStart == -1);
}

static void TestCryStringFillAssignmentInvariants(CryTestRunner& runner)
{
	const size_t initialMemory = string::_usedMemory(0);
	{
		string original("alpha");
		string copy(original);
		copy.assign(4, 'z');

		CRY_TEST_CHECK(runner, original == "alpha");
		CRY_TEST_CHECK(runner, copy == "zzzz");
		CRY_TEST_CHECK(runner, original.c_str() != copy.c_str());

		copy.assign(0, 'z');
		CRY_TEST_CHECK(runner, copy.empty());
		CRY_TEST_CHECK(runner, copy.c_str()[0] == '\0');
	}
	CRY_TEST_CHECK(runner, string::_usedMemory(0) == initialMemory);
}

static void TestCryWideStringCopyOnWrite(CryTestRunner& runner)
{
	wstring original(L"alpha");
	wstring copy(original);

	CRY_TEST_CHECK(runner, std::wcscmp(original.c_str(), L"alpha") == 0);
	CRY_TEST_CHECK(runner, original.c_str() == copy.c_str());

	copy += L"-beta";
	CRY_TEST_CHECK(runner, std::wcscmp(original.c_str(), L"alpha") == 0);
	CRY_TEST_CHECK(runner, std::wcscmp(copy.c_str(), L"alpha-beta") == 0);
	CRY_TEST_CHECK(runner, original.c_str() != copy.c_str());
}

static void TestCryStringHeaderWidthModel(CryTestRunner& runner)
{
	constexpr size_t beyondLegacyMaximum = static_cast<size_t>(INT_MAX) + 1;
	const int legacyLength = std::bit_cast<int>(
		static_cast<uint32_t>(beyondLegacyMaximum));
	CryStringWideHeaderLayout wideHeader = { 1, beyondLegacyMaximum, beyondLegacyMaximum };

	CRY_TEST_CHECK(runner, legacyLength < 0);
	CRY_TEST_CHECK(runner, static_cast<size_t>(legacyLength) != beyondLegacyMaximum);
	CRY_TEST_CHECK(runner, wideHeader.nLength == beyondLegacyMaximum);
	CRY_TEST_CHECK(runner, wideHeader.nAllocSize == beyondLegacyMaximum);
}

static void TestCryStringCheckedAllocationArithmeticModel(CryTestRunner& runner)
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	size_t allocationSize = 0;

	CRY_TEST_CHECK(
		runner,
		CryStringTryAllocationSizeModel(
			15,
			sizeof(char),
			sizeof(CryStringWideHeaderLayout),
			allocationSize));
	CRY_TEST_CHECK(runner, allocationSize == sizeof(CryStringWideHeaderLayout) + 16);

	CRY_TEST_CHECK(
		runner,
		!CryStringTryAllocationSizeModel(
			maximum,
			sizeof(char),
			sizeof(CryStringWideHeaderLayout),
			allocationSize));
	CRY_TEST_CHECK(runner, allocationSize == 0);
}

static void TestCryStringLengthArithmeticModels(CryTestRunner& runner)
{
	constexpr size_t maximum = (std::numeric_limits<size_t>::max)();
	constexpr size_t stringLength = 16;
	constexpr size_t offset = 12;
	constexpr size_t requestedCount = maximum - 7;
	constexpr size_t legacyEnd = offset + requestedCount;
	constexpr size_t availableCount = stringLength - offset;
	constexpr size_t originalLength = 10;
	constexpr size_t sourceLength = 4;
	constexpr size_t replacementLength = 1;
	constexpr size_t replacementCount = 2;
	constexpr size_t legacyReplacementDelta = replacementLength - sourceLength;
	constexpr size_t repairedReplacementLength =
		originalLength - (sourceLength - replacementLength) * replacementCount;

	CRY_TEST_CHECK(runner, legacyEnd < stringLength);
	CRY_TEST_CHECK(runner, requestedCount > availableCount);
	CRY_TEST_CHECK(runner, legacyReplacementDelta > originalLength);
	CRY_TEST_CHECK(runner, repairedReplacementLength == 4);
	CRY_TEST_CHECK(
		runner,
		string::max_size() ==
			(maximum - sizeof(CryStringWideHeaderLayout)) / sizeof(char) - 1);
	CRY_TEST_CHECK(
		runner,
		wstring::max_size() ==
			(maximum - sizeof(CryStringWideHeaderLayout)) / sizeof(wchar_t) - 1);
}

void RunCryStringCompatibilityTests(CryTestRunner& runner)
{
	runner.Run("CryString copy-on-write preserves source data", TestCryStringCopyOnWrite);
	runner.Run("CryString core mutations preserve contents", TestCryStringCoreMutationInvariants);
	runner.Run("CryString storage invariants survive reallocation", TestCryStringStorageInvariants);
	runner.Run("CryString concatenation preserves capacity and aliasing invariants", TestCryStringConcatenationInvariants);
	runner.Run("CryString aliased mutations preserve source data", TestCryStringAliasedMutationInvariants);
	runner.Run("CryString replacement handles growth and shrinkage", TestCryStringReplacementInvariants);
	runner.Run("CryString range operations clamp without wrapping", TestCryStringRangeInvariants);
	runner.Run("CryString fill assignment preserves ownership and accounting", TestCryStringFillAssignmentInvariants);
	runner.Run("CryString wide copy-on-write preserves source data", TestCryWideStringCopyOnWrite);
	runner.Run("CryString legacy header narrowing is observable", TestCryStringHeaderWidthModel);
	runner.Run("CryString allocation arithmetic has checked boundaries", TestCryStringCheckedAllocationArithmeticModel);
	runner.Run("CryString length arithmetic models legacy wrapping", TestCryStringLengthArithmeticModels);
}
