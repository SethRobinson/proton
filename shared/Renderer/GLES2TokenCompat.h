//  ***************************************************************
//  GLES2TokenCompat - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

//Fixed-function GL *enum tokens* for builds that use pure GLES2 headers
//(RT_SHADER_PIPELINE_ONLY, e.g. the WebGL build).  The functions that took
//these are gone: the tokens survive only as keys consumed by the
//ShaderPipeline's own state tracking (rtMatrixMode, rtEnable, client array
//enables, glLightfv params...), so the values just need to match the classic
//gl.h ones.

#ifndef GLES2TokenCompat_h__
#define GLES2TokenCompat_h__

#ifndef GL_MODELVIEW
	#define GL_MODELVIEW 0x1700
	#define GL_PROJECTION 0x1701
#endif
#ifndef GL_MODELVIEW_MATRIX
	#define GL_MODELVIEW_MATRIX 0x0BA6
	#define GL_PROJECTION_MATRIX 0x0BA7
#endif
#ifndef GL_VERTEX_ARRAY
	#define GL_VERTEX_ARRAY 0x8074
	#define GL_NORMAL_ARRAY 0x8075
	#define GL_COLOR_ARRAY 0x8076
	#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_CLIP_PLANE0
	#define GL_CLIP_PLANE0 0x3000
#endif
#ifndef GL_ALPHA_TEST
	#define GL_ALPHA_TEST 0x0BC0
#endif
#ifndef GL_LIGHTING
	#define GL_LIGHTING 0x0B50
	#define GL_LIGHT0 0x4000
	#define GL_COLOR_MATERIAL 0x0B57
	#define GL_NORMALIZE 0x0BA1
	#define GL_FOG 0x0B60
	#define GL_POSITION 0x1203
	#define GL_AMBIENT 0x1200
	#define GL_DIFFUSE 0x1201
#endif
#ifndef GL_LINE_SMOOTH
	#define GL_LINE_SMOOTH 0x0B20
	#define GL_LINE_SMOOTH_HINT 0x0C52
#endif
#ifndef GL_GENERATE_MIPMAP
	#define GL_GENERATE_MIPMAP 0x8191
#endif
#ifndef GL_SMOOTH
	#define GL_SMOOTH 0x1D01
#endif

#endif // GLES2TokenCompat_h__
