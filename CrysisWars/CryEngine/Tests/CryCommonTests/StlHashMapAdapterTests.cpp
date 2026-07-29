#include <platform.h>

#include "StlHashMapAdapterTests.h"
#include "TestRunner.h"

#include <StlHashMap.h>

#include <type_traits>
#include <utility>


typedef stl::CryHashMap<uint32, int, stl::hash_uint32> UInt32Map;

static_assert(
	std::is_same< UInt32Map::value_type, std::pair<const uint32, int> >::value,
	"stl::CryHashMap must expose the std::unordered_map const-key value_type");

static void TestUInt32InsertFindAndErase(CryTestRunner& runner)
{
	UInt32Map values;
	values.emplace(7u, 70);
	values.emplace(11u, 110);

	CRY_TEST_CHECK(runner, values.size() == 2u);
	CRY_TEST_CHECK(runner, values.find(7u) != values.end());
	CRY_TEST_CHECK(runner, values.find(7u)->second == 70);
	CRY_TEST_CHECK(runner, values.find(9u) == values.end());

	CRY_TEST_CHECK(runner, values.erase(7u) == 1u);
	CRY_TEST_CHECK(runner, values.find(7u) == values.end());
	CRY_TEST_CHECK(runner, values.size() == 1u);
}

static void TestUInt32SubscriptOverwritesExistingValue(CryTestRunner& runner)
{
	UInt32Map values;
	values[42u] = 100;
	values[42u] = 200;

	CRY_TEST_CHECK(runner, values.size() == 1u);
	CRY_TEST_CHECK(runner, values.at(42u) == 200);
}

static void TestCaseSensitiveCStringKeysUseStringContent(CryTestRunner& runner)
{
	typedef stl::CryHashMap<const char*, int, stl::hash_strcmp<const char*> > StringMap;

	char storedKey[] = "CryNetwork";
	char equivalentKey[] = "CryNetwork";
	char differentCaseKey[] = "crynetwork";
	StringMap values;
	values.emplace(storedKey, 1);

	CRY_TEST_CHECK(
		runner,
		static_cast<const void*>(storedKey) != static_cast<const void*>(equivalentKey));
	CRY_TEST_CHECK(runner, values.find(equivalentKey) != values.end());
	CRY_TEST_CHECK(runner, values.find(equivalentKey)->second == 1);
	CRY_TEST_CHECK(runner, values.find(differentCaseKey) == values.end());
}

static void TestCaseInsensitiveCStringKeysUseFoldedContent(CryTestRunner& runner)
{
	typedef stl::CryHashMap<const char*, int, stl::hash_stricmp<const char*> > StringMap;

	char storedKey[] = "CryRenderer";
	char equivalentKey[] = "cryrenderer";
	StringMap values;
	values.emplace(storedKey, 1);
	values[equivalentKey] = 2;

	CRY_TEST_CHECK(runner, values.size() == 1u);
	CRY_TEST_CHECK(runner, values.find(equivalentKey) != values.end());
	CRY_TEST_CHECK(runner, values.find(equivalentKey)->second == 2);
}

void RunStlHashMapAdapterTests(CryTestRunner& runner)
{
	runner.Run("stl::CryHashMap uint32 insert/find/erase", TestUInt32InsertFindAndErase);
	runner.Run("stl::CryHashMap uint32 overwrite", TestUInt32SubscriptOverwritesExistingValue);
	runner.Run("stl::CryHashMap case-sensitive C-string content", TestCaseSensitiveCStringKeysUseStringContent);
	runner.Run("stl::CryHashMap case-insensitive C-string content", TestCaseInsensitiveCStringKeysUseFoldedContent);
}
