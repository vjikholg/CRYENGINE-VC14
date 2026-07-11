/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: TeamRush specific HUD code

-------------------------------------------------------------------------
History:
- 09/11/2008: Created by Jan Neugebauer

*************************************************************************/

#ifndef HUD_TEAMRUSH_H
#define HUD_TEAMRUSH_H

# pragma once


#include "HUDObject.h"

#include "HUD.h"

class CGameFlashAnimation;

class CHUDTeamRush : public CHUDObject
{
	friend class CHUD;
public:

	CHUDTeamRush(CHUD *pHUD);
	~CHUDTeamRush();

	void Update(float fDeltaTime);
	void Reset();
	void Show(bool show);
	void SetHUDColor();
	void UpdateStats();
	void TrySpawning();
	void SetTeam(int team);

	int m_ownTeamScore;
	int m_enemyTeamScore;
	int m_ownTeam;
	int m_spawnTeam;
	int m_ballState;
	bool m_clientBall;
	int m_ballConnected;

private:

	void PushToFlash();
	CGameFlashAnimation m_animTRScore;
	CHUD *g_pHUD;

	int m_roundTime;
	int m_scoreLimit;

};

#endif
