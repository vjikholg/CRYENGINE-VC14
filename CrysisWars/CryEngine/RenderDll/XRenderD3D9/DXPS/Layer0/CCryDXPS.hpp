#ifndef __CRYDXPS__
#define __CRYDXPS__

//#define CRY_MM_DEBUG
//#define CRY_MM_DEBUG_ITEMLIST
#define CRY_DXPS_PROFILE


#if defined(CRY_DXPS_PROFILE)
	#define DXPS_PROFILE_FRAME	PROFILE_FRAME
#else
	#define DXPS_PROFILE_FRAME(X)
#endif

#if defined(CRY_MM_DEBUG_ITEMLIST)
	#define CRY_MM_VALIDATE Validate()
	#define CRY_MM_VALIDATE_ITEM(it) it->Validate()
#else
	#define CRY_MM_VALIDATE 
	#define CRY_MM_VALIDATE_ITEM(it)
#endif


#include "../CCryTypes.hpp"


#ifdef 	WIN32
	#include "CCryDXPS_PC.hpp"
#else
	#include "CCryDXPS_PS3.hpp"
#endif

#ifdef CRY_USE_DX9
	#include "CCryDXPSDX9.hpp"
#elif CRY_USE_OGL
	#include "CCryDXPSOGL.hpp"
#elif DPSGL
	#include "CCryDXPSOGL.hpp"
#elif CRY_USE_GCM
	#define CRY_DXPS_CREATE(T,X) tdLayer0::Instance().Create ## T X
	#define CRY_DXPS_NEW(T) tdLayer0::Instance().New<T>()
	#define CRY_DXPS_NEWARRAY(T,S) tdLayer0::Instance().New<T>(S)

	#define CRY_DXPS_RELEASE(P)	tdLayer0::Instance().Release(P)
	#define CRY_DXPS_DELETE(X) tdLayer0::Instance().Delete(X)
	#define CRY_DXPS_DELETEARRAY(X) tdLayer0::Instance().DeleteArray(X)

	#include "CCryDXPSGCMConfig.hpp"
	#include "CCryDXPSGCM.hpp"
#else
	#define CRY_DXPS_CREATE(T,X) new T X
	#define CRY_DXPS_NEW(T) new T
	#define CRY_DXPS_NEWARRAY(T,S) new T[S]

	#define CRY_DXPS_RELEASE(P)	P->Release()
	#define CRY_DXPS_DELETE(X) delete (X)
	#define CRY_DXPS_DELETEARRAY(X) delete[] (X)
#endif

#ifndef CRY_DXPS_CREATE
	#define CRY_DXPS_CREATE(T,X) new T X
	#define CRY_DXPS_NEW(T) new T
	#define CRY_DXPS_NEWARRAY(T,S) new T[S]

	#define CRY_DXPS_RELEASE(P)	delete P->Release()
	#define CRY_DXPS_DELETE(X) delete (X)
	#define CRY_DXPS_DELETEARRAY(X) delete[] (X)
#endif

#include "../CCryDXPSAPtr.hpp"

#endif


