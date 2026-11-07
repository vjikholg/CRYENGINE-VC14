#include "StdAfx.h"
#include "CryActionRegressionProbe.h"
#include "CryActionRuntimeRegression.h"
#include "CryActionBoundaryRegression.h"
#include <AIHandler.h>
#include <CryAction.h>
#include <ActorSystem.h>
#include <GameplayRecorder/GameplayRecorder.h>
#include <GameObjects/GameObject.h>
#include <GameObjects/GameObjectSystem.h>
#include <LevelSystem.h>
#include <Network/GameStats.h>
#include <Network/NetworkCVars.h>
#include <Network/JointBreak.h>
#include <Network/ProceduralBreakingPlaybackListener.h>
#include <Network/ProceduralBreakingRecordingListener.h>
#include <FlowSystem/FlowSystem.h>
#include <UISystem.h>
#include <ScriptBind_UI.h>
#include <algorithm>
#include <cmath>
#include <clocale>
#include <limits>
#include <memory>
#if defined(__SANITIZE_ADDRESS__)
#include <sanitizer/asan_interface.h>
#endif

#define BOUNDARY_CHECK(expression) check(context, (expression) ? 1u : 0u, #expression, __FILE__, __LINE__)
#define BOUNDARY_REQUIRE(expression, reason) if (!(expression)) { check(context, 2, reason, __FILE__, __LINE__); return; }

CryActionRegressionBoundaryFixture::CryActionRegressionBoundaryFixture(CryActionRegressionCheck check, void* context)
    : system(gEnv->pSystem), savedEnvironment(*gEnv),
      m_savedSystemEnvironment(*system->GetGlobalEnvironment()),
      m_savedGameObjectSystem(CGameObject::m_pGOS), m_check(check), m_context(context)
{
    fonts.font = &font;
    m_savedLocale = std::setlocale(LC_CTYPE, nullptr);
}

void CryActionRegressionBoundaryFixture::Trace(const char* text)
{
    m_check(m_context, 3, text, __FILE__, __LINE__);
}

CryActionRegressionBoundaryFixture::~CryActionRegressionBoundaryFixture()
{
    DestroyUI();
    std::setlocale(LC_CTYPE, m_savedLocale.c_str());
    if (m_priorityVariable) m_priorityVariable->ForceSet(std::to_string(m_pakPriority).c_str());
    if (m_mounted) system->GetIPak()->ClosePack(m_uiArchive.c_str());
    if (!m_uiArchive.empty()) DeleteFileA(m_uiArchive.c_str());
    delete framework;
    CGameObject::m_pGOS = m_savedGameObjectSystem;
    *system->GetGlobalEnvironment() = m_savedSystemEnvironment;
    *gEnv = savedEnvironment;
}

bool CryActionRegressionBoundaryFixture::CreateFramework()
{
    if (framework) return true;
    if (!gEnv->pEntitySystem || !gEnv->pScriptSystem) return false;
    Trace("framework: construct actual CCryAction");
    framework = new CCryAction;
    framework->m_pSystem = system;
    framework->m_pEntitySystem = gEnv->pEntitySystem;
    framework->m_pScriptSystem = gEnv->pScriptSystem;
    framework->m_pGameFrameworkListeners = new CCryAction::TGameFrameworkListeners;
    framework->m_pGameFrameworkListenersTemp = new CCryAction::TGameFrameworkListeners;
    framework->m_pCryActionCVars = nullptr;
    framework->m_pTimeOfDayScheduler = nullptr;
    framework->m_pGameStatsConfig = nullptr;
    Trace("framework: construct actual CLevelSystem");
    framework->m_pLevelSystem = new CLevelSystem(system, "__cryaction_regression_no_levels__");
    Trace("framework: construct actual CActorSystem/CGameplayRecorder");
    framework->m_pActorSystem = new CActorSystem(system, gEnv->pEntitySystem);
    framework->m_pGameplayRecorder = new CGameplayRecorder(framework);
    Trace("framework: construct/initialize actual CGameObjectSystem");
    framework->m_pGameObjectSystem = new CGameObjectSystem;
    if (!framework->m_pGameObjectSystem->Init()) return false;
    CGameObject::m_pGOS = framework->m_pGameObjectSystem;
    game.framework = framework;
    gEnv->pGame = &game;
    gEnv->bServer = true;
    gEnv->bClient = false;
    Trace("framework: local collaborators ready");
    return true;
}

bool CryActionRegressionBoundaryFixture::CreateUI(bool useRuntimeRenderer)
{
    if (!CreateFramework()) return false;
    Trace("ui: create and mount local script archive");
    // Mount a tiny local script instead of depending on an installed game's UI.
    char modulePath[MAX_PATH]{};
    HMODULE module = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<const char*>(&CryActionRunRegressionCase), &module);
    GetModuleFileNameA(module, modulePath, sizeof(modulePath));
    m_uiArchive = modulePath;
    m_uiArchive.erase(m_uiArchive.find_last_of("\\/") + 1);
    m_uiArchive += "fixture-" + std::to_string(GetCurrentProcessId()) + "-ui.pak";
    {
        _smart_ptr<ICryArchive> archive = gEnv->pCryPak->OpenArchive(m_uiArchive.c_str(),
            ICryArchive::FLAGS_CREATE_NEW | ICryArchive::FLAGS_RELATIVE_PATHS_ONLY);
        if (!archive) return false;
        char script[] = "CryActionRegressionUiFixture = true; UI = UI or {}; function UI.OnInit() end; function UI.OnRelease() end";
        if (archive->UpdateFile("scripts/ui/ui.lua", script, sizeof(script) - 1) != 0) return false;
    }
    m_mounted = gEnv->pCryPak->OpenPack("Game/", m_uiArchive.c_str());
    m_priorityVariable = gEnv->pConsole->GetCVar("sys_PakPriority");
    if (!m_priorityVariable) return false;
    m_pakPriority = m_priorityVariable->GetIVal();
    m_priorityVariable->ForceSet("1");
    Trace(m_priorityVariable->GetIVal() == 1 ? "ui: pak priority verified" : "ui: pak priority rejected");
    if (!m_mounted || !gEnv->pCryPak->IsFileExist("scripts/ui/ui.lua")) return false;
    Trace("ui: install input/font/renderer collaborators");
    auto* runtime = system->GetGlobalEnvironment();
    runtime->pInput = &input;
    runtime->pCryFont = &fonts;
    runtime->pRenderer = useRuntimeRenderer ? savedEnvironment.pRenderer : &renderer;
    gEnv->pInput = runtime->pInput;
    gEnv->pCryFont = runtime->pCryFont;
    gEnv->pRenderer = runtime->pRenderer;
    Trace("ui: construct actual CUISystem");
    ui = new CUISystem(system, framework);
    bool loadedFixture = false;
    system->GetIScriptSystem()->GetGlobalValue("CryActionRegressionUiFixture", loadedFixture);
    Trace(loadedFixture ? "ui: local script verified" : "ui: WRONG script loaded");
    const std::string fontState = "ui: font resets=" + std::to_string(font.resets) +
        " pixels=" + std::to_string(font.pixels) + " effect=" + font.effect;
    Trace(fontState.c_str());
    if (!loadedFixture) return false;
    Trace("ui: actual CUISystem constructed");
    ui->Begin2D(640, 480);
    return true;
}

