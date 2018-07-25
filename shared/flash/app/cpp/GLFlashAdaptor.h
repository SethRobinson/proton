//  ***************************************************************
//  GLFlashAdaptor - Creation date:  06/02/2012
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef GLFlashAdaptor_h__
#define GLFlashAdaptor_h__

#include "PlatformSetup.h"
#define _DLL_EXPORTS
#include "GLES/gl.h"

bool GLFlashAdaptor_Initialize();
GL_API void GL_APIENTRY glVertexPointerEx (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, int vertCount);
//Special version that also sends size, allowing more advanced caching techniques
#endif // GLFlashAdaptor_h__
