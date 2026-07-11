////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animcameranode.cpp
//  Version:     v1.00
//  Created:     16/8/2002 by Lennert.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AnimCameraNode.h"
#include "SelectTrack.h"

#include <ISystem.h>
#include <I3DEngine.h>
#include <IEntitySystem.h>

enum {
  APARAM_FOCUS_ENTITY = APARAM_USER,
};

#define s_nodeParamsInitialized s_nodeParamsInitializedCam
#define s_nodeParams s_nodeParamsCam
#define AddSupportedParam AddSupportedParamCam

//////////////////////////////////////////////////////////////////////////
namespace
{
	bool s_nodeParamsInitialized = false;
	std::vector<IAnimNode::SParamInfo> s_nodeParams;

	void AddSupportedParam( const char *sName,int paramId,EAnimValue valueType )
	{
		IAnimNode::SParamInfo param;
		param.name = sName;
		param.paramId = paramId;
		param.valueType = valueType;
		s_nodeParams.push_back( param );
	}
};

CAnimCameraNode::CAnimCameraNode( IMovieSystem *sys )
: CAnimEntityNode(sys)
{
	m_dwSupportedTracks = PARAM_BIT(APARAM_POS)|PARAM_BIT(APARAM_ROT)|
												PARAM_BIT(APARAM_EVENT)|PARAM_BIT(APARAM_FOV);
	m_pMovie=sys;
	m_fFOV = 60.0f;
  m_lastFocusKey = -1;

	if (!s_nodeParamsInitialized)
	{
		s_nodeParamsInitialized = true;
		AddSupportedParam( "Position",APARAM_POS,AVALUE_VECTOR );
		AddSupportedParam( "Rotation",APARAM_ROT,AVALUE_QUAT );
		AddSupportedParam( "Fov",APARAM_FOV,AVALUE_FLOAT );
		AddSupportedParam( "Event",APARAM_EVENT,AVALUE_EVENT );
    AddSupportedParam( "FocusEntity",APARAM_FOCUS_ENTITY,AVALUE_SELECT );
	}
}

//////////////////////////////////////////////////////////////////////////
CAnimCameraNode::~CAnimCameraNode()
{
}

//////////////////////////////////////////////////////////////////////////
int CAnimCameraNode::GetParamCount() const
{
	return s_nodeParams.size();
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCameraNode::GetParamInfo( int nIndex, SParamInfo &info ) const
{
	if (nIndex >= 0 && nIndex < (int)s_nodeParams.size())
	{
		info = s_nodeParams[nIndex];
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCameraNode::GetParamInfoFromId( int paramId, SParamInfo &info ) const
{
	for (unsigned int i = 0; i < s_nodeParams.size(); i++)
	{
		if (s_nodeParams[i].paramId == paramId)
		{
			info = s_nodeParams[i];
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CAnimCameraNode::CreateDefaultTracks()
{
	CreateTrack(APARAM_POS);
	CreateTrack(APARAM_ROT);
	CreateTrack(APARAM_FOV);
};

//////////////////////////////////////////////////////////////////////////
void CAnimCameraNode::Animate( SAnimContext &ec )
{
	CAnimEntityNode::Animate(ec);
	IAnimBlock *anim = GetAnimBlock();
	if (!anim)
		return;
	IAnimTrack *pFOVTrack = anim->GetTrack(APARAM_FOV);
	
	float fov = m_fFOV;
	
	if (pFOVTrack)
		pFOVTrack->GetValue(ec.time, fov);

	// is this camera active ? if so, set the current fov
	if (m_pMovie->GetCameraParams().cameraEntityId == m_EntityId)
	{
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity(m_EntityId);
		if (pEntity)
		{
			pEntity->SetFlags(pEntity->GetFlags() | ENTITY_FLAG_TRIGGER_AREAS);
			//pEntity->AddFlags(ENTITY_FLAG_TRIGGER_AREAS);
		}

		SCameraParams CamParams = m_pMovie->GetCameraParams();
		CamParams.fFOV = DEG2RAD(fov);
		m_pMovie->SetCameraParams(CamParams);
	}
	else
	{
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity(m_EntityId);
		if (pEntity)
		{
			pEntity->ClearFlags(ENTITY_FLAG_TRIGGER_AREAS);
		}

	}

  CSelectTrack *pFocusTrack = (CSelectTrack*)anim->GetTrack(APARAM_FOCUS_ENTITY);
  if (pFocusTrack)
  {
    ISelectKey key;
    int nkey = pFocusTrack->GetActiveKey(ec.time,&key);
    if (nkey != m_lastFocusKey)
    {
      if (!ec.bSingleFrame || key.time == ec.time) // If Single frame update key time must match current time.
        ApplyFocusKey( key,ec );
    }
    m_lastFocusKey = nkey;
  }

  if (m_focusEntityId)
  {
    IEntity *pEntity = gEnv->pEntitySystem->GetEntity(m_focusEntityId);
    if (pEntity)
    {
      Vec3 pos = pEntity->GetPos();
			assert("!GetI3DEngine()->SetCameraFocus() is not supported");
//      GetMovieSystem()->GetSystem()->GetI3DEngine()->SetCameraFocus( pos );
    }
  }

	if (fov != m_fFOV)
	{
		m_fFOV = fov;
		if (m_pOwner)
		{
			m_pOwner->OnNodeAnimated(this);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimCameraNode::Reset()
{
	CAnimEntityNode::Reset();
  m_lastFocusKey = -1;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCameraNode::SetParamValue( float time,AnimParamType param,float value )
{
	if (param == APARAM_FOV)
	{
		// Set default value.
		m_fFOV = value;
	}
	return CAnimEntityNode::SetParamValue( time,param,value );
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCameraNode::GetParamValue( float time,AnimParamType param,float &value )
{
	if (CAnimEntityNode::GetParamValue(time,param,value))
		return true;
	value = m_fFOV;
	return true;
}

//////////////////////////////////////////////////////////////////////////
IAnimTrack* CAnimCameraNode::CreateTrack(int nParamType)
{
	IAnimTrack *pTrack = CAnimEntityNode::CreateTrack(nParamType);
	if (nParamType == APARAM_FOV)
	{
		if (pTrack)
			pTrack->SetValue(0,m_fFOV,true);
	}
	return pTrack;
}

//////////////////////////////////////////////////////////////////////////
void CAnimCameraNode::Serialize( XmlNodeRef &xmlNode,bool bLoading )
{
	CAnimEntityNode::Serialize( xmlNode,bLoading );
	if (bLoading)
	{
		xmlNode->getAttr( "FOV",m_fFOV );
	}
	else
	{
		xmlNode->setAttr( "FOV",m_fFOV );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimCameraNode::ApplyFocusKey( ISelectKey &key,SAnimContext &ec )
{
  m_focusEntityId = 0;
  if (key.szSelection[0])
  {
    IEntity *pEntity = GetMovieSystem()->GetSystem()->GetIEntitySystem()->FindEntityByName( key.szSelection );
    if (pEntity)
    {
      m_focusEntityId = pEntity->GetId();
    }
  }
}

#undef s_nodeParamsInitialized
#undef s_nodeParams
#undef AddSupportedParam

