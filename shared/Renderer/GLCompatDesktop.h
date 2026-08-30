//  ***************************************************************
//  GLCompatDesktop - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

//Proton's renderer is written against the GLES 1.1 API names.  When building
//against desktop OpenGL (C_GL_MODE on Windows/Linux/OSX/HTML5, and the OSMesa
//console mode), this maps the GLES-only names onto their desktop equivalents.
//This used to be copy-pasted into each platform's PlatformSetup*.h; it lives
//here so the shader-pipeline work only has one place to change.
//
//The matching reverse mapping (desktop-style names for real GLES 1.x targets)
//is in GLCompatGLES.h.

#ifndef GLCompatDesktop_h__
#define GLCompatDesktop_h__

#define glTexParameterx glTexParameteri
#define glFrustumf glFrustum
#define glOrthof glOrtho
#define glColor4x(r,g,b,a) glColor4f( (float(r)/65536.0f),  (float(g)/65536.0f) , (float(b)/65536.0f), (float(a)/65536.0f));

//OSX's <OpenGL/gl.h> declares glActiveTexture/glClientActiveTexture natively
//(it defines RT_GL_HAS_ACTIVE_TEXTURE before including this); the GL 1.1-era
//headers used on the other desktop targets don't, so there the names route to
//the ARB versions (resolved at runtime on Windows, see RTGLESExt)
#ifndef RT_GL_HAS_ACTIVE_TEXTURE
	#define glActiveTexture glActiveTextureARB
	#define glClientActiveTexture glClientActiveTextureARB
#endif

#endif // GLCompatDesktop_h__
