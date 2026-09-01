#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "AnimationAssetTests.h"

#pragma warning(push, 0)
#include <CryCheckedArithmetic.h>
#include <CryHeaders.h>
#include <I3DEngine.h>
#include <ICryAnimation.h>
#include <IStatObj.h>
#include <ISystem.h>
#pragma warning(pop)

#include <windows.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <string>
#include <system_error>
#include <vector>

struct AnimationAssetResult
{
	std::filesystem::path path;
	std::string stage;
	std::string extension;
	std::string message;
	uintmax_t fileSize = 0;
	size_t chunkCount = 0;
	uint32 jointCount = 0;
	uint32 animationCount = 0;
	bool passed = false;
};

struct AnimationAssetCounts
{
	size_t passed = 0;
	size_t failed = 0;
};

struct AnimationAssetSystemCallback : ISystemUserCallback
{
	bool OnError(const char* errorString) override
	{
		std::fprintf(stderr, "CrySystem error: %s\n", errorString != nullptr ? errorString : "<null>");
		return false;
	}

	void OnSaveDocument() override {}
	void OnProcessSwitch() override {}
	void OnInitProgress(const char* progressMessage) override
	{
		if (progressMessage != nullptr)
			std::printf("CrySystem: %s\n", progressMessage);
	}
	void GetMemoryUsage(ICrySizer*) override {}
};

struct AnimationRuntimeState
{
	using CreateSystemFunction = ISystem* (__cdecl*)(const SSystemInitParams& params);
	using ValidateAnimationAssetFunction = bool (__cdecl*)(
		const char* path,
		char* errorMessage,
		size_t errorMessageCapacity);

	HMODULE crySystemModule = nullptr;
	ISystem* system = nullptr;
	ICharacterManager* characterManager = nullptr;
	I3DEngine* engine3D = nullptr;
	ValidateAnimationAssetFunction validateAnimationAsset = nullptr;
	std::filesystem::path previousWorkingDirectory;
	AnimationAssetSystemCallback systemCallback;
	HWND testWindow = nullptr;
};

static const std::array<const char*, 5> AnimationAssetExtensions = {
	".anm", ".caf", ".cga", ".cgf", ".chr"
};

static std::string AnimationAssetLowercase(std::string value)
{
	std::transform(
		value.begin(),
		value.end(),
		value.begin(),
		[](unsigned char character) { return static_cast<char>(std::tolower(character)); });
	return value;
}

static bool AnimationAssetIsSupportedExtension(const std::filesystem::path& path)
{
	const std::string extension = AnimationAssetLowercase(path.extension().string());
	return std::find(
		AnimationAssetExtensions.begin(),
		AnimationAssetExtensions.end(),
		extension) != AnimationAssetExtensions.end();
}

static std::string AnimationAssetSanitizeReportField(std::string value)
{
	for (char& character : value)
	{
		if (character == '\t' || character == '\r' || character == '\n')
			character = ' ';
	}
	return value;
}

static std::string AnimationAssetPathForReport(const std::filesystem::path& path)
{
	return AnimationAssetSanitizeReportField(path.generic_string());
}

static void AnimationAssetWriteReportHeader(std::ofstream& report)
{
	report << "status\tstage\textension\tbytes\tchunks\tjoints\tanimations\tpath\tmessage\n";
}

static void AnimationAssetWriteResult(
	std::ofstream& report,
	const AnimationAssetResult& result)
{
	report
		<< (result.passed ? "PASS" : "FAIL") << '\t'
		<< result.stage << '\t'
		<< result.extension << '\t'
		<< result.fileSize << '\t'
		<< result.chunkCount << '\t'
		<< result.jointCount << '\t'
		<< result.animationCount << '\t'
		<< AnimationAssetPathForReport(result.path) << '\t'
		<< AnimationAssetSanitizeReportField(result.message) << '\n';
	report.flush();
}

