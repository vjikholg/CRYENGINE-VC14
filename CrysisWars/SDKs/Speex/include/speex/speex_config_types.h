#ifndef __SPEEX_TYPES_H__
#define __SPEEX_TYPES_H__

#if defined(unix) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <stdint.h>
typedef int16_t spx_int16_t;
typedef uint16_t spx_uint16_t;
typedef int32_t spx_int32_t;
typedef uint32_t spx_uint32_t;
#elif defined(_MSC_VER)
typedef __int16 spx_int16_t;
typedef unsigned __int16 spx_uint16_t;
typedef __int32 spx_int32_t;
typedef unsigned __int32 spx_uint32_t;
#else
#error Write me.
#endif

#endif
