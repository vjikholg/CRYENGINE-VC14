#include "StdAfx.h"
#include "CryActionRegressionProbe.h"
#include <AnimationGraph/AnimationGraph.h>
#include <Network/GameQueryListener.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iterator>
#include <memory>
#include <MTPseudoRandom.h>

extern CMTRand_int32 g_random_generator;

#define ACTION_CHECK(expression) check(context, (expression) ? 1u : 0u, #expression, __FILE__, __LINE__)

void CryActionRegressionProbe::ReadBuffer(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    AG_FILE input;
    input.file = nullptr;
    input.m_curpos = 0;
    FileSerializationHelper reader;
    reader.reading = true;
    reader.swappingEndian = false;
    reader.ag_file = &input;

    if (testCase == CryActionRegressionCase::ReadTruncated ||
        testCase == CryActionRegressionCase::ReadAtEnd)
    {
        input.m_buffer.resize(3);
        std::memset(input.m_buffer.begin(), 0x11, 3);
        if (testCase == CryActionRegressionCase::ReadAtEnd)
            input.m_curpos = 3;
        const size_t originalCursor = input.m_curpos;
        std::uint32_t output = 0xdeadbeef;
        reader.ReadFromBuffer(&output);
        ACTION_CHECK(output == 0xdeadbeef);
        ACTION_CHECK(input.m_curpos == originalCursor);
        ACTION_CHECK(input.m_curpos <= static_cast<size_t>(input.m_buffer.size()));
        return;
    }
    if (testCase == CryActionRegressionCase::ReadEmptyString ||
        testCase == CryActionRegressionCase::ReadString ||
        testCase == CryActionRegressionCase::ReadRecord)
    {
        const char* text = testCase == CryActionRegressionCase::ReadEmptyString ? "" : "Legacy graph";
        const int length = static_cast<int>(std::strlen(text)) + 1;
        const std::uint32_t prefix = 0x1234abcd;
        const std::uint16_t suffix = 0x5678;
        const bool record = testCase == CryActionRegressionCase::ReadRecord;
        const size_t total = sizeof(length) + static_cast<size_t>(length) +
            (record ? sizeof(prefix) + sizeof(suffix) : 0);
        input.m_buffer.resize(static_cast<int>(total));
        auto* cursor = input.m_buffer.begin();
        if (record)
        {
            std::memcpy(cursor, &prefix, sizeof(prefix));
            cursor += sizeof(prefix);
        }
        std::memcpy(cursor, &length, sizeof(length));
        cursor += sizeof(length);
        std::memcpy(cursor, text, static_cast<size_t>(length));
        cursor += length;
        if (record)
            std::memcpy(cursor, &suffix, sizeof(suffix));
        FILE* file = nullptr;
        ACTION_CHECK(tmpfile_s(&file) == 0 && file != nullptr);
        if (file == nullptr)
            return;
        reader.file = file;
        if (record)
        {
            std::uint32_t actual = 0;
            reader.ReadFromBuffer(&actual);
            ACTION_CHECK(actual == prefix);
        }
        string actualText("unchanged");
        reader.StringValue(&actualText);
        ACTION_CHECK(actualText == text);
        if (record)
        {
            std::uint16_t actual = 0;
            reader.ReadFromBuffer(&actual);
            ACTION_CHECK(actual == suffix);
        }
        ACTION_CHECK(input.m_curpos == total);
        std::fclose(file);
        return;
    }
    const std::array<std::uint32_t, 3> values{0x01020304, 0x12345678, 0xfedcba98};
    input.m_buffer.resize(static_cast<int>(sizeof(values)));
    std::memcpy(input.m_buffer.begin(), values.data(), sizeof(values));
    if (testCase == CryActionRegressionCase::ReadZeroElements)
    {
        std::uint32_t output = 42;
        reader.ReadFromBuffer(&output, 0);
        ACTION_CHECK(output == 42);
        ACTION_CHECK(input.m_curpos == 0);
    }
    else if (testCase == CryActionRegressionCase::ReadArray)
    {
        std::array<std::uint32_t, 3> output{};
        reader.ReadFromBuffer(output.data(), static_cast<int>(output.size()));
        ACTION_CHECK(output == values);
        ACTION_CHECK(input.m_curpos == sizeof(values));
    }
    else
    {
        for (const auto expected : values)
        {
            std::uint32_t output = 0;
            reader.ReadFromBuffer(&output);
            ACTION_CHECK(output == expected);
        }
        ACTION_CHECK(input.m_curpos == sizeof(values));
    }
}

