#include "StdAfx.h"
#include "CryActionRegressionProbe.h"
#include <AnimationGraph/AnimationGraph.h>
#include <AnimationGraph/AnimationGraphState.h>
#include <AnimationGraph/AnimationGraphManager.h>

#include <algorithm>
#include <array>
#include <cstdio>

#define LINK_CHECK(expression) check(context, (expression) ? 1u : 0u, #expression, __FILE__, __LINE__)

void CryActionRegressionProbe::LinkCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    if (testCase >= CryActionRegressionCase::LinkXmlLookup)
    {
        LinkTransition(testCase, check, context);
        return;
    }
    auto graph = CAnimationGraph::CreateInstance(nullptr, "AN04LinkFixture");
    for (uint16 i = 0; i < 3; ++i)
    {
        CAnimationGraph::SLinkInfo link{};
        link.from = testCase == CryActionRegressionCase::LinkSameTarget ? 1 + i * 2 : 1;
        link.to = testCase == CryActionRegressionCase::LinkDuplicateSelection ? 2 :
            testCase == CryActionRegressionCase::LinkSameTarget ? 4 : 2 + i * 2;
        link.cost = 30 + i;
        link.forceFollowChance = 5 + i;
        link.overrideTransitionTime = 0.25f * (i + 1);
        graph->m_links.push_back(link);
    }
    const auto original = graph->m_links;
    if (testCase == CryActionRegressionCase::LinkDuplicateSelection)
    {
        const auto* selected = graph->FindLink(1, 2);
        LINK_CHECK(selected != nullptr);
        if (selected)
        {
            LINK_CHECK(selected >= graph->m_links.data() && selected < graph->m_links.data() + original.size());
            const size_t index = static_cast<size_t>(selected - graph->m_links.data());
            char description[128]{};
            std::snprintf(description, sizeof(description),
                "duplicate selection: slot=%zu cost=%u transition=%.2f", index, selected->cost,
                selected->overrideTransitionTime);
            check(context, index < original.size(), description, __FILE__, __LINE__);
            LINK_CHECK(index < original.size() && selected->cost == original[index].cost);
            for (int repeat = 0; repeat < 32; ++repeat)
                LINK_CHECK(graph->FindLink(1, 2) == selected);
        }
        LINK_CHECK(graph->IsDirectlyLinked(1, 2));
    }
    else if (testCase == CryActionRegressionCase::LinkPayloadOrdering)
    {
        auto first = original[0];
        auto sameEndpoints = first;
        sameEndpoints.cost = 65535;
        sameEndpoints.forceFollowChance = 65535;
        sameEndpoints.overrideTransitionTime = 10.0f;
        LINK_CHECK(!(first < sameEndpoints) && !(sameEndpoints < first));
        sameEndpoints.to = 3;
        LINK_CHECK(first < sameEndpoints && !(sameEndpoints < first));
        sameEndpoints.from = 0;
        LINK_CHECK(sameEndpoints < first && !(first < sameEndpoints));
        const auto* found = graph->FindLink(1, 2);
        LINK_CHECK(found && found->cost == original[0].cost && found->overrideTransitionTime == 0.25f);
    }
    else if (testCase == CryActionRegressionCase::LinkInvalidState)
    {
        LINK_CHECK(graph->FindLink(CAnimationGraph::INVALID_STATE, 2) == nullptr);
        LINK_CHECK(graph->FindLink(1, CAnimationGraph::INVALID_STATE) == nullptr);
        LINK_CHECK(graph->FindLink(CAnimationGraph::INVALID_STATE, CAnimationGraph::INVALID_STATE) == nullptr);
    }
    else if (testCase == CryActionRegressionCase::LinkRepeatStable)
    {
        uint32 seed = 0x43525932;
        for (int repeat = 0; repeat < 1024; ++repeat)
        {
            seed ^= seed << 13;
            seed ^= seed >> 17;
            seed ^= seed << 5;
            const uint16 from = static_cast<uint16>(seed % 6);
            const uint16 to = static_cast<uint16>((seed >> 8) % 7);
            // A linear fixture lookup is an independent oracle for the production binary search.
            const auto expected = std::find_if(original.begin(), original.end(), [&](const auto& link) {
                return link.from._value == from && link.to._value == to;
            });
            const auto* found = graph->FindLink(from, to);
            LINK_CHECK((found != nullptr) == (expected != original.end()));
            if (found && expected != original.end())
            {
                LINK_CHECK(found->cost == expected->cost && found->overrideTransitionTime == expected->overrideTransitionTime);
                LINK_CHECK(graph->IsDirectlyLinked(from, to));
            }
        }
    }
    else
    {
        for (const auto& expected : original)
        {
            const auto* found = graph->FindLink(expected.from, expected.to);
            LINK_CHECK(found != nullptr);
            if (found)
            {
                LINK_CHECK(found->from._value == expected.from._value && found->to._value == expected.to._value);
                LINK_CHECK(found->cost == expected.cost && found->forceFollowChance == expected.forceFollowChance);
                LINK_CHECK(found->overrideTransitionTime == expected.overrideTransitionTime);
            }
            LINK_CHECK(graph->IsDirectlyLinked(expected.from, expected.to));
        }
        if (testCase == CryActionRegressionCase::LinkSameSource)
        {
            LINK_CHECK(graph->FindLink(1, 3) == nullptr);
            LINK_CHECK(!graph->IsDirectlyLinked(1, 3));
        }
        else
        {
            LINK_CHECK(graph->FindLink(2, 4) == nullptr);
            LINK_CHECK(!graph->IsDirectlyLinked(2, 4));
        }
    }
    LINK_CHECK(graph->m_links.size() == original.size());
    for (size_t i = 0; i < original.size() && i < graph->m_links.size(); ++i)
    {
        const auto& before = original[i];
        const auto& after = graph->m_links[i];
        LINK_CHECK(before.from._value == after.from._value && before.to._value == after.to._value);
        LINK_CHECK(before.cost == after.cost && before.forceFollowChance == after.forceFollowChance &&
            before.overrideTransitionTime == after.overrideTransitionTime);
    }
}

