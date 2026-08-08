#include "ComparisonCompatibilityTests.h"
#include "MemoryAllocatorTests.h"
#include "StlHashMapAdapterTests.h"
#include "TestRunner.h"
#include "VectorMapTests.h"

#include <cstdio>
#include <cstring>

int main(int argc, char* argv[])
{
	if (argc == 2 && std::strcmp(argv[1], "--mm01-trace") == 0)
	{
		return RunMM01AllocatorTrace();
	}

	if (argc != 1)
	{
		std::printf("Usage: CryCommonTests.exe [--mm01-trace]\n");
		return 2;
	}

	CryTestRunner runner;
	RunComparisonCompatibilityTests(runner);
	RunMemoryAllocatorDataModelTests(runner);
	RunStlHashMapAdapterTests(runner);
	RunVectorMapTests(runner);
	return runner.ExitCode();
}
