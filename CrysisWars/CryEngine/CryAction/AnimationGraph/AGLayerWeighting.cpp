#include "StdAfx.h"
#include "AGLayerWeighting.h"
#include "AnimationGraphState.h"

#define LAYER_NAMES LAYER_NAMES_AGWEIGHT

static const char * LAYER_NAMES[] = {
	"WeightingLayer1",
	"WeightingLayer2",
	"WeightingLayer3",
	"WeightingLayer4",
	"WeightingLayer5",
	"WeightingLayer6",
	"WeightingLayer7",
	"WeightingLayer8",
	"WeightingLayer9",
};

CAGLayerWeighting::CAGLayerWeighting( CAGLayerWeightingFactory * pFactory ) : IAnimationStateNode(eASNF_Update), m_pFactory(pFactory), m_inputId(-7)
{
}

void CAGLayerWeighting::EnterState( SAnimationStateData& data, bool dueToRollback )
{
	m_inputId = data.pState->GetInputId(m_pFactory->m_blendInput.c_str());

	ICharacterInstance *pCharacter = data.pEntity->GetCharacter(0);

	if (pCharacter)
		m_weight = max(0.0f,pCharacter->GetISkeletonAnim()->GetIWeight(m_pFactory->m_layer));
	else
		m_weight = 0;
}

bool CAGLayerWeighting::CanLeaveState( SAnimationStateData& data )
{
	return true;
}

void CAGLayerWeighting::LeaveState( SAnimationStateData& data )
{
}

void CAGLayerWeighting::LeftState(SAnimationStateData& data, bool wasEntered)
{
	delete this;
}

void CAGLayerWeighting::GetCompletionTimes( SAnimationStateData& data, CTimeValue start, CTimeValue& hard, CTimeValue& sticky )
{
}

void CAGLayerWeighting::Update( SAnimationStateData& data )
{
	//m_pFactory->PerformOp( data.pState->GetInputAsFloat(m_inputId), data );

	float blending(data.pState->GetInputAsFloat(m_inputId));
	blending = CLAMP( (blending + m_pFactory->m_blendOffset) * m_pFactory->m_blendMultiplier, 0, 1 );
	Interpolate(m_weight,blending,3.0f,gEnv->pTimer->GetFrameTime());

	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);
	if (!pCharacter)
		return;

//	pCharacter->GetISkeleton()->SetBlendSpaceControl0(m_pFactory->m_layer, AnimParams(0,0,1,0,0) );
	pCharacter->GetISkeletonAnim()->SetIWeight(m_pFactory->m_layer, m_weight );
}

const IAnimationStateNodeFactory::Params * CAGLayerWeightingFactory::GetParameters()
{
	static const Params params[] = 
	{
		{true,  "string",   "input", "Blending Input",                ""},
		{true,  "float",   "offset",      "Blending Offset", "0"},
		{true,  "float",   "multiplier",      "Blending Multiplier", "0"},
		{0}
	};
	return params;
}

bool CAGLayerWeightingFactory::Init( const XmlNodeRef& node, IAnimationGraphPtr )
{
	bool ok = true;

	m_blendInput = node->getAttr("input");
	ok &= node->getAttr("offset", m_blendOffset);
	ok &= node->getAttr("multiplier", m_blendMultiplier);

	return ok;
}

void CAGLayerWeightingFactory::Release()
{
	delete this;
}

IAnimationStateNode * CAGLayerWeightingFactory::Create()
{
	return new CAGLayerWeighting(this);
}

const char * CAGLayerWeightingFactory::GetCategory()
{
	return LAYER_NAMES[m_layer];
}

const char * CAGLayerWeightingFactory::GetName()
{
	return LAYER_NAMES[m_layer];
}

void CAGLayerWeightingFactory::PerformOp( float blending, const SAnimationStateData& data )
{
	blending = CLAMP( (blending + m_blendOffset) * m_blendMultiplier, 0, 1 );

	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);
	if (!pCharacter)
		return;
	//pCharacter->GetISkeleton()->SetBlendSpaceControl0(m_layer, AnimParams(0,0,1,0,0) );
	pCharacter->GetISkeletonAnim()->SetIWeight(m_layer, blending );
}

void CAGLayerWeightingFactory::SerializeAsFile(bool reading, AG_FILE *file)
{
	FileSerializationHelper h(reading, file);
	h.Value(&m_layer);
	h.Value(&m_blendInput);
	h.Value(&m_blendMultiplier);
	h.Value(&m_blendOffset);
}

#undef LAYER_NAMES
