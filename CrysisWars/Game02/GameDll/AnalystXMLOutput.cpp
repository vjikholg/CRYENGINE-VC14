/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:  Handles saving of game statistics to XML format files
-------------------------------------------------------------------------
History:
- 12/08/2008   : Steve Humphreys, Created
*************************************************************************/

#include <StdAfx.h>

#include "Analyst.h"

#ifdef WIN32
#include <windows.h>
#endif
#include "Game.h"
#include "GameRules.h"

static const string xmlHeader = "<?xml version=\"1.0\"?>\n<?mso-application progid=\"Excel.Sheet\"?>\n<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\nxmlns:o=\"urn:schemas-microsoft-com:office:office\"\nxmlns:x=\"urn:schemas-microsoft-com:office:excel\"\nxmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\n";
static const string xmlHeader2= "xmlns:html=\"http://www.w3.org/TR/REC-html40\">\n<ExcelWorkbook xmlns=\"urn:schemas-microsoft-com:office:excel\">\n</ExcelWorkbook>\n";
static const string xmlStyle  = "<Styles>\n<Style ss:ID=\"Default\" ss:Name=\"Normal\">\n</Style>\n<Style ss:ID=\"grey\">\n<Interior ss:Color=\"#C0C0C0\" ss:Pattern=\"Solid\"/>\n</Style>\n<Style ss:ID=\"bold\">\n<Font x:Family=\"Swiss\" ss:Bold=\"1\"/>\n</Style>\n</Styles>\n";

static const string xmlFooter = "</Workbook>";

//------------------------------------------------------------------------
void CGameplayAnalyst::DumpToXML()
{
	ICryPak* pCryPak = gEnv->pCryPak;
	FILE *pFile=0;

	string fname = m_gameanalysis.level;

	// Get time and version
	const SFileVersion& ver = gEnv->pSystem->GetProductVersion();
	time_t ltime;
	time( &ltime );
	tm *today = localtime( &ltime );
	string temp;
	temp.Format("(%d)_%02d_%02d_%02d%02d", ver.v[0], today->tm_mon+1, today->tm_mday, today->tm_hour, today->tm_min);
	fname += temp;

	fname += ".xml";
	fname.insert(0, "%USER%/Stats/");
	pFile = pCryPak->FOpen(fname.c_str(), "wb");
		
	if (!pFile)
		return;

	pCryPak->FWrite(xmlHeader.c_str(), xmlHeader.size(), 1, pFile);
	pCryPak->FWrite(xmlHeader2.c_str(), xmlHeader2.size(), 1, pFile);
	pCryPak->FWrite(xmlStyle.c_str(), xmlStyle.size(), 1, pFile);

	// first add up the weapon totals over all players
	Weapons weaponTotals;
	// loop through players
	for(Players::iterator it = m_gameanalysis.players.begin(); it != m_gameanalysis.players.end(); ++it)
	{
		// loop through weapons
		for(Weapons::iterator weaponIt = it->second.weapons.begin(); weaponIt != it->second.weapons.end(); ++weaponIt)
		{
			Weapons::iterator entry = weaponTotals.find(CONST_TEMP_STRING(weaponIt->first));
			if(entry == weaponTotals.end())
			{
				// if not yet seen, add new entry to weapon totals array
				weaponTotals.insert(Weapons::value_type(weaponIt->first, weaponIt->second));
			}
			else
			{
				// if seen already, increment values in the weapon totals array
				entry->second += weaponIt->second;
			}
		}
	}

	// now write the data to each individual worksheet
	WriteMainSheet(pFile);
	WriteScoresSheet(pFile);
	WritePlayerWeaponsSheet(pFile);
	WriteWeaponTotalsSheet(pFile, weaponTotals);
	WriteSuitSheet(pFile);
	if(m_gameanalysis.gameRules == "PowerStruggle")
		WritePSSheet(pFile);
	WriteLifetimesSheet(pFile);

	pCryPak->FWrite(xmlFooter.c_str(), xmlFooter.size(), 1, pFile);
	pCryPak->FClose(pFile);

	CryLog("Player statistics saved to %s...", fname.c_str());
}

// insert a text cell into the document on the current row
void WriteCell(FILE* pFile, const char* text, bool bold = false, bool grey = false)
{
	string cell;
	cell.Format("<Cell ss:StyleID=\"%s\"><Data ss:Type=\"String\">%s</Data></Cell>\n", bold ? "bold" : (grey ? "grey" : "Default"), text);
	gEnv->pCryPak->FWrite(cell.c_str(), cell.size(), 1, pFile);
}

// insert a number cell into the document on the current row
void WriteCell(FILE* pFile, int number, bool bold = false, bool grey = false)
{
	string cell;
	cell.Format("<Cell ss:StyleID=\"%s\"><Data ss:Type=\"Number\">%d</Data></Cell>\n", bold ? "bold" : (grey ? "grey" : "Default"), number);
	gEnv->pCryPak->FWrite(cell.c_str(), cell.size(), 1, pFile);
}

// insert a float cell into the document on the current row
void WriteCellF(FILE* pFile, float number, bool bold = false, bool grey = false)
{
	string cell;
	cell.Format("<Cell ss:StyleID=\"%s\"><Data ss:Type=\"Number\">%.2f</Data></Cell>\n", bold ? "bold" : (grey ? "grey" : "Default"), number);
	gEnv->pCryPak->FWrite(cell.c_str(), cell.size(), 1, pFile);
}

// handles creation and closing of a row
struct SXmlRow
{
	SXmlRow(FILE* pFile)
	{
		m_pFile = pFile;
		static string newRow = "<Row>\n";
		gEnv->pCryPak->FWrite(newRow.c_str(), newRow.size(), 1, pFile);
	}
	~SXmlRow()
	{
		static string endRow = "</Row>\n";
		assert(m_pFile);
		if(m_pFile)
			gEnv->pCryPak->FWrite(endRow.c_str(), endRow.size(), 1, m_pFile);
	}

	FILE* m_pFile;
};

// handles creation and closing of a worksheet
struct SXmlSheet
{
	SXmlSheet(FILE* pFile, const char* name, const char* title)
	{
		m_pFile = pFile;
		string start;
		start.Format("<Worksheet ss:Name=\"%s\">\n<Table>\n<Column ss:Width=\"100\"/>\n", name);

		gEnv->pCryPak->FWrite(start.c_str(), start.size(), 1, pFile);

		{
			SXmlRow row(pFile);
			WriteCell(pFile, title, true);
		}
		{	SXmlRow row(pFile);	}
	}

	~SXmlSheet()
	{
		assert(m_pFile);
		static string end = "</Table>\n</Worksheet>\n";
		if(m_pFile)
			gEnv->pCryPak->FWrite(end.c_str(), end.size(), 1, m_pFile);
	}

	FILE* m_pFile;
};

