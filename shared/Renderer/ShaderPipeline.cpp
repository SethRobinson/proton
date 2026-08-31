//  ***************************************************************
//  ShaderPipeline - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

//this file implements the backend, so it must see the REAL gl functions, not
//the app-compatibility macros from GL1ShaderShim.h.  GL1ShaderShimUndef.h
//removes those macros again right after the precomp; that works identically
//whether they came from a real include or were baked into an MSVC
//precompiled header (where nothing before the PlatformPrecomp.h line even
//compiles, so an opt-out define here would be silently ignored - see C4603).
#include "PlatformPrecomp.h"
#include "GL1ShaderShimUndef.h"

#ifdef RT_SHADER_PIPELINE_AVAILABLE

#include "ShaderPipeline.h"
#include "util/MathUtils.h" //for CL_Mat4f (used for the clip plane inverse only)

//the shader pipeline is the default wherever it's compiled in; the
//-fixedpipeline launch parm switches back to fixed function for comparison
//(except in RT_SHADER_PIPELINE_ONLY builds, which have no legacy path).
//A project that REALLY wants to default to the legacy path can define
//PROTON_USE_FIXED_PIPELINE project-wide (-shaderpipeline still opts in at
//runtime).
#if defined(PROTON_USE_FIXED_PIPELINE) && defined(RT_SHADER_PIPELINE_ONLY)
	#error PROTON_USE_FIXED_PIPELINE makes no sense in an RT_SHADER_PIPELINE_ONLY build - there is no fixed-function path compiled in
#endif
#ifdef PROTON_USE_FIXED_PIPELINE
bool g_bShaderPipelineActive = false;
#else
bool g_bShaderPipelineActive = true;
#endif

//---------------------------------------------------------------------------
// GL2 entry points + constants.  The vendored desktop gl.h is 1.1-only, so
// Windows and Linux declare and load what we need at runtime; GLES2 targets
// (and Mac, whose OpenGL framework headers cover 2.1) get them directly.
//---------------------------------------------------------------------------

#ifndef GL_FRAGMENT_SHADER
	#define GL_FRAGMENT_SHADER 0x8B30
	#define GL_VERTEX_SHADER 0x8B31
	#define GL_COMPILE_STATUS 0x8B81
	#define GL_LINK_STATUS 0x8B82
	#define GL_INFO_LOG_LENGTH 0x8B84
#endif

#ifndef GL_PROGRAM_POINT_SIZE
	#define GL_PROGRAM_POINT_SIZE 0x8642
#endif

#ifndef GL_FRAMEBUFFER
	#define GL_FRAMEBUFFER 0x8D40
	#define GL_COLOR_ATTACHMENT0 0x8CE0
	#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
	#define GL_FRAMEBUFFER_BINDING 0x8CA6
#endif

#if (defined(_WIN32) || defined(PLATFORM_LINUX)) && defined(C_GL_MODE)

//desktop GL on Windows and Linux ships a GL 1.1 header, so everything GL2+
//must be fetched at runtime.  Windows uses wglGetProcAddress; Linux goes
//through SDL (mandatory in Proton's Linux builds, and works on X11 and
//Wayland alike).  SDL's own declaration is plain cdecl on Linux, so declare
//it here rather than dragging SDL.h into the renderer.
#ifdef PLATFORM_LINUX
extern "C" void * SDL_GL_GetProcAddress(const char *proc);
#define SP_GETPROC(name) SDL_GL_GetProcAddress(name)
#else
#define SP_GETPROC(name) wglGetProcAddress(name)
#endif

