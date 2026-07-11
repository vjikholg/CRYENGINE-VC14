/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Spectator specific map code

-------------------------------------------------------------------------
History:
- 11/05/2008 : Created by Jan Neugebauer

*************************************************************************/

#include "StdAfx.h"
#include "HUDSpectatorMap.h"
#include "HUD.h"

static const int MAX_SPECTATORMAP_STATE = 3;

CHUDSpectatorMap::CHUDSpectatorMap(CHUD *pHUD) : 
g_pHUD(pHUD)
{
	m_animMap.Init("Libs/UI/HUD_SpectatorMap.gfx", eFD_Right, eFAF_ManualRender|eFAF_Visible);
	m_state = 0;
}

CHUDSpectatorMap::~CHUDSpectatorMap()
{
	if(m_animMap.IsLoaded())
		m_animMap.Unload();
}

void CHUDSpectatorMap::Reset()
{
}

void CHUDSpectatorMap::SetHUDColor()
{
	g_pHUD->SetFlashColor(&m_animMap);
}

void CHUDSpectatorMap::Toggle()
{
	m_state++;
	if(m_state>MAX_SPECTATORMAP_STATE)
		m_state = 0;
	m_animMap.Invoke("toggleState", m_state);
	m_animMap.GetFlashPlayer()->SetVariable("PlayerTeam",SFlashVarValue("KOREAN"));
}

void CHUDSpectatorMap::Close()
{
	m_state = 0;
	m_animMap.Invoke("toggleState",m_state);
}

void CHUDSpectatorMap::Show(bool show)
{
	bool isLoaded = m_animMap.IsLoaded();
	if(show && !isLoaded)
		m_animMap.Reload();
	else if(!show && isLoaded)
		m_animMap.Unload();
}

void CHUDSpectatorMap::Update(float fDeltaTime)
{
	if(!m_animMap.IsLoaded() || !m_animMap.GetVisible())
		return;

	m_animMap.GetFlashPlayer()->Advance(fDeltaTime);
	m_animMap.GetFlashPlayer()->Render();
}