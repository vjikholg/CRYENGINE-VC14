#include "ComparisonCompatibilityTests.h"
#include "LegacyStdioCompatibilityTests.h"
#include "MemoryAllocatorTests.h"
#include "MemoryAccountingTests.h"
#include "MemoryManagerTests.h"
#include "MM06Tests.h"
#include "SafeAllocTests.h"
#include "StlHashMapAdapterTests.h"
#include "TestRunner.h"
#include "VectorMapTests.h"

#include <cstdio>
#include <cstring>
#include <filesystem>

int main(int argc, char* argv[])
{
	if (argc == 2 && std::strcmp(argv[1], "--mm01-trace") == 0)
	{
		return RunMM01AllocatorTrace();
	}

	if (argc == 2 && std::strcmp(argv[1], "--mm02-layout") == 0)
	{
		return RunMM02MemoryManagementTrace();
	}

	if (argc == 2 &&
		(std::strcmp(argv[1], "-mm03-trace") == 0 ||
		 std::strcmp(argv[1], "--mm03-trace") == 0))
	{
		return RunMM03Trace();
	}

	if (argc == 2 &&
		(std::strcmp(argv[1], "-mm04-trace") == 0 ||
		 std::strcmp(argv[1], "--mm04-trace") == 0))
	{
		return RunMM04DataModelTrace();
	}

	if (argc == 3 && std::strcmp(argv[1], "--mm02-trace") == 0)
	{
		const std::filesystem::path crySystemPath(argv[2]);
		return RunMM02RuntimeTrace(crySystemPath.c_str());
	}

	if (argc == 3 && std::strcmp(argv[1], "--mm02-overflow-probe") == 0)
	{
		const std::filesystem::path crySystemPath(argv[2]);
		return RunMM02OverflowProbe(crySystemPath.c_str());
	}

	if (argc == 3 && std::strcmp(argv[1], "--mm02-realloc-failure-probe") == 0)
	{
		const std::filesystem::path crySystemPath(argv[2]);
		return RunMM02ReallocFailureProbe(crySystemPath.c_str());
	}

	if (argc == 3 && std::strcmp(argv[1], "--mm06-trace") == 0)
	{
		const std::filesystem::path crySystemPath(argv[2]);
		return RunMM06RuntimeTrace(crySystemPath.c_str());
	}

	if (argc == 4 && std::strcmp(argv[1], "--mm06-crash-probe") == 0)
	{
		const std::filesystem::path crySystemPath(argv[2]);
		const std::filesystem::path dumpDirectory(argv[3]);
		return RunMM06CrashProbe(crySystemPath.c_str(), dumpDirectory.c_str());
	}

	if (argc == 3 &&
		(std::strcmp(argv[1], "-mm04-trace") == 0 ||
		 std::strcmp(argv[1], "--mm04-trace") == 0))
	{
		const std::filesystem::path crySystemPath(argv[2]);
		return RunMM04AccountingTrace(crySystemPath.c_str());
	}

	if (argc != 1)
	{
		std::printf(
			"Usage: CryCommonTests.exe [--mm01-trace | --mm02-layout | -mm03-trace | "
			"--mm02-trace <CrySystem.dll> | --mm04-trace [CrySystem.dll] | "
			"--mm06-trace <CrySystem.dll>]\n");
		return 2;
	}

	CryTestRunner runner;
	RunComparisonCompatibilityTests(runner);
	RunLegacyStdioCompatibilityTests(runner);
	RunMemoryAllocatorDataModelTests(runner);
	RunMemoryManagerLayoutTests(runner);
	RunMM06SizeArithmeticTests(runner);
	RunMM04DataModelTests(runner);
	RunStlHashMapAdapterTests(runner);
	RunVectorMapTests(runner);
	return runner.ExitCode();
}
