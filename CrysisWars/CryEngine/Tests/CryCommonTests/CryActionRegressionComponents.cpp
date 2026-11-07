#include "StdAfx.h"
#include "CryActionRegressionProbe.h"
#include "CryActionRegressionFixtures.h"
#include <AnimationGraph/AnimationGraph.h>
#include <GameObjects/GameObjectDispatch.h>
#include <LevelSystem.h>
#include <ISaveGame.h>
#include <IMetadataRecorder.h>
#include <ItemParams.h>
#include <Inventory.h>
#include <PlayerProfiles/PlayerProfileImplFS.h>
#include <PlayerProfiles/BMPHelper.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <set>

#define ACTION_CHECK(expression) check(context, (expression) ? 1u : 0u, #expression, __FILE__, __LINE__)

bool CryActionRegressionRotationFile::Save(XmlNodeRef value)
{
    root = value;
    ++writes;
    return true;
}
XmlNodeRef CryActionRegressionRotationFile::Load() { return root; }
void CryActionRegressionRotationFile::Complete() {}

void CryActionRegressionProtocol::AddMessageSink(
    INetMessageSink*, const SNetProtocolDef& out, const SNetProtocolDef& in)
{
    sending = out.vMessages;
    receiving = in.vMessages;
    sendCount = out.nMessages;
    receiveCount = in.nMessages;
    ++calls;
}

std::string CryActionRegressionProbe::ScratchPath(const char* name)
{
    MEMORY_BASIC_INFORMATION memory{};
    if (!VirtualQuery(reinterpret_cast<const void*>(&CryActionRunRegressionCase), &memory, sizeof(memory)))
        return {};
    char path[32768]{};
    const DWORD length = GetModuleFileNameA(static_cast<HMODULE>(memory.AllocationBase), path, sizeof(path));
    if (length == 0 || length >= sizeof(path))
        return {};
    std::string result(path, length);
    result.erase(result.find_last_of("\\/") + 1);
    return result + "fixture-" + std::to_string(GetCurrentProcessId()) + "-" + name;
}

FILE* CryActionRegressionProbe::OpenFixture(const std::string& path, const char* mode)
{
    // This overload preserves an existing absolute directory instead of applying the
    // ordinary write route's %USER% redirection and automatic directory creation.
    char adjustedPath[32768]{};
    return gEnv->pCryPak->FOpen(path.c_str(), mode, adjustedPath, sizeof(adjustedPath));
}

