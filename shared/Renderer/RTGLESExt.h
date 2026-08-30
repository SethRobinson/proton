//  ***************************************************************
//  RTGLESExt - Creation date: 12/07/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef RTGLESExt_h__
#define RTGLESExt_h__

//#include "Irrlicht\source\Irrlicht\wglext.h"

#if defined(_WIN32) && defined C_GL_MODE

	#define _IRR_OGLES1_USE_EXTPOINTER_



extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;

#endif
class RTGLESExt
{
public:
	RTGLESExt();
	virtual ~RTGLESExt();

	bool InitExtensions();



};

extern RTGLESExt g_glesExt;
#endif // RTGLESExt_h__
