//  ***************************************************************
//  ShaderPipeline - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

//The shader (GLES2-class) implementation of the RenderPipeline surface: CPU
//matrix stacks feeding matrix uniforms, an ubershader cache keyed on a small
//state bitfield (texturing / per-vertex color / clip plane), and client-side
//vertex arrays fed straight to glVertexAttribPointer (legal in desktop GL and
//GLES2; the WebGL target will add a streaming VBO when it flips).
//
//Only compiled into projects that add ShaderPipeline.cpp AND define
//RT_SHADER_PIPELINE_AVAILABLE (project-wide, so the inline dispatch in
//RenderPipeline.h is identical in every translation unit).  Runtime opt-in:
//launch any app with the -shaderpipeline parm.  Without the parm the legacy
//fixed-function path runs exactly as before.

#ifndef ShaderPipeline_h__
#define ShaderPipeline_h__

#ifdef RT_SHADER_PIPELINE_AVAILABLE

extern bool g_bShaderPipelineActive; //set by the -shaderpipeline parm before GL init

void SP_ResetState(); //called when the parm is seen, before any GL exists

//mirrors of the rt* surface in RenderPipeline.h
void SP_MatrixMode(GLenum mode);
void SP_PushMatrix();
void SP_PopMatrix();
void SP_LoadIdentity();
void SP_Translatef(float x, float y, float z);
void SP_Rotatef(float degrees, float x, float y, float z);
void SP_Scalef(float x, float y, float z);
void SP_Orthof(float left, float right, float bottom, float top, float zNear, float zFar);
void SP_LoadMatrixf(const float *pMat16);
void SP_MultMatrixf(const float *pMat16);
void SP_GetMatrixf(GLenum pname, float *pMat16Out);
void SP_Color4x(int r, int g, int b, int a);
void SP_EnableClientState(GLenum array);
void SP_DisableClientState(GLenum array);
void SP_VertexPointer(GLint size, GLenum type, GLsizei stride, const void *pData);
void SP_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pData);
void SP_ColorPointer(GLint size, GLenum type, GLsizei stride, const void *pData);
void SP_NormalPointer(GLenum type, GLsizei stride, const void *pData);
void SP_DrawArrays(GLenum mode, GLint first, GLsizei count);
void SP_DrawElements(GLenum mode, GLsizei count, GLenum type, const void *pIndices);
void SP_Enable(GLenum cap);
void SP_Disable(GLenum cap);
void SP_Lightfv(GLenum light, GLenum pname, const float *params);
void SP_Hint(GLenum target, GLenum mode);
void SP_ClipPlane(GLenum plane, const float *pEq4);

#endif // RT_SHADER_PIPELINE_AVAILABLE

#endif // ShaderPipeline_h__
