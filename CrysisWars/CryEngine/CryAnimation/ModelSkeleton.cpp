////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	10/9/2004 - Created by Ivo Herzeg <ivo@crytek.de>
//
//  Contains:
//  default-skeleton of the model   
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ModelSkeleton.h"
#include <StlAlgorithms.h>
#include <StlContainerUtils.h>
#include "StringUtils.h"





CModelJoint::~CModelJoint()
{
	IPhysicalWorld *pIPhysicalWorld = g_pIPhysicalWorld;
	IGeomManager* pPhysGeomManager = pIPhysicalWorld?pIPhysicalWorld->GetGeomManager():NULL;

	for(int nLod=0; nLod<2; nLod++)
	{
		if (m_PhysInfo[nLod].pPhysGeom)
		{
			if (pPhysGeomManager)
			{
				pPhysGeomManager->UnregisterGeometry(m_PhysInfo[nLod].pPhysGeom);
			}
			else
				CryWarning(VALIDATOR_MODULE_ANIMATION,VALIDATOR_WARNING,"todo: delete bones phys");
		}
	}
}



// scales the bone with the given multiplier
void CModelJoint::scale (f32 fScale)
{
	assert(0);
}



size_t CModelJoint::SizeOfThis() const
{
	unsigned nSize = sizeof(CModelJoint);
//	nSize += sizeofVector(m_arrControllersMJoint);
	return nSize;
}





//////////////////////////////////////////////////////////////////////////
// Updates the given lod level bone physics info from the bones found in
// the given chunk.
// THis is required to update the dead body physics info from the lod1 geometry
// without rebuilding the new bone structure. If the lod1 has another bone structure,
// then the bones are mapped to the lod0 ones using controller ids. Matching bones'
// physics info is updated
//////////////////////////////////////////////////////////////////////////
void CModelJoint::UpdateHierarchyPhysics( DynArray<BONE_ENTITY> arrBoneEntities, int nLod )
{

	UnsignedToCryBoneMap mapCtrlId;
	AddHierarchyToControllerIdMap(mapCtrlId);

	uint32 numBoneEntities = arrBoneEntities.size();

	// update physics for each bone entity
	for (uint32 i=0; i<numBoneEntities; i++)
	{
		CModelJoint* pBone = stl::find_in_map(mapCtrlId, arrBoneEntities[i].ControllerID, (CModelJoint*)NULL);
		if (pBone)
		{
			if (nLod==1)
				CopyPhysInfo(pBone->m_PhysInfo[nLod], arrBoneEntities[i].phys);

			int nFlags = 0;
			if (!arrBoneEntities[i].prop[0])
			{
				nFlags = joint_no_gravity|joint_isolated_accelerations;
			}
			else
			{
				if (!CryStringUtils::strnstr(arrBoneEntities[i].prop,"gravity", sizeof(arrBoneEntities[i].prop)))
					nFlags |= joint_no_gravity;
				if (!CryStringUtils::strnstr(arrBoneEntities[i].prop,"active_phys",sizeof(arrBoneEntities[i].prop)))
					nFlags |= joint_isolated_accelerations;
			}
			(pBone->m_PhysInfo[nLod].flags &= ~(joint_no_gravity|joint_isolated_accelerations)) |= nFlags;
		}

	}

}


//////////////////////////////////////////////////////////////////////////
// Updates the given lod level bone physics info from the bones found in
// the given chunk.
// THis is required to update the dead body physics info from the lod1 geometry
// without rebuilding the new bone structure. If the lod1 has another bone structure,
// then the bones are mapped to the lod0 ones using controller ids. Matching bones'
// physics info is updated
void CModelJoint::UpdateHierarchyPhysics (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize, int nLodLevel)
{
	UnsignedToCryBoneMap mapCtrlId;
	AddHierarchyToControllerIdMap(mapCtrlId);

	// the first bone entity
	const BONE_ENTITY* pBoneEntity = (const BONE_ENTITY*)(pChunk+1);
	// the actual end of the chunk
	const BONE_ENTITY* pBoneEntityEnd = (const BONE_ENTITY*)(((const char*)pChunk)+nChunkSize);

	// if you get this assert, it means the lod 1 file is corrupted
	if (pBoneEntity + pChunk->nBones > pBoneEntityEnd)
	{
		assert (0);
		return;
	}

	// update physics for each bone entity
	for (; pBoneEntity < pBoneEntityEnd; ++pBoneEntity)
	{
		CModelJoint* pBone = stl::find_in_map(mapCtrlId, pBoneEntity->ControllerID, (CModelJoint*)NULL);
		if (pBone)
		{
			//pBone->UpdatePhysics(*pBoneEntity, nLodLevel);

	//		assert (nLod >= 0 && nLod < SIZEOF_ARRAY(m_PhysInfo));
			CopyPhysInfo(pBone->m_PhysInfo[nLodLevel], pBoneEntity->phys);

			int nFlags = 0;
			if (!pBoneEntity->prop[0])
			{
				nFlags = joint_no_gravity|joint_isolated_accelerations;
			}
			else
			{
				if (!CryStringUtils::strnstr(pBoneEntity->prop,"gravity", sizeof(pBoneEntity->prop)))
					nFlags |= joint_no_gravity;

				if (!CryStringUtils::strnstr(pBoneEntity->prop,"active_phys",sizeof(pBoneEntity->prop)))
					nFlags |= joint_isolated_accelerations;
			}

			(pBone->m_PhysInfo[nLodLevel].flags &= ~(joint_no_gravity|joint_isolated_accelerations)) |= nFlags;


		}
	}
}

//////////////////////////////////////////////////////////////////////////
// adds this bone and all its children to the given map controller id-> bone ptr
//////////////////////////////////////////////////////////////////////////
void CModelJoint::AddHierarchyToControllerIdMap (UnsignedToCryBoneMap& mapControllerIdToCryBone)
{
	mapControllerIdToCryBone.insert (UnsignedToCryBoneMap::value_type(m_nJointCRC32, this));

	uint32 numChilds = numChildren();
	for (uint32 i=0; i<numChilds; i++)
		getChild(i)->AddHierarchyToControllerIdMap(mapControllerIdToCryBone);

}

//! Performs post-initialization after bone-mesh chunks have been registered with the physics system.
//! Resolves the serialized nPhysGeomId and stores the resulting runtime geometry in pPhysGeom.
//! RETURNS:
//    true if the corresponding physical geometry object has been found
//!	NOTE:
//!	The entries of the map that were used are deleted
bool CModelJoint::PostInitPhysGeom (ChunkIdToPhysGeomMap& mapChunkIdToPhysGeom, int nLodLevel, bool bAllowRopePhys)
{
	BONE_PHYSICS& physics = m_PhysInfo[nLodLevel];
	if (physics.pPhysGeom)
		return true;

	if (physics.nPhysGeomId <= 0)
		return false;

	ChunkIdToPhysGeomMap::iterator it = mapChunkIdToPhysGeom.find(static_cast<uint32>(physics.nPhysGeomId));
	if (it != mapChunkIdToPhysGeom.end() && (bAllowRopePhys || strnicmp(GetJointName(),"rope",4))) 
	{
		physics.pPhysGeom = it->second;
		mapChunkIdToPhysGeom.erase (it);
		return true;
	} 
	else
	{
		return false;
	}
}

void CModelJoint::PostInitialize()
{
	m_qDefaultRelPhysParent[0].SetIdentity();
	m_qDefaultRelPhysParent[1].SetIdentity();

	if (getParent())
	{
		CModelJoint *pParent,*pPhysParent;
		for(int nLod=0; nLod<2; nLod++)
		{
			for(pParent=pPhysParent=getParent(); pPhysParent && !pPhysParent->m_PhysInfo[nLod].pPhysGeom; pPhysParent=pPhysParent->getParent());
			if (pPhysParent) 
				m_qDefaultRelPhysParent[nLod] = !pParent->m_DefaultAbsPose.q * pPhysParent->m_DefaultAbsPose.q;
			else 
				m_qDefaultRelPhysParent[nLod].SetIdentity();
		}
	}

#if (EXTREME_TEST==1)
	uint32 valid0 = m_qDefaultRelPhysParent[0].IsValid();
	if (valid0==0)
		CryError("CryAnimation: m_qRelPhysParent[0] is invalid in PostInitialize" );

	uint32 valid1 = m_qDefaultRelPhysParent[1].IsValid();
	if (valid1==0)
		CryError("CryAnimation: m_qRelPhysParent[1] is invalid in PostInitialize" );
#endif

}


