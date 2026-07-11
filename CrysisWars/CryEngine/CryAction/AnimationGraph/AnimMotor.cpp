#include "StdAfx.h"
#include "AnimMotor.h"
#include "ICryAnimation.h"

void CAnimMotor::EnterState( SAnimationStateData& data, bool dueToRollback )
{
	if (m_followAnimation)
	{
		IPhysicalEntity *pPhysEnt = data.pEntity ? data.pEntity->GetPhysics() : NULL;

		if (pPhysEnt)
		{
			pe_player_dynamics pd;
			pPhysEnt->GetParams(&pd);

			m_lastInertia = pd.kInertia;
			m_lastInertiaAccel = pd.kInertiaAccel;

			pd = pe_player_dynamics();
			pd.kInertia = pd.kInertiaAccel = 0.0f;

			pPhysEnt->SetParams(&pd);
		}

		ICharacterInstance * pCharacter =  (data.pEntity) ? data.pEntity->GetCharacter(0) : NULL;

		if (pCharacter)
		{
			ISkeletonAnim* pISkeletonAnim = pCharacter->GetISkeletonAnim();
			ISkeletonPose* pISkeletonPose = pCharacter->GetISkeletonPose();
			pISkeletonAnim->SetAnimationDrivenMotion(1);
			pISkeletonPose->SetFootAnchoring(1);
		}
	}
}

bool CAnimMotor::CanLeaveState( SAnimationStateData& data )
{
	return true;
}

void CAnimMotor::LeaveState( SAnimationStateData& data )
{
	if (m_followAnimation)
	{
		IPhysicalEntity *pPhysEnt = data.pEntity ? data.pEntity->GetPhysics() : NULL;

		if (pPhysEnt)
		{
			pe_player_dynamics pd;
			pd.kInertia = m_lastInertia;
			pd.kInertiaAccel = m_lastInertiaAccel;

			pPhysEnt->SetParams(&pd);
		}

		//ICharacterInstance * pCharacter = (data.pEntity) ? data.pEntity->GetCharacter(0) : NULL;

		//if (pCharacter)
		//	pCharacter->SetTransRot2000(0);
	}
}

const IAnimationStateNodeFactory::Params * CAnimMotor::GetParameters()
{
	static const Params params[] = 
	{
		{true,  "vec3",   "speedVector",  "Velocity",          "0,0,0"},
		{true,  "bool",   "followAnimation",  "Follow animation vector",          "0"},
		{0}
	};
	return params;
}

void CAnimMotor::Update( SAnimationStateData& data )
{
	IPhysicalEntity *pPhysEnt = data.pEntity ? data.pEntity->GetPhysics() : NULL;
	
	if (pPhysEnt)
	{
		pe_action_move aMove;
		aMove.iJump = 0;

		ICharacterInstance *pCharacter = data.pEntity->GetCharacter(0);

		if (m_followAnimation && pCharacter)
		{
			//
			if (pCharacter)
				pCharacter->SetFlags(pCharacter->GetFlags() | CS_FLAG_UPDATE_ALWAYS);

	//		Vec3 localT(pCharacter->GetISkeleton()->GetRelTranslation() / -gEnv->pTimer->GetFrameTime());
			Vec3 localT(pCharacter->GetISkeletonAnim()->GetRelTranslation() / gEnv->pTimer->GetFrameTime());

			aMove.dir = Matrix33(data.pEntity->GetRotation()) * localT;
		}
		else
		{
			aMove.dir = m_speedVec;
		}

		pPhysEnt->Action(&aMove);
	}
}

bool CAnimMotor::Init( const XmlNodeRef& node, IAnimationGraphPtr )
{
	m_speedVec.Set(0,0,0);
	m_followAnimation = 0;

	node->getAttr("speedVector", m_speedVec);
	node->getAttr("followAnimation", m_followAnimation);

	return true;
}

void CAnimMotor::Release()
{
	delete this;
}

IAnimationStateNode * CAnimMotor::Create()
{
	return this;
}

const char * CAnimMotor::GetCategory()
{
	return "Motor";
}

const char * CAnimMotor::GetName()
{
	return "Motor";
}
