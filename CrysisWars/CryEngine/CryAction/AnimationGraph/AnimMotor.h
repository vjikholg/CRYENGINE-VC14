#ifndef __ANIMMOTOR_H__
#define __ANIMMOTOR_H__

#pragma once

#include "IAnimationStateNode.h"

class CAnimMotor : public IAnimationStateNodeFactory, public IAnimationStateNode
{
public:
	CAnimMotor() : IAnimationStateNode(eASNF_Update) {}

	// IAnimationStateNode
	virtual void EnterState( SAnimationStateData& data, bool dueToRollback );
	virtual EHasEnteredState HasEnteredState( SAnimationStateData& data ) { return eHES_Instant; }
	virtual void LeaveState( SAnimationStateData& data );
	virtual bool CanLeaveState( SAnimationStateData& data );
	virtual void EnteredState( SAnimationStateData& data ) {}
	virtual void LeftState( SAnimationStateData& data, bool ) {}
	virtual void Update( SAnimationStateData& data );
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
	virtual const char * GetCategory();
	virtual const char * GetName();
	virtual void SerializeAsFile(bool reading, AG_FILE *file)
	{
		SerializeAsFile_NodeBase(reading, file);
		FileSerializationHelper h(reading, file);
		h.Value(&m_speedVec);
		h.Value(&m_followAnimation);
		h.Value(&m_lastInertia);
		h.Value(&m_lastInertiaAccel);
	}
	virtual bool IsLessThan( IAnimationStateNodeFactory * pFactory )
	{
		AG_LT_BEGIN_FACTORY(CAnimMotor);
			AG_LT_ELEM(m_speedVec.x);
			AG_LT_ELEM(m_speedVec.y);
			AG_LT_ELEM(m_speedVec.z);
			AG_LT_ELEM(m_followAnimation);
			AG_LT_ELEM(m_lastInertia);
			AG_LT_ELEM(m_lastInertiaAccel);
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

	Vec3 m_speedVec;
	int m_followAnimation;

	float m_lastInertia;
	float m_lastInertiaAccel;
};

#endif
