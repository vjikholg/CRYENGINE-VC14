#include "StlHashMapAdapterTests.h"
#include "TestRunner.h"
#include "VectorMapTests.h"

int main()
{
	CryTestRunner runner;
	RunStlHashMapAdapterTests(runner);
	RunVectorMapTests(runner);
	return runner.ExitCode();
}
