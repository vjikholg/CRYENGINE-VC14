#include "RenderPCH.h"
#include "../../CCryTypes.hpp"
#include "CCryDXPSGCM_MemBank.hpp"
#include <stdio.h>
#include "../CCryDXPS.hpp"

uint32 CCryDXPSGCMMemBank::Alloc(uint32 Count,CCryDXPSGCMMemItemList& rItemFreeList)
{
	if(Count>CRY_MM_PAGEBANK_COUNT)
	{
		CRY_DEBUGOUT("Page request exceeds max number of pages per Bank\n");
		return 0;
	}

	if(!m_Valid)
	{
		CRY_DEBUGOUT("!!!Tried to allocate pages in Invalid Bank!!!\n");
		return 0;
	}

//	CRY_DEBUGOUT("\nseeking best fitting freeItem\n");
	//if first time usage, initialise big free block
	if(m_ItemList.Empty())
	{
		CCryDXPSGCMMemItem* pItem	=	rItemFreeList.Pop();
		if(!pItem)
		{
			CRY_DEBUGOUT("Could not get Item from Freelist for this Bank\n");
			return 0;
		}
		m_ItemList.AddAsFirst(pItem);
		pItem->Count(CRY_MM_PAGEBANK_COUNT);
		if(pItem->Count()!=CRY_MM_PAGEBANK_COUNT)
		{
			CRY_DEBUGOUT("pItem->Count()==%d CRY_MM_PAGEBANK_COUNT==%d\n",pItem->Count(),CRY_MM_PAGEBANK_COUNT);
		}
		pItem->StartPage(m_StartPage);
		pItem->Free();
	}

	//garbage collect items that could not be unlinked
	if(m_ItemList.First()->IsFree() && m_ItemList.First()->Count()==0)
		rItemFreeList.AddAsFirst(m_ItemList.Pop());

	//seek smallest block with enough pages
	CCryDXPSGCMMemItem* pBestItem=0;
	for(CCryDXPSGCMMemItem* pItem=m_ItemList.First();pItem;pItem=pItem->Next())
	{
//		CRY_DEBUGOUT("Item: Free:%s PageCount:%d\n",pItem->IsFree()?"Yes":"No",pItem->Count());
		//kinda slow but helps reduce memory fragmentation
		if(pItem->IsFree() && pItem->Count()>=Count && (!pBestItem || pItem->Count()<pBestItem->Count()))
			pBestItem	=	pItem;
	}

	if(!pBestItem)
	{
//		CRY_DEBUGOUT("no block with enough space found\n");
		return 0;
	}
			

	CCryDXPSGCMMemItem* pItem	= rItemFreeList.Pop();
	if(!pItem)
	{
		CRY_DEBUGOUT("Could not get Item from Freelist for this Bank(2)\n");//should never ever happen, impossible
		return 0;	//no block left
	}

	//split space for ne allocation
	pItem->InUse();
	m_ItemList.AddBefore(pItem,pBestItem);
	pItem->Count(Count);
	pBestItem->Count(pBestItem->Count()-Count);
	pItem->StartPage(pBestItem->StartPage());
	pBestItem->StartPage(pBestItem->StartPage()+Count);

	return pItem->ID();
}




