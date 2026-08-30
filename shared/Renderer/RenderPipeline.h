//  ***************************************************************
//  RenderPipeline - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

//The funnel for every fixed-function GL feature the engine itself uses: the
//matrix stack, the current color, client-side vertex arrays and the draw
//calls fed by them, and the fixed-function enable/disable bits.  Engine code
//calls these rt* versions instead of raw GL.
//
//On the legacy GL1/GLES1 path (today) they are inline passthroughs, so
//behavior is bit-identical.  The shader backend will reimplement this exact
//surface (CPU matrix stacks -> uniforms, client arrays -> a streaming vertex
//buffer, color -> a uniform, alpha test/clip plane -> shader discard), which
//is also what the app-compatibility shim will sit on.
//
//Deliberately NOT wrapped, because it exists unchanged in GLES2/WebGL:
//texture objects (glGenTextures/glBindTexture/glTexImage2D/glTexParameter*),
//blend (glEnable(GL_BLEND)/glBlendFunc), depth, cull, scissor, viewport,
//clear, and glReadPixels.  Use rtEnable/rtDisable ONLY for the
//fixed-function-era enums listed above them.
//
//Include from a .cpp that already pulled in the platform GL headers (i.e.
//after PlatformPrecomp.h).

#ifndef RenderPipeline_h__
#define RenderPipeline_h__

//---------------------------------------------------------------------------
// Matrix stack
//---------------------------------------------------------------------------

inline void rtMatrixMode(GLenum mode) { glMatrixMode(mode); }
inline void rtPushMatrix() { glPushMatrix(); }
inline void rtPopMatrix() { glPopMatrix(); }
inline void rtLoadIdentity() { glLoadIdentity(); }
inline void rtTranslatef(float x, float y, float z) { glTranslatef(x, y, z); }
inline void rtRotatef(float degrees, float x, float y, float z) { glRotatef(degrees, x, y, z); }
inline void rtScalef(float x, float y, float z) { glScalef(x, y, z); }
inline void rtOrthof(float left, float right, float bottom, float top, float zNear, float zFar) { glOrthof(left, right, bottom, top, zNear, zFar); }
inline void rtLoadMatrixf(const float *pMat16) { glLoadMatrixf(pMat16); }
inline void rtMultMatrixf(const float *pMat16) { glMultMatrixf(pMat16); }

//pname is GL_MODELVIEW_MATRIX or GL_PROJECTION_MATRIX
inline void rtGetMatrixf(GLenum pname, float *pMat16Out) { glGetFloatv(pname, pMat16Out); }

//---------------------------------------------------------------------------
// Current color (16.16 fixed point, matching the engine's existing usage)
//---------------------------------------------------------------------------

inline void rtColor4x(int r, int g, int b, int a) { glColor4x(r, g, b, a); }

//---------------------------------------------------------------------------
// Client-side vertex arrays + draws
//---------------------------------------------------------------------------

inline void rtEnableClientState(GLenum array) { glEnableClientState(array); }
inline void rtDisableClientState(GLenum array) { glDisableClientState(array); }
inline void rtVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pData) { glVertexPointer(size, type, stride, pData); }
inline void rtTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pData) { glTexCoordPointer(size, type, stride, pData); }
inline void rtColorPointer(GLint size, GLenum type, GLsizei stride, const void *pData) { glColorPointer(size, type, stride, pData); }
inline void rtNormalPointer(GLenum type, GLsizei stride, const void *pData) { glNormalPointer(type, stride, pData); }
inline void rtDrawArrays(GLenum mode, GLint first, GLsizei count) { glDrawArrays(mode, first, count); }
inline void rtDrawElements(GLenum mode, GLsizei count, GLenum type, const void *pIndices) { glDrawElements(mode, count, type, pIndices); }

//---------------------------------------------------------------------------
// Fixed-function state bits.  ONLY for enums that die in the shader pipeline:
// GL_TEXTURE_2D, GL_ALPHA_TEST, GL_LIGHTING, GL_LINE_SMOOTH, GL_CLIP_PLANE0,
// GL_COLOR_MATERIAL.  ES2-legal state (GL_BLEND, GL_DEPTH_TEST,
// GL_SCISSOR_TEST, GL_CULL_FACE) keeps using glEnable/glDisable directly.
//---------------------------------------------------------------------------

inline void rtEnable(GLenum cap) { glEnable(cap); }
inline void rtDisable(GLenum cap) { glDisable(cap); }
inline void rtHint(GLenum target, GLenum mode) { glHint(target, mode); } //GL_LINE_SMOOTH_HINT is the only engine use

//pEq4 points at 4 floats.  Note: this preserves the engine's historical call
//exactly; on desktop GL the cast means the doubles read are garbage bits, so
//user clip planes have likely only ever worked correctly on real GLES1 -
//documenting, not fixing, while the suite guards for zero behavior change.
inline void rtClipPlane(GLenum plane, const float *pEq4) { glClipPlane(plane, (GLdouble*)pEq4); }

#endif // RenderPipeline_h__
