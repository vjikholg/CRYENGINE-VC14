#include "StdAfx.h"
#include "ViewNote.h"
#include "CryActionWindows.h"
#include <memory>
#include <vector>

#ifdef WIN32
SWaitInfo::SWaitInfo() : pi(), handle(nullptr), szFile(nullptr), references(2)
{
    InitializeCriticalSection(&registrationLock);
}

SWaitInfo::~SWaitInfo()
{
    DeleteCriticalSection(&registrationLock);
    if (pi.hThread) CloseHandle(pi.hThread);
    if (pi.hProcess) CloseHandle(pi.hProcess);
    delete[] szFile;
}

void SWaitInfo::Release()
{
    if (InterlockedDecrement(&references) == 0) delete this;
}
#endif

void ViewNote(IConsoleCmdArgs*)
{
#ifdef WIN32
    char path[1024];
    if (!CryActionWindows::BuildViewNotePath(path, sizeof(path)))
    {
        CryLog("ViewNote: temporary path unavailable or too long");
        return;
    }
    std::unique_ptr<SWaitInfo> info(new SWaitInfo);
    info->szFile = new char[strlen(path) + 1];
    strcpy(info->szFile, path);
    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        CryLog("Could not open file %s (error %lu)", path, GetLastError());
        return;
    }
    CloseHandle(file);
    char command[sizeof(path) + 32];
    snprintf(command, sizeof(command), "notepad.exe \"%s\"", path);
    STARTUPINFOA startup = {};
    startup.cb = sizeof(startup);
    startup.dwX = startup.dwY = startup.dwYSize = 100;
    startup.dwXSize = 300;
    startup.dwFlags = STARTF_USEPOSITION | STARTF_USESIZE;
    if (!CreateProcessA(nullptr, command, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &info->pi))
    {
        CryLog("ViewNote: CreateProcess failed (error %lu)", GetLastError());
        DeleteFileA(path);
        return;
    }
    SWaitInfo* shared = info.get();
    EnterCriticalSection(&shared->registrationLock);
    if (!RegisterWaitForSingleObject(&shared->handle, shared->pi.hProcess,
        WaitForViewNote, shared, INFINITE, WT_EXECUTEONLYONCE))
    {
        CryLog("ViewNote: RegisterWaitForSingleObject failed (error %lu)", GetLastError());
        LeaveCriticalSection(&shared->registrationLock);
        return;
    }
    info.release();
    LeaveCriticalSection(&shared->registrationLock);
    shared->Release();
#endif
}

#ifdef WIN32
void CALLBACK WaitForViewNote(void* state, BOOLEAN)
{
    SWaitInfo* info = static_cast<SWaitInfo*>(state);
    if (!info) return;
    EnterCriticalSection(&info->registrationLock);
    UnregisterWait(info->handle);
    LeaveCriticalSection(&info->registrationLock);

    HANDLE file = CreateFileA(info->szFile, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        CryLog("Could not open file %s (error %lu)", info->szFile, GetLastError());
    else
    {
        LARGE_INTEGER size;
        if (GetFileSizeEx(file, &size) && size.QuadPart >= 0 && size.QuadPart <= 1024 * 1024)
        {
            std::vector<char> text(static_cast<size_t>(size.QuadPart) + 1, 0);
            DWORD read = 0;
            if (ReadFile(file, text.data(), static_cast<DWORD>(text.size() - 1), &read, nullptr))
            {
                text[read] = 0;
                CryLog("ViewNote: %s", text.data());
            }
            else CryLog("ViewNote: ReadFile failed (error %lu)", GetLastError());
        }
        else CryLog("ViewNote: invalid or oversized note file");
        CloseHandle(file);
        DeleteFileA(info->szFile);
    }
    info->Release();
}
#endif