void CryActionRegressionBoundaryFixture::DestroyUI()
{
    if (!ui) return;
    delete ui;
    ui = nullptr;
    m_check(m_context, input.adds == input.removes ? 1u : 0u,
        "CUISystem balances input subscription ownership", __FILE__, __LINE__);
    m_check(m_context, input.listeners.empty() ? 1u : 0u,
        "CUISystem leaves no borrowed input listener", __FILE__, __LINE__);
}

CryActionRegressionInspector::CryActionRegressionInspector(IFlowSystem* flow) : CFlowInspectorDefault(flow) {}
void CryActionRegressionInspector::Draw(const std::deque<TFlowRecord>& records, int row, float maxAge, float now)
{
    m_currentTime = CTimeValue(now);
    DrawRecords(records, row, maxAge);
}
void CryActionRegressionInspector::DrawCurrent(int row)
{
    std::deque<TFlowRecord> records(m_curRecords.begin(), m_curRecords.end());
    DrawRecords(records, row, 10.0f);
}
const std::vector<CFlowInspectorDefault::TFlowRecord>& CryActionRegressionInspector::Current() const { return m_curRecords; }

bool CryActionRegressionExtension::OnRegressionRmi(const SJointBreakRec&, INetChannel*) { std::abort(); }

CryActionRegressionExtension::CryActionRegressionExtension(std::vector<std::string>& events, bool succeeds)
    : m_events(events), m_succeeds(succeeds) { m_events.emplace_back("construct"); }
bool CryActionRegressionExtension::Init(IGameObject* object)
{
    SetGameObject(object); m_events.emplace_back("init"); return m_succeeds;
}
void CryActionRegressionExtension::PostInit(IGameObject* object)
{
    if (object != GetGameObject()) std::abort();
    m_events.emplace_back("post-init");
}
void CryActionRegressionExtension::Release() { m_events.emplace_back("release"); delete this; }
void CryActionRegressionExtension::InitClient(int) { m_events.emplace_back("init-client"); }
void CryActionRegressionExtension::PostInitClient(int) { m_events.emplace_back("post-init-client"); }
void CryActionRegressionExtension::FullSerialize(TSerialize) { std::abort(); }
bool CryActionRegressionExtension::NetSerialize(TSerialize, EEntityAspects, uint8, int) { std::abort(); }
void CryActionRegressionExtension::PostSerialize() { std::abort(); }
void CryActionRegressionExtension::SerializeSpawnInfo(TSerialize) { m_events.emplace_back("spawn"); }
ISerializableInfoPtr CryActionRegressionExtension::GetSpawnInfo() { return nullptr; }
void CryActionRegressionExtension::Update(SEntityUpdateContext&, int) { m_events.emplace_back("update"); }
void CryActionRegressionExtension::HandleEvent(const SGameObjectEvent&) {}
void CryActionRegressionExtension::ProcessEvent(SEntityEvent&) {}
void CryActionRegressionExtension::GetMemoryStatistics(ICrySizer*) {}
void CryActionRegressionExtension::SetChannelId(uint16) {}
void CryActionRegressionExtension::SetAuthority(bool) {}
const void* CryActionRegressionExtension::GetRMIBase() const { return nullptr; }
void CryActionRegressionExtension::PostUpdate(float) { m_events.emplace_back("post-update"); }
void CryActionRegressionExtension::PostRemoteSpawn() {}
IGameObjectExtension* CryActionRegressionExtensionFactory::Create()
{
    events.emplace_back("create");
    return createSucceeds ? new CryActionRegressionExtension(events, initSucceeds) : nullptr;
}
void CryActionRegressionExtensionFactory::GetGameObjectExtensionRMIData(void** data, size_t* count)
{
    *data = nullptr; *count = 0;
}

std::uint32_t CryActionRegressionProbe::BoundaryCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    if (!gEnv->pSystem || !gEnv->pEntitySystem || !gEnv->pScriptSystem)
    {
        check(context, 2, "Local CrySystem/script/entity prerequisites unavailable", __FILE__, __LINE__);
        return 2;
    }
    if (testCase >= CryActionRegressionCase::RegistryMissing) WindowsCases(testCase, check, context);
    else if (testCase >= CryActionRegressionCase::DebugEmpty) DiagnosticCases(testCase, check, context);
    else if (testCase >= CryActionRegressionCase::BreakReferences) BreakRepairCases(testCase, check, context);
    else if (testCase >= CryActionRegressionCase::UiRendererLimit && testCase <= CryActionRegressionCase::UiCircleInvalid) UiCases(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::MovementTrace) MovementCases(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::ExtensionInitFailure) ExtensionCases(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::InspectorGraph) InspectorCases(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::BreakChunks) BreakCases(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::ReportPregame) ReportCases(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::UiOffscreen) UiCases(testCase, check, context);
    else TextCases(testCase, check, context);
    return 0;
}

