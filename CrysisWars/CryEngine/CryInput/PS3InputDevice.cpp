/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
-------------------------------------------------------------------------
History:
- Dec 16,2005:	Created by Marco Koegler

*************************************************************************/
#include "StdAfx.h"
#include "PS3InputDevice.h"

#ifdef USE_PS3INPUT

CPS3InputDevice::CPS3InputDevice(CPS3Input& input, const char* deviceName) : CInputDevice((IInput&)input, deviceName),
m_ps3Input(input)
{
}

CPS3InputDevice::~CPS3InputDevice()
{
}

CPS3Input& CPS3InputDevice::GetPS3Input() const
{
	return m_ps3Input;
}

#endif //USE_PS3INPUT
