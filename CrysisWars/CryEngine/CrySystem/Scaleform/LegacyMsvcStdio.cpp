#include "StdAfx.h"
#include "LegacyMsvcStdio.h"

static FILE* __cdecl CryLegacyMsvcIobFunction()
{
	return CLegacyMsvcStdioBridge::MakeIobBase(
		stderr,
		CLegacyMsvcStdioBridge::StandardErrorIndex);
}

typedef FILE* (__cdecl* LegacyMsvcIobFunction)();

extern "C"
{
	LegacyMsvcIobFunction __imp___iob_func = &CryLegacyMsvcIobFunction;
}
