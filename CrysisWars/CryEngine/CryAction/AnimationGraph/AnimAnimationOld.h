// implements far cry style animation with the locomotion manager interfaces in the animation graph

#ifndef __ANIMANIMATIONOLD_H__
#define __ANIMANIMATIONOLD_H__

#pragma once

#include "IAnimationStateNode.h"

class CAnimAnimationOld : public IAnimationStateNodeFactory, public IAnimationStateNode
{
public:
	CAnimAnimationOld();
	virtual ~CAnimAnimationOld();

	// IAnimationStateNode
	virtual void EnterState( SAnimationStateData& data, bool dueToRollback );
	virtual EHasEnteredState HasEnteredState( SAnimationStateData& data ) { return eHES_Instant; }
	virtual bool CanLeaveState( SAnimationStateData& data );
	virtual void LeaveState( SAnimationStateData& data );
	virtual void EnteredState( SAnimationStateData& data ) {}
	virtual void LeftState( SAnimationStateData& data, bool ) {}
	virtual void Update( SAnimationStateData& data );
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
		AG_LT_BEGIN_FACTORY(CAnimAnimationOld);
			AG_LT_ELEM(m_currentAnimID);
			AG_LT_ELEM(m_animation);
			AG_LT_ELEM(m_blendTime);
			AG_LT_ELEM(m_blendOutTime);
			AG_LT_ELEM(m_wait);
			AG_LT_ELEM(m_layerID);
			AG_LT_ELEM(m_ensureInStack);
			AG_LT_ELEM(m_loopAnimation);
			AG_LT_ELEM(m_repeatLastKey);
			AG_LT_ELEM(m_phaseSyncing);
			AG_LT_ELEM(m_vtimeWarping);
			AG_LT_ELEM(m_startAfter);
			AG_LT_ELEM(m_ignoreFootplants);
			AG_LT_ELEM(m_footAnchorTranslation);
			AG_LT_ELEM(m_footAnchorRotation);
			AG_LT_ELEM(m_recursive);
			AG_LT_ELEM(m_aligned);
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
	bool m_wait;
	char m_layerID;

	bool m_ensureInStack;
	bool m_loopAnimation;
	bool m_repeatLastKey;
	bool m_phaseSyncing;
	bool m_vtimeWarping;
	bool m_startAfter;
	bool m_ignoreFootplants;
	bool m_footAnchorTranslation;
	bool m_footAnchorRotation;
	bool m_recursive;
	bool m_aligned;

	float m_nextAnimation;
	CTimeValue m_completionTime;

	static bool UpgradeWait( XmlNodeRef node );
	static bool UpgradeBlendIn( XmlNodeRef node );
	static bool UpgradeBlendOut( XmlNodeRef node );
	static bool UpgradeEnsureInStack( XmlNodeRef node );

	static bool UpgradeLoopAnimation( XmlNodeRef node );
	static bool UpgradeRepeatLastKey( XmlNodeRef node );
	static bool UpgradePhaseSyncing( XmlNodeRef node );
	static bool UpgradeVerticalTimeWarping( XmlNodeRef node );
	static bool UpgradeStartAfter( XmlNodeRef node );
	static bool UpgradeIgnoreFootplants( XmlNodeRef node );
	static bool UpgradeFootAnchorTranslation( XmlNodeRef node );
	static bool UpgradeFootAnchorRotation( XmlNodeRef node );
	static bool UpgradeAligned( XmlNodeRef node );
};

#endif
