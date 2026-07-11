/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	Base class from which all Xenon devices should derive
							themselves. Allows access to the concrete IInput implementation
							used for this platform.
-------------------------------------------------------------------------
History:
- Dec 05,2005:	Created by Marco Koegler

*************************************************************************/
#ifndef __XENONINPUTDEVICE_H__
#define __XENONINPUTDEVICE_H__
#pragma once

#ifdef USE_XENONINPUT
#include "InputDevice.h"

class CXenonInput;

class CXenonInputDevice : public CInputDevice
{
public:
	CXenonInputDevice(CXenonInput& input, const char* deviceName);
	virtual ~CXenonInputDevice();

	//! return reference to the input interface which created the instance of this class
	CXenonInput& GetXenonInput() const;

private:
	CXenonInput&		m_xenonInput;	// pointer to the input interface, which created this device (initialized from constructor)
};

#endif //USE_XENONINPUT

#endif // __XENONINPUTDEVICE_H__