typedef char SPGLchar;
typedef GLuint (APIENTRY *PFNSPCREATESHADER)(GLenum type);
typedef void (APIENTRY *PFNSPSHADERSOURCE)(GLuint shader, GLsizei count, const SPGLchar **string, const GLint *length);
typedef void (APIENTRY *PFNSPCOMPILESHADER)(GLuint shader);
typedef void (APIENTRY *PFNSPGETSHADERIV)(GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNSPGETSHADERINFOLOG)(GLuint shader, GLsizei bufSize, GLsizei *length, SPGLchar *infoLog);
typedef GLuint (APIENTRY *PFNSPCREATEPROGRAM)(void);
typedef void (APIENTRY *PFNSPATTACHSHADER)(GLuint program, GLuint shader);
typedef void (APIENTRY *PFNSPBINDATTRIBLOCATION)(GLuint program, GLuint index, const SPGLchar *name);
typedef void (APIENTRY *PFNSPLINKPROGRAM)(GLuint program);
typedef void (APIENTRY *PFNSPGETPROGRAMIV)(GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNSPGETPROGRAMINFOLOG)(GLuint program, GLsizei bufSize, GLsizei *length, SPGLchar *infoLog);
typedef void (APIENTRY *PFNSPUSEPROGRAM)(GLuint program);
typedef void (APIENTRY *PFNSPDELETESHADER)(GLuint shader);
typedef void (APIENTRY *PFNSPDELETEPROGRAM)(GLuint program);
typedef GLint (APIENTRY *PFNSPGETUNIFORMLOCATION)(GLuint program, const SPGLchar *name);
typedef void (APIENTRY *PFNSPUNIFORMMATRIX4FV)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY *PFNSPUNIFORM4FV)(GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY *PFNSPUNIFORM1I)(GLint location, GLint v0);
typedef void (APIENTRY *PFNSPUNIFORM1F)(GLint location, GLfloat v0);
typedef void (APIENTRY *PFNSPENABLEVERTEXATTRIBARRAY)(GLuint index);
typedef void (APIENTRY *PFNSPDISABLEVERTEXATTRIBARRAY)(GLuint index);
typedef void (APIENTRY *PFNSPVERTEXATTRIBPOINTER)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (APIENTRY *PFNSPVERTEXATTRIB4F)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY *PFNSPGENFRAMEBUFFERS)(GLsizei n, GLuint *framebuffers);
typedef void (APIENTRY *PFNSPDELETEFRAMEBUFFERS)(GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRY *PFNSPBINDFRAMEBUFFER)(GLenum target, GLuint framebuffer);
typedef void (APIENTRY *PFNSPFRAMEBUFFERTEXTURE2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum (APIENTRY *PFNSPCHECKFRAMEBUFFERSTATUS)(GLenum target);

static PFNSPCREATESHADER spCreateShader = NULL;
static PFNSPSHADERSOURCE spShaderSource = NULL;
static PFNSPCOMPILESHADER spCompileShader = NULL;
static PFNSPGETSHADERIV spGetShaderiv = NULL;
static PFNSPGETSHADERINFOLOG spGetShaderInfoLog = NULL;
static PFNSPCREATEPROGRAM spCreateProgram = NULL;
static PFNSPATTACHSHADER spAttachShader = NULL;
static PFNSPBINDATTRIBLOCATION spBindAttribLocation = NULL;
static PFNSPLINKPROGRAM spLinkProgram = NULL;
static PFNSPGETPROGRAMIV spGetProgramiv = NULL;
static PFNSPGETPROGRAMINFOLOG spGetProgramInfoLog = NULL;
static PFNSPUSEPROGRAM spUseProgram = NULL;
static PFNSPDELETESHADER spDeleteShader = NULL;
static PFNSPDELETEPROGRAM spDeleteProgram = NULL;
static PFNSPGETUNIFORMLOCATION spGetUniformLocation = NULL;
static PFNSPUNIFORMMATRIX4FV spUniformMatrix4fv = NULL;
static PFNSPUNIFORM4FV spUniform4fv = NULL;
static PFNSPUNIFORM1I spUniform1i = NULL;
static PFNSPUNIFORM1F spUniform1f = NULL;
static PFNSPENABLEVERTEXATTRIBARRAY spEnableVertexAttribArray = NULL;
static PFNSPDISABLEVERTEXATTRIBARRAY spDisableVertexAttribArray = NULL;
static PFNSPVERTEXATTRIBPOINTER spVertexAttribPointer = NULL;
static PFNSPVERTEXATTRIB4F spVertexAttrib4f = NULL;
static PFNSPGENFRAMEBUFFERS spGenFramebuffers = NULL;
static PFNSPDELETEFRAMEBUFFERS spDeleteFramebuffers = NULL;
static PFNSPBINDFRAMEBUFFER spBindFramebuffer = NULL;
static PFNSPFRAMEBUFFERTEXTURE2D spFramebufferTexture2D = NULL;
static PFNSPCHECKFRAMEBUFFERSTATUS spCheckFramebufferStatus = NULL;

static bool LoadGL2FunctionPointers()
{
	#define SP_LOAD(var, name) var = (decltype(var)) SP_GETPROC(name); if (!var) { LogError("ShaderPipeline: missing GL function %s", name); return false; }
	SP_LOAD(spCreateShader, "glCreateShader");
	SP_LOAD(spShaderSource, "glShaderSource");
	SP_LOAD(spCompileShader, "glCompileShader");
	SP_LOAD(spGetShaderiv, "glGetShaderiv");
	SP_LOAD(spGetShaderInfoLog, "glGetShaderInfoLog");
	SP_LOAD(spCreateProgram, "glCreateProgram");
	SP_LOAD(spAttachShader, "glAttachShader");
	SP_LOAD(spBindAttribLocation, "glBindAttribLocation");
	SP_LOAD(spLinkProgram, "glLinkProgram");
	SP_LOAD(spGetProgramiv, "glGetProgramiv");
	SP_LOAD(spGetProgramInfoLog, "glGetProgramInfoLog");
	SP_LOAD(spUseProgram, "glUseProgram");
	SP_LOAD(spDeleteShader, "glDeleteShader");
	SP_LOAD(spDeleteProgram, "glDeleteProgram");
	SP_LOAD(spGetUniformLocation, "glGetUniformLocation");
	SP_LOAD(spUniformMatrix4fv, "glUniformMatrix4fv");
	SP_LOAD(spUniform4fv, "glUniform4fv");
	SP_LOAD(spUniform1i, "glUniform1i");
	SP_LOAD(spUniform1f, "glUniform1f");
	SP_LOAD(spEnableVertexAttribArray, "glEnableVertexAttribArray");
	SP_LOAD(spDisableVertexAttribArray, "glDisableVertexAttribArray");
	SP_LOAD(spVertexAttribPointer, "glVertexAttribPointer");
	SP_LOAD(spVertexAttrib4f, "glVertexAttrib4f");
	SP_LOAD(spGenFramebuffers, "glGenFramebuffers");
	SP_LOAD(spDeleteFramebuffers, "glDeleteFramebuffers");
	SP_LOAD(spBindFramebuffer, "glBindFramebuffer");
	SP_LOAD(spFramebufferTexture2D, "glFramebufferTexture2D");
	SP_LOAD(spCheckFramebufferStatus, "glCheckFramebufferStatus");
	#undef SP_LOAD
	return true;
}

#else
//GLES2-style platforms get these from their headers when those targets flip
static bool LoadGL2FunctionPointers() { return true; }
#define spCreateShader glCreateShader
#define spShaderSource glShaderSource
#define spCompileShader glCompileShader
#define spGetShaderiv glGetShaderiv
#define spGetShaderInfoLog glGetShaderInfoLog
#define spCreateProgram glCreateProgram
#define spAttachShader glAttachShader
#define spBindAttribLocation glBindAttribLocation
#define spLinkProgram glLinkProgram
#define spGetProgramiv glGetProgramiv
#define spGetProgramInfoLog glGetProgramInfoLog
#define spUseProgram glUseProgram
#define spDeleteShader glDeleteShader
#define spDeleteProgram glDeleteProgram
#define spGetUniformLocation glGetUniformLocation
#define spUniformMatrix4fv glUniformMatrix4fv
#define spUniform4fv glUniform4fv
#define spUniform1i glUniform1i
#define spUniform1f glUniform1f
#define spEnableVertexAttribArray glEnableVertexAttribArray
#define spDisableVertexAttribArray glDisableVertexAttribArray
#define spVertexAttribPointer glVertexAttribPointer
#define spVertexAttrib4f glVertexAttrib4f
#define spGenFramebuffers glGenFramebuffers
#define spDeleteFramebuffers glDeleteFramebuffers
#define spBindFramebuffer glBindFramebuffer
#define spFramebufferTexture2D glFramebufferTexture2D
#define spCheckFramebufferStatus glCheckFramebufferStatus
#endif

//---------------------------------------------------------------------------
// Column-major 4x4 matrix math, following the GL spec exactly (post-multiply)
//---------------------------------------------------------------------------

struct SPMat16
{
	float m[16];
};

static void MatIdentity(SPMat16 &out)
{
	memset(out.m, 0, sizeof(out.m));
	out.m[0] = out.m[5] = out.m[10] = out.m[15] = 1.0f;
}

//out = a * b (column-major); safe even if out aliases a or b
static void MatMul(SPMat16 &out, const SPMat16 &a, const SPMat16 &b)
{
	SPMat16 r;
	for (int c = 0; c < 4; c++)
	{
		for (int row = 0; row < 4; row++)
		{
			r.m[c * 4 + row] =
				a.m[0 * 4 + row] * b.m[c * 4 + 0] +
				a.m[1 * 4 + row] * b.m[c * 4 + 1] +
				a.m[2 * 4 + row] * b.m[c * 4 + 2] +
				a.m[3 * 4 + row] * b.m[c * 4 + 3];
		}
	}
	out = r;
}

//---------------------------------------------------------------------------
// Pipeline state
//---------------------------------------------------------------------------

const int SP_MAX_STACK_DEPTH = 32;

struct SPMatrixStack
{
	SPMat16 stack[SP_MAX_STACK_DEPTH];
	int depth;
};

struct SPClientArray
{
	bool bEnabled;
	GLint size;
	GLenum type;
	GLsizei stride;
	const void *pData;
};

enum { SP_ARRAY_VERTEX = 0, SP_ARRAY_TEXCOORD, SP_ARRAY_COLOR, SP_ARRAY_NORMAL, SP_ARRAY_COUNT };

//program variant bits
enum { SP_VARIANT_TEXTURE = 1, SP_VARIANT_VCOLOR = 2, SP_VARIANT_CLIP = 4, SP_VARIANT_LIGHT = 8, SP_VARIANT_COUNT = 16 };

struct SPProgram
{
	GLuint program;
	GLint locProj, locMV, locColor, locClipPlane;
	GLint locNormalMat, locLightPosEye, locLightAmbient, locLightDiffuse, locColorMaterial;
};

enum { SP_STDLOC_PROJ = 0, SP_STDLOC_MV, SP_STDLOC_COLOR };

struct SPState
{
	SPState() { Reset(); } //the pipeline is on by default, so state must be sane before any parm parsing
	void Reset();

	bool bInitted;
	bool bInitFailed;

	SPMatrixStack matrix[2]; //0 = modelview, 1 = projection
	int curMatrix;           //index into matrix[]

	float color[4];
	SPClientArray arrays[SP_ARRAY_COUNT];
	bool bTexture2D;
	bool bClipPlane;
	float clipPlaneEye[4]; //already transformed to eye space, per the GL spec

	//single-light fixed-function emulation (GL_LIGHT0 only, which is all any
	//Proton app has ever used)
	bool bLighting;
	bool bLight0;
	bool bColorMaterial;
	float lightPosEye[4];   //transformed to eye space at glLightfv time, per the GL spec
	float lightAmbient[4];
	float lightDiffuse[4];

	SPProgram programs[SP_VARIANT_COUNT];
	GLuint boundProgram;

	//render target state (single level, no nesting)
	GLuint boundRenderTargetFBO;
	GLint savedFBOBinding;
	GLint savedViewport[4];
	GLint savedCullFaceMode;

	RTShader *pActiveShader;
};

static SPState g_sp;

static SPMat16 & CurMat() { return g_sp.matrix[g_sp.curMatrix].stack[g_sp.matrix[g_sp.curMatrix].depth]; }

//---------------------------------------------------------------------------
// Shaders.  Fixed-function equivalence: fragment = texture * color (the
// GL_MODULATE default; the engine never changes texenv), untextured =
// color.  Clip plane keeps fragments with dot(planeEye, eyePos) >= 0.
// Alpha test needs nothing: the engine never calls glAlphaFunc, and the GL
// default (GL_ALWAYS) makes GL_ALPHA_TEST a no-op.
//---------------------------------------------------------------------------

static const char *GetVertexShaderSource(int variant)
{
	static string s;
	s = "";
#ifndef C_GL_MODE
	s += "precision highp float;\n";
#endif
	s += "attribute vec4 a_pos;\n"
		"uniform mat4 uProj;\n"
		"uniform mat4 uMV;\n"
		"uniform vec4 uColor;\n"
		"varying vec4 v_color;\n";
	if (variant & SP_VARIANT_TEXTURE) s += "attribute vec2 a_uv;\nvarying vec2 v_uv;\n";
	if (variant & SP_VARIANT_VCOLOR) s += "attribute vec4 a_color;\n";
	if (variant & SP_VARIANT_CLIP) s += "uniform vec4 uClipPlane;\nvarying float v_clipDist;\n";
	if (variant & SP_VARIANT_LIGHT)
	{
		s += "attribute vec4 a_normal;\n"
			"uniform mat4 uNormalMat;\n"     //transpose(inverse(uMV)); upper 3x3 is what matters
			"uniform vec4 uLightPosEye;\n"   //already in eye space, w=0 means directional
			"uniform vec4 uLightAmbient;\n"
			"uniform vec4 uLightDiffuse;\n"
			"uniform float uColorMaterial;\n"; //1 = material ambient+diffuse track the color (GL_COLOR_MATERIAL)
	}
	s += "void main() {\n"
		"	vec4 eyePos = uMV * vec4(a_pos.xyz, 1.0);\n"
		"	gl_Position = uProj * eyePos;\n";
#ifndef C_GL_MODE
	s += "	gl_PointSize = 1.0;\n"; //ES2 points render size 0 without this; desktop uses glPointSize state instead
#endif
	if (variant & SP_VARIANT_TEXTURE) s += "	v_uv = a_uv;\n";
	s += (variant & SP_VARIANT_VCOLOR) ? "	vec4 baseColor = a_color;\n" : "	vec4 baseColor = uColor;\n";
	if (variant & SP_VARIANT_LIGHT)
	{
		//fixed-function equation with Proton's defaults: emission 0, specular 0,
		//attenuation 1, global ambient 0.2, materials 0.2/0.8 unless
		//GL_COLOR_MATERIAL tracks the color.  Normals deliberately NOT
		//normalized (GL_NORMALIZE was never enabled on the fixed pipeline).
		s += "	vec3 n = (uNormalMat * vec4(a_normal.xyz, 0.0)).xyz;\n"
			"	vec3 L;\n"
			"	if (uLightPosEye.w == 0.0) L = normalize(uLightPosEye.xyz);\n"
			"	else L = normalize(uLightPosEye.xyz - eyePos.xyz);\n"
			"	vec3 matAmb = mix(vec3(0.2), baseColor.rgb, uColorMaterial);\n"
			"	vec3 matDif = mix(vec3(0.8), baseColor.rgb, uColorMaterial);\n"
			"	float litAlpha = mix(1.0, baseColor.a, uColorMaterial);\n"
			"	vec3 lit = matAmb * (vec3(0.2) + uLightAmbient.rgb) + matDif * uLightDiffuse.rgb * max(0.0, dot(n, L));\n"
			"	v_color = vec4(clamp(lit, 0.0, 1.0), litAlpha);\n";
	}
	else
	{
		s += "	v_color = baseColor;\n";
	}
	if (variant & SP_VARIANT_CLIP) s += "	v_clipDist = dot(uClipPlane, eyePos);\n";
	s += "}\n";
	return s.c_str();
}

static const char *GetFragmentShaderSource(int variant)
{
	static string s;
	s = "";
#ifndef C_GL_MODE
	s += "precision mediump float;\n";
#endif
	s += "varying vec4 v_color;\n";
	if (variant & SP_VARIANT_TEXTURE) s += "uniform sampler2D uTex;\nvarying vec2 v_uv;\n";
	if (variant & SP_VARIANT_CLIP) s += "varying float v_clipDist;\n";
	s += "void main() {\n";
	if (variant & SP_VARIANT_CLIP) s += "	if (v_clipDist < 0.0) discard;\n";
	if (variant & SP_VARIANT_TEXTURE)
		s += "	gl_FragColor = texture2D(uTex, v_uv) * v_color;\n";
	else
		s += "	gl_FragColor = v_color;\n";
	s += "}\n";
	return s.c_str();
}

static GLuint CompileShader(GLenum type, const char *pSource)
{
	GLuint shader = spCreateShader(type);
	spShaderSource(shader, 1, &pSource, NULL);
	spCompileShader(shader);
	GLint status = 0;
	spGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		char log[2048];
		spGetShaderInfoLog(shader, sizeof(log), NULL, log);
		LogError("ShaderPipeline: shader compile failed: %s\nSource:\n%s", log, pSource);
		spDeleteShader(shader);
		return 0;
	}
	return shader;
}

