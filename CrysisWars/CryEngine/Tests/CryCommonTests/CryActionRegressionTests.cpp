#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "CryActionRegressionTests.h"
#include "CryActionRegressionHost.h"
#include <windows.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>
#include <vector>
#include <chrono>
#include <IConsole.h>

CryActionRegressionHost::CryActionRegressionHost(const std::filesystem::path& detailPath)
    : m_detail(detailPath, std::ios::trunc)
    , m_engineLogPath(detailPath.string() + ".engine.log")
{
}

void CryActionRegressionHost::Stage(const char* stage)
{
    m_detail << "STAGE\t" << stage << '\n';
    m_detail.flush();
}

bool CryActionRegressionHost::Initialize(
    const std::filesystem::path& runtimeDirectory, const std::filesystem::path& gameRoot, bool needsSystem, bool needsGraphics)
{
    if (!m_detail)
        return false;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    Stage("load");
    using SetDirectory = BOOL (WINAPI*)(LPCWSTR);
    const auto setDirectory = reinterpret_cast<SetDirectory>(
        GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetDllDirectoryW"));
    if (setDirectory == nullptr || !setDirectory(runtimeDirectory.c_str()))
        return false;
    const auto crySystem = LoadLibraryExW((runtimeDirectory / L"CrySystem.dll").c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (crySystem == nullptr)
    {
        m_detail << "ERROR\tLoad CrySystem\t" << GetLastError() << '\n';
        return false;
    }
    if (needsSystem)
    {
        Stage("initialize-system");
        if (!SetCurrentDirectoryW(gameRoot.c_str()))
            return false;
        using CreateSystem = ISystem* (*)(const SSystemInitParams&);
        const auto createSystem = reinterpret_cast<CreateSystem>(GetProcAddress(crySystem, "CreateSystemInterface"));
        if (createSystem == nullptr)
            return false;
        const HWND window = CreateWindowExW(0, L"STATIC", L"CryAction regression", WS_POPUP,
            0, 0, 64, 64, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (window == nullptr)
            return false;
        SSystemInitParams parameters;
        std::memset(&parameters, 0, sizeof(parameters));
        parameters.hInstance = GetModuleHandleW(nullptr);
        parameters.hWnd = window;
        parameters.pUserCallback = this;
        parameters.bPreview = true;
        parameters.bTestMode = true;
        parameters.sLogFileName = m_engineLogPath.c_str();
        // CrySystem already recognizes this marker to disable its interactive crash handler.
        strcpy_s(parameters.szSystemCmdLine, needsGraphics ?
            "-devmode -DX10 -AssetCompatibilityTest -CryActionRegressionTest" :
            "-devmode -DX10 -PhysicsRegressionTest -CryActionRegressionTest");
        m_system = createSystem(parameters);
        if (m_system == nullptr)
        {
            Stage("system-unavailable");
            m_detail << "PREREQUISITE\t" << (needsGraphics ? "Runtime renderer/3D preview initialization failed; inspect INIT/ERROR observations" : "CrySystem preview initialization failed") << '\n';
            return false;
        }
        if (auto* noDialog = m_system->GetIConsole()->GetCVar("sys_no_crash_dialog"))
            noDialog->Set(1);
    }
    Stage("load-probe");
    const auto modulePath = runtimeDirectory / L"CryActionRegression.dll";
    m_module = LoadLibraryExW(modulePath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (m_module == nullptr)
    {
        m_detail << "ERROR\tLoad CryActionRegression\t" << GetLastError() << '\n';
        return false;
    }
    wchar_t loadedPath[32768]{};
    if (GetModuleFileNameW(m_module, loadedPath, static_cast<DWORD>(std::size(loadedPath))) == 0)
        return false;
    std::error_code error;
    if (!std::filesystem::equivalent(modulePath, loadedPath, error) || error)
        return false;
    m_detail << "MODULE\t" << std::filesystem::path(loadedPath).string() << '\n';
    m_run = reinterpret_cast<CryActionRegressionRun>(GetProcAddress(m_module, "CryActionRunRegressionCase"));
    return m_run != nullptr;
}

void CryActionRegressionHost::Check(
    void* raw, std::uint32_t condition, const char* expression, const char* source, int line)
{
    auto& host = *static_cast<CryActionRegressionHost*>(raw);
    if (condition == 3)
    {
        host.m_detail << "OBSERVE\t" << expression << '\n';
        host.m_detail.flush();
        return;
    }
    if (condition == 2)
    {
        host.m_prerequisite = true;
        host.m_detail << "PREREQUISITE\t" << expression << '\n';
        host.m_detail.flush();
        return;
    }
    host.m_detail << "CHECK\t" << (condition ? "PASS" : "FAIL") << '\t'
        << expression << '\t' << source << ':' << line << '\n';
    host.m_detail.flush();
    host.m_runner.Check(condition != 0, expression, source, line);
}

bool CryActionRegressionHost::InitializeEntitySystem(const std::filesystem::path& runtimeDirectory)
{
    if (!m_system) return false;
    auto* environment = m_system->GetGlobalEnvironment();
    Stage("initialize-entity-collaborators");
    if (!environment->pScriptSystem)
    {
        const auto module = LoadLibraryExW((runtimeDirectory / L"CryScriptSystem.dll").c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        using CreateScript = IScriptSystem* (*)(ISystem*, bool);
        const auto create = module ? reinterpret_cast<CreateScript>(GetProcAddress(module, "CreateScriptSystem")) : nullptr;
        if (!create) { m_detail << "PREREQUISITE\tCryScriptSystem factory unavailable\n"; return false; }
        environment->pScriptSystem = create(m_system, true);
        if (!environment->pScriptSystem) { m_detail << "PREREQUISITE\tCryScriptSystem initialization failed\n"; return false; }
    }
    if (!environment->pEntitySystem)
    {
        const auto module = LoadLibraryExW((runtimeDirectory / L"CryEntitySystem.dll").c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        using CreateEntities = IEntitySystem* (*)(ISystem*);
        const auto create = module ? reinterpret_cast<CreateEntities>(GetProcAddress(module, "CreateEntitySystem")) : nullptr;
        if (!create) { m_detail << "PREREQUISITE\tCryEntitySystem factory unavailable\n"; return false; }
        environment->pEntitySystem = create(m_system);
    }
    if (!environment->pEntitySystem) { m_detail << "PREREQUISITE\tCryEntitySystem initialization failed\n"; return false; }
    return true;
}

void CryActionRegressionHost::RunBridge(CryTestRunner&, CryActionRegressionHost& host)
{
    host.m_bridgeResult = host.m_run(CryActionRegressionAbi, host.m_case->id,
        host.m_system, Check, &host);
}

int CryActionRegressionHost::Run(const CryActionRegressionCaseInfo& testCase)
{
    m_case = &testCase;
    Stage("invoke");
    m_runner.Run(testCase.name, RunBridge, *this);
    if (m_bridgeResult != 0 || m_prerequisite)
        return 2;
    const int result = m_runner.ExitCode();
    m_detail << "COMPLETE\t" << result << '\t' << m_runner.FailureCount() << '\n';
    m_detail.flush();
    return result;
}

bool CryActionRegressionHost::OnError(const char* text)
{
    m_detail << "ERROR\t" << (text ? text : "<null>") << '\n';
    m_detail.flush();
    return false;
}
void CryActionRegressionHost::OnSaveDocument() {}
void CryActionRegressionHost::OnProcessSwitch() {}
void CryActionRegressionHost::OnInitProgress(const char* text)
{
    m_detail << "INIT\t" << (text ? text : "") << '\n';
    m_detail.flush();
}
void CryActionRegressionHost::GetMemoryUsage(ICrySizer*) {}

static const CryActionRegressionCaseInfo* FindActionCase(const char* name)
{
    for (const auto& testCase : CryActionRegressionCases)
        if (std::strcmp(name, testCase.name) == 0)
            return &testCase;
    return nullptr;
}

int RunCryActionRegressionCase(
    const wchar_t* runtimeDirectory, const wchar_t* gameRoot, const char* caseName, const wchar_t* detailPath)
{
    const auto* testCase = FindActionCase(caseName);
    if (testCase == nullptr)
        return 2;
    CryActionRegressionHost host(detailPath);
    if (!host.Initialize(runtimeDirectory, gameRoot, testCase->needsSystem,
        testCase->id == CryActionRegressionCase::UiOffscreen || testCase->id == CryActionRegressionCase::BreakChunks))
        return 2;
    if (testCase->id >= CryActionRegressionCase::FlowRegistration && testCase->id <= CryActionRegressionCase::FlowGraphCleanup ||
        testCase->id >= CryActionRegressionCase::MovementNoCharacter)
        if (!host.InitializeEntitySystem(runtimeDirectory)) return 2;
    return host.Run(*testCase);
}

static std::wstring QuoteActionArgument(const std::wstring& argument)
{
    // CommandLineToArgvW rules, including paths ending in a backslash.
    std::wstring result = L"\"";
    size_t slashes = 0;
    for (const wchar_t ch : argument)
    {
        if (ch == L'\\') { ++slashes; continue; }
        result.append(ch == L'\"' ? 2 * slashes + 1 : slashes, L'\\');
        slashes = 0;
        result += ch;
    }
    result.append(2 * slashes, L'\\');
    return result + L'\"';
}

int RunCryActionRegressionPipeline(
    const wchar_t* runtimeDirectory, const wchar_t* gameRoot, const wchar_t* reportPath, const char* family)
{
    if (family)
    {
        bool knownFamily = false;
        for (const auto& testCase : CryActionRegressionCases)
            knownFamily |= std::strcmp(family, testCase.family) == 0;
        if (!knownFamily)
        {
            std::fprintf(stderr, "No implemented CryAction cases for family %s\n", family);
            return 2;
        }
    }
    const auto runtime = std::filesystem::absolute(runtimeDirectory).lexically_normal();
    const auto game = std::filesystem::absolute(gameRoot).lexically_normal();
    const auto report = std::filesystem::absolute(reportPath).lexically_normal();
    std::filesystem::create_directories(report.parent_path());
    std::ofstream output(report, std::ios::trunc);
    if (!output)
        return 2;
    output << "case\tfamily\tstatus\texit_code\tchecks\tfailed_checks\tdefect_case\tstage\tpid\tasan_report\tcomplete\telapsed_ms\tprocess_outcome\tprerequisite\n";
    wchar_t sanitizerPrefix[32768]{};
    GetEnvironmentVariableW(L"CRY_ACTION_ASAN_LOG_PREFIX", sanitizerPrefix, static_cast<DWORD>(std::size(sanitizerPrefix)));
    wchar_t originalAsanOptions[32768]{};
    GetEnvironmentVariableW(L"ASAN_OPTIONS", originalAsanOptions, static_cast<DWORD>(std::size(originalAsanOptions)));
    wchar_t executable[32768]{};
    const DWORD pathLength = GetModuleFileNameW(nullptr, executable, static_cast<DWORD>(std::size(executable)));
    if (pathLength == 0 || pathLength >= std::size(executable))
        return 2;
    unsigned passed = 0, failed = 0, crashed = 0, blocked = 0, timedOut = 0, sanitizerReports = 0;
    for (const auto& testCase : CryActionRegressionCases)
    {
        if (family && std::strcmp(family, testCase.family) != 0)
            continue;
        const auto detail = std::filesystem::path(report.wstring() + L"." +
            std::to_wstring(static_cast<unsigned>(testCase.id)) + L".log");
        std::string status;
        std::string stage = "not-started";
        DWORD exitCode = 2;
        DWORD processId = 0;
        std::string sanitizerReport;
        bool timeout = false;
        bool complete = false;
        unsigned checks = 0, failedChecks = 0;
        const auto started = std::chrono::steady_clock::now();
        std::string prerequisite;
        // Windows may reuse a PID before this pipeline finishes. A PID-only log
        // name can make a passing child inherit another child's old diagnosis.
        const std::wstring caseSanitizerPrefix = sanitizerPrefix[0] ?
            std::wstring(sanitizerPrefix) + L"-case-" + std::to_wstring(static_cast<unsigned>(testCase.id)) : L"";
        {
            std::error_code removeError;
            std::filesystem::remove(detail, removeError);
            if (removeError)
            {
                output << testCase.name << '\t' << testCase.family
                    << "\tBLOCKED\t2\t0\t0\t" << testCase.defectCase << "\tstale-detail-file\t0\t\t0\t0\tnot-started\tCannot remove stale detail file\n";
                ++blocked;
                continue;
            }
            const std::wstring name(testCase.name, testCase.name + std::strlen(testCase.name));
            std::wstring command = QuoteActionArgument(executable) + L" --cryaction-regression-case " +
                QuoteActionArgument(runtime.wstring()) + L" " + QuoteActionArgument(game.wstring()) + L" " +
                QuoteActionArgument(name) + L" " + QuoteActionArgument(detail.wstring());
            STARTUPINFOW startup{};
            startup.cb = sizeof(startup);
            PROCESS_INFORMATION process{};
            if (!caseSanitizerPrefix.empty())
            {
                const std::wstring options = std::wstring(originalAsanOptions) + L":log_path=\"" + caseSanitizerPrefix + L"\"";
                if (!SetEnvironmentVariableW(L"ASAN_OPTIONS", options.c_str()))
                    return 2;
            }
            const BOOL created = CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                nullptr, nullptr, &startup, &process);
            if (!caseSanitizerPrefix.empty())
                SetEnvironmentVariableW(L"ASAN_OPTIONS", originalAsanOptions[0] ? originalAsanOptions : nullptr);
            if (created)
            {
                processId = process.dwProcessId;
                const DWORD wait = WaitForSingleObject(process.hProcess, 120000);
                timeout = wait == WAIT_TIMEOUT;
                if (wait != WAIT_OBJECT_0)
                {
                    TerminateProcess(process.hProcess, 3);
                    WaitForSingleObject(process.hProcess, 5000);
                }
                GetExitCodeProcess(process.hProcess, &exitCode);
                CloseHandle(process.hThread);
                CloseHandle(process.hProcess);
                if (sanitizerPrefix[0])
                {
                    const auto diagnostic = std::filesystem::path(caseSanitizerPrefix + L"." + std::to_wstring(processId));
                    std::ifstream reportInput(diagnostic);
                    for (std::string line; std::getline(reportInput, line);)
                        if (line.find("ERROR: AddressSanitizer:") != std::string::npos)
                        {
                            sanitizerReport = diagnostic.string();
                            ++sanitizerReports;
                            break;
                        }
                }
            }
            else
                exitCode = 2;
            std::ifstream input(detail);
            for (std::string line; std::getline(input, line);)
            {
                if (line.starts_with("STAGE\t")) stage = line.substr(6);
                if (line.starts_with("COMPLETE\t")) complete = true;
                if (line.starts_with("CHECK\t")) ++checks;
                if (line.starts_with("CHECK\tFAIL\t")) ++failedChecks;
                if (line.starts_with("PREREQUISITE\t")) prerequisite = line.substr(13);
                if (line.starts_with("ERROR\t") && prerequisite.empty()) prerequisite = line.substr(6);
            }
        }
        if (timeout) { status = "TIMEOUT"; ++timedOut; }
        else if (exitCode >= 0x80000000u) { status = "CRASH"; ++crashed; }
        else if (exitCode == 2) { status = "BLOCKED"; ++blocked; }
        else if (exitCode != 0 || failedChecks != 0 || !complete || checks == 0 || !sanitizerReport.empty()) { status = "FAIL"; ++failed; }
        else { status = "PASS"; ++passed; }
        const char* processOutcome = timeout ? "timeout" : exitCode >= 0x80000000u ? "exception" :
            !sanitizerReport.empty() ? "sanitizer-termination" : exitCode == 2 ? "prerequisite" :
            !complete ? "incomplete-exit" : "completed";
        output << testCase.name << '\t' << testCase.family << '\t' << status << '\t'
            << exitCode << '\t' << checks << '\t' << failedChecks << '\t'
            << testCase.defectCase << '\t' << stage << '\t' << processId << '\t' << sanitizerReport << '\t'
            << complete << '\t' << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - started).count() << '\t' << processOutcome << '\t' << prerequisite << '\n';
        output.flush();
        std::printf("[%s] %s (%u checks, %u failed checks, exit 0x%08lx, stage %s)\n",
            status.c_str(), testCase.name, checks, failedChecks, exitCode, stage.c_str());
        std::fflush(stdout);
    }
    std::printf("CryAction: %u passed, %u failed, %u crashed, %u timed out, %u blocked\n",
        passed, failed, crashed, timedOut, blocked);
    std::printf("AddressSanitizer reports: %u (included in failure totals, not additional cases)\n", sanitizerReports);
    return failed || crashed || timedOut ? 1 : blocked ? 2 : 0;
}