void CryActionRegressionProbe::StateIndex(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    auto builder = std::make_unique<CStateIndex::Builder>();
    const int count = testCase == CryActionRegressionCase::StateIndexEmpty ? 0 :
        testCase == CryActionRegressionCase::StateIndexSingle ? 1 : 3;
    if (count != 0)
    {
        for (uint8 input = 0; input != 2; ++input)
            for (uint8 value = 0; value != 2; ++value)
                builder->DeclInputValue(input, value, 1);
        for (uint16 state = 0; state < count; ++state)
            builder->DeclState(state);
        builder->RestrictStateToInputValue(0, 0, 0);
        builder->RestrictStateToInputValue(0, 1, 0);
        if (count == 3)
        {
            builder->RestrictStateToInputValue(1, 0, 1);
            builder->RestrictStateToInputValue(1, 1, 1);
            builder->RestrictStateToInputValue(2, 1, 1);
        }
    }
    CStateIndex index;
    index.Init(std::move(builder));
    std::set<uint16> beforeRoundtrip;
    for (const auto& input : index.m_inputData)
    {
        ACTION_CHECK(input.dbOffset <= index.m_database.size() &&
            input.numStates <= index.m_database.size() - input.dbOffset);
        ACTION_CHECK(input.dbExplicitOffset <= index.m_database.size() &&
            input.numExplicitStates <= index.m_database.size() - input.dbExplicitOffset);
    }

    if (testCase == CryActionRegressionCase::StateIndexRoundtrip)
    {
        const std::string path = ScratchPath("state-index.bin");
        ACTION_CHECK(!path.empty());
        if (path.empty()) return;
        AG_FILE image{};
        image.file = OpenFixture(path, "wb");
        ACTION_CHECK(image.file != nullptr);
        if (image.file == nullptr) return;
        index.SerializeAsFile(false, &image);
        gEnv->pCryPak->FClose(image.file);
        const CStateIndex::InputValue beforeQuery[]{0, CStateIndex::INPUT_VALUE_DONT_CARE};
        CStateIndex::StateIDVec beforeResult;
        CStateIndex::QueryStats beforeStats{};
        index.Query(beforeQuery, beforeResult, beforeStats);
        for (const auto state : beforeResult) beforeRoundtrip.insert(state._value);
        image.file = OpenFixture(path, "rb");
        ACTION_CHECK(image.file != nullptr);
        if (image.file == nullptr) { DeleteFileA(path.c_str()); return; }
        image.SetFile(true, image.file);

        // Independent wire decoder: InputData is 15 serialized bytes, not sizeof(InputData).
        size_t cursor = 0;
        const size_t length = static_cast<size_t>(image.m_buffer.size());
        auto take = [&]<class T>(T& value) {
            if (cursor > length || sizeof(T) > length - cursor) return false;
            std::memcpy(&value, image.m_buffer.begin() + cursor, sizeof(T));
            cursor += sizeof(T);
            return true;
        };
        int32 selectors = -1;
        ACTION_CHECK(take(selectors) && selectors == 4);
        struct Range { uint16 count, explicitCount; uint32 start, explicitStart; };
        std::array<Range, 4> ranges{};
        bool decoded = selectors == 4;
        for (size_t i = 0; decoded && i < ranges.size(); ++i)
        {
            uint8 input = 255, value = 255, priority = 0;
            auto& range = ranges[i];
            decoded = take(input) && take(value) && take(range.count) && take(range.explicitCount) &&
                take(priority) && take(range.start) && take(range.explicitStart);
            ACTION_CHECK(decoded && input == i / 2 && value == i % 2 && priority == 1);
        }
        int32 states = -1;
        decoded = decoded && take(states) && states >= 0;
        ACTION_CHECK(decoded && states <= 32);
        for (const auto& range : ranges)
        {
            ACTION_CHECK(decoded && range.start <= static_cast<uint32>(states) &&
                range.count <= static_cast<uint32>(states) - range.start);
            ACTION_CHECK(decoded && range.explicitStart <= static_cast<uint32>(states) &&
                range.explicitCount <= static_cast<uint32>(states) - range.explicitStart);
        }
        for (int32 i = 0; decoded && i < states && i < 32; ++i)
        {
            uint16 state = 0xffff;
            decoded = take(state) && state < 3;
        }
        uint8 inputCount = 0;
        uint16 stateCount = 0;
        int32 cachedQueries = -1;
        decoded = decoded && take(inputCount) && take(stateCount) && take(cachedQueries);
        ACTION_CHECK(decoded && inputCount == 2 && stateCount == 3 && cachedQueries == 0 && cursor == length);
        CStateIndex loaded;
        loaded.SerializeAsFile(true, &image);
        ACTION_CHECK(image.m_curpos == length);
        ACTION_CHECK(loaded.m_database.size() == index.m_database.size());
        ACTION_CHECK(loaded.m_inputData.size() == index.m_inputData.size());
        for (size_t i = 0; i < loaded.m_database.size() && i < index.m_database.size(); ++i)
            ACTION_CHECK(loaded.m_database[i]._value == index.m_database[i]._value);
        for (size_t i = 0; i < loaded.m_inputData.size() && i < index.m_inputData.size(); ++i)
        {
            const auto& before = index.m_inputData[i];
            const auto& after = loaded.m_inputData[i];
            ACTION_CHECK(before.id == after.id && before.value == after.value && before.priority == after.priority);
            ACTION_CHECK(before.numStates == after.numStates && before.dbOffset == after.dbOffset);
            ACTION_CHECK(before.numExplicitStates == after.numExplicitStates && before.dbExplicitOffset == after.dbExplicitOffset);
        }
        gEnv->pCryPak->FClose(image.file);
        ACTION_CHECK(DeleteFileA(path.c_str()) != FALSE);
        index.Swap(loaded);
    }

    CStateIndex::InputValue query[]{0, CStateIndex::INPUT_VALUE_DONT_CARE};
    std::set<uint16> expected;
    if (count != 0) expected.insert(0);
    // State 0 explicitly matches input 0; state 2 is unrestricted, so ranking
    // must prefer 0 rather than return both merely-eligible states.
    if (testCase == CryActionRegressionCase::StateIndexWildcard)
    {
        query[0] = CStateIndex::INPUT_VALUE_DONT_CARE;
        expected = {0, 1, 2};
    }
    if (testCase == CryActionRegressionCase::StateIndexIntersection)
    {
        query[1] = 0;
        expected = {0};
    }
    CStateIndex::StateIDVec actual;
    CStateIndex::QueryStats stats{};
    index.Query(query, actual, stats);
    std::set<uint16> found;
    for (const auto state : actual) found.insert(state._value);
    ACTION_CHECK(found == expected);
    if (testCase == CryActionRegressionCase::StateIndexRoundtrip)
        ACTION_CHECK(found == beforeRoundtrip);
    ACTION_CHECK(actual.size() == found.size());
    ACTION_CHECK(stats.nQueryableStates == count);
    if (count != 0)
    {
        ACTION_CHECK(index.StateMatchesInput(0, 0, 0, eSMIF_EnforceMatchesInput));
        ACTION_CHECK(!index.StateMatchesInput(0, 0, 1, eSMIF_EnforceMatchesInput));
    }
}

