#include "StdAfx.h"
#include "CryActionRegressionProbe.h"
#include "CryActionRepairRegression.h"
#include "CryActionBoundaryRegression.h"
#include <CryAction.h>
#include <CryActionWindows.h>
#include <BreakablePlane.h>
#include <ActionGame.h>
#include <GameObjects/GameObjectSystem.h>
#include <AnimationGraph/AnimationGraph.h>
#include <AnimationGraph/AnimationGraphState.h>
#include <AnimationGraph/AnimationGraphManager.h>
#include <AnimationGraph/AnimationGraphCVars.h>
#include <AnimationGraph/AGForceStartImmediately.h>
#include <Network/NetworkCVars.h>
#include <Network/JointBreak.h>
#include <Network/ProceduralBreakingRecordingListener.h>
#include <Network/ProceduralBreakingPlaybackListener.h>
#include <Serialization/SerializeScriptTableWriter.h>
#include <Serialization/SerializeScriptTableReader.h>
#include <SimpleSerialize.h>
#include <algorithm>
#include <limits>

#define REPAIR_CHECK(expression) check(context, (expression) ? 1u : 0u, #expression, __FILE__, __LINE__)
#define REPAIR_REQUIRE(expression, reason) if (!(expression)) { check(context, 2, reason, __FILE__, __LINE__); return; }

CryActionRegressionDiagnostics::CryActionRegressionDiagnostics()
    : m_verbosity(gEnv->pLog->GetVerbosityLevel()), m_fileVerbosity(0)
{
    gEnv->pLog->SetVerbosity(4);
    if (auto* variable = gEnv->pConsole->GetCVar("log_FileVerbosity"))
    { m_fileVerbosity = variable->GetIVal(); variable->ForceSet("4"); }
    gEnv->pLog->AddCallback(this);
}
CryActionRegressionDiagnostics::~CryActionRegressionDiagnostics()
{
    gEnv->pLog->RemoveCallback(this);
    gEnv->pLog->SetVerbosity(m_verbosity);
    if (auto* variable = gEnv->pConsole->GetCVar("log_FileVerbosity"))
        variable->ForceSet(std::to_string(m_fileVerbosity).c_str());
}
void CryActionRegressionDiagnostics::OnWriteToConsole(const char* text, bool) { lines.emplace_back(text); }
void CryActionRegressionDiagnostics::OnWriteToFile(const char* text, bool) { lines.emplace_back(text); }
bool CryActionRegressionDiagnostics::Contains(const char* text) const
{
    for (const auto& line : lines) if (line.find(text) != std::string::npos) return true;
    return false;
}

CryActionRegressionWindowsFixture* CryActionRegressionWindowsFixture::active = nullptr;
CryActionRegressionWindowsFixture::CryActionRegressionWindowsFixture()
{
    path = "Software\\CryActionRegression\\" + std::to_string(GetCurrentProcessId());
    active = this;
    CryActionWindows::registryOverride = path.c_str();
    CryActionWindows::keyObserver = CaptureKey;
}
CryActionRegressionWindowsFixture::~CryActionRegressionWindowsFixture()
{
    CryActionWindows::registryOverride = nullptr;
    CryActionWindows::keyObserver = nullptr;
    RegDeleteKeyExA(HKEY_CURRENT_USER, path.c_str(), KEY_WOW64_32KEY, 0);
    active = nullptr;
}
LONG CryActionRegressionWindowsFixture::Store(DWORD type, const char* text, DWORD length)
{
    HKEY key = nullptr;
    LONG result = RegCreateKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, nullptr, 0,
        KEY_WRITE | KEY_WOW64_32KEY, nullptr, &key, nullptr);
    if (result != ERROR_SUCCESS) return result;
    result = RegSetValueExA(key, nullptr, 0, type, reinterpret_cast<const BYTE*>(text), length);
    RegCloseKey(key);
    return result;
}
void CryActionRegressionWindowsFixture::CaptureKey(const char* key)
{
    active->observed = key;
    ++active->observations;
}
DWORD WINAPI CryActionRegressionWindowsFixture::TempPath(DWORD capacity, LPSTR output)
{
    ++active->tempCalls;
    if (active->temporaryResult) return active->temporaryResult;
    if (active->temporary.size() >= capacity) return static_cast<DWORD>(active->temporary.size() + 1);
    memcpy(output, active->temporary.c_str(), active->temporary.size() + 1);
    return static_cast<DWORD>(active->temporary.size());
}

