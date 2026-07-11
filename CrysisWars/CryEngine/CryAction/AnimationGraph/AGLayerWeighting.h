#ifndef __AGLAYERWEIGHTING_H__
#define __AGLAYERWEIGHTING_H__

#pragma once

#include "IAnimationStateNode.h"

class CAGLayerWeighting;

class CAGLayerWeightingFactory : public IAnimationStateNodeFactory
{
	friend class CAGLayerWeighting;

public:
	CAGLayerWeightingFactory( int layer ) : m_layer(layer), m_blendMultiplier(0), m_blendOffset(0) {}
	inline virtual ~CAGLayerWeightingFactory() {}

	// IAnimationStateNodeFactory
	virtual bool Init( const XmlNodeRef& node, IAnimationGraphPtr );
	virtual void Release();
	virtual IAnimationStateNode * Create();
	virtual const char * GetCategory();
	virtual const char * GetName();
	virtual const Params * GetParameters();
	virtual void SerializeAsFile(bool reading, AG_FILE *file);

	virtual bool IsLessThan( IAnimationStateNodeFactory * pFactory )
	{
		AG_LT_BEGIN(CAGLayerWeightingFactory);
			AG_LT_ELEM(m_layer);
			AG_LT_ELEM(m_blendMultiplier);
			AG_LT_ELEM(m_blendOffset);
		AG_LT_END();
	}
	// ~IAnimationStateNodeFactory

	virtual void GetFactoryMemoryStatistics(ICrySizer * s)
	{
		s->Add(*this);
	}

private:
	void PerformOp( float blending, const SAnimationStateData& data );

	int m_layer;
	CCryName m_blendInput;
	float m_blendMultiplier;
	float m_blendOffset;
};

class CAGLayerWeighting : public IAnimationStateNode
{
public:
	CAGLayerWeighting( CAGLayerWeightingFactory * pFactory );

	// IAnimationStateNode
	virtual void EnterState( SAnimationStateData& data, bool dueToRollback );
	virtual EHasEnteredState HasEnteredState( SAnimationStateData& data ) { return eHES_Instant; }
	virtual bool CanLeaveState( SAnimationStateData& data );
	virtual void LeaveState( SAnimationStateData& data );
	virtual void EnteredState(SAnimationStateData& data ) {}
	virtual void LeftState(SAnimationStateData& data, bool wasEntered);
	virtual void Update( SAnimationStateData& data );
	virtual void GetCompletionTimes( SAnimationStateData& data, CTimeValue start, CTimeValue& hard, CTimeValue& sticky );
	virtual IAnimationStateNodeFactory * GetFactory() { return m_pFactory; }
	virtual void DebugDraw( SAnimationStateData& data, IRenderer * pRenderer, int x, int& y, int yIncrement ) {}
	// ~IAnimationStateNode

	virtual void GetStateMemoryStatistics(ICrySizer * s)
	{
		s->Add(*this);
	}

private:
	CAGLayerWeightingFactory * m_pFactory;
	int m_inputId;
	float m_weight;
};

template <int N>
class CAGLayerWeightingFactoryLayer : public CAGLayerWeightingFactory
{
public:
	CAGLayerWeightingFactoryLayer() : CAGLayerWeightingFactory(N) {}
};

#endif
