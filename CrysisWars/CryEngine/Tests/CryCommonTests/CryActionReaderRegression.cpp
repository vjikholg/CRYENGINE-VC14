#include "StdAfx.h"
#include "CryActionRegressionProbe.h"
#include <IAnimationStateNode.h>
#include <AnimationGraph/AnimationGraph.h>
#include <AnimationGraph/AnimationGraphManager.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#define READ_CHECK(expression) check(context, (expression) ? 1u : 0u, #expression, __FILE__, __LINE__)

void CryActionRegressionProbe::ReaderCases(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    if (testCase >= CryActionRegressionCase::ReadFileEmpty && testCase <= CryActionRegressionCase::ReadFileFixture)
    {
        ReaderFiles(testCase, check, context);
        return;
    }
    if (testCase >= CryActionRegressionCase::ReadStringMissingTerminator && testCase <= CryActionRegressionCase::ReadStringSequence)
    {
        ReaderStrings(testCase, check, context);
        return;
    }
    AG_FILE input{};
    FileSerializationHelper reader;
    reader.ag_file = &input;
    reader.reading = true;
    reader.swappingEndian = false;
    const std::array<uint8, 8> bytes{0x01, 0x02, 0x03, 0x04, 0x11, 0x22, 0x33, 0x44};
    input.m_buffer.resize(static_cast<int>(bytes.size()));
    std::memcpy(input.m_buffer.begin(), bytes.data(), bytes.size());
    if (testCase == CryActionRegressionCase::ReadValueWithoutFile)
    {
        uint32 output = 0xdeadbeef;
        string text("unchanged");
        reader.Value(&output);
        reader.StringValue(&text);
        READ_CHECK(output == 0xdeadbeef && text == "unchanged");
        READ_CHECK(input.m_curpos == 0);
    }
    else if (testCase == CryActionRegressionCase::ReadExactSuffix)
    {
        input.m_curpos = 4;
        uint32 output = 0;
        reader.ReadFromBuffer(&output);
        READ_CHECK(output == 0x44332211);
        READ_CHECK(input.m_curpos == bytes.size());
    }
    else if (testCase == CryActionRegressionCase::ReadByteCanaries)
    {
        std::array<uint8, 10> output{};
        output.fill(0xa5);
        reader.ReadFromBuffer(output.data() + 1, static_cast<int>(bytes.size()));
        READ_CHECK(output.front() == 0xa5 && output.back() == 0xa5);
        READ_CHECK(std::equal(bytes.begin(), bytes.end(), output.begin() + 1));
        READ_CHECK(input.m_curpos == bytes.size());
    }
    else if (testCase == CryActionRegressionCase::ReadWordCanaries)
    {
        const std::array<uint16, 3> words{0x1234, 0x5678, 0x9abc};
        input.m_buffer.resize(7);
        input.m_buffer[0] = 0xff;
        std::memcpy(input.m_buffer.begin() + 1, words.data(), sizeof(words));
        input.m_curpos = 1;
        std::array<uint16, 5> output{0xa5a5, 0, 0, 0, 0x5a5a};
        reader.ReadFromBuffer(output.data() + 1, static_cast<int>(words.size()));
        READ_CHECK(output.front() == 0xa5a5 && output.back() == 0x5a5a);
        READ_CHECK(std::equal(words.begin(), words.end(), output.begin() + 1));
        READ_CHECK(input.m_curpos == 7);
        READ_CHECK(input.m_buffer[0] == 0xff);
        READ_CHECK(std::memcmp(input.m_buffer.begin() + 1, words.data(), sizeof(words)) == 0);
        return;
    }
    else if (testCase == CryActionRegressionCase::ReadArrayTruncated)
    {
        input.m_buffer.resize(7);
        std::array<uint32, 4> output{0x11111111, 0x22222222, 0x33333333, 0x44444444};
        const auto unchanged = output;
        reader.ReadFromBuffer(output.data() + 1, 2);
        READ_CHECK(output == unchanged);
        READ_CHECK(input.m_curpos == 0);
        READ_CHECK(input.m_curpos <= static_cast<size_t>(input.m_buffer.size()));
        return;
    }
    else
    {
        uint32 output = 0xdeadbeef;
        int count = 1;
        if (testCase == CryActionRegressionCase::ReadNegativeElements) count = -1;
        if (testCase == CryActionRegressionCase::ReadCursorBeyondEnd) input.m_curpos = bytes.size() + 1;
        if (testCase == CryActionRegressionCase::ReadZeroAtEnd) { input.m_curpos = bytes.size(); count = 0; }
        const size_t cursor = input.m_curpos;
        reader.ReadFromBuffer(&output, count);
        READ_CHECK(output == 0xdeadbeef);
        READ_CHECK(input.m_curpos == cursor);
    }
    READ_CHECK(input.m_buffer.size() == bytes.size());
    READ_CHECK(std::equal(bytes.begin(), bytes.end(), input.m_buffer.begin()));
}