void CryActionRegressionProbe::Rotation(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CLevelRotation rotation;
    const int count = testCase == CryActionRegressionCase::RotationEmpty ? 0 :
        testCase == CryActionRegressionCase::RotationSingle ? 1 : 3;
    const char* names[]{"island", "mesa", "shore"};
    for (int i = 0; i < count; ++i)
    {
        ACTION_CHECK(rotation.AddLevel(names[i], "PowerStruggle") == i);
        rotation.AddSetting(i, "g_fixture 1");
        rotation.AddSetting(i, "g_fixture 2");
    }
    ACTION_CHECK(rotation.GetLength() == count);
    if (testCase == CryActionRegressionCase::RotationRandom)
    {
        std::srand(0x435259);
        rotation.SetRandom(true);
        ACTION_CHECK(rotation.IsRandom());
    }
    ACTION_CHECK(rotation.First() == (count != 0));
    std::set<std::string> visited;
    for (int i = 0; i < count; ++i)
    {
        const char* level = rotation.GetNextLevel();
        ACTION_CHECK(level != nullptr);
        if (level != nullptr) visited.insert(level);
        if (!rotation.IsRandom()) ACTION_CHECK(level && std::strcmp(level, names[i]) == 0);
        ACTION_CHECK(std::strcmp(rotation.GetNextGameRules(), "PowerStruggle") == 0);
        ACTION_CHECK(rotation.GetNextSettingsNum() == 2);
        ACTION_CHECK(std::strcmp(rotation.GetNextSetting(0), "g_fixture 1") == 0);
        ACTION_CHECK(std::strcmp(rotation.GetNextSetting(1), "g_fixture 2") == 0);
        ACTION_CHECK(rotation.GetNextSetting(-1) == nullptr && rotation.GetNextSetting(2) == nullptr);
        ACTION_CHECK(rotation.Advance() == (i + 1 < count));
    }
    ACTION_CHECK(visited.size() == static_cast<size_t>(count));
    ACTION_CHECK(rotation.GetNextLevel() == nullptr && rotation.GetNextSettingsNum() == 0);
    if (testCase == CryActionRegressionCase::RotationXml)
    {
        CryActionRegressionRotationFile file;
        ACTION_CHECK(rotation.Save(&file));
        ACTION_CHECK(file.writes == 1 && file.root && file.root->getChildCount() == count);
        for (int i = 0; file.root && i < file.root->getChildCount(); ++i)
        {
            const auto child = file.root->getChild(i);
            ACTION_CHECK(std::strcmp(child->getAttr("name"), names[i]) == 0);
            ACTION_CHECK(std::strcmp(child->getAttr("gameRules"), "PowerStruggle") == 0);
            ACTION_CHECK(child->getChildCount() == 2);
            ACTION_CHECK(std::strcmp(child->getChild(1)->getAttr("setting"), "g_fixture 2") == 0);
        }
    }
    rotation.Reset();
    ACTION_CHECK(rotation.GetLength() == 0 && !rotation.First() && !rotation.IsRandom());
}

