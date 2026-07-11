#ifndef __ANIMANIMATIONUPPERBODY_H__
#define __ANIMANIMATIONUPPERBODY_H__

#pragma once

#include "IAnimationStateNode.h"

class CAnimAnimationUpperbody : public CAnimAnimation
{
public:
	virtual bool Init( const XmlNodeRef& node, IAnimationGraphPtr pG ) 
	{
		if (CAnimAnimation::Init(node, pG))
		{
			m_layerID = 1;

			node->getAttr("layer", m_layerID);

			return true;
		}
		else
		{
			return false;
		}
	};

	virtual const char *GetName()	{return "AnimationUpperbody";};
	virtual const char *GetCategory() {return "AnimationUpperbody";};
};

#endif
