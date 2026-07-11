/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
-------------------------------------------------------------------------
History:
- Dec 05,2005:	Created by Marco Koegler

*************************************************************************/
#include "StdAfx.h"
#include "XenonInputDevice.h"

#ifdef USE_XENONINPUT

CXenonInputDevice::CXenonInputDevice(CXenonInput& input, const char* deviceName) : CInputDevice((IInput&)input, deviceName),
m_xenonInput(input)
{
}

CXenonInputDevice::~CXenonInputDevice()
{
}

CXenonInput& CXenonInputDevice::GetXenonInput() const
{
	return m_xenonInput;
}

#endif //USE_XENONINPUT
