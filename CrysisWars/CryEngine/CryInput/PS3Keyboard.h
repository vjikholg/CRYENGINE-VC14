/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	Keyboard for PS3
-------------------------------------------------------------------------
History:
- Dec 16,2005:	Created by Marco Koegler

*************************************************************************/
#ifndef __PS3KEYBOARD_H__
#define __PS3KEYBOARD_H__
#pragma once

#ifdef USE_PS3INPUT
#include "PS3InputDevice.h"

class CPS3Input;

class CPS3Keyboard : public CPS3InputDevice
{
public:
	CPS3Keyboard(CPS3Input& input);
	virtual ~CPS3Keyboard();

	// IInputDevice overrides
	virtual bool Init();
	virtual void Update();
	virtual const char *GetKeyName(const SInputEvent& event, bool bGUI);
	virtual bool IsOfDeviceType( EInputDeviceType type ) const { return m_connected && type == eIDT_Keyboard; }
	// ~IInputDevice

private:
	unsigned char Event2ASCII(const SInputEvent& event);
	void	ProcessKey(uint32 key, bool pressed);
	bool	UpdateConnectionState(bool current);
	void	SetupKeyNames();
	int GetEventModifiers();

	bool					m_connected;
	unsigned int	m_modifiers;					// state of the modifiers
};

#endif //USE_PS3INPUT

#endif //__PS3KEYBOARD_H__
