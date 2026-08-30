#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <platform.h>

#if defined(DIRECT3D9) || defined(DIRECT3D10)
// The legacy renderer uses D3D9 utility interfaces in both PC backends.
#if !defined(CRY_USE_GCM)
#include "d3d9.h"
#include "d3dx9.h"
#endif

#if !defined(XENON)
#if defined(CRY_USE_GCM)
#include "XRenderD3D9/DXPS/CCryDXPSMisc.hpp"
#include "XRenderD3D9/DXPS/CCryDXPSRenderer.hpp"
#include "XRenderD3D9/DXPS/DXPSGI/CCryDXPSGI.hpp"
#else
#include "dxgi.h"
#include "d3d10.h"
#include "d3dx10.h"
#endif
#endif
#elif defined(NULL_RENDERER) && defined(WIN32)
#include "windows.h"
#endif