void CryActionRegressionProbe::MovementCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionBoundaryFixture fixture(check, context);
    BOUNDARY_REQUIRE(fixture.CreateFramework(), "AI fixture needs local framework collaborators");
    CryActionRegressionEntity entity;
    CryActionRegressionCharacter character;
    CryActionRegressionSkeleton skeleton;
    CGameObject object;
    // Only the entity attachment is needed by this constructed handler.
    object.m_pEntity = &entity;
    object.m_entityId = entity.id;
    CAIHandler handler(&object);
    entity.character = testCase == CryActionRegressionCase::MovementNoCharacter ? nullptr : &character;
    character.skeleton = testCase == CryActionRegressionCase::MovementNoSkeleton ? nullptr : &skeleton;
    skeleton.movement = static_cast<float>(eMCM_Animation);
    if (testCase <= CryActionRegressionCase::MovementNoSkeleton)
    {
        handler.m_playingSignalAnimation = handler.m_playingActionAnimation = true;
        BOUNDARY_CHECK(!handler.IsAnimationBlockingMovement());
        BOUNDARY_CHECK(skeleton.reads.empty());
        BOUNDARY_CHECK(entity.characterReads == 1);
        BOUNDARY_CHECK(character.skeletonReads == (entity.character ? 1u : 0u));
        return;
    }
    for (unsigned flags = 0; flags != 4; ++flags)
    {
        handler.m_playingSignalAnimation = (flags & 1) != 0;
        handler.m_playingActionAnimation = (flags & 2) != 0;
        BOUNDARY_CHECK(handler.IsAnimationBlockingMovement() == (flags != 0));
    }
    if (testCase == CryActionRegressionCase::MovementValues)
    {
        for (const float value : {0.0f, 2.0f, -1.0f, 42.0f,
            std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity()})
        {
            if (value == static_cast<float>(eMCM_Animation)) continue;
            skeleton.movement = value;
            BOUNDARY_CHECK(!handler.IsAnimationBlockingMovement());
        }
    }
    if (testCase == CryActionRegressionCase::MovementNearby)
    {
        skeleton.movement = std::nextafter(static_cast<float>(eMCM_Animation), -INFINITY);
        BOUNDARY_CHECK(!handler.IsAnimationBlockingMovement());
        skeleton.movement = std::nextafter(static_cast<float>(eMCM_Animation), INFINITY);
        BOUNDARY_CHECK(!handler.IsAnimationBlockingMovement());
    }
    if (testCase == CryActionRegressionCase::MovementTrace)
    {
        handler.m_playingSignalAnimation = true; handler.m_playingActionAnimation = false;
        BOUNDARY_CHECK(handler.IsAnimationBlockingMovement());
        handler.m_playingSignalAnimation = false;
        BOUNDARY_CHECK(!handler.IsAnimationBlockingMovement());
        handler.m_playingActionAnimation = true;
        BOUNDARY_CHECK(handler.IsAnimationBlockingMovement());
        skeleton.movement = static_cast<float>(eMCM_Animation) + 0.25f;
        BOUNDARY_CHECK(!handler.IsAnimationBlockingMovement());
    }
    BOUNDARY_CHECK(!skeleton.reads.empty());
    for (int id : skeleton.reads) BOUNDARY_CHECK(id == eAGUD_MovementControlMethodH);
}

void CryActionRegressionProbe::ExtensionCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionBoundaryFixture fixture(check, context);
    BOUNDARY_REQUIRE(fixture.CreateFramework(), "Extension fixture needs initialized local GameObjectSystem");
    CryActionRegressionExtensionFactory factory;
    factory.createSucceeds = testCase != CryActionRegressionCase::ExtensionCreateFailure;
    factory.initSucceeds = testCase != CryActionRegressionCase::ExtensionInitFailure;
    auto* system = fixture.framework->GetIGameObjectSystem();
    system->RegisterExtension("Regression:Extension", &factory, nullptr);
    BOUNDARY_CHECK(system->GetID("Regression:Extension") != IGameObjectSystem::InvalidExtensionID);
    SEntitySpawnParams params;
    params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
    params.sName = "RegressionExtensionEntity";
    IEntity* entity = gEnv->pEntitySystem->SpawnEntity(params);
    BOUNDARY_REQUIRE(entity, "Actual entity creation for extension fixture failed");
    {
        CGameObject object;
        BOUNDARY_CHECK(object.ActivateExtension("Regression:Extension"));
        BOUNDARY_CHECK(factory.events.empty());
        if (testCase == CryActionRegressionCase::ExtensionRepeated)
        {
            BOUNDARY_CHECK(object.ActivateExtension("Regression:Extension"));
            BOUNDARY_CHECK(object.ActivateExtension("Regression:Extension"));
            BOUNDARY_CHECK(factory.events.empty());
        }
        object.Init(entity, params);
        auto* extension = object.QueryExtension("Regression:Extension");
        if (!factory.createSucceeds || !factory.initSucceeds)
        {
            BOUNDARY_CHECK(extension == nullptr);
            const std::vector<std::string> expected = factory.createSucceeds ?
                std::vector<std::string>{"create","construct","init","release"} : std::vector<std::string>{"create"};
            BOUNDARY_CHECK(factory.events == expected);
            object.Done();
            BOUNDARY_CHECK(factory.events == expected);
        }
        else
        {
            BOUNDARY_CHECK(extension != nullptr);
            BOUNDARY_CHECK((factory.events == std::vector<std::string>{"create","construct","init","post-init"}));
            BOUNDARY_CHECK(object.AcquireExtension("Regression:Extension") == extension);
            BOUNDARY_CHECK(object.AcquireExtension("Regression:Extension") == extension);
            BOUNDARY_CHECK(object.ActivateExtension("Regression:Extension"));
            object.DeactivateExtension("Regression:Extension");
            object.ReleaseExtension("Regression:Extension");
            BOUNDARY_CHECK(object.QueryExtension("Regression:Extension") == extension);
            object.ReleaseExtension("Regression:Extension");
            // Pre-init activation makes the extension sticky until Done().
            BOUNDARY_CHECK(object.QueryExtension("Regression:Extension") == extension);
            BOUNDARY_CHECK(factory.events.size() == 4);
            object.Done();
            BOUNDARY_CHECK((factory.events == std::vector<std::string>{"create","construct","init","post-init","release"}));
            BOUNDARY_CHECK(object.QueryExtension("Regression:Extension") == nullptr);
        }
    }
    gEnv->pEntitySystem->RemoveEntity(entity->GetId(), true);
}

void CryActionRegressionProbe::InspectorCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionBoundaryFixture fixture(check, context);
    gEnv->pRenderer = &fixture.renderer;
    CFlowSystem flow;
    CryActionRegressionInspector inspector(&flow);
    if (testCase == CryActionRegressionCase::InspectorGraph)
    {
        unsigned destructions = 0;
        IFlowNodeFactoryPtr factory = new CryActionRegressionFlowFactory(destructions);
        const auto type = flow.RegisterType("Regression:Layout", factory);
        IFlowGraphPtr graph = flow.CreateFlowGraph();
        graph->CreateNode(type, "source"); graph->CreateNode(type, "target");
        const auto from = graph->ResolveAddress("source:Value", true);
        const auto to = graph->ResolveAddress("target:Value", false);
        BOUNDARY_CHECK(graph->LinkNodes(from, to));
        graph->InitializeValues(); graph->Update();
        inspector.PreUpdate(graph);
        for (int value : {17,29,43})
        {
            graph->ActivatePort(from, value); graph->Update();
            inspector.NotifyFlow(graph, from, to);
        }
        BOUNDARY_CHECK(inspector.Current().size() == 3);
        inspector.DrawCurrent(-2);
        BOUNDARY_CHECK(fixture.renderer.labels.size() == 3);
        for (size_t i = 0; i != fixture.renderer.labels.size(); ++i)
        {
            const auto& label = fixture.renderer.labels[i];
            BOUNDARY_CHECK(label.text == inspector.Current()[i].m_message.c_str());
            const char* endings[]{" Val=17"," Val=29"," Val=43"};
            BOUNDARY_CHECK(label.text.ends_with(endings[i]));
            BOUNDARY_CHECK(label.text.find("Regression:Layout Node:source:Value") != std::string::npos);
            BOUNDARY_CHECK(label.text.find("Regression:Layout Node:target:Value") != std::string::npos);
            BOUNDARY_CHECK(label.y == 50.0f + (0 - static_cast<int>(i)) * 9.5f);
        }
        return;
    }
    const unsigned count = testCase == CryActionRegressionCase::InspectorEmpty ? 0 :
        testCase == CryActionRegressionCase::InspectorSingle ? 1 : 3;
    std::deque<CFlowInspectorDefault::TFlowRecord> records;
    for (unsigned i = 0; i != count; ++i)
    {
        CFlowInspectorDefault::TFlowRecord rec{};
        rec.m_type = CFlowInspectorDefault::eRTEntity;
        rec.m_message.Format("record-%u", i);
        const float timestamps[]{-0.5f,4.5f,10.0f};
        rec.m_tstamp = CTimeValue(testCase == CryActionRegressionCase::InspectorFade ? timestamps[i] : 10.0f);
        records.push_back(rec);
    }
    const int base = testCase == CryActionRegressionCase::InspectorSigned ? -5 : 4;
    inspector.Draw(records, base, 10.0f, 10.0f);
    BOUNDARY_CHECK(fixture.renderer.labels.size() == count);
    for (unsigned i = 0; i != count; ++i)
    {
        const auto& label = fixture.renderer.labels[i];
        BOUNDARY_CHECK(label.text == "record-" + std::to_string(i));
        BOUNDARY_CHECK(label.x == 10.0f);
        BOUNDARY_CHECK(label.y == 50.0f + (base + static_cast<int>(count) - 1 - static_cast<int>(i)) * 9.5f);
        BOUNDARY_CHECK(std::fabs(label.scale - 1.2f) < 0.00001f);
        const float fades[]{0.0f,0.5f,1.0f};
        BOUNDARY_CHECK(label.color.a == (testCase == CryActionRegressionCase::InspectorFade ? fades[i] : 1.0f));
    }
}

