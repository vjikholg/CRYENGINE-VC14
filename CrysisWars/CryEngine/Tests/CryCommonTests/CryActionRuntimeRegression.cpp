#include "StdAfx.h"
#include "CryActionRegressionProbe.h"
#include "CryActionRuntimeRegression.h"
#include <AIReadabilityManager.h>
#include <AnimationGraph/AnimationGraphManager.h>
#include <AnimationGraph/AnimationGraphState.h>
#include <AnimationGraph/AnimationGraphStates.h>
#include <DialogSystem/DialogScript.h>
#include <GameplayRecorder/GameplayRecorder.h>
#include <SimpleHttpServer/SimpleHttpServerListener.h>
#include <Network/GameQueryListener.h>
#include <FlowSystem/FlowSystem.h>
#include <FlowSystem/FlowData.h>
#include <TimeDemo/TimeDemoRecorder.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <type_traits>

#define RUNTIME_CHECK(expression) check(context, (expression) ? 1u : 0u, #expression, __FILE__, __LINE__)

CryActionRegressionEvents::CryActionRegressionEvents(int identity, std::vector<int>* order)
    : m_identity(identity), m_order(order) {}
void CryActionRegressionEvents::SetOutput(const char* name, const char* value)
{
    outputs.push_back(std::string(name) + "=" + value);
    if (m_order) m_order->push_back(m_identity);
}
void CryActionRegressionEvents::QueryComplete(TAnimationGraphQueryID, bool) { ++queries; }
void CryActionRegressionEvents::DestroyedState(IAnimationGraphState*) { ++destroyed; }
void CryActionRegressionEvents::OnGameplayEvent(IEntity*, const GameplayEvent& event)
{
    // Bounded callback storage avoids involving the engine pool's STL allocation
    // annotations in an observer whose maximum expected transcript is five events.
    if (gameplayCount >= events.size()) std::abort();
    events[gameplayCount] = event.event;
    payloads[gameplayCount] = reinterpret_cast<std::uintptr_t>(event.extra);
    descriptions[gameplayCount++] = event.description ? event.description : "";
    if (m_order) m_order->push_back(m_identity);
}

CryActionRegressionView::CryActionRegressionView(ISystem* system) : CView(system) {}
void CryActionRegressionView::ProcessShake(SShake* shake, float seconds)
{
    visited.push_back(shake->ID);
    CView::ProcessShake(shake, seconds);
}
const std::vector<CView::SShake>& CryActionRegressionView::Shakes() const { return m_shakes; }

void CryActionRegressionHttp::Start(uint16, const string&, IHttpServerListener*) { ++starts; }
void CryActionRegressionHttp::Stop() {}
void CryActionRegressionHttp::SendResponse(EStatusCode status, EContentType, const string& body, bool close)
{
    bodies.emplace_back(body.c_str());
    statuses.push_back(status);
    closes.push_back(close);
}
void CryActionRegressionHttp::SendWebpage(const string& body) { bodies.emplace_back(body.c_str()); }

CryActionRegressionFlowFactory::CryActionRegressionFlowFactory(unsigned& destructions)
    : m_destructions(destructions) {}
void CryActionRegressionFlowFactory::AddRef() { ++m_refs; }
void CryActionRegressionFlowFactory::Release()
{
    if (--m_refs == 0) { ++m_destructions; delete this; }
}
IFlowNodePtr CryActionRegressionFlowFactory::Create(IFlowNode::SActivationInfo* info) { return new CryActionRegressionFlowNode(info); }
void CryActionRegressionFlowFactory::GetMemoryStatistics(ICrySizer* s) { s->Add(*this); }

CryActionRegressionFlowNode::CryActionRegressionFlowNode(SActivationInfo* info)
    : m_values(static_cast<std::vector<int>*>(info->m_pUserData)) {}
IFlowNodePtr CryActionRegressionFlowNode::Clone(SActivationInfo* info) { return new CryActionRegressionFlowNode(info); }
void CryActionRegressionFlowNode::GetConfiguration(SFlowNodeConfig& config)
{
    static const SInputPortConfig inputs[]{InputPortConfig<int>("Value", 0), {0}};
    static const SOutputPortConfig outputs[]{OutputPortConfig<int>("Value"), {0}};
    config.pInputPorts = inputs;
    config.pOutputPorts = outputs;
}
void CryActionRegressionFlowNode::ProcessEvent(EFlowEvent event, SActivationInfo* info)
{
    if (event != eFE_Activate || !IsPortActive(info, 0)) return;
    const int value = GetPortInt(info, 0);
    if (m_values) m_values->push_back(value);
    ActivateOutput(info, 0, value);
}
void CryActionRegressionFlowNode::GetMemoryStatistics(ICrySizer* s) { s->Add(*this); }

std::uint32_t CryActionRegressionProbe::RuntimeCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    if (testCase == CryActionRegressionCase::HttpNoParams)
    {
        HttpCallbacks(testCase, check, context);
        return 0;
    }
    if (testCase >= CryActionRegressionCase::FlowRegistration && testCase <= CryActionRegressionCase::FlowGraphCleanup)
    {
        if (!gEnv->pEntitySystem || !gEnv->pEntitySystem->GetClassRegistry())
        {
            check(context, 2, "CFlowSystem construction requires an initialized IEntitySystem/class registry; preview does not provide one", __FILE__, __LINE__);
            return 2;
        }
        FlowRegistry(testCase, check, context);
    }
    else if (testCase <= CryActionRegressionCase::ReadabilityHistory) Readability(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::ViewReset) ViewShakes(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::DialogBounds) DialogCounts(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::RichXmlWrongTag) RichXml(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::PathRepeated) GraphPaths(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::LayersDestroyed) GraphLayers(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::GameplayIds) GameplayCallbacks(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::HttpEscaping) HttpCallbacks(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::ServerDuplicate) ServerLifecycle(testCase, check, context);
    else TimeDemoFixture(testCase, check, context);
    return 0;
}

