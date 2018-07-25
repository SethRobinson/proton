#ifndef PlatformSetupBBX_h__
#define PlatformSetupBBX_h__

//need these for the real compile

#include <unistd.h>
#include <stdint.h>

#include "BBXUtils.h"
#include <GLES/gl.h>
#include <stdarg.h>


#ifdef _IRR_STATIC_LIB_
#include "Irrlicht/source/Irrlicht/gles-ext.h"
#endif

typedef GLfloat GLdouble;
#define glClipPlane glClipPlanef

#define M_PI 3.141592f


#ifndef _SINT64
typedef int64_t sint64;
#define _SINT64
#endif

#ifndef _UINT64
typedef uint64_t uint64;
#define _UINT64
#endif

#ifndef _SINT32
typedef int32_t sint32;
#define _SINT32
#endif
#ifndef _SINT16
typedef int16_t sint16;
#define _SINT16
#endif
#ifndef _SINT8
typedef int8_t sint8;
#define _SINT8
#endif


#ifndef _UINT32S

typedef uint32_t uint32;
#define _UINT32S
#endif

#ifndef _UINT16
typedef uint16_t uint16;
#define _UINT16
#endif
#ifndef _UINT8
typedef uint8_t uint8;
#define _UINT8
#endif

#ifndef _INT32S
typedef int32_t int32;
#define _INT32S
#endif

#ifndef _INT16
typedef int16_t int16;
#define _INT16
#endif


#if defined(__GNUC__)
#define __int64 long long
#endif

#define stricmp strcasecmp

#ifndef _INT64_DEFINED
typedef  __int64            int64;       /* Signed 64 bit value */
#define _INT64_DEFINED
#endif

#ifndef _BYTE
typedef uint8_t byte;
#define _BYTE
#endif
#endif // PlatformSetupWebOS_h__