void CryActionRegressionProbe::ReportCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionBoundaryFixture fixture(check, context);
    BOUNDARY_REQUIRE(fixture.CreateFramework(), "GameStats needs local framework/recorder/level collaborators");
    auto* maximum = gEnv->pConsole->GetCVar("sv_maxplayers");
    if (!maximum) maximum = gEnv->pConsole->RegisterInt("sv_maxplayers", 3, 0, "Local regression capacity");
    maximum->Set(3);
    CryActionRegressionReport report;
    {
        CGameStats stats(fixture.framework);
        stats.m_serverReport = &report;
        stats.m_playing = testCase == CryActionRegressionCase::ReportPlaying;
        const unsigned count = testCase == CryActionRegressionCase::ReportEmpty ? 0 :
            testCase == CryActionRegressionCase::ReportSingle ? 1 : 3;
        std::array<IEntity*,3> players{};
        for (unsigned i = 0; i != count; ++i)
        {
            SEntitySpawnParams spawn;
            const std::string name = "report-player-" + std::to_string(i);
            spawn.sName = name.c_str();
            spawn.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
            players[i] = gEnv->pEntitySystem->SpawnEntity(spawn);
            BOUNDARY_REQUIRE(players[i], "Local report player entity creation failed");
            int status[]{static_cast<int>(i + 1),0};
            GameplayEvent joined(eGE_Connected, nullptr, 0.0f, status);
            fixture.framework->GetIGameplayRecorder()->Event(players[i], joined);
            fixture.framework->GetIGameplayRecorder()->Event(players[i], GameplayEvent(eGE_Scored, "score", 10.0f + i));
            CGameStats::STeamStats team;
            team.id = i + 1; team.name.Format("team-%u", i); team.score = i * 10;
            // No production team insertion route exists in this owner. Supply its input map.
            stats.m_teamMap.emplace(team.id, team);
        }
        report.order.clear();
        stats.Report();
        std::vector<EntityId> expected;
        for (unsigned i = 0; i != count; ++i) expected.push_back(players[i]->GetId());
        std::sort(expected.begin(), expected.end());
        if (testCase >= CryActionRegressionCase::ReportRemoveFirst && testCase <= CryActionRegressionCase::ReportRemoveLast)
        {
            const unsigned removed = testCase == CryActionRegressionCase::ReportRemoveFirst ? 0 :
                testCase == CryActionRegressionCase::ReportRemoveMiddle ? 1 : 2;
            report.order.clear();
            stats.RemovePlayer(expected[removed], false);
            expected.erase(expected.begin() + removed);
            BOUNDARY_CHECK(report.updates == 2); // one full-server add and one full-server removal
            BOUNDARY_CHECK(report.order.back() == "update");
        }
        BOUNDARY_CHECK(report.players == expected.size());
        BOUNDARY_CHECK(report.teams == count);
        BOUNDARY_CHECK(report.order.size() >= 4);
        BOUNDARY_CHECK(report.order[0] == "params");
        BOUNDARY_CHECK(report.order[1] == "server:gamemode");
        BOUNDARY_CHECK(report.order[2] == "server:timeleft");
        BOUNDARY_CHECK(report.order[3] == "server:numplayers");
        BOUNDARY_CHECK(report.rows[0].value == (stats.m_playing ? "game" : "pre-game"));
        BOUNDARY_CHECK(report.rows[1].value == "-");
        BOUNDARY_CHECK(report.rows[2].value == std::to_string(expected.size()));
        unsigned rowCount = 0;
        for (const auto& row : report.rows)
        {
            if (row.key != "player") continue;
            BOUNDARY_CHECK(row.row == rowCount);
            BOUNDARY_CHECK(rowCount < expected.size());
            if (rowCount < expected.size())
                BOUNDARY_CHECK(row.value == gEnv->pEntitySystem->GetEntity(expected[rowCount])->GetName());
            ++rowCount;
        }
        BOUNDARY_CHECK(rowCount == report.players);
        for (unsigned i = 0; i != expected.size(); ++i)
        {
            const size_t first = 3 + i * 4;
            BOUNDARY_CHECK(report.rows[first + 1].key == "rank" && report.rows[first + 1].value == "1");
            BOUNDARY_CHECK(report.rows[first + 2].key == "team");
            BOUNDARY_CHECK(report.rows[first + 3].key == "score");
        }
        for (auto* player : players) if (player) gEnv->pEntitySystem->RemoveEntity(player->GetId(), true);
    }
    BOUNDARY_CHECK(report.stops == 1);
}

