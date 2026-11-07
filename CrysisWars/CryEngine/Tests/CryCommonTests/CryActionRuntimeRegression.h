#pragma once

#include <IAnimationGraph.h>
#include <IGameplayRecorder.h>
#include <ISimpleHttpServer.h>
#include <ViewSystem/View.h>
#include <IFlowSystem.h>
#include <FlowSystem/Nodes/FlowBaseNode.h>
#include <string>
#include <vector>
#include <array>

// Callback observations are copied while the production borrowed arguments are valid.
class CryActionRegressionEvents : public IAnimationGraphStateListener, public IGameplayListener
{
public:
    explicit CryActionRegressionEvents(int identity = 0, std::vector<int>* order = nullptr);
    void SetOutput(const char*, const char*) override;
    void QueryComplete(TAnimationGraphQueryID, bool) override;
    void DestroyedState(IAnimationGraphState*) override;
    void OnGameplayEvent(IEntity*, const GameplayEvent&) override;
    std::vector<std::string> outputs;
    std::array<std::uintptr_t, 8> payloads{};
    std::array<unsigned, 8> events{};
    std::array<std::string, 8> descriptions;
    unsigned gameplayCount = 0;
    unsigned destroyed = 0;
    unsigned queries = 0;
private:
    int m_identity;
    std::vector<int>* m_order;
};

class CryActionRegressionView : public CView
{
public:
    explicit CryActionRegressionView(ISystem*);
    void ProcessShake(SShake*, float) override;
    const std::vector<SShake>& Shakes() const;
    std::vector<int> visited;
};

// No socket is opened: this observes the production listener's transport boundary.
class CryActionRegressionHttp : public ISimpleHttpServer
{
public:
    void Start(uint16, const string&, IHttpServerListener*) override;
    void Stop() override;
    void SendResponse(EStatusCode, EContentType, const string&, bool) override;
    void SendWebpage(const string&) override;
    std::vector<std::string> bodies;
    std::vector<EStatusCode> statuses;
    std::vector<bool> closes;
    unsigned starts = 0;
};

class CryActionRegressionFlowFactory : public IFlowNodeFactory
{
public:
    explicit CryActionRegressionFlowFactory(unsigned& destructions);
    void AddRef() override;
    void Release() override;
    IFlowNodePtr Create(IFlowNode::SActivationInfo*) override;
    void GetMemoryStatistics(ICrySizer*) override;
private:
    unsigned m_refs = 0;
    unsigned& m_destructions;
};

class CryActionRegressionFlowNode : public CFlowBaseNode
{
public:
    explicit CryActionRegressionFlowNode(SActivationInfo*);
    IFlowNodePtr Clone(SActivationInfo*) override;
    void GetConfiguration(SFlowNodeConfig&) override;
    void ProcessEvent(EFlowEvent, SActivationInfo*) override;
    void GetMemoryStatistics(ICrySizer*) override;
private:
    std::vector<int>* m_values;
};

// Existing external production functions; declarations only, no copied implementation.
bool WriteXMLNode(uint32, const XmlNodeRef&, FILE*, bool, const char*);
bool ReadTag(FILE*, uint32&, bool);
bool SkipXMLTagData(uint32, FILE*);
char* ReadXMLTagData(uint32, FILE*, bool);
