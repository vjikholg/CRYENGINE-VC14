#pragma once

#include <ISystem.h>
#include "CryActionRegressionBridge.h"
#include "TestRunner.h"
#include <filesystem>
#include <fstream>

class CryActionRegressionHost : public ISystemUserCallback
{
public:
    explicit CryActionRegressionHost(const std::filesystem::path& detailPath);
    bool Initialize(const std::filesystem::path& runtimeDirectory, const std::filesystem::path& gameRoot, bool needsSystem, bool needsGraphics = false);
    bool InitializeEntitySystem(const std::filesystem::path& runtimeDirectory);
    int Run(const CryActionRegressionCaseInfo& testCase);
    bool OnError(const char*) override;
    void OnSaveDocument() override;
    void OnProcessSwitch() override;
    void OnInitProgress(const char*) override;
    void GetMemoryUsage(ICrySizer*) override;

private:
    static void Check(void*, std::uint32_t, const char*, const char*, int);
    static void RunBridge(CryTestRunner&, CryActionRegressionHost&);
    void Stage(const char*);
    CryTestRunner m_runner;
    std::ofstream m_detail;
    std::string m_engineLogPath;
    HMODULE m_module = nullptr;
    ISystem* m_system = nullptr;
    CryActionRegressionRun m_run = nullptr;
    const CryActionRegressionCaseInfo* m_case = nullptr;
    std::uint32_t m_bridgeResult = 2;
    bool m_prerequisite = false;
};
