#pragma once

#include "CryActionBoundaryCollaborators.h"
#include "CryActionRegressionBridge.h"
#include <FlowSystem/Inspectors/FlowInspectorDefault.h>
#include <IGameObject.h>

class CCryAction;
class CUISystem;
class CGameObjectSystem;

// Owns the local subsystem wiring and restores both environment views before exit.
// This is a component fixture, not a claim of a complete playable framework boot.
class CryActionRegressionBoundaryFixture
{
public:
    CryActionRegressionBoundaryFixture(CryActionRegressionCheck, void*);
    ~CryActionRegressionBoundaryFixture();
    bool CreateFramework();
    bool CreateUI(bool useRuntimeRenderer = false);
    void DestroyUI();
    ISystem* system;
    CCryAction* framework = nullptr;
    CUISystem* ui = nullptr;
    CryActionRegressionRenderer renderer;
    CryActionRegressionInput input;
    CryActionRegressionFont font;
    CryActionRegressionFonts fonts;
    CryActionRegressionGame game;
    CryActionRegressionEngine3D engine;
    SSystemGlobalEnvironment savedEnvironment;
private:
    void Trace(const char*);
    CryActionRegressionCheck m_check;
    void* m_context;
    SSystemGlobalEnvironment m_savedSystemEnvironment;
    CGameObjectSystem* m_savedGameObjectSystem;
    std::string m_uiArchive;
    bool m_mounted = false;
    int m_pakPriority = 0;
    ICVar* m_priorityVariable = nullptr;
    std::string m_savedLocale;
};

class CryActionRegressionInspector : public CFlowInspectorDefault
{
public:
    explicit CryActionRegressionInspector(IFlowSystem*);
    void Draw(const std::deque<TFlowRecord>&, int, float, float);
    void DrawCurrent(int);
    const std::vector<TFlowRecord>& Current() const;
};

struct SJointBreakRec;
class CryActionRegressionExtension : public IGameObjectExtension
{
public:
    CryActionRegressionExtension(std::vector<std::string>&, bool);
    bool OnRegressionRmi(const SJointBreakRec&, INetChannel*);
    bool Init(IGameObject*) override;
    void PostInit(IGameObject*) override;
    void InitClient(int) override;
    void PostInitClient(int) override;
    void Release() override;
    void FullSerialize(TSerialize) override;
    bool NetSerialize(TSerialize, EEntityAspects, uint8, int) override;
    void PostSerialize() override;
    void SerializeSpawnInfo(TSerialize) override;
    ISerializableInfoPtr GetSpawnInfo() override;
    void Update(SEntityUpdateContext&, int) override;
    void HandleEvent(const SGameObjectEvent&) override;
    void ProcessEvent(SEntityEvent&) override;
    void GetMemoryStatistics(ICrySizer*) override;
    void SetChannelId(uint16) override;
    void SetAuthority(bool) override;
    const void* GetRMIBase() const override;
    void PostUpdate(float) override;
    void PostRemoteSpawn() override;
private:
    std::vector<std::string>& m_events;
    bool m_succeeds;
};

class CryActionRegressionExtensionFactory : public IGameObjectExtensionCreatorBase
{
public:
    IGameObjectExtension* Create() override;
    void GetGameObjectExtensionRMIData(void**, size_t*) override;
    std::vector<std::string> events;
    bool createSucceeds = true, initSucceeds = true;
};
