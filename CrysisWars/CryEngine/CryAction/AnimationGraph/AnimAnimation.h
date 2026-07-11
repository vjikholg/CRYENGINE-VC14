// implements far cry style animation with the old cryanimation interfaces in the animation graph

#ifndef __ANIMANIMATION_H__
#define __ANIMANIMATION_H__

#pragma once

#include "IAnimationStateNode.h"

class CAnimAnimation : public IAnimationStateNodeFactory, public IAnimationStateNode
{
public:
	CAnimAnimation();
	virtual ~CAnimAnimation();

	// IAnimationStateNode
	virtual void EnterState( SAnimationStateData& data, bool dueToRollback );
	virtual EHasEnteredState HasEnteredState( SAnimationStateData& data ) { return eHES_Instant; }
	virtual bool CanLeaveState( SAnimationStateData& data );
	virtual void LeaveState( SAnimationStateData& data );
	virtual void EnteredState( SAnimationStateData& data ) {}
	virtual void LeftState( SAnimationStateData& data, bool wasEntered ) {}
	virtual void GetCompletionTimes( SAnimationStateData& data, CTimeValue start, CTimeValue& hard, CTimeValue& sticky );
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
	virtual void SerializeAsFile(bool reading, AG_FILE *file);
	virtual bool IsLessThan( IAnimationStateNodeFactory * pFactory )
	{
		AG_LT_BEGIN_FACTORY(CAnimAnimation);
			AG_LT_ELEM(m_currentAnimID);
			AG_LT_ELEM(m_animation);
			AG_LT_ELEM(m_blendTime);
			AG_LT_ELEM(m_blendOutTime);
			AG_LT_ELEM(m_loop);
			AG_LT_ELEM(m_recursive);
			AG_LT_ELEM(m_wait);
			AG_LT_ELEM(m_layerID);
			AG_LT_ELEM(m_nextAnimation);
			AG_LT_ELEM(m_completionTime);
		AG_LT_END();
	}
	// ~IAnimationStateNodeFactory

	virtual void GetStateMemoryStatistics(ICrySizer * s)
	{
	}
	virtual void GetFactoryMemoryStatistics(ICrySizer * s)
	{
		s->Add(*this);
		s->Add(m_animation);
	}

protected:
	int m_currentAnimID;
	string m_animation;
	float m_blendTime;
	float m_blendOutTime;
	bool m_loop;
	bool m_recursive;
	bool m_wait;
	char m_layerID;

	float m_nextAnimation;
	CTimeValue m_completionTime;

	static bool UpgradeWait( XmlNodeRef node );
	static bool UpgradeBlendIn( XmlNodeRef node );
	static bool UpgradeBlendOut( XmlNodeRef node );
};

#endif