void CryActionRegressionProbe::Dispatch(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    CGameObjectDispatch dispatch;
    std::array<SGameObjectExtensionRMI, 3> messages{};
    const size_t count = testCase == CryActionRegressionCase::DispatchEmpty ? 0 :
        testCase == CryActionRegressionCase::DispatchSingle ? 1 : 3;
    for (size_t i = 0; i < count; ++i)
    {
        messages[i].description = i == 0 ? "first" : i == 1 ? "second" : "third";
        messages[i].pBase = messages.data();
        messages[i].isServerCall = i != 1;
        messages[i].attach = i == 2 ? eRAT_PreAttach : eRAT_NoAttach;
        messages[i].reliability = eNRT_ReliableOrdered;
        messages[i].lowDelay = i == 0;
    }
    dispatch.RegisterInterface(messages.data(), count);
    dispatch.RegisterInterface(messages.data(), count); // same interface must not register twice
    CryActionRegressionProtocol server, client;
    dispatch.GetServerSink()->DefineProtocol(&server);
    dispatch.GetClientSink()->DefineProtocol(&client);
    ACTION_CHECK(server.calls == 1 && client.calls == 1);
    ACTION_CHECK(server.sendCount == (count == 3 ? 1u : 0u));
    ACTION_CHECK(server.receiveCount == (count == 3 ? 2u : count));
    ACTION_CHECK(client.sendCount == server.receiveCount && client.receiveCount == server.sendCount);
    for (size_t i = 0; i < count; ++i)
    {
        const auto* definition = messages[i].pMsgDef;
        ACTION_CHECK(definition != nullptr);
        if (!definition) continue;
        ACTION_CHECK(definition->nUser == i);
        ACTION_CHECK(std::strcmp(definition->description, messages[i].description) == 0);
        ACTION_CHECK(definition->reliability == (i == 2 ? eNRT_UnreliableOrdered : eNRT_ReliableOrdered));
        ACTION_CHECK(((definition->parallelFlags & eMPF_NoSendDelay) != 0) == (i == 0));
        ACTION_CHECK((definition->parallelFlags & eMPF_DiscardIfNoEntity) != 0);
    }
}

void CryActionRegressionProbe::Thumbnail(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    const bool rich = testCase >= CryActionRegressionCase::ThumbnailRichRgb;
    const unsigned kind = static_cast<unsigned>(testCase) - static_cast<unsigned>(rich ?
        CryActionRegressionCase::ThumbnailRichRgb : CryActionRegressionCase::ThumbnailFsRgb);
    const int previous = CPlayerProfileManager::sUseRichSaveGames;
    CPlayerProfileManager::sUseRichSaveGames = rich ? 1 : 0;
    auto* profile = new CPlayerProfileImplFSDir;
    CPlayerProfileManager::SUserEntry user("CryActionRegression");
    ISaveGame* save = profile->CreateSaveGame(&user);
    CPlayerProfileManager::sUseRichSaveGames = previous;
    ACTION_CHECK(save != nullptr);
    if (save)
    {
        const int depth = kind == 1 ? 4 : 3;
        const int width = kind == 3 ? 0 : 3;
        constexpr int height = 2;
        std::array<uint8, 24> pixels{};
        for (size_t i = 0; i < pixels.size(); ++i) pixels[i] = static_cast<uint8>(i * 7 + 3);
        const auto unchanged = pixels;
        uint8* data = save->SetThumbnail(kind == 2 || kind == 3 ? nullptr : pixels.data(), width, height, depth);
        if (width != 0)
        {
            ACTION_CHECK(data != nullptr);
            if (data)
                for (int i = 0; i < width * height * depth; ++i)
                    ACTION_CHECK(data[i] == (kind == 2 ? 0 : pixels[static_cast<size_t>(i)]));
        }
        ACTION_CHECK(pixels == unchanged);
        // Replacement must release/replace the old payload without changing subsequent valid bytes.
        data = save->SetThumbnail(pixels.data(), 1, 1, 4);
        ACTION_CHECK(data && std::memcmp(data, pixels.data(), 4) == 0);
        ACTION_CHECK(save->SetThumbnail(pixels.data(), 65536, 65536, 4) == nullptr);
        ACTION_CHECK(data && std::memcmp(data, pixels.data(), 4) == 0);
        ACTION_CHECK(save->SetThumbnail(pixels.data(), -1, 1, 4) == nullptr);
        ACTION_CHECK(save->SetThumbnail(pixels.data(), 1, 1, 2) == nullptr);
        ACTION_CHECK(data && std::memcmp(data, pixels.data(), 4) == 0);
        ACTION_CHECK(!save->Complete(false)); // delete owner without writing an uninitialized save
    }
    profile->Release();
}

