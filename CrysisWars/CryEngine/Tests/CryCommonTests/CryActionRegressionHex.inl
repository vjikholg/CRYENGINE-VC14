// Included only by the test-enabled GameContext translation unit, beside the real helpers.
#include "CryActionRegressionProbe.h"
#include <array>
#include <cstring>

void CryActionRegressionProbe::Hex(
    CryActionRegressionCase testCase, CryActionRegressionCheck check, void* context)
{
#define HEX_CHECK(expression) check(context, (expression) ? 1u : 0u, #expression, __FILE__, __LINE__)
    if (testCase == CryActionRegressionCase::HexInvalid)
    {
        string text("01xz23");
        HEX_CHECK(!FromHexStr(text));
        HEX_CHECK(text == "01xz23");
    }
    else
    {
        const std::array<unsigned char, 9> bytes{0x00, 0x01, 0x7f, 0x80, 0xff, 0x42, 0x10, 0x99, 0xab};
        const int counts[]{0, 1, 7, 8, 9};
        const char* golden = "00017f80ff421099ab";
        for (const int count : counts)
        {
            if (testCase == CryActionRegressionCase::HexEmpty && count != 0) continue;
            string encoded = ToHexStr(reinterpret_cast<const char*>(bytes.data()), count);
            HEX_CHECK(encoded.length() == static_cast<size_t>(count) * 2);
            HEX_CHECK(std::strncmp(encoded.c_str(), golden, encoded.length()) == 0);
            HEX_CHECK(FromHexStr(encoded));
            HEX_CHECK(encoded.length() == static_cast<size_t>(count));
            HEX_CHECK(std::memcmp(encoded.data(), bytes.data(), static_cast<size_t>(count)) == 0);
        }
    }
#undef HEX_CHECK
}
