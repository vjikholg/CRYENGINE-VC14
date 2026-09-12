#include "CryNetworkTestPreamble.h"

#include <StdAfx.h>
#include <Context/NetContextState.h>

#include <cstdlib>

void NetWarning(const char*, ...)
{
}

// The string-codec tests link ArithModel.cpp without the full NetContextState
// runtime. Any accidental entity-ID decoding in this runner must fail loudly.
void CNetContextState::Resaltify(SNetObjectID&)
{
	std::abort();
}