void CryActionRegressionProbe::LinkLookup(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    auto graph = CAnimationGraph::CreateInstance(nullptr, "CryActionRegression");
    if (testCase == CryActionRegressionCase::FindEmptyLink)
    {
        ACTION_CHECK(graph->FindLink(0, 0) == nullptr);
        return;
    }
    if (testCase == CryActionRegressionCase::HasEmptyLink)
    {
        ACTION_CHECK(!graph->IsDirectlyLinked(0, 0));
        return;
    }
    const std::array<std::pair<uint16, uint16>, 3> endpoints{{{1, 2}, {1, 4}, {3, 5}}};
    const size_t count = testCase == CryActionRegressionCase::FindSingleLink ? 1 : endpoints.size();
    for (size_t i = 0; i < count; ++i)
    {
        CAnimationGraph::SLinkInfo link{};
        link.from = endpoints[i].first;
        link.to = endpoints[i].second;
        link.cost = static_cast<uint16>(10 + i);
        graph->m_links.push_back(link);
    }
    uint16 from = 1;
    uint16 to = 2;
    bool expected = true;
    if (testCase == CryActionRegressionCase::FindLastLink) { from = 3; to = 5; }
    if (testCase == CryActionRegressionCase::FindMissingBelow) { from = 0; to = 0; expected = false; }
    if (testCase == CryActionRegressionCase::FindMissingBetween) { from = 1; to = 3; expected = false; }
    if (testCase == CryActionRegressionCase::FindMissingAbove) { from = 9; to = 9; expected = false; }
    const auto* found = graph->FindLink(from, to);
    ACTION_CHECK((found != nullptr) == expected);
    if (found != nullptr)
    {
        ACTION_CHECK(static_cast<uint16>(found->from) == from);
        ACTION_CHECK(static_cast<uint16>(found->to) == to);
        ACTION_CHECK(found >= graph->m_links.data() && found < graph->m_links.data() + graph->m_links.size());
    }
    ACTION_CHECK(graph->IsDirectlyLinked(from, to) == expected);
    ACTION_CHECK(graph->m_links.size() == count);
}

void CryActionRegressionProbe::ServerList(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    constexpr int counts[]{0, 1, 49, 50, 51, 119, 120, 121};
    const auto offset = static_cast<unsigned>(testCase) - static_cast<unsigned>(CryActionRegressionCase::Enumerate0);
    const int expected = counts[offset];
    CGameQueryListener listener;
    CryActionRegressionPingSink sink(&listener);
    listener.SetNetListener(&sink);
    for (int i = 0; i < expected; ++i)
    {
        char address[64];
        std::snprintf(address, sizeof(address), "127.0.0.1:%d", 6000 + i);
        listener.AddServer("fixture", address, "fixture-data", 20);
    }
    ACTION_CHECK(sink.PingCount() == static_cast<unsigned>(expected));
    char** servers = nullptr;
    int count = -1;
    listener.GetCurrentServers(&servers, count);
    ACTION_CHECK(count == expected);
    std::unique_ptr<char*[]> ownedList(servers);
    if (count != expected) return;
    ACTION_CHECK((servers != nullptr) == (count > 0));
    for (int i = 0; i < count; ++i)
    {
        char address[64];
        std::snprintf(address, sizeof(address), "127.0.0.1:%d", 6000 + i);
        ACTION_CHECK(servers != nullptr && std::strcmp(servers[i], address) == 0);
        char* actualAddress = nullptr;
        char* data = nullptr;
        int ping = -1;
        listener.GetServer(i, &actualAddress, &data, ping);
        ACTION_CHECK(actualAddress != nullptr && std::strcmp(actualAddress, address) == 0);
        ACTION_CHECK(data != nullptr && std::strcmp(data, "fixture-data") == 0);
        ACTION_CHECK(ping == 20);
    }
    if (count > 0)
    {
        listener.RemoveServer(string("127.0.0.1:6000"));
        listener.GetCurrentServers(&servers, count);
        ownedList.reset(servers);
        ACTION_CHECK(count == expected - 1);
        if (count > 0)
            ACTION_CHECK(std::strcmp(servers[0], "127.0.0.1:6001") == 0);
    }
}

void CryActionRegressionProbe::ParseServer(CryActionRegressionCheck check, void* context)
{
    CGameQueryListener listener;
    char text[] = "\"Forest\" \"4\" \"PowerStruggle\" \"1.2.1\" \"16\"";
    SServerData data{};
    listener.GetValuesFromData(text, &data);
    ACTION_CHECK(data.strMap == "Forest");
    ACTION_CHECK(data.iNumPlayers == 4);
    ACTION_CHECK(data.strMode == "PowerStruggle");
    ACTION_CHECK(data.strVersion == "1.2.1");
    ACTION_CHECK(data.iMaxPlayers == 16);
    ACTION_CHECK(std::strcmp(text, "\"Forest\" \"4\" \"PowerStruggle\" \"1.2.1\" \"16\"") == 0);
}

