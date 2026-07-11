/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: TeamInstantAction specific HUD object

-------------------------------------------------------------------------
History:
- 03/18/2008: Created by Jan Neugebauer

*************************************************************************/

#ifndef HUD_SPECTATORMAP_H
#define HUD_SPECTATORMAP_H

# pragma once


#include "HUDObject.h"

#include "HUD.h"

class CGameFlashAnimation;

class CHUDSpectatorMap : public CHUDObject
{
	friend class CHUD;
public:

	CHUDSpectatorMap(CHUD *pHUD);
	~CHUDSpectatorMap();

	void Update(float fDeltaTime);
	void Reset();
	void Show(bool show);
	void SetHUDColor();
	void Toggle();
	void Close();
	ILINE CGameFlashAnimation* GetFlashAnim() { return &m_animMap; }
	int GetState() const { return m_state; }

private:

	CGameFlashAnimation m_animMap;
	int m_state;
	CHUD *g_pHUD;

};

#endif
