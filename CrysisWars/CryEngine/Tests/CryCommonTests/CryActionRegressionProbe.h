#pragma once

#include "CryActionRegressionBridge.h"
#include <ITimer.h>
#include <string>
#include <cstdio>

class CryActionRegressionProbe
{
public:
    static std::uint32_t Run(CryActionRegressionCase, CryActionRegressionCheck, void*);
    // Implemented beside GameContext's file-local production helpers, in test builds only.
    static void Hex(CryActionRegressionCase, CryActionRegressionCheck, void*);
private:
    static std::uint32_t BoundaryCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void MovementCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ExtensionCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void InspectorCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void BreakRepairCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void DiagnosticCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void WindowsCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void BreakCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ReportCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void UiCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void TextCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ReadBuffer(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void LinkLookup(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ServerList(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ParseServer(CryActionRegressionCheck, void*);
    static void StateIndex(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void Rotation(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void Dispatch(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void Thumbnail(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void Bitmap(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void Metadata(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ItemParams(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ModuleImage(CryActionRegressionCheck, void*);
    static void GraphXml(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void Inventory(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void LinkCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void LinkTransition(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ReaderCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ReaderStrings(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ReaderFiles(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static std::uint32_t RuntimeCases(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void Readability(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ViewShakes(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void DialogCounts(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void RichXml(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void GraphPaths(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void GraphLayers(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void GameplayCallbacks(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void HttpCallbacks(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void ServerLifecycle(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void FlowRegistry(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static void TimeDemoFixture(CryActionRegressionCase, CryActionRegressionCheck, void*);
    static std::string ScratchPath(const char*);
    static FILE* OpenFixture(const std::string&, const char*);
};

class CryActionRegressionTimer : public ITimer
{
public:
    void ResetTimer() override;
    void UpdateOnFrameStart() override;
    float GetCurrTime(ETimer) const override;
    CTimeValue GetFrameStartTime(ETimer) const override;
    CTimeValue GetAsyncTime() const override;
    float GetAsyncCurTime() override;
    float GetFrameTime(ETimer) const override;
    float GetRealFrameTime() const override;
    float GetTimeScale() const override;
    void SetTimeScale(float) override;
    void EnableTimer(bool) override;
    bool IsTimerEnabled() const override;
    float GetFrameRate() override;
    float GetProfileFrameBlending(float*, int*) override;
    void Serialize(TSerialize) override;
    bool PauseTimer(ETimer, bool) override;
    bool IsTimerPaused(ETimer) override;
    bool SetTimer(ETimer, float) override;
    void SecondsToDateUTC(time_t, tm&) override;
    time_t DateToSecondsUTC(tm&) override;
};

class CryActionRegressionPingSink : public ILanQueryListener
{
public:
    explicit CryActionRegressionPingSink(IGameQueryListener*);
    void DeleteNetQueryListener() override;
    void SendPingTo(const char*) override;
    IGameQueryListener* GetGameQueryListener() override;
    void GetMemoryStatistics(ICrySizer*) override;
    std::uint32_t PingCount() const;
private:
    IGameQueryListener* m_listener;
    std::uint32_t m_pings = 0;
};