void CryActionRegressionProbe::Readability(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    using Group = CAIReadabilityManager::ReadabilitySoundGroup;
    Group group;
    const int count = testCase == CryActionRegressionCase::ReadabilityEmpty ? 0 :
        testCase == CryActionRegressionCase::ReadabilitySingle ? 1 : 3;
    for (int i = 0; i < count; ++i)
    {
        string filename = i == 0 ? "first.wav" : i == 1 ? "second.wav" : "third.wav";
        group.m_aloneSounds.emplace_back(filename, 1, i != 1);
    }
    if (testCase == CryActionRegressionCase::ReadabilityFallback)
        group.m_groupSounds.swap(group.m_aloneSounds);
    auto& sounds = testCase == CryActionRegressionCase::ReadabilityFallback ? group.m_groupSounds : group.m_aloneSounds;
    int previous = -1, beforePrevious = -1;
    for (int i = 0; i < 64; ++i)
    {
        auto* selected = group.GetMostLikelySound(Group::SND_ALONE);
        RUNTIME_CHECK((selected != nullptr) == (count != 0));
        if (!selected) continue;
        const auto found = std::find_if(sounds.begin(), sounds.end(), [&](const auto& sound) { return &sound == selected; });
        RUNTIME_CHECK(found != sounds.end());
        if (found == sounds.end()) return;
        const int index = static_cast<int>(found - sounds.begin());
        RUNTIME_CHECK(selected->m_probability == 1 && selected->voice == (index != 1));
        if (count == 3) RUNTIME_CHECK(index != previous && index != beforePrevious);
        beforePrevious = previous;
        previous = index;
    }
    RUNTIME_CHECK(group.GetMostLikelySound(Group::SND_RESPONSE) == nullptr);
    if (testCase == CryActionRegressionCase::ReadabilityHistory)
    {
        Group::SoundHistory history;
        RUNTIME_CHECK(!history.Find(7));
        history.Add(7); history.Add(9);
        RUNTIME_CHECK(history.Find(7) && history.Find(9));
        history.Add(11);
        RUNTIME_CHECK(!history.Find(7) && history.Find(9) && history.Find(11));
        history.Reset();
        RUNTIME_CHECK(!history.Find(9) && !history.Find(11));
    }
}

void CryActionRegressionProbe::ViewShakes(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    // The constructor resolves this production CVar; the preview system has no game CVars.
    ICVar* multiplier = gEnv->pConsole->GetCVar("c_shakeMult");
    if (!multiplier) multiplier = gEnv->pConsole->RegisterFloat("c_shakeMult", 1.0f, 0);
    RUNTIME_CHECK(multiplier != nullptr);
    if (!multiplier) return;
    const float previous = multiplier->GetFVal();
    multiplier->Set(1.0f);
    CryActionRegressionView view(gEnv->pSystem);
    const int count = testCase == CryActionRegressionCase::ViewEmpty ? 0 :
        testCase == CryActionRegressionCase::ViewSingle ? 1 : 3;
    for (int i = 0; i < count; ++i)
        view.SetViewShake(Ang3(0, 0, 0), Vec3(static_cast<float>(i + 1), 0, 0), 1.0f, 0.25f, 0.0f, 10 + i, false);
    if (testCase == CryActionRegressionCase::ViewUpdate)
        view.SetViewShake(Ang3(1, 2, 3), Vec3(99, 0, 0), 99, 0.5f, 1.0f, 11, true, true);
    RUNTIME_CHECK(view.Shakes().size() == count);
    for (int i = 0; i < count; ++i)
    {
        RUNTIME_CHECK(view.Shakes()[i].ID == 10 + i && view.Shakes()[i].duration == 1.0f);
        RUNTIME_CHECK(view.Shakes()[i].amountVector.x == i + 1 && view.Shakes()[i].randomness == 0);
        RUNTIME_CHECK(view.Shakes()[i].frequency == (testCase == CryActionRegressionCase::ViewUpdate && i == 1 ? 0.5f : 0.25f));
    }
    view.ProcessShaking(0.25f);
    RUNTIME_CHECK(view.visited.size() == count);
    for (int i = 0; i < count; ++i) RUNTIME_CHECK(view.visited[i] == 10 + i);
    if (testCase != CryActionRegressionCase::ViewUpdate)
        RUNTIME_CHECK(view.GetCurrentParams()->currentShakeShift.IsEquivalent(Vec3(count == 3 ? 6.0f : static_cast<float>(count), 0, 0), 0.00001f));
    if (testCase == CryActionRegressionCase::ViewReset)
    {
        view.ResetShaking();
        view.ProcessShaking(0.25f);
        RUNTIME_CHECK(view.GetCurrentParams()->currentShakeShift.IsZero());
        RUNTIME_CHECK(view.GetCurrentParams()->shakingRatio == 0);
        for (const auto& shake : view.Shakes()) RUNTIME_CHECK(!shake.updating && shake.duration == -1);
    }
    multiplier->Set(previous);
}

