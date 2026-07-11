/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	JoyPad for PS3
-------------------------------------------------------------------------
History:
- Dec 18,2005:	Created by Marco Koegler

*************************************************************************/
#ifndef __PS3PAD_H__
#define __PS3PAD_H__
#pragma once

#ifdef USE_PS3INPUT
#include "PS3InputDevice.h"
#include <cell/pad.h>    // USB Keyboard Library

//this enables toggled keys that do not exists: toggled when threshold of x/y/z-rotation axis is crossed
#define USE_LEAN_HACK

class CPS3Input;

class CPS3Pad : public CPS3InputDevice
{
public:
	CPS3Pad(CPS3Input& input, int deviceNo);
	virtual ~CPS3Pad();

	// IInputDevice overrides
	virtual bool Init();
	virtual void Update();
	virtual bool IsOfDeviceType( EInputDeviceType type ) const { return type == eIDT_Gamepad; }
	// ~IInputDevice

private:
	bool	UpdateConnectionState(bool current);

	int							m_deviceNo; // device number (from 0 to CPS3Input::m_maxPads) for this device
	bool						m_connected;
	int							m_buttons[16];		//current 16 button values
	int							m_buttonsLast[16];//last updates 16 button values

	int							m_xAxis[3];	//x-axis values
	int							m_yAxis[3];	//y-axis values
	int							m_zAxis;		//z-axis values
	int							m_gyro;			//gyro values

#if defined(USE_LEAN_HACK)
	bool						m_rotXLActive;
	bool						m_rotXRActive;
	bool						m_rotZDActive;
	bool						m_rotZUActive;
#endif

	void MapDigDirControl(const CellPadData& crData);
	void ReadAxis(const CellPadData& crData);
};

#endif //USE_PS3INPUT

#endif //__PS3KEYBOARD_H__
