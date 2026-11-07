#include "StdAfx.h"
#include "CryActionWindows.h"
#include <limits>
#include <cstring>

#if defined(WIN32) || defined(WIN64)
#if defined(CRY_ACTION_REGRESSION_TESTS)
const char* CryActionWindows::registryOverride = nullptr;
void (*CryActionWindows::keyObserver)(const char*) = nullptr;
#endif

const char* CryActionWindows::RegistryPath()
{
#if defined(CRY_ACTION_REGRESSION_TESTS)
    if (registryOverride) return registryOverride;
#endif
    return "SOFTWARE\\Electronic Arts\\Electronic Arts\\Crysis Wars\\ergc\\";
}

LONG CryActionWindows::ReadKey(HKEY root, const char* path, char* output, size_t capacity)
{
    if (!output || capacity == 0 || capacity > (std::numeric_limits<DWORD>::max)()) return ERROR_INVALID_PARAMETER;
    output[0] = 0;
    HKEY key = nullptr;
    LONG result = RegOpenKeyExA(root, path, 0, KEY_READ | KEY_WOW64_32KEY, &key);
    if (result != ERROR_SUCCESS) return result;
    DWORD type = 0, bytes = static_cast<DWORD>(capacity);
    result = RegQueryValueExA(key, nullptr, nullptr, &type, reinterpret_cast<BYTE*>(output), &bytes);
    RegCloseKey(key);
    if (result == ERROR_SUCCESS)
    {
        if (type != REG_SZ) result = ERROR_BAD_FORMAT;
        else if (bytes < capacity) output[bytes] = 0; // Legacy writer omitted NUL.
        else if (!std::memchr(output, 0, capacity)) result = ERROR_MORE_DATA;
    }
    if (result != ERROR_SUCCESS) output[0] = 0;
    return result;
}

LONG CryActionWindows::WriteKey(HKEY root, const char* path, const char* text, size_t length)
{
    if (!text || length >= (std::numeric_limits<DWORD>::max)()) return ERROR_INVALID_PARAMETER;
    HKEY key = nullptr;
    LONG result = RegCreateKeyExA(root, path, 0, nullptr, REG_OPTION_NON_VOLATILE,
        KEY_WRITE | KEY_WOW64_32KEY, nullptr, &key, nullptr);
    if (result != ERROR_SUCCESS) return result;
    result = RegSetValueExA(key, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(text), static_cast<DWORD>(length + 1));
    RegCloseKey(key);
    return result;
}

bool CryActionWindows::BuildViewNotePath(char* output, size_t capacity, DWORD (WINAPI* getTempPath)(DWORD, LPSTR))
{
    static const char fileName[] = "viewnote.txt";
    if (!output || capacity == 0) return false;
    output[0] = 0;
    if (!getTempPath || capacity <= sizeof(fileName) || capacity > (std::numeric_limits<DWORD>::max)()) return false;
    const DWORD available = static_cast<DWORD>(capacity - (sizeof(fileName) - 1));
    const DWORD length = getTempPath(available, output);
    if (!length || length >= available) { output[0] = 0; return false; }
    std::memcpy(output + length, fileName, sizeof(fileName));
    return true;
}
#endif
