#include "StdAfx.h"
#include "AnimSpeed.h"
#include "ICryAnimation.h"
#include "AnimationGraphState.h"

CAnimSpeed::CAnimSpeed() : IAnimationStateNode(eASNF_Update)
{
}

CAnimSpeed::~CAnimSpeed()
{
}

void CAnimSpeed::EnterState( SAnimationStateData& data, bool dueToRollback )
{
	m_speedInputID = data.pState->GetInputId("Speed");
}

void CAnimSpeed::LeaveState( SAnimationStateData& data )
{
}

const IAnimationStateNodeFactory::Params * CAnimSpeed::GetParameters()
{
	static const Params params[] = 
	{
		{true,  "float",  "minSpeed",     "Min Speed",         "0"},
		{true,  "float",  "maxSpeed",     "Max Speed",         "0"},
		{true,  "float",  "animSpeedMin", "Anim. Speed Min",   "0"},
		{true,  "float",  "animSpeedMax", "Anim. Speed Max",   "0"},
		{0}
	};
	return params;
}

bool CAnimSpeed::CanLeaveState( SAnimationStateData& data )
{
	return true;
}

void CAnimSpeed::Update( SAnimationStateData& data )
{
	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);

	if (!pCharacter)
		return;

	float plSpeed = data.pState->GetInputAsFloat(m_speedInputID);

	float excursion(m_chSpeedMax - m_chSpeedMin);
	float speedDelta(m_animSpeedMax - m_animSpeedMin);
	float animSpeed = m_animSpeedMin + min(1.0f,(max(0.0f,plSpeed - m_chSpeedMin) / excursion)) * speedDelta;

	//CryLogAlways("%.1f",animSpeed);

	pCharacter->GetISkeletonAnim()->SetLayerUpdateMultiplier(0,animSpeed);
}

bool CAnimSpeed::Init( const XmlNodeRef& node, IAnimationGraphPtr )
{
	m_speedInputID = -1;

	m_chSpeedMin = 0;
	m_chSpeedMax = 0;

	m_animSpeedMin = 0;
	m_animSpeedMax = 0;

	node->getAttr("minSpeed", m_chSpeedMin);
	node->getAttr("maxSpeed", m_chSpeedMax);
	node->getAttr("animSpeedMin", m_animSpeedMin);
	node->getAttr("animSpeedMax", m_animSpeedMax);

	return true;
}

void CAnimSpeed::Release()
{
	delete this;
}

IAnimationStateNode * CAnimSpeed::Create()
{
	return this;
}
