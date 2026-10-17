#include "AnimationAssetTests.h"
#include "ComparisonCompatibilityTests.h"
#include "CryFileNameTests.h"
#include "CryNetworkCompatibilityTests.h"
#include "CryStringCompatibilityTests.h"
#include "DxtCompatibilityTests.h"
#include "LegacyStdioCompatibilityTests.h"
#include "MemoryAllocatorTests.h"
#include "MemoryAccountingTests.h"
#include "MemoryManagerTests.h"
#include "MM05Tests.h"
#include "MM06Tests.h"
#include "MM07Tests.h"
#include "MM08Tests.h"
#include "MM09Tests.h"
#include "PhysicsRegressionTests.h"
#include "RuntimeModuleCompatibilityTests.h"
#include "SafeAllocTests.h"
#include "SerializedSizeTests.h"
#include "StlHashMapAdapterTests.h"
#include "TestRunner.h"
#include "VectorMapTests.h"
#include "ZlibCompatibilityTests.h"

#include <cstdio>
#include <cstring>
#include <filesystem>

int main(int argc, char* argv[])
{
	if ((argc == 4 || argc == 5) &&
		std::strcmp(argv[1], "--physics-regression") == 0)
	{
		const std::filesystem::path runtimeDirectory(argv[2]);
		const std::filesystem::path gameRoot(argv[3]);
		const std::filesystem::path reportPath = argc == 5
			? std::filesystem::path(argv[4])
			: std::filesystem::current_path() / "PhysicsRegression.tsv";
		return RunPhysicsRegressionPipeline(
			runtimeDirectory.c_str(),
			gameRoot.c_str(),
			reportPath.c_str());
	}

	if (argc == 6 && std::strcmp(argv[1], "--physics-regression-case") == 0)
	{
		const std::filesystem::path runtimeDirectory(argv[2]);
		const std::filesystem::path gameRoot(argv[3]);
		const std::filesystem::path reportPath(argv[5]);
		return RunPhysicsRegressionCase(
			runtimeDirectory.c_str(),
			gameRoot.c_str(),
			argv[4],
			reportPath.c_str());
	}

	if ((argc == 3 || argc == 4) &&
		std::strcmp(argv[1], "--animation-asset-format") == 0)
	{
		const std::filesystem::path assetRoot(argv[2]);
		const std::filesystem::path reportPath = argc == 4
			? std::filesystem::path(argv[3])
			: std::filesystem::current_path() / "AnimationAssetCompatibility.tsv";
		return RunAnimationAssetFormatCorpus(assetRoot.c_str(), reportPath.c_str());
	}

	if ((argc == 5 || argc == 6) &&
		std::strcmp(argv[1], "--animation-assets") == 0)
	{
		const std::filesystem::path runtimeDirectory(argv[2]);
		const std::filesystem::path gameRoot(argv[3]);
		const std::filesystem::path assetRoot(argv[4]);
		const std::filesystem::path reportPath = argc == 6
			? std::filesystem::path(argv[5])
			: std::filesystem::current_path() / "AnimationAssetCompatibility.tsv";
		return RunAnimationAssetCompatibilityPipeline(
			runtimeDirectory.c_str(),
			gameRoot.c_str(),
			assetRoot.c_str(),
			reportPath.c_str());
	}

	if (argc == 7 && std::strcmp(argv[1], "--animation-assets-runtime") == 0)
	{
		const std::filesystem::path runtimeDirectory(argv[2]);
		const std::filesystem::path gameRoot(argv[3]);
		const std::filesystem::path assetRoot(argv[4]);
		const std::filesystem::path reportPath(argv[5]);
		const std::filesystem::path checkpointPath(argv[6]);
		return RunAnimationAssetRuntimeCorpus(
			runtimeDirectory.c_str(),
			gameRoot.c_str(),
			assetRoot.c_str(),
			reportPath.c_str(),
			checkpointPath.c_str());
	}

	if (argc == 2 && std::strcmp(argv[1], "--mm01-trace") == 0)
	{
		return RunMM01AllocatorTrace();
	}

	if (argc == 2 && std::strcmp(argv[1], "--mm02-layout") == 0)
	{
		return RunMM02MemoryManagementTrace();
	}

	if (argc == 2 && std::strcmp(argv[1], "--mm05-trace") == 0)
	{
		return RunMM05Trace();
	}

	if (argc == 2 && std::strcmp(argv[1], "--mm08-trace") == 0)
	{
		return RunMM08Trace();
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

	if (argc == 3 && std::strcmp(argv[1], "--mm07-trace") == 0)
	{
		const std::filesystem::path crySystemPath(argv[2]);
		return RunMM07RuntimeTrace(crySystemPath.c_str());
	}

	if (argc == 3 && std::strcmp(argv[1], "--mm09-trace") == 0)
	{
		const std::filesystem::path crySystemPath(argv[2]);
		return RunMM09RuntimeTrace(crySystemPath.c_str());
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
			"Usage: CryCommonTests.exe [--physics-regression <runtime-dir> <game-root> [report.tsv] | "
			"--animation-asset-format <asset-root> [report.tsv] | "
			"--animation-assets <runtime-dir> <game-root> <asset-root> [report.tsv] | "
			"--mm01-trace | --mm02-layout | -mm03-trace | "
			"--mm02-trace <CrySystem.dll> | --mm04-trace [CrySystem.dll] | "
			"--mm05-trace | --mm08-trace | "
			"--mm06-trace <CrySystem.dll> | --mm07-trace <CrySystem.dll> | "
			"--mm09-trace <CrySystem.dll>]\n");
		return 2;
	}

	CryTestRunner runner;
	RunComparisonCompatibilityTests(runner);
	RunRuntimeModuleCompatibilityTests(runner);
	RunCryFileNameTests(runner);
	RunCryNetworkCompatibilityTests(runner);
	RunCryStringCompatibilityTests(runner);
	RunDxtCompatibilityTests(runner);
	RunLegacyStdioCompatibilityTests(runner);
	RunMemoryAllocatorDataModelTests(runner);
	RunMemoryManagerLayoutTests(runner);
	RunMM05ModelTests(runner);
	RunMM06SizeArithmeticTests(runner);
	RunMM07MemoryCoverageTests(runner);
	RunMM08PoolAllocatorTests(runner);
	RunMM09CrtSizeTests(runner);
	RunMM04DataModelTests(runner);
	RunSerializedSizeTests(runner);
	RunStlHashMapAdapterTests(runner);
	RunVectorMapTests(runner);
	RunZlibCompatibilityTests(runner);
	return runner.ExitCode();
}
