#include "LegacyStdioCompatibilityTests.h"
#include "TestRunner.h"

#include <Scaleform/LegacyMsvcStdio.h>

#include <cstdio>

static void TestLegacyMsvcIobLayout(CryTestRunner& runner)
{
	FILE* const base = CLegacyMsvcStdioBridge::MakeIobBase(
		stderr,
		CLegacyMsvcStdioBridge::StandardErrorIndex);
	FILE* const legacyStderr = CLegacyMsvcStdioBridge::ResolveLegacyStream(
		base,
		CLegacyMsvcStdioBridge::StandardErrorIndex);

	CRY_TEST_CHECK(runner, base != nullptr);
	CRY_TEST_CHECK(runner, legacyStderr == stderr);
	CRY_TEST_CHECK(runner, _fileno(legacyStderr) == _fileno(stderr));
}

void RunLegacyStdioCompatibilityTests(CryTestRunner& runner)
{
	runner.Run("VC8 x64 stdio bridge preserves stream layout", TestLegacyMsvcIobLayout);
}