static bool BuildProgram(int variant)
{
	GLuint vs = CompileShader(GL_VERTEX_SHADER, GetVertexShaderSource(variant));
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, GetFragmentShaderSource(variant));
	if (!vs || !fs) return false;

	GLuint prog = spCreateProgram();
	spAttachShader(prog, vs);
	spAttachShader(prog, fs);
	spBindAttribLocation(prog, SP_ARRAY_VERTEX, "a_pos");
	if (variant & SP_VARIANT_TEXTURE) spBindAttribLocation(prog, SP_ARRAY_TEXCOORD, "a_uv");
	if (variant & SP_VARIANT_VCOLOR) spBindAttribLocation(prog, SP_ARRAY_COLOR, "a_color");
	if (variant & SP_VARIANT_LIGHT) spBindAttribLocation(prog, SP_ARRAY_NORMAL, "a_normal");
	spLinkProgram(prog);
	spDeleteShader(vs);
	spDeleteShader(fs);

	GLint status = 0;
	spGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (!status)
	{
		char log[2048];
		spGetProgramInfoLog(prog, sizeof(log), NULL, log);
		LogError("ShaderPipeline: program link failed (variant %d): %s", variant, log);
		return false;
	}

	SPProgram &p = g_sp.programs[variant];
	p.program = prog;
	p.locProj = spGetUniformLocation(prog, "uProj");
	p.locMV = spGetUniformLocation(prog, "uMV");
	p.locColor = spGetUniformLocation(prog, "uColor");
	p.locClipPlane = spGetUniformLocation(prog, "uClipPlane");
	p.locNormalMat = spGetUniformLocation(prog, "uNormalMat");
	p.locLightPosEye = spGetUniformLocation(prog, "uLightPosEye");
	p.locLightAmbient = spGetUniformLocation(prog, "uLightAmbient");
	p.locLightDiffuse = spGetUniformLocation(prog, "uLightDiffuse");
	p.locColorMaterial = spGetUniformLocation(prog, "uColorMaterial");
	if (variant & SP_VARIANT_TEXTURE)
	{
		spUseProgram(prog);
		spUniform1i(spGetUniformLocation(prog, "uTex"), 0);
		spUseProgram(g_sp.boundProgram);
	}
	return true;
}

