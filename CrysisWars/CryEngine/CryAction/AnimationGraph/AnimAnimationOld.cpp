#include "StdAfx.h"
#include "AnimAnimationOld.h"
#include "ICryAnimation.h"

CAnimAnimationOld::CAnimAnimationOld() : IAnimationStateNode(eASNF_Update)
{
}

CAnimAnimationOld::~CAnimAnimationOld()
{
}

const IAnimationStateNodeFactory::Params * CAnimAnimationOld::GetParameters()
{
	static const Params params[] = 
	{
		{true,  "string", "play",         "Animation",          ""},
		{true,  "float",  "blendInTime",  "Blend in time",      "0.3", UpgradeBlendIn},
		{true,  "float",  "blendOutTime", "Blend out time",     "0.3", UpgradeBlendOut},
		{true,  "bool",   "wait",         "Wait for animation", "1",   UpgradeWait},
		{true, "bool",   "ensureInStack", "Ensure animation is in the stack?", "0", UpgradeEnsureInStack},

		{true, "bool",   "loopAnimation", "Loop animation", "1", UpgradeLoopAnimation},
		{true, "bool",   "repeatLastKey", "Repeat last key", "0", UpgradeRepeatLastKey},
		{true, "bool",   "phaseSyncing", "Phase syncing", "1", UpgradePhaseSyncing},
		{true, "bool",   "vtimeWarping", "Vertical time warping", "1", UpgradeVerticalTimeWarping},
		{true, "bool",   "startAfter", "Start after", "0", UpgradeStartAfter},
		{true, "bool",   "ignoreFootplants", "Ignore footplants", "0", UpgradeIgnoreFootplants},
		{true, "bool",   "recursive", "Recursive", "1"},
		{true, "bool",   "aligned", "Aligned", "0", UpgradeAligned},
		{0}
	};
	return params;
}

bool CAnimAnimationOld::UpgradeEnsureInStack( XmlNodeRef node )
{
	node->setAttr("ensureInStack", false);
	return true;
}

bool CAnimAnimationOld::UpgradeWait( XmlNodeRef node )
{
	bool loop = false;
	node->getAttr("loop", loop);
	node->setAttr("wait", !loop);
	return true;
}

bool CAnimAnimationOld::UpgradeBlendIn( XmlNodeRef node )
{
	float blend = 0.3f;
	node->getAttr("blendTime", blend);
	node->setAttr("blendInTime", blend);
	return true;
}

bool CAnimAnimationOld::UpgradeBlendOut( XmlNodeRef node )
{
	float blend;
	node->getAttr("blendTime", blend);
	node->setAttr("blendOutTime", blend);
	return true;
}

bool CAnimAnimationOld::CanLeaveState( SAnimationStateData& data )
{
	if (m_ensureInStack)
	{
		ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);
		if (!pCharacter)
			return true;
		IAnimationSet* pIAnimationSet = pCharacter->GetIAnimationSet();
		ISkeletonAnim* pSkeletonAnim = pCharacter->GetISkeletonAnim();

		// break out early if there are no anims on stack -> we can leave the state!
		if (pSkeletonAnim->GetNumAnimsInFIFO(0) == 0)
			return true;

		CAnimation& animation = pSkeletonAnim->GetAnimFromFIFO(0,0);
		int32 id = animation.m_LMG0.m_nAnimID[0];

		int32 id0 = pIAnimationSet->GetAnimIDByName( m_animation );

		return (id == id0);
	}
	return true;
}

void CAnimAnimationOld::EnterState( SAnimationStateData& data, bool dueToRollback )
{
	//	if (m_animation != "combat_walk_nw_forward_01")
	//		return;

	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);
	if (!pCharacter)
	{
		GameWarning("Entity %s has no character attached", data.pEntity->GetClass()->GetName());
		return;
	}

	/*int numAnims = pCharacter->GetISkeleton()->GetNumAnimsInFIFO(m_layerID);
	if (numAnims>=3)
		return;*/

	IAnimationSet * pAnimSet = pCharacter->GetIAnimationSet();

	m_currentAnimID = pAnimSet->GetAnimIDByName(m_animation);