void CryActionRegressionProbe::ReaderStrings(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    const auto path = ScratchPath("string-reader-marker.bin");
    READ_CHECK(!path.empty());
    if (path.empty()) return;
    FILE* file = nullptr;
    READ_CHECK(fopen_s(&file, path.c_str(), "w+b") == 0 && file != nullptr);
    if (!file) return;
    auto close = [&](FILE* handle) { std::fclose(handle); DeleteFileA(path.c_str()); };
    std::unique_ptr<FILE, decltype(close)> guard(file, close);
    AG_FILE input{};
    input.file = file;
    FileSerializationHelper reader;
    reader.file = file;
    reader.ag_file = &input;
    reader.reading = true;
    reader.swappingEndian = false;

    if (testCase == CryActionRegressionCase::ReadStringLengths)
    {
        // The final small records exercise reuse after the largest temporary string buffer.
        for (const int count : {0, 1, 255, 256, 1023, 3, 0})
        {
            std::string expected(static_cast<size_t>(count), 'x');
            for (int i = 0; i < count; ++i) expected[static_cast<size_t>(i)] = static_cast<char>('A' + i % 26);
            const int encodedLength = count + 1;
            input.m_buffer.resize(static_cast<int>(sizeof(encodedLength)) + encodedLength);
            input.m_curpos = 0;
            std::memcpy(input.m_buffer.begin(), &encodedLength, sizeof(encodedLength));
            std::memcpy(input.m_buffer.begin() + sizeof(encodedLength), expected.c_str(), static_cast<size_t>(encodedLength));
            const auto original = input.m_buffer;
            string actual("unchanged");
            reader.StringValue(&actual);
            READ_CHECK(actual.length() == expected.size());
            READ_CHECK(std::memcmp(actual.c_str(), expected.c_str(), expected.size() + 1) == 0);
            READ_CHECK(input.m_curpos == static_cast<size_t>(input.m_buffer.size()));
            READ_CHECK(input.m_strBuffer.size() >= encodedLength && input.m_strBuffer[encodedLength] == 0);
            READ_CHECK(std::equal(original.begin(), original.end(), input.m_buffer.begin()));
        }
        return;
    }
    if (testCase == CryActionRegressionCase::ReadStringSequence)
    {
        const std::array<uint8, 20> fixture{
            1, 0, 0, 0, 0, 4, 0, 0, 0, 'a', 'b', 'c', 0, 2, 0, 0, 0, 'Z', 0, 0xa5};
        input.m_buffer.resize(static_cast<int>(fixture.size()));
        std::memcpy(input.m_buffer.begin(), fixture.data(), fixture.size());
        string text;
        reader.StringValue(&text);
        READ_CHECK(text.empty() && input.m_curpos == 5);
        reader.StringValue(&text);
        READ_CHECK(text == "abc" && input.m_curpos == 13);
        reader.StringValue(&text);
        READ_CHECK(text == "Z" && input.m_curpos == 19);
        uint8 suffix = 0;
        reader.Value(&suffix);
        READ_CHECK(suffix == 0xa5 && input.m_curpos == fixture.size());
        READ_CHECK(std::equal(fixture.begin(), fixture.end(), input.m_buffer.begin()));
        return;
    }

    const int declaredLength = testCase == CryActionRegressionCase::ReadStringZeroLength ? 0 : 4;
    const int payloadLength = testCase == CryActionRegressionCase::ReadStringZeroLength ? 0 :
        testCase == CryActionRegressionCase::ReadStringTruncatedPayload ? 2 : 4;
    input.m_buffer.resize(static_cast<int>(sizeof(declaredLength)) + payloadLength);
    std::memcpy(input.m_buffer.begin(), &declaredLength, sizeof(declaredLength));
    const std::array<uint8, 4> payload = testCase == CryActionRegressionCase::ReadStringTruncatedPayload ?
        std::array<uint8, 4>{'A', 0, 0, 0} : std::array<uint8, 4>{'A', 'B', 'C', 'D'};
    if (payloadLength)
        std::memcpy(input.m_buffer.begin() + sizeof(declaredLength), payload.data(), static_cast<size_t>(payloadLength));
    const auto original = input.m_buffer;
    string actual("unchanged");
    reader.StringValue(&actual);
    // These are proposed rejection/transactionality contracts for malformed file data,
    // not an assertion that the current void reader already implements error reporting.
    READ_CHECK(actual == "unchanged");
    READ_CHECK(input.m_curpos == 0);
    READ_CHECK(input.m_curpos <= static_cast<size_t>(input.m_buffer.size()));
    READ_CHECK(std::equal(original.begin(), original.end(), input.m_buffer.begin()));
}