void CryActionRegressionProbe::DialogCounts(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    static_assert(std::is_same_v<decltype(std::declval<CDialogScript>().GetNumLines()), int>);
    static_assert(sizeof(CDialogScript::TActorID) == 1);
    CDialogScript script("fixture.dialog");
    const int count = testCase == CryActionRegressionCase::DialogEmpty ? 0 :
        testCase == CryActionRegressionCase::DialogSingle ? 1 : 3;
    const char* sounds[]{"first", "second", "third"};
    for (int i = 0; i < count; ++i)
        RUNTIME_CHECK(script.AddLine(static_cast<uint8>(i * 2), sounds[i], "anim", "face", CDialogScript::NO_ACTOR_ID, 0.25f, 1, 0.5f, false, false, false, false, true, false));
    RUNTIME_CHECK(!script.IsCompleted() && script.GetNumLines() == 0 && script.GetNumRequiredActors() == 0);
    RUNTIME_CHECK(script.Complete());
    RUNTIME_CHECK(script.IsCompleted() && script.GetNumLines() == count && script.GetNumRequiredActors() == count);
    for (int i = 0; i < count; ++i)
    {
        const auto* line = script.GetLine(i);
        RUNTIME_CHECK(line && line->m_actor == i * 2 && line->m_sound == sounds[i] && line->m_delay == 0.25f);
    }
    RUNTIME_CHECK(script.GetLine(-1) == nullptr && script.GetLine(count) == nullptr);
    if (testCase == CryActionRegressionCase::DialogBounds)
    {
        RUNTIME_CHECK(!script.AddLine(8, "invalid", "", "", CDialogScript::NO_ACTOR_ID, 0, 0, 0, false, false, false, false, false, false));
        RUNTIME_CHECK(script.GetNumLines() == count && script.IsCompleted());
        RUNTIME_CHECK(script.AddLine(7, "last", "", "", 0, 0, 0, 0, false, false, false, false, false, false));
        RUNTIME_CHECK(!script.IsCompleted() && script.GetNumLines() == 0);
        RUNTIME_CHECK(script.Complete() && script.GetNumLines() == count + 1 && script.GetNumRequiredActors() == count + 1);
    }
}

void CryActionRegressionProbe::RichXml(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    const auto path = ScratchPath("rich-chunk.bin");
    FILE* file = OpenFixture(path, "w+b");
    RUNTIME_CHECK(file != nullptr);
    if (!file) return;
    const auto close = [&](FILE* handle) { gEnv->pCryPak->FClose(handle); DeleteFileA(path.c_str()); };
    std::unique_ptr<FILE, decltype(close)> guard(file, close);
    constexpr uint32 tag = 0x54534554; // TEST, literal little-endian chunk tag
    const bool compressed = testCase == CryActionRegressionCase::RichXmlCompressed;
    XmlNodeRef node;
    if (testCase != CryActionRegressionCase::RichXmlNull)
    {
        node = gEnv->pSystem->CreateXmlNode("Fixture");
        node->setAttr("id", 0xfffffffeu);
        node->newChild("first")->setAttr("value", "a&b");
        node->newChild("second")->setContent(std::string(4096, 'x').c_str());
    }
    RUNTIME_CHECK(WriteXMLNode(tag, node, file, compressed, ""));
    const long end = gEnv->pCryPak->FTell(file);
    RUNTIME_CHECK(end >= 8);
    gEnv->pCryPak->FSeek(file, 0, SEEK_SET);
    std::array<uint8, 12> header{};
    RUNTIME_CHECK(gEnv->pCryPak->FReadRaw(header.data(), 1, compressed ? 12 : 8, file) == (compressed ? 12 : 8));
    uint32 payload = 0;
    std::memcpy(&payload, header.data() + 4, 4);
    RUNTIME_CHECK(header[0] == 'T' && header[1] == 'E' && header[2] == 'S' && header[3] == 'T');
    RUNTIME_CHECK(payload + size_t{8} == end);
    if (!node) RUNTIME_CHECK(payload == 0 && end == 8);
    if (compressed)
    {
        uint32 uncompressed = 0;
        std::memcpy(&uncompressed, header.data() + 8, 4);
        RUNTIME_CHECK(uncompressed > 4096 && payload < uncompressed);
    }
    gEnv->pCryPak->FSeek(file, 0, SEEK_SET);
    uint32 observed = 0;
    RUNTIME_CHECK(ReadTag(file, observed, true) && observed == tag && gEnv->pCryPak->FTell(file) == 0);
    if (testCase == CryActionRegressionCase::RichXmlWrongTag)
    {
        RUNTIME_CHECK(!SkipXMLTagData(tag + 1, file));
        RUNTIME_CHECK(gEnv->pCryPak->FTell(file) == 0);
        RUNTIME_CHECK(ReadXMLTagData(tag + 1, file, false) == nullptr);
        return;
    }
    if (testCase == CryActionRegressionCase::RichXmlTruncated)
    {
        // Make a small physically truncated chunk, never an oversized allocation.
        guard.reset();
        file = OpenFixture(path, "w+b");
        RUNTIME_CHECK(file != nullptr);
        if (!file) return;
        guard.reset(file);
        const std::array<uint8, 10> truncated{'T', 'E', 'S', 'T', 4, 0, 0, 0, '<', 'x'};
        gEnv->pCryPak->FWrite(truncated.data(), truncated.size(), file);
        gEnv->pCryPak->FSeek(file, 0, SEEK_SET);
        RUNTIME_CHECK(ReadXMLTagData(tag, file, false) == nullptr);
        return;
    }
    if (testCase == CryActionRegressionCase::RichXmlSequence)
    {
        gEnv->pCryPak->FSeek(file, end, SEEK_SET);
        RUNTIME_CHECK(WriteXMLNode(tag + 1, XmlNodeRef(), file, false, ""));
        gEnv->pCryPak->FSeek(file, 0, SEEK_SET);
        RUNTIME_CHECK(SkipXMLTagData(tag, file) && gEnv->pCryPak->FTell(file) == end);
        RUNTIME_CHECK(ReadTag(file, observed, true) && observed == tag + 1);
        RUNTIME_CHECK(SkipXMLTagData(tag + 1, file) && gEnv->pCryPak->FTell(file) == end + 8);
        RUNTIME_CHECK(!ReadTag(file, observed, false));
        return;
    }
    std::unique_ptr<char[]> xml(ReadXMLTagData(tag, file, compressed));
    RUNTIME_CHECK(xml != nullptr);
    if (!xml) return;
    RUNTIME_CHECK(gEnv->pCryPak->FTell(file) == end);
    if (!node) { RUNTIME_CHECK(xml[0] == 0); return; }
    XmlNodeRef loaded = gEnv->pSystem->LoadXmlFromString(xml.get());
    RUNTIME_CHECK(loaded && loaded->isTag("Fixture") && loaded->getChildCount() == 2);
    if (!loaded || loaded->getChildCount() != 2) return;
    uint32 id = 0;
    RUNTIME_CHECK(loaded->getAttr("id", id) && id == 0xfffffffeu);
    RUNTIME_CHECK(loaded->getChild(0)->isTag("first") && std::strcmp(loaded->getChild(0)->getAttr("value"), "a&b") == 0);
    RUNTIME_CHECK(loaded->getChild(1)->isTag("second") && std::strlen(loaded->getChild(1)->getContent()) == 4096);
}

