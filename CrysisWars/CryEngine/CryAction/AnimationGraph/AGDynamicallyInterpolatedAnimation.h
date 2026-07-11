#ifndef __AGDYNAMICALLYINTERPOLATEDANIMATION_H__
#define __AGDYNAMICALLYINTERPOLATEDANIMATION_H__

#include "AGAnimation.h"

class CAGDynamicallyInterpolatedAnimation : public CAGAnimation
{
public:
	CAGDynamicallyInterpolatedAnimation( int layer ) : CAGAnimation(layer) {}

	// IAnimationStateNode
	virtual const Params * GetParameters();
	// ~IAnimationStateNode

	// IAnimationStateNodeFactory
	virtual bool Init( const XmlNodeRef& node, IAnimationGraphPtr );
	virtual const char * GetName();
	virtual void SerializeAsFile(bool reading, AG_FILE *file);

	bool IsLessThan( IAnimationStateNodeFactory * pFactory )
	{
		AG_LT_BEGIN_PARENT(CAGDynamicallyInterpolatedAnimation, CAGAnimation);
			AG_LT_ELEM(m_input);
			AG_LT_ELEM(m_multiplier);
			AG_LT_ELEM(m_offset);
		AG_LT_END();
	}
	// ~IAnimationStateNodeFactory

protected:
	virtual CryCharAnimationParams GetAnimationParams( SAnimationStateData& data );

private:
	CCryName m_input;
	float m_multiplier;
	float m_offset;
};

template <int N>
class CAGDynamicallyInterpolatedAnimationLayer : public CAGDynamicallyInterpolatedAnimation
{
public:
	CAGDynamicallyInterpolatedAnimationLayer() : CAGDynamicallyInterpolatedAnimation(N) {}
};

#endif
