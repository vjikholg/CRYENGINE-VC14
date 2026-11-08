#include <platform.h>

#include "RuntimeModuleCompatibilityTests.h"
#include "TestRunner.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>

#include <CryCheckedArithmetic.h>
#include <RenderNodeReference.h>
#include <CryAction/IVehicleSystem.h>
#include <CryFont/XmlParserFont.h>

static_assert(std::is_same_v<decltype(std::declval<const SVehicleImpls&>().Count()), std::size_t>);

static void TestVehicleImplementationEnumeration(CryTestRunner& runner)
{
	SVehicleImpls implementations;
	CRY_TEST_CHECK(runner, implementations.Count() == 0);
	CRY_TEST_CHECK(runner, implementations.GetAt(0).empty());
	implementations.Add("Wheeled");
	implementations.Add("Tracked");
	CRY_TEST_CHECK(runner, implementations.Count() == 2);
	CRY_TEST_CHECK(runner, implementations.GetAt(0) == "Wheeled");
	CRY_TEST_CHECK(runner, implementations.GetAt(1) == "Tracked");
	CRY_TEST_CHECK(runner, implementations.GetAt(2).empty());
	CRY_TEST_CHECK(runner, implementations.GetAt(static_cast<std::size_t>(-1)).empty());

	int scriptCount = 0;
	CRY_TEST_CHECK(runner, CryTryNarrowSize(implementations.Count(), scriptCount));
	CRY_TEST_CHECK(runner, scriptCount == 2);
	const auto maximumIndex = static_cast<std::size_t>((std::numeric_limits<int>::max)());
	CRY_TEST_CHECK(runner, CryTryNarrowSize(maximumIndex, scriptCount));
	CRY_TEST_CHECK(runner, !CryTryNarrowSize(maximumIndex + 1, scriptCount));
}

static void TestFontParserConstSearch(CryTestRunner& runner)
{
	char input[] = "<font>text</font>";
	XmlParserFont parser;
	CRY_TEST_CHECK(runner, parser.create(input, sizeof(input) - 1));
	const char* needle = "</font>";
	CRY_TEST_CHECK(runner, parser.find(needle, 0) == 10);
	CRY_TEST_CHECK(runner, parser.find("<font>", 0) == 0);
	CRY_TEST_CHECK(runner, parser.find("missing", 0) == -1);
	CRY_TEST_CHECK(runner, parser.find(needle, 0, 10) == -1);
	CRY_TEST_CHECK(runner, std::strcmp(input, "<font>text</font>") == 0);
}

static void TestFontParserDocument(CryTestRunner& runner)
{
	char input[] = "<font>text</font>";
	XmlParserFont parser;
	CRY_TEST_CHECK(runner, parser.parse(input, sizeof(input) - 1));
	CRY_TEST_CHECK(runner, parser.getName() == "font");
	CRY_TEST_CHECK(runner, parser.getValue() == "text");
}

static void TestStatObjectSpriteRenderNode(CryTestRunner& runner)
{
	constexpr std::uintptr_t spritePlaceholder = 0xFFFFFFFFu;
	constexpr auto nativePlaceholder = (std::numeric_limits<std::uintptr_t>::max)();
	std::uintptr_t storage = 0;
	CRY_TEST_CHECK(runner, !CryHasRenderNode(nullptr));
	CRY_TEST_CHECK(runner, !CryHasRenderNode(reinterpret_cast<IRenderNode*>(spritePlaceholder)));
	CRY_TEST_CHECK(runner, !CryHasRenderNode(reinterpret_cast<IRenderNode*>(nativePlaceholder)));
	CRY_TEST_CHECK(runner, CryHasRenderNode(reinterpret_cast<IRenderNode*>(&storage)));
}

void RunRuntimeModuleCompatibilityTests(CryTestRunner& runner)
{
	runner.Run("Cry3DEngine rejects sprite render-node placeholders", TestStatObjectSpriteRenderNode);
	runner.Run("CryAction vehicle implementation enumeration and script limits", TestVehicleImplementationEnumeration);
	runner.Run("CryFont parser accepts const search text without mutation", TestFontParserConstSearch);
	runner.Run("CryFont parser preserves document parsing", TestFontParserDocument);
}