void CryActionRegressionProbe::GraphPaths(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CAnimationGraphManager manager;
    auto graph = CAnimationGraph::CreateInstance(&manager, "PathFixture");
    XmlNodeRef root = gEnv->pSystem->CreateXmlNode("AnimationGraph");
    root->newChild("Inputs"); root->newChild("Views");
    auto states = root->newChild("States");
    for (const char* name : {"Alpha", "Beta", "Gamma", "Delta"})
    {
        auto state = states->newChild("State");
        state->setAttr("id", name); state->setAttr("cost", 0); state->setAttr("allowSelect", false);
    }
    auto transitions = root->newChild("Transitions");
    const auto add = [&](const char* from, const char* to, int cost) {
        auto link = transitions->newChild("Link");
        link->setAttr("from", from); link->setAttr("to", to); link->setAttr("cost", cost);
    };
    const bool indirect = testCase == CryActionRegressionCase::PathIndirect || testCase == CryActionRegressionCase::PathRepeated;
    const bool tied = testCase == CryActionRegressionCase::PathTie;
    if (testCase != CryActionRegressionCase::PathImplicit)
    {
        if (!tied) add("Alpha", "Delta", 9);
        if (indirect || tied) { add("Alpha", "Beta", 1); add("Beta", "Delta", 1); }
        if (tied) { add("Alpha", "Gamma", 1); add("Gamma", "Delta", 1); }
    }
    RUNTIME_CHECK(graph->Init(root));
    CAnimationGraphState state(graph);
    const auto from = graph->StateNameToID("Alpha");
    const auto to = testCase == CryActionRegressionCase::PathSame ? from : graph->StateNameToID("Delta");
    const auto middle = graph->StateNameToID("Beta");
    const auto alternate = graph->StateNameToID("Gamma");
    for (int repeat = 0; repeat < (testCase == CryActionRegressionCase::PathRepeated ? 32 : 1); ++repeat)
    {
        CAnimationGraph::StateList path;
        CAnimationGraph::PathFindStats stats{};
        SAnimationMovement movement;
        CAnimationGraph::SPathFindParams params;
        params.pGraphState = &state;
        params.pCurInputValues = state.GetCurrentInputs();
        params.destinationStateID = to; params.pTransitions = &path; params.pStats = &stats; params.pMovement = &movement;
        params.randomNumber = 0x43525932;
        RUNTIME_CHECK(graph->PathFindBetweenStates(from, params) == eTS_true);
        if (testCase == CryActionRegressionCase::PathSame) { RUNTIME_CHECK(path.Empty()); continue; }
        RUNTIME_CHECK(path.Size() == (indirect || tied ? 2 : 1));
        if (path.Empty()) return;
        RUNTIME_CHECK(path.Back() == to);
        // LoadTransitions stores XML cost minus one, and CachePass adds state cost + one.
        if (indirect) RUNTIME_CHECK(path.Front() == middle && stats.finalCost == 2);
        if (tied) RUNTIME_CHECK((path.Front() == middle || path.Front() == alternate) && stats.finalCost == 2);
        if (testCase == CryActionRegressionCase::PathDirect) RUNTIME_CHECK(stats.finalCost == 9);
        if (testCase == CryActionRegressionCase::PathImplicit) RUNTIME_CHECK(stats.finalCost == 0xffffffffu);
        RUNTIME_CHECK(movement.translation.IsZero());
        // Consume the production path with the same queue interface as graph-state transitions.
        while (!path.Empty()) path.Pop();
        RUNTIME_CHECK(path.Empty());
    }
}

