#ifndef __CRYDXPSGCM_MEMMAN__
#define __CRYDXPSGCM_MEMMAN__

#include "CCryDXPSGCM_MemConfig.hpp"
#include "CCryDXPSGCM_MemItemList.hpp"
#include "CCryDXPSGCM_MemBank.hpp"

class CCryDXPSGCMMemMan
{
private:
	CCryDXPSGCMMemItemList	m_ItemFreelist;
	CCryDXPSGCMMemBank			m_Bank[CRY_MM_BANK_COUNT];
	void*										m_pMemory;
	uint32									m_Size;

#ifdef CRY_MM_DEBUG
	uint32									m_InUse;
#endif

	uint32	AllocatePages(uint32 Count);
	void	Merge(CCryDXPSGCMMemItem*	pItem);
public:
	void Init(void* pMemory,uint32 Size);

	uint32 Allocate(uint32 Size);
	void Free(uint32 ID);

//	void* AllocateHuge(uint32 Size);
//	void FreeHuge(void* pMem);

	void* ResolveHandle(uint32 ID);
};

#endif

