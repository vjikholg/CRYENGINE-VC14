/*************************************************************************
 Crytek Source File.
 Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
 $Id$
 $DateTime$
 Description:  the part of a defence wall that every connection needs
 -------------------------------------------------------------------------
 History:
 - 10/08/2004   : Created by Craig Tiller, based on CDefenceWall by Timur
*************************************************************************/

#ifndef __DEFENCECONTEXT_H__
#define __DEFENCECONTEXT_H__

#pragma once

#include "Config.h"
#include "Network.h"

#if USE_DEFENCE

#include "IDataProbe.h"
#include "DefenceData.h"

struct IDefenceContext : public INetMessageSink
{
	inline virtual ~IDefenceContext() {}
	virtual void AddProtectedFile( const CDefenceData::SProtectedFile& ) = 0;
	virtual void ClearProtectedFiles() = 0;
	virtual bool HasRemoteDef( const SNetMessageDef * pDef ) = 0;

	// true once there's no outstanding validation requests
	virtual bool CanRemove() = 0;
};

#endif //USE_DEFENCE

#endif
