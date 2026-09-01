////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	20/3/2005 - Created by Ivo Herzeg <ivo@crytek.de>
//
//  Contains:
//  loads and initialises CGA objects 
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __AnimObjectLoader_h__
#define __AnimObjectLoader_h__
#pragma once

#include <limits>

struct CInternalSkinningInfo;
class CChunkFileReader;

typedef spline::TCBSpline<Vec3> CControllerTCBVec3;

// Internal description of node.
struct NodeDesc
{
	static constexpr uint16 InvalidJointId = (std::numeric_limits<uint16>::max)();

	NodeDesc()
	{
		active		=	0;
		node_idx	= InvalidJointId;	//index of joint
		parentID	= InvalidJointId;	//if of parent-chunk
		pos_cont_id	= InvalidJointId;	// position controller chunk id
		rot_cont_id	= InvalidJointId;	// rotation controller chunk id
		scl_cont_id	= InvalidJointId;	// scale		controller chunk id
	};

	uint16 active; 
	uint16 node_idx;		//index of joint
	uint16 parentID;
	uint16 pos_cont_id;	// position controller chunk id
	uint16 rot_cont_id;	// rotation controller chunk id
	uint16 scl_cont_id;	// scale		controller chunk id
};



//////////////////////////////////////////////////////////////////////////
// Loads AnimObject from CGF/CAF files.
//////////////////////////////////////////////////////////////////////////
class CryCGALoader
{
public:

	CryCGALoader() {};
	
	// Load animation object from cgf or caf.
	CCharacterModel* LoadNewCGA( const char *geomName, CharacterManager* pManager );

//private:
public:
	bool InitNodes( CInternalSkinningInfo* pSkinningInfo, CCharacterModel* pCGAModel, const char* animFile, const string& strName, bool bMakeNodes, uint32 unique_model_id  );

	// Load all animations for this object.
	void LoadAnimations( const char *cgaFile, CCharacterModel* pCGAModel, uint32 unique_model_id );
	bool LoadAnimationANM( const char *animFile, CCharacterModel* pCGAModel, uint32 unique_model_id );
	uint32 LoadANM ( CCharacterModel* pModel,const char* pFilePath, const char* pAnimName, std::vector<CControllerTCB>& m_LoadCurrAnimation, uint32 unique_model_id   );

	
	std::vector<CControllerTCBVec3> m_CtrlVec3;
	std::vector<spline::TCBAngleAxisSpline> m_CtrlQuat;

	// Array of controllers.
	DynArray<CControllerType> m_arrControllers;

	std::vector<NodeDesc> m_arrChunkNodes;

	// ticks per one max frame.
	int m_ticksPerFrame;
	//! controller ticks per second.
	float m_secsPerTick;
	int m_start;
	int m_end;

	size_t m_DefaultNodeCount;

	//the controllers for CGA are in this array
	std::vector<CControllerTCB> m_NodeAnims;

	// Created animation object
	ModelAnimationHeader m_LoadCurrAnimation;
};

#endif //__AnimObjectLoader_h__