void CryActionRegressionProbe::GraphLayers(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    auto graph = CAnimationGraph::CreateInstance(nullptr, "LayerFixture");
    const int low = graph->DeclareOutput("stance", "low");
    const int high = graph->DeclareOutput("stance", "high");
    std::vector<int> order;
    CryActionRegressionEvents first(1, &order), second(2, &order);
    std::vector<std::unique_ptr<CAnimationGraphState>> layers;
    {
        CAnimationGraphStates wrapper;
        wrapper.AddListener("first", &first);
        wrapper.AddListener("first-again", &first); // pointer identity suppresses duplicates
        wrapper.AddListener("second", &second);
        const int count = testCase == CryActionRegressionCase::LayersEmpty ? 0 :
            testCase == CryActionRegressionCase::LayersSingle ? 1 : 3;
        for (int i = 0; i < count; ++i)
        {
            layers.push_back(std::make_unique<CAnimationGraphState>(graph));
            wrapper.AddLayerReference(layers.back().get());
        }
        RUNTIME_CHECK(wrapper.QueryOutput("missing") == nullptr && wrapper.QueryOutput("stance") == nullptr);
        for (int i = 0; i < count; ++i)
        {
            layers[i]->SetOutput(i == 0 ? low : high);
            RUNTIME_CHECK(first.outputs.size() == i + 1 && second.outputs.size() == i + 1);
            RUNTIME_CHECK(order[static_cast<size_t>(i) * 2] == 2 && order[static_cast<size_t>(i) * 2 + 1] == 1);
        }
        if (count)
        {
            RUNTIME_CHECK(std::strcmp(wrapper.QueryOutput("stance"), count == 1 ? "low" : "high") == 0);
            if (testCase == CryActionRegressionCase::LayersClear)
            {
                layers.back()->ClearOutput(high);
                RUNTIME_CHECK(wrapper.QueryOutput("stance") && std::strcmp(wrapper.QueryOutput("stance"), "") == 0);
                RUNTIME_CHECK(first.outputs.back() == "stance=" && second.outputs.back() == "stance=");
            }
            if (testCase == CryActionRegressionCase::LayersRemoveListener)
            {
                wrapper.RemoveListener(&first);
                layers.front()->SetOutput(high);
                RUNTIME_CHECK(first.outputs.size() == count && second.outputs.size() == count + 1);
            }
            if (testCase == CryActionRegressionCase::LayersDestroyed)
            {
                layers.back().reset();
                RUNTIME_CHECK(std::strcmp(wrapper.QueryOutput("stance"), "high") == 0);
                layers[1].reset();
                RUNTIME_CHECK(std::strcmp(wrapper.QueryOutput("stance"), "low") == 0);
            }
        }
        RUNTIME_CHECK(first.queries == 0 && second.queries == 0);
    }
    RUNTIME_CHECK(second.destroyed == 1 && first.destroyed == (testCase == CryActionRegressionCase::LayersRemoveListener ? 0u : 1u));
    // Wrapper references are borrowed: layers remain alive and listeners were detached.
    const size_t callbacks = second.outputs.size();
    if (!layers.empty() && layers.front()) layers.front()->SetOutput(low);
    RUNTIME_CHECK(second.outputs.size() == callbacks);
}

void CryActionRegressionProbe::GameplayCallbacks(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    std::vector<int> order;
    CryActionRegressionEvents first(1, &order), second(2, &order), third(3, &order);
    CGameplayRecorder recorder(nullptr);
    const int count = testCase == CryActionRegressionCase::GameplayEmpty ? 0 :
        testCase == CryActionRegressionCase::GameplaySingle ? 1 : 3;
    std::array<CryActionRegressionEvents*, 3> listeners{&first, &second, &third};
    for (int i = 0; i < count; ++i) recorder.RegisterListener(listeners[i]);
    if (count) recorder.RegisterListener(&first);
    const std::array<uint32, 4> ids{0, 1, 0x80000001u, 0xfffffffeu};
    const size_t eventCount = testCase == CryActionRegressionCase::GameplayIds ? ids.size() : 1;
    for (size_t eventIndex = 0; eventIndex < eventCount; ++eventIndex)
    {
        const auto id = ids[eventIndex];
        // Event.extra is an opaque, non-owning ID payload in these production callers.
        GameplayEvent event(eGE_EnteredVehicle, "fixture", 0.5f, reinterpret_cast<void*>(static_cast<std::uintptr_t>(id)));
        recorder.Event(nullptr, event);
        RUNTIME_CHECK(event.event == eGE_EnteredVehicle && event.value == 0.5f && std::strcmp(event.description, "fixture") == 0);
    }
    RUNTIME_CHECK(order.size() == eventCount * count);
    for (size_t event = 0; event < eventCount; ++event)
        for (int i = 0; i < count; ++i)
        {
            RUNTIME_CHECK(order[event * count + i] == i + 1);
            RUNTIME_CHECK(listeners[i]->payloads[event] == ids[event] && listeners[i]->events[event] == eGE_EnteredVehicle);
            RUNTIME_CHECK(listeners[i]->descriptions[event] == "fixture");
        }
    recorder.UnregisterListener(&second);
    recorder.UnregisterListener(&second);
    const size_t before = second.gameplayCount;
    recorder.Event(nullptr, GameplayEvent(eGE_LeftVehicle));
    RUNTIME_CHECK(second.gameplayCount == before);
    RUNTIME_CHECK(first.gameplayCount == (count ? eventCount + 1 : 0));
    RUNTIME_CHECK(recorder.GetSampleInterval().GetSeconds() == 0.5f);
}

void CryActionRegressionProbe::HttpCallbacks(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CryActionRegressionHttp transport;
    auto& listener = CSimpleHttpServerListener::GetSingleton(&transport);
    auto& callbacks = static_cast<IHttpServerListener&>(listener);
    ICVar* password = gEnv->pConsole->GetCVar("http_password");
    if (!password) password = gEnv->pConsole->RegisterString("http_password", "fixture-password", 0);
    RUNTIME_CHECK(password != nullptr && gEnv->pSystem->GetIDataProbe() != nullptr);
    if (!password || !gEnv->pSystem->GetIDataProbe()) return;
    const string previous = password->GetString();
    check(context, 3, "http: captured password CVar", __FILE__, __LINE__);
    password->Set("fixture-password");
    check(context, 3, "http: set password CVar", __FILE__, __LINE__);
    callbacks.OnClientConnected("local-fixture");
    check(context, 3, "http: client callback returned", __FILE__, __LINE__);
    const auto rpc = [&](const char* method, const char* parameter) {
        XmlNodeRef xml = gEnv->pSystem->CreateXmlNode("methodCall");
        xml->newChild("methodName")->setContent(method);
        auto parameters = xml->newChild("params");
        if (parameter) parameters->newChild("param")->newChild("value")->newChild("string")->setContent(parameter);
        // getXML() returns an owning CryString allocated in CrySystem. Its per-module
        // accounting cannot be transferred to this DLL; copy borrowed XML data locally.
        _smart_ptr<IXmlStringData> serialized = xml->getXMLData();
        callbacks.OnRpcRequest(serialized->GetString());
        check(context, 3, "http: request callback returned", __FILE__, __LINE__);
    };
    if (testCase == CryActionRegressionCase::HttpNoParams)
    {
        // OnRpcRequest explicitly documents parameterless commands as supported.
        callbacks.OnRpcRequest("<methodCall><methodName>challenge</methodName></methodCall>");
        listener.Update();
        RUNTIME_CHECK(transport.bodies.size() == 1 && transport.bodies.back().find("10.000000") != std::string::npos);
    }
    else if (testCase == CryActionRegressionCase::HttpMalformed)
    {
        callbacks.OnRpcRequest("<invalid/>");
        RUNTIME_CHECK(transport.statuses.size() == 1 && transport.statuses.back() == ISimpleHttpServer::eSC_BadRequest && transport.closes.back());
    }
    else
    {
        rpc("challenge", nullptr);
        listener.Update();
        RUNTIME_CHECK(transport.bodies.size() == 1 && transport.bodies.back().find("10.000000") != std::string::npos && !transport.closes.back());
        if (testCase != CryActionRegressionCase::HttpChallenge)
        {
            rpc("authenticate", testCase == CryActionRegressionCase::HttpReject ? "incorrect" : "86e6b81c35a5e9bdcd86bb77f2d23cc0");
            if (testCase == CryActionRegressionCase::HttpQueued)
            {
                // Harmless console assignments exercise command queue ordering after authentication.
                rpc("http_password", "queued-first");
                rpc("http_password", "queued-last");
            }
            listener.Update();
            RUNTIME_CHECK(transport.bodies.size() >= 2);
            if (transport.bodies.size() >= 2)
            {
                const bool reject = testCase == CryActionRegressionCase::HttpReject;
                RUNTIME_CHECK(transport.bodies[1].find(reject ? "Authorization Failed" : "authorized") != std::string::npos);
                RUNTIME_CHECK(transport.closes[1] == reject);
            }
            if (testCase == CryActionRegressionCase::HttpQueued)
            {
                RUNTIME_CHECK(transport.bodies.size() == 4 && std::strcmp(password->GetString(), "queued-last") == 0);
                const size_t before = transport.bodies.size();
                listener.Update();
                RUNTIME_CHECK(transport.bodies.size() == before);
            }
            if (testCase == CryActionRegressionCase::HttpEscaping)
            {
                static_cast<IOutputPrintSink&>(listener).Print("<&>\"'%");
                rpc("http_password", nullptr);
                listener.Update();
                RUNTIME_CHECK(transport.bodies.size() == 3);
                if (transport.bodies.size() == 3)
                    RUNTIME_CHECK(transport.bodies.back().find("&lt;&amp;&gt;&quot;&apos;&#37;") != std::string::npos);
            }
        }
    }
    callbacks.OnClientDisconnected();
    listener.Update();
    password->Set(previous.c_str());
    RUNTIME_CHECK(transport.starts == 0); // no listening socket or external traffic
}

void CryActionRegressionProbe::ServerLifecycle(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CGameQueryListener listener;
    CryActionRegressionPingSink sink(&listener);
    listener.SetNetListener(&sink);
    if (testCase == CryActionRegressionCase::ServerTarget255 || testCase == CryActionRegressionCase::ServerTarget256)
    {
        const std::string address(testCase == CryActionRegressionCase::ServerTarget255 ? 255 : 256, 'a');
        listener.AddServer("fixture", address.c_str(), "data", 20);
        char** targets = nullptr; int count = -1;
        listener.GetCurrentServers(&targets, count);
        std::unique_ptr<char*[]> ownedList(targets);
        RUNTIME_CHECK(count == 1 && targets && std::strcmp(targets[0], address.c_str()) == 0);
        RUNTIME_CHECK(listener.m_serverAddresses.size() == address.size() + 1 &&
            listener.m_serverAddresses.back() == '\0');
        return;
    }
    listener.AddServer("fixture", "127.0.0.1:6000", "payload", 20);
    if (testCase == CryActionRegressionCase::ServerDuplicate)
    {
        listener.AddServer("fixture", "127.0.0.1:6000", "payload", 40);
        // Repeated announcements of the same endpoint must update, not duplicate it.
        RUNTIME_CHECK(listener.m_servers.size() == 1 && sink.PingCount() == 1);
        return;
    }
    if (testCase == CryActionRegressionCase::ServerExpiry)
    {
        listener.AddServer("fixture", "127.0.0.1:6001", "second", 30);
        listener.AddServer("fixture", "127.0.0.1:6002", "third", 40);
        listener.m_servers[0].m_lastTime = CTimeValue(-1.0f); // age 11, expires
        listener.m_servers[1].m_lastTime = CTimeValue(-2.0f); // adjacent expired entry
        listener.m_servers[2].m_lastTime = CTimeValue(0.0f); // age exactly 10, retained
        listener.Update();
        RUNTIME_CHECK(listener.m_servers.size() == 1 && listener.m_servers[0].m_target == "127.0.0.1:6002");
        listener.m_servers[0].m_lastTime = CTimeValue(-1.0f);
        listener.Update();
        RUNTIME_CHECK(listener.m_servers.empty());
    }
    else if (testCase == CryActionRegressionCase::ServerPong)
    {
        listener.AddPong("127.0.0.1:6000", 40);
        int ping = -1;
        const char* data = listener.GetServerData("127.0.0.1:6000", ping);
        RUNTIME_CHECK(data && std::strcmp(data, "payload") == 0 && ping == 25);
        listener.AddPong("absent", 999);
        RUNTIME_CHECK(listener.m_servers.size() == 1 && listener.m_servers[0].m_ping == 25);
        RUNTIME_CHECK(listener.GetServerData("missing", ping) == nullptr && ping == -1);
    }
    else
    {
        listener.AddServer("fixture", "127.0.0.1:6001", "second", 30);
        listener.RefreshPings();
        RUNTIME_CHECK(sink.PingCount() == 4);
        listener.RemoveServer("absent");
        listener.RemoveServer("127.0.0.1:6000");
        listener.RefreshPings();
        RUNTIME_CHECK(sink.PingCount() == 5 && listener.m_servers.size() == 1);
    }
}

void CryActionRegressionProbe::FlowRegistry(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    static_assert(sizeof(TFlowNodeTypeId) == 2 && sizeof(TFlowPortId) == 1);
    static_assert(CFlowData::TYPE_MAX_COUNT == 4096);
    unsigned destructions = 0;
    std::array<IFlowGraphPtr, 3> liveGraphs;
    {
        CFlowSystem flow;
        if (testCase == CryActionRegressionCase::FlowGraphCleanup)
        {
            for (auto& graph : liveGraphs) { graph = flow.CreateFlowGraph(); RUNTIME_CHECK(!!graph); }
        }
        IFlowNodeFactoryPtr factory = new CryActionRegressionFlowFactory(destructions);
        const auto id = flow.RegisterType("Regression:fixture", factory);
        RUNTIME_CHECK(id != InvalidFlowNodeTypeId && flow.GetTypeId("Regression:fixture") == id);
        RUNTIME_CHECK(std::strcmp(flow.GetTypeName(id), "Regression:fixture") == 0);
        RUNTIME_CHECK(flow.RegisterType("Regression:fixture", factory) == id && destructions == 0);
        RUNTIME_CHECK(flow.GetTypeId("Regression:absent") == InvalidFlowNodeTypeId);
        if (testCase == CryActionRegressionCase::FlowRegistration)
        {
            std::vector<int> seen;
            IFlowGraphPtr graph = flow.CreateFlowGraph();
            const auto source = graph->CreateNode(id, "source", &seen);
            const auto target = graph->CreateNode(id, "target", &seen);
            RUNTIME_CHECK(source != InvalidFlowNodeId && target != InvalidFlowNodeId && source != target);
            const auto output = graph->ResolveAddress("source:Value", true);
            const auto input = graph->ResolveAddress("target:Value", false);
            RUNTIME_CHECK(output.node == source && output.port == 0 && output.isOutput);
            RUNTIME_CHECK(input.node == target && input.port == 0 && !input.isOutput);
            RUNTIME_CHECK(graph->LinkNodes(output, input));
            graph->InitializeValues();
            graph->Update();
            seen.clear();
            graph->ActivatePort(output, 17);
            graph->Update();
            RUNTIME_CHECK(seen.size() == 1 && seen[0] == 17);
            graph->ActivatePort(output, 29);
            graph->Update();
            RUNTIME_CHECK(seen.size() == 2 && seen[1] == 29);
        }
        if (testCase == CryActionRegressionCase::FlowExhaustion)
        {
            auto current = id;
            while (current < CFlowData::TYPE_MAX_COUNT - 1)
            {
                const std::string name = "Regression:type" + std::to_string(current + 1);
                const auto next = flow.RegisterType(name.c_str(), factory);
                RUNTIME_CHECK(next == current + 1);
                if (next != current + 1) return;
                current = next;
            }
            // The next registration calls CryError, which terminates Profile. It is
            // not a recoverable false/invalid-ID API contract. Validate the complete
            // legal 12-bit domain here; preserve the earlier isolated fatal/timeout
            // report separately instead of weakening the runner's completion rule.
            RUNTIME_CHECK(current == CFlowData::TYPE_MAX_COUNT - 1);
            RUNTIME_CHECK(flow.GetTypeId("Regression:type4095") == current);
            RUNTIME_CHECK(flow.GetTypeId("Regression:fixture") == id);
        }
    }
    RUNTIME_CHECK(destructions == 1);
    // Real FlowSystem destruction notifies every live graph before the graphs die.
    if (testCase == CryActionRegressionCase::FlowGraphCleanup)
    {
        for (const auto& graph : liveGraphs) RUNTIME_CHECK(!!graph);
        liveGraphs[1] = nullptr; liveGraphs[0] = nullptr; liveGraphs[2] = nullptr;
        RUNTIME_CHECK(std::all_of(liveGraphs.begin(), liveGraphs.end(), [](const auto& graph) { return !graph; }));
    }
}

void CryActionRegressionProbe::TimeDemoFixture(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CTimeDemoRecorder recorder(gEnv->pSystem);
    const bool v2 = testCase >= CryActionRegressionCase::TimeDemoV2;
    const int count = testCase == CryActionRegressionCase::TimeDemoEmpty ? 0 :
        testCase == CryActionRegressionCase::TimeDemoSingle ? 1 : 3;
    const uint32 frameSize = v2 ? 221 : 72;
    std::vector<uint8> frames(frameSize * count, 0);
    const auto write = [](std::vector<uint8>& bytes, size_t offset, auto value) {
        std::memcpy(bytes.data() + offset, &value, sizeof(value));
    };
    for (int i = 0; i < count; ++i)
    {
        const size_t start = frameSize * i;
        write(frames, start, static_cast<float>(i + 1)); // position.x
        if (v2) write(frames, start + 36, 1.0f); // quaternion.w
        write(frames, start + (v2 ? 40 : 24), 0.125f);
        write(frames, start + (v2 ? 44 : 28), uint32{0x80000001u});
        write(frames, start + (v2 ? 52 : 36), 0.25f);
        write(frames, start + (v2 ? 56 : 40), int32{100 + i});
    }
    const bool compressed = testCase == CryActionRegressionCase::TimeDemoCompressed;
    std::vector<uint8> payload = frames;
    if (compressed)
    {
        payload.resize(frames.size() * 2 + 1024);
        size_t length = payload.size();
        RUNTIME_CHECK(gEnv->pSystem->CompressDataBlock(frames.data(), frames.size(), payload.data(), length));
        if (length > payload.size()) return;
        payload.resize(length);
    }
    std::vector<uint8> header(283, 0);
    header[0] = 'C'; header[1] = 'R'; header[2] = 'Y';
    write(header, 3, int32{150}); write(header, 7, int32{v2 ? 2 : 1});
    write(header, 11, int32{283}); write(header, 15, int32{count}); write(header, 19, frameSize);
    write(header, 23, count * 0.125f);
    write(header, 155, uint32{compressed ? 1u : 0u});
    write(header, 159, static_cast<uint32>(compressed ? payload.size() : 0));
    write(header, 163, static_cast<uint32>(frames.size()));
    const auto path = ScratchPath("timedemo.tmd");
    FILE* file = OpenFixture(path, "wb");
    RUNTIME_CHECK(file != nullptr);
    if (!file) return;
    gEnv->pCryPak->FWrite(header.data(), header.size(), file);
    const size_t length = testCase == CryActionRegressionCase::TimeDemoTruncated ? payload.size() - 1 : payload.size();
    if (length) gEnv->pCryPak->FWrite(payload.data(), length, file);
    gEnv->pCryPak->FClose(file);
    const bool loaded = recorder.Load(path.c_str());
    RUNTIME_CHECK(DeleteFileA(path.c_str()) != FALSE);
    if (testCase == CryActionRegressionCase::TimeDemoTruncated)
    {
        RUNTIME_CHECK(!loaded && recorder.GetNumFrames() == 0);
        return;
    }
    RUNTIME_CHECK(loaded && recorder.GetNumFrames() == count && !recorder.IsPlaying() && !recorder.IsRecording());
    for (int i = 0; i < count && i < recorder.GetNumFrames(); ++i)
    {
        const auto& frame = recorder.m_records[i];
        RUNTIME_CHECK(frame.playerPos.IsEquivalent(Vec3(static_cast<float>(i + 1), 0, 0)));
        RUNTIME_CHECK(frame.frameTime == 0.125f && frame.nActionFlags[0] == 0x80000001u && frame.fLeaning == 0.25f);
        RUNTIME_CHECK(frame.nPolygons == 100 + i && frame.inputEventsList.empty());
    }
}