void CryActionRegressionProbe::UiCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionBoundaryFixture fixture(check, context);
    if (testCase >= CryActionRegressionCase::UiPrecacheEmpty && testCase <= CryActionRegressionCase::UiPrecacheThree)
    {
        BOUNDARY_REQUIRE(fixture.CreateFramework(), "Precache needs local framework and entity registry");
        auto* runtime = fixture.system->GetGlobalEnvironment();
        runtime->pGame = &fixture.game;
        auto* enabled = gEnv->pConsole->GetCVar("e_precache_level");
        if (!enabled) enabled = gEnv->pConsole->RegisterInt("e_precache_level", 1, 0, "Local precache");
        enabled->Set(1);
        IEntityClassRegistry::SEntityClassDesc desc;
        desc.sName = "PrecacheCamera";
        auto* cameraClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(desc.sName);
        if (!cameraClass) cameraClass = gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(desc);
        BOUNDARY_REQUIRE(cameraClass, "Precache camera class registration failed");
        const unsigned count = testCase == CryActionRegressionCase::UiPrecacheEmpty ? 0 :
            testCase == CryActionRegressionCase::UiPrecacheSingle ? 1 : 3;
        std::vector<EntityId> entities;
        for (unsigned i = 0; i != count; ++i)
        {
            SEntitySpawnParams spawn;
            spawn.pClass = cameraClass; spawn.sName = "regression-camera";
            spawn.vPosition = Vec3(11.0f + i, 23.0f + i * 2, 37.0f + i * 3);
            auto* entity = gEnv->pEntitySystem->SpawnEntity(spawn);
            BOUNDARY_REQUIRE(entity, "Precache camera entity creation failed");
            entities.push_back(entity->GetId());
        }
        std::vector<Vec3> expected;
        IEntityItPtr iterator = gEnv->pEntitySystem->GetEntityIterator();
        while (auto* entity = iterator->Next())
            if (entity->GetClass() == cameraClass) expected.push_back(entity->GetWorldPos());
        runtime->p3DEngine = &fixture.engine;
        gEnv->p3DEngine = &fixture.engine;
        static_cast<CLevelSystem*>(fixture.framework->GetILevelSystem())->PrecacheLevelRenderData();
        BOUNDARY_CHECK(fixture.engine.calls == 1 && fixture.engine.enabled);
        BOUNDARY_CHECK(fixture.engine.positions.size() == count);
        BOUNDARY_CHECK(fixture.engine.nullPoints == (count == 0));
        BOUNDARY_CHECK(fixture.engine.positions == expected);
        for (auto id : entities) gEnv->pEntitySystem->RemoveEntity(id, true);
        return;
    }
    if (testCase == CryActionRegressionCase::UiOffscreen)
    {
        BOUNDARY_REQUIRE(fixture.savedEnvironment.pRenderer, "Offscreen UI requires an initialized runtime renderer/device; preview renderer absent");
        BOUNDARY_REQUIRE(fixture.CreateUI(true), "Offscreen UI local script/font/input setup failed");
        const int target = fixture.ui->CreateRenderTarget(16,16);
        BOUNDARY_REQUIRE(target > 0, "Runtime renderer failed to allocate a deterministic offscreen target");
        auto* renderer = gEnv->pRenderer;
        auto* asyncShaders = gEnv->pConsole->GetCVar("r_ShadersAsyncCompiling");
        const int previousAsync = asyncShaders ? asyncShaders->GetIVal() : 0;
        if (asyncShaders) asyncShaders->ForceSet("0");
        renderer->BeginFrame();
        CCamera camera;
        camera.SetFrustum(16, 16, DEG2RAD(60.0f), 0.1f, 100.0f);
        renderer->SetCamera(camera);
        renderer->ResetToDefault();
        fixture.ui->SetRenderTarget(target);
        fixture.ui->Begin2D(16,16);
        fixture.ui->Clear(ColorB(0,0,0,255));
        fixture.ui->DrawQuad(0,0,16,16,-1,ColorB(255,0,0,255),nullptr);
        fixture.ui->End2D();
        fixture.ui->SetRenderTarget(0);
        auto* texture = renderer->EF_GetTextureByID(target);
        int pitch = 0;
        byte* pixels = texture ? texture->LockData(pitch) : nullptr;
        if (!pixels)
        {
            fixture.ui->DestroyRenderTarget(target);
            renderer->EndFrame();
            if (asyncShaders) asyncShaders->ForceSet(std::to_string(previousAsync).c_str());
            check(context, 2, "Runtime renderer does not expose offscreen texture readback through ITexture::LockData", __FILE__, __LINE__);
            return;
        }
        BOUNDARY_CHECK(pitch >= 16 * 4);
        if (pitch >= 16 * 4)
        {
            const byte* center = pixels + 8 * pitch + 8 * 4;
            char observed[100];
            snprintf(observed, sizeof(observed), "Offscreen center BGRA = %u, %u, %u, %u", center[0], center[1], center[2], center[3]);
            check(context, 3, observed, __FILE__, __LINE__);
            BOUNDARY_CHECK(center[0] == 0 && center[1] == 0 && center[2] == 255);
        }
        texture->UnlockData();
        fixture.ui->DestroyRenderTarget(target);
        renderer->EndFrame();
        if (asyncShaders) asyncShaders->ForceSet(std::to_string(previousAsync).c_str());
        return;
    }
    BOUNDARY_REQUIRE(fixture.CreateUI(), "CUISystem requires mounted local UI script and initialized script/input/font/renderer collaborators");
    auto& ui = *fixture.ui;
    BOUNDARY_CHECK(fixture.input.adds == 1 && fixture.input.listeners.size() == 1);
    BOUNDARY_CHECK(fixture.font.resets == 1 && fixture.font.pixels && fixture.font.effect == "default");
    CryActionRegressionTexture texture1, texture2;
    texture1.id = 7; texture2.id = 11;
    fixture.renderer.textures.emplace(7, &texture1); fixture.renderer.textures.emplace(11, &texture2);
    const ColorB color(17,39,83,255);
    float tex[]{0.1f,0.2f,0.3f,0.4f,0.5f,0.6f};
    if (testCase == CryActionRegressionCase::UiRendererLimit)
    {
        fixture.renderer.vertexCapacity = 4;
        ui.DrawQuad(10,20,2,3,-1,color,nullptr);
        ui.DrawQuad(30,40,2,3,-1,color,nullptr);
        ui.End2D();
        BOUNDARY_CHECK(fixture.renderer.draws.size() == 2);
        if (fixture.renderer.draws.size() == 2)
        {
            BOUNDARY_CHECK(fixture.renderer.draws[0].vertices[0].xyz == Vec3(10,20,0));
            BOUNDARY_CHECK(fixture.renderer.draws[1].vertices[0].xyz == Vec3(30,40,0));
        }
        fixture.renderer.vertexCapacity = 0;
        ui.DrawLine(1,2,3,4,color);
        ui.End2D();
        BOUNDARY_CHECK(fixture.renderer.draws.size() == 2);
#if defined(__SANITIZE_ADDRESS__)
        // Reproduce a pool slot's old container tail poison, then verify that
        // the actual allocation owner establishes a fresh lifetime on reuse.
        auto* first = static_cast<char*>(CryModuleMalloc(24));
        BOUNDARY_REQUIRE(first, "Pool shadow-lifetime fixture allocation failed");
        __asan_poison_memory_region(first + 16, 8);
        CryModuleFree(first);
        const bool releasedPoison = __asan_address_is_poisoned(first) != 0;
        auto* second = static_cast<char*>(CryModuleMalloc(24));
        BOUNDARY_REQUIRE(second, "Pool shadow-lifetime fixture reallocation failed");
        bool accessible = true;
        for (unsigned i = 0; i < 24; ++i) accessible &= __asan_address_is_poisoned(second + i) == 0;
        const bool reused = first == second;
        CryModuleFree(second);
        BOUNDARY_CHECK(releasedPoison && reused && accessible);
#endif
        return;
    }
    if (testCase == CryActionRegressionCase::UiCircleInvalid)
    {
        for (int slices : {-1,0,1,2}) ui.DrawCircle(1,2,3,slices,color);
        ui.End2D();
        BOUNDARY_CHECK(fixture.renderer.draws.empty());
        return;
    }
    if (testCase == CryActionRegressionCase::UiCircleLarge)
    {
        ui.DrawCircle(50,60,10,70000,color);
        ui.End2D();
        size_t triangles = 0;
        Vec3 previousEnd;
        bool continuous = true, bounded = true, centers = true;
        for (const auto& draw : fixture.renderer.draws)
        {
            bounded &= draw.vertices.size() <= 16384 && draw.indices.size() <= 32768;
            if (triangles) continuous &= (draw.vertices[1].xyz - previousEnd).len() < 0.0001f;
            previousEnd = draw.vertices.back().xyz;
            for (size_t i = 0; i < draw.indices.size(); i += 3)
                centers &= draw.vertices[draw.indices[i]].xyz == Vec3(50,60,0);
            triangles += draw.indices.size() / 3;
        }
        BOUNDARY_CHECK(triangles == 70000 && bounded && centers && continuous);
        BOUNDARY_CHECK((previousEnd - Vec3(60,60,0)).len() < 0.0001f);
        return;
    }
    if (testCase == CryActionRegressionCase::UiLine) ui.DrawLine(11,23,37,41,color);
    if (testCase == CryActionRegressionCase::UiTri) ui.DrawTri(11,23,37,41,53,67,7,color,tex);
    if (testCase == CryActionRegressionCase::UiQuad) ui.DrawQuad(11,23,26,18,7,color,tex);
    if (testCase == CryActionRegressionCase::UiCircle) ui.DrawCircle(50,60,10,4,color);
    if (testCase == CryActionRegressionCase::UiMixed)
    {
        ui.DrawLine(1,2,3,4,color);
        ui.DrawTri(11,23,37,41,53,67,-1,color,tex);
        ui.DrawQuad(70,80,10,20,-1,color,nullptr);
        ui.DrawCircle(100,120,5,4,color);
    }
    if (testCase == CryActionRegressionCase::UiTexture)
    {
        ui.DrawQuad(1,2,3,4,7,color,nullptr);
        ui.DrawQuad(11,12,3,4,11,color,nullptr);
        ui.DrawQuad(21,22,3,4,7,color,nullptr);
    }
    if (testCase == CryActionRegressionCase::UiScissor)
    {
        ui.DrawQuad(0,0,40,40,7,color,nullptr);
        ui.SetScissor(10,10,20,20);
        ui.DrawQuad(0,0,40,40,7,color,nullptr);
        BOUNDARY_CHECK(fixture.font.clipping);
        BOUNDARY_CHECK((fixture.font.clip == std::array<float,4>{10,10,20,20}));
        ui.SetScissor(0,0,0,0);
        BOUNDARY_CHECK(!fixture.font.clipping);
    }
    if (testCase == CryActionRegressionCase::UiBlend)
    {
        ui.DrawQuad(1,2,3,4,-1,color,nullptr);
        ui.SetBlend(GS_BLSRC_ONE, GS_BLDST_ONE);
        ui.DrawQuad(11,12,3,4,-1,color,nullptr);
    }
    if (testCase == CryActionRegressionCase::UiIndexLast || testCase == CryActionRegressionCase::UiIndexCrossLine)
    {
        const int count = testCase == CryActionRegressionCase::UiIndexLast ? 32768 : 32769;
        for (int i = 0; i != count; ++i) ui.DrawLine(i * 2.0f,0,i * 2.0f + 1,0,color);
    }
    if (testCase == CryActionRegressionCase::UiIndexCrossTri)
        for (int i = 0; i != 21846; ++i) ui.DrawTri(i * 3.0f,0,i * 3.0f + 1,0,i * 3.0f + 2,0,-1,color,tex);
    if (testCase == CryActionRegressionCase::UiIndexCrossQuad)
        for (int i = 0; i != 16385; ++i) ui.DrawQuad(i * 4.0f,0,2,3,-1,color,nullptr);
    if (testCase == CryActionRegressionCase::UiIndexCrossCircle)
    {
        for (int i = 0; i != 16383; ++i) ui.DrawQuad(i * 4.0f,0,2,3,-1,color,nullptr);
        ui.DrawCircle(100000,120000,10,4,color);
    }
    ui.End2D();
    auto& draws = fixture.renderer.draws;
    size_t vertices = 0, indices = 0;
    for (const auto& draw : draws)
    {
        vertices += draw.vertices.size(); indices += draw.indices.size();
        BOUNDARY_CHECK(draw.vertices.size() <= static_cast<size_t>(fixture.renderer.vertexCapacity));
        BOUNDARY_CHECK(draw.indices.size() <= 32768);
        const bool bounded = std::all_of(draw.indices.begin(), draw.indices.end(),
            [&draw](uint16 index) { return index < draw.vertices.size(); });
        BOUNDARY_CHECK(bounded);
    }
    if (testCase == CryActionRegressionCase::UiEmpty) BOUNDARY_CHECK(draws.empty());
    if (testCase == CryActionRegressionCase::UiLine)
    {
        BOUNDARY_CHECK(vertices == 2 && indices == 2 && draws.size() == 1);
        BOUNDARY_CHECK(draws[0].primitive == R_PRIMV_LINES);
        BOUNDARY_CHECK(draws[0].vertices[draws[0].indices[0]].xyz == Vec3(11,23,0));
        BOUNDARY_CHECK(draws[0].vertices[draws[0].indices[1]].xyz == Vec3(37,41,0));
    }
    if (testCase == CryActionRegressionCase::UiTri)
    {
        BOUNDARY_CHECK(vertices == 3 && indices == 3);
        const Vec3 expected[]{Vec3(11,23,0),Vec3(37,41,0),Vec3(53,67,0)};
        for (unsigned i = 0; i != 3; ++i)
        {
            const auto& vertex = draws[0].vertices[draws[0].indices[i]];
            BOUNDARY_CHECK(vertex.xyz == expected[i]);
            BOUNDARY_CHECK(vertex.st[0] == tex[i*2] && vertex.st[1] == tex[i*2+1]);
            BOUNDARY_CHECK(vertex.color.dcolor == RGBA8(17,39,83,255));
        }
        BOUNDARY_CHECK(draws[0].texture == 7);
    }
    if (testCase == CryActionRegressionCase::UiQuad)
    {
        BOUNDARY_CHECK(vertices == 4 && indices == 6);
        const Vec3 expected[]{Vec3(11,23,0),Vec3(37,23,0),Vec3(11,41,0),Vec3(37,23,0),Vec3(37,41,0),Vec3(11,41,0)};
        for (unsigned i = 0; i != 6; ++i) BOUNDARY_CHECK(draws[0].vertices[draws[0].indices[i]].xyz == expected[i]);
    }
    if (testCase == CryActionRegressionCase::UiCircle)
    {
        BOUNDARY_CHECK(vertices == 6 && indices == 12);
        const Vec3 expected[]{Vec3(50,60,0),Vec3(60,60,0),Vec3(50,70,0),Vec3(40,60,0),Vec3(50,50,0),Vec3(60,60,0)};
        for (unsigned i = 0; i != 6; ++i) BOUNDARY_CHECK((draws[0].vertices[i].xyz - expected[i]).len() < 0.0001f);
    }
    if (testCase == CryActionRegressionCase::UiMixed)
    {
        BOUNDARY_CHECK(vertices == 15 && indices == 23 && draws.size() == 2);
        BOUNDARY_CHECK(draws[0].primitive == R_PRIMV_LINES && draws[1].primitive == R_PRIMV_TRIANGLES);
        BOUNDARY_CHECK(draws[1].vertices[draws[1].indices[0]].xyz == Vec3(11,23,0));
    }
    if (testCase == CryActionRegressionCase::UiTexture)
    {
        BOUNDARY_CHECK(vertices == 12 && indices == 18 && draws.size() == 3);
        BOUNDARY_CHECK(draws[0].texture == 7 && draws[1].texture == 11 && draws[2].texture == 7);
    }
    if (testCase == CryActionRegressionCase::UiScissor)
    {
        BOUNDARY_CHECK(vertices == 8 && indices == 12);
        BOUNDARY_CHECK(draws[0].vertices[0].xyz == Vec3(0,0,0));
        BOUNDARY_CHECK(draws[0].vertices[4].xyz == Vec3(10,10,0));
        BOUNDARY_CHECK(draws[0].vertices[6].xyz == Vec3(30,30,0));
    }
    if (testCase == CryActionRegressionCase::UiBlend)
    {
        BOUNDARY_CHECK(vertices == 8 && indices == 12);
        // An explicit state transition must preserve submission order and chosen blending.
        BOUNDARY_CHECK(draws.size() == 2);
        if (!draws.empty()) BOUNDARY_CHECK(draws[0].state == (GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST));
        if (draws.size() >= 2) BOUNDARY_CHECK(draws[1].state == (GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST));
    }
    if (testCase >= CryActionRegressionCase::UiIndexLast && testCase <= CryActionRegressionCase::UiIndexCrossTri)
    {
        const size_t expected = testCase == CryActionRegressionCase::UiIndexLast ? 65536 : 65538;
        BOUNDARY_CHECK(vertices == expected && indices == expected);
        size_t ordinal = 0;
        bool association = true;
        for (const auto& draw : draws)
            for (auto index : draw.indices) association &= draw.vertices[index].xyz.x == static_cast<float>(ordinal++);
        BOUNDARY_CHECK(ordinal == expected && association);
    }
    if (testCase == CryActionRegressionCase::UiIndexCrossQuad || testCase == CryActionRegressionCase::UiIndexCrossCircle)
    {
        const bool circle = testCase == CryActionRegressionCase::UiIndexCrossCircle;
        BOUNDARY_CHECK(vertices == (circle ? 65538 : 65540));
        BOUNDARY_CHECK(indices == (circle ? 98310 : 98310));
        size_t ordinal = 0;
        bool association = true;
        const unsigned corners[]{0,1,3,1,2,3};
        for (const auto& draw : draws)
            for (auto index : draw.indices)
            {
                const size_t quad = ordinal / 6;
                if (quad < (circle ? 16383u : 16385u))
                {
                    const unsigned corner = corners[ordinal % 6];
                    const Vec3 expected(static_cast<float>(quad * 4 + ((corner == 1 || corner == 2) ? 2 : 0)), corner >= 2 ? 3.0f : 0.0f,0);
                    association &= draw.vertices[index].xyz == expected;
                }
                else association &= draw.vertices[index].xyz.x >= 99989.0f;
                ++ordinal;
            }
        BOUNDARY_CHECK(association);
    }
}

