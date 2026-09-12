#include "CryNetworkTestPreamble.h"
#include "GameSpyWrapperBoundaryTests.h"
#include "TestRunner.h"

#include <StdAfx.h>
#include <Services/GameSpy/GSServerReport.h>

// SValueMap is the only state-contained public GameSpy wrapper surface that
// can be linked here without a GameSpy service object or an SDK object file.
// GSStorage payloads, GSChat vector conversion, QR2 destinations,
// QR2CountCallback, and StatsTrack report filling therefore remain explicit
// coverage gaps. The minimal future seams are a test-only payload/callback
// seam or an injectable SDK call table; QR2 handles also must be exercised by
// the SDK itself because their storage is opaque.

static void TestServerReportValueMapPublicState(CryTestRunner& runner)
{
	SValueMap values;
	string value;

	CRY_TEST_CHECK(runner, values.GetCount() == 0ull);
	CRY_TEST_CHECK(runner, !values.GetValue(12, value));

	values.SetValue(31, "west");
	values.SetValue(12, "ranked");
	values.SetValue(44, "audit");

	CRY_TEST_CHECK(runner, values.GetCount() == 3ull);
	CRY_TEST_CHECK(runner, values.GetValue(12, value));
	CRY_TEST_CHECK(runner, value == "ranked");
	CRY_TEST_CHECK(runner, values.GetValue(31, value));
	CRY_TEST_CHECK(runner, value == "west");

	values.SetValue(31, "updated");
	CRY_TEST_CHECK(runner, values.GetCount() == 3ull);
	CRY_TEST_CHECK(runner, values.GetValue(31, value));
	CRY_TEST_CHECK(runner, value == "updated");
}

void RunGameSpyWrapperBoundaryTests(CryTestRunner& runner)
{
	runner.Run("GameSpy server-report value-map public state", TestServerReportValueMapPublicState);
}
