#pragma once

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#include <cstddef>
// Windows-boundary policy shared by the actual framework and ViewNote owners.
class CryActionWindows
{
public:
    static const char* RegistryPath();
    // Accept legacy REG_SZ values without NUL when there is append room.
    // Errors leave an empty output; a full, unterminated value is rejected.
    static LONG ReadKey(HKEY root, const char* path, char* output, size_t capacity);
    // length excludes the caller-owned trailing NUL, which is persisted too.
    static LONG WriteKey(HKEY root, const char* path, const char* text, size_t length);
    static bool BuildViewNotePath(char* output, size_t capacity,
        DWORD (WINAPI* getTempPath)(DWORD, LPSTR) = GetTempPathA);
#if defined(CRY_ACTION_REGRESSION_TESTS)
    static const char* registryOverride;
    static void (*keyObserver)(const char*);
#endif
};
#endif
