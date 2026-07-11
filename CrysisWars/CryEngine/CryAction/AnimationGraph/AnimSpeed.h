#ifndef __ANIMSPEED_H__
#define __ANIMSPEED_H__

#pragma once

#include "IAnimationStateNode.h"

class CAnimSpeed : public IAnimationStateNodeFactory, public IAnimationStateNode
{
public:
	CAnimSpeed();
	~CAnimSpeed();

	// IAnimationStateNode
	virtual void EnterState( SAnimationStateData& data, bool dueToRollback );
	virtual EHasEnteredState HasEnteredState( SAnimationStateData& data ) { return eHES_Instant; }
	virtual bool CanLeaveState( SAnimationStateData& data );
	virtual void LeaveState( SAnimationStateData& data );
	virtual void Update( SAnimationStateData& data );
	virtual void EnteredState( SAnimationStateData& data ) {}
	virtual void LeftState( SAnimationStateData& data, bool ) {}
	void GetCompletionTimes( SAnimationStateData& data, CTimeValue start, CTimeValue& hard, CTimeValue& sticky ) 
	{
		hard = sticky = 0.0f;
	}
	virtual const Params * GetParameters();
	virtual IAnimationStateNodeFactory * GetFactory() { return this; }
	virtual void DebugDraw( SAnimationStateData& data, IRenderer * pRenderer, int x, int& y, int yIncrement ) {}
	// ~IAnimationStateNode

	// IAnimationStateNodeFactory
	virtual bool Init( const XmlNodeRef& node, IAnimationGraphPtr );
	virtual void Release();
	virtual IAnimationStateNode * Create();
	virtual const char * GetCategory() {return "General";}
	virtual const char * GetName() {return "AnimationSpeed";}
	virtual void SerializeAsFile(bool reading, AG_FILE *file)
	{
		SerializeAsFile_NodeBase(reading, file);
		FileSerializationHelper h(reading, file);
		h.Value(&m_chSpeedMin);
		h.Value(&m_chSpeedMax);
		h.Value(&m_animSpeedMin);
		h.Value(&m_animSpeedMax);
		h.Value(&m_speedInputID);
	}
	virtual bool IsLessThan( IAnimationStateNodeFactory * pFactory )
	{
		AG_LT_BEGIN_FACTORY(CAnimSpeed);
			AG_LT_ELEM(m_chSpeedMin);
			AG_LT_ELEM(m_chSpeedMax);
			AG_LT_ELEM(m_animSpeedMin);
			AG_LT_ELEM(m_animSpeedMax);
			AG_LT_ELEM(m_speedInputID);
		AG_LT_END();
	}
	// ~IAnimationStateNodeFactory

	virtual void GetStateMemoryStatistics(ICrySizer * s)
	{
	}
	virtual void GetFactoryMemoryStatistics(ICrySizer * s)
	{
		s->Add(*this);
	}


private:
	float m_chSpeedMin;
	float m_chSpeedMax;

	float m_animSpeedMin;
	float m_animSpeedMax;

	int m_speedInputID;
};

#endif
