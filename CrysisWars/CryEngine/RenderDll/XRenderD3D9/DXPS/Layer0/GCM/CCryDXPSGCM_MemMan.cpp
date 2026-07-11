#include "RenderPCH.h"
#include "../CCryDXPS.hpp"
#include "../../CCryTypes.hpp"
#include "CCryDXPSGCM_MemMan.hpp"


void CCryDXPSGCMMemMan::Init(void* pMemory,uint32 Size)
{
#ifdef CRY_MM_DEBUG
CRY_DEBUGOUT("--== MEMORYMANAGER INIT ==--\n");
	m_InUse=0;
#endif
	m_ItemFreelist.InitAsFreelist();
/*
	for(uint32 a=0,d=0;a<Size;a++)
	{
		d=a^(a<<13)^(a>>2)^d;
		reinterpret_cast<uint8*>(pMemory)[a]=d^(d>>8)^(a>>16)^(a>>24);
	}
*/
	m_pMemory	=	pMemory;
	m_Size	=	Size;
	Size	>>=	CRY_MM_BANK_SHIFT;
	for(uint32 a=0;a<CRY_MM_BANK_COUNT;a++)
		m_Bank[a].Init(a<Size,a*CRY_MM_PAGEBANK_COUNT);
}

uint32 CCryDXPSGCMMemMan::AllocatePages(uint32 Count)
{
#ifdef CRY_MM_DEBUG
		CRY_DEBUGOUT("Allocating %d pages..",Count);
#endif

	for(uint32 a=0;a<CRY_MM_BANK_COUNT;a++)
		if(m_Bank[a].Valid())	//just if bank is avaible try to allocate
		{
			uint32 ID	=	m_Bank[a].Alloc(Count,m_ItemFreelist);
			if(ID)
			{
#ifdef CRY_MM_DEBUG
				m_InUse	+=	Count*CRY_MM_PAGE_SIZE;
				CRY_DEBUGOUT("successfully\n  %d%% of %dMB in use, Offset:0x%x\n",m_InUse/(m_Size/100),m_Size>>20,ID);
#endif
				return ID;
			}
		}
#ifdef CRY_MM_DEBUG
		CRY_DEBUGOUT("failed\n");
#endif
	return 0;
}

uint32 CCryDXPSGCMMemMan::Allocate(uint32 Size)
{
#ifdef CRY_MM_DEBUG
	CRY_DEBUGOUT("Allocation request for %d bytes, rounded to %d bytes or %d pages\n",
						Size,
						((Size+CRY_MM_PAGE_SIZE-1)>>CRY_MM_PAGE_SHIFT)<<CRY_MM_PAGE_SHIFT,
						(Size+CRY_MM_PAGE_SIZE-1)>>CRY_MM_PAGE_SHIFT);
#endif
	return AllocatePages((Size+CRY_MM_PAGE_SIZE-1)>>CRY_MM_PAGE_SHIFT);
}
/*
void*  CCryDXPSGCMMemMan::AllocateHuge(uint32 Size)
{
	//fits into one bank?
	if(Size<CRY_MM_BANK_SIZE)
	{
		uint32 Handle=Allocate(Size);
		return ResolveHandle(Handle);
	}

	const uint32 Pages=(Size+CRY_MM_PAGE_SIZE-1)>>CRY_MM_PAGE_SHIFT;
	const uint32 Banks=(Pages+CRY_MM_PAGEBANK_COUNT)/CRY_MM_PAGEBANK_COUNT;

	for(uint32 a=0;a<CRY_MM_BANK_COUNT-Banks;a++)
	{
		uint32 FreeBanks=0;
		for(uint32 b=0;b<Banks;b++)
			FreeBanks+=(m_Bank[a+b].Valid() && m_Bank[a+b].UnUsed());	//Count free Banks

		if(FreeBanks==Banks)
		{
			for(uint32 b=0;b<Banks-1;b++)
				m_Bank[a+b].Validate(0);

			uint32 ID	=	m_Bank[a+Banks].Alloc(Pages&(CRY_MM_PAGEBANK_COUNT-1),m_ItemFreelist);
			if(ID)
			{
#ifdef CRY_MM_DEBUG
				m_InUse	+=	Pages*CRY_MM_PAGE_SIZE;
				CRY_DEBUGOUT("successfully\n  %d%% of %dMB in use, Offset:0x%x\n",m_InUse/(m_Size/100),m_Size>>20,ID);
#endif
				return &reinterpret_cast<uint8*>(m_pMemory)[m_Bank[a].StartPage()*CRY_MM_PAGE_SIZE];
			}
			else
			{
				CRY_DEBUGOUT("Failed to allocate mem from VMem in a free bank, SHOULD NEVER HAPPEN ");
				abort();
				return 0;
			}
		}
	}

	CRY_DEBUGOUT("Failed to allocate mem from VMem, no block found with enough mem");
	abort();
	return 0;
}

void  CCryDXPSGCMMemMan::FreeHuge(void* pMem)
{
}
*/

void CCryDXPSGCMMemMan::Merge(CCryDXPSGCMMemItem*	pItem)
{
	if(!pItem->Next())
		return;

	CCryDXPSGCMMemItem*	pNext	=	pItem->Next();
	if(!pNext	|| !pNext->IsFree())
		return;

	CRY_MM_VALIDATE_ITEM(pItem);
	CRY_MM_VALIDATE_ITEM(pNext);

	//is not last item?
	if(pNext->Next())
	{
		//merge
		pItem->Count(pNext->Count()+pItem->Count());
		//unlink
		pItem->Next(pNext->Next());
		pItem->Next()->Prev(pItem);
		//add to freelist
		m_ItemFreelist.AddAsFirst(pNext);
		CRY_MM_VALIDATE_ITEM(pItem);
		CRY_MM_VALIDATE_ITEM(pNext);
	}
	else
	if(pItem->Prev())//next item is the last one, so maybe this item is not the first one?
	{
		//merge
		pNext->Count(pNext->Count()+pItem->Count());
		pNext->StartPage(pItem->StartPage());
		//unlink
		pNext->Prev(pItem->Prev());
		pNext->Prev()->Next(pNext);
		//add to freelist
		m_ItemFreelist.AddAsFirst(pItem);
		CRY_MM_VALIDATE_ITEM(pItem);
		CRY_MM_VALIDATE_ITEM(pNext);
	}
	else//worst case, both items are on ends of list
	{
		//merge
		pNext->Count(pNext->Count()+pItem->Count());
		pNext->StartPage(pItem->StartPage());
		//as we cannot unlink, just set the size to 0, next time the itemlist is used it will be freed
		pItem->Count(0);
		CRY_MM_VALIDATE_ITEM(pItem);
		CRY_MM_VALIDATE_ITEM(pNext);
	}
}

void CCryDXPSGCMMemMan::Free(uint32 ID)
{
	CCryDXPSGCMMemItem*	pItem	=	CCryDXPSGCMMemItem::ID2Pointer(ID);
#if defined(CRY_MM_DEBUG_ITEMLIST)
	if(ID==0 || pItem->IsFree() || pItem->Count()==0)
	{
		CRY_DEBUGOUT("Trying to free an already freed vmem-item\n");
		int a=1;
		while(a)
		{
		}
	}
	CCryDXPSGCMMemItem*	pEmptyItem	=	m_ItemFreelist.Find(ID);
	if(pEmptyItem)
	{
		CRY_DEBUGOUT("Trying to free an already freed vmem-item\n");
		int a=1;
		while(a)
		{
		}
	}
#endif

	pItem->Free();

#ifdef CRY_MM_DEBUG
	m_InUse	-=	pItem->Count()*CRY_MM_PAGE_SIZE;
	CRY_DEBUGOUT("%dByte freed  %d%% of %dMB in use, Offset:0x%x\n",pItem->Count()*CRY_MM_PAGE_SIZE,m_InUse/(m_Size/100),m_Size>>20,ID);
#endif
	//if next memblock is not allocated as well, merge blocks to lower the fragmentation
	CCryDXPSGCMMemItem*	pPrev	=	pItem->Prev();
	Merge(pItem);
	if(pPrev && pPrev->IsFree())
		Merge(pPrev);
}

void* CCryDXPSGCMMemMan::ResolveHandle(uint32 ID)
{
	return &reinterpret_cast<uint8*>(m_pMemory)[CCryDXPSGCMMemItem::ID2Pointer(ID)->StartPage()*CRY_MM_PAGE_SIZE];
}