std::uint32_t CryActionRegressionProbe::Run(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    if (testCase >= CryActionRegressionCase::MovementNoCharacter)
        return BoundaryCases(testCase, check, context);
    if (testCase >= CryActionRegressionCase::ReadabilityEmpty)
        return RuntimeCases(testCase, check, context);
    if (testCase <= CryActionRegressionCase::ReadTruncated)
        ReadBuffer(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::FindMissingAbove)
        LinkLookup(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::Enumerate121)
        ServerList(testCase, check, context);
    else if (testCase == CryActionRegressionCase::ParseServerRecord)
        ParseServer(check, context);
    else if (testCase == CryActionRegressionCase::ReadAtEnd)
        ReadBuffer(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::StateIndexRoundtrip)
        StateIndex(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::RotationXml)
        Rotation(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::DispatchThree)
        Dispatch(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::ThumbnailRichEmpty)
        Thumbnail(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::BmpSizeOverflow)
        Bitmap(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::MetadataClone)
        Metadata(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::ItemParamsTree)
        ItemParams(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::HexInvalid)
        Hex(testCase, check, context);
    else if (testCase == CryActionRegressionCase::ModuleImage)
        ModuleImage(check, context);
    else if (testCase <= CryActionRegressionCase::GraphXmlExclude)
        GraphXml(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::InventoryThree)
        Inventory(testCase, check, context);
    else if (testCase <= CryActionRegressionCase::LinkTransitionRollback)
        LinkCases(testCase, check, context);
    else
        ReaderCases(testCase, check, context);
    return 0;
}

extern "C" __declspec(dllexport) std::uint32_t CryActionRunRegressionCase(
    std::uint32_t abi, CryActionRegressionCase testCase, ISystem* system,
    CryActionRegressionCheck check, void* context)
{
    if (abi != CryActionRegressionAbi || check == nullptr || testCase >= CryActionRegressionCase::Count)
        return 2;
    if (CryActionRegressionCases[static_cast<unsigned>(testCase)].needsSystem && system == nullptr)
        return 2;
    auto* previous = gEnv;
    CryActionRegressionTimer timer;
    SSystemGlobalEnvironment environment{};
    if (system != nullptr)
        environment = *system->GetGlobalEnvironment();
    environment.pTimer = &timer;
    environment.bProfilerEnabled = false;
    environment.bIgnoreAllAsserts = true;
    gEnv = &environment;
    // Every case has its own child/module instance; reset this module's actual RNG.
    g_random_generator.seed(0x43525932);
    const auto result = CryActionRegressionProbe::Run(testCase, check, context);
    gEnv = previous;
    return result;
}

void CryActionRegressionTimer::ResetTimer() {}
void CryActionRegressionTimer::UpdateOnFrameStart() {}
float CryActionRegressionTimer::GetCurrTime(ETimer) const { return 10.0f; }
CTimeValue CryActionRegressionTimer::GetFrameStartTime(ETimer) const { return CTimeValue(10.0f); }
CTimeValue CryActionRegressionTimer::GetAsyncTime() const { return CTimeValue(10.0f); }
float CryActionRegressionTimer::GetAsyncCurTime() { return 10.0f; }
float CryActionRegressionTimer::GetFrameTime(ETimer) const { return 1.0f / 60.0f; }
float CryActionRegressionTimer::GetRealFrameTime() const { return 1.0f / 60.0f; }
float CryActionRegressionTimer::GetTimeScale() const { return 1.0f; }
void CryActionRegressionTimer::SetTimeScale(float) {}
void CryActionRegressionTimer::EnableTimer(bool) {}
bool CryActionRegressionTimer::IsTimerEnabled() const { return true; }
float CryActionRegressionTimer::GetFrameRate() { return 60.0f; }
float CryActionRegressionTimer::GetProfileFrameBlending(float* time, int* mode)
{
    if (time) *time = 0.0f;
    if (mode) *mode = 0;
    return 0.0f;
}
void CryActionRegressionTimer::Serialize(TSerialize) {}
bool CryActionRegressionTimer::PauseTimer(ETimer, bool) { return true; }
bool CryActionRegressionTimer::IsTimerPaused(ETimer) { return false; }
bool CryActionRegressionTimer::SetTimer(ETimer, float) { return true; }
void CryActionRegressionTimer::SecondsToDateUTC(time_t seconds, tm& result) { gmtime_s(&result, &seconds); }
time_t CryActionRegressionTimer::DateToSecondsUTC(tm& value) { return _mkgmtime(&value); }

CryActionRegressionPingSink::CryActionRegressionPingSink(IGameQueryListener* listener) : m_listener(listener) {}
void CryActionRegressionPingSink::DeleteNetQueryListener() {}
void CryActionRegressionPingSink::SendPingTo(const char*) { ++m_pings; }
IGameQueryListener* CryActionRegressionPingSink::GetGameQueryListener() { return m_listener; }
void CryActionRegressionPingSink::GetMemoryStatistics(ICrySizer*) {}
std::uint32_t CryActionRegressionPingSink::PingCount() const { return m_pings; }
