/********************************************************************
CryGame Source File.
Copyright (C), Crytek Studios, 2001-2008.
-------------------------------------------------------------------------
File name:   ModManager.cpp
Version:     v1.00
Description: reading out mod information

-------------------------------------------------------------------------
History:
- 10:12:2007   12:20 : Created by Jan Müller

*********************************************************************/

#include "StdAfx.h"
#include "ModManager.h"
#include "IGameFramework.h"

bool CModManager::GetModInfo(SModInfo *info, const char *pModPath)
{
	if(!info)
		return false;

	if(GetModInfo(*info, pModPath))
		return true;
	return false;
}

bool CModManager::GetModInfo(SModInfo &info, const char *pModPath)
{
	string modPath(pModPath);
	string fullPath("");

	if(!modPath.size())
	{
		modPath = string(gEnv->pCryPak->GetModDir());
	}
	if(!modPath.size())
		return false;

	info.m_description = info.m_name = info.m_displayname = info.m_team = info.m_url = info.m_screenshot = info.m_version = NULL;

	int slashPos = modPath.rfind('\\', modPath.length()-2);
	if(slashPos == -1)
		slashPos = modPath.rfind('/', modPath.length()-2);

	string modName = modPath.substr(slashPos+1, modPath.length()-(slashPos+2));

	fullPath.append(modPath);
	fullPath.append("\\info.xml");
	XmlNodeRef modInfo = GetISystem()->LoadXmlFile(fullPath.c_str());
	if(modInfo)
	{
		bool foundName = false;
		for(int a = 0; a < modInfo->getNumAttributes(); ++a)
		{
			foundName = true;
			info.m_name = modName;
			const char *key, *value;
			if(modInfo->getAttributeByIndex(a, &key, &value))
			{
				if(!stricmp(key, "displayname"))
					info.m_displayname = value;
				else if(!stricmp(key, "team"))
					info.m_team = value;
				else if(!stricmp(key, "description"))
					info.m_description = value;
				else if(!stricmp(key, "version"))
					info.m_version = value;
				else if(!stricmp(key, "screenshot"))
					info.m_screenshot = value;
				else if(!stricmp(key, "url"))
					info.m_url = value;
			}
		}

		if(!foundName)
			return false;
		return true;
	}
	return false;
}
