#include "StlHashMapAdapterTests.h"
#include "TestRunner.h"

int main()
{
	CryTestRunner runner;
	RunStlHashMapAdapterTests(runner);
	return runner.ExitCode();
}
