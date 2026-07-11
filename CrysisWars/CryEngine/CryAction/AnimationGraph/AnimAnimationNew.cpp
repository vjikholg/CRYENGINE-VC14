#include "StdAfx.h"
#include "AnimAnimationNew.h"
#include "ICryAnimation.h"

#include "AnimationGraphState.h"

CAnimAnimationNew::CAnimAnimationNew(CAnimAnimationNewFactory * pFactory) : IAnimationStateNode(eASNF_Update)
{
	m_pFactory = pFactory;
	//
	m_currentAnimID = -1;
	m_weightAmount = 0.0f;
}

CAnimAnimationNew::~CAnimAnimationNew()
{
}

void CAnimAnimationNew::StartAnimation(SAnimationStateData& data)
{
	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);

	if (!pCharacter)
	{
		GameWarning("Entity %s has no character attached", data.pEntity->GetClass()->GetName());
		return;
	}

	if (pCharacter->GetISkeletonAnim()->GetTrackViewStatus())
		return;

	/*int numAnims = pCharacter->GetISkeleton()->GetNumAnimsInFIFO(m_pFactory->m_layerID);
	if (numAnims>=3)
		return;*/

	IAnimationSet* pAnimSet = pCharacter->GetIAnimationSet();
	
	CryCharAnimationParams params (0);

	params.m_nFlags |= m_pFactory->m_loopAnimation * CA_LOOP_ANIMATION;
	params.m_nFlags |= m_pFactory->m_repeatLastKey * CA_REPEAT_LAST_KEY;
	params.m_nFlags |= m_pFactory->m_vtimeWarping * CA_TRANSITION_TIMEWARPING;
	params.m_nFlags |= m_pFactory->m_startAfter * CA_START_AFTER;

	params.m_fTransTime = m_pFactory->m_transitionTime;
	params.m_fInterpolation = 1.0f;
//	params.m_nCopyTimeFromLayer = -1;
	params.m_nLayerID = m_pFactory->m_layerID;

	params.m_fUserData[eAGUD_MovementControlMethodH] = data.MovementControlMethodH;
	params.m_fUserData[eAGUD_MovementControlMethodV] = data.MovementControlMethodV;
	params.m_fUserData[eAGUD_AnimationControlledView] = data.animationControlledView? 1.0f : 0.0f;
	params.m_fUserData[eAGUD_AdditionalTurnMultiplier] = data.additionalTurnMultiplier;

	m_currentAnimID = pAnimSet->GetAnimIDByName(m_pFactory->m_animation.c_str());
//	if (m_currentAnimID < 0)
//		GameWarning("%s:cannot find animation %s", data.pEntity->GetName(),m_pFactory->m_animation.c_str());

	bool skipAnim(false);

	if (m_pFactory->m_skipIfSameAnimation)
	{
		int numAnims = pCharacter->GetISkeletonAnim()->GetNumAnimsInFIFO(m_pFactory->m_layerID);
		for (int i=0;i<numAnims;++i)
		{
			int currentID = pCharacter->GetISkeletonAnim()->GetAnimFromFIFO(params.m_nLayerID,i).m_LMG0.m_nAnimID[0];
			if (m_currentAnimID == currentID)
			{
				skipAnim = true;
				break;
			}
		}
	}

	if (!skipAnim)
	{
		if (m_pFactory->m_stopCurrentAnimation)
			pCharacter->GetISkeletonAnim()->StopAnimationInLayer(params.m_nLayerID,0.0f);

		bool secondAnim(!m_pFactory->m_animation2.empty());

		m_weightAmount = max(0.0f,pCharacter->GetISkeletonAnim()->GetIWeight(params.m_nLayerID));

		bool res = pCharacter->GetISkeletonAnim()->StartAnimation(m_pFactory->m_animation.c_str(),secondAnim?m_pFactory->m_animation2.c_str():0, 0,0, params);
		//CryLogAlways("StartAnimationNew:%s:%s",m_pFactory->m_animation.c_str(),res?"ok":"failed");

		//pCharacter->GetISkeleton()->SetIWeight(params.nLayerID,Vec4(0,0,0,m_weightAmount));
	//	pCharacter->GetISkeleton()->SetBlendSpaceControl0(params.m_nLayerID,AnimParams(0,0,1,0,0));
		pCharacter->GetISkeletonAnim()->SetIWeight(params.m_nLayerID,m_weightAmount);
		//pCharacter->GetISkeleton()->MotionMixer(LAYER00,params.nLayerID,FINALREL,0,BLENDLAYER);

		//pCharacter->SetLoop( m_pFactory->m_animation.c_str(), true, false );
		pCharacter->GetISkeletonAnim()->SetLayerUpdateMultiplier(params.m_nLayerID,m_pFactory->m_animSpeed);
	}
}