//---------------------------------------------------------------------------
// GL context rebuilds.  Windows recreates the context on a window resize or
// fullscreen toggle (win/app/main.cpp, MESSAGE_SET_VIDEO_MODE) and Android
// can lose it in the background; both announce it through BaseApp's
// m_sig_unloadSurfaces (old context, still alive on Windows) and
// m_sig_loadSurfaces (new context current).  Every GL object cached here dies
// with the context: the ubershader variants (rebuilt lazily by the next
// draw), the app's RTShaders (relinked from their kept source) and SPInit's
// per-context setup (redone by clearing bInitted; the entry points get
// re-fetched then, but stay valid in between so the Surface unload slots can
// still delete their framebuffers).
//---------------------------------------------------------------------------

static std::vector<RTShader*> & GetShaderRegistry()
{
	static std::vector<RTShader*> registry; //function-local: RTShaders can be globals that construct before other statics
	return registry;
}

static void SP_OnUnloadSurfaces()
{
	for (int i = 0; i < SP_VARIANT_COUNT; i++)
	{
		if (g_sp.programs[i].program)
		{
			spDeleteProgram(g_sp.programs[i].program);
			g_sp.programs[i].program = 0;
		}
	}
	if (g_sp.boundProgram)
	{
		spUseProgram(0);
		g_sp.boundProgram = 0;
	}
	g_sp.boundRenderTargetFBO = 0;

	std::vector<RTShader*> &shaders = GetShaderRegistry();
	for (size_t i = 0; i < shaders.size(); i++) shaders[i]->OnGLContextLost();

	glGetError(); //deleting names on a context that is already gone (Android) errors harmlessly

	g_sp.bInitted = false; //next SPInit re-fetches the entry points and redoes the per-context setup
	g_sp.bInitFailed = false;
}

static void SP_OnLoadSurfaces()
{
	std::vector<RTShader*> &shaders = GetShaderRegistry();
	for (size_t i = 0; i < shaders.size(); i++) shaders[i]->OnGLContextRestored();
}

static bool SPInit()
{
	if (g_sp.bInitted) return !g_sp.bInitFailed;
	g_sp.bInitted = true;

	if (!LoadGL2FunctionPointers())
	{
		g_sp.bInitFailed = true;
		LogError("ShaderPipeline: GL2 functions unavailable, staying on the fixed-function path");
		g_bShaderPipelineActive = false;
		return false;
	}

#if defined(_WIN32) && defined(C_GL_MODE)
	glEnable(GL_PROGRAM_POINT_SIZE); //so GL_POINTS behave; harmless if unsupported
	glGetError(); //eat any error from the above on old drivers
#endif

	//once per process: follow the GL context through rebuilds (see above).
	//Group 0 runs before the Surfaces' group 1 slots.
	static bool bHookedContextSignals = false;
	if (!bHookedContextSignals)
	{
		bHookedContextSignals = true;
		GetBaseApp()->m_sig_unloadSurfaces.connect(0, &SP_OnUnloadSurfaces);
		GetBaseApp()->m_sig_loadSurfaces.connect(0, &SP_OnLoadSurfaces);
	}

	LogMsg("ShaderPipeline: initialized (%s)", (const char*)glGetString(GL_VERSION));
	return true;
}

