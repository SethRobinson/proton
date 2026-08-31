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
//RenderPipeline.h is identical in every translation unit).  It is the
//DEFAULT renderer in those builds; launch with -fixedpipeline to fall back
//to the legacy fixed-function path (not possible in RT_SHADER_PIPELINE_ONLY
//builds, which compile the legacy path out entirely).

#ifndef ShaderPipeline_h__
#define ShaderPipeline_h__

#ifdef RT_SHADER_PIPELINE_AVAILABLE

extern bool g_bShaderPipelineActive; //defaults true; cleared by -fixedpipeline before GL init

void SP_ResetState(); //re-inits backend state; safe before any GL exists
void SP_DropPushedMatrices(); //pops both stacks to depth 0 (keeping the base matrices): for when the engine abandons pushed state along with the ortho flag on a GL context rebuild (win/app/main.cpp InitVideo)

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

//---------------------------------------------------------------------------
// Render targets (FBOs).  Shader-pipeline only.  Usually used through
// Surface::InitRenderTarget / BeginRenderTarget / EndRenderTarget.
//---------------------------------------------------------------------------

//creates an FBO around an existing texture; returns 0 on failure
unsigned int SP_CreateFrameBuffer(unsigned int glTextureID, int width, int height);
void SP_DestroyFrameBuffer(unsigned int frameBufferID);
void SP_BindFrameBuffer(unsigned int frameBufferID, int width, int height); //saves previous binding+viewport, no nesting
void SP_UnbindFrameBuffer(); //restores what SP_BindFrameBuffer saved

//---------------------------------------------------------------------------
// RTShader: a custom GLSL program for app use.  Shader-pipeline only.
//
// Contract: the engine binds attributes a_pos (vec4), a_uv (vec2), a_color
// (vec4) and sets uniforms uProj, uMV (mat4), uColor (vec4) and uTex
// (sampler2D, unit 0) if declared.  Write GLSL ES 1.00 style; on ES2 builds
// "precision mediump float;" is prepended to the fragment shader if missing.
// Because of that mediump default, keep time-like uniforms SMALL (wrap them
// on the CPU): a seconds-since-launch value stutters visibly once it grows
// past a few hundred (see the uTime handling in RTShader's App.cpp).
// While a shader is active (SetActiveShader), every engine draw (Surface
// blits, RenderBatcher, DrawFilledRect...) renders through it.
// A GL context rebuild (window resize or fullscreen toggle on Windows, a
// lost context on Android) is transparent: the pipeline listens to BaseApp's
// m_sig_unloadSurfaces/m_sig_loadSurfaces, drops every program there and
// relinks each live RTShader from the source it keeps, custom uniform values
// included.  Only an explicit Kill() makes a shader stay dead.
//---------------------------------------------------------------------------

class RTShader
{
public:
	RTShader();
	~RTShader();

	bool Load(const char *pVertexSource, const char *pFragmentSource); //compiles+links now; needs GL context
	void Kill();
	bool IsLoaded() const { return m_program != 0; }

	//set custom uniforms (applied on every draw while this shader is active)
	void SetUniform1f(const char *pName, float v);
	void SetUniform4f(const char *pName, float x, float y, float z, float w);

	//internal use by the pipeline:
	unsigned int GetProgram() const { return m_program; }
	int GetStandardLoc(int which) const { return m_standardLocs[which]; }
	void ApplyCustomUniforms();
	void OnGLContextLost();     //forget the program (deleting it while the context is alive), keep source + uniforms
	void OnGLContextRestored(); //relink from the kept source, re-resolve the custom uniform locations

private:

	bool Build(); //compile + link m_vertexSource/m_fragmentSource into m_program

	unsigned int m_program;
	int m_standardLocs[8];
	std::string m_vertexSource, m_fragmentSource; //kept so a context rebuild can relink; cleared by Kill()

	struct CustomUniform
	{
		char name[48];
		int loc; //-1 = the shader doesn't have it (warned once, then ignored)
		int count; //1 or 4 floats
		float v[4];
	};
	static const int MAX_CUSTOM_UNIFORMS = 16;
	CustomUniform m_customUniforms[MAX_CUSTOM_UNIFORMS];
	int m_customUniformCount;

	int FindOrAddCustomUniform(const char *pName, int count);
};

//while set, all engine draws use this program instead of the built-in ubershader
void SetActiveShader(RTShader *pShader); //NULL to return to normal rendering
RTShader * GetActiveShader();

#endif // RT_SHADER_PIPELINE_AVAILABLE

#endif // ShaderPipeline_h__
