// implements new (locomotion manager) animations in the animation graph

#ifndef __ANIMANIMATIONNEW_H__
#define __ANIMANIMATIONNEW_H__

#pragma once

#include "IAnimationStateNode.h"

class CAnimAnimationNewFactory;

class CAnimAnimationNew : public IAnimationStateNode
{
public:
  CAnimAnimationNew(CAnimAnimationNewFactory * pFactory);
  virtual ~CAnimAnimationNew();

  // IAnimationStateNode
  virtual void EnterState( SAnimationStateData& data, bool dueToRollback );
	virtual EHasEnteredState HasEnteredState( SAnimationStateData& data ) { return eHES_Instant; }
	virtual void LeaveState( SAnimationStateData& data );
	virtual bool CanLeaveState( SAnimationStateData& data );
  virtual void Update( SAnimationStateData& data );
	virtual IAnimationStateNodeFactory * GetFactory();
	virtual void EnteredState( SAnimationStateData& data ) {}
	virtual void LeftState( SAnimationStateData& data, bool wasEntered ) {delete this;}
	virtual void DebugDraw( SAnimationStateData& data, IRenderer * pRenderer, int x, int& y, int yIncrement ) {}

	void GetCompletionTimes( SAnimationStateData& data, CTimeValue start, CTimeValue& hard, CTimeValue& sticky ) 
	{
		hard = sticky = 0.0f;
	}
  // ~IAnimationStateNode

	virtual void StartAnimation(SAnimationStateData& data);

	virtual void GetStateMemoryStatistics( ICrySizer * s )
	{
		s->Add(*this);
	}

protected:

	CAnimAnimationNewFactory * m_pFactory;

	int m_currentAnimID;
	float m_weightAmount;

	bool m_startAnim;
};

class CAnimAnimationNewFactory : public IAnimationStateNodeFactory
{
	friend class CAnimAnimationNew;

public:
	CAnimAnimationNewFactory();
	virtual ~CAnimAnimationNewFactory();

	// IAnimationStateNodeFactory
	virtual bool Init( const XmlNodeRef& node, IAnimationGraphPtr );
	virtual void Release();
	virtual IAnimationStateNode * Create() { return new CAnimAnimationNew(this); }
	virtual const char * GetCategory() {return "AnimationNew";}
	virtual const char * GetName() {return "AnimationNew";}
	virtual const Params * GetParameters();
	virtual void SerializeAsFile(bool reading, AG_FILE *file);

	virtual bool IsLessThan( IAnimationStateNodeFactory * pFactory )
	{
		AG_LT_BEGIN(CAnimAnimationNewFactory);
			AG_LT_ELEM(m_animation);
			AG_LT_ELEM(m_animation2);
			AG_LT_ELEM(m_mixerInput);
			AG_LT_ELEM(m_mixerInputID);
			AG_LT_ELEM(m_mixerMult);
			AG_LT_ELEM(m_weightInput);
			AG_LT_ELEM(m_weightInputID);
			AG_LT_ELEM(m_weightMult);
			AG_LT_ELEM(m_weightOffset);
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
			AG_LT_ELEM(m_stopCurrentAnimation);
			AG_LT_ELEM(m_skipIfSameAnimation);
			AG_LT_ELEM(m_transitionTime);
			AG_LT_ELEM(m_animSpeed);
			AG_LT_ELEM(m_layerID);
		AG_LT_END();
	}
	// ~IAnimationStateNodeFactory

	virtual void GetFactoryMemoryStatistics(ICrySizer * s)
	{
		s->Add(*this);
		s->Add(m_animation);
		s->Add(m_animation2);
		s->Add(m_mixerInput);
		s->Add(m_mixerMult);
		s->Add(m_weightInput);
	}

protected:

	string m_animation;
	string m_animation2;
	
	//inputs
	string m_mixerInput;
	int m_mixerInputID;
	float m_mixerMult;

	string m_weightInput;
	int m_weightInputID;
	float m_weightMult;
	float m_weightOffset;

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
	bool m_stopCurrentAnimation;
	bool m_skipIfSameAnimation;
	//

	float m_transitionTime;

	float m_animSpeed;

	char m_layerID;

	static bool UpgradeEnsureInStack( XmlNodeRef node );
	static bool UpgradeLoopAnimation( XmlNodeRef node );
	static bool UpgradeRepeatLastKey( XmlNodeRef node );
	static bool UpgradePhaseSyncing( XmlNodeRef node );
	static bool UpgradeVerticalTimeWarping( XmlNodeRef node );
	static bool UpgradeStartAfter( XmlNodeRef node );
	static bool UpgradeIgnoreFootplants( XmlNodeRef node );
	static bool UpgradeFootAnchorTranslation( XmlNodeRef node );
	static bool UpgradeFootAnchorRotation( XmlNodeRef node );
	static bool UpgradeRecursive( XmlNodeRef node );
	static bool UpgradeAligned( XmlNodeRef node );
	static bool UpgradeStopCurrentAnimation( XmlNodeRef node );
	static bool UpgradeSkipIfSameAnimation( XmlNodeRef node );
};

#endif
