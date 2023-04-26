#ifndef PlatformSetupPSP2_h__
#define PlatformSetupPSP2_h__

#include <psp2/kernel/clib.h>
#include <psp2/types.h>

#include <vitaGL.h> // GLdouble, etc...

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef _SINT64
typedef SceInt64 sint64;
#define _SINT64
#endif

#ifndef _UINT64
typedef SceUInt64 uint64;
#define _UINT64
#endif

#ifndef _SINT32
typedef SceInt32 sint32;
#define _SINT32
#endif

#ifndef _SINT16
typedef SceInt16 sint16;
#define _SINT16
#endif

#ifndef _SINT8
typedef SceChar8 sint8;
#define _SINT8
#endif

#ifndef _UINT32
typedef SceUInt32 uint32;
#define _UINT32
#endif

#ifndef _UINT16
typedef SceUInt16 uint16;
#define _UINT16
#endif

#ifndef _UINT8
typedef SceUChar8 uint8;
#define _UINT8
#endif

#ifndef _INT32
typedef SceInt32 int32;
#define _INT32
#endif

#ifndef _INT16
typedef SceInt16 int16;
#define _INT16
#endif

#ifndef _INT8_DEFINED
typedef int8_t int8; /* Signed 8  bit value */
#define _INT8_DEFINED
#endif

#ifndef _INT64_DEFINED
typedef  SceInt64 int64; /* Signed 64 bit value */
#define _INT64_DEFINED
#endif

#ifndef _BYTE
typedef uint8_t byte;
#define _BYTE
#endif

#endif // PlatformSetupPSP2_h__