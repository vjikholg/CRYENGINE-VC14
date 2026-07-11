/********************************************************************
	Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  File name:   PipeManager.cpp
	$Id$
  Description: 
  
 -------------------------------------------------------------------------
  History:
  - 8:6:2005   14:17 : Created by Kirill Bulatsev

*********************************************************************/


#include "StdAfx.h"
#include <ISerialize.h>
#include "PipeManager.h"
#include "GoalPipe.h"

CPipeManager::CPipeManager(void):
m_bDynamicGoalPipes(false)
{
	CreateGoalPipe("_first_");
}

CPipeManager::~CPipeManager(void)
{
	ShutDown();
}



void CPipeManager::ShutDown()
{
	GoalMap::iterator gi;
	for (gi=m_mapGoals.begin();gi!=m_mapGoals.end();++gi)
		delete gi->second;

	m_mapGoals.clear();
}


//
//-----------------------------------------------------------------------------------------------------------
IGoalPipe *CPipeManager::CreateGoalPipe(const char *pName)
{
	if(!strcmp("_last_",pName))
	{
		m_bDynamicGoalPipes = true;
		return NULL;
	}

	// always create a new goal pipe
	CGoalPipe *pNewPipe = new CGoalPipe(pName, GetAISystem(), m_bDynamicGoalPipes);

	// try to insert the new element in the map
	std::pair<GoalMap::iterator,bool> insertResult = m_mapGoals.insert(GoalMap::iterator::value_type(pName,pNewPipe));

	// if it was there already...
	if ( !insertResult.second )
	{
		// ...destroy the old one...
		delete insertResult.first->second;
		// ...and assign the new one
		insertResult.first->second = pNewPipe;
	}

	return pNewPipe;
}

//
//-----------------------------------------------------------------------------------------------------------
IGoalPipe *CPipeManager::OpenGoalPipe(const char *pName)
{
  if (!pName)
    return 0;
	GoalMap::iterator gi = m_mapGoals.find(CONST_TEMP_STRING(pName));
	return gi != m_mapGoals.end() ? gi->second : NULL;
}

//
//-----------------------------------------------------------------------------------------------------------
CGoalPipe *CPipeManager::IsGoalPipe(const string &name)
{
	GoalMap::iterator gi = m_mapGoals.find(name);
	return gi != m_mapGoals.end() ? gi->second->Clone() : NULL;
}


//
//-----------------------------------------------------------------------------------------------------------
void CPipeManager::Serialize( TSerialize ser )
{
	int counter(0);
	char	nameBuffer[16];
	ser.BeginGroup("DynamicGoalPipes");
	{
		sprintf(nameBuffer,"DynPipe_%d",++counter);
		if(ser.IsWriting())
		{
			for (GoalMap::iterator gi(m_mapGoals.begin()); gi!=m_mapGoals.end(); ++gi)
			{
				if(!gi->second->IsDynamic())
					continue;
				ser.BeginOptionalGroup(nameBuffer, true);
				string tempstr = gi->first;
				ser.Value("PipeName", tempstr);
				gi->second->SerializeDynamic(ser);
				ser.EndGroup();
				sprintf(nameBuffer,"DynPipe_%d",++counter);
			}
			ser.BeginOptionalGroup(nameBuffer, false);
		}
		else
		{
			while(ser.BeginOptionalGroup(nameBuffer, true))
			{
				string name;
				ser.Value("PipeName", name);
				CGoalPipe *pNewPipe=static_cast<CGoalPipe*>(CreateGoalPipe( name ));
				pNewPipe->SerializeDynamic(ser);
				ser.EndGroup();
				sprintf(nameBuffer,"DynPipe_%d",++counter);
			}
		}
	}
	ser.EndGroup();
}

//
//-----------------------------------------------------------------------------------------------------------
