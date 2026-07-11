#ifndef __ANALYST_H__
#define __ANALYST_H__

#pragma once


#include "IGameplayRecorder.h"

// stats stored for each weapon
enum EWeaponStats
{
	eWS_Kills = 0,					// kills with this weapon against enemies
	eWS_TeamKills,					// kills against friends
	eWS_Suicides,						// self kills
	eWS_Deaths,							// deaths while carrying this weapon
	eWS_Shots,							// total number of shots fired with this weapon
	eWS_ActorHits,					// total number of hits *on an actor* with this weapon
	eWS_VehicleHits,				// total number of hits *on a vehicle* with this weapon
	eWS_OtherHits,					// total number of hits on other entities with this weapon
	eWS_Damage,							// total damage dealt with this weapon
	eWS_Usage,							// number of times this weapon was selected
	eWS_Reloads,						// total number of reloads with this weapon
	eWS_Melee,							// total number of melee attacks with this weapon
	eWS_FireModeChanges,		// total number of firemode changes
	eWS_ZoomUsage,					// number of zoom ins
	eWS_HitsSuffered,				// number of hits suffered when using this weapon
	eWS_DamageTaken,				// total damage taken when using this weapon
	
	eWS_NumWeaponStats,
};

// stats stored for each player. Note that numbers start from the end of the above, so 
//	players also have the above for totals
enum EPlayerStats
{
	ePS_Promotions = eWS_NumWeaponStats,			// number of promotions achieved
	ePS_Demotions,					// number of demotions suffered
	ePS_Rank,								// current rank
	ePS_MaxRank,						// max rank achieved

	ePS_NumPlayerStats,
};

class CGameplayAnalyst : public IGameplayListener
{
public:
	CGameplayAnalyst();
	virtual ~CGameplayAnalyst();

	// IGameplayListener
	virtual void OnGameplayEvent(IEntity *pEntity, const GameplayEvent &event);
	//~IGameplayListener

	void ProcessPlayerEvent(EntityId id, const GameplayEvent &event);

	// Feel free to add
	void GetMemoryStatistics(ICrySizer * s) {}

	typedef struct WeaponAnalysis
	{
		WeaponAnalysis() : timeUsed(0.0f), timeZoomed(0.0f)
		{
			for(int i=0; i<eWS_NumWeaponStats; ++i)
				m_stats[i] = 0;
		}

		CTimeValue	timeUsed;	// total time this weapon was used
		CTimeValue	timeZoomed; // total times zoomed with this weapon

		int m_stats[eWS_NumWeaponStats];

		const WeaponAnalysis& operator +=(const WeaponAnalysis &rhs)
		{
			timeUsed.SetSeconds(timeUsed.GetSeconds() + rhs.timeUsed.GetSeconds());
			timeZoomed.SetSeconds(timeZoomed.GetSeconds() + rhs.timeZoomed.GetSeconds());
			for(int i=0; i<eWS_NumWeaponStats; ++i)
				m_stats[i] += rhs.m_stats[i];

			return *this;
		}
	};

	typedef struct SuitAnalysis
	{
		SuitAnalysis() 
		: mode(3)
		{
			for (int i=0; i<4; i++)
			{
				timeUsed[i] = 0.0f;
				usage[i] = 0;
				kills[i] = 0;
				deaths[i] = 0;
			}
		}

		CTimeValue	usageStart;
		CTimeValue	timeUsed[4];	// total time using each suit mode
		int					usage[4];			// number of times this suit mode was selected

		int					kills[4];			// number of kills using each suit mode
		int					deaths[4];		// number of deaths using each suit mode
		int					mode;
	};

	typedef std::map<string, WeaponAnalysis> Weapons;

	typedef struct LifeTime
	{
		LifeTime()
		{
			spawnTime.SetSeconds(0.0f);
			actionTime.SetSeconds(0.0f);
			deathTime.SetSeconds(0.0f);
		}
		CTimeValue spawnTime;
		CTimeValue actionTime;
		CTimeValue deathTime;
	};

	typedef std::vector<LifeTime> Lives;

	typedef struct PlayerAnalysis
	{
		PlayerAnalysis()
		: rankStart(0.0f),
			deathStart(0.0f),
			timeDead(0.0f),
			timeAlive(0.0f),
			timeStart(0.0f),
			alive(false),
			entityId(0)
		{
			memset(rankTime, 0, sizeof(rankTime));
			for(int i=0; i<ePS_NumPlayerStats; ++i)
				m_playerStats[i] = 0;
		};

		bool operator < (const PlayerAnalysis &rhs) const;

		string			name;	// player name
		EntityId		entityId;
	
		CTimeValue	rankStart;
		CTimeValue	rankTime[10];// total time spent in each rank

		bool				alive;

		CTimeValue	deathStart;
		CTimeValue	timeDead;		// time dead / waiting to respawn
		CTimeValue	timeAlive;	// time alive
		CTimeValue	timeStart;	// total time played

		Weapons						weapons;	// weapon analysis
		SuitAnalysis			suit;
		int								m_playerStats[ePS_NumPlayerStats];
		Lives							lifeTimes;
	};

	typedef std::map<EntityId, PlayerAnalysis> Players;

	typedef struct GameAnalysis
	{
		GameAnalysis() : currentPlayers(0), maxPlayers(0), gameWinner(0), gameWinType(0)
		{
			gameStartTime.SetSeconds(0.0f);
		}

		Players	players;
		int currentPlayers;
		int maxPlayers;
		CTimeValue gameStartTime;
		string gameRules;
		string level;
		int gameWinner;	// entity id or team number
		int gameWinType;// 1=HQ destroyed, 2=time limit, 3=score limit
	};

	void Release() { delete this; };

	void Reset(bool addPlayers);
	
	void DumpToXML();							//	dump current stats into an xml file
	void CollateTotals();					//	make a Totals.xml file out of several individual stats xml files.

private:
	void NewPlayer(IEntity *pEntity);
	bool IsPlayer(EntityId entityId) const;
	bool IsWeapon(EntityId entityId) const;
	bool IsVehicle(EntityId entityId) const;
	
	PlayerAnalysis &GetPlayer(EntityId playerId);

	WeaponAnalysis &GetWeapon(EntityId playerId, EntityId weaponId);
	WeaponAnalysis &GetCurrentWeapon(EntityId playerId);

	// write out pages to an excel xml file. These functions are in AnalystXMLOutput.xml
	void WriteMainSheet(FILE* pFile);
	void WriteScoresSheet(FILE* pFile);
	void WritePlayerWeaponsSheet(FILE* pFile);
	void WriteWeaponTotalsSheet(FILE* pFile, Weapons& weaponTotals);
	void WriteSuitSheet(FILE* pFile);
	void WritePSSheet(FILE* pFile);
	void WriteLifetimesSheet(FILE* pFile);

	void ReadWeaponStatsFromXml(XmlNodeRef& xmlFile, Weapons& weapons);

	GameAnalysis m_gameanalysis;

	typedef std::map<string, string> TAmmoWeaponMap;			// ammo entity class name, weapon entity class name
	TAmmoWeaponMap m_ammoWeaponMap;
};
#endif