//	if (m_currentAnimID < 0)
//		GameWarning("%s:cannot find animation %s", data.pEntity->GetName(),m_animation.c_str());

	//same animation? do nothing
	CryLogAlways("Using deprecated AnimAnimation node in animation graph (entity %s)", data.pEntity->GetName());
	
	//old anim-system: removed by Ivo
	/*if (m_currentAnimID != pCharacter->GetCurrentAnimation(m_layerID))
	{
		m_nextAnimation = pAnimSet->GetLength(m_currentAnimID);
		m_completionTime = gEnv->pTimer->GetFrameStartTime() + m_nextAnimation;

		pCharacter->SetLoop( m_animation, m_loopAnimation, m_recursive );

		CryCharAnimationParams p;

		p.fBlendInTime = m_blendTime;
		p.fBlendOutTime = m_blendOutTime;
		p.nLayerID = m_layerID;

		p.nFlags |= m_loopAnimation * CA_LOOP_ANIMATION;
		p.nFlags |= m_repeatLastKey * CA_REPEAT_LAST_KEY;
		p.nFlags |= m_phaseSyncing * CA_PHASE_SYNCHING;
		p.nFlags |= m_vtimeWarping * CA_VTIME_WARPING;
		p.nFlags |= m_startAfter * CA_START_AFTER;
		p.nFlags |= m_ignoreFootplants * CA_IGNORE_FOOTPLANTS;

		p.fTransTime = m_blendTime;

		ISkeleton* pISkeleton	=	pCharacter->GetISkeleton();
		pISkeleton->StartAnimationNew( m_animation.c_str(), NULL, p );

		pISkeleton->ClearMotionMixerArray();
		uint32 numAnimsLayer0 = pISkeleton->GetNumAnimsInFIFO(p.nLayerID);
		if (numAnimsLayer0)
			pISkeleton->PushMotionMixer(p.nLayerID,(uint32)-1,FINALREL,-1,COPYLAYER);

	//	pCharacter->GetISkeleton()->ClearMotionMixerArray();
	//	int numAnims = pCharacter->GetISkeleton()->GetNumAnimsInFIFO(p.nLayerID);
	//	if (numAnims)
	//		pCharacter->GetISkeleton()->PushMotionMixer(p.nLayerID,-1,FINALREL,-1,COPYLAYER);

		pCharacter->SetAnimationSpeedLayer(p.nLayerID,1.0f);
	}*/

}

void CAnimAnimationOld::LeaveState( SAnimationStateData& data )
{
	ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);

	if (!pCharacter)
	{
		GameWarning("Entity %s has no character attached", data.pEntity->GetClass()->GetName());
		return;
	}

	//stop the animation only if its the same animation called by this node, kind of dangerous.
	CryLogAlways("Using deprecated AnimAnimation node in animation graph (entity %s)", data.pEntity->GetName());

}

void CAnimAnimationOld::Update( SAnimationStateData& data )
{
	/*ICharacterInstance * pCharacter = data.pEntity->GetCharacter(0);
	if (pCharacter)
	{

		ISkeleton* pISkeleton = pCharacter->GetISkeleton();
		pISkeleton->ClearMotionMixerArray();

		uint32 numAnimsLayer0 = pISkeleton->GetNumAnimsInFIFO(0);
		if (numAnimsLayer0)
			pISkeleton->PushMotionMixer(0,(uint32)-1,FINALREL,-1,COPYLAYER);

	}*/
}

bool CAnimAnimationOld::Init( const XmlNodeRef& node, IAnimationGraphPtr )
{
	m_animation = node->getAttr("play");
	if (m_animation.empty())
		return false;

	m_currentAnimID = -1;
	m_blendTime = -1;
	m_loopAnimation = false;
	m_recursive = false;
	m_wait = false;
	m_nextAnimation = 0;
	m_completionTime = 0.0f;
	m_layerID = 0;

	node->getAttr("blendTime", m_blendTime);
	node->getAttr("blendOutTime", m_blendOutTime);

	node->getAttr("wait", m_wait);


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

	return true;
}

#undef DECL_UPGRADE_FLAG
#define DECL_UPGRADE_FLAG(param, name, value) \
	bool CAnimAnimationOld::name( XmlNodeRef node ) \
{ \
	node->setAttr(param, value); \
	return true; \
}

bool CAnimAnimationOld::UpgradeLoopAnimation( XmlNodeRef node )
{
	bool loop = false;
	node->getAttr("loop", loop);
	node->setAttr("loopAnimation", loop);
	return true;
}

DECL_UPGRADE_FLAG("repeatLastKey", UpgradeRepeatLastKey, false)
DECL_UPGRADE_FLAG("phaseSyncing", UpgradePhaseSyncing, true)
DECL_UPGRADE_FLAG("vtimeWarping", UpgradeVerticalTimeWarping, true)
DECL_UPGRADE_FLAG("startAfter", UpgradeStartAfter, false)
DECL_UPGRADE_FLAG("ignoreFootplants", UpgradeIgnoreFootplants, false)
DECL_UPGRADE_FLAG("aligned", UpgradeAligned, false)

void CAnimAnimationOld::Release()
{
	delete this;
}

IAnimationStateNode * CAnimAnimationOld::Create()
{
	return this;
}

const char * CAnimAnimationOld::GetCategory()
{
	return "Animation";
}

const char * CAnimAnimationOld::GetName()
{
	return "AnimationOld";
}

void CAnimAnimationOld::GetCompletionTimes( SAnimationStateData& data, CTimeValue start, CTimeValue& hard, CTimeValue& sticky ) 
{ 
	if (m_wait)
		hard = m_completionTime;
	else
		hard = 0.0f;
	if (m_loopAnimation)
		sticky = 0.0f;
	else
		sticky = m_completionTime;
}

void CAnimAnimationOld::SerializeAsFile(bool reading, AG_FILE *file)
{
	SerializeAsFile_NodeBase(reading, file);

	FileSerializationHelper h(reading, file);

	h.StringValue(&m_animation);
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
	h.Value(&m_layerID);
	h.Value(&m_blendTime);
	h.Value(&m_blendOutTime);
	h.Value(&m_wait);
	h.Value(&m_nextAnimation);
	h.Value(&m_completionTime);
}