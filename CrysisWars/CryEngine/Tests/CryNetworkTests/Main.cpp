#include "STLMementoAllocatorTests.h"
#include "TestRunner.h"

int main()
{
	CryTestRunner runner;
	RunSTLMementoAllocatorTests(runner);
	return runner.ExitCode();
}
