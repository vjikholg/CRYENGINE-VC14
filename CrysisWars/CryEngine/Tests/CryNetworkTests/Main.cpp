#include "MM05HandleTests.h"
#include "STLMementoAllocatorTests.h"
#include "TestRunner.h"

#include <cstring>

int main(int argc, char* argv[])
{
	CryTestRunner runner;
	if (argc == 2 && std::strcmp(argv[1], "--mm05-trace") == 0)
	{
		RunMM05HandleTests(runner);
		return runner.ExitCode();
	}
	if (argc != 1)
		return 2;

	RunMM05HandleTests(runner);
	RunSTLMementoAllocatorTests(runner);
	return runner.ExitCode();
}