void CryActionRegressionProbe::Bitmap(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    if (testCase == CryActionRegressionCase::BmpSizeOverflow)
    {
        // No allocation: 65536 * 65536 * 4 is 16 GiB, representable on Win64.
        // Accept an explicit zero rejection or the correct size, never wrapped header-only size.
        const size_t actual = BMPHelper::CalcBMPSize(65536, 65536, 4);
        const size_t expected = (size_t{1} << 34) + 54;
        char description[128]{};
        std::snprintf(description, sizeof(description), "CalcBMPSize=%zu; expected rejection (0) or %zu", actual, expected);
        check(context, actual == 0 || actual == expected, description, __FILE__, __LINE__);
        return;
    }
    const int depth = testCase == CryActionRegressionCase::BmpRgbRows ? 3 : 4;
    const auto path = ScratchPath("thumbnail.bmp");
    ACTION_CHECK(!path.empty());
    if (path.empty()) return;
    for (int width = 1; width <= 4; ++width)
    {
        constexpr int height = 3;
        const size_t bytes = static_cast<size_t>(width * height * depth);
        std::vector<uint8> pixels(bytes);
        for (size_t i = 0; i < bytes; ++i) pixels[i] = static_cast<uint8>(i * 11 + width);
        const auto original = pixels;
        FILE* file = OpenFixture(path, "wb");
        ACTION_CHECK(file != nullptr);
        if (!file) continue;
        ACTION_CHECK(BMPHelper::SaveBMP(file, pixels.data(), width, height, depth, true));
        gEnv->pCryPak->FClose(file);
        file = OpenFixture(path, "rb");
        ACTION_CHECK(file != nullptr);
        if (!file) continue;
        std::array<uint8, 54> header{};
        ACTION_CHECK(gEnv->pCryPak->FReadRaw(header.data(), 1, header.size(), file) == header.size());
        uint32 fileSize = 0, dataOffset = 0, storedWidth = 0, storedHeight = 0;
        uint16 bits = 0;
        std::memcpy(&fileSize, header.data() + 2, 4);
        std::memcpy(&dataOffset, header.data() + 10, 4);
        std::memcpy(&storedWidth, header.data() + 18, 4);
        std::memcpy(&storedHeight, header.data() + 22, 4);
        std::memcpy(&bits, header.data() + 28, 2);
        const size_t row = (static_cast<size_t>(width * depth) + 3) & ~size_t{3};
        ACTION_CHECK(header[0] == 'B' && header[1] == 'M' && dataOffset == 54);
        ACTION_CHECK(storedWidth == width && storedHeight == height && bits == depth * 8);
        ACTION_CHECK(fileSize == 54 + row * height && fileSize == gEnv->pCryPak->FGetSize(file));
        ACTION_CHECK(BMPHelper::CalcBMPSize(width, height, depth) == fileSize);
        std::vector<uint8> disk(row * height);
        ACTION_CHECK(gEnv->pCryPak->FReadRaw(disk.data(), 1, disk.size(), file) == disk.size());
        for (int y = 0; y < height; ++y)
        {
            ACTION_CHECK(std::memcmp(disk.data() + y * row,
                pixels.data() + (height - y - 1) * width * depth, static_cast<size_t>(width * depth)) == 0);
            for (size_t i = static_cast<size_t>(width * depth); i < row; ++i)
                ACTION_CHECK(disk[static_cast<size_t>(y) * row + i] == 0);
        }
        int w = -1, h = -1, d = -1;
        gEnv->pCryPak->FSeek(file, 0, SEEK_SET);
        ACTION_CHECK(BMPHelper::LoadBMP(file, nullptr, w, h, d));
        ACTION_CHECK(gEnv->pCryPak->FTell(file) == 0);
        ACTION_CHECK(w == width && h == height && d == depth);
        std::vector<uint8> output(bytes + 2, 0xa5);
        ACTION_CHECK(BMPHelper::LoadBMP(file, output.data() + 1, w, h, d));
        ACTION_CHECK(output.front() == 0xa5 && output.back() == 0xa5);
        ACTION_CHECK(std::equal(pixels.begin(), pixels.end(), output.begin() + 1));
        ACTION_CHECK(pixels == original);
        gEnv->pCryPak->FClose(file);
    }
    ACTION_CHECK(DeleteFileA(path.c_str()) != FALSE);
}

void CryActionRegressionProbe::Metadata(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    IMetadataPtr value;
    value->SetTag(0x74657374);
    std::array<uint8, 255> bytes{};
    for (size_t i = 0; i < bytes.size(); ++i) bytes[i] = static_cast<uint8>(i ^ 0x5a);
    const uint8 count = testCase == CryActionRegressionCase::Metadata1 ? 1 :
        testCase == CryActionRegressionCase::Metadata254 ? 254 : 255;
    ACTION_CHECK(value->SetValue(0x62797465, bytes.data(), count));
    ACTION_CHECK(value->GetValueSize() == count);
    uint8 capacity = 0;
    ACTION_CHECK(!value->GetValue(nullptr, &capacity) && capacity == count);
    std::array<uint8, 257> result{};
    result.fill(0xa5);
    uint8 shortCapacity = static_cast<uint8>(count - 1);
    ACTION_CHECK(!value->GetValue(result.data() + 1, &shortCapacity) && shortCapacity == count);
    ACTION_CHECK(std::all_of(result.begin(), result.end(), [](uint8 b) { return b == 0xa5; }));
    capacity = count;
    ACTION_CHECK(value->GetValue(result.data() + 1, &capacity) && capacity == count);
    ACTION_CHECK(result.front() == 0xa5 && result[static_cast<size_t>(count) + 1] == 0xa5);
    ACTION_CHECK(std::equal(bytes.begin(), bytes.begin() + count, result.begin() + 1));
    if (testCase == CryActionRegressionCase::MetadataClone)
    {
        IMetadataPtr parent;
        parent->SetTag(0x726f6f74);
        ACTION_CHECK(parent->AddField(value.get()));
        IMetadata* clone = parent->Clone();
        ACTION_CHECK(clone != nullptr);
        parent->Reset();
        value->Reset();
        if (clone)
        {
            ACTION_CHECK(clone->GetNumFields() == 1 && clone->GetFieldByIndex(1) == nullptr);
            const auto* child = clone->GetFieldByIndex(0);
            ACTION_CHECK(child != nullptr && child->GetTag() == 0x74657374 && child->GetValueSize() == count);
            if (child)
            {
                capacity = count;
                ACTION_CHECK(child->GetValue(result.data() + 1, &capacity));
                ACTION_CHECK(std::equal(bytes.begin(), bytes.end(), result.begin() + 1));
            }
            clone->Delete();
        }
    }
}

void CryActionRegressionProbe::ItemParams(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    auto* root = new CItemParamsNode;
    const int emptyBytes = root->GetMemorySize();
    ACTION_CHECK(root->GetAttributeCount() == 0 && root->GetChildCount() == 0);
    ACTION_CHECK(emptyBytes >= sizeof(CItemParamsNode));
    if (testCase == CryActionRegressionCase::ItemParamsTree)
    {
        XmlNodeRef xml = gEnv->pSystem->CreateXmlNode("item");
        xml->setAttr("name", "fixture");
        xml->setAttr("capacity", "17");
        xml->setAttr("scale", "1.25");
        xml->newChild("first")->setAttr("enabled", "true");
        xml->newChild("second")->setAttr("enabled", "false");
        root->ConvertFromXML(xml);
        int capacity = 0, enabled = -1;
        float scale = 0;
        ACTION_CHECK(root->GetAttributeCount() == 3 && root->GetChildCount() == 2);
        ACTION_CHECK(root->GetAttribute("capacity", capacity) && capacity == 17);
        ACTION_CHECK(root->GetAttribute("scale", scale) && scale == 1.25f);
        ACTION_CHECK(root->GetChild(0)->GetAttribute("enabled", enabled) && enabled == 1);
        ACTION_CHECK(std::strcmp(root->GetChildName(0), "first") == 0);
        ACTION_CHECK(std::strcmp(root->GetChildName(1), "second") == 0);
        const int beforeName = root->GetMemorySize();
        root->SetName("1234567");
        ACTION_CHECK(root->GetMemorySize() == beforeName + 7);
        ACTION_CHECK(root->GetMemorySize() >= emptyBytes + root->GetChild(0)->GetMemorySize() +
            root->GetChild(1)->GetMemorySize());
    }
    root->Release();
}

void CryActionRegressionProbe::ModuleImage(CryActionRegressionCheck check, void* context)
{
    MEMORY_BASIC_INFORMATION memory{};
    ACTION_CHECK(VirtualQuery(reinterpret_cast<const void*>(&CryActionRunRegressionCase), &memory, sizeof(memory)) != 0);
    const auto* base = static_cast<const uint8*>(memory.AllocationBase);
    if (!base) return;
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    ACTION_CHECK(dos->e_magic == IMAGE_DOS_SIGNATURE);
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    ACTION_CHECK(nt->Signature == IMAGE_NT_SIGNATURE);
    ACTION_CHECK(nt->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64);
    ACTION_CHECK(nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);
    ACTION_CHECK(GetProcAddress(static_cast<HMODULE>(memory.AllocationBase), "CreateGameFramework") != nullptr);
    ACTION_CHECK(FindResourceA(static_cast<HMODULE>(memory.AllocationBase), MAKEINTRESOURCEA(1), RT_VERSION) != nullptr);
}

void CryActionRegressionProbe::GraphXml(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    auto graph = CAnimationGraph::CreateInstance(nullptr, "CryActionXmlRegression");
    XmlNodeRef root = gEnv->pSystem->CreateXmlNode("AnimationGraph");
    XmlNodeRef states = root->newChild("States");
    root->newChild("Transitions");
    root->newChild("Views");
    const int values = testCase == CryActionRegressionCase::GraphXmlEmpty ? 0 :
        testCase == CryActionRegressionCase::GraphXmlOneValue ? 1 : 3;
    if (values)
    {
        auto state = states->newChild("State");
        state->setAttr("id", "walk_[direction]");
        state->setAttr("animation", "anim_[direction]");
        state->setAttr("untouched", "literal");
        auto nested = state->newChild("SelectWhen");
        nested->newChild("Action")->setAttr("value", "[direction]");
        auto parameters = state->newChild("Parameterization");
        auto parameter = parameters->newChild("Parameter");
        parameter->setAttr("id", "direction");
        const char* directions[]{"back", "forward", "left"};
        for (int i = 0; i < values; ++i)
            parameter->newChild("Value")->setAttr("id", directions[i]);
        if (testCase == CryActionRegressionCase::GraphXmlExclude)
        {
            auto overrideNode = parameters->newChild("Override");
            overrideNode->setAttr("id", "forward");
            overrideNode->newChild("ExcludeFromGraph");
        }
    }
    ACTION_CHECK(graph->PreprocessParameterization(root));
    const int expected = values - (testCase == CryActionRegressionCase::GraphXmlExclude ? 1 : 0);
    ACTION_CHECK(states->getChildCount() == expected);
    std::set<std::string> names;
    for (int i = 0; i < states->getChildCount(); ++i)
    {
        const auto state = states->getChild(i);
        const std::string name = state->getAttr("id");
        names.insert(name);
        ACTION_CHECK(name.starts_with("walk_") && name.find('[') == std::string::npos);
        const std::string direction = name.substr(5);
        ACTION_CHECK(std::string(state->getAttr("animation")) == "anim_" + direction);
        ACTION_CHECK(std::strcmp(state->getAttr("untouched"), "literal") == 0);
        const auto selection = state->findChild("SelectWhen");
        ACTION_CHECK(selection && selection->getChildCount() == 1);
        if (selection && selection->getChildCount() == 1)
            ACTION_CHECK(std::string(selection->getChild(0)->getAttr("value")) == direction);
        ACTION_CHECK(!state->findChild("Parameterization"));
    }
    ACTION_CHECK(names.size() == static_cast<size_t>(expected));
    if (values) ACTION_CHECK(names.contains("walk_back"));
    if (values == 3) ACTION_CHECK(names.contains("walk_left"));
    if (testCase == CryActionRegressionCase::GraphXmlExclude) ACTION_CHECK(!names.contains("walk_forward"));
    ACTION_CHECK(root->findChild("Transitions")->getChildCount() == 0);
}

