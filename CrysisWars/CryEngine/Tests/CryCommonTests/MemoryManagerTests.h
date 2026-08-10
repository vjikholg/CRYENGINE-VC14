#pragma once

class CryTestRunner;

void RunMemoryManagerLayoutTests(CryTestRunner& runner);
int RunMM02MemoryManagementTrace();
int RunMM02RuntimeTrace(const wchar_t* crySystemPath);
int RunMM02OverflowProbe(const wchar_t* crySystemPath);
int RunMM02ReallocFailureProbe(const wchar_t* crySystemPath);