void SPState::Reset()
{
	memset(this, 0, sizeof(SPState));
	for (int i = 0; i < 2; i++)
	{
		matrix[i].depth = 0;
		MatIdentity(matrix[i].stack[0]);
	}
	curMatrix = 0;
	color[0] = color[1] = color[2] = color[3] = 1.0f;

	//GL_LIGHT0 defaults per the GL spec
	lightPosEye[0] = 0; lightPosEye[1] = 0; lightPosEye[2] = 1.0f; lightPosEye[3] = 0;
	lightAmbient[3] = 1.0f; //(0,0,0,1)
	lightDiffuse[0] = lightDiffuse[1] = lightDiffuse[2] = lightDiffuse[3] = 1.0f;
}

void SP_ResetState()
{
	g_sp.Reset();
}

void SP_DropPushedMatrices()
{
	//the base matrices stay: at startup the perspective projection is already
	//in place when this runs, and a rebuild re-sets it anyway (OnScreenSizeChange)
	g_sp.matrix[0].depth = 0;
	g_sp.matrix[1].depth = 0;
}

//---------------------------------------------------------------------------
// Matrix ops
//---------------------------------------------------------------------------

void SP_MatrixMode(GLenum mode)
{
	if (mode == GL_MODELVIEW) g_sp.curMatrix = 0;
	else if (mode == GL_PROJECTION) g_sp.curMatrix = 1;
	else assert(!"ShaderPipeline: unsupported matrix mode");
}

void SP_PushMatrix()
{
	SPMatrixStack &s = g_sp.matrix[g_sp.curMatrix];
	if (s.depth >= SP_MAX_STACK_DEPTH - 1) { assert(!"matrix stack overflow"); return; }
	s.stack[s.depth + 1] = s.stack[s.depth];
	s.depth++;
}

void SP_PopMatrix()
{
	SPMatrixStack &s = g_sp.matrix[g_sp.curMatrix];
	if (s.depth <= 0) { assert(!"matrix stack underflow"); return; }
	s.depth--;
}

void SP_LoadIdentity() { MatIdentity(CurMat()); }

void SP_Translatef(float x, float y, float z)
{
	SPMat16 t;
	MatIdentity(t);
	t.m[12] = x; t.m[13] = y; t.m[14] = z;
	MatMul(CurMat(), CurMat(), t);
}

void SP_Scalef(float x, float y, float z)
{
	SPMat16 t;
	MatIdentity(t);
	t.m[0] = x; t.m[5] = y; t.m[10] = z;
	MatMul(CurMat(), CurMat(), t);
}

void SP_Rotatef(float degrees, float x, float y, float z)
{
	float len = sqrtf(x * x + y * y + z * z);
	if (len == 0) return;
	x /= len; y /= len; z /= len;
	float rad = degrees * (float)M_PI / 180.0f;
	float c = cosf(rad), s = sinf(rad), ic = 1.0f - c;

	SPMat16 r;
	MatIdentity(r);
	r.m[0] = x * x * ic + c;     r.m[4] = x * y * ic - z * s; r.m[8] = x * z * ic + y * s;
	r.m[1] = y * x * ic + z * s; r.m[5] = y * y * ic + c;     r.m[9] = y * z * ic - x * s;
	r.m[2] = x * z * ic - y * s; r.m[6] = y * z * ic + x * s; r.m[10] = z * z * ic + c;
	MatMul(CurMat(), CurMat(), r);
}

void SP_Orthof(float l, float r, float b, float t, float n, float f)
{
	SPMat16 o;
	MatIdentity(o);
	o.m[0] = 2.0f / (r - l);
	o.m[5] = 2.0f / (t - b);
	o.m[10] = -2.0f / (f - n);
	o.m[12] = -(r + l) / (r - l);
	o.m[13] = -(t + b) / (t - b);
	o.m[14] = -(f + n) / (f - n);
	MatMul(CurMat(), CurMat(), o);
}

void SP_LoadMatrixf(const float *pMat16) { memcpy(CurMat().m, pMat16, sizeof(float) * 16); }

void SP_MultMatrixf(const float *pMat16)
{
	SPMat16 t;
	memcpy(t.m, pMat16, sizeof(t.m));
	MatMul(CurMat(), CurMat(), t);
}

void SP_GetMatrixf(GLenum pname, float *pMat16Out)
{
	int idx = (pname == GL_PROJECTION_MATRIX) ? 1 : 0;
	memcpy(pMat16Out, g_sp.matrix[idx].stack[g_sp.matrix[idx].depth].m, sizeof(float) * 16);
}

//---------------------------------------------------------------------------
// Color / state / arrays
//---------------------------------------------------------------------------

void SP_Color4x(int r, int g, int b, int a)
{
	g_sp.color[0] = float(r) / 65536.0f;
	g_sp.color[1] = float(g) / 65536.0f;
	g_sp.color[2] = float(b) / 65536.0f;
	g_sp.color[3] = float(a) / 65536.0f;
}

static int ArrayIndexFromEnum(GLenum array)
{
	switch (array)
	{
		case GL_VERTEX_ARRAY: return SP_ARRAY_VERTEX;
		case GL_TEXTURE_COORD_ARRAY: return SP_ARRAY_TEXCOORD;
		case GL_COLOR_ARRAY: return SP_ARRAY_COLOR;
		case GL_NORMAL_ARRAY: return SP_ARRAY_NORMAL;
	}
	assert(!"unknown client array");
	return SP_ARRAY_NORMAL;
}

void SP_EnableClientState(GLenum array) { g_sp.arrays[ArrayIndexFromEnum(array)].bEnabled = true; }
void SP_DisableClientState(GLenum array) { g_sp.arrays[ArrayIndexFromEnum(array)].bEnabled = false; }

static void SetArray(int idx, GLint size, GLenum type, GLsizei stride, const void *pData)
{
	SPClientArray &a = g_sp.arrays[idx];
	a.size = size; a.type = type; a.stride = stride; a.pData = pData;
}

void SP_VertexPointer(GLint size, GLenum type, GLsizei stride, const void *pData) { SetArray(SP_ARRAY_VERTEX, size, type, stride, pData); }
void SP_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pData) { SetArray(SP_ARRAY_TEXCOORD, size, type, stride, pData); }
void SP_ColorPointer(GLint size, GLenum type, GLsizei stride, const void *pData) { SetArray(SP_ARRAY_COLOR, size, type, stride, pData); }
void SP_NormalPointer(GLenum type, GLsizei stride, const void *pData) { SetArray(SP_ARRAY_NORMAL, 3, type, stride, pData); }

//true for caps that no longer exist as GL state in the shader pipeline; these
//are tracked (or deliberately ignored) instead of being passed to glEnable
static bool IsFixedFunctionOnlyCap(GLenum cap)
{
	switch (cap)
	{
		case GL_TEXTURE_2D:
		case GL_CLIP_PLANE0:
		case GL_ALPHA_TEST:
		case GL_LIGHTING:
		case GL_LIGHT0:
		case GL_LINE_SMOOTH:
		case GL_COLOR_MATERIAL:
		case GL_NORMALIZE:
		case GL_FOG:
			return true;
	}
	return false;
}

void SP_Enable(GLenum cap)
{
	if (cap == GL_TEXTURE_2D) { g_sp.bTexture2D = true; return; }
	if (cap == GL_CLIP_PLANE0) { g_sp.bClipPlane = true; return; }
	if (cap == GL_LIGHTING) { g_sp.bLighting = true; return; }
	if (cap == GL_LIGHT0) { g_sp.bLight0 = true; return; }
	if (cap == GL_COLOR_MATERIAL) { g_sp.bColorMaterial = true; return; }
#ifdef C_GL_MODE
	//line smoothing is rasterizer state, not fixed-function shading: on desktop
	//GL it works fine alongside shaders, and DrawLine's visuals depend on it
	//(GLES2 targets use DrawLine's quad-based variant instead)
	if (cap == GL_LINE_SMOOTH) { glEnable(cap); return; }
#endif
	if (IsFixedFunctionOnlyCap(cap)) return;
	//GL_ALPHA_TEST etc are no-ops here (alpha test: the engine never sets
	//glAlphaFunc, so GL's default GL_ALWAYS made it a no-op before too).
	//Anything else (GL_BLEND, GL_SCISSOR_TEST, GL_DEPTH_TEST, GL_CULL_FACE...)
	//is still real GL state in ES2 and passes straight through - app code
	//reaches here via the compatibility shim's glEnable remap.
	glEnable(cap);
}

void SP_Disable(GLenum cap)
{
	if (cap == GL_TEXTURE_2D) { g_sp.bTexture2D = false; return; }
	if (cap == GL_CLIP_PLANE0) { g_sp.bClipPlane = false; return; }
	if (cap == GL_LIGHTING) { g_sp.bLighting = false; return; }
	if (cap == GL_LIGHT0) { g_sp.bLight0 = false; return; }
	if (cap == GL_COLOR_MATERIAL) { g_sp.bColorMaterial = false; return; }
#ifdef C_GL_MODE
	if (cap == GL_LINE_SMOOTH) { glDisable(cap); return; }
#endif
	if (IsFixedFunctionOnlyCap(cap)) return;
	glDisable(cap);
}

void SP_Hint(GLenum target, GLenum mode) {}

void SP_Lightfv(GLenum light, GLenum pname, const float *params)
{
	if (light != GL_LIGHT0) return; //only light 0 is emulated; no Proton app has ever used another

	if (pname == GL_POSITION)
	{
		//per the GL spec the position is transformed by the modelview at call
		//time and stored in eye space (w == 0 means directional)
		const float *mv = g_sp.matrix[0].stack[g_sp.matrix[0].depth].m;
		for (int r = 0; r < 4; r++)
		{
			g_sp.lightPosEye[r] =
				mv[0 * 4 + r] * params[0] +
				mv[1 * 4 + r] * params[1] +
				mv[2 * 4 + r] * params[2] +
				mv[3 * 4 + r] * params[3];
		}
	}
	else if (pname == GL_AMBIENT)
	{
		memcpy(g_sp.lightAmbient, params, sizeof(float) * 4);
	}
	else if (pname == GL_DIFFUSE)
	{
		memcpy(g_sp.lightDiffuse, params, sizeof(float) * 4);
	}
	//other pnames (specular, attenuation, spot) keep their defaults; nothing uses them
}

void SP_ClipPlane(GLenum plane, const float *pEq4)
{
	//per the GL spec the plane is transformed by the inverse of the modelview
	//at call time and stored in eye space
	CL_Mat4f mv;
	memcpy(mv.matrix, g_sp.matrix[0].stack[g_sp.matrix[0].depth].m, sizeof(float) * 16);
	CL_Mat4f inv = mv.inverse();

	for (int r = 0; r < 4; r++)
	{
		g_sp.clipPlaneEye[r] =
			inv.matrix[r * 4 + 0] * pEq4[0] +
			inv.matrix[r * 4 + 1] * pEq4[1] +
			inv.matrix[r * 4 + 2] * pEq4[2] +
			inv.matrix[r * 4 + 3] * pEq4[3];
	}
}

//---------------------------------------------------------------------------
// Draws
//---------------------------------------------------------------------------

static bool PrepareToDraw()
{
	if (!SPInit()) return false;

	int variant = 0;
	if (g_sp.bTexture2D && g_sp.arrays[SP_ARRAY_TEXCOORD].bEnabled) variant |= SP_VARIANT_TEXTURE;
	if (g_sp.arrays[SP_ARRAY_COLOR].bEnabled) variant |= SP_VARIANT_VCOLOR;
	if (g_sp.bClipPlane) variant |= SP_VARIANT_CLIP;
	if (g_sp.bLighting && g_sp.bLight0) variant |= SP_VARIANT_LIGHT;

	SPProgram *pProg = NULL;

	if (g_sp.pActiveShader)
	{
		//a custom app shader replaces the ubershader entirely; the emulated
		//clip plane / lighting don't apply while it's active
		variant &= ~(SP_VARIANT_CLIP | SP_VARIANT_LIGHT);
		GLuint prog = g_sp.pActiveShader->GetProgram();
		if (g_sp.boundProgram != prog)
		{
			spUseProgram(prog);
			g_sp.boundProgram = prog;
		}
		int loc;
		if ((loc = g_sp.pActiveShader->GetStandardLoc(SP_STDLOC_PROJ)) >= 0) spUniformMatrix4fv(loc, 1, GL_FALSE, g_sp.matrix[1].stack[g_sp.matrix[1].depth].m);
		if ((loc = g_sp.pActiveShader->GetStandardLoc(SP_STDLOC_MV)) >= 0) spUniformMatrix4fv(loc, 1, GL_FALSE, g_sp.matrix[0].stack[g_sp.matrix[0].depth].m);
		if ((loc = g_sp.pActiveShader->GetStandardLoc(SP_STDLOC_COLOR)) >= 0) spUniform4fv(loc, 1, g_sp.color);
		g_sp.pActiveShader->ApplyCustomUniforms();
	}
	else
	{
		pProg = &g_sp.programs[variant];
		if (!pProg->program)
		{
			if (!BuildProgram(variant)) { g_sp.bInitFailed = true; return false; }
		}

		if (g_sp.boundProgram != pProg->program)
		{
			spUseProgram(pProg->program);
			g_sp.boundProgram = pProg->program;
		}

		//uniforms: cheap enough to set every draw for now; cache when profiling says to
		spUniformMatrix4fv(pProg->locProj, 1, GL_FALSE, g_sp.matrix[1].stack[g_sp.matrix[1].depth].m);
		spUniformMatrix4fv(pProg->locMV, 1, GL_FALSE, g_sp.matrix[0].stack[g_sp.matrix[0].depth].m);
		spUniform4fv(pProg->locColor, 1, g_sp.color);
		if (variant & SP_VARIANT_CLIP) spUniform4fv(pProg->locClipPlane, 1, g_sp.clipPlaneEye);
	}

	//attributes straight from the client-side arrays (fine on desktop GL and
	//GLES2; the WebGL flip will route these through a streaming VBO)
	const SPClientArray &v = g_sp.arrays[SP_ARRAY_VERTEX];
	spEnableVertexAttribArray(SP_ARRAY_VERTEX);
	spVertexAttribPointer(SP_ARRAY_VERTEX, v.size, v.type, GL_FALSE, v.stride, v.pData);

	if (variant & SP_VARIANT_TEXTURE)
	{
		const SPClientArray &t = g_sp.arrays[SP_ARRAY_TEXCOORD];
		spEnableVertexAttribArray(SP_ARRAY_TEXCOORD);
		spVertexAttribPointer(SP_ARRAY_TEXCOORD, t.size, t.type, GL_FALSE, t.stride, t.pData);
	}
	else
	{
		spDisableVertexAttribArray(SP_ARRAY_TEXCOORD);
	}

	if (variant & SP_VARIANT_VCOLOR)
	{
		const SPClientArray &c = g_sp.arrays[SP_ARRAY_COLOR];
		bool bNormalize = (c.type != GL_FLOAT);
		spEnableVertexAttribArray(SP_ARRAY_COLOR);
		spVertexAttribPointer(SP_ARRAY_COLOR, c.size, c.type, bNormalize ? GL_TRUE : GL_FALSE, c.stride, c.pData);
	}
	else
	{
		spDisableVertexAttribArray(SP_ARRAY_COLOR);
	}

	if (variant & SP_VARIANT_LIGHT)
	{
		//normal matrix: transpose(inverse(modelview)), computed per lit draw
		//(lit draws are rare enough that caching can wait)
		CL_Mat4f mv;
		memcpy(mv.matrix, g_sp.matrix[0].stack[g_sp.matrix[0].depth].m, sizeof(float) * 16);
		CL_Mat4f inv = mv.inverse();
		SPMat16 normalMat;
		for (int c = 0; c < 4; c++)
		{
			for (int r = 0; r < 4; r++) normalMat.m[c * 4 + r] = inv.matrix[r * 4 + c];
		}
		spUniformMatrix4fv(pProg->locNormalMat, 1, GL_FALSE, normalMat.m);
		spUniform4fv(pProg->locLightPosEye, 1, g_sp.lightPosEye);
		spUniform4fv(pProg->locLightAmbient, 1, g_sp.lightAmbient);
		spUniform4fv(pProg->locLightDiffuse, 1, g_sp.lightDiffuse);
		spUniform1f(pProg->locColorMaterial, g_sp.bColorMaterial ? 1.0f : 0.0f);

		const SPClientArray &nrm = g_sp.arrays[SP_ARRAY_NORMAL];
		if (nrm.bEnabled)
		{
			spEnableVertexAttribArray(SP_ARRAY_NORMAL);
			spVertexAttribPointer(SP_ARRAY_NORMAL, 3, nrm.type, GL_FALSE, nrm.stride, nrm.pData);
		}
		else
		{
			spDisableVertexAttribArray(SP_ARRAY_NORMAL);
			spVertexAttrib4f(SP_ARRAY_NORMAL, 0, 0, 1.0f, 0); //GL's default current normal
		}
	}
	else
	{
		spDisableVertexAttribArray(SP_ARRAY_NORMAL);
	}

	return true;
}

void SP_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
	if (!PrepareToDraw()) return;
	glDrawArrays(mode, first, count);
}

void SP_DrawElements(GLenum mode, GLsizei count, GLenum type, const void *pIndices)
{
	if (!PrepareToDraw()) return;
	glDrawElements(mode, count, type, pIndices);
}

//---------------------------------------------------------------------------
// Render targets
//---------------------------------------------------------------------------

unsigned int SP_CreateFrameBuffer(unsigned int glTextureID, int width, int height)
{
	if (!SPInit()) return 0;

	GLint prevBinding = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBinding);

	GLuint fbo = 0;
	spGenFramebuffers(1, &fbo);
	spBindFramebuffer(GL_FRAMEBUFFER, fbo);
	spFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTextureID, 0);
	GLenum status = spCheckFramebufferStatus(GL_FRAMEBUFFER);
	spBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevBinding);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		LogError("SP_CreateFrameBuffer: framebuffer incomplete (status 0x%X) for %dx%d texture", status, width, height);
		spDeleteFramebuffers(1, &fbo);
		return 0;
	}
	return fbo;
}

void SP_DestroyFrameBuffer(unsigned int frameBufferID)
{
	if (frameBufferID)
	{
		GLuint fbo = frameBufferID;
		spDeleteFramebuffers(1, &fbo);
	}
}

void SP_BindFrameBuffer(unsigned int frameBufferID, int width, int height)
{
	assert(g_sp.boundRenderTargetFBO == 0 && "render target nesting isn't supported");
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &g_sp.savedFBOBinding);
	glGetIntegerv(GL_VIEWPORT, g_sp.savedViewport);
	glGetIntegerv(GL_CULL_FACE_MODE, &g_sp.savedCullFaceMode);
	spBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
	glViewport(0, 0, width, height);
	g_sp.boundRenderTargetFBO = frameBufferID;
}

void SP_UnbindFrameBuffer()
{
	if (!g_sp.boundRenderTargetFBO) return;
	spBindFramebuffer(GL_FRAMEBUFFER, (GLuint)g_sp.savedFBOBinding);
	glViewport(g_sp.savedViewport[0], g_sp.savedViewport[1], g_sp.savedViewport[2], g_sp.savedViewport[3]);
	glCullFace(g_sp.savedCullFaceMode);
	g_sp.boundRenderTargetFBO = 0;
}

