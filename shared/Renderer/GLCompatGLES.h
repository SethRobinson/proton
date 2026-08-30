//  ***************************************************************
//  GLCompatGLES - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

//The reverse of GLCompatDesktop.h: a couple of desktop-GL style names Proton
//code uses that the real GLES 1.x headers (iOS/Android/Raspberry Pi/PowerVR
//emulator) don't have.  Include after the platform's GLES headers.

#ifndef GLCompatGLES_h__
#define GLCompatGLES_h__

typedef GLfloat GLdouble;
#define glClipPlane glClipPlanef

#endif // GLCompatGLES_h__
