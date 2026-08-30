//  ***************************************************************
//  GL1ShaderShim - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

//The app-compatibility shim for the shader pipeline: remaps the fixed-function
//GL names that Proton apps actually call (per the Aug 2026 census: matrix
//stack, glColor4x, client arrays + draws, fixed-function enables, one clip
//plane, glGetFloatv matrix readback) onto the runtime-dispatched rt* layer.
//App source needs NO changes: on the legacy path the rt* calls fall straight
//through to real GL (bit-identical), and with -shaderpipeline they route to
//the ShaderPipeline backend.
//
//Included automatically at the end of PlatformSetup.h when
//RT_SHADER_PIPELINE_AVAILABLE is defined.  A translation unit that implements
//the pipeline itself opts out by defining RT_RENDERER_INTERNAL before
//PlatformPrecomp.h.
//
//Deliberately NOT remapped (still real GL, fine in ES2): glClear/glClearColor,
//glScissor, glViewport, glReadPixels, glBlendFunc, glBindTexture and all
//texture calls, glGetError/glGetString, glGetBooleanv/glGetIntegerv.
//Deliberately unsupported (no Proton app uses them): immediate mode, display
//lists, glTexEnv*, fog, multitexture, glAlphaFunc with a real reference.
//glLight*/glShadeModel are accepted but ignored until the single-light
//ubershader variant lands, so lit 3D apps (RTMindWall) stay on the legacy
//path for now.

#ifndef GL1ShaderShim_h__
#define GL1ShaderShim_h__

//compile the rt* dispatch layer BEFORE any remapping macros exist, so its
//legacy branches still call the real GL functions
#include "Renderer/RenderPipeline.h"

//--- signature adapters ----------------------------------------------------

//desktop glOrtho takes doubles
inline void rtShim_glOrtho(double l, double r, double b, double t, double zNear, double zFar)
{
	rtOrthof((float)l, (float)r, (float)b, (float)t, (float)zNear, (float)zFar);
}

//matrix pnames come from the CPU stacks in shader mode; anything else is
//still a live GL query
inline void rtShim_glGetFloatv(GLenum pname, float *pOut)
{
	if (pname == GL_MODELVIEW_MATRIX || pname == GL_PROJECTION_MATRIX)
	{
		rtGetMatrixf(pname, pOut);
		return;
	}
	glGetFloatv(pname, pOut);
}

//apps historically cast their float[4] plane to GLdouble* to satisfy the
//desktop prototype; the values were always floats, so cast straight back
inline void rtShim_glClipPlane(GLenum plane, const void *pEq)
{
	rtClipPlane(plane, (const float*)pEq);
}

//lighting: glLightfv routes into the single-light (GL_LIGHT0) ubershader
//emulation on the shader pipeline, and through to real GL on the legacy one.
//glLightf and glShadeModel pass through on legacy and are ignored on the
//shader path (Gouraud/GL_SMOOTH is the only shading model implemented, and
//no Proton app uses the scalar light params).
inline void rtShim_glLightf(GLenum light, GLenum pname, float param)
{
#ifndef RT_SHADER_PIPELINE_ONLY
	if (g_bShaderPipelineActive) return;
	glLightf(light, pname, param);
#endif
}
inline void rtShim_glShadeModel(GLenum mode)
{
#ifndef RT_SHADER_PIPELINE_ONLY
	if (g_bShaderPipelineActive) return;
	glShadeModel(mode);
#endif
}

//--- the remap -------------------------------------------------------------

#define glMatrixMode rtMatrixMode
#define glPushMatrix rtPushMatrix
#define glPopMatrix rtPopMatrix
#define glLoadIdentity rtLoadIdentity
#define glTranslatef rtTranslatef
#define glRotatef rtRotatef
#define glScalef rtScalef
#define glOrtho rtShim_glOrtho
#define glLoadMatrixf rtLoadMatrixf
#define glMultMatrixf rtMultMatrixf
#define glGetFloatv rtShim_glGetFloatv

//note: glOrthof/glColor4x may already be macros from GLCompatDesktop.h; undo
//that first so ours win (the engine itself now calls rt* directly)
#ifdef glOrthof
	#undef glOrthof
#endif
#define glOrthof rtOrthof
#ifdef glColor4x
	#undef glColor4x
#endif
#define glColor4x rtColor4x

#define glEnableClientState rtEnableClientState
#define glDisableClientState rtDisableClientState
#define glVertexPointer rtVertexPointer
#define glTexCoordPointer rtTexCoordPointer
#define glColorPointer rtColorPointer
#define glNormalPointer rtNormalPointer
#define glDrawArrays rtDrawArrays
#define glDrawElements rtDrawElements

#define glEnable rtEnable
#define glDisable rtDisable
#ifdef glClipPlane
	#undef glClipPlane
#endif
#define glClipPlane rtShim_glClipPlane
#ifdef glClipPlanef
	#undef glClipPlanef
#endif
#define glClipPlanef rtShim_glClipPlane

#define glLightf rtShim_glLightf
#define glLightfv rtLightfv
#define glShadeModel rtShim_glShadeModel

#endif // GL1ShaderShim_h__