void CAnimAnimationNew::EnterState( SAnimationStateData& data, bool dueToRollback )
{
	m_pFactory->m_mixerInputID = data.pState->GetInputId(m_pFactory->m_mixerInput.c_str());
	m_pFactory->m_weightInputID = data.pState->GetInputId(m_pFactory->m_weightInput.c_str());

	m_startAnim = true;

	StartAnimation(data);
}

bool CAnimAnimationNew::CanLeaveState( SAnimationStateData& data )
{
	if (m_pFactory->m_ensureInStack)
	{
		ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);
		if (!pCharacter)
			return true;
		IAnimationSet* pIAnimationSet = pCharacter->GetIAnimationSet();
		ISkeletonAnim* pSkeletonAnim = pCharacter->GetISkeletonAnim();

		// break out early if there are no anims on stack -> we can leave the state!
		int animsInFIFO(pSkeletonAnim->GetNumAnimsInFIFO(m_pFactory->m_layerID));
		if (animsInFIFO <= 0)
			return true;

		/*do
		{
			CAnimation& animation = pSkeleton->GetAnimFromFIFO(m_pFactory->m_layerID,animsInFIFO-1);*/

			CAnimation& animation = pSkeletonAnim->GetAnimFromFIFO(m_pFactory->m_layerID,0);
			
			int32 currID0 = animation.m_LMG0.m_nAnimID[0];
			int32 currID1 = animation.m_LMG1.m_nAnimID[0];

			int32 id0 = pIAnimationSet->GetAnimIDByName( m_pFactory->m_animation );
			int32 id1 = pIAnimationSet->GetAnimIDByName( m_pFactory->m_animation2 );

			if (currID0 == id0 || currID1 == id1)
			{
				//float color[] = {1,1,1,0.5f};
				//gEnv->pRenderer->Draw2dLabel(100,100,2,color,false,"CANNOT leave state!");

				return false;
			}

		/*	--animsInFIFO;
		} while(animsInFIFO>0);*/
	}

	return true;
}

void CAnimAnimationNew::LeaveState( SAnimationStateData& data )
{
	/*ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);

	if (pCharacter)
		pCharacter->GetISkeleton()->StopAnimation(m_layerID);*/
}