void CryActionRegressionProbe::WindowsCases(CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionBoundaryFixture fixture(check, context);
    CryActionRegressionWindowsFixture windows;
    if (testCase == CryActionRegressionCase::ViewNotePath)
    {
        char real[1024];
        REPAIR_CHECK(CryActionWindows::BuildViewNotePath(real, sizeof(real)));
        REPAIR_CHECK(strlen(real) >= 12 && strcmp(real + strlen(real) - 12, "viewnote.txt") == 0);
        struct { char before; char path[64]; char after; } buffer{'L', {}, 'R'};
        windows.temporary = "C:\\A path with spaces\\";
        REPAIR_CHECK(CryActionWindows::BuildViewNotePath(buffer.path, sizeof(buffer.path), windows.TempPath));
        REPAIR_CHECK(std::string(buffer.path) == windows.temporary + "viewnote.txt");
        windows.temporary.assign(51, 'x'); // 51 + 12 + NUL exactly fills 64.
        REPAIR_CHECK(CryActionWindows::BuildViewNotePath(buffer.path, sizeof(buffer.path), windows.TempPath));
        REPAIR_CHECK(strlen(buffer.path) == 63);
        windows.temporary.push_back('x');
        REPAIR_CHECK(!CryActionWindows::BuildViewNotePath(buffer.path, sizeof(buffer.path), windows.TempPath));
        REPAIR_CHECK(buffer.path[0] == 0);
        windows.temporary.clear();
        REPAIR_CHECK(!CryActionWindows::BuildViewNotePath(buffer.path, sizeof(buffer.path), windows.TempPath));
        windows.temporaryResult = MAXDWORD;
        REPAIR_CHECK(!CryActionWindows::BuildViewNotePath(buffer.path, sizeof(buffer.path), windows.TempPath));
        unsigned calls = windows.tempCalls;
        REPAIR_CHECK(!CryActionWindows::BuildViewNotePath(buffer.path, 1, windows.TempPath));
        REPAIR_CHECK(!CryActionWindows::BuildViewNotePath(nullptr, 64, windows.TempPath));
        REPAIR_CHECK(calls == windows.tempCalls && buffer.before == 'L' && buffer.after == 'R');
        return;
    }
    REPAIR_REQUIRE(fixture.CreateFramework(), "Local framework unavailable");
    CryActionRegressionDiagnostics diagnostics;
    if (testCase == CryActionRegressionCase::RegistryMissing)
    {
        fixture.framework->ReadCDKey();
        REPAIR_CHECK(windows.observations == 0);
        REPAIR_CHECK(diagnostics.Contains("Failed to read CDKey from registry"));
    }
    else if (testCase == CryActionRegressionCase::RegistryLengths)
    {
        for (unsigned length : {0u, 1u, 3u, 30u, 31u})
        {
            std::string key(length, 'K');
            fixture.framework->SaveCDKey(key.c_str());
            fixture.framework->ReadCDKey();
            REPAIR_CHECK(windows.observed == key);
        }
        REPAIR_CHECK(windows.observations == 5);
        REPAIR_CHECK(CryActionWindows::WriteKey(HKEY_CURRENT_USER, windows.path.c_str(), "", MAXDWORD) == ERROR_INVALID_PARAMETER);
        REPAIR_CHECK(CryActionWindows::WriteKey(HKEY_CURRENT_USER, windows.path.c_str(), nullptr, 0) == ERROR_INVALID_PARAMETER);
    }
    else if (testCase == CryActionRegressionCase::RegistryMalformed)
    {
        for (const auto& value : {std::string(32,'X'), std::string(33,'Y')})
        {
            REPAIR_CHECK(windows.Store(REG_SZ, value.c_str(), static_cast<DWORD>(value.size())) == ERROR_SUCCESS);
            fixture.framework->ReadCDKey();
            REPAIR_CHECK(windows.observations == 0);
            struct { char before; char text[32]; char after; } buffer{'L', {}, 'R'};
            REPAIR_CHECK(CryActionWindows::ReadKey(HKEY_CURRENT_USER, windows.path.c_str(), buffer.text, sizeof(buffer.text)) == ERROR_MORE_DATA);
            REPAIR_CHECK(buffer.text[0] == 0 && buffer.before == 'L' && buffer.after == 'R');
        }
        REPAIR_CHECK(windows.Store(REG_BINARY, "KEY", 4) == ERROR_SUCCESS);
        fixture.framework->ReadCDKey();
        REPAIR_CHECK(windows.observations == 0);
        REPAIR_CHECK(diagnostics.Contains("Failed to read CDKey from registry"));
    }
    else
    {
        const char* key = "LOCAL-REGRESSION-KEY";
        if (testCase == CryActionRegressionCase::RegistryLegacy)
            REPAIR_CHECK(windows.Store(REG_SZ, key, static_cast<DWORD>(strlen(key))) == ERROR_SUCCESS);
        else fixture.framework->SaveCDKey(key);
        fixture.framework->ReadCDKey();
        REPAIR_CHECK(windows.observed == key && windows.observations == 1);
        if (testCase == CryActionRegressionCase::RegistryRoundtrip)
        {
            HKEY handle = nullptr;
            REPAIR_CHECK(RegOpenKeyExA(HKEY_CURRENT_USER, windows.path.c_str(), 0, KEY_READ | KEY_WOW64_32KEY, &handle) == ERROR_SUCCESS);
            if (handle)
            {
                char stored[32] = {}; DWORD type = 0, length = sizeof(stored);
                REPAIR_CHECK(RegQueryValueExA(handle, nullptr, nullptr, &type, reinterpret_cast<BYTE*>(stored), &length) == ERROR_SUCCESS);
                REPAIR_CHECK(type == REG_SZ && length == strlen(key) + 1 && stored[length - 1] == 0);
                RegCloseKey(handle);
            }
        }
        if (testCase == CryActionRegressionCase::RegistryHandles)
        {
            DWORD before = 0, after = 0;
            REPAIR_CHECK(GetProcessHandleCount(GetCurrentProcess(), &before));
            for (unsigned i = 0; i != 128; ++i)
            { fixture.framework->SaveCDKey(key); fixture.framework->ReadCDKey(); }
            REPAIR_CHECK(GetProcessHandleCount(GetCurrentProcess(), &after));
            REPAIR_CHECK(before == after && windows.observations == 129);
        }
    }
}

void CryActionRegressionProbe::DiagnosticCases(CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionBoundaryFixture fixture(check, context);
    if (testCase == CryActionRegressionCase::DebugRmi)
    {
        REPAIR_REQUIRE(fixture.CreateFramework(), "Local framework unavailable");
        SEntitySpawnParams params;
        params.sName = "diagnostic%entity";
        params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
        IEntity* entity = gEnv->pEntitySystem->SpawnEntity(params);
        REPAIR_REQUIRE(entity, "Local entity unavailable");
        IGameObject* object = fixture.framework->GetIGameObjectSystem()->CreateGameObjectForEntity(entity->GetId());
        REPAIR_CHECK(object);
        if (object)
        {
            REPAIR_CHECK(fixture.framework->GetGameObject(entity->GetId()) == object);
            int base = 0;
            SNetMessageDef message = {}; message.description = "diagnostic%RMI";
            SGameObjectExtensionRMI rmi = {}; rmi.pBase = &base; rmi.pMsgDef = &message;
            CryActionRegressionDiagnostics diagnostics;
            auto* item = CRMIAtSyncItem<SJointBreakRec,CryActionRegressionExtension>::Create(
                SJointBreakRec{}, entity->GetId(), &rmi, &CryActionRegressionExtension::OnRegressionRmi, nullptr);
            REPAIR_CHECK(item->Sync());
            item->DeleteThis();
            for (const auto& line : diagnostics.lines)
                check(context, 3, line.c_str(), __FILE__, __LINE__);
            char expected[80]; snprintf(expected, sizeof(expected), "base %p", static_cast<void*>(&base));
            REPAIR_CHECK(diagnostics.Contains(expected));
            REPAIR_CHECK(diagnostics.Contains("diagnostic%entity") && diagnostics.Contains("diagnostic%RMI"));
        }
        gEnv->pEntitySystem->RemoveEntity(entity->GetId(), true);
        return;
    }
    gEnv->pRenderer = &fixture.renderer;
    CAnimationGraphCVars variables;
    CAnimationGraphManager manager;
    auto graph = CAnimationGraph::CreateInstance(&manager, "DiagnosticsFixture");
    auto root = gEnv->pSystem->CreateXmlNode("AnimationGraph");
    auto inputs = root->newChild("Inputs"); root->newChild("Views");
    const unsigned count = testCase == CryActionRegressionCase::DebugEmpty ? 0 : testCase == CryActionRegressionCase::DebugSingle ? 1 : 3;
    for (unsigned i = 0; i != count; ++i)
    {
        auto input = inputs->newChild("KeyState");
        input->setAttr("name", ("Input" + std::to_string(i)).c_str());
        input->setAttr("defaultValue", "value%literal");
        input->newChild("Key")->setAttr("value", "value%literal");
    }
    auto states = root->newChild("States");
    auto node = states->newChild("State"); node->setAttr("id", "State%literal");
    node->setAttr("cost", 0); node->setAttr("allowSelect", false);
    root->newChild("Transitions");
    REPAIR_CHECK(graph->Init(root));
    CAnimationGraphState state(graph);
    state.m_debugLayer = state.m_layerIndex;
    for (unsigned i = 0; i != count; ++i)
    {
        state.m_inputQueryIDs[i] = 4000000000u + i;
        state.m_queryChangedInputIDs[i] = 3000000000u + i;
    }
    CAGForceStartImmediately factory;
    if (testCase == CryActionRegressionCase::DebugPointer) state.m_currentStateNodes.push_back(&factory);
    if (testCase == CryActionRegressionCase::DebugPercent)
    {
        state.m_state = &CAnimationGraphState::Update_SteadyState;
        state.m_currentStateID = graph->StateNameToID("State%literal");
    }
    state.DebugDisplay();
    state.m_currentStateNodes.clear();
    std::string labels;
    for (const auto& label : fixture.renderer.labels) labels += label.text + "\n";
    for (unsigned i = 0; i != count; ++i)
    {
        REPAIR_CHECK(labels.find("[query " + std::to_string(4000000000u + i) + "] [onchange " + std::to_string(3000000000u + i) + "]") != std::string::npos);
        REPAIR_CHECK(state.m_inputQueryIDs[i] == 4000000000u + i && state.m_queryChangedInputIDs[i] == 3000000000u + i);
    }
    if (!count) REPAIR_CHECK(labels.find("[query ") == std::string::npos);
    if (testCase == CryActionRegressionCase::DebugPercent) REPAIR_CHECK(labels.find("steady state State%literal") != std::string::npos);
    if (count) REPAIR_CHECK(labels.find("value%literal") != std::string::npos);
    if (testCase == CryActionRegressionCase::DebugPointer)
    {
        char expected[128];
        snprintf(expected, sizeof(expected), "ForceStartImmediately[%p]: instant", static_cast<void*>(static_cast<IAnimationStateNodeFactory*>(&factory)));
        REPAIR_CHECK(labels.find(expected) != std::string::npos);
    }
}

CryActionRegressionBreakFixture::CryActionRegressionBreakFixture(IPhysicalWorld* world) : m_world(world) {}
CryActionRegressionBreakFixture::~CryActionRegressionBreakFixture()
{
    for (auto* body : m_bodies) m_world->DestroyPhysicalEntity(body);
}
void CryActionRegressionBreakFixture::Destroy(IPhysicalEntity* body)
{
    m_world->DestroyPhysicalEntity(body);
    m_bodies.erase(std::remove(m_bodies.begin(), m_bodies.end(), body), m_bodies.end());
}
IPhysicalEntity* CryActionRegressionBreakFixture::CreateBody(IEntity* entity, int jointId)
{
    pe_params_pos position; position.pos = Vec3(0,0,10); position.q = IDENTITY;
    auto* body = m_world->CreatePhysicalEntity(PE_RIGID, &position, entity, PHYS_FOREIGN_ID_ENTITY);
    if (!body) return nullptr;
    m_bodies.push_back(body);
    primitives::box box; box.Basis.SetIdentity(); box.bOriented = 0;
    box.center = ZERO; box.size = Vec3(.5f,.5f,.5f);
    auto* geometry = m_world->GetGeomManager()->CreatePrimitive(primitives::box::type, &box);
    if (!geometry) return nullptr;
    auto* registered = m_world->GetGeomManager()->RegisterGeometry(geometry);
    geometry->Release();
    if (!registered) return nullptr;
    pe_geomparams params; params.mass = 1;
    params.pos = Vec3(-.5f,0,0); body->AddGeometry(registered, &params, 0);
    params.pos = Vec3(.5f,0,0); body->AddGeometry(registered, &params, 1);
    m_world->GetGeomManager()->UnregisterGeometry(registered);
    pe_params_structural_joint joint;
    joint.id = jointId; joint.partid[0] = 0; joint.partid[1] = 1;
    joint.pt = ZERO; joint.n = Vec3(1,0,0);
    joint.maxForcePush = joint.maxForcePull = joint.maxForceShift = 1000;
    joint.maxTorqueBend = joint.maxTorqueTwist = 1000;
    joint.bBreakable = 1; joint.bBroken = 0;
    return body->SetParams(&joint) ? body : nullptr;
}

IStatObj* CryActionRegressionBreakFixture::CreatePlane()
{
    IStatObj* object = gEnv->p3DEngine->CreateStatObj();
    if (!object) return nullptr;
    object->SetMaterial(gEnv->p3DEngine->GetMaterialManager()->GetDefaultMaterial());
    IIndexedMesh* indexed = object->GetIndexedMesh();
    indexed->SetVertexCount(8);
    indexed->SetTexCoordsAndTangentsCount(8);
    indexed->SetIndexCount(12);
    CMesh* mesh = indexed->GetMesh();
    const Vec3 corners[] = {Vec3(-2,-2,0),Vec3(2,-2,0),Vec3(2,2,0),Vec3(-2,2,0)};
    for (int i = 0; i != 8; ++i)
    {
        mesh->m_pPositions[i] = corners[i % 4] + Vec3(0,0,i < 4 ? .02f : -.02f);
        mesh->m_pNorms[i] = Vec3(0,0,i < 4 ? 1.f : -1.f);
        mesh->m_pTexCoord[i].s = (corners[i % 4].x + 2) / 4;
        mesh->m_pTexCoord[i].t = (corners[i % 4].y + 2) / 4;
        mesh->m_pTangents[i].Tangent = Vec4sf(32767,0,0,-32768);
        mesh->m_pTangents[i].Binormal = Vec4sf(0,32767,0,-32768);
    }
    const uint16 indices[] = {0,1,2,0,2,3,4,6,5,4,7,6};
    std::copy(std::begin(indices), std::end(indices), mesh->m_pIndices);
    primitives::box box; box.Basis.SetIdentity(); box.bOriented = 0;
    box.center = ZERO; box.size = Vec3(2,2,.02f);
    auto* geometry = m_world->GetGeomManager()->CreatePrimitive(primitives::box::type, &box);
    if (!geometry) { object->Release(); return nullptr; }
    object->SetPhysGeom(m_world->GetGeomManager()->RegisterGeometry(geometry));
    geometry->Release();
    return object;
}

void CryActionRegressionBreakFixture::ClearPlaneGrid(IStatObj* object)
{
    // ActionGame owns this cleanup during broken-object restoration.
    if (!object || !object->GetPhysGeom()) return;
    auto* geometry = object->GetPhysGeom()->pGeom;
    auto* grid = static_cast<CBreakablePlane*>(geometry->GetForeignData(1));
    geometry->SetForeignData(nullptr, 1);
    delete grid;
}

