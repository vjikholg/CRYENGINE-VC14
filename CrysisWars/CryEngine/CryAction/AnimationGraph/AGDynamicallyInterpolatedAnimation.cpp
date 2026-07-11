#include "StdAfx.h"
#include "AGDynamicallyInterpolatedAnimation.h"
#include "AnimationGraphState.h"

#define LAYER_NAMES LAYER_NAMES_AGDYNANIM

static const char * LAYER_NAMES[] = {
	"DynamicallyInterpolatedAnimationLayer1",
	"DynamicallyInterpolatedAnimationLayer2",
	"DynamicallyInterpolatedAnimationLayer3",
	"DynamicallyInterpolatedAnimationLayer4",
	"DynamicallyInterpolatedAnimationLayer5",
	"DynamicallyInterpolatedAnimationLayer6",
	"DynamicallyInterpolatedAnimationLayer7",
	"DynamicallyInterpolatedAnimationLayer8",
	"DynamicallyInterpolatedAnimationLayer9",
};

const char * CAGDynamicallyInterpolatedAnimation::GetName()
{
	return LAYER_NAMES[GetLayer()];
}

CryCharAnimationParams CAGDynamicallyInterpolatedAnimation::GetAnimationParams( SAnimationStateData& data )
{
	CryCharAnimationParams params = CAGAnimation::GetAnimationParams(data);
	float interpolation = data.pState->GetInputAsFloat( data.pState->GetInputId(m_input.c_str()) );
	params.m_fInterpolation = CLAMP( (interpolation + m_offset) * m_multiplier, 0, 1 );
	return params;
}

const IAnimationStateNodeFactory::Params * CAGDynamicallyInterpolatedAnimation::GetParameters()
{
	static const Params params[] = 
	{
		{true,  "string", "animation1",    "Animation 1",                                      "" },
		{true,  "string", "animation2",    "Animation 2",                                      "" },
		{true,  "bool",   "ensureInStack", "Wait for animation to be playing?",                "0"},
		{true,  "float",  "stickyOutTime", "Sticky out time (or <0 for non-sticky playback)",  "-1"},
		{true,  "float",  "speedMultiplier", "Speed multiplier (increases playback speed on this animation)", "1"},
		{true,  "string", "interpolationInput", "interpolationInput", ""},
		{true,  "float",  "interpolationMultiplier", "interpolationMultiplier",  "1"},
		{true,  "float",  "interpolationOffset", "interpolationMultiplier",  "0"},
		{0}
	};
	return params;
}

bool CAGDynamicallyInterpolatedAnimation::Init( const XmlNodeRef& node, IAnimationGraphPtr pG )
{
	bool ok = CAGAnimation::Init(node, pG);
	m_input = node->getAttr("interpolationInput");
	ok &= node->getAttr("interpolationMultiplier", m_multiplier);
	ok &= node->getAttr("interpolationOffset", m_offset);
	return ok;
}

void CAGDynamicallyInterpolatedAnimation::SerializeAsFile(bool reading, AG_FILE *file)
{
	CAGAnimation::SerializeAsFile(reading, file);
	FileSerializationHelper h(reading, file);
	h.Value(&m_input);
	h.Value(&m_offset);
	h.Value(&m_multiplier);
}

#undef LAYER_NAMES
