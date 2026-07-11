/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	Input implementation for PS
-------------------------------------------------------------------------
History:
- Dec 05,2005:	Created by Marco Koegler

*************************************************************************/
#include "StdAfx.h"
#include "PS3Input.h"

#ifdef USE_PS3INPUT

#include <IConsole.h>
#include <ILog.h>
#include <ISystem.h>

#include <cell/pad.h>      // USB Gamepad Library

#include "PS3Keyboard.h"
#include "PS3Pad.h"

CPS3Input::CPS3Input(ISystem *pSystem) : CBaseInput()
{ 
	m_pSystem			= pSystem;
	m_pLog				=	pSystem->GetILog();
	m_maxPads			= 1;
};

bool CPS3Input::Init()
{
	m_pLog->Log("Initializing PS3Input\n");
	if (!CBaseInput::Init())
	{
		m_pLog->Log("Error: CBaseInput::Init failed\n");
		return false;
	}

	// initialize the keyboard
	if (!AddInputDevice(new CPS3Keyboard(*this)))
	{
		m_pLog->Log("Error: Initializing PS3 keyboard\n");
		return false;
	}
	// init pad library
	const int RetInit	=	cellPadInit(m_maxPads);
	if ( RetInit != CELL_PAD_OK 
#if defined(CRY_USE_GCM_HUD)
		&& RetInit != CELL_PAD_ERROR_ALREADY_INITIALIZED
#endif
		)
	{
		m_pLog->Log("Error: Initializing Cell USB Pad library\n");
		return false;
	}

	// add pads
	for (int i=0; i < m_maxPads; ++i)
	{
		if (!AddInputDevice(new CPS3Pad(*this, i)))
		{
			m_pLog->Log("Error: Adding PS3 pad input device\n");
			return false;
		}
	}

	return true;	
}

///////////////////////////////////////////
void CPS3Input::ShutDown()
{
	m_pLog->Log("PS3Input Shutdown\n");
	CBaseInput::ShutDown();

	// terminate pad library
	cellPadEnd();

	delete this;
}

#endif //USE_PS3INPUT