void CryActionRegressionProbe::LinkTransition(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CAnimationGraphManager manager;
    auto graph = CAnimationGraph::CreateInstance(&manager, "AN04XmlFixture");
    XmlNodeRef root = gEnv->pSystem->CreateXmlNode("AnimationGraph");
    root->newChild("Inputs");
    root->newChild("Views");
    auto states = root->newChild("States");
    for (const char* name : {"Alpha", "Beta"})
    {
        auto state = states->newChild("State");
        state->setAttr("id", name);
        state->setAttr("cost", 1);
        state->setAttr("allowSelect", false);
    }
    const bool missing = testCase == CryActionRegressionCase::LinkTransitionMissing;
    auto link = root->newChild("Transitions")->newChild("Link");
    link->setAttr("from", missing ? "Beta" : "Alpha");
    link->setAttr("to", missing ? "Alpha" : "Beta");
    link->setAttr("transitionTime", 0.75f);
    link->setAttr("cost", 7);
    LINK_CHECK(graph->Init(root));
    LINK_CHECK(graph->GetAllStates().size() == 2 && graph->m_links.size() == 1);
    const auto from = graph->StateNameToID("Alpha");
    const auto to = graph->StateNameToID("Beta");
    LINK_CHECK(from != CAnimationGraph::INVALID_STATE && to != CAnimationGraph::INVALID_STATE && from != to);
    if (from == CAnimationGraph::INVALID_STATE || to == CAnimationGraph::INVALID_STATE)
        return;
    const auto* found = graph->FindLink(from, to);
    LINK_CHECK((found != nullptr) == !missing);
    if (found)
    {
        LINK_CHECK(graph->IsDirectlyLinked(from, to));
        LINK_CHECK(found->overrideTransitionTime == 0.75f && found->cost == 7 && found->forceFollowChance == 0);
    }
    if (testCase == CryActionRegressionCase::LinkXmlLookup)
        return;

    // Use a properly constructed graph state, with no character nodes in this component fixture.
    // ActivateNextState is the real FindLink consumer; the full per-frame transition is separate.
    CAnimationGraphState state(graph);
    state.m_currentStateID = from;
    state.m_nextStateID = to;
    const bool entered = testCase == CryActionRegressionCase::LinkTransitionEnteredOverride;
    const bool rollback = testCase == CryActionRegressionCase::LinkTransitionRollback;
    const bool largerPending = testCase == CryActionRegressionCase::LinkTransitionPendingOverride;
    const float initial = entered || largerPending ? 1.25f : 0.25f;
    state.m_currentStateWasEntered = entered;
    state.m_basicState.overrideTransitionTime = initial;
    state.m_lastEnteredToken = 77;
    const uint32 beforeToken = state.m_token;
    const auto beforeLink = graph->m_links[0];
    state.ActivateNextState(rollback);
    const float expected = missing || rollback ? initial : largerPending ? 1.25f : 0.75f;
    LINK_CHECK(state.m_basicState.overrideTransitionTime == expected);
    LINK_CHECK(state.m_currentStateID == from && state.m_nextStateID == to);
    LINK_CHECK(state.m_currentStateWasEntered == rollback);
    LINK_CHECK(state.m_token == (rollback ? 77 : beforeToken + (entered ? 1 : 0)));
    LINK_CHECK(state.m_pendingLeftStateIDs.size() == 1 && state.m_pendingLeftStateIDs.back() == to);
    LINK_CHECK(state.m_currentStateNodes.empty() && state.m_nextStateNodes.empty());
    LINK_CHECK(!state.m_resetLock && !state.m_isStateNodeCacheValid);
    LINK_CHECK(state.m_stateStartTime == gEnv->pTimer->GetFrameStartTime());
    LINK_CHECK(graph->m_links.size() == 1 && graph->m_links[0].cost == beforeLink.cost &&
        graph->m_links[0].overrideTransitionTime == beforeLink.overrideTransitionTime);
}
