#pragma once

class CryTestRunner;

void RunMM06SizeArithmeticTests(CryTestRunner& runner);
int RunMM06RuntimeTrace(const wchar_t* crySystemPath);
int RunMM06CrashProbe(
	const wchar_t* crySystemPath,
	const wchar_t* dumpDirectory);

