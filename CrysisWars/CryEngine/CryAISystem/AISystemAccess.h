#ifndef __AISYSTEMACCESS_H__
#define __AISYSTEMACCESS_H__

#if _MSC_VER > 1000
#pragma once
#endif

class CAISystem;

extern CAISystem* g_pAISystem;

inline CAISystem* GetAISystem()
{
	return g_pAISystem;
}

#endif // __AISYSTEMACCESS_H__
