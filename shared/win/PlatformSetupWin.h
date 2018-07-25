#pragma once
#include <winsock2.h>
#include <ws2tcpip.h> //needed for ipv6 stuff
#include <windows.h>
#include <TCHAR.h>
#include "time.h"

#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS
#endif


#define snprintf _snprintf

#ifndef _CONSOLE
#ifdef C_GL_MODE
#include "Renderer/GL/gl.h"

//help with compatibility so I can use the GL ES calls with normal GL

#define glTexParameterx glTexParameteri
#define glFrustumf glFrustum
#define glOrthof glOrtho
#define glColor4x(r,g,b,a) glColor4f( (float(r)/65536.0f),  (float(g)/65536.0f) , (float(b)/65536.0f), (float(a)/65536.0f));
#define glActiveTexture glActiveTextureARB
#define glClientActiveTexture glClientActiveTextureARB


#ifdef _IRR_STATIC_LIB_
#include <Irrlicht/source/Irrlicht/glext.h>
#endif
#include "Renderer/RTGLESExt.h"



#else

#ifdef RT_WEBOS
#include <GLES/gl.h>
#ifdef _IRR_STATIC_LIB_
#include "Irrlicht/source/Irrlicht/gles-ext.h"
#else
#include <GLES/glext.h>
#endif

#else

#ifdef RT_FLASH_TEST
	//Allow us to use our fake flash functions from GLFlashAdaptor.cpp, can help with debugging certain things
	//even though we can't really talk to Flash from MSVC++
	#define _DLL_EXPORTS

	#define inline_as3(...)  ((void)0)
#endif

#include <GLES/egl.h>
#include <GLES/gl.h>
#include "Renderer/GLES/glext.h"
#endif

#ifdef _IRR_STATIC_LIB_
	#include "Irrlicht/source/Irrlicht/gles-ext.h"
#endif

typedef GLfloat GLdouble;
#define glClipPlane glClipPlanef
#endif

#else
//CONSOLE mode
#ifdef RT_USING_OSMESA 
//ogl in console mode... useful for testing linux headless glrendering in windows
#include "Renderer/GL/gl.h"

	//help with compatibility so I can use the GL ES calls with normal GL

	#define glTexParameterx glTexParameteri
	#define glFrustumf glFrustum
	#define glOrthof glOrtho
	#define glColor4x(r,g,b,a) glColor4f( (float(r)/65536.0f),  (float(g)/65536.0f) , (float(b)/65536.0f), (float(a)/65536.0f));
	#define glActiveTexture glActiveTextureARB
	#define glClientActiveTexture glClientActiveTextureARB

#endif

#endif

#ifndef M_PI
#define M_PI 3.141592f
#endif
/* The following definitions are the same across platforms.  This first
** group are the sanctioned types.
*/
#ifndef _BOOLEAN_DEFINED
typedef  unsigned char      boolean;     /* Boolean value type. */
#define _BOOLEAN_DEFINED
#endif

#ifndef _UINT32_DEFINED
typedef  unsigned long int  uint32;      /* Unsigned 32 bit value */
#define _UINT32_DEFINED
#endif

#ifndef _UINT16_DEFINED
typedef  unsigned short     uint16;      /* Unsigned 16 bit value */
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
#define _UINT8_DEFINED
#endif

#ifndef _INT32_DEFINED
typedef  signed long int    int32;       /* Signed 32 bit value */

//for mysql
#define HAVE_INT32

#define _INT32_DEFINED
#endif

#ifndef _INT16_DEFINED
typedef  signed short       int16;       /* Signed 16 bit value */
#define _INT16_DEFINED
#endif

#ifndef _INT8_DEFINED
typedef  signed char        int8;        /* Signed 8  bit value */
#define _INT8_DEFINED
#endif

#if defined(__GNUC__)
#define __int64 long long
#endif

#ifndef _UINT64_DEFINED
typedef  unsigned __int64   uint64;      /* Unsigned 64 bit value */
#define _UINT64_DEFINED
#endif

#ifndef _INT64_DEFINED
typedef  __int64            int64;       /* Signed 64 bit value */
#define _INT64_DEFINED
#endif

#ifndef _SINT64_DEFINED
typedef  __int64            sint64;       /* Signed 64 bit value */
#define _SINT64_DEFINED
#endif

#ifndef _BYTE_DEFINED
typedef  unsigned char      byte;        /* byte type */
#define  _BYTE_DEFINED
#endif


#include "WinUtils.h"

#ifdef RT_FLASH_TEST
#include "flash/app/cpp/GLFlashAdaptor.h"
#endif
