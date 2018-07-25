/*
 *  PlatformSetupOSX.h
 *  Created by Seth Robinson on 12/13/2010.
 *  For license info, check the license.txt file that should have come with this.
 *
 */

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
//#include </System/Library/Frameworks/Foundation.framework/Versions/C/Headers/NSUtilities.h>
//#include <Foundation/Foundation.h>
#include <stdint.h>
//#include <MacTypes.h>
#include "OSXUtils.h"
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#define glTexParameterx glTexParameteri
#define glFrustumf glFrustum
#define glOrthof glOrtho
#define glColor4x(r,g,b,a) glColor4f( (float(r)/65536.0f),  (float(g)/65536.0f) , (float(b)/65536.0f), (float(a)/65536.0f));

#ifndef PLATFORM_OSX
    #define PLATFORM_OSX
#endif


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
#ifndef _UINT32
typedef uint32_t uint32;
#define _UINT32
#endif
#ifndef _UINT16
typedef uint16_t uint16;
#define _UINT16
#endif
#ifndef _UINT8
typedef uint8_t uint8;
#define _UINT8
#endif

#ifndef _INT8
typedef int8_t int8;
#define _INT8
#endif
#ifndef _INT32
typedef int32_t int32;
#define _INT32
#endif

#ifndef _INT16
typedef int16_t int16;
#define _INT16
#endif


#if defined(__GNUC__)
#define __int64 long long
#endif



#ifndef _INT64_DEFINED
typedef  __int64            int64;       /* Signed 64 bit value */
#define _INT64_DEFINED
#endif

#ifndef _BYTE
typedef uint8_t byte;
#define _BYTE
#endif