void CryActionRegressionProbe::TextCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionBoundaryFixture fixture(check, context);
    BOUNDARY_REQUIRE(fixture.CreateUI(), "Text conversion requires a valid CUISystem fixture");
    const char* selected = std::setlocale(LC_CTYPE, "C");
    BOUNDARY_REQUIRE(selected, "CRT C locale unavailable");
    const bool multibyte = (testCase >= CryActionRegressionCase::TextMultibyte && testCase <= CryActionRegressionCase::TextInvalid) ||
        testCase == CryActionRegressionCase::TextInvalidMiddle || testCase == CryActionRegressionCase::TextDbcs;
    if (multibyte) BOUNDARY_REQUIRE(std::setlocale(LC_CTYPE, ".UTF8"), "CRT UTF-8 locale unavailable for explicit locale characterization");
    if (testCase == CryActionRegressionCase::TextLegacyByte) BOUNDARY_REQUIRE(std::setlocale(LC_CTYPE, ".1252"), "CRT Windows-1252 locale unavailable");
    if (testCase == CryActionRegressionCase::TextDbcs) BOUNDARY_REQUIRE(std::setlocale(LC_CTYPE, ".932"), "CRT Windows-932 locale unavailable");
    std::string input;
    if (testCase == CryActionRegressionCase::TextAscii || testCase == CryActionRegressionCase::TextFont) input = "Legacy UI 123";
    if (testCase == CryActionRegressionCase::Text2047) input.assign(2047, 'A');
    if (testCase == CryActionRegressionCase::Text2048) input.assign(2048, 'B');
    if (testCase == CryActionRegressionCase::Text2049) input.assign(2049, 'C');
    if (testCase == CryActionRegressionCase::TextMultibyte) input = "A\xc3\xa9Z";
    if (testCase == CryActionRegressionCase::TextMultibyteBoundary) input = std::string(2046,'A') + "\xc3\xa9Z";
    if (testCase == CryActionRegressionCase::TextInvalid) input = "\xc3(";
    if (testCase == CryActionRegressionCase::TextInvalidMiddle) input = "A\xc3(";
    if (testCase == CryActionRegressionCase::TextLegacyByte) input = "\xe9";
    if (testCase == CryActionRegressionCase::TextDbcs) input = "\x82\xa0";
    if (testCase == CryActionRegressionCase::TextRepeated)
    {
        const auto* first = fixture.ui->ToWideString("long first string");
        const std::wstring owned(first);
        const auto* next = fixture.ui->ToWideString("X");
        BOUNDARY_CHECK(first == next);
        BOUNDARY_CHECK(std::wcscmp(next,L"X") == 0);
        BOUNDARY_CHECK(owned == L"long first string");
        BOUNDARY_CHECK(std::wcscmp(fixture.ui->ToWideString(""),L"") == 0);
        std::setlocale(LC_CTYPE,"C");
        return;
    }
    const std::string unchanged(input);
    // Poison prior contents through a legal call; conversion errors must not expose it.
    if (multibyte) fixture.ui->ToWideString("stale previous text");
    const wchar_t* result = fixture.ui->ToWideString(testCase == CryActionRegressionCase::TextNull ? nullptr : input.c_str());
    size_t length = 0;
    while (length != 2048 && result[length] != 0) ++length;
    BOUNDARY_CHECK(input == unchanged);
    BOUNDARY_CHECK(length < 2048);
    if (testCase == CryActionRegressionCase::Text2048 || testCase == CryActionRegressionCase::Text2049)
        BOUNDARY_CHECK(length == 2047 && result[2046] == (testCase == CryActionRegressionCase::Text2048 ? L'B' : L'C'));
    if (testCase == CryActionRegressionCase::TextDbcs) BOUNDARY_CHECK(length == 1 && result[0] == L'\u3042');
    if (input.size() < 2048 && !multibyte)
    {
        BOUNDARY_CHECK(length == input.size());
        for (size_t i = 0; i != length; ++i) BOUNDARY_CHECK(result[i] == static_cast<unsigned char>(input[i]));
    }
    if (testCase == CryActionRegressionCase::TextMultibyte) BOUNDARY_CHECK(length == 3 && std::wmemcmp(result,L"A\u00e9Z",3) == 0);
    if (testCase == CryActionRegressionCase::TextMultibyteBoundary)
    {
        BOUNDARY_CHECK(length == 2047 && result[2046] == L'\u00e9');
    }
    if (testCase == CryActionRegressionCase::TextInvalid || testCase == CryActionRegressionCase::TextInvalidMiddle)
    {
        // Invalid sequences reject the result, including a previously converted prefix.
        BOUNDARY_CHECK(length == 0);
    }
    if (length < 2048)
    {
        fixture.ui->DrawText(11,23,result,16,ColorB(17,39,83,255),eUIA_Left,eUIA_Top);
        BOUNDARY_CHECK(fixture.font.borrowed == result && fixture.font.draws == 1);
        BOUNDARY_CHECK(fixture.font.text.size() == length);
        BOUNDARY_CHECK(fixture.font.size.x == 16 && fixture.font.size.y == 16);
    }
    std::setlocale(LC_CTYPE,"C");
}

void CryActionRegressionProbe::BreakCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    if (testCase >= CryActionRegressionCase::BreakRecord)
    { BreakRepairCases(testCase, check, context); return; }
    BOUNDARY_REQUIRE(gEnv->pPhysicalWorld, "CryPhysics world unavailable in the local preview runtime");
    CryActionRegressionBoundaryFixture fixture(check, context);
    CNetworkCVars network;
    network.BreakageLog = 0;
    CryActionRegressionEntities entities;
    gEnv->pEntitySystem = &entities;
    std::array<CryActionRegressionEntity,3> objects;
    std::array<CryActionRegressionPhysicalEntity,3> physics;
    IProceduralBreakTypePtr type = new CJointBreak;
    const unsigned count = testCase == CryActionRegressionCase::BreakEmpty ? 0 :
        testCase == CryActionRegressionCase::BreakSingle ? 1 : 3;
    for (unsigned i = 0; i != count; ++i)
    {
        objects[i].id = 100 + i; objects[i].physics = &physics[i]; physics[i].entity = &objects[i];
        entities.entities.emplace(objects[i].id, &objects[i]);
        SExplosiveObjectState state{};
        state.isEnt = true; state.entId = objects[i].id;
        state.entPos = Vec3(0,0,0); state.entRot = IDENTITY; state.entScale = Vec3(1,1,1);
        BOUNDARY_CHECK(type->GotExplosiveObjectState(&state));
        BOUNDARY_CHECK(type->GetVirtualId(&physics[i]) == i + 1);
    }
    _smart_ptr<CProceduralBreakingPlaybackStream> stream = new CProceduralBreakingPlaybackStream(type);
    BOUNDARY_CHECK(stream->SetMagicId(7));
    BOUNDARY_CHECK(!stream->SetMagicId(8));
    for (unsigned i = 0; i != count; ++i)
    {
        SJointBreakRec record{};
        record.idxRef = -static_cast<int>(i + 1);
        record.id = testCase == CryActionRegressionCase::BreakSentinel ? -1 : 11 + i;
        record.epicenter = 31 + i;
        record.frame = testCase == CryActionRegressionCase::BreakFrame ? 1 : 0;
        BOUNDARY_CHECK(stream->GotJointBreakRec(&record));
        physics[i].succeeds = testCase != CryActionRegressionCase::BreakRetry &&
            (testCase != CryActionRegressionCase::BreakMixed || i != 1);
    }
    CProceduralBreakingPlaybackListener playback(stream, nullptr);
    playback.Start();
    if (testCase == CryActionRegressionCase::BreakFrame)
    {
        BOUNDARY_CHECK(stream->m_jointBreaks.size() == 3);
        for (const auto& object : physics) BOUNDARY_CHECK(object.joints.empty());
        playback.IncrementFrame();
    }
    const unsigned retained = testCase == CryActionRegressionCase::BreakRetry ? count :
        testCase == CryActionRegressionCase::BreakMixed ? 1 : 0;
    BOUNDARY_CHECK(stream->m_jointBreaks.size() == retained);
    BOUNDARY_CHECK(playback.AllowComplete() == (retained == 0));
    for (unsigned i = 0; i != count; ++i)
    {
        BOUNDARY_CHECK(physics[i].joints.size() == 1);
        if (physics[i].joints.size() == 1)
        {
            const auto& joint = physics[i].joints[0];
            BOUNDARY_CHECK(joint.partidEpicenter == 31 + i);
            if (testCase == CryActionRegressionCase::BreakSentinel) BOUNDARY_CHECK(joint.idx == -1);
            else BOUNDARY_CHECK(joint.id == 11 + i && joint.bBroken && joint.bReplaceExisting);
        }
    }
    if (retained)
    {
        BOUNDARY_CHECK(stream->m_jointBreaks[0].idxRef == (retained == 1 ? -2 : -1));
        for (auto& object : physics) object.succeeds = true;
        playback.UpdateJointBreaks();
        BOUNDARY_CHECK(stream->m_jointBreaks.empty() && playback.AllowComplete());
        for (unsigned i = 0; i != count; ++i)
            BOUNDARY_CHECK(physics[i].joints.size() == ((retained == count || i == 1) ? 2u : 1u));
    }
    BOUNDARY_CHECK(!playback.OnEndFrame());
}