void CryActionRegressionProbe::Inventory(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    const size_t count = testCase == CryActionRegressionCase::InventoryEmpty ? 0 :
        testCase == CryActionRegressionCase::InventorySingle ? 1 : 3;
    const std::array<EntityId, 3> ids{1, 0x80000001u, 0xfffffffeu};
    CInventory original;
    original.SetCapacity(7);
    original.m_stats.slots.assign(ids.begin(), ids.begin() + count);
    original.m_stats.currentItemId = count ? ids[0] : 0;
    original.m_stats.holsteredItemId = count == 3 ? ids[1] : 0;
    original.m_stats.lastItemId = count == 3 ? ids[2] : 0;
    XmlNodeRef node = gEnv->pSystem->CreateXmlNode("InventoryFixture");
    _smart_ptr<IXmlSerializer> serializer = gEnv->pSystem->GetXmlUtils()->CreateXmlSerializer();
    ACTION_CHECK(!!serializer);
    if (!serializer) return;
    original.FullSerialize(TSerialize(serializer->GetWriter(node)));
    auto items = node->findChild("InventoryItems");
    ACTION_CHECK(!!items);
    if (items)
    {
        int size = -1, capacity = -1;
        EntityId current = 0;
        // The XML serializer deliberately omits default zero-valued attributes.
        const bool hasSize = items->getAttr("InventorySize", size);
        ACTION_CHECK(hasSize == (count != 0));
        ACTION_CHECK((hasSize ? size : 0) == count);
        ACTION_CHECK(items->getAttr("Capacity", capacity) && capacity == 7);
        const bool hasCurrent = items->getAttr("CurrentItem", current);
        ACTION_CHECK(hasCurrent == (original.m_stats.currentItemId != 0));
        ACTION_CHECK(current == original.m_stats.currentItemId);
    }
    CInventory restored;
    restored.FullSerialize(TSerialize(serializer->GetReader(node)));
    ACTION_CHECK(restored.GetCount() == count && restored.GetCapacity() == 7);
    for (size_t i = 0; i < count; ++i)
        ACTION_CHECK(restored.GetItem(static_cast<int>(i)) == ids[i]);
    ACTION_CHECK(restored.GetItem(-1) == 0 && restored.GetItem(static_cast<int>(count)) == 0);
    ACTION_CHECK(restored.GetCurrentItem() == original.GetCurrentItem());
    ACTION_CHECK(restored.GetHolsteredItem() == original.GetHolsteredItem());
    ACTION_CHECK(restored.GetLastItem() == original.GetLastItem());
    ACTION_CHECK(original.m_stats.slots.size() == count);
}
