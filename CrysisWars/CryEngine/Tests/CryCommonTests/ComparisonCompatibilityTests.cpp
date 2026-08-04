#include <platform.h>

#include "ComparisonCompatibilityTests.h"
#include "TestRunner.h"

#include <CryName.h>
#include <IInput.h>

#include <type_traits>
#include <utility>

static_assert(
	std::is_same_v<
		decltype(std::declval<const string&>() == std::declval<const CCryName&>()),
		bool>,
	"CryName equality must support a string left operand");

static_assert(
	std::is_same_v<
		decltype(std::declval<const char*>() == std::declval<const CCryName&>()),
		bool>,
	"CryName equality must support a C-string left operand");

static_assert(
	std::is_same_v<
		decltype(std::declval<const char*>() == std::declval<const TKeyName&>()),
		bool>,
	"TKeyName equality must accept a const right operand");

static void TestKeyNameSymmetricComparison(CryTestRunner& runner)
{
	const TKeyName alpha("Alpha");
	const TKeyName beta("beta");

	CRY_TEST_CHECK(runner, alpha == "alpha");
	CRY_TEST_CHECK(runner, "alpha" == alpha);
	CRY_TEST_CHECK(runner, !(alpha != "ALPHA"));
	CRY_TEST_CHECK(runner, !("ALPHA" != alpha));
	CRY_TEST_CHECK(runner, alpha < beta);
	CRY_TEST_CHECK(runner, "alpha" < beta);
	CRY_TEST_CHECK(runner, beta > "alpha");
	CRY_TEST_CHECK(runner, "beta" > alpha);
}

void RunComparisonCompatibilityTests(CryTestRunner& runner)
{
	runner.Run("CryCommon symmetric key-name comparisons", TestKeyNameSymmetricComparison);
}