void CAnimAnimationNew::Update( SAnimationStateData& data )
{
	/*if (m_startAnim)
	{
		StartAnimation(data);
		m_startAnim = false;
	}*/
  
	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);
	if (!pCharacter || pCharacter->GetISkeletonAnim()->GetTrackViewStatus())
		return;

	float inputVal;
	float amount;

	int layerID(m_pFactory->m_layerID);

	if (!m_pFactory->m_animation2.empty() || m_weightAmount>0.001f)
	{
		inputVal = data.pState->GetInputAsFloat(m_pFactory->m_weightInputID);
		amount = max(0.0f,min(1.0f,(m_pFactory->m_weightOffset + inputVal)*m_pFactory->m_weightMult));

		Interpolate(m_weightAmount,amount,3.0f,gEnv->pTimer->GetFrameTime());

	//	pCharacter->GetISkeleton()->SetBlendSpaceControl0(layerID,AnimParams(0,0,1,0,0) );
		pCharacter->GetISkeletonAnim()->SetIWeight(layerID,m_weightAmount );
		//CryLogAlways("%s weight:%.2f",m_animation.c_str(),m_weightAmount);
	}
	else
	{
		//pCharacter->GetISkeleton()->SetBlendSpaceControl0(layerID,AnimParams(0,0,1,0,0) );
		pCharacter->GetISkeletonAnim()->SetIWeight(layerID,0);
	}
			
	inputVal = data.pState->GetInputAsFloat(m_pFactory->m_mixerInputID);
	amount = max(0.0f,min(1.0f,inputVal*m_pFactory->m_mixerMult));
		
	if (amount>0.0001f)
	{
		/*ISkeleton* pISkeleton = pCharacter->GetISkeleton();
		pISkeleton->ClearMotionMixerArray();

		uint32 numAnimsLayer0 = pISkeleton->GetNumAnimsInFIFO(LAYER00);
		uint32 numAnimsLayer1 = pISkeleton->GetNumAnimsInFIFO(layerID);
		if (numAnimsLayer0 && numAnimsLayer1)
			pISkeleton->PushMotionMixer(LAYER00,layerID,FINALREL,amount,BLENDLAYER);
	*/

		//CryLogAlways("%s mixer:%.2f",m_animation.c_str(),amount);
	}

	pCharacter->GetISkeletonAnim()->SetLayerUpdateMultiplier(layerID,m_pFactory->m_animSpeed);
}

//======================================================
//CAnimAnimationNewFactory
//======================================================
CAnimAnimationNewFactory::CAnimAnimationNewFactory()
{
}

CAnimAnimationNewFactory::~CAnimAnimationNewFactory()
{
}

const IAnimationStateNodeFactory::Params * CAnimAnimationNewFactory::GetParameters()
{
	static const Params params[] = 
	{
		{true,  "string", "play",       "Animation",        ""},
		{true,  "string", "play2",      "Animation 2",      ""},

		{true, "int",		"layer",			"Layer",						"0"},
		{true, "float", "transitionTime",			"Transition time",				"-1.0"},

		{true, "string", "mixerInput",			"Mixer Input name",				""},
		{true, "float",  "mixerMult",	"Mixer Input multiplier",	"1.0"},

		{true, "string", "weightInput",			"Weight Input name",				""},
		{true, "float",  "weightMult",	"Weight Input multiplier",	"1.0"},
		{true, "float",  "weightOffset",	"Weight Input offset",	"0.0"},

		{true, "bool",   "ensureInStack", "Ensure animation is in the stack?", "0", UpgradeEnsureInStack},

		{true, "bool",   "loopAnimation", "Loop animation", "1", UpgradeLoopAnimation},
		{true, "bool",   "repeatLastKey", "Repeat last key", "0", UpgradeRepeatLastKey},
		{true, "bool",   "phaseSyncing", "Phase syncing", "1", UpgradePhaseSyncing},
		{true, "bool",   "vtimeWarping", "Vertical time warping", "1", UpgradeVerticalTimeWarping},
		{true, "bool",   "startAfter", "Start after", "0", UpgradeStartAfter},
		{true, "bool",   "ignoreFootplants", "Ignore footplants", "0", UpgradeIgnoreFootplants},
		{true, "bool",   "recursive", "Recursive", "1", UpgradeRecursive},
		{true, "bool",   "aligned", "Aligned", "0", UpgradeAligned},
		{true, "bool",   "stopCurrentAnimation", "Stop the current Animation first", "0", UpgradeStopCurrentAnimation},
		{true, "bool",   "skipIfSameAnimation", "Dont start a new animation if there is the same animation already playing.", "0", UpgradeSkipIfSameAnimation},
		{0}
	};
	return params;
}

#undef DECL_UPGRADE_FLAG
#define DECL_UPGRADE_FLAG(param, name, value) \
	bool CAnimAnimationNewFactory::name( XmlNodeRef node ) \
	{ \
		node->setAttr(param, value); \
		return true; \
	}

