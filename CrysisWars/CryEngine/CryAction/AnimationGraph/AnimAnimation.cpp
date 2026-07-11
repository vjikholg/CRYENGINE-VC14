#include "StdAfx.h"
#include "AnimAnimation.h"
#include "ICryAnimation.h"

CAnimAnimation::CAnimAnimation()
{
}

CAnimAnimation::~CAnimAnimation()
{
}

const IAnimationStateNodeFactory::Params * CAnimAnimation::GetParameters()
{
	static const Params params[] = 
	{
		{true,  "string", "play",         "Animation",          ""},
		{true,  "float",  "blendInTime",  "Blend in time",      "0.3", UpgradeBlendIn},
		{true,  "float",  "blendOutTime", "Blend out time",     "0.3", UpgradeBlendOut},
		{true,  "bool",   "loop",         "Loop",               "0"},
		{true,  "bool",   "wait",         "Wait for animation", "1",   UpgradeWait},
		{true,  "bool",   "recursive",    "Recursive",          "1"},
		{true, "int",		"layer",			"Layer",						"0"},
		{0}
	};
	return params;
}

bool CAnimAnimation::UpgradeWait( XmlNodeRef node )
{
	bool loop = false;
	node->getAttr("loop", loop);
	node->setAttr("wait", !loop);
	return true;
}

bool CAnimAnimation::UpgradeBlendIn( XmlNodeRef node )
{
	float blend = 0.3f;
	node->getAttr("blendTime", blend);
	node->setAttr("blendInTime", blend);
	return true;
}

bool CAnimAnimation::UpgradeBlendOut( XmlNodeRef node )
{
	float blend;
	node->getAttr("blendTime", blend);
	node->setAttr("blendOutTime", blend);
	return true;
}

void CAnimAnimation::EnterState( SAnimationStateData& data, bool dueToRollback )
{
	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);
	if (!pCharacter)
	{
		GameWarning("Entity %s has no character attached", data.pEntity->GetClass()->GetName());
		return;
	}
	IAnimationSet * pAnimSet = pCharacter->GetIAnimationSet();
	
	CryCharAnimationParams params;
	params.m_nLayerID = m_layerID;
	//params.m_nFlags |= (m_recursive? CA_RECURSIVE : 0);

//	if (params.m_nLayerID != 0)
//		params.m_nFlags |= CA_PHASE_SYNCHING;

	m_currentAnimID = pAnimSet->GetAnimIDByName(m_animation);
//	if (m_currentAnimID < 0)
//		GameWarning("%s:cannot find animation %s", data.pEntity->GetName(),m_animation.c_str());
	//same animation? do nothing


	CryLogAlways("Using deprecated AnimAnimation node in animation graph (entity %s)", data.pEntity->GetName());
	//old anim-system: removed by ivo
/*	if (m_currentAnimID != pCharacter->GetCurrentAnimation(params.nLayerID))
	{
		m_nextAnimation = pAnimSet->GetLength(m_currentAnimID);
		m_completionTime = gEnv->pTimer->GetFrameStartTime() + m_nextAnimation;

		pCharacter->SetLoop( m_animation, m_loop, m_recursive );
		pCharacter->StartAnimation( m_animation, params );
		pCharacter->SetAnimationSpeedLayer(params.nLayerID,1.0f);
	}*/

}

bool CAnimAnimation::CanLeaveState( SAnimationStateData& data )
{
	return true;
}

void CAnimAnimation::LeaveState( SAnimationStateData& data )
{
	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);

	if (!pCharacter)
	{
		GameWarning("Entity %s has no character attached", data.pEntity->GetClass()->GetName());
		return;
	}

	//stop the animation only if its the same animation called by this node, kind of dangerous.
//	if (m_currentAnimID == pCharacter->GetCurrentAnimation(m_layerID))
//		pCharacter->StopAnimationInLayer(m_layerID);
	CryLogAlways("Using deprecated AnimAnimation node in animation graph (entity %s)", data.pEntity->GetName());
}

bool CAnimAnimation::Init( const XmlNodeRef& node, IAnimationGraphPtr )
{
	m_animation = node->getAttr("play");
	if (m_animation.empty())
		return false;

	m_currentAnimID = -1;
	m_blendTime = 0.125f;
	m_loop = false;
	m_recursive = false;
	m_wait = false;
	m_nextAnimation = 0;
	m_completionTime = 0.0f;
	m_layerID = 0;

	node->getAttr("blendInTime", m_blendTime);
	node->getAttr("blendOutTime", m_blendOutTime);

	node->getAttr("loop", m_loop);
	node->getAttr("recursive", m_recursive);
	node->getAttr("wait", m_wait);

	node->getAttr("layer", m_layerID);
	return true;
}

void CAnimAnimation::Release()
{
	delete this;
}

IAnimationStateNode * CAnimAnimation::Create()
{
	return this;
}

const char * CAnimAnimation::GetCategory()
{
	return "Animation";
}

const char * CAnimAnimation::GetName()
{
	return "Animation";
}

void CAnimAnimation::GetCompletionTimes( SAnimationStateData& data, CTimeValue start, CTimeValue& hard, CTimeValue& sticky ) 
{ 
	if (m_wait)
		hard = m_completionTime;
	else
		hard = 0.0f;
	if (m_loop)
		sticky = 0.0f;
	else
		sticky = m_completionTime;
}

void CAnimAnimation::SerializeAsFile(bool reading, AG_FILE *file)
{
	SerializeAsFile_NodeBase(reading, file);

	FileSerializationHelper h(reading, file);

	h.Value(&m_currentAnimID);
	h.StringValue(&m_animation);
	h.Value(&m_blendTime);
	h.Value(&m_blendOutTime);
	h.Value(&m_loop);
	h.Value(&m_recursive);
	h.Value(&m_wait);
	h.Value(&m_layerID);
	h.Value(&m_nextAnimation);
	h.Value(&m_completionTime);
}