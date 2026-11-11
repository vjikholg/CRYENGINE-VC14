#pragma once

#if defined(_WIN32)
// The buffer remains valid for the rest of the process and includes its final NUL.
extern "C" DLL_EXPORT unsigned long CryCaptureCrashBreadcrumbs(const char** text);
void WriteCrashBreadcrumbsSidecar();
#endif