//---------------------------------------------------------------------------------
//---      for certain IK-operation we need special bone-configurations      ------
//---------------------------------------------------------------------------------
void CModelSkeleton::SetIKJointID()
{
	const size_t numJoints	= m_arrModelJoints.size();
	for (uint32 i=0; i<numJoints; i++)
	{
		const char * BoneName =  m_arrModelJoints[i].GetJointName();
		if (0 == stricmp(BoneName,"Bip01"))
			m_IdxArray[eIM_BipIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 Pelvis"))
			m_IdxArray[eIM_PelvisIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 Spine"))
			m_IdxArray[eIM_Spine0Idx] = i;  
		if (0 == stricmp(BoneName,"Bip01 Spine1"))
			m_IdxArray[eIM_Spine1Idx] = i; 	
		if (0 == stricmp(BoneName,"Bip01 Spine2"))
			m_IdxArray[eIM_Spine2Idx] = i;
		if (0 == stricmp(BoneName,"Bip01 Spine3"))
			m_IdxArray[eIM_Spine3Idx] = i; 	
		if (0 == stricmp(BoneName,"Bip01 Neck"))
			m_IdxArray[eIM_NeckIdx] = i; 
		if (0 == stricmp(BoneName,"Bip01 Head"))
			m_IdxArray[eIM_HeadIdx] = i;
		if (0 == stricmp(BoneName,"eye_left_bone"))
			m_IdxArray[eIM_LeftEyeIdx] = i;
		if (0 == stricmp(BoneName,"eye_right_bone"))
			m_IdxArray[eIM_RightEyeIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R Clavicle"))
			m_IdxArray[eIM_RClavicleIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R UpperArm"))
			m_IdxArray[eIM_RUpperArmIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R ForeArm"))
			m_IdxArray[eIM_RForeArmIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R Hand"))
			m_IdxArray[eIM_RHandIdx] = i;
		if (0 == stricmp(BoneName,"weapon_bone"))
		{
			m_IdxArray[eIM_WeaponBoneIdx] = i;
			//enable COOL re-rigging for weapon bone
			//m_arrModelJoints[i].m_DefaultRelPose.q.SetRotationZ(-gf_PI/2);
		}


		if (0 == stricmp(BoneName,"Bip01 L Clavicle"))
			m_IdxArray[eIM_LClavicleIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L UpperArm"))
			m_IdxArray[eIM_LUpperArmIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L ForeArm"))
			m_IdxArray[eIM_LForeArmIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L Hand"))
			m_IdxArray[eIM_LHandIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L Finger2"))
			m_IdxArray[eIM_LHandFinger2Idx] = i;

		//	if (0 == stricmp(BoneName,"alt_weapon_bone01"))
		//		int dsdsds=0;

		if (0 == stricmp(BoneName,"Bip01 L Thigh"))
			m_IdxArray[eIM_LThighIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L knee"))
			m_IdxArray[eIM_LKneeIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L knee_end"))
			m_IdxArray[eIM_LKneeEndIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L Calf"))
			m_IdxArray[eIM_LCalfIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L Foot"))
			m_IdxArray[eIM_LFootIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L Heel"))
			m_IdxArray[eIM_LHeelIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 L Toe0"))
			m_IdxArray[eIM_LToe0Idx] = i;
		if (0 == stricmp(BoneName,"Bip01 L Toe0Nub"))
			m_IdxArray[eIM_LToe0NubIdx] = i;

		if (0 == stricmp(BoneName,"Bip01 R Thigh"))
			m_IdxArray[eIM_RThighIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R knee"))
			m_IdxArray[eIM_RKneeIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R knee_end"))
			m_IdxArray[eIM_RKneeEndIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R Calf"))
			m_IdxArray[eIM_RCalfIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R Foot"))
			m_IdxArray[eIM_RFootIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R Heel"))
			m_IdxArray[eIM_RHeelIdx] = i;
		if (0 == stricmp(BoneName,"Bip01 R Toe0"))
			m_IdxArray[eIM_RToe0Idx] = i;
		if (0 == stricmp(BoneName,"Bip01 R Toe0Nub"))
			m_IdxArray[eIM_RToe0NubIdx] = i;


		//if both bone-names are in the skeleton, then we assume this is the Hunter
		if (0 == stricmp(BoneName,"root"))
			m_IdxArray[eIM_HunterRootIdx] = i;

		if (0 == stricmp(BoneName,"frontLegLeft01"))
			m_IdxArray[eIM_HunterFrontLegL01Idx] = i;
		if (0 == stricmp(BoneName,"frontLegLeft12"))
			m_IdxArray[eIM_HunterFrontLegL12Idx] = i;

		if (0 == stricmp(BoneName,"frontLegRight01"))
			m_IdxArray[eIM_HunterFrontLegR01Idx] = i;
		if (0 == stricmp(BoneName,"frontLegRight12"))
			m_IdxArray[eIM_HunterFrontLegR12Idx] = i;

		if (0 == stricmp(BoneName,"backLegLeft02"))
			m_IdxArray[eIM_HunterBackLegL01Idx] = i;
		if (0 == stricmp(BoneName,"backLegLeft13"))
			m_IdxArray[eIM_HunterBackLegL13Idx] = i;

		if (0 == stricmp(BoneName,"backLegRight02"))
			m_IdxArray[eIM_HunterBackLegR01Idx] = i;
		if (0 == stricmp(BoneName,"backLegRight13"))
			m_IdxArray[eIM_HunterBackLegR13Idx] =i;

	}
}

bool CModelSkeleton::IsHuman()
{
	bool HasLegs = true;

	if (m_IdxArray[eIM_RHeelIdx] == -1) HasLegs = false;
	else
		if (m_IdxArray[eIM_LHeelIdx] == -1) HasLegs = false;
		else
			if (m_IdxArray[eIM_RToe0NubIdx]	== -1) HasLegs = false;
			else
				if (m_IdxArray[eIM_LToe0NubIdx]	== -1) HasLegs = false;

	return HasLegs;
}