void CryActionRegressionProbe::BreakRepairCases(CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    REPAIR_REQUIRE(gEnv->pPhysicalWorld, "Local CryPhysics world unavailable");
    CryActionRegressionBoundaryFixture fixture(check, context);
    CNetworkCVars network;
    network.BreakageLog = testCase == CryActionRegressionCase::BreakDump ? 1 : 0;
    CryActionRegressionDiagnostics diagnostics;
    CryActionRegressionEntities entities;
    gEnv->pEntitySystem = &entities;
    if (testCase == CryActionRegressionCase::BreakMalformed)
    {
        IProceduralBreakTypePtr type = new CJointBreak;
        _smart_ptr<CProceduralBreakingPlaybackStream> stream = new CProceduralBreakingPlaybackStream(type);
        REPAIR_CHECK(!type->GotExplosiveObjectState(nullptr));
        REPAIR_CHECK(!stream->GotJointBreakRec(nullptr));
        REPAIR_CHECK(!stream->GotProceduralSpawnRec(nullptr));
        SProceduralSpawnRec spawn;
        REPAIR_CHECK(!stream->GotProceduralSpawnRec(&spawn));
        spawn.op = static_cast<ENetBreakOperation>(-1);
        REPAIR_CHECK(!stream->GotProceduralSpawnRec(&spawn));
        SJointBreakRec joint{}; joint.idxRef = INT_MIN;
        REPAIR_CHECK(!stream->GotJointBreakRec(&joint));
        CProceduralBreakingPlaybackListener playback(stream, nullptr);
        REPAIR_CHECK(playback.GetObjectSelector(INT_MIN).Find() == nullptr);
        REPAIR_CHECK(stream->m_spawnRecs.empty() && stream->m_jointBreaks.empty());
        return;
    }
    if (testCase == CryActionRegressionCase::BreakChunks)
    {
        REPAIR_REQUIRE(gEnv->p3DEngine, "Chunk owner needs the real 3D engine");
        REPAIR_REQUIRE(gEnv->p3DEngine->InitLevelForEditor("__cryaction_regression_no_level__", "Regression"),
            "Real stat-object lifetime requires the editor object manager");
        _smart_ptr<IStatObj> object = gEnv->p3DEngine->CreateStatObj();
        REPAIR_REQUIRE(object, "Real empty stat object unavailable");
        for (int count : {0, 1, 3})
        {
            std::vector<EntityId> chunks(count, 99);
            const auto before = chunks;
            SBreakEvent event{};
            IStatObj* result = CBreakablePlane::ProcessImpact(Vec3(0,0,0), Vec3(0,0,1), 1, 1,
                object, Matrix34(IDENTITY), nullptr, nullptr, 1, event, chunks, false);
            REPAIR_CHECK(result == object && event.idxChunkId0 == count && event.nChunks == 0 && chunks == before);
            for (const auto& range : {std::pair<int,int>{-1,0}, {0,-1}, {count+1,0}, {count,1}, {INT_MAX,INT_MAX}})
            {
                event.idxChunkId0 = range.first; event.nChunks = range.second;
                result = CBreakablePlane::ProcessImpact(Vec3(0,0,0), Vec3(0,0,1), 1, 1,
                    object, Matrix34(IDENTITY), nullptr, nullptr, 1, event, chunks, true);
                REPAIR_CHECK(result == object && chunks == before);
                REPAIR_CHECK(event.idxChunkId0 == range.first && event.nChunks == range.second);
            }
        }
        REPAIR_CHECK(diagnostics.Contains("invalid chunk range"));
        gEnv->pEntitySystem = fixture.savedEnvironment.pEntitySystem;
        CryActionRegressionBreakFixture world(gEnv->pPhysicalWorld);
        auto* surface = gEnv->p3DEngine->GetMaterialManager()->GetSurfaceType(0);
        REPAIR_REQUIRE(surface, "Real default surface unavailable");
        for (int prefix : {0,1,3})
        {
            IStatObj* plane = world.CreatePlane();
            REPAIR_REQUIRE(plane && plane->GetPhysGeom(), "Real breakable-plane mesh/geometry unavailable");
            plane->AddRef();
            std::vector<EntityId> chunks(prefix, 99);
            SBreakEvent event{};
            IStatObj* remaining = CBreakablePlane::ProcessImpact(Vec3(.1f,.2f,0), Vec3(0,0,1), 1, 1,
                plane, Matrix34(IDENTITY), surface, plane->GetMaterial(), 1, event, chunks, false);
            REPAIR_CHECK(event.idxChunkId0 == prefix && event.nChunks > 0);
            REPAIR_CHECK(chunks.size() == size_t(prefix) + event.nChunks);
            for (int i = 0; i != prefix; ++i) REPAIR_CHECK(chunks[i] == 99);
            std::set<EntityId> generated;
            for (size_t i = prefix; i < chunks.size(); ++i)
            {
                REPAIR_CHECK(chunks[i] != 0 && generated.insert(chunks[i]).second);
                REPAIR_CHECK(gEnv->pEntitySystem->GetEntity(chunks[i]));
                gEnv->pEntitySystem->RemoveEntity(chunks[i], true);
            }
            world.ClearPlaneGrid(remaining);
            if (remaining && remaining != plane) remaining->Release();
            plane->Release();

            // Recreate the geometry, then replay the recorded seed and stable
            // chunk IDs through the owner's loading branch.
            plane = world.CreatePlane();
            REPAIR_REQUIRE(plane && plane->GetPhysGeom(), "Fresh chunk replay geometry unavailable");
            plane->AddRef();
            const auto recordedIds = chunks;
            const SBreakEvent recordedEvent = event;
            remaining = CBreakablePlane::ProcessImpact(Vec3(.1f,.2f,0), Vec3(0,0,1), 1, 1,
                plane, Matrix34(IDENTITY), surface, plane->GetMaterial(), 1, event, chunks, true);
            REPAIR_CHECK(chunks == recordedIds && event.idxChunkId0 == recordedEvent.idxChunkId0 && event.nChunks == recordedEvent.nChunks);
            for (size_t i = prefix; i < chunks.size(); ++i)
            {
                REPAIR_CHECK(gEnv->pEntitySystem->GetEntity(chunks[i]));
                gEnv->pEntitySystem->RemoveEntity(chunks[i], true);
            }
            world.ClearPlaneGrid(remaining);
            if (remaining && remaining != plane) remaining->Release();
            plane->Release();
        }
        return;
    }
    if (testCase == CryActionRegressionCase::BreakDump)
    {
        DynArray<SProceduralSpawnRec> spawns;
        DynArray<SJointBreakRec> joints;
        DumpSpawnRecs(spawns, joints, "empty-regression");
        for (int i = 0; i != 3; ++i)
        {
            SProceduralSpawnRec spawn; spawn.op = eNBO_Create; spawn.idxRef = -i-1; spawn.idx = i; spawn.partid = 41+i;
            spawns.push_back(spawn);
            SJointBreakRec joint{}; joint.idxRef = -i-1; joint.id = 11+i; joint.epicenter = 31+i; joint.frame = i;
            joints.push_back(joint);
            if (i == 0) DumpSpawnRecs(spawns, joints, "single-regression");
        }
        DumpSpawnRecs(spawns, joints, "three-regression");
        REPAIR_CHECK(diagnostics.Contains("empty-regression") && diagnostics.Contains("single-regression") && diagnostics.Contains("three-regression"));
        REPAIR_CHECK(diagnostics.Contains("[000] create on -1 part 41 -> 0"));
        REPAIR_CHECK(diagnostics.Contains("[002] create on -3 part 43 -> 2"));
        REPAIR_CHECK(diagnostics.Contains(".break 13 on -3 epipart=33 @ frame 2"));
        REPAIR_CHECK(spawns.size() == 3 && joints.size() == 3);
        return;
    }
    if (testCase == CryActionRegressionCase::BreakReferences)
    {
        std::array<CryActionRegressionEntity,3> objects;
        std::array<CryActionRegressionPhysicalEntity,3> physical;
        IProceduralBreakTypePtr type = new CJointBreak;
        for (int i = 0; i != 3; ++i)
        {
            objects[i].id = 100+i; objects[i].physics = &physical[i]; physical[i].entity = &objects[i];
            entities.entities.emplace(objects[i].id, &objects[i]);
        }
        SExplosiveObjectState state{}; state.isEnt = true; state.entId = 100;
        REPAIR_CHECK(type->GotExplosiveObjectState(&state));
        _smart_ptr<CProceduralBreakingPlaybackStream> stream = new CProceduralBreakingPlaybackStream(type);
        for (int reference : {-1,0,3,99})
        {
            SJointBreakRec record{}; record.idxRef = reference; record.id = 11; record.epicenter = 0;
            REPAIR_CHECK(stream->GotJointBreakRec(&record));
        }
        CProceduralBreakingPlaybackListener playback(stream, nullptr);
        playback.m_localEntityIds.insert(std::make_pair(101,0));
        playback.m_localEntityIds.insert(std::make_pair(102,3));
        playback.Start();
        REPAIR_CHECK(physical[0].joints.size() == 1 && physical[1].joints.empty() && physical[2].joints.empty());
        REPAIR_CHECK(stream->m_jointBreaks.size() == 3);
        playback.FinishBreakOp(eNBO_Create, -1, 101, 0);
        playback.FinishBreakOp(eNBO_Create, -1, 102, 3);
        REPAIR_CHECK(physical[1].joints.size() == 1 && physical[2].joints.size() == 1);
        REPAIR_CHECK(stream->m_jointBreaks.size() == 1 && stream->m_jointBreaks[0].idxRef == 99);
        REPAIR_CHECK(!playback.AllowComplete() && playback.GetObjectSelector(99).Find() == nullptr);
        return;
    }
    // Record through the real event owner, serialize through both production
    // script-table serializers, then replay through the real listener.
    for (int count : {0,1,3})
    {
        std::array<CryActionRegressionEntity,3> objects;
        std::array<CryActionRegressionPhysicalEntity,3> physical;
        CryActionRegressionBreakFixture world(gEnv->pPhysicalWorld);
        std::array<IPhysicalEntity*,3> bodies{};
        IProceduralBreakTypePtr type = new CJointBreak;
        for (int i = 0; i != count; ++i)
        {
            objects[i].id = 100+i;
            if (testCase == CryActionRegressionCase::BreakPhysics)
            {
                bodies[i] = world.CreateBody(&objects[i], 11+i);
                REPAIR_REQUIRE(bodies[i], "Real two-part structural-joint body unavailable");
            }
            else { bodies[i] = &physical[i]; physical[i].entity = &objects[i]; }
            objects[i].physics = bodies[i];
            entities.entities[objects[i].id] = &objects[i];
            SExplosiveObjectState state{}; state.isEnt = true; state.entId = objects[i].id;
            REPAIR_CHECK(type->GotExplosiveObjectState(&state));
        }
        CProceduralBreakingRecordingListener recording(type);
        REPAIR_CHECK(recording.m_jointBreaks.empty() && recording.m_spawnRecs.empty());
        for (int i = 0; i != count; ++i)
        {
            if (testCase == CryActionRegressionCase::BreakPhysics)
            {
                pe_params_structural_joint broken; broken.id = 11+i;
                broken.bBroken = 1; broken.bReplaceExisting = 1; broken.partidEpicenter = 0;
                REPAIR_CHECK(bodies[i]->SetParams(&broken));
                pe_params_structural_joint observed; observed.id = 11+i;
                REPAIR_CHECK(bodies[i]->GetParams(&observed) && observed.bBroken);
            }
            EventPhysJointBroken event{}; event.pEntity[0] = bodies[i]; event.idJoint = 11+i; event.partidEpicenter = 0;
            REPAIR_CHECK(recording.AcceptJointBroken(&event));
            event.idJoint = 1000000;
            REPAIR_CHECK(recording.AcceptJointBroken(&event));
            REPAIR_CHECK(recording.m_jointBreaks.size() == i+1); // Synthetic tree joint is filtered.
        }
        _smart_ptr<CProceduralBreakingPlaybackStream> stream = new CProceduralBreakingPlaybackStream(type);
        for (int i = 0; i != count; ++i)
        {
            SmartScriptTable table(gEnv->pScriptSystem);
            CSerializeScriptTableWriterImpl writer(table);
            CSimpleSerialize<CSerializeScriptTableWriterImpl> output(writer);
            recording.m_jointBreaks[i].SerializeWith(TSerialize(&output));
            int reference = 0, id = 0, frame = -1, epicenter = -1;
            REPAIR_CHECK(table->GetValue("ref", reference) && reference == -i-1);
            REPAIR_CHECK(table->GetValue("id", id) && id == 11+i);
            REPAIR_CHECK(table->GetValue("frame", frame) && frame == 0);
            REPAIR_CHECK(table->GetValue("epicenter", epicenter) && epicenter == 0);
            CSerializeScriptTableReaderImpl reader(table);
            CSimpleSerialize<CSerializeScriptTableReaderImpl> input(reader);
            SJointBreakRec decoded{}; decoded.SerializeWith(TSerialize(&input));
            REPAIR_CHECK(input.Ok() && stream->GotJointBreakRec(&decoded));
            if (testCase == CryActionRegressionCase::BreakPhysics)
            {
                world.Destroy(bodies[i]);
                bodies[i] = world.CreateBody(&objects[i], 11+i);
                REPAIR_REQUIRE(bodies[i], "Fresh replay structural-joint body unavailable");
                objects[i].physics = bodies[i];
                pe_params_structural_joint joint; joint.id = 11+i;
                REPAIR_CHECK(bodies[i]->GetParams(&joint) && !joint.bBroken);
            }
        }
        CProceduralBreakingPlaybackListener playback(stream, nullptr);
        playback.Start();
        REPAIR_CHECK(stream->m_jointBreaks.empty() && playback.AllowComplete());
        for (int i = 0; i != count; ++i)
        {
            if (testCase == CryActionRegressionCase::BreakPhysics)
            {
                pe_params_structural_joint joint; joint.id = 11+i;
                REPAIR_CHECK(bodies[i]->GetParams(&joint) && joint.bBroken);
            }
            else REPAIR_CHECK(physical[i].joints.size() == 1 && physical[i].joints[0].id == 11+i);
        }
        if (testCase == CryActionRegressionCase::BreakRecord)
        {
            for (int i = 0; i != count; ++i)
                REPAIR_CHECK(recording.GotBreakOp(eNBO_Create, -1, 41+i, 200+i) == i);
            REPAIR_CHECK(recording.m_ents.size() == count && recording.m_spawnRecs.size() == count);
            for (int i = 0; i != count; ++i)
            {
                SmartScriptTable table(gEnv->pScriptSystem);
                CSerializeScriptTableWriterImpl writer(table);
                CSimpleSerialize<CSerializeScriptTableWriterImpl> output(writer);
                recording.m_spawnRecs[i].SerializeWith(TSerialize(&output));
                int operation = -1, part = -1, index = -1, reference = 0;
                REPAIR_CHECK(table->GetValue("typ",operation) && operation == eNBO_Create);
                REPAIR_CHECK(table->GetValue("partid",part) && part == 41+i);
                REPAIR_CHECK(table->GetValue("idx",index) && index == i);
                REPAIR_CHECK(table->GetValue("idxRef",reference) && reference == -1);
                CSerializeScriptTableReaderImpl reader(table);
                CSimpleSerialize<CSerializeScriptTableReaderImpl> input(reader);
                SProceduralSpawnRec decoded;
                decoded.SerializeWith(TSerialize(&input));
                REPAIR_CHECK(input.Ok() && decoded.op == eNBO_Create && decoded.partid == 41+i && decoded.idx == i && decoded.idxRef == -1);
            }
            if (count) REPAIR_CHECK(recording.GetEntRef(200) == 0 && recording.m_ents.size() == count);
            recording.GotBreakOp(eNBO_PostStep, -1, -1, 0);
            recording.GotBreakOp(eNBO_PostStep, -1, -1, 0);
            REPAIR_CHECK(recording.m_spawnRecs.size() == count+1 && recording.m_frame == 1);
            recording.GotJointBreak(-1, 77, 4);
            REPAIR_CHECK(recording.m_jointBreaks.back().frame == 1);
        }
        entities.entities.clear();
    }
}
