#include "GameSpyWrapperBoundaryTests.h"
#include "MM05HandleTests.h"
#include "NetworkIdAndIndexTests.h"
#include "STLMementoAllocatorTests.h"
#include "TestRunner.h"
#include "WireFormatTests.h"

#include <cstring>

int main(int argc, char* argv[])
{
	CryTestRunner runner;
	if (argc == 2)
	{
		if (std::strcmp(argv[1], "--mm05-trace") == 0)
			RunMM05HandleTests(runner);
		else if (std::strcmp(argv[1], "--gamespy") == 0)
			RunGameSpyWrapperBoundaryTests(runner);
		else if (std::strcmp(argv[1], "--network-id") == 0)
			RunNetworkIdAndIndexTests(runner);
		else if (std::strcmp(argv[1], "--stl-memento") == 0)
			RunSTLMementoAllocatorTests(runner);
		else if (std::strcmp(argv[1], "--wire-format") == 0)
			RunCryNetworkWireFormatTests(runner);
		else
			return 2;
		return runner.ExitCode();
	}
	if (argc != 1)
		return 2;

	RunMM05HandleTests(runner);
	RunGameSpyWrapperBoundaryTests(runner);
	RunNetworkIdAndIndexTests(runner);
	RunSTLMementoAllocatorTests(runner);
	RunCryNetworkWireFormatTests(runner);
	return runner.ExitCode();
}
