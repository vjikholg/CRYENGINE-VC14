#pragma once

class CryTestRunner;

void RunMM04DataModelTests(CryTestRunner& runner);
int RunMM04DataModelTrace();
int RunMM04AccountingTrace(const wchar_t* crySystemPath);
