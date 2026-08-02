#include <platform.h>

#include "VectorMapTests.h"

#include "TestRunner.h"

#include <VectorMap.h>

#include <array>
#include <iterator>

struct VectorMapDirectionalCompare
{
	explicit VectorMapDirectionalCompare(bool descending = false)
		: descending(descending)
	{
	}

	bool operator()(int left, int right) const
	{
		return descending ? left > right : left < right;
	}

	bool descending;
};

struct VectorMapNonDefaultMapped
{
	explicit VectorMapNonDefaultMapped(int value)
		: value(value)
	{
	}

	int value;
};

static void TestVectorMapInsertFindAndErase(CryTestRunner& runner)
{
	VectorMap<int, int> values;
	values.insert(std::make_pair(4, 40));
	values.insert(std::make_pair(2, 20));
	values.insert(std::make_pair(6, 60));

	CRY_TEST_CHECK(runner, values.size() == 3u);
	CRY_TEST_CHECK(runner, values.begin()->first == 2);
	CRY_TEST_CHECK(runner, values.find(4) != values.end());
	CRY_TEST_CHECK(runner, values.find(4)->second == 40);
	CRY_TEST_CHECK(runner, values.find(5) == values.end());
	CRY_TEST_CHECK(runner, values.count(4) == 1u);
	CRY_TEST_CHECK(runner, values.count(5) == 0u);
	CRY_TEST_CHECK(runner, values.erase(4) == 1u);
	CRY_TEST_CHECK(runner, values.erase(4) == 0u);
}

static void TestVectorMapBounds(CryTestRunner& runner)
{
	VectorMap<int, int> values;
	values.insert(std::make_pair(2, 20));
	values.insert(std::make_pair(4, 40));
	values.insert(std::make_pair(6, 60));

	CRY_TEST_CHECK(runner, values.lower_bound(3)->first == 4);
	CRY_TEST_CHECK(runner, values.upper_bound(4)->first == 6);

	const VectorMap<int, int>& constValues = values;
	const std::pair<VectorMap<int, int>::const_iterator, VectorMap<int, int>::const_iterator> found =
		constValues.equal_range(4);
	CRY_TEST_CHECK(runner, std::distance(found.first, found.second) == 1);
	CRY_TEST_CHECK(runner, found.first->first == 4);

	const std::pair<VectorMap<int, int>::const_iterator, VectorMap<int, int>::const_iterator> missing =
		constValues.equal_range(5);
	CRY_TEST_CHECK(runner, missing.first == missing.second);
	CRY_TEST_CHECK(runner, missing.first->first == 6);
}

static void TestVectorMapBulkInsertionKeepsFirstEquivalentKey(CryTestRunner& runner)
{
	typedef VectorMap<int, int> TestMap;
	const std::array<TestMap::value_type, 5> initial = {
		std::make_pair(3, 30),
		std::make_pair(1, 10),
		std::make_pair(3, 300),
		std::make_pair(2, 20),
		std::make_pair(1, 100)
	};
	TestMap values(initial.begin(), initial.end());

	CRY_TEST_CHECK(runner, values.size() == 3u);
	CRY_TEST_CHECK(runner, values.find(1)->second == 10);
	CRY_TEST_CHECK(runner, values.find(3)->second == 30);

	const std::array<TestMap::value_type, 3> additional = {
		std::make_pair(2, 200),
		std::make_pair(4, 40),
		std::make_pair(4, 400)
	};
	values.insert(additional.begin(), additional.end());

	CRY_TEST_CHECK(runner, values.size() == 4u);
	CRY_TEST_CHECK(runner, values.find(2)->second == 20);
	CRY_TEST_CHECK(runner, values.find(4)->second == 40);
}

static void TestVectorMapStatefulComparatorCopyAndSwap(CryTestRunner& runner)
{
	typedef VectorMap<int, int, VectorMapDirectionalCompare> TestMap;
	TestMap descending(VectorMapDirectionalCompare(true));
	descending.insert(std::make_pair(1, 10));
	descending.insert(std::make_pair(3, 30));
	descending.insert(std::make_pair(2, 20));

	CRY_TEST_CHECK(runner, descending.begin()->first == 3);
	CRY_TEST_CHECK(runner, descending.key_comp().descending);

	TestMap copied(descending);
	CRY_TEST_CHECK(runner, copied.begin()->first == 3);
	CRY_TEST_CHECK(runner, copied.key_comp().descending);

	TestMap ascending(VectorMapDirectionalCompare(false));
	ascending.insert(std::make_pair(4, 40));
	ascending.insert(std::make_pair(5, 50));
	descending.swap(ascending);

	CRY_TEST_CHECK(runner, !descending.key_comp().descending);
	CRY_TEST_CHECK(runner, descending.begin()->first == 4);
	CRY_TEST_CHECK(runner, ascending.key_comp().descending);
	CRY_TEST_CHECK(runner, ascending.begin()->first == 3);
}

static void TestVectorMapVectorInterchange(CryTestRunner& runner)
{
	typedef VectorMap<int, int> TestMap;
	TestMap values;
	values.insert(std::make_pair(9, 90));

	TestMap::container_type incoming;
	incoming.push_back(std::make_pair(3, 30));
	incoming.push_back(std::make_pair(1, 10));
	incoming.push_back(std::make_pair(3, 300));
	values.SwapElementsWithVector(incoming);

	CRY_TEST_CHECK(runner, values.size() == 2u);
	CRY_TEST_CHECK(runner, values.begin()->first == 1);
	CRY_TEST_CHECK(runner, values.find(3)->second == 30);
	CRY_TEST_CHECK(runner, incoming.size() == 1u);
	CRY_TEST_CHECK(runner, incoming.front().first == 9);
}

static void TestVectorMapLookupDoesNotRequireDefaultMappedType(CryTestRunner& runner)
{
	typedef VectorMap<int, VectorMapNonDefaultMapped> TestMap;
	TestMap values;
	values.insert(TestMap::value_type(7, VectorMapNonDefaultMapped(70)));

	CRY_TEST_CHECK(runner, values.count(7) == 1u);
	CRY_TEST_CHECK(runner, values.count(8) == 0u);
	CRY_TEST_CHECK(runner, values.find(7)->second.value == 70);
}

void RunVectorMapTests(CryTestRunner& runner)
{
	runner.Run("VectorMap insert/find/erase", TestVectorMapInsertFindAndErase);
	runner.Run("VectorMap bounds", TestVectorMapBounds);
	runner.Run("VectorMap bulk insertion", TestVectorMapBulkInsertionKeepsFirstEquivalentKey);
	runner.Run("VectorMap stateful comparator copy/swap", TestVectorMapStatefulComparatorCopyAndSwap);
	runner.Run("VectorMap vector interchange", TestVectorMapVectorInterchange);
	runner.Run("VectorMap non-default mapped lookup", TestVectorMapLookupDoesNotRequireDefaultMappedType);
}
