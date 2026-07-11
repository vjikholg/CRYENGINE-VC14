/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	Base class from which all PS3 devices should derive
							themselves. Allows access to the concrete IInput implementation
							used for this platform.
-------------------------------------------------------------------------
History:
- Dec 16,2005:	Created by Marco Koegler

*************************************************************************/
#ifndef __PS3INPUTDEVICE_H__
#define __PS3INPUTDEVICE_H__
#pragma once

#ifdef USE_PS3INPUT
#include "InputDevice.h"

class CPS3Input;

class CPS3InputDevice : public CInputDevice
{
public:
	CPS3InputDevice(CPS3Input& input, const char* deviceName);
	virtual ~CPS3InputDevice();

	//! return reference to the input interface which created the instance of this class
	CPS3Input& GetPS3Input() const;

private:
	CPS3Input&								m_ps3Input;	// pointer to the input interface, which created this device (initialized from constructor)
};

#endif //USE_PS3INPUT

#endif // __PS3INPUTDEVICE_H__
