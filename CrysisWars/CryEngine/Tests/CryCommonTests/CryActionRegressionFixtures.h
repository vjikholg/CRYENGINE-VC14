#pragma once

#include <IPlayerProfiles.h>
#include <INetwork.h>

// Captures the real rotation writer; it does not replace rotation serialization.
class CryActionRegressionRotationFile : public ILevelRotationFile
{
public:
    bool Save(XmlNodeRef) override;
    XmlNodeRef Load() override;
    void Complete() override;
    XmlNodeRef root;
    unsigned writes = 0;
};

class CryActionRegressionProtocol : public IProtocolBuilder
{
public:
    void AddMessageSink(INetMessageSink*, const SNetProtocolDef&, const SNetProtocolDef&) override;
    const SNetMessageDef* sending = nullptr;
    const SNetMessageDef* receiving = nullptr;
    size_t sendCount = 0;
    size_t receiveCount = 0;
    unsigned calls = 0;
};
