//  ***************************************************************
//  ShaderPipeline - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

#include "PlatformPrecomp.h"

#ifdef RT_SHADER_PIPELINE_AVAILABLE

#include "ShaderPipeline.h"
#include "util/MathUtils.h" //for CL_Mat4f (used for the clip plane inverse only)

bool g_bShaderPipelineActive = false;

//---------------------------------------------------------------------------
// GL2 entry points + constants.  The vendored desktop gl.h is 1.1-only, so on
// Windows we declare and load what we need ourselves; GLES2 targets get the
// functions from their platform headers directly.
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

#if defined(_WIN32) && defined(C_GL_MODE)

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
typedef GLint (APIENTRY *PFNSPGETUNIFORMLOCATION)(GLuint program, const SPGLchar *name);
typedef void (APIENTRY *PFNSPUNIFORMMATRIX4FV)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY *PFNSPUNIFORM4FV)(GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY *PFNSPUNIFORM1I)(GLint location, GLint v0);
typedef void (APIENTRY *PFNSPENABLEVERTEXATTRIBARRAY)(GLuint index);
typedef void (APIENTRY *PFNSPDISABLEVERTEXATTRIBARRAY)(GLuint index);
typedef void (APIENTRY *PFNSPVERTEXATTRIBPOINTER)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (APIENTRY *PFNSPVERTEXATTRIB4F)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

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
static PFNSPGETUNIFORMLOCATION spGetUniformLocation = NULL;
static PFNSPUNIFORMMATRIX4FV spUniformMatrix4fv = NULL;
static PFNSPUNIFORM4FV spUniform4fv = NULL;
static PFNSPUNIFORM1I spUniform1i = NULL;
static PFNSPENABLEVERTEXATTRIBARRAY spEnableVertexAttribArray = NULL;
static PFNSPDISABLEVERTEXATTRIBARRAY spDisableVertexAttribArray = NULL;
static PFNSPVERTEXATTRIBPOINTER spVertexAttribPointer = NULL;
static PFNSPVERTEXATTRIB4F spVertexAttrib4f = NULL;

static bool LoadGL2FunctionPointers()
{
	#define SP_LOAD(var, name) var = (decltype(var)) wglGetProcAddress(name); if (!var) { LogError("ShaderPipeline: missing GL function %s", name); return false; }
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
	SP_LOAD(spGetUniformLocation, "glGetUniformLocation");
	SP_LOAD(spUniformMatrix4fv, "glUniformMatrix4fv");
	SP_LOAD(spUniform4fv, "glUniform4fv");
	SP_LOAD(spUniform1i, "glUniform1i");
	SP_LOAD(spEnableVertexAttribArray, "glEnableVertexAttribArray");
	SP_LOAD(spDisableVertexAttribArray, "glDisableVertexAttribArray");
	SP_LOAD(spVertexAttribPointer, "glVertexAttribPointer");
	SP_LOAD(spVertexAttrib4f, "glVertexAttrib4f");
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
#define spGetUniformLocation glGetUniformLocation
#define spUniformMatrix4fv glUniformMatrix4fv
#define spUniform4fv glUniform4fv
#define spUniform1i glUniform1i
#define spEnableVertexAttribArray glEnableVertexAttribArray
#define spDisableVertexAttribArray glDisableVertexAttribArray
#define spVertexAttribPointer glVertexAttribPointer
#define spVertexAttrib4f glVertexAttrib4f
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
enum { SP_VARIANT_TEXTURE = 1, SP_VARIANT_VCOLOR = 2, SP_VARIANT_CLIP = 4, SP_VARIANT_COUNT = 8 };

struct SPProgram
{
	GLuint program;
	GLint locProj, locMV, locColor, locClipPlane;
};

struct SPState
{
	bool bInitted;
	bool bInitFailed;

	SPMatrixStack matrix[2]; //0 = modelview, 1 = projection
	int curMatrix;           //index into matrix[]

	float color[4];
	SPClientArray arrays[SP_ARRAY_COUNT];
	bool bTexture2D;
	bool bClipPlane;
	float clipPlaneEye[4]; //already transformed to eye space, per the GL spec

	SPProgram programs[SP_VARIANT_COUNT];
	GLuint boundProgram;
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
	s += "void main() {\n"
		"	vec4 eyePos = uMV * vec4(a_pos.xyz, 1.0);\n"
		"	gl_Position = uProj * eyePos;\n";
	if (variant & SP_VARIANT_TEXTURE) s += "	v_uv = a_uv;\n";
	s += (variant & SP_VARIANT_VCOLOR) ? "	v_color = a_color;\n" : "	v_color = uColor;\n";
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
	if (variant & SP_VARIANT_TEXTURE)
	{
		spUseProgram(prog);
		spUniform1i(spGetUniformLocation(prog, "uTex"), 0);
		spUseProgram(g_sp.boundProgram);
	}
	return true;
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

	LogMsg("ShaderPipeline: initialized (%s)", (const char*)glGetString(GL_VERSION));
	return true;
}

//called once from BaseApp when the parm is seen, before any GL state exists
void SP_ResetState()
{
	memset(&g_sp, 0, sizeof(g_sp));
	for (int i = 0; i < 2; i++)
	{
		g_sp.matrix[i].depth = 0;
		MatIdentity(g_sp.matrix[i].stack[0]);
	}
	g_sp.curMatrix = 0;
	g_sp.color[0] = g_sp.color[1] = g_sp.color[2] = g_sp.color[3] = 1.0f;
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

void SP_Enable(GLenum cap)
{
	if (cap == GL_TEXTURE_2D) g_sp.bTexture2D = true;
	else if (cap == GL_CLIP_PLANE0) g_sp.bClipPlane = true;
	//GL_ALPHA_TEST / GL_LIGHTING / GL_LINE_SMOOTH / GL_COLOR_MATERIAL: no-ops here
	//(alpha test: the engine never sets glAlphaFunc, so GL's default GL_ALWAYS
	//made it a no-op on the fixed pipeline too)
}

void SP_Disable(GLenum cap)
{
	if (cap == GL_TEXTURE_2D) g_sp.bTexture2D = false;
	else if (cap == GL_CLIP_PLANE0) g_sp.bClipPlane = false;
}

void SP_Hint(GLenum target, GLenum mode) {}

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

	SPProgram &p = g_sp.programs[variant];
	if (!p.program)
	{
		if (!BuildProgram(variant)) { g_sp.bInitFailed = true; return false; }
	}

	if (g_sp.boundProgram != p.program)
	{
		spUseProgram(p.program);
		g_sp.boundProgram = p.program;
	}

	//uniforms: cheap enough to set every draw for now; cache when profiling says to
	spUniformMatrix4fv(p.locProj, 1, GL_FALSE, g_sp.matrix[1].stack[g_sp.matrix[1].depth].m);
	spUniformMatrix4fv(p.locMV, 1, GL_FALSE, g_sp.matrix[0].stack[g_sp.matrix[0].depth].m);
	spUniform4fv(p.locColor, 1, g_sp.color);
	if (variant & SP_VARIANT_CLIP) spUniform4fv(p.locClipPlane, 1, g_sp.clipPlaneEye);

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

#endif // RT_SHADER_PIPELINE_AVAILABLE
