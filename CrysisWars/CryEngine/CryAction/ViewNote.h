#pragma once

struct IConsoleCmdArgs;
void ViewNote(IConsoleCmdArgs* args);

#ifdef WIN32
#include <windows.h>
// Registering thread and callback retain a reference each. The lock publishes
// the wait handle before an already-exited editor can notify the callback.
struct SWaitInfo
{
 SWaitInfo();
 ~SWaitInfo();
 void Release();
 PROCESS_INFORMATION pi;
 HANDLE handle;
 char* szFile;
 CRITICAL_SECTION registrationLock;
 LONG references;
};
void CALLBACK WaitForViewNote(void* state, BOOLEAN timedOut);
#endif