//---------------------------------------------------------------------------
// RTShader
//---------------------------------------------------------------------------

RTShader::RTShader()
{
	m_program = 0;
	m_customUniformCount = 0;
	for (int i = 0; i < 8; i++) m_standardLocs[i] = -1;
	GetShaderRegistry().push_back(this);
}

RTShader::~RTShader()
{
	//no GL calls here: global RTShaders die at process exit, after the context
	//is gone, and a driver entry point with no current context is not safe to call
	if (g_sp.pActiveShader == this) g_sp.pActiveShader = NULL;
	m_program = 0;

	std::vector<RTShader*> &shaders = GetShaderRegistry();
	for (size_t i = 0; i < shaders.size(); i++)
	{
		if (shaders[i] == this)
		{
			shaders.erase(shaders.begin() + i);
			break;
		}
	}
}

void RTShader::Kill()
{
	OnGLContextLost();
	m_vertexSource.clear(); //an explicit kill stays dead through context rebuilds
	m_fragmentSource.clear();
	m_customUniformCount = 0;
}

void RTShader::OnGLContextLost()
{
	if (!m_program) return;
	if (g_sp.pActiveShader == this) SetActiveShader(NULL);
	spDeleteProgram(m_program); //on a context that is already gone the name is just stale; harmless
	m_program = 0;
}

void RTShader::OnGLContextRestored()
{
	if (m_program || m_vertexSource.empty()) return; //still alive, or never loaded / explicitly killed

	if (!Build())
	{
		LogError("RTShader: relink after the GL context rebuild failed, the shader stays unloaded");
		return;
	}

	//same source, so the uniforms are all still there; only their locations may have moved
	for (int i = 0; i < m_customUniformCount; i++)
	{
		m_customUniforms[i].loc = spGetUniformLocation(m_program, m_customUniforms[i].name);
	}
}

bool RTShader::Load(const char *pVertexSource, const char *pFragmentSource)
{
	Kill();
	m_vertexSource = pVertexSource;
	m_fragmentSource = pFragmentSource;
	if (Build()) return true;

	//don't keep source that won't compile: a context rebuild would just retry it
	m_vertexSource.clear();
	m_fragmentSource.clear();
	return false;
}

bool RTShader::Build()
{
	if (!SPInit()) return false;

	string fragSrc = m_fragmentSource;
#ifndef C_GL_MODE
	//ES2 requires a default float precision in fragment shaders
	if (fragSrc.find("precision") == string::npos)
	{
		fragSrc = "precision mediump float;\n" + fragSrc;
	}
#endif

	GLuint vs = CompileShader(GL_VERTEX_SHADER, m_vertexSource.c_str());
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());
	if (!vs || !fs) return false;

	GLuint prog = spCreateProgram();
	spAttachShader(prog, vs);
	spAttachShader(prog, fs);
	//same attribute contract as the built-in ubershader, so the engine's draw
	//path needs no special cases
	spBindAttribLocation(prog, SP_ARRAY_VERTEX, "a_pos");
	spBindAttribLocation(prog, SP_ARRAY_TEXCOORD, "a_uv");
	spBindAttribLocation(prog, SP_ARRAY_COLOR, "a_color");
	spLinkProgram(prog);
	spDeleteShader(vs);
	spDeleteShader(fs);

	GLint status = 0;
	spGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (!status)
	{
		char log[2048];
		spGetProgramInfoLog(prog, sizeof(log), NULL, log);
		LogError("RTShader: link failed: %s", log);
		return false;
	}

	m_program = prog;
	m_standardLocs[SP_STDLOC_PROJ] = spGetUniformLocation(prog, "uProj");
	m_standardLocs[SP_STDLOC_MV] = spGetUniformLocation(prog, "uMV");
	m_standardLocs[SP_STDLOC_COLOR] = spGetUniformLocation(prog, "uColor");

	GLint texLoc = spGetUniformLocation(prog, "uTex");
	if (texLoc >= 0)
	{
		spUseProgram(prog);
		spUniform1i(texLoc, 0);
		spUseProgram(g_sp.boundProgram);
	}
	return true;
}

int RTShader::FindOrAddCustomUniform(const char *pName, int count)
{
	if (!m_program) return -1;

	for (int i = 0; i < m_customUniformCount; i++)
	{
		if (strncmp(m_customUniforms[i].name, pName, sizeof(m_customUniforms[i].name)) == 0)
		{
			m_customUniforms[i].count = count;
			return m_customUniforms[i].loc < 0 ? -1 : i;
		}
	}

	if (m_customUniformCount >= MAX_CUSTOM_UNIFORMS) { LogError("RTShader: too many custom uniforms"); return -1; }

	GLint loc = spGetUniformLocation(m_program, pName);
	if (loc < 0)
	{
		//warn once, then remember the miss so setting it every frame is cheap
		//and silent (a uniform the GLSL compiler optimized out ends up here too)
		LogMsg("RTShader: no uniform named %s in this shader, ignoring", pName);
	}
	CustomUniform &u = m_customUniforms[m_customUniformCount];
	strncpy(u.name, pName, sizeof(u.name) - 1);
	u.name[sizeof(u.name) - 1] = 0;
	u.loc = loc;
	u.count = count;
	m_customUniformCount++;
	return loc < 0 ? -1 : m_customUniformCount - 1;
}

void RTShader::SetUniform1f(const char *pName, float v)
{
	int i = FindOrAddCustomUniform(pName, 1);
	if (i >= 0) m_customUniforms[i].v[0] = v;
}

void RTShader::SetUniform4f(const char *pName, float x, float y, float z, float w)
{
	int i = FindOrAddCustomUniform(pName, 4);
	if (i < 0) return;
	m_customUniforms[i].v[0] = x; m_customUniforms[i].v[1] = y;
	m_customUniforms[i].v[2] = z; m_customUniforms[i].v[3] = w;
}

void RTShader::ApplyCustomUniforms()
{
	for (int i = 0; i < m_customUniformCount; i++)
	{
		if (m_customUniforms[i].count == 1) spUniform1f(m_customUniforms[i].loc, m_customUniforms[i].v[0]);
		else spUniform4fv(m_customUniforms[i].loc, 1, m_customUniforms[i].v);
	}
}

void SetActiveShader(RTShader *pShader)
{
	if (pShader && !pShader->IsLoaded())
	{
		LogError("SetActiveShader: shader isn't loaded, ignoring");
		return;
	}
	g_sp.pActiveShader = pShader;
}

RTShader * GetActiveShader()
{
	return g_sp.pActiveShader;
}

#endif // RT_SHADER_PIPELINE_AVAILABLE