static std::vector<std::filesystem::path> AnimationAssetDiscoverCorpus(
	const std::filesystem::path& assetRoot,
	std::string& error)
{
	std::vector<std::filesystem::path> assets;
	std::error_code statusError;
	if (!std::filesystem::is_directory(assetRoot, statusError))
	{
		error = "Asset root is not an accessible directory";
		return assets;
	}

	std::error_code iterationError;
	std::filesystem::recursive_directory_iterator iterator(
		assetRoot,
		std::filesystem::directory_options::skip_permission_denied,
		iterationError);
	const std::filesystem::recursive_directory_iterator end;
	for (; iterator != end; iterator.increment(iterationError))
	{
		if (iterationError)
		{
			error = "Unable to enumerate the complete asset root: " + iterationError.message();
			return {};
		}

		std::error_code fileError;
		if (iterator->is_regular_file(fileError) &&
			!fileError &&
			AnimationAssetIsSupportedExtension(iterator->path()))
		{
			assets.push_back(iterator->path());
		}
	}

	std::sort(
		assets.begin(),
		assets.end(),
		[](const std::filesystem::path& left, const std::filesystem::path& right)
		{
			return AnimationAssetLowercase(left.generic_string()) <
				AnimationAssetLowercase(right.generic_string());
		});

	if (assets.empty())
		error = "No CHR, CGA, ANM, CAF, or CGF files were found";
	return assets;
}

static bool AnimationAssetReadAt(
	std::ifstream& stream,
	uintmax_t offset,
	void* destination,
	size_t byteCount)
{
	if (offset > static_cast<uintmax_t>((std::numeric_limits<std::streamoff>::max)()))
		return false;

	stream.clear();
	stream.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
	if (!stream)
		return false;

	stream.read(static_cast<char*>(destination), static_cast<std::streamsize>(byteCount));
	return stream.good() ||
		(stream.eof() && stream.gcount() == static_cast<std::streamsize>(byteCount));
}