void CryActionRegressionProbe::ReaderFiles(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
    const auto path = ScratchPath("animation-reader.bin");
    READ_CHECK(!path.empty());
    if (path.empty()) return;
    const std::array<uint8, 15> golden{1, 2, 3, 4, 4, 0, 0, 0, 'a', 'b', 'c', 0, 0x11, 0x22, 0xa5};
    AG_FILE output{};
    output.file = OpenFixture(path, "wb");
    READ_CHECK(output.file != nullptr);
    if (!output.file) return;
    if (testCase == CryActionRegressionCase::ReadFileRecord)
    {
        FileSerializationHelper writer(false, &output);
        uint32 prefix = 0x04030201;
        uint16 suffix = 0x2211;
        uint8 tail = 0xa5;
        string text("abc");
        writer.Value(&prefix);
        writer.StringValue(&text);
        writer.Value(&suffix);
        writer.Value(&tail);
        READ_CHECK(text == "abc" && prefix == 0x04030201 && suffix == 0x2211 && tail == 0xa5);
    }
    else if (testCase == CryActionRegressionCase::ReadFileFixture)
    {
        // FWrite is void; verify the resulting file position and the independent
        // byte fixture on readback instead of inventing a returned byte count.
        gEnv->pCryPak->FWrite(golden.data(), golden.size(), output.file);
        READ_CHECK(gEnv->pCryPak->FTell(output.file) == golden.size());
    }
    gEnv->pCryPak->FClose(output.file);

    AG_FILE input{};
    input.file = OpenFixture(path, "rb");
    READ_CHECK(input.file != nullptr);
    if (!input.file) { DeleteFileA(path.c_str()); return; }
    input.SetFile(true, input.file);
    READ_CHECK(input.m_curpos == 0);
    const size_t expectedSize = testCase == CryActionRegressionCase::ReadFileEmpty ? 0 : golden.size();
    READ_CHECK(input.m_buffer.size() == expectedSize);
    READ_CHECK(gEnv->pCryPak->FTell(input.file) == expectedSize);
    READ_CHECK(gEnv->pCryPak->FGetSize(input.file) == expectedSize);
    FileSerializationHelper reader(true, &input);
    if (expectedSize == 0)
    {
        uint32 untouched = 0xdeadbeef;
        reader.Value(&untouched, 0);
        READ_CHECK(untouched == 0xdeadbeef && input.m_curpos == 0);
    }
    else
    {
        // The literal byte fixture is independent of the production encoder/decoder.
        READ_CHECK(input.m_buffer.size() == golden.size() &&
            std::equal(golden.begin(), golden.end(), input.m_buffer.begin()));
        uint32 prefix = 0;
        uint16 suffix = 0;
        uint8 tail = 0;
        string text("unchanged");
        reader.Value(&prefix);
        READ_CHECK(prefix == 0x04030201 && input.m_curpos == 4);
        reader.StringValue(&text);
        READ_CHECK(text == "abc" && input.m_curpos == 12);
        reader.Value(&suffix);
        READ_CHECK(suffix == 0x2211 && input.m_curpos == 14);
        reader.Value(&tail);
        READ_CHECK(tail == 0xa5 && input.m_curpos == golden.size());
        READ_CHECK(std::equal(golden.begin(), golden.end(), input.m_buffer.begin()));
    }
    gEnv->pCryPak->FClose(input.file);
    READ_CHECK(DeleteFileA(path.c_str()) != FALSE);
    if (testCase == CryActionRegressionCase::ReadFileFixture)
    {
        CAnimationGraphManager manager;
        auto graph = CAnimationGraph::CreateInstance(&manager, "BinaryReaderFixture");
        auto root = gEnv->pSystem->CreateXmlNode("AnimationGraph");
        root->newChild("Inputs");
        root->newChild("Views");
        auto state = root->newChild("States")->newChild("State");
        state->setAttr("id", "OnlyState");
        state->setAttr("cost", 0);
        state->setAttr("allowSelect", false);
        root->newChild("Transitions");
        READ_CHECK(graph->Init(root));
        READ_CHECK(graph->SerializeAsFile(path.c_str(), false));
        auto loaded = CAnimationGraph::CreateInstance(&manager, "BinaryReaderLoaded");
        READ_CHECK(loaded->SerializeAsFile(path.c_str(), true));
        READ_CHECK(loaded->StateNameToID("OnlyState") == graph->StateNameToID("OnlyState"));

        FILE* file = OpenFixture(path, "rb");
        READ_CHECK(file);
        if (!file) return;
        const size_t length = gEnv->pCryPak->FGetSize(file);
        std::vector<uint8> bytes(length);
        READ_CHECK(gEnv->pCryPak->FReadRaw(bytes.data(), 1, bytes.size(), file) == bytes.size());
        gEnv->pCryPak->FClose(file);
        file = OpenFixture(path, "wb");
        READ_CHECK(file && length > 1);
        if (!file) return;
        gEnv->pCryPak->FWrite(bytes.data(), 1, length - 1, file);
        gEnv->pCryPak->FClose(file);
        auto truncated = CAnimationGraph::CreateInstance(&manager, "BinaryReaderTruncated");
        READ_CHECK(!truncated->SerializeAsFile(path.c_str(), true));
        READ_CHECK(DeleteFileA(path.c_str()) != FALSE); // Failure closed the file.
    }
}
