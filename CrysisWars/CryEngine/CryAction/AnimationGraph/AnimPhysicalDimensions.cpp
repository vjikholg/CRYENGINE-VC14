#include "StdAfx.h"
#include "AnimPhysicalDimensions.h"

void CAnimPhysicalDimensionsFactory::EnterState( SAnimationStateData& data, bool dueToRollback )
{
}

void CAnimPhysicalDimensionsFactory::LeaveState( SAnimationStateData& data )
{
}

bool CAnimPhysicalDimensionsFactory::Init( const XmlNodeRef& node, IAnimationGraphPtr )
{
	return true;
}

void CAnimPhysicalDimensionsFactory::Release()
{
	delete this;
}

const IAnimationStateNodeFactory::Params * CAnimPhysicalDimensionsFactory::GetParameters()
{
	static const Params params[] = 
	{
		{0}
	};
	return params;
}

bool CAnimPhysicalDimensionsFactory::CanLeaveState( SAnimationStateData& data )
{
	return true;
}

IAnimationStateNode * CAnimPhysicalDimensionsFactory::Create()
{
	return this;
}

const char * CAnimPhysicalDimensionsFactory::GetCategory()
{
	return "PhysicalParams";
}

const char * CAnimPhysicalDimensionsFactory::GetName()
{
	return "PhysicalDimensions";
}