DECL_UPGRADE_FLAG("loopAnimation", UpgradeLoopAnimation, true)
DECL_UPGRADE_FLAG("repeatLastKey", UpgradeRepeatLastKey, false)
DECL_UPGRADE_FLAG("phaseSyncing", UpgradePhaseSyncing, true)
DECL_UPGRADE_FLAG("vtimeWarping", UpgradeVerticalTimeWarping, true)
DECL_UPGRADE_FLAG("startAfter", UpgradeStartAfter, false)
DECL_UPGRADE_FLAG("ignoreFootplants", UpgradeIgnoreFootplants, false)
DECL_UPGRADE_FLAG("recursive", UpgradeRecursive, true)
DECL_UPGRADE_FLAG("aligned", UpgradeAligned, false)
DECL_UPGRADE_FLAG("stopCurrentAnimation", UpgradeStopCurrentAnimation, false)
DECL_UPGRADE_FLAG("skipIfSameAnimation", UpgradeSkipIfSameAnimation, false)

bool CAnimAnimationNewFactory::UpgradeEnsureInStack( XmlNodeRef node )
{
	int layer = 1;
	node->getAttr("layer", layer);
	node->setAttr("ensureInStack", layer==0);
	return true;
}

bool CAnimAnimationNewFactory::Init( const XmlNodeRef& node, IAnimationGraphPtr )
{
	m_animation = node->getAttr("play");
	if (m_animation.empty())
		return false;

	m_animation2 = node->getAttr("play2");

	m_layerID = 0;
	m_transitionTime = -1.0f;
	node->getAttr("layer", m_layerID);
	node->getAttr("transitionTime", m_transitionTime);

	m_mixerInputID = -1;
	m_mixerMult = 1.0f;
	node->getAttr("mixerMult", m_mixerMult);
	m_mixerInput = node->getAttr("mixerInput");

	m_weightInputID = -1;
	m_weightMult = 1.0f;
	m_weightOffset = 0.0f;
	
	node->getAttr("weightMult", m_weightMult);
	node->getAttr("weightOffset", m_weightOffset);
	m_weightInput = node->getAttr("weightInput");

	m_animSpeed = 1.0f;
	node->getAttr("animSpeed", m_animSpeed);

#define GETFLAG(name, def) m_##name = def; node->getAttr(#name, m_##name);

	GETFLAG(ensureInStack, false);
	GETFLAG(loopAnimation, true);
	GETFLAG(repeatLastKey, false);
	GETFLAG(phaseSyncing, true);
	GETFLAG(vtimeWarping, true);
	GETFLAG(startAfter, false);
	GETFLAG(ignoreFootplants, false);
	GETFLAG(recursive, true);
	GETFLAG(aligned, false);
	GETFLAG(stopCurrentAnimation, false);
	GETFLAG(skipIfSameAnimation, false);
		
	return true;
}

void CAnimAnimationNewFactory::Release()
{
	delete this;
}

IAnimationStateNodeFactory * CAnimAnimationNew::GetFactory()
{
	return m_pFactory;
}

void CAnimAnimationNewFactory::SerializeAsFile(bool reading, AG_FILE *file)
{
	FileSerializationHelper h(reading, file);

	h.StringValue(&m_animation);
	h.StringValue(&m_animation2);
	h.StringValue(&m_mixerInput);
	h.Value(&m_mixerInputID);
	h.Value(&m_mixerMult);
	h.StringValue(&m_weightInput);
	h.Value(&m_weightInputID);
	h.Value(&m_weightMult);
	h.Value(&m_weightOffset);
	h.Value(&m_ensureInStack);
	h.Value(&m_loopAnimation);
	h.Value(&m_repeatLastKey);
	h.Value(&m_phaseSyncing);
	h.Value(&m_vtimeWarping);
	h.Value(&m_startAfter);
	h.Value(&m_ignoreFootplants);
	h.Value(&m_footAnchorRotation);
	h.Value(&m_footAnchorTranslation);
	h.Value(&m_recursive);
	h.Value(&m_aligned);
	h.Value(&m_stopCurrentAnimation);
	h.Value(&m_skipIfSameAnimation);
	h.Value(&m_transitionTime);
	h.Value(&m_animSpeed);
	h.Value(&m_layerID);
}