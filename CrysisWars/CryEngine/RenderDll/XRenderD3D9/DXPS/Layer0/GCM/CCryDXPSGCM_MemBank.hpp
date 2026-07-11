#ifndef __CRYDXPSGCM_MEMBANK__
#define __CRYDXPSGCM_MEMBANK__

#include "CCryDXPSGCM_MemConfig.hpp"
#include "CCryDXPSGCM_MemItemList.hpp"

class CCryDXPSGCMMemBank
{
private:
	CCryDXPSGCMMemItemList	m_ItemList;
	uint16		m_StartPage;
	uint8			m_Valid:1;
public:
	inline CCryDXPSGCMMemBank():m_StartPage(~0),m_Valid(0){};
	inline void Init(uint32 Valid,uint32 StartPage){m_Valid=Valid;m_StartPage=StartPage;}
	inline uint32 Valid()const{return m_Valid;}
	inline void Validate(uint32 V){m_Valid=V?1:0;}

	uint16 StartPage()const{return m_StartPage;}
	uint32 Alloc(uint32 Count,CCryDXPSGCMMemItemList& rItemFreeList);
	uint32 UnUsed()const{return m_ItemList.Empty();}

};


#endif

