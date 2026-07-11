/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	Input implementation for Xenon
-------------------------------------------------------------------------
History:
- Dec 05,2005:	Created by Marco Koegler

*************************************************************************/
#include "StdAfx.h"
#include "XenonInput.h"

#ifdef USE_XENONINPUT

#include <ILog.h>
#include <ISystem.h>
#include "XInputDevice.h"
#include "XInputKeyboard.h"

CXenonInput::CXenonInput(ISystem *pSystem) : CBaseInput()
{ 
	m_pSystem			= pSystem;
	m_pLog				=	pSystem->GetILog();
};

bool CXenonInput::Init()
{
	m_pLog->Log("Initializing XenonInput\n");
	if (!CBaseInput::Init()) return false;

	if (!AddInputDevice(new CXInputKeyboard(*this))) return false;

	// add xinput controllers devices
	for (int i= 0; i < 4; ++i)
	{
		if (!AddInputDevice(new CXInputDevice(*this, i))) return false;
	}
#ifdef DEBUG_KEYBOARD
  if (!AddInputDevice(new CXDebugKeyboard(*this)))
  {
    m_pLog->Log("Error: Initializing Xenon keyboard\n");
  }
#endif
	return true;	
}

///////////////////////////////////////////
void CXenonInput::ShutDown()
{
	m_pLog->Log("XenonInput Shutdown\n");
	CBaseInput::ShutDown();

	delete this;
}

#endif //USE_XENONINPUT