static AnimationAssetResult AnimationAssetValidateChunkFile(
	const std::filesystem::path& path,
	const std::filesystem::path& assetRoot)
{
	AnimationAssetResult result;
	std::error_code relativeError;
	result.path = std::filesystem::relative(path, assetRoot, relativeError);
	if (relativeError)
		result.path = path;
	result.stage = "format";
	result.extension = AnimationAssetLowercase(path.extension().string());

	std::error_code sizeError;
	result.fileSize = std::filesystem::file_size(path, sizeError);
	if (sizeError)
	{
		result.message = "Unable to query file size: " + sizeError.message();
		return result;
	}
	if (result.fileSize < sizeof(FILE_HEADER))
	{
		result.message = "File is smaller than FILE_HEADER";
		return result;
	}

	std::ifstream stream(path, std::ios::binary);
	if (!stream)
	{
		result.message = "Unable to open file";
		return result;
	}

	FILE_HEADER fileHeader{};
	if (!AnimationAssetReadAt(stream, 0, &fileHeader, sizeof(fileHeader)))
	{
		result.message = "Unable to read FILE_HEADER";
		return result;
	}
	if (std::memcmp(fileHeader.Signature, FILE_SIGNATURE, sizeof(FILE_SIGNATURE)) != 0)
	{
		result.message = "CryTek file signature is invalid";
		return result;
	}
	if (fileHeader.Version != GeomFileVersion)
	{
		result.message = "Unsupported chunk-file version";
		return result;
	}
	if (fileHeader.FileType != FileType_Geom && fileHeader.FileType != FileType_Anim)
	{
		result.message = "Unsupported chunk-file type";
		return result;
	}
	if (fileHeader.ChunkTableOffset < 0)
	{
		result.message = "Negative chunk-table offset";
		return result;
	}

	const size_t chunkTableOffset = static_cast<size_t>(fileHeader.ChunkTableOffset);
	if (!CryIsRangeWithinSize(
		static_cast<size_t>(result.fileSize),
		chunkTableOffset,
		sizeof(int)))
	{
		result.message = "Chunk count lies outside the file";
		return result;
	}

	int chunkCount = 0;
	if (!AnimationAssetReadAt(stream, chunkTableOffset, &chunkCount, sizeof(chunkCount)))
	{
		result.message = "Unable to read chunk count";
		return result;
	}
	if (chunkCount <= 0 || chunkCount > 10000)
	{
		result.message = "Chunk count is outside the engine-supported range";
		return result;
	}
	result.chunkCount = static_cast<size_t>(chunkCount);

	size_t chunkHeadersOffset = 0;
	if (!CryTryAddSize(chunkTableOffset, sizeof(chunkCount), chunkHeadersOffset))
	{
		result.message = "Chunk-header offset overflow";
		return result;
	}
	size_t chunkHeadersByteCount = 0;
	if (!CryTryMultiplySize(result.chunkCount, sizeof(CHUNK_HEADER), chunkHeadersByteCount))
	{
		result.message = "Chunk-header byte-count overflow";
		return result;
	}
	size_t chunkHeadersEnd = 0;
	if (!CryTryAddSize(chunkHeadersOffset, chunkHeadersByteCount, chunkHeadersEnd) ||
		chunkHeadersEnd > result.fileSize)
	{
		result.message = "Chunk-header table lies outside the file";
		return result;
	}

	std::vector<CHUNK_HEADER> chunkHeaders(result.chunkCount);
	if (!AnimationAssetReadAt(
		stream,
		chunkHeadersOffset,
		chunkHeaders.data(),
		chunkHeadersByteCount))
	{
		result.message = "Unable to read chunk headers";
		return result;
	}

	const bool tablePrecedesChunks = chunkTableOffset == sizeof(FILE_HEADER);
	const size_t chunkDataBegin = tablePrecedesChunks
		? chunkHeadersEnd
		: sizeof(FILE_HEADER);
	const size_t chunkDataEnd = tablePrecedesChunks
		? static_cast<size_t>(result.fileSize)
		: chunkTableOffset;
	if (chunkDataEnd < chunkDataBegin)
	{
		result.message = "Chunk-data range is inverted";
		return result;
	}

	for (const CHUNK_HEADER& header : chunkHeaders)
	{
		if (header.FileOffset < 0 || header.ChunkID < 0)
		{
			result.message = "Chunk contains a negative offset or ID";
			return result;
		}
	}

	std::sort(
		chunkHeaders.begin(),
		chunkHeaders.end(),
		[](const CHUNK_HEADER& left, const CHUNK_HEADER& right)
		{
			return left.FileOffset < right.FileOffset;
		});
	for (size_t index = 0; index < chunkHeaders.size(); ++index)
	{
		const size_t chunkOffset = static_cast<size_t>(chunkHeaders[index].FileOffset);
		const size_t nextOffset = index + 1 < chunkHeaders.size()
			? static_cast<size_t>(chunkHeaders[index + 1].FileOffset)
			: chunkDataEnd;
		if (chunkOffset < chunkDataBegin ||
			nextOffset <= chunkOffset ||
			nextOffset > chunkDataEnd)
		{
			result.message = "Chunk offset or extent lies outside the chunk-data range";
			return result;
		}
	}

	std::sort(
		chunkHeaders.begin(),
		chunkHeaders.end(),
		[](const CHUNK_HEADER& left, const CHUNK_HEADER& right)
		{
			return left.ChunkID < right.ChunkID;
		});
	for (size_t index = 1; index < chunkHeaders.size(); ++index)
	{
		if (chunkHeaders[index - 1].ChunkID == chunkHeaders[index].ChunkID)
		{
			result.message = "Chunk IDs are not unique";
			return result;
		}
	}

	result.passed = true;
	result.message = "Chunk table and chunk extents are valid";
	return result;
}

static void AnimationAssetPrintSummary(
	const char* stage,
	const std::map<std::string, AnimationAssetCounts>& counts)
{
	std::printf("\n%s asset coverage:\n", stage);
	for (const auto& [extension, extensionCounts] : counts)
	{
		std::printf(
			"  %-4s %zu passed, %zu failed\n",
			extension.c_str(),
			extensionCounts.passed,
			extensionCounts.failed);
	}
}

static int AnimationAssetRunFormatCorpusInternal(
	const std::filesystem::path& assetRoot,
	const std::filesystem::path& reportPath)
{
	std::string discoveryError;
	const std::vector<std::filesystem::path> assets =
		AnimationAssetDiscoverCorpus(assetRoot, discoveryError);
	if (!discoveryError.empty())
	{
		std::fprintf(stderr, "Animation asset discovery failed: %s\n", discoveryError.c_str());
		return 2;
	}

	std::ofstream report(reportPath, std::ios::out | std::ios::trunc);
	if (!report)
	{
		std::fprintf(stderr, "Unable to create report: %s\n", reportPath.string().c_str());
		return 2;
	}
	AnimationAssetWriteReportHeader(report);

	std::map<std::string, AnimationAssetCounts> counts;
	for (size_t index = 0; index < assets.size(); ++index)
	{
		AnimationAssetResult result = AnimationAssetValidateChunkFile(assets[index], assetRoot);
		AnimationAssetCounts& extensionCounts = counts[result.extension];
		if (result.passed)
			++extensionCounts.passed;
		else
		{
			++extensionCounts.failed;
			std::fprintf(
				stderr,
				"[FAIL] format %s: %s\n",
				result.path.string().c_str(),
				result.message.c_str());
		}
		AnimationAssetWriteResult(report, result);

		if ((index + 1) % 100 == 0 || index + 1 == assets.size())
			std::printf("Validated chunk format for %zu/%zu assets\n", index + 1, assets.size());
	}

	AnimationAssetPrintSummary("Format", counts);
	for (const auto& [extension, extensionCounts] : counts)
	{
		if (extensionCounts.failed != 0)
			return 1;
	}
	return 0;
}

static bool AnimationAssetEqualsCaseInsensitive(
	const std::filesystem::path& value,
	const wchar_t* expected)
{
	return _wcsicmp(value.c_str(), expected) == 0;
}

static std::string AnimationAssetWideToNarrow(const std::wstring& value)
{
	if (value.empty())
		return {};
	const int required = WideCharToMultiByte(
		CP_ACP,
		0,
		value.data(),
		static_cast<int>(value.size()),
		nullptr,
		0,
		nullptr,
		nullptr);
	if (required <= 0)
		return {};

	std::string converted(static_cast<size_t>(required), '\0');
	WideCharToMultiByte(
		CP_ACP,
		0,
		value.data(),
		static_cast<int>(value.size()),
		converted.data(),
		required,
		nullptr,
		nullptr);
	return converted;
}

static bool AnimationAssetSetDllDirectory(const wchar_t* directory)
{
	using SetDllDirectoryFunction = BOOL (WINAPI*)(LPCWSTR pathName);
	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (kernel32 == nullptr)
		return false;
	const auto setDllDirectory = reinterpret_cast<SetDllDirectoryFunction>(
		GetProcAddress(kernel32, "SetDllDirectoryW"));
	return setDllDirectory != nullptr && setDllDirectory(directory) != FALSE;
}

static std::string AnimationAssetBuildSystemCommandLine(
	const std::filesystem::path& gameRoot,
	const std::filesystem::path& assetRoot)
{
	std::string commandLine = "-devmode -DX10 -AssetCompatibilityTest";
	std::error_code relativeError;
	const std::filesystem::path relativeRoot =
		std::filesystem::relative(assetRoot, gameRoot, relativeError);
	if (relativeError)
		return commandLine;

	auto component = relativeRoot.begin();
	if (component == relativeRoot.end() ||
		!AnimationAssetEqualsCaseInsensitive(*component, L"Mods"))
	{
		return commandLine;
	}
	++component;
	if (component == relativeRoot.end())
		return commandLine;
	const std::string modName = AnimationAssetWideToNarrow(component->wstring());
	if (!modName.empty())
		commandLine += " -MOD " + modName;
	return commandLine;
}

static bool AnimationAssetInitializeRuntime(
	const std::filesystem::path& runtimeDirectory,
	const std::filesystem::path& gameRoot,
	const std::filesystem::path& assetRoot,
	AnimationRuntimeState& state,
	std::string& error)
{
	std::error_code statusError;
	if (!std::filesystem::is_directory(runtimeDirectory, statusError))
	{
		error = "Runtime directory is not accessible";
		return false;
	}
	if (!std::filesystem::is_directory(gameRoot, statusError))
	{
		error = "Game root is not accessible";
		return false;
	}

	state.previousWorkingDirectory = std::filesystem::current_path(statusError);
	if (statusError || !SetCurrentDirectoryW(gameRoot.c_str()))
	{
		error = "Unable to set the game working directory";
		return false;
	}
	if (!AnimationAssetSetDllDirectory(runtimeDirectory.c_str()))
	{
		error = "Unable to set the runtime DLL directory";
		return false;
	}

	const std::filesystem::path crySystemPath = runtimeDirectory / L"CrySystem.dll";
	state.crySystemModule = LoadLibraryExW(
		crySystemPath.c_str(),
		nullptr,
		LOAD_WITH_ALTERED_SEARCH_PATH);
	if (state.crySystemModule == nullptr)
	{
		error = "Unable to load CrySystem.dll, Windows error " + std::to_string(GetLastError());
		return false;
	}

	const auto createSystem = reinterpret_cast<AnimationRuntimeState::CreateSystemFunction>(
		GetProcAddress(state.crySystemModule, "CreateSystemInterface"));
	if (createSystem == nullptr)
	{
		error = "CrySystem.dll does not export CreateSystemInterface";
		return false;
	}
	state.testWindow = CreateWindowExW(
		0,
		L"STATIC",
		L"CryCommonTests asset compatibility",
		WS_POPUP,
		0,
		0,
		64,
		64,
		nullptr,
		nullptr,
		GetModuleHandleW(nullptr),
		nullptr);
	if (state.testWindow == nullptr)
	{
		error = "Unable to create the hidden renderer test window";
		return false;
	}

	SSystemInitParams startupParams;
	std::memset(&startupParams, 0, sizeof(startupParams));
	startupParams.hInstance = GetModuleHandleW(nullptr);
	startupParams.hWnd = state.testWindow;
	startupParams.sLogFileName = "AssetCompatibility.log";
	startupParams.pUserCallback = &state.systemCallback;
	startupParams.bPreview = true;
	startupParams.bTestMode = true;
	startupParams.bDedicatedServer = false;
	const std::string commandLine = AnimationAssetBuildSystemCommandLine(gameRoot, assetRoot);
	strncpy_s(
		startupParams.szSystemCmdLine,
		sizeof(startupParams.szSystemCmdLine),
		commandLine.c_str(),
		_TRUNCATE);

	state.system = createSystem(startupParams);
	if (state.system == nullptr)
	{
		error = "CreateSystemInterface failed; inspect AssetCompatibility.log";
		return false;
	}
	state.characterManager = state.system->GetIAnimationSystem();
	state.engine3D = state.system->GetI3DEngine();
	if (state.characterManager == nullptr || state.engine3D == nullptr)
	{
		error = "CrySystem did not initialize CryAnimation and Cry3DEngine";
		return false;
	}

	HMODULE cryAnimationModule = GetModuleHandleW(L"CryAnimation.dll");
	if (cryAnimationModule == nullptr)
	{
		error = "CryAnimation.dll is not loaded";
		return false;
	}
	state.validateAnimationAsset =
		reinterpret_cast<AnimationRuntimeState::ValidateAnimationAssetFunction>(
			GetProcAddress(cryAnimationModule, "CryValidateAnimationAsset"));
	if (state.validateAnimationAsset == nullptr)
	{
		error = "CryAnimation.dll does not export CryValidateAnimationAsset";
		return false;
	}
	return true;
}

static void AnimationAssetShutdownRuntime(AnimationRuntimeState& state)
{
	if (state.system != nullptr)
	{
		state.system->Release();
		state.system = nullptr;
	}
	state.characterManager = nullptr;
	state.engine3D = nullptr;
	state.validateAnimationAsset = nullptr;
	if (state.crySystemModule != nullptr)
	{
		FreeLibrary(state.crySystemModule);
		state.crySystemModule = nullptr;
	}
	if (state.testWindow != nullptr)
	{
		DestroyWindow(state.testWindow);
		state.testWindow = nullptr;
	}
	AnimationAssetSetDllDirectory(nullptr);
	if (!state.previousWorkingDirectory.empty())
		SetCurrentDirectoryW(state.previousWorkingDirectory.c_str());
}

static AnimationAssetResult AnimationAssetRunRuntimeProbe(
	const std::filesystem::path& path,
	const std::filesystem::path& assetRoot,
	AnimationRuntimeState& state)
{
	AnimationAssetResult result;
	std::error_code relativeError;
	result.path = std::filesystem::relative(path, assetRoot, relativeError);
	if (relativeError)
		result.path = path;
	result.stage = "runtime";
	result.extension = AnimationAssetLowercase(path.extension().string());
	std::error_code sizeError;
	result.fileSize = std::filesystem::file_size(path, sizeError);

	const AnimationAssetResult formatResult = AnimationAssetValidateChunkFile(path, assetRoot);
	result.chunkCount = formatResult.chunkCount;
	if (!formatResult.passed)
	{
		result.message = "Runtime probe skipped after format failure: " + formatResult.message;
		return result;
	}

	const std::string enginePath = result.path.generic_string();
	if (result.extension == ".chr" || result.extension == ".cga")
	{
		ICharacterInstance* instance =
			state.characterManager->CreateInstance(enginePath.c_str());
		if (instance == nullptr)
		{
			result.message = "ICharacterManager::CreateInstance returned null";
			return result;
		}

		ISkeletonPose* skeletonPose = instance->GetISkeletonPose();
		IAnimationSet* animationSet = instance->GetIAnimationSet();
		if (skeletonPose != nullptr)
			result.jointCount = skeletonPose->GetJointCount();
		if (animationSet != nullptr)
			result.animationCount = animationSet->numAnimations();
		result.passed = skeletonPose != nullptr && animationSet != nullptr;
		result.message = result.passed
			? "Character instance, skeleton, and animation set loaded"
			: "Character instance did not expose its skeleton or animation set";
		instance->Release();
		return result;
	}

	if (result.extension == ".cgf")
	{
		IStatObj* statObject = state.engine3D->LoadStatObj(enginePath.c_str());
		if (statObject == nullptr)
		{
			result.message = "I3DEngine::LoadStatObj returned null";
			return result;
		}
		result.passed = true;
		result.message = "Static object loaded";
		statObject->Release();
		return result;
	}

	if (result.extension == ".caf" || result.extension == ".anm")
	{
		char loaderError[1024] = {};
		result.passed = state.validateAnimationAsset(
			enginePath.c_str(),
			loaderError,
			sizeof(loaderError));
		result.message = result.passed
			? "CryAnimation controller/timing chunks loaded"
			: loaderError;
		if (!result.passed && result.message.empty())
			result.message = "CryAnimation asset loader returned false without a diagnostic";
		return result;
	}

	result.message = "Unsupported runtime asset extension";
	return result;
}

static void AnimationAssetWriteCheckpoint(
	const std::filesystem::path& checkpointPath,
	const std::filesystem::path& assetPath)
{
	std::ofstream checkpoint(checkpointPath, std::ios::out | std::ios::trunc);
	if (checkpoint)
		checkpoint << AnimationAssetPathForReport(assetPath) << '\n';
}

static std::wstring AnimationAssetQuoteCommandArgument(const std::filesystem::path& argument)
{
	std::wstring value = argument.wstring();
	std::wstring quoted = L"\"";
	for (wchar_t character : value)
	{
		if (character == L'\"')
			quoted += L'\\';
		quoted += character;
	}
	quoted += L'\"';
	return quoted;
}

static int AnimationAssetLaunchRuntimeChild(
	const std::filesystem::path& runtimeDirectory,
	const std::filesystem::path& gameRoot,
	const std::filesystem::path& assetRoot,
	const std::filesystem::path& reportPath,
	const std::filesystem::path& checkpointPath)
{
	std::vector<wchar_t> executablePath(32768, L'\0');
	const DWORD executableLength = GetModuleFileNameW(
		nullptr,
		executablePath.data(),
		static_cast<DWORD>(executablePath.size()));
	if (executableLength == 0 || executableLength >= executablePath.size())
	{
		std::fprintf(stderr, "Unable to resolve CryCommonTests executable path\n");
		return 2;
	}

	const std::filesystem::path executable(executablePath.data());
	std::wstring commandLine = AnimationAssetQuoteCommandArgument(executable);
	commandLine += L" --animation-assets-runtime ";
	commandLine += AnimationAssetQuoteCommandArgument(runtimeDirectory) + L" ";
	commandLine += AnimationAssetQuoteCommandArgument(gameRoot) + L" ";
	commandLine += AnimationAssetQuoteCommandArgument(assetRoot) + L" ";
	commandLine += AnimationAssetQuoteCommandArgument(reportPath) + L" ";
	commandLine += AnimationAssetQuoteCommandArgument(checkpointPath);

	STARTUPINFOW startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInfo{};
	if (!CreateProcessW(
		executable.c_str(),
		commandLine.data(),
		nullptr,
		nullptr,
		FALSE,
		0,
		nullptr,
		nullptr,
		&startupInfo,
		&processInfo))
	{
		std::fprintf(
			stderr,
			"Unable to launch runtime asset probe, Windows error %lu\n",
			GetLastError());
		return 2;
	}

	WaitForSingleObject(processInfo.hProcess, INFINITE);
	DWORD exitCode = 2;
	GetExitCodeProcess(processInfo.hProcess, &exitCode);
	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);
	if (exitCode != 0)
	{
		std::ifstream checkpoint(checkpointPath);
		std::string lastAsset;
		std::getline(checkpoint, lastAsset);
		if (!lastAsset.empty())
			std::fprintf(stderr, "Runtime probe stopped while processing: %s\n", lastAsset.c_str());
	}
	return exitCode == 0 ? 0 : 1;
}

int RunAnimationAssetFormatCorpus(
	const wchar_t* assetRoot,
	const wchar_t* reportPath)
{
	if (assetRoot == nullptr || reportPath == nullptr)
		return 2;
	return AnimationAssetRunFormatCorpusInternal(assetRoot, reportPath);
}

int RunAnimationAssetRuntimeCorpus(
	const wchar_t* runtimeDirectory,
	const wchar_t* gameRoot,
	const wchar_t* assetRoot,
	const wchar_t* reportPath,
	const wchar_t* checkpointPath)
{
	if (runtimeDirectory == nullptr ||
		gameRoot == nullptr ||
		assetRoot == nullptr ||
		reportPath == nullptr ||
		checkpointPath == nullptr)
	{
		return 2;
	}

	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	SetEnvironmentVariableW(L"LINES", L"25");
	SetEnvironmentVariableW(L"COLUMNS", L"120");
	std::string discoveryError;
	const std::vector<std::filesystem::path> assets =
		AnimationAssetDiscoverCorpus(assetRoot, discoveryError);
	if (!discoveryError.empty())
	{
		std::fprintf(stderr, "Animation asset discovery failed: %s\n", discoveryError.c_str());
		return 2;
	}

	std::ofstream report(reportPath, std::ios::out | std::ios::app);
	if (!report)
	{
		std::fprintf(stderr, "Unable to append runtime results to the report\n");
		return 2;
	}

	AnimationRuntimeState runtime;
	std::string initializationError;
	if (!AnimationAssetInitializeRuntime(
		runtimeDirectory,
		gameRoot,
		assetRoot,
		runtime,
		initializationError))
	{
		AnimationAssetResult result;
		result.stage = "runtime-init";
		result.message = initializationError;
		AnimationAssetWriteResult(report, result);
		std::fprintf(stderr, "Runtime initialization failed: %s\n", initializationError.c_str());
		AnimationAssetShutdownRuntime(runtime);
		return 2;
	}

	std::map<std::string, AnimationAssetCounts> counts;
	for (size_t index = 0; index < assets.size(); ++index)
	{
		std::error_code relativeError;
		const std::filesystem::path relativePath =
			std::filesystem::relative(assets[index], assetRoot, relativeError);
		AnimationAssetWriteCheckpoint(
			checkpointPath,
			relativeError ? assets[index] : relativePath);

		AnimationAssetResult result = AnimationAssetRunRuntimeProbe(
			assets[index],
			assetRoot,
			runtime);
		AnimationAssetCounts& extensionCounts = counts[result.extension];
		if (result.passed)
			++extensionCounts.passed;
		else
		{
			++extensionCounts.failed;
			std::fprintf(
				stderr,
				"[FAIL] runtime %s: %s\n",
				result.path.string().c_str(),
				result.message.c_str());
		}
		AnimationAssetWriteResult(report, result);

		if ((index + 1) % 25 == 0 || index + 1 == assets.size())
			std::printf("Runtime-loaded %zu/%zu assets\n", index + 1, assets.size());
	}

	std::error_code removeError;
	std::filesystem::remove(checkpointPath, removeError);
	AnimationAssetPrintSummary("Runtime", counts);

	int resultCode = 0;
	for (const auto& [extension, extensionCounts] : counts)
	{
		if (extensionCounts.failed != 0)
			resultCode = 1;
	}

	AnimationAssetResult teardownResult;
	teardownResult.stage = "runtime-teardown";
	teardownResult.passed = true;
	teardownResult.message =
		"Engine teardown is delegated to isolated process termination; individual asset handles were released";
	AnimationAssetWriteResult(report, teardownResult);
	std::fflush(nullptr);
	TerminateProcess(GetCurrentProcess(), static_cast<UINT>(resultCode));
	return resultCode;
}

int RunAnimationAssetCompatibilityPipeline(
	const wchar_t* runtimeDirectory,
	const wchar_t* gameRoot,
	const wchar_t* assetRoot,
	const wchar_t* reportPath)
{
	if (runtimeDirectory == nullptr ||
		gameRoot == nullptr ||
		assetRoot == nullptr ||
		reportPath == nullptr)
	{
		return 2;
	}

	const std::filesystem::path absoluteReport =
		std::filesystem::absolute(reportPath).lexically_normal();
	const int formatResult = AnimationAssetRunFormatCorpusInternal(assetRoot, absoluteReport);
	if (formatResult == 2)
		return formatResult;

	std::filesystem::path checkpointPath = absoluteReport;
	checkpointPath += L".current";
	const int runtimeResult = AnimationAssetLaunchRuntimeChild(
		runtimeDirectory,
		gameRoot,
		assetRoot,
		absoluteReport,
		checkpointPath);

	std::printf("\nAsset compatibility report: %s\n", absoluteReport.string().c_str());
	return formatResult == 0 && runtimeResult == 0 ? 0 : 1;
}
