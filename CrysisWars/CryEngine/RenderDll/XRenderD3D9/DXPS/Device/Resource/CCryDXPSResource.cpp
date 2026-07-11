#include "RenderPCH.h"
#include "CCryDXPSResource.hpp"
//#include <assert.h>
#define eCryModule eCryM_Render
#include <CryModuleDefs.h>
#include <platform.h>

unsigned int CCryDXPSResource::Release()
{
	assert("Not implemented yet, needed switch to each resource");
	return 1;//dummy, should be real refcount
}
