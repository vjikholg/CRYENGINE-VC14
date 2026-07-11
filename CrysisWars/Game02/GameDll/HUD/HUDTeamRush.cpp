/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: TeamRush specific HUD code

-------------------------------------------------------------------------
History:
- 09/11/2008 : Created by Jan Neugebauer

*************************************************************************/

#include "StdAfx.h"
#include "HUDTeamRush.h"

#include "HUD.h"
#include "GameFlashAnimation.h"

CHUDTeamRush::CHUDTeamRush(CHUD *pHUD) : 
g_pHUD(pHUD)
{
	m_animTRScore.Load("Libs/UI/HUD_TRScore.gfx", eFD_Center, eFAF_ManualRender|eFAF_Visible);
	m_animTRScore.SetVisible(false);
}

CHUDTeamRush::~CHUDTeamRush()
{
	m_animTRScore.Unload();
}

void CHUDTeamRush::Reset()
{
	m_spawnTeam = 0;
}

void CHUDTeamRush::SetHUDColor()
{
	g_pHUD->SetFlashColor(&m_animTRScore);
}

void CHUDTeamRush::TrySpawning()
{
	CActor *pActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
	CGameRules *pGameRules = g_pGame->GetGameRules();
	if(pActor && pGameRules && !g_pGame->GetGameRules()->IsPlayerActivelyPlaying(pActor->GetEntityId()) && m_spawnTeam!=0)
	{
		CryFixedStringT<64> cmd("team ");
		cmd.append(pGameRules->GetTeamName(m_spawnTeam));
		gEnv->pConsole->ExecuteString(cmd);

		if(m_spawnTeam==1)
			g_pHUD->DisplayGeneralMessage(eTextMessageTeam, "@ui_team_msg_join_0", "NK");
		else
			g_pHUD->DisplayGeneralMessage(eTextMessageTeam, "@ui_team_msg_join_0", "US");
	}
}

void CHUDTeamRush::SetTeam(int team)
{
	m_spawnTeam = team;
	g_pHUD->ShowLoadout(true);
}

void CHUDTeamRush::UpdateStats()
{
	CGameRules *pGameRules=g_pGame->GetGameRules();
	if(!pGameRules)
		return;

	IActor *pClientActor=g_pGame->GetIGameFramework()->GetClientActor();
	if(!pClientActor)
		return;

	IScriptTable *pGameRulesScript=pGameRules->GetEntity()->GetScriptTable();
	if(!pGameRulesScript)
		return;

	int ownTeam = pGameRules->GetTeam(pClientActor->GetEntityId());

	int ownTeamScore = 0;
	int enemyTeamScore = 0;
	int roundTime = 0;
	int firstTeam = 1;
	int scoreLimit = 100;
	int ballState = 0;
	int ballConnected = 0;
	bool clientball = false;

	if(ownTeam!=0) //no spectator
	{
		firstTeam = ownTeam;
	}
	for(int i=1; i<3; ++i)
	{
		HSCRIPTFUNCTION pfnGetScoreFlags=0;
		float score = 0.0f;
		if (pGameRulesScript->GetValue("GetTeamScore", pfnGetScoreFlags))
		{
			ScriptHandle actorId(pClientActor->GetEntityId());
			Script::CallReturn(gEnv->pScriptSystem, pfnGetScoreFlags, pGameRulesScript, i, score);
			gEnv->pScriptSystem->ReleaseFunc(pfnGetScoreFlags);
		}

		if(firstTeam==i)
		{
			ownTeamScore = static_cast<int>(floor(score+0.5f));
		}
		else
		{
			enemyTeamScore = static_cast<int>(floor(score+0.5f));
		}
	}

	HSCRIPTFUNCTION pfnGetScoreFlags=0;
	if (pGameRulesScript->GetValue("GetBallInGoal", pfnGetScoreFlags))
	{
		ScriptHandle actorId(pClientActor->GetEntityId());
		Script::CallReturn(gEnv->pScriptSystem, pfnGetScoreFlags, pGameRulesScript, ballConnected);
		gEnv->pScriptSystem->ReleaseFunc(pfnGetScoreFlags);
	}
	
	if(ballConnected!=0)
	{
		if(ballConnected==ownTeam)
			ballConnected = 1;
		else
			ballConnected = 2;
	}

	EntityId ball = pGameRules->GetBall();
	Vec3 ballPos; bool isCloaked;
	if(ball)
	{
		IItem* pBall(gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(ball));
		if(pBall)
		{
			EntityId ballOwnerId(pBall->GetOwnerId());
			ballState = pGameRules->GetBallState(ball, pClientActor->GetEntityId(), ballPos, isCloaked);
			clientball = ballOwnerId == pClientActor->GetEntityId();
		}
	}

	roundTime = static_cast<int>(floor(pGameRules->GetRemainingGameTime()));
	scoreLimit = g_pGameCVars->g_scorelimit;

	if(	ownTeamScore!=m_ownTeamScore ||
		enemyTeamScore!=m_enemyTeamScore ||
		roundTime!=m_roundTime ||
		scoreLimit!=m_scoreLimit ||
		ownTeam!=m_ownTeam ||
		ballState!=m_ballState ||
		clientball!=m_clientBall ||
		ballConnected!=m_ballConnected)
	{
		m_ownTeamScore = ownTeamScore;
		m_enemyTeamScore = enemyTeamScore;
		m_roundTime = roundTime;
		m_scoreLimit = scoreLimit;
		m_ownTeam = ownTeam;
		m_ballState = ballState;
		m_clientBall = clientball;
		m_ballConnected = ballConnected;
		PushToFlash();
	}


}

void CHUDTeamRush::Show(bool show)
{
	m_animTRScore.SetVisible(show);
}

void CHUDTeamRush::PushToFlash()
{
	SFlashVarValue args[7] = {m_scoreLimit, m_ownTeamScore, m_enemyTeamScore, m_roundTime, m_ownTeam, m_ballState, m_ballConnected};
	m_animTRScore.Invoke("setValues", args, 7);
	if(m_ownTeam==0)
		g_pHUD->SetTeamDisplay("", m_clientBall);
	else
		g_pHUD->SetTeamDisplay(m_ownTeam==1?"NK":"US", m_clientBall);
}

void CHUDTeamRush::Update(float fDeltaTime)
{
	if(!m_animTRScore.IsLoaded() || !m_animTRScore.GetVisible())
		return;

	UpdateStats();

	m_animTRScore.GetFlashPlayer()->Advance(fDeltaTime);
	m_animTRScore.GetFlashPlayer()->Render();
}