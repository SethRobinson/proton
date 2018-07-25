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



/*

	void extGlBindFramebuffer(GLenum target, GLuint framebuffer)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlBindFramebufferOES)
			pGlBindFramebufferOES(target, framebuffer);
#elif defined(GL_OES_framebuffer_object)
		glBindFramebufferOES(target, framebuffer);
#endif
	}


	void extGlDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlDeleteFramebuffersOES)
			pGlDeleteFramebuffersOES(n, framebuffers);
#elif defined(GL_OES_framebuffer_object)
		glDeleteFramebuffersOES(n, framebuffers);
#endif

	}

	void extGlGenFramebuffers(GLsizei n, GLuint *framebuffers)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlGenFramebuffersOES)
			pGlGenFramebuffersOES(n, framebuffers);
#elif defined(GL_OES_framebuffer_object)
		glGenFramebuffersOES(n, framebuffers);
#endif

	}

	GLenum extGlCheckFramebufferStatus(GLenum target)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlCheckFramebufferStatusOES)
			return pGlCheckFramebufferStatusOES(target);
		else
			return 0;
#elif defined(GL_OES_framebuffer_object)
		return glCheckFramebufferStatusOES(target);
#endif

		
	}

	void extGlFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlFramebufferTexture2DOES)
			pGlFramebufferTexture2DOES(target, attachment, textarget, texture, level);
#elif defined(GL_OES_framebuffer_object)
		glFramebufferTexture2DOES(target, attachment, textarget, texture, level);
#endif

	}

	void extGlBindRenderbuffer(GLenum target, GLuint renderbuffer)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlBindRenderbufferOES)
			pGlBindRenderbufferOES(target, renderbuffer);
#elif defined(GL_OES_framebuffer_object)
		glBindRenderbufferOES(target, renderbuffer);
#endif

	}

	void extGlDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlDeleteRenderbuffersOES)
			pGlDeleteRenderbuffersOES(n, renderbuffers);
#elif defined(GL_OES_framebuffer_object)
		glDeleteRenderbuffersOES(n, renderbuffers);
#endif

	}

	void extGlGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlGenRenderbuffersOES)
			pGlGenRenderbuffersOES(n, renderbuffers);
#elif defined(GL_OES_framebuffer_object)
		glGenRenderbuffersOES(n, renderbuffers);
#endif

	}

	void extGlRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlRenderbufferStorageOES)
			pGlRenderbufferStorageOES(target, internalformat, width, height);
#elif defined(GL_OES_framebuffer_object)
		glRenderbufferStorageOES(target, internalformat, width, height);
#endif

	}

	void extGlFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
	{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
		if (pGlFramebufferRenderbufferOES)
			pGlFramebufferRenderbufferOES(target, attachment, renderbuffertarget, renderbuffer);
#elif defined(GL_OES_framebuffer_object)
		glFramebufferRenderbufferOES(target, attachment, renderbuffertarget, renderbuffer);
#endif

	}

	void extGlDrawTex(GLfloat X, GLfloat Y, GLfloat Z, GLfloat W, GLfloat H)
	{
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
		if (pGlDrawTexfOES)
			pGlDrawTexfOES(X, Y, Z, W, H);
#elif defined(GL_OES_draw_texture)
		glDrawTexfOES(X, Y, Z, W, H);
#endif


	}

	void extGlDrawTex(GLint X, GLint Y, GLint Z, GLint W, GLint H)
	{
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
		if (pGlDrawTexiOES)
			pGlDrawTexiOES(X, Y, Z, W, H);
#elif defined(GL_OES_draw_texture)
		glDrawTexiOES(X, Y, Z, W, H);
#endif


	}

	void extGlDrawTex(GLfloat* coords)
	{
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
		if (pGlDrawTexfvOES)
			pGlDrawTexfvOES(coords);
#elif defined(GL_OES_draw_texture)
		glDrawTexfvOES(coords);
#else
		os::Printer::log("glDrawTexture not supported", ELL_ERROR);
#endif

	}

	void extGlDrawTex(GLint* coords)
	{
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
		if (pGlDrawTexivOES)
			pGlDrawTexivOES(coords);
#elif defined(GL_OES_draw_texture)
		glDrawTexivOES(coords);
#endif
	}


private:


	typedef void (APIENTRY * PFNGLDRAWTEXIOES) (GLint x, GLint y, GLint z, GLint width, GLint height);
	typedef void (APIENTRY * PFNGLDRAWTEXIVOES) (const GLint* coords);
	typedef void (APIENTRY * PFNGLDRAWTEXFOES) (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
	typedef void (APIENTRY * PFNGLDRAWTEXFVOES) (const GLfloat* coords);
	typedef GLboolean (GL_APIENTRYP PFNGLISRENDERBUFFEROES) (GLuint renderbuffer);
	typedef void (GL_APIENTRYP PFNGLBINDRENDERBUFFEROES) (GLenum target, GLuint renderbuffer);
	typedef void (GL_APIENTRYP PFNGLDELETERENDERBUFFERSOES) (GLsizei n, const GLuint* renderbuffers);
	typedef void (GL_APIENTRYP PFNGLGENRENDERBUFFERSOES) (GLsizei n, GLuint* renderbuffers);
	typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEOES) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	typedef void (GL_APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVOES) (GLenum target, GLenum pname, GLint* params);
	typedef GLboolean (GL_APIENTRYP PFNGLISFRAMEBUFFEROES) (GLuint framebuffer);
	typedef void (GL_APIENTRYP PFNGLBINDFRAMEBUFFEROES) (GLenum target, GLuint framebuffer);
	typedef void (GL_APIENTRYP PFNGLDELETEFRAMEBUFFERSOES) (GLsizei n, const GLuint* framebuffers);
	typedef void (GL_APIENTRYP PFNGLGENFRAMEBUFFERSOES) (GLsizei n, GLuint* framebuffers);
	typedef GLenum (GL_APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSOES) (GLenum target);
	typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEROES) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DOES) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOES) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
	typedef void (GL_APIENTRYP PFNGLGENERATEMIPMAPOES) (GLenum target);

	PFNGLDRAWTEXIOES pGlDrawTexiOES;
	PFNGLDRAWTEXFOES pGlDrawTexfOES;
	PFNGLDRAWTEXIVOES pGlDrawTexivOES;
	PFNGLDRAWTEXFVOES pGlDrawTexfvOES;
	PFNGLBINDRENDERBUFFEROES pGlBindRenderbufferOES;
	PFNGLDELETERENDERBUFFERSOES pGlDeleteRenderbuffersOES;
	PFNGLGENRENDERBUFFERSOES pGlGenRenderbuffersOES;
	PFNGLRENDERBUFFERSTORAGEOES pGlRenderbufferStorageOES;
	PFNGLBINDFRAMEBUFFEROES pGlBindFramebufferOES;
	PFNGLDELETEFRAMEBUFFERSOES pGlDeleteFramebuffersOES;
	PFNGLGENFRAMEBUFFERSOES pGlGenFramebuffersOES;
	PFNGLCHECKFRAMEBUFFERSTATUSOES pGlCheckFramebufferStatusOES;
	PFNGLFRAMEBUFFERRENDERBUFFEROES pGlFramebufferRenderbufferOES;
	PFNGLFRAMEBUFFERTEXTURE2DOES pGlFramebufferTexture2DOES;
	PFNGLGENERATEMIPMAPOES pGlGenerateMipMapOES;
*/
};

extern RTGLESExt g_glesExt;
#endif // RTGLESExt_h__