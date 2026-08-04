#include "ComparisonCompatibilityTests.h"
#include "StlHashMapAdapterTests.h"
#include "TestRunner.h"
#include "VectorMapTests.h"

int main()
{
	CryTestRunner runner;
	RunComparisonCompatibilityTests(runner);
	RunStlHashMapAdapterTests(runner);
	RunVectorMapTests(runner);
	return runner.ExitCode();
}
