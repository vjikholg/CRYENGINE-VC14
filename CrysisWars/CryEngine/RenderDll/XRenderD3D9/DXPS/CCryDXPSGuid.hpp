#ifndef __CRYDXPSGUID__
#define __CRYDXPSGUID__


enum ECryGUID
{
	ID3D10Texture1D__GUID,
	ID3D10Texture2D__GUID,
	ID3D10Texture3D__GUID,
	IDXGIFactory__GUID,
	IDXGIDevice__GUID,
	ID3D10Debug__GUID,
};

typedef ECryGUID				CRYIID;


#undef REFGUID
#undef REFIID
#define REFGUID const ECryGUID &
#define REFIID const CRYIID &

#undef __uuidof
#define __uuidof(name) name##__GUID


#endif

