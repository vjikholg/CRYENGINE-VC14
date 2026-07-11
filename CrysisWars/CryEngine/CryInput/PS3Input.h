/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	Input implementation for PS3
-------------------------------------------------------------------------
History:
- Dec 05,2005:	Created by Marco Koegler

*************************************************************************/
#ifndef __PS3INPUT_H__
#define __PS3INPUT_H__
#pragma once

#ifdef USE_PS3INPUT

#include "BaseInput.h"

struct	ILog;

class CPS3Input : public CBaseInput
{
public:
	CPS3Input(ISystem *pSystem); 

	// IInput
	// listener functions are implemented in CBaseInput
	virtual bool	Init();
	virtual void	ShutDown();
	// ~IInput

private:
	// shortcuts
	ISystem*			m_pSystem;
	ILog*					m_pLog;
	int						m_maxPads;
};

#endif //USE_PS3INPUT

#endif //__PS3INPUT_H__
