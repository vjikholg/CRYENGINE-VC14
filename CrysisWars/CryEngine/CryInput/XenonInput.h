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

#ifndef __XENONINPUT_H__
#define __XENONINPUT_H__
#pragma once

#ifdef USE_XENONINPUT

//#define DEBUG_KEYBOARD 1

#include "BaseInput.h"

#ifdef DEBUG_KEYBOARD
#include "XDebugKeyboard.h"
#endif //DEBUG_KEYBOARD 

class CXenonInput : public CBaseInput
{
public:
	CXenonInput(ISystem *pSystem);

	// IInput overrides
	virtual bool	Init();
	virtual void	ShutDown();
	// ~IInput

private:
	// shortcuts
	ISystem*			m_pSystem;
	ILog*					m_pLog;
};

#endif //USE_XENONINPUT

#endif // __XENONINPUT_H__