// Main sheet contains game status and results
void CGameplayAnalyst::WriteMainSheet(FILE* pFile)
{
	if(!pFile)
		return;

	SXmlSheet sheet(pFile, "Summary", "Game summary");

	{
		SXmlRow row(pFile);
		WriteCell(pFile, "Build number:", false, true);
		WriteCell(pFile, "");
		const SFileVersion& ver = gEnv->pSystem->GetProductVersion();
		WriteCell(pFile, ver.v[0]);
	}
	{
		SXmlRow row(pFile);
		WriteCell(pFile, "Game type:", false, true);
		WriteCell(pFile, "");
		WriteCell(pFile, m_gameanalysis.gameRules);
	}
	{
		SXmlRow row(pFile);

		WriteCell(pFile, "Map:", false, true);
		WriteCell(pFile, "");
		WriteCell(pFile, m_gameanalysis.level);
	}

	// current time
#ifdef WIN32
	{	SXmlRow row(pFile);	}
	{
		SXmlRow row(pFile);

		static char buffer[512];
		SYSTEMTIME st;
		GetSystemTime(&st);
		sprintf(buffer, "%.2d:%.2d:%.2d.%.3d UTC", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		WriteCell(pFile, "Game end time:", false, true);
		WriteCell(pFile, "");
		WriteCell(pFile, buffer);
	}
#endif

	// game length
	CTimeValue length = CTimeValue(gEnv->pTimer->GetCurrTime()) - m_gameanalysis.gameStartTime;
	{
		SXmlRow row(pFile);
		WriteCell(pFile, "Game length:", false, true);
		WriteCell(pFile, "");
		string time;
		time.Format("%dmin %2ds", int(length.GetSeconds()/60.0f), int(length.GetSeconds()) % 60);
		WriteCell(pFile, time.c_str());
	}

	{	SXmlRow row(pFile);	}

	bool teams = false;
	if(CGameRules* pGR = g_pGame->GetGameRules())
		teams = (g_pGame->GetGameRules()->GetTeamCount() == 2);

	// won by
	{
		SXmlRow row(pFile); 
		WriteCell(pFile, "Winner:", false, true);
		WriteCell(pFile, "");

		if(!teams)
		{
			// IA, so one player won the game
			if(m_gameanalysis.gameWinner)
				WriteCell(pFile, GetPlayer(m_gameanalysis.gameWinner).name);
			else
				WriteCell(pFile, "No winner");
		}
		else
		{
			int team = 0;
			if(g_pGame->GetGameRules())
				team = g_pGame->GetGameRules()->GetTeam(m_gameanalysis.gameWinner);
			if(m_gameanalysis.gameWinType == 1 && team != 0)	
			{
				// PS, hq destroyed
				string win;
				win.Format("%s (%s)", team == 1 ? "NK" : "US", GetPlayer(m_gameanalysis.gameWinner).name.c_str());
				WriteCell(pFile, win.c_str());
			}
			else if(team != 0)
			{
				// TIA or PS with a team winning
				WriteCell(pFile, team == 1 ? "NK" : "US");	
			}
			else
				WriteCell(pFile, "Unknown");
		}
	}

	// condition for winning
	{
		SXmlRow row(pFile); 
		WriteCell(pFile, "Win type:", false, true);
		WriteCell(pFile, "");
		if(m_gameanalysis.gameWinType == 1)
			WriteCell(pFile, "HQ Destroyed");
		else if(m_gameanalysis.gameWinType == 2)
			WriteCell(pFile, "Time limit");
		else if(m_gameanalysis.gameWinType == 3)
			WriteCell(pFile, "Score limit");
		else 
			WriteCell(pFile, "Not finished");
	}


	{	SXmlRow row(pFile);	}

	// current number of players
	{
		SXmlRow row(pFile);
		WriteCell(pFile, "Current players:", false, true);
		WriteCell(pFile, "");
		WriteCell(pFile, m_gameanalysis.currentPlayers);
	}

	// max players 
	{
		SXmlRow row(pFile);
		WriteCell(pFile, "Max players:", false, true);
		WriteCell(pFile, "");
		WriteCell(pFile, m_gameanalysis.maxPlayers);
	}

	// total players seen during this game
	{
		SXmlRow row(pFile);
		WriteCell(pFile, "Total players:", false, true);
		WriteCell(pFile, "");
		WriteCell(pFile, m_gameanalysis.players.size());
	}
}

// Scores sheet contains the current scoreboard. Tends to be gamemode specific.
void CGameplayAnalyst::WriteScoresSheet(FILE* pFile)
{
	if(!pFile)
		return;

	// start sheet
	SXmlSheet sheet(pFile, "Scoreboard", "Current scoreboard");

	CGameRules* pGR = g_pGame->GetGameRules();
	if(!pGR)
		return;

	bool teams = pGR->GetTeamCount() == 2;

	// scoreboard
	if(!teams)
	{
		// Instant Action.
		// easy: just playername / kills / deaths, sorted by kills 
		{
			SXmlRow row(pFile);
			WriteCell(pFile, "Player", false, true);
			WriteCell(pFile, "Kills", false, true);
			WriteCell(pFile, "Deaths", false, true);
		}

		std::vector<PlayerAnalysis> playerScores;
		playerScores.reserve(m_gameanalysis.players.size());
		for(Players::iterator it = m_gameanalysis.players.begin(); it != m_gameanalysis.players.end(); ++it)
		{
			playerScores.push_back(it->second);
		}
		std::sort(playerScores.begin(), playerScores.end());

		for(int i=0; i<playerScores.size(); ++i)
		{
			SXmlRow row(pFile);
			WriteCell(pFile, playerScores[i].name);
			WriteCell(pFile, playerScores[i].m_playerStats[eWS_Kills]);
			WriteCell(pFile, playerScores[i].m_playerStats[eWS_Deaths]);
		}
	}
	else
	{

		bool ps = false;
		if(!strcmp(pGR->GetEntity()->GetClass()->GetName(),"PowerStruggle"))
			ps = true;

		// first put players into two lists: NK and US
		std::vector<PlayerAnalysis> playerScores[3];
		for(Players::iterator it = m_gameanalysis.players.begin(); it != m_gameanalysis.players.end(); ++it)
		{
			playerScores[pGR->GetTeam(it->first)].push_back(it->second);
		}
		std::sort(playerScores[1].begin(), playerScores[1].end());
		std::sort(playerScores[2].begin(), playerScores[2].end());

		// for getting PP
		IScriptTable *pGameRulesScript=pGR->GetEntity()->GetScriptTable();
		int ppKey = 0;
		if(pGameRulesScript)
		{
			pGameRulesScript->GetValue("PP_AMOUNT_KEY", ppKey);
		}

		static const char* teamNames[] = { "Spectators / Not found", "NK", "US" };
		if(ps)
		{
			// PowerStruggle: name / rank / kills / deaths / PP
			for(int team = 0; team < 3; team++)
			{
				{
					SXmlRow row(pFile);
					WriteCell(pFile, teamNames[team], true);
				}
				{
					SXmlRow row(pFile);
					WriteCell(pFile, "Player", false, true);
					WriteCell(pFile, "Rank", false, true);
					WriteCell(pFile, "Kills", false, true);
					WriteCell(pFile, "Deaths", false, true);
					WriteCell(pFile, "PP", false, true);
				}
				for(int i=0; i<playerScores[team].size(); ++i)
				{
					SXmlRow row(pFile);
					WriteCell(pFile, playerScores[team][i].name);
					WriteCell(pFile, playerScores[team][i].m_playerStats[ePS_Rank]);
					WriteCell(pFile, playerScores[team][i].m_playerStats[eWS_Kills]);
					WriteCell(pFile, playerScores[team][i].m_playerStats[eWS_Deaths]);

					int playerPP=0;
					pGR->GetSynchedEntityValue(playerScores[team][i].entityId, TSynchedKey(ppKey), playerPP);
					WriteCell(pFile, playerPP);
				}

				// blank row
				{ SXmlRow row(pFile);	}
			}
		}
		else
		{
			// TIA: name / score / kills / deaths / teamkills
			for(int team = 0; team < 3; team++)
			{
				{
					SXmlRow row(pFile);
					WriteCell(pFile, teamNames[team], true);

					if(team != 0)
					{
						int score = 0;
						HSCRIPTFUNCTION pfnGetTeamScore = 0;
						if (pGameRulesScript->GetValue("GetTeamScore", pfnGetTeamScore) && pfnGetTeamScore)
						{
							Script::CallReturn(gEnv->pScriptSystem, pfnGetTeamScore, pGameRulesScript, team, score);
							gEnv->pScriptSystem->ReleaseFunc(pfnGetTeamScore);
						}
						WriteCell(pFile, score);
					}
				}
				{
					SXmlRow row(pFile);
					WriteCell(pFile, "Player", false, true);
					WriteCell(pFile, "Score", false, true);
					WriteCell(pFile, "Kills", false, true);
					WriteCell(pFile, "Deaths", false, true);
					WriteCell(pFile, "TeamKills", false, true);
				}
				for(int i=0; i<playerScores[team].size(); ++i)
				{
					SXmlRow row(pFile);
					WriteCell(pFile, playerScores[team][i].name);

					int score = 0;
					HSCRIPTFUNCTION pfnGetPlayerScore = 0;
					if (pGameRulesScript->GetValue("GetPlayerScore", pfnGetPlayerScore) && pfnGetPlayerScore)
					{
						Script::CallReturn(gEnv->pScriptSystem, pfnGetPlayerScore, pGameRulesScript, ScriptHandle(playerScores[team][i].entityId), score);
						gEnv->pScriptSystem->ReleaseFunc(pfnGetPlayerScore);
					}

					WriteCell(pFile, score);
					WriteCell(pFile, playerScores[team][i].m_playerStats[eWS_Kills]);
					WriteCell(pFile, playerScores[team][i].m_playerStats[eWS_Deaths]);
					WriteCell(pFile, playerScores[team][i].m_playerStats[eWS_TeamKills]);
				}

				// blank row
				{ SXmlRow row(pFile);	}
			}
		}
	}
}

// these should match the stats in the EWeaponStats enum, then calculated stats (eg accuracy) should follow
static const char* weaponHeader[] = {"", "Kills", "TK", "Suicide", "Deaths", "Shots", "ActorHits", "VehicleHits", "Other Hits", "Damage", "Usage", "Reloads", "MeleeAttacks", "FireModeChanges", "ZoomIn", "HitsSuffered", "DamageTaken", "KillsPerDeath","ShotsPerKill", "Accuracy"};

// weapons sheet contains stats for each player, and each weapon they used during the round
void CGameplayAnalyst::WritePlayerWeaponsSheet(FILE* pFile)
{
	if(!pFile)
		return;

	SXmlSheet sheet(pFile, "Players", "Player / Weapon summary");

	for(Players::iterator it=m_gameanalysis.players.begin(); it!=m_gameanalysis.players.end(); ++it)
	{
		PlayerAnalysis &player=GetPlayer(it->first);

		{
			SXmlRow row(pFile);
			int numColumns = sizeof(weaponHeader) / sizeof(const char*);
			for(int column = 0; column < numColumns; ++column)
			{
				// write column headers
				WriteCell(pFile, weaponHeader[column], false, true);
			}
		}

		// totals
		{
			SXmlRow row(pFile);

			int totalHits = player.m_playerStats[eWS_ActorHits] + player.m_playerStats[eWS_VehicleHits] + player.m_playerStats[eWS_OtherHits];
			WriteCell(pFile, player.name.c_str(), true);

			for(int stat = 0; stat < eWS_NumWeaponStats; ++stat)
				WriteCell(pFile, player.m_playerStats[stat]);

			// kills per death
			WriteCellF(pFile, player.m_playerStats[eWS_Deaths] ? player.m_playerStats[eWS_Kills]/(float)player.m_playerStats[eWS_Deaths]: 0);
			// shots per kill
			WriteCellF(pFile, player.m_playerStats[eWS_Kills] ? player.m_playerStats[eWS_Shots]/(float)player.m_playerStats[eWS_Kills] : 0);
			// accuracy
			WriteCellF(pFile, player.m_playerStats[eWS_Shots] ? totalHits/(float)player.m_playerStats[eWS_Shots] : 0);
		}

		// now each weapon results
		for(Weapons::iterator weaponIt = player.weapons.begin(); weaponIt != player.weapons.end(); ++weaponIt)
		{		
			WeaponAnalysis weapon = weaponIt->second;
			int totalHits = weapon.m_stats[eWS_ActorHits] + weapon.m_stats[eWS_VehicleHits] + weapon.m_stats[eWS_OtherHits];

			{
				SXmlRow row(pFile);
				WriteCell(pFile, weaponIt->first.c_str(), false, true);

				for(int stat = 0; stat < eWS_NumWeaponStats; ++stat)
					WriteCell(pFile, weapon.m_stats[stat]);

				// kills per death
				WriteCellF(pFile, weapon.m_stats[eWS_Deaths] ? weapon.m_stats[eWS_Kills]/(float)weapon.m_stats[eWS_Deaths]: 0);
				// shots per kill
				WriteCellF(pFile, weapon.m_stats[eWS_Kills] ? weapon.m_stats[eWS_Shots]/(float)weapon.m_stats[eWS_Kills] : 0);
				// accuracy
				WriteCellF(pFile, weapon.m_stats[eWS_Shots] ? totalHits/(float)weapon.m_stats[eWS_Shots] : 0);
			}
		}

		// blank rows between each player
		{ SXmlRow row(pFile); }
		{ SXmlRow	row(pFile);	}
	}
}


// Total stats for each weapon type, over all players
void CGameplayAnalyst::WriteWeaponTotalsSheet(FILE* pFile, Weapons& weaponTotals)
{
	if(!pFile)
		return;

	SXmlSheet sheet(pFile, "Weapon Totals", "Per weapon stats");

	// column titles
	{
		SXmlRow row(pFile);
		int numColumns = sizeof(weaponHeader) / sizeof(const char*);
		for(int column = 0; column < numColumns; ++column)
		{
			WriteCell(pFile, weaponHeader[column], false, true);
		}
	} 

	// now loop through weaponTotals and output each
	for(Weapons::iterator weaponIt = weaponTotals.begin(); weaponIt != weaponTotals.end(); ++weaponIt)
	{
		WeaponAnalysis weapon = weaponIt->second;
		int totalHits = weapon.m_stats[eWS_ActorHits] + weapon.m_stats[eWS_VehicleHits] + weapon.m_stats[eWS_OtherHits];

		{
			SXmlRow row(pFile);
			WriteCell(pFile, weaponIt->first.c_str(), false, true);

			for(int stat = 0; stat < eWS_NumWeaponStats; ++stat)
				WriteCell(pFile, weapon.m_stats[stat]);

			// kills per death
			WriteCellF(pFile, weapon.m_stats[eWS_Deaths] ? weapon.m_stats[eWS_Kills]/(float)weapon.m_stats[eWS_Deaths]: 0);
			// shots per kill
			WriteCellF(pFile, weapon.m_stats[eWS_Kills] ? weapon.m_stats[eWS_Shots]/(float)weapon.m_stats[eWS_Kills] : 0);
			// accuracy
			WriteCellF(pFile, weapon.m_stats[eWS_Shots] ? totalHits/(float)weapon.m_stats[eWS_Shots] : 0);
		}
	}
}

// Nanosuit sheet lists suit mode usage statistics
void CGameplayAnalyst::WriteSuitSheet(FILE* pFile)
{
	if(!pFile)
		return;

	SXmlSheet sheet(pFile, "Nanosuit", "Nanosuit usage summary");

	// column titles
	static const char* suitHeader[] = { "", "ModeChanges", "SpeedKills", "SpeedDeaths", "SpeedUsage", "SpeedTime", "StrengthKills", "StrengthDeaths", "StrengthUsage", "StrengthTime", "CloakKills", "CloakDeaths", "CloakUsage", "CloakTime", "ArmorKills", "ArmorDeaths", "ArmorUsage", "ArmorTime" };
	{
		SXmlRow row(pFile);
		int numColumns = sizeof(suitHeader) / sizeof(const char*);
		for(int column = 0; column < numColumns; ++column)
		{
			WriteCell(pFile, suitHeader[column], false, true);
		}
	}

	for (Players::iterator it=m_gameanalysis.players.begin(); it!=m_gameanalysis.players.end(); ++it)
	{
		SuitAnalysis &suit=it->second.suit;
		int totalUsage=suit.usage[0]+suit.usage[1]+suit.usage[2]+suit.usage[3];

		{
			SXmlRow row(pFile);

			// stats for this player
			WriteCell(pFile, it->second.name.c_str(), true);
			WriteCell(pFile, totalUsage);
			for(int i=0; i<4; ++i)
			{
				WriteCell(pFile, suit.kills[i]);
				WriteCell(pFile, suit.deaths[i]);
				WriteCell(pFile, suit.usage[i]);
				WriteCellF(pFile, suit.timeUsed[i].GetSeconds());
			}
		}
	}
}

// Powerstruggle specific stats - ranks mostly but could be extended to include PP, buying, etc
void CGameplayAnalyst::WritePSSheet(FILE* pFile)
{
	if(!pFile)
		return;

	// start sheet
	SXmlSheet sheet(pFile, "PS", "Power Struggle");

	// column titles
	const char* header[] = { "", "Highest Rank", "Promotions", "Demotions", "PVT Time", "CPL Time", "SGT Time", "LT Time", "CPT Time", "MAJ Time", "COL Time", "GEN Time"};
	int numColumns = sizeof(header) / sizeof(const char*);

	{
		SXmlRow row(pFile);
		for(int column = 0; column < numColumns; ++column)
		{
			WriteCell(pFile, header[column], false, true);
		}
	}

	for (Players::iterator it=m_gameanalysis.players.begin(); it!=m_gameanalysis.players.end(); ++it)
	{
		PlayerAnalysis &player=it->second;

		CTimeValue now=gEnv->pTimer->GetFrameStartTime();
		if (player.rankStart.GetMilliSeconds()==0)
			player.rankStart=now;
		player.rankTime[player.m_playerStats[ePS_Rank]]+=now-player.rankStart;
		player.rankStart=now;

		{
			SXmlRow row(pFile);
			WriteCell(pFile, player.name.c_str(), true);
			WriteCell(pFile, player.m_playerStats[ePS_MaxRank]);
			WriteCell(pFile, player.m_playerStats[ePS_Promotions]);
			WriteCell(pFile, player.m_playerStats[ePS_Demotions]);
			for(int i=0; i<8; ++i)
				WriteCellF(pFile, player.rankTime[i].GetSeconds());
		}
	}
}

// weapons sheet contains stats for each player, and each weapon they used during the round
void CGameplayAnalyst::WriteLifetimesSheet(FILE* pFile)
{
	if(!pFile)
		return;

	SXmlSheet sheet(pFile, "Lifetime", "Lifetime / interest ratio summary");

	for(Players::iterator it=m_gameanalysis.players.begin(); it!=m_gameanalysis.players.end(); ++it)
	{
		PlayerAnalysis &player=GetPlayer(it->first);

		static const char* lifeTimeHeader[] = {"Name", "Spawn-Action", "Action-Death", "Spawn-Death", "Ratio", "% Bored", "% Action"};
		{
			SXmlRow row(pFile);
			int numColumns = sizeof(lifeTimeHeader) / sizeof(const char*);
			for(int column = 0; column < numColumns; ++column)
			{
				// write column headers
				WriteCell(pFile, lifeTimeHeader[column], false, true);
			}
		}

		float totalStoA = 0.0f;
		float totalAtoD = 0.0f;
		float totalStoD = 0.0f;

		// now each lifetime results
		bool name = true;
		for(int life = 0; life < player.lifeTimes.size(); ++life)
		{	
			// only for complete lives (spawn, action and death times all set)
			if(player.lifeTimes[life].actionTime.GetSeconds() != 0.0f && player.lifeTimes[life].deathTime.GetSeconds() != 0.0f && player.lifeTimes[life].spawnTime.GetSeconds() != 0.0f)
			{
				SXmlRow row(pFile);
				if(name)
				{
					name = false;
					WriteCell(pFile, player.name.c_str(), true, false);
				}
				else
					WriteCell(pFile, "", false, false);

				float stoa = (player.lifeTimes[life].actionTime - player.lifeTimes[life].spawnTime).GetSeconds();
				float atod = (player.lifeTimes[life].deathTime - player.lifeTimes[life].actionTime).GetSeconds();
				float stod = (player.lifeTimes[life].deathTime - player.lifeTimes[life].spawnTime).GetSeconds();

				totalStoA += stoa;
				totalAtoD += atod;
				totalStoD += stod;

				WriteCellF(pFile, stoa > 0.0f ? stoa : 0.0f);
				WriteCellF(pFile, atod > 0.0f ? atod : 0.0f);
				WriteCellF(pFile, stod > 0.0f ? stod : 0.0f);
				WriteCellF(pFile, (stoa != 0.0f) ? atod / stoa : 0.0f);
				WriteCellF(pFile, (stod > 0.0f) ? 100.0f * stoa / stod : 0.0f);
				WriteCellF(pFile, (stod > 0.0f) ? 100.0f * atod / stod : 0.0f);
			}
		}

		{
			SXmlRow row(pFile);
			WriteCell(pFile, "Totals:", true);
			WriteCellF(pFile, totalStoA > 0.0f ? totalStoA : 0.0f);
			WriteCellF(pFile, totalAtoD > 0.0f ? totalAtoD : 0.0f);
			WriteCellF(pFile, totalStoD > 0.0f ? totalStoD : 0.0f);
			WriteCellF(pFile, (totalStoA != 0.0f) ? totalAtoD / totalStoA : 0.0f);
			WriteCellF(pFile, (totalStoD > 0.0f) ? 100.0f * totalStoA / totalStoD : 0.0f);
			WriteCellF(pFile, (totalStoD > 0.0f) ? 100.0f * totalAtoD / totalStoD : 0.0f);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//	Collating of stats into a totals file
//
///////////////////////////////////////////////////////////////////////////////////////////////

void CGameplayAnalyst::CollateTotals()
{
	Weapons weaponTotals;
	string totalsFile = "%USER%/StatsTotals/Totals.xml";

	// first read any existing totals file
	XmlNodeRef root = gEnv->pSystem->LoadXmlFile( totalsFile );
	if (!root)
	{
		CryLog( "Totals file not found: will be created");
	}
	else
	{
		// read initial state from totals file
		ReadWeaponStatsFromXml(root, weaponTotals);

		// move the totals file to 'Totals_Backup.xml'
#ifdef WIN32
		char origFile[256];
		char newFile[256];
		gEnv->pSystem->GetIPak()->AdjustFileName(totalsFile.c_str(), origFile, 0);
		gEnv->pSystem->GetIPak()->AdjustFileName("%USER%/StatsTotals/Totals_Backup.xml", newFile, 0);
		DeleteFile(newFile);
		CopyFile(origFile, newFile, false);
		DeleteFile(origFile);
#endif
	}

	// now iterate through all other xml files in the folder and attempt to load them
	// as stats files
	ICryPak *pPak = gEnv->pSystem->GetIPak();

	_finddata_t fd;
	intptr_t handle = 0;

	handle = pPak->FindFirst("%USER%/StatsTotals/*.xml", &fd);

	if (handle > -1)
	{
		do
		{
			// skip totals files
			if(!stricmp(fd.name, "Totals.xml") || !stricmp(fd.name, "Totals_Backup.xml"))
				continue;

			string xmlFile = string("%USER%/StatsTotals/") + fd.name;
			XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFile(xmlFile.c_str());
			if(rootNode)
			{
				CryLog("Processing stats file %s", fd.name);
				ReadWeaponStatsFromXml(rootNode, weaponTotals);
			}

		} while (pPak->FindNext(handle, &fd) >= 0);

		pPak->FindClose(handle);
	}

	// now write out the weapons totals again.
	ICryPak* pCryPak = gEnv->pCryPak;
	FILE *pFile=0;
	string fname = "%USER%/StatsTotals/Totals.xml";
	if (pFile = pCryPak->FOpen(fname.c_str(), "wb"))
	{
		pCryPak->FWrite(xmlHeader.c_str(), xmlHeader.size(), 1, pFile);
		pCryPak->FWrite(xmlHeader2.c_str(), xmlHeader2.size(), 1, pFile);
		pCryPak->FWrite(xmlStyle.c_str(), xmlStyle.size(), 1, pFile);

		CryLog("Saving new totals.xml file");
		WriteWeaponTotalsSheet(pFile, weaponTotals);

		pCryPak->FWrite(xmlFooter.c_str(), xmlFooter.size(), 1, pFile);
		pCryPak->FClose(pFile);
	}
}

void CGameplayAnalyst::ReadWeaponStatsFromXml(XmlNodeRef& xmlFile, Weapons& weapons)
{
	// read all relevent details from the 'weapon totals' page, add to the weapons map
	for(int i=0; i < xmlFile->getChildCount(); ++i)
	{
		XmlNodeRef child = xmlFile->getChild(i);
		string childName = child->getTag();
		if (childName.empty() || childName != "Worksheet")
			continue;

		XmlString workSheetName;
		child->getAttr("ss:Name",workSheetName);
		if(workSheetName != "Weapon Totals")
			continue;

		// now we have the worksheet containing the totals.
		XmlNodeRef nodeTable = child->findChild( "Table" );
		if (!nodeTable)
			return;

		// loop through all child nodes, if they are Rows then try to extract data.
		// NB:	node 0: <Column>
		//			node 1: <Row> with title
		//			node 2: blank <Row>
		//			node 3: <Row> of column titles
		for(int row = 4; row < nodeTable->getChildCount(); ++row)
		{
			WeaponAnalysis* pThisWeapon = NULL;

			XmlNodeRef nodeRow = nodeTable->getChild(row);
			if (!nodeRow || !nodeRow->isTag("Row"))
				continue;

			// loop through the cells on this row
			for(int cell = 0; cell < nodeRow->getChildCount(); ++cell)
			{
				XmlNodeRef nodeCell = nodeRow->getChild(cell);
				if(!nodeCell || !nodeCell->isTag("Cell"))
					continue;

				// get the data from this cell
				XmlNodeRef nodeCellData = nodeCell->findChild("Data");
				if (!nodeCellData)
					continue;

				const char *sCellContent = nodeCellData->getContent();

				string weaponName;
				switch(cell)
				{
					case 0:
					{
						// cell 0 contains the name. Try to find it in the weapon map, inserting if not found
						weaponName = sCellContent;
						std::pair<Weapons::iterator, bool> result=weapons.insert(Weapons::value_type(weaponName, WeaponAnalysis()));
						Weapons::iterator it = weapons.find(weaponName);
						pThisWeapon = &it->second;
						break;
					}

					case 17:
					case 18:
					case 19:
					{
						// these are compound stats and shouldn't be read: generated from new totals data
						break;
					}

					default:
					{
						// read numerical data and store in thisWeapon
						if(pThisWeapon && cell-1 < eWS_NumWeaponStats)
						{
							int data = atoi(sCellContent);
							pThisWeapon->m_stats[cell-1] += data;
						}
						break;
					}
				}
			}
		}

		// done
		break;
	}
}