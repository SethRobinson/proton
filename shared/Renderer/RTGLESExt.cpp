#include "PlatformPrecomp.h"
#include "RTGLESExt.h"

//you must call InitExtensions on this before using though.  I don't want it in the constructor because I want to be able to control
//the order of initialization

#if defined(_WIN32) && defined C_GL_MODE

PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;

RTGLESExt g_glesExt;

RTGLESExt::RTGLESExt()
{
}

RTGLESExt::~RTGLESExt()
{
}

bool RTGLESExt::InitExtensions()
{
	bool bResult = true;

	LogMsg("Initializing extensions");

	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
	if (!glActiveTextureARB) bResult = false;

	glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) wglGetProcAddress("glClientActiveTextureARB");
	if (!glClientActiveTextureARB) bResult = false;
	return bResult;
}

#endif