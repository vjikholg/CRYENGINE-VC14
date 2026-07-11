#include "StdAfx.h"
#include "Analyst.h"

#include <ILevelSystem.h>
#include <IItemSystem.h>
#include <IVehicleSystem.h>
#include <IGameRulesSystem.h>
#include "Game.h"
#include "GameRules.h"
#include "GameCVars.h"

//------------------------------------------------------------------------
CGameplayAnalyst::CGameplayAnalyst()
{
	g_pGame->GetIGameFramework()->GetIGameplayRecorder()->RegisterListener(this);
}


//------------------------------------------------------------------------
CGameplayAnalyst::~CGameplayAnalyst()
{
	g_pGame->GetIGameFramework()->GetIGameplayRecorder()->UnregisterListener(this);
}

void CGameplayAnalyst::ProcessPlayerEvent(EntityId id, const GameplayEvent &event)
{
	if(!g_pGameCVars->g_gamePlayAnalyst)
		return;

	switch (event.event)
	{
	case eGE_Connected:
		{
			IEntity *pEntity=gEnv->pEntitySystem->GetEntity(id);
			NewPlayer(pEntity);

			m_gameanalysis.maxPlayers = max(++m_gameanalysis.currentPlayers, m_gameanalysis.maxPlayers);
		}
		break;
	case eGE_Disconnected:
		// don't need to do anything about the player itself, just the number of players.
		--m_gameanalysis.currentPlayers;
		break;
	case eGE_Renamed:
		GetPlayer(id).name=event.description;
		break;
	case eGE_Kill:
		{
			bool tk = false;
			CGameRules* pGR = g_pGame->GetGameRules();
			EntityId shooterId = 0;
			EntityId weaponId = 0;
			if(event.extra)
			{
				shooterId = ((EntityId*)event.extra)[0]; 
				weaponId = ((EntityId*)event.extra)[1];
			}
			if(pGR && pGR->GetTeamCount() > 1 && pGR->GetTeam(id) == pGR->GetTeam(shooterId))
				tk = true;

			PlayerAnalysis &player=GetPlayer(id);
			if(!tk)
			{
				player.m_playerStats[eWS_Kills]++;
				GetWeapon(id, weaponId).m_stats[eWS_Kills]++;
			}
			else
			{
				player.m_playerStats[eWS_TeamKills]++;
				GetWeapon(id, weaponId).m_stats[eWS_TeamKills]++;
			}

			player.suit.kills[player.suit.mode]++;
		}
		break;
	case eGE_Death:
		{
			PlayerAnalysis &player=GetPlayer(id);
			
			player.m_playerStats[eWS_Deaths]++;
			GetCurrentWeapon(id).m_stats[eWS_Deaths]++;

			// record suicides as well
			if(id == (EntityId)event.extra)
			{
				player.m_playerStats[eWS_Suicides]++;
				GetCurrentWeapon(id).m_stats[eWS_Suicides]++;
			}

			player.suit.deaths[player.suit.mode]++;

			CTimeValue now=gEnv->pTimer->GetFrameStartTime();

			if (player.deathStart.GetMilliSeconds()==0)
				player.deathStart=now;

			if (player.alive)
				player.timeAlive+=now-player.deathStart;

			player.deathStart=now;
			player.alive=false;

			int lives = player.lifeTimes.size();
			if(lives > 0)
			{
				assert(player.lifeTimes[lives-1].deathTime.GetSeconds() == 0.0f);
				player.lifeTimes[lives-1].deathTime = now;
			}
		}
		break;

	case eGE_Revive:
		{
			PlayerAnalysis &player=GetPlayer(id);
			CTimeValue now=gEnv->pTimer->GetFrameStartTime();

			if (player.deathStart.GetMilliSeconds()==0)
				player.deathStart=now;

			if (!player.alive)
				player.timeDead+=now-player.deathStart;
			player.deathStart=now;
			player.alive=true;
			
			LifeTime newLife;
			newLife.spawnTime = now;
			player.lifeTimes.push_back(newLife);
		}
		break;

	case eGE_WeaponHit:
	{
		// figure out what we hit: actor / vehicle / other
		EntityId hitEntityId = ((EntityId*)event.extra)[1];
		if(IsPlayer(hitEntityId))
		{
			PlayerAnalysis& shooter = GetPlayer(id);
			PlayerAnalysis& target = GetPlayer(hitEntityId);

			shooter.m_playerStats[eWS_ActorHits]++;
			GetWeapon(id, ((EntityId*)event.extra)[0]).m_stats[eWS_ActorHits]++;

			// also record that the other player was shot
			target.m_playerStats[eWS_HitsSuffered]++;

			// finally, if neither player is marked as 'in action' yet, do so.
			CTimeValue now = gEnv->pTimer->GetFrameStartTime();
			int lives = shooter.lifeTimes.size();
			if(lives > 0 && shooter.lifeTimes[lives-1].actionTime.GetSeconds() == 0.0f)
				shooter.lifeTimes[lives-1].actionTime = now;
			lives = target.lifeTimes.size();
			if(lives > 0 && target.lifeTimes[lives-1].actionTime.GetSeconds() == 0.0f)
				target.lifeTimes[lives-1].actionTime = now;
		}
		else if(IsVehicle(hitEntityId))
		{
			GetPlayer(id).m_playerStats[eWS_VehicleHits]++;
			GetWeapon(id, ((EntityId*)event.extra)[0]).m_stats[eWS_VehicleHits]++;
		}
		else
		{
			GetPlayer(id).m_playerStats[eWS_OtherHits]++;
			GetWeapon(id, ((EntityId*)event.extra)[0]).m_stats[eWS_OtherHits]++;
		}

		break;
	}
	case eGE_WeaponShot:
		GetPlayer(id).m_playerStats[eWS_Shots]+=(int)event.value;
		GetWeapon(id, (EntityId)event.extra).m_stats[eWS_Shots]+=(int)event.value;
		break;
	case eGE_WeaponMelee:
		GetPlayer(id).m_playerStats[eWS_Melee]++;
		GetWeapon(id, (EntityId)event.extra).m_stats[eWS_Melee]++;
		break;
	case eGE_WeaponReload:
		GetPlayer(id).m_playerStats[eWS_Reloads]++;
		GetWeapon(id, (EntityId)event.extra).m_stats[eWS_Reloads]++;
		break;
	case eGE_WeaponFireModeChanged:
		GetPlayer(id).m_playerStats[eWS_FireModeChanges]++;
		GetWeapon(id, (EntityId)event.extra).m_stats[eWS_FireModeChanges]++;
		break;
	case eGE_ZoomedIn:
		GetPlayer(id).m_playerStats[eWS_ZoomUsage]++;
		GetWeapon(id, (EntityId)event.extra).m_stats[eWS_ZoomUsage]++;
		break;

	case eGE_ItemSelected:
		GetPlayer(id).m_playerStats[eWS_Usage]++;
		GetWeapon(id, (EntityId)event.extra).m_stats[eWS_Usage]++;
		break;

	case eGE_Damage:
		GetPlayer(id).m_playerStats[eWS_Damage]+=(int)event.value;
		GetWeapon(id, ((EntityId*)event.extra)[0]).m_stats[eWS_Damage]+=(int)event.value;

		// also record damage to other players
		GetPlayer(((EntityId*)event.extra)[1]).m_playerStats[eWS_DamageTaken] += (int)event.value;
		break;

	case eGE_SuitModeChanged:
		{
			PlayerAnalysis &player=GetPlayer(id);
			SuitAnalysis &suit=player.suit;

			CTimeValue now=gEnv->pTimer->GetFrameStartTime();

			if (suit.usageStart.GetMilliSeconds()==0)
				suit.usageStart=player.timeStart;

			suit.timeUsed[suit.mode] += now-suit.usageStart;

			suit.mode=(int)event.value;
			suit.usage[(int)event.value]++;
			suit.usageStart=now;
		}
		break;

	case eGE_Rank:
		{
			PlayerAnalysis &player=GetPlayer(id);

			CTimeValue now=gEnv->pTimer->GetFrameStartTime();

			if (player.rankStart.GetMilliSeconds()==0)
				player.rankStart=player.timeStart;

			if (player.m_playerStats[ePS_MaxRank]<event.value)
				player.m_playerStats[ePS_MaxRank]=(int)event.value;

			if (player.m_playerStats[ePS_Rank]>event.value)
				++player.m_playerStats[ePS_Demotions];
			if (player.m_playerStats[ePS_Rank]<event.value)
				++player.m_playerStats[ePS_Promotions];

			player.rankTime[player.m_playerStats[ePS_Rank]] += now-player.rankStart;
			player.m_playerStats[ePS_Rank]=(int)event.value;
			player.rankStart=now;
		}
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
void CGameplayAnalyst::OnGameplayEvent(IEntity *pEntity, const GameplayEvent &event)
{
	if(!gEnv->bServer)
		return;

	if(!g_pGameCVars->g_gamePlayAnalyst)
		return;

	EntityId id=pEntity?pEntity->GetId():0;

	if (id && IsPlayer(id))
		ProcessPlayerEvent(id, event);

	switch (event.event)
	{
	case eGE_GameStarted:
		break;
	case eGE_GameReset:
		Reset(false);
		break;
	case eGE_GameEnd:
		if(event.value)//only for server event
			DumpToXML();
		break;
	case eGE_GameWon:
		// event.value contains either the player who won (IA, PS by HQ destroyed) or the team (TIA, PS by energy).
		// event.extra contains the win condition
		m_gameanalysis.gameWinner = (int)event.value;
		m_gameanalysis.gameWinType = (int)event.extra;
		break;
	case eGE_RegisterWeapon:
	{
		// event contains a weapon name : ammo name pair. Store these for later
		//	as many hit events come through with the ammo id as the 'weapon' - using this mapping
		//	we can figure out what weapon type fired the ammo.
		//	NB some weapons may share ammo types.
		bool found = false;
		TAmmoWeaponMap::iterator it = m_ammoWeaponMap.find(event.description);
		if(it == m_ammoWeaponMap.end())
		{
			m_ammoWeaponMap.insert(TAmmoWeaponMap::value_type((const char*)event.extra, event.description));
		}
		break;
	}
	default:
		break;
	}
}

//------------------------------------------------------------------------
void CGameplayAnalyst::NewPlayer(IEntity *pEntity)
{
	if (!pEntity)
		return;

	std::pair<Players::iterator, bool> result=m_gameanalysis.players.insert(Players::value_type(pEntity->GetId(), PlayerAnalysis()));
	result.first->second.name=pEntity->GetName();
	result.first->second.entityId = pEntity->GetId();
	result.first->second.timeStart=gEnv->pTimer->GetFrameStartTime();
}

//------------------------------------------------------------------------
bool CGameplayAnalyst::IsPlayer(EntityId entityId) const
{
	if (IActor *pActor= g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId))
		return pActor->IsPlayer();
	return false;
}

//------------------------------------------------------------------------
bool CGameplayAnalyst::IsWeapon(EntityId entityId) const
{
	if(IItem* pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(entityId))
		return (pItem->GetIWeapon() != NULL);
	return false;
}

//------------------------------------------------------------------------
bool CGameplayAnalyst::IsVehicle(EntityId entityId) const
{
	return (g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(entityId) != NULL);
}

//------------------------------------------------------------------------
CGameplayAnalyst::PlayerAnalysis &CGameplayAnalyst::GetPlayer(EntityId playerId)
{
	Players::iterator it=m_gameanalysis.players.find(playerId);
	if (it!=m_gameanalysis.players.end())
		return it->second;

	static PlayerAnalysis def;
	return def;
}

//------------------------------------------------------------------------
CGameplayAnalyst::WeaponAnalysis &CGameplayAnalyst::GetWeapon(EntityId playerId, EntityId weaponId)
{
	IEntity *pEntity=gEnv->pEntitySystem->GetEntity(weaponId);
	if(!pEntity)
	{
		static WeaponAnalysis def;
		return def;
	}

	PlayerAnalysis &player=GetPlayer(playerId);

	CryFixedStringT<32> weaponName = (pEntity->GetClass()->GetName());
	if(!IsWeapon(weaponId))
	{
		if(IsVehicle(weaponId))
			weaponName = "Vehicle";	// eg run over
		else
		{
			TAmmoWeaponMap::iterator it = m_ammoWeaponMap.find(weaponName.c_str());
			if(it != m_ammoWeaponMap.end())
			{
				weaponName = it->second.c_str();
			}
			else
			{
				CryLog("Unexpected weapon type for gameplay stats: %s", weaponName.c_str());
				weaponName = "Other";
			}
		}
	}

	Weapons::iterator it=player.weapons.find(CONST_TEMP_STRING(weaponName));
	if (it==player.weapons.end())
	{
		std::pair<Weapons::iterator, bool> result=player.weapons.insert(Weapons::value_type(weaponName, WeaponAnalysis()));
		return result.first->second;
	}
	else
		return it->second;
}

//------------------------------------------------------------------------
CGameplayAnalyst::WeaponAnalysis &CGameplayAnalyst::GetCurrentWeapon(EntityId playerId)
{
	if (IActor *pActor=g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId))
	{
		if (IInventory *pInventory = pActor->GetInventory())
			return GetWeapon(playerId, pInventory->GetCurrentItem());
	}

	static WeaponAnalysis def;
	return def;
}

//------------------------------------------------------------------------
void CGameplayAnalyst::Reset(bool addPlayers)
{
	m_gameanalysis=GameAnalysis();

	if(addPlayers)
	{
		IActorSystem *pActorSystem=g_pGame->GetIGameFramework()->GetIActorSystem();
		IActorIteratorPtr pActorIt=pActorSystem->CreateActorIterator();
		while (IActor *pActor = pActorIt->Next())
		{
			if (pActor && pActor->GetChannelId())
				NewPlayer(pActor->GetEntity());
		}
	}

	m_gameanalysis.gameStartTime = gEnv->pTimer->GetCurrTime();

	if(ILevel* pLevel = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel())
		m_gameanalysis.level = pLevel->GetLevelInfo()->GetDisplayName();
	if(IEntity* pGR = g_pGame->GetIGameFramework()->GetIGameRulesSystem()->GetCurrentGameRulesEntity())
		m_gameanalysis.gameRules = pGR->GetClass()->GetName();

	CryLog("Player statistics reset...");
}

bool CGameplayAnalyst::PlayerAnalysis::operator < (const PlayerAnalysis &rhs) const
{
	bool ps = false;
	if(CGameRules* pGR = g_pGame->GetGameRules())
	{
		if(!strcmp(pGR->GetEntity()->GetClass()->GetName(),"PowerStruggle"))
			ps = true;
	}

	if(!ps)
	{
		// simplest: IA + TIA
		if(rhs.m_playerStats[eWS_Kills] < m_playerStats[eWS_Kills])
			return true;
	}
	else
	{
		// ps
		if(rhs.m_playerStats[ePS_Rank] < m_playerStats[ePS_Rank])
			return true;

		if(rhs.m_playerStats[eWS_Kills] < m_playerStats[eWS_Kills])
			return true;
	}

	return false;
};