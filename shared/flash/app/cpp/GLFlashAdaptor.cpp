#if defined(RT_FLASH_TEST) || !defined(WIN32)

#include "GLFlashAdaptor.h"
#include "util/MiscUtils.h"
#include "util/MathUtils.h"
#include "Renderer/SoftSurface.h"
#include "Renderer/Surface.h"

#ifndef RT_FLASH_TEST
#include <AS3/AS3.h>
#else

#endif

//byte * g_flashBuffer = new unsigned char[64*1024];

GLclampf g_glClearColorR = 0;
GLclampf g_glClearColorB = 0;
GLclampf g_glClearColorG = 0;
GLclampf g_glClearColorA = 1;


class VertexPointerData
{
public:
	VertexPointerData()
	{
		pData = NULL;
		size = 0;
		type = 0;
		stride = 0;
		count = 0; //0 means "we don't know"
		vboCache = -1;
		bCacheValid = false;
	}

	void Setup(GLint newSize, GLenum newType, GLsizei newStride, const GLvoid *pointer, int vertCount)
	{
		count = vertCount;
		pData = (GLvoid*)pointer;
		size = newSize;
		type = newType;
		stride = newStride;
		bCacheValid = false;
	}

	void ClearVBOCache()
	{
		if (vboCache != -1)
		{
			glDeleteBuffers(1,&vboCache);
			vboCache = -1;
		}
		bCacheValid = false;
	}

	GLvoid *pData;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLuint count; //0 means "we don't know"
	GLuint vboCache;
	bool bCacheValid;

};

enum ePointerData
{
	C_PD_VERTEX,
	C_PD_TEX_UV,
	C_PD_VERT_COLORS,
	C_PD_NORMALS,

	//more above here
	C_PD_COUNT
};

VertexPointerData g_pointerData[C_PD_COUNT];


GLenum g_matrixMode = GL_MODELVIEW;
GLboolean g_depthMask = true;
GLenum g_CullFace = GL_BACK;
GLenum g_SourceBlend = GL_SRC_ALPHA;
GLenum g_DestBlend = GL_ONE_MINUS_SRC_ALPHA;
GLboolean g_GL_TEXTURE_2D =true;
GLboolean g_GL_DEPTH_TEST = true;
GLboolean g_GL_ALPHA_TEST = false;
GLboolean g_GL_BLEND = true;
GLboolean g_GL_LIGHTING = true;
GLboolean g_GL_CULL_FACE = true;
GLboolean g_GL_LINE_SMOOTH = false;
GLboolean g_GL_SCISSOR_TEST = false;
GLboolean g_GL_COLOR_MATERIAL = false;

GLboolean g_GL_TEXTURE_COORD_ARRAY = true;
GLboolean g_GL_VERTEX_ARRAY = true;
GLboolean g_GL_COLOR_ARRAY = false;
GLboolean g_GL_NORMAL_ARRAY = false;

CL_Rect g_scissorRect;

GLenum g_DEPTHFUNC = GL_LEQUAL;
GLuint g_lastBoundTexture = -1; //yes, it rolls over to positive, that's fine, we just don't want it to be near 0
GLuint g_lastTextureSentToFlash = -2;
GLuint g_lastBoundVBO = -1;
bool g_lastBoundVBOIsIndexBuffer = false;
vector<CL_Mat4f> g_projectionStack;
vector<CL_Mat4f> g_modelStack;

vector<CL_Mat4f> *g_pActiveStack = &g_modelStack;
vector<uint16> g_tempIndexBuff;
GLenum g_glFrontFace = GL_CCW; //this is the default for GLES, right?

class GLFlashLight
{
public:
	GLFlashLight()
	{
		m_bEnabled = false;
	}
	bool m_bEnabled;
};

const int C_GL_LIGHTCOUNT = 8;

GLFlashLight g_lightInfo[C_GL_LIGHTCOUNT];

bool GLFlashAdaptor_Initialize()
{
	LogMsg("initializing GLFlashAdaptor");
	g_projectionStack.reserve(2);
	g_projectionStack.push_back(CL_Mat4f::identity());

	g_modelStack.reserve(32);
	g_modelStack.push_back(CL_Mat4f::identity());
	g_scissorRect = CL_Rectf(0,0,GetScreenSizeXf(), GetScreenSizeYf());

	return true;
}

void GLResetFBOCaches()
{
	for (int i=0; i < C_PD_COUNT; i++)
	{
		g_pointerData[i].ClearVBOCache();
	}

}

void GLFlashAdaptor_Kill()
{
	LogMsg("Killing GLFlashAdaptor");

	GLResetFBOCaches();
}

GL_API void GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	g_glClearColorR = red;
	g_glClearColorG = green;
	g_glClearColorB = blue;
	g_glClearColorA = alpha;
	//LogMsg("Changing color to %.2f", g_glClearColorR);
}


GL_API void GL_APIENTRY glClear (GLbitfield mask)
{


	inline_as3(
		"import flash.display.Stage;\n"
		"import flash.display.Graphics;\n"
		"import flash.display3D.Context3D;\n"
		"import flash.display3D.Context3DClearMask;\n"
		"import com.rtsoft.GLES1Adaptor;\n"
		: :);

	if (mask & GL_COLOR_BUFFER_BIT && mask & GL_DEPTH_BUFFER_BIT)
	{

		inline_as3(
			"GLES1Adaptor.current.context.clear(%0, %1, %2, %3, 1.0, 0, Context3DClearMask.ALL);\n"
			: : "r"(g_glClearColorR),  "r"(g_glClearColorG),  "r"(g_glClearColorB),  "r"(g_glClearColorA)
			);
	} else
	{
		if (mask & GL_COLOR_BUFFER_BIT)
		{
			inline_as3(
				"GLES1Adaptor.current.context.clear(%0, %1, %2, %3, 1.0, 0, Context3DClearMask.COLOR);\n"
				: : "r"(g_glClearColorR),  "r"(g_glClearColorG),  "r"(g_glClearColorB),  "r"(g_glClearColorA)
				);

		}

		if (mask & GL_DEPTH_BUFFER_BIT)
		{
			inline_as3(
				"GLES1Adaptor.current.context.clear(%0, %1, %2, %3, 1.0, 0, Context3DClearMask.DEPTH);\n"
				: : "r"(g_glClearColorR),  "r"(g_glClearColorG),  "r"(g_glClearColorB),  "r"(g_glClearColorA)
				);

		}

	}



}

GL_API GLenum GL_APIENTRY glGetError (void)
{
	return 0;
}


void UpdateScissorRect()
{

	if (g_GL_SCISSOR_TEST)
	{
		//LogMsg("Enable scissoring: %s", PrintRect(g_scissorRect).c_str());

		//enable it
		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"import flash.geom.Rectangle;"
			"GLES1Adaptor.current.context.setScissorRectangle(new Rectangle(%0, %1, %2, %3));"
			: : "r"(g_scissorRect.left),  "r"(g_scissorRect.top),  "r"(g_scissorRect.get_width()),  "r"(g_scissorRect.get_height())
			);
	} else
	{
		//LogMsg("Disabling scissor");

		//disable
		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.context.setScissorRectangle(null);"
			: : 
		);
	}

}


void UpdateDepthTest()
{
	inline_as3(
		"import flash.display3D.Context3DCompareMode;"
		"import com.rtsoft.GLES1Adaptor;"
		: :
	);

	if (g_GL_DEPTH_TEST)
	{
		//we care about what's in  the depth buffer, but do we also write to it?

		if (g_depthMask)
		{
			inline_as3("GLES1Adaptor.current.context.setDepthTest(true, Context3DCompareMode.LESS_EQUAL);" : :);

		} else
		{
			inline_as3("GLES1Adaptor.current.context.setDepthTest(false, Context3DCompareMode.LESS_EQUAL);" : :);

		}
	} else
	{
		//we don't care what's in the depth buffer.  But should we write to it?
		if (g_depthMask)
		{
			//disable blending
			inline_as3("GLES1Adaptor.current.context.setDepthTest(true, Context3DCompareMode.ALWAYS);" : :);

		} else
		{
			//disable blending
			inline_as3("GLES1Adaptor.current.context.setDepthTest(false, Context3DCompareMode.ALWAYS);" : :);

		}
	}
}

void UpdateBlendModes()
{
	inline_as3(
		"import flash.display3D.Context3DBlendFactor;"
		"import com.rtsoft.GLES1Adaptor;"
		: :
	);

	if (!g_GL_BLEND)
	{
		//disable blending
		inline_as3("GLES1Adaptor.current.context.setBlendFactors( Context3DBlendFactor.ONE, Context3DBlendFactor.ZERO);" : :);
	} else
	{

		if (g_SourceBlend == GL_SRC_ALPHA && g_DestBlend == GL_ONE_MINUS_SRC_ALPHA)
		{
			inline_as3("GLES1Adaptor.current.context.setBlendFactors(Context3DBlendFactor.SOURCE_ALPHA, Context3DBlendFactor.ONE_MINUS_SOURCE_ALPHA);" : :);
		} else
			if (g_SourceBlend == GL_SRC_ALPHA && g_DestBlend == GL_ONE)
			{
				inline_as3("GLES1Adaptor.current.context.setBlendFactors(Context3DBlendFactor.SOURCE_ALPHA, Context3DBlendFactor.ONE);" : :);
			} else if (g_SourceBlend == GL_ONE &&  g_DestBlend ==  GL_ONE_MINUS_SRC_ALPHA)
			{
				inline_as3("GLES1Adaptor.current.context.setBlendFactors(Context3DBlendFactor.ONE, Context3DBlendFactor.ONE_MINUS_SOURCE_ALPHA);" : :);

			} else
			{
#ifdef _DEBUG
				LogError("Unhandled blend mode");
#endif
			}

	}
}

void UpdateCullingOptions()
{


	bool backCulling = g_CullFace == GL_BACK;



	if (g_glFrontFace == GL_CW)
	{
		//reverse it to fake reverse winding, which stage3d doesn't actually allow
		backCulling = !backCulling;
	}


	inline_as3(
		"import flash.display3D.Context3DTriangleFace;"
		"import com.rtsoft.GLES1Adaptor;"
		: :
	);

	if (g_GL_CULL_FACE)
	{
		if (!backCulling)
		{
			inline_as3("GLES1Adaptor.current.context.setCulling(Context3DTriangleFace.BACK);" : :);
		} else
		{
			inline_as3("GLES1Adaptor.current.context.setCulling(Context3DTriangleFace.FRONT);" : :);
		}
	} else
	{
		inline_as3("GLES1Adaptor.current.context.setCulling(Context3DTriangleFace.NONE);" : :);
	}

}


GL_API void GL_APIENTRY glFrontFace (GLenum mode)
{
	//Stage3d doesn't have a winding setting.. uhh.. so behind the scenes, we'll just reverse the culling to emulate it.

	if (g_glFrontFace != mode)
	{
		g_glFrontFace = mode;

		UpdateCullingOptions();
	}
}

void glEnableEx(GLenum cap, bool bSetting)
{
	switch (cap)
	{
	case GL_TEXTURE_2D: g_GL_TEXTURE_2D = bSetting; break;
	case GL_DEPTH_TEST:
		{
			if (g_GL_DEPTH_TEST != (GLboolean)bSetting)
			{

			}
			g_GL_DEPTH_TEST = bSetting;
			UpdateDepthTest();
		}
		break;

	case GL_LINE_SMOOTH: g_GL_LINE_SMOOTH = bSetting; break;

	case GL_SCISSOR_TEST: 
		{
			if (g_GL_SCISSOR_TEST != (GLboolean)bSetting)
			{
				g_GL_SCISSOR_TEST = bSetting; 
				UpdateScissorRect();
			}
		}
		break;

	case GL_ALPHA_TEST: g_GL_ALPHA_TEST = bSetting; break;
	case GL_BLEND:
		{
			if (g_GL_BLEND != (GLboolean)bSetting)
			{
				g_GL_BLEND = bSetting; 
				UpdateBlendModes();

			}
			break;
		}

	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:

		g_lightInfo[cap-GL_LIGHT0].m_bEnabled = (GLboolean)bSetting;

		if (cap-GL_LIGHT0 > 0)
		{
			LogMsg("GLFlashAdaptor ignoring light %d, only light 0 is supported right now",cap-GL_LIGHT0);
		}
		break;

	case GL_COLOR_MATERIAL: 
		g_GL_COLOR_MATERIAL = (GLboolean)bSetting;
		break;
	case GL_LIGHTING: g_GL_LIGHTING = bSetting; break;
	case GL_CULL_FACE:
		if (g_GL_CULL_FACE != (GLboolean)bSetting)
		{
			g_GL_CULL_FACE = bSetting;
			UpdateCullingOptions();
		}
		break;

	case GL_POLYGON_OFFSET_FILL:

		break;

	case GL_NORMALIZE:
		//we rescale all normals in the shader, ignore this
		break;

	case GL_RESCALE_NORMAL:
		//we rescale all normals in the shader, ignore this
		break;

	default:
		;
#ifdef _DEBUG
		LogMsg("Unhandled glEnable/Disable: enum %d", (int)cap);
#endif
		//	assert(!"Unhandled GLEnable/Disable");
	}
}

GL_API void GL_APIENTRY glDisable (GLenum cap)
{
	glEnableEx(cap, false);
}

GL_API void GL_APIENTRY glEnable (GLenum cap)
{
	glEnableEx(cap, true);
}

void glEnableClientStateEx(GLenum cap, bool bSetting)
{
	switch (cap)
	{
	case GL_TEXTURE_COORD_ARRAY: g_GL_TEXTURE_COORD_ARRAY = bSetting; break;
	case GL_VERTEX_ARRAY: g_GL_VERTEX_ARRAY = bSetting; break;
	case GL_COLOR_ARRAY: g_GL_COLOR_ARRAY = bSetting; break;
	case GL_NORMAL_ARRAY: g_GL_NORMAL_ARRAY = bSetting; break;
	default:
#ifdef _DEBUG
		LogMsg("Unhandled glEnableClientState/Disable: enum %d", cap);
#endif
		assert(!"Unhandled glEnableClientState/Disable");
	}
}

GL_API void GL_APIENTRY glEnableClientState (GLenum array)
{
	glEnableClientStateEx(array, true);
}

GL_API void GL_APIENTRY glDisableClientState (GLenum array)
{
	glEnableClientStateEx(array, false);
};

GL_API void GL_APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	//LogMsg("C wants %.2f, %.2f, %.2f, %.2f", red, green, blue, alpha);

	//OPTIMIZE:  Only set if changed.. but there is an issue with getting 0.997 uinstead of 1 due to rounding issues...
	inline_as3(
		"import com.rtsoft.GLES1Adaptor;"
		"GLES1Adaptor.current.SetPolyColor(%0, %1, %2, %3);"
		: : "r"(red),  "r"(green),  "r"(blue),  "r"(alpha)
		);
}

GL_API void GL_APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	glColor4f( (float(red)/255.0f),  (float(green)/255.0f) , (float(blue)/255.0f), (float(alpha)/255.0f));
}


GL_API void GL_APIENTRY glColor4x (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
	glColor4f( (float(red)/65536.0f),  (float(green)/65536.0f) , (float(blue)/65536.0f), (float(alpha)/65536.0f));
}

GL_API void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer)
{
	g_lastBoundVBOIsIndexBuffer = (target == GL_ELEMENT_ARRAY_BUFFER);
	g_lastBoundVBO = buffer;
}

void SendTextureToFlash(int texSlot, bool bTextureActive, GLuint texID)
{

#ifdef _DEBUG
	if (bTextureActive)
	{
		//	LogMsg("Sending texID %d to texSlot %d", texID, texSlot); 
	} else
	{
		//LogMsg("Clearing tex");
	}
#endif
	//OPTIMISE, state check this
	inline_as3(
		"import com.rtsoft.GLES1Adaptor;"
		"GLES1Adaptor.current.SetTexture(%0,%1,%2);"
		: : "r" (texSlot), "r" (bTextureActive), "r" (texID)
		);

}

GL_API void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *params)
{
	switch(pname)
	{
	case GL_DEPTH_BITS:
		*params = 8;
		break;

	case GL_VIEWPORT:
		params[0] = 0;
		params[1] = 0;
		params[2] = GetScreenSizeX();
		params[3] = GetScreenSizeY();
		break;

	default:

#ifdef _DEBUG
		LogMsg("Unhandled glGetIntegerv: %d", pname);
		assert(!"Unhandled glGetIntegerv");
#endif
		;
	}
}
GL_API void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *params)
{
	switch(pname)
	{
	case GL_MODELVIEW_MATRIX:
		*(CL_Mat4f*)params = g_modelStack.back();
		break;

	case GL_PROJECTION_MATRIX:
		*(CL_Mat4f*)params = g_projectionStack.back();
		break;

	default:
		LogMsg("Unhandled glGetFloatv parm: %d", pname);
		assert(!"glGetFloatv");
	}
}
GL_API const GLubyte * GL_APIENTRY glGetString (GLenum name)
{
	return (GLubyte *) "Proton GLESFlashAdaptor";
}
GL_API void GL_APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
	g_pActiveStack->back().translate_self(x,y,z);
}

GL_API void GL_APIENTRY glMatrixMode (GLenum mode)
{
	g_matrixMode = mode;
	switch(mode)
	{
	case GL_MODELVIEW:
		g_pActiveStack = &g_modelStack;
		break;

	case GL_PROJECTION:
		g_pActiveStack = &g_projectionStack;
		break;

	default:
		LogError("glMatrixMode> Unhandled mode!");
		assert(!"Unhandled matrix mode");
	}
}


GL_API void GL_APIENTRY glVertexPointerEx (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, int vertCount)
{
	if (size != 3)
	{
		LogError("glVertexPointer: Unhandled size");
	}
	if (type != GL_FLOAT)
	{
		LogError("glVertexPointer: Unhandled type (only know GL_FLOAT)");
	}

	if (stride == 0)
	{
		//guess
		stride = 3*sizeof(float);
	}

	g_pointerData[C_PD_VERTEX].Setup(size, type, stride, pointer, vertCount);
}


GL_API void GL_APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	glVertexPointerEx(size, type, stride, pointer, 0);
}

GL_API void GL_APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	if (stride == 0)
	{
		//guess
		stride = 2*sizeof(float);
	}

	g_pointerData[C_PD_TEX_UV].Setup(size, type, stride, pointer, 0);
}


GL_API void GL_APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	if (stride == 0)
	{
		//guess
		stride = 1*sizeof(int32);
	}
	g_pointerData[C_PD_VERT_COLORS].Setup(size, type, stride, pointer, 0);
}

GL_API void GL_APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
	if (stride == 0)
	{
		//guess
		stride = 3*sizeof(float);
	}

	g_pointerData[C_PD_NORMALS].Setup(4, type, stride, pointer, 0);
}

GL_API void GL_APIENTRY glLoadIdentity (void)
{
	g_pActiveStack->back() = CL_Mat4f::identity();
}

GL_API void GL_APIENTRY glLoadMatrixf (const GLfloat *m)
{
	g_pActiveStack->back() = *(CL_Mat4f*)m;
}

GL_API void GL_APIENTRY glPushMatrix (void)
{
	g_pActiveStack->push_back(g_pActiveStack->back());
}

GL_API void GL_APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	g_pActiveStack->back().multiply(CL_Mat4f::rotate(CL_Angle::from_degrees(angle), x,y,z, false));
}

GL_API void GL_APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z)
{
	g_pActiveStack->back().scale_self(x,y,z);
}

GL_API void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
	//assert(!"GLCommand not implemented yet");
	LogMsg("Ignoring glReadPixels command");
}

GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture)
{
	assert(target == GL_TEXTURE_2D);
	g_lastBoundTexture = texture;
};

GL_API void GL_APIENTRY glHint (GLenum target, GLenum mode)
{
	//	assert(!"GLCommand not implemented yet");
}

GL_API void GL_APIENTRY glDepthFunc (GLenum func)
{
	g_DEPTHFUNC = func;
}

GL_API void GL_APIENTRY glDepthMask (GLboolean flag)
{
	if (g_depthMask != (GLboolean) flag)
	{
		g_depthMask = flag;
		UpdateDepthTest();
	}
}

GL_API void GL_APIENTRY glCullFace (GLenum mode)
{
	if (mode != g_CullFace)
	{
		g_CullFace = mode;
		UpdateCullingOptions();
	}
}

GL_API void GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar)
{
	assert(!"GLCommand not implemented yet");
}

GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor)
{
	bool bChanged = false;
	if (sfactor != g_SourceBlend)
	{
		g_SourceBlend = sfactor;
		bChanged = true;
	}

	if (dfactor != g_DestBlend)
	{
		g_DestBlend = dfactor;
		bChanged = true;
	}

	if (bChanged) UpdateBlendModes();
}

GL_API void GL_APIENTRY glPopMatrix (void)
{
	if (g_pActiveStack->size() > 1)
	{
		g_pActiveStack->pop_back();
	} else
	{
		LogMsg("Error: glPopMatrix already at the bottom, your pops and pushes don't match up");
	}
}

GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param)
{

#ifdef _DEBUG
	LogMsg("Unhandled texture state");
#endif
}

GL_API void GL_APIENTRY glTexParameterx (GLenum target, GLenum pname, GLfixed param)
{
#ifdef _DEBUG
	//LogMsg("glTexParameterx unsupported");
#endif
}


GL_API void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param)
{
#ifdef _DEBUG
	//LogMsg("glTexParameterf unsupported");
#endif
}


GL_API void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params)
{
#ifdef _DEBUG
	//LogMsg("glTexParameterv unsupported");
#endif
}

GL_API void GL_APIENTRY glMultMatrixf (const GLfloat *m)
{
	g_pActiveStack->back().multiply(*(CL_Mat4f*)m);
}

GL_API void GL_APIENTRY glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
	g_pActiveStack->back().multiply(CL_Mat4f::ortho(left, right, bottom, top, zNear, zFar));
}

GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures)
{
#ifdef _DEBUG
	LogMsg("Deleting texture handle %d", textures[0]);
#endif

	for (int i=0; i < n; i++)
	{
		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.DeleteTexture(%0);"
			: :"r"(textures[i]));
	}
}

void AttachVBONormalsToShader(GLuint vboIndex, int offsetInt)
{
	inline_as3(
		"import com.rtsoft.GLES1Adaptor;"
		"GLES1Adaptor.current.AttachVBONormalsToShader(%0, %1);"
		: : "r"(vboIndex), "r"(offsetInt)
		);
}


void AttachVBOVertsToShader(GLuint vboIndex, int offsetInt)
{
	inline_as3(
		"import com.rtsoft.GLES1Adaptor;"
		"GLES1Adaptor.current.AttachVBOVertsToShader(%0, %1);"
		: : "r"(vboIndex), "r"(offsetInt)
		);
}

void AttachVBOColorsToShader(GLuint vboIndex, int offsetInt)
{
	inline_as3(
		"import com.rtsoft.GLES1Adaptor;"
		"GLES1Adaptor.current.AttachVBOColorsToShader(%0, %1);"
		: : "r"(vboIndex), "r"(offsetInt)
		);
}


void AttachVBOTexUVToShader(GLuint vboIndex, int offsetInt)
{
	inline_as3(
		"import com.rtsoft.GLES1Adaptor;"
		"GLES1Adaptor.current.AttachVBOTexUVToShader(%0, %1);"
		: : "r"(vboIndex), "r"(offsetInt)
		);

}


GL_API void GL_APIENTRY glBufferDataEx (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage, int vertCount, int dataSizePerVertexInts)
{
	if (GL_ELEMENT_ARRAY_BUFFER == target)
	{
		//index buffer.. slightly different
		vertCount = size/2;
		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.SetIndexVBO(%0, %1, %2);"
			: : "r"(g_lastBoundVBO), "r"(data), "r"(vertCount)
			);
	} else
	{
		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.SetVBO(%0, %1, %2, %3);"
			: : "r"(g_lastBoundVBO), "r"(data), "r"(vertCount),  "r"(dataSizePerVertexInts)
			);
	}
}


GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{

	//unsafe, fix later
	int vertCount = size/12;
	int dataSizePerVertexInts = 3;

	glBufferDataEx(target, size, data, usage, vertCount, dataSizePerVertexInts);
}

GL_API void GL_APIENTRY glBufferDataUV (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage,int vertCount, int dataSizePerVertexInts)
{
	//TODO: Don't do the real upload here, because we're stuck at guessing the contents...cache and do later when needed?
	inline_as3(
		"import com.rtsoft.GLES1Adaptor;"
		"GLES1Adaptor.current.SetVBO(%0, %1, %2, %3);"
		: : "r"(g_lastBoundVBO), "r"(data), "r"(vertCount),  "r"(dataSizePerVertexInts)
		);
}


GL_API void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers)
{
	//LogMsg("glGenBuffers creating vbo");

	for (int i=0; i < n; i++)
	{
		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"%0 = GLES1Adaptor.current.GenVBO();"
			:"=r"(buffers[i]) : );

#ifdef RT_FLASH_TEST
		//alt version for testing
		static int vboCounter = 0;
		buffers[i] = vboCounter++;
#endif
	}
}

GL_API void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers)
{
	if (buffers[0] == -1)
	{
		return;
	}
	//LogMsg("Deleting vbo handle %d", buffers[0]);

	for (int i=0; i < n; i++)
	{
		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.DeleteVBO(%0);"
			: :"r"(buffers[i]));
	}
}

GL_API void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures)
{
	for (int i=0; i < n; i++)
	{
		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"%0 = GLES1Adaptor.current.GenTexture();"
			:"=r"(textures[i]) : );

#ifdef RT_FLASH_TEST
		//alt version for testing
		static int texCounter = 0;
		textures[i] = texCounter++;
#endif
	}

}

GL_API void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
#ifdef _DEBUG
	LogMsg("Binding %d x %d at level %d, format %d", width, height, level, format);
#endif

	if (format != GL_RGBA && format != GL_RGB)
	{
		LogError("glTexImage2D : Unhandled pixel format on Flash side");
	}

	if (type != GL_UNSIGNED_BYTE)
	{
		LogError("glTexImage2D : Unhandled storageFormat format on Flash side");
	}

	if (!pixels)
	{
		LogMsg("glTexImage2D:  Sent in no data, ignoring.. not sure how we want to handle this");
		return;
	}
	if (format == GL_RGBA)
	{

		//convert to BGRA in-place because that's what Flash needs
		byte *pPixelData = (byte*)pixels;
		int pixelCount = width*height;
		byte temp;
		for (int i=0; i < pixelCount; i++)
		{
			temp = *pPixelData;
			*pPixelData = pPixelData[2];
			pPixelData[2] = temp;
			pPixelData += 4;
		}

		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.LoadTexture(%0,%1,%2,%3,%4);"
			: : "r"(g_lastBoundTexture) ,"r"(level),"r"(width), "r"(height), "r"(pixels)
			);
	} else
	{
		SoftSurface s;
		s.Init(width, height, SoftSurface::SURFACE_RGBA);
		byte *pNewSurf = s.GetPixelData();

		//need to convert RGB to RGBA I guess, and then flip to BGR

		byte *pPixelData = (byte*)pixels;
		int pixelCount = width*height;

		for (int i=0; i < pixelCount; i++)
		{
			pNewSurf[0] = pPixelData[2];
			pNewSurf[1] = pPixelData[1];
			pNewSurf[2] = pPixelData[0];
			pNewSurf[3] = 255; //alpha
			pPixelData += 3;
			pNewSurf += 4;
		}

		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.LoadTexture(%0,%1,%2,%3,%4);"
			: : "r"(g_lastBoundTexture) ,"r"(level),"r"(width), "r"(height), "r"(s.GetPixelData())
			);
	}


}

GL_API void GL_APIENTRY glLineWidth (GLfloat width)
{
	//LogMsg("Ignoring line width");
}

GL_API void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{

	/*

	_view.renderer.swapBackBuffer = false;
	_view.render();
	_view.stage3DProxy.context3D.drawToBitmapData(bitmapHolder.bitmapData);
	_view.renderer.swapBackBuffer = true;

	*/

	LogMsg("Ignoring glTexSubImage2D");
}

float * GenerateSmartBuffer(ePointerData pdType, int first, int count)
{
#ifdef _DEBUG
	if (!g_pointerData[pdType].pData)
	{
		LogMsg("GenerateSmartBuffer: error, no data for type %d", pdType);
	}
#endif

	float * pDataFinal = (float*)g_pointerData[pdType].pData;
	//handle the texture coords - unlike real GLES, we can connect multiple VBOs at once
	pDataFinal +=  ((g_pointerData[pdType].stride/4)*first); //skip to the relevant verts
	
	//actually put the data on the video card in stage3d, but only if it's not already there
	if (g_pointerData[pdType].bCacheValid)
	{
#ifdef _DEBUG
		//LogMsg("%d Re-using vbo cache %d",pdType, g_pointerData[pdType].vboCache );
#endif
		glBindBuffer(GL_ARRAY_BUFFER, g_pointerData[pdType].vboCache);

	} else
	{

		g_pointerData[pdType].ClearVBOCache();

		glGenBuffers(1, &g_pointerData[pdType].vboCache);
		glBindBuffer(GL_ARRAY_BUFFER, g_pointerData[pdType].vboCache);

#ifdef _DEBUG
		//LogMsg("%d Creating vbo cache %d",pdType, g_pointerData[pdType].vboCache );
#endif
		if (pdType == C_PD_TEX_UV)
		{
			glBufferDataUV(GL_ARRAY_BUFFER, g_pointerData[pdType].stride*count, pDataFinal, GL_STATIC_DRAW,
				count, g_pointerData[pdType].stride/4);
		} else
		{
			glBufferDataEx(GL_ARRAY_BUFFER, g_pointerData[pdType].stride*count, pDataFinal, GL_STATIC_DRAW,
				count, g_pointerData[pdType].stride/4);
		}

		g_pointerData[pdType].bCacheValid = true;

	}


	switch(pdType)
	{
	case C_PD_VERTEX:
		AttachVBOVertsToShader(g_pointerData[pdType].vboCache, 0);
		break;

	case C_PD_TEX_UV:
		AttachVBOTexUVToShader(g_pointerData[pdType].vboCache, 0);
		break;

	case C_PD_VERT_COLORS:
		AttachVBOColorsToShader(g_pointerData[pdType].vboCache, 0);
		break;

	case C_PD_NORMALS:
		AttachVBONormalsToShader(g_pointerData[pdType].vboCache, 0);
		break;

	default:

		LogMsg("GenerateSmartBuffer: Don't know how to create this kind of buffer");
	}

	return pDataFinal;
}

void CountUsedVertsInIndexBuffer(uint16 * pIndexBuff, int count, int *pLow, int *pHigh, int *pTotal)
{
	//look at used vert range
  *pHigh = -1;
  *pLow = 200000000;

	for (int i=0; i < count; i++)
	{
		if (pIndexBuff[i] > *pHigh)
		{
			*pHigh = pIndexBuff[i];
		}

		if (pIndexBuff[i] < *pLow)
		{
			*pLow = pIndexBuff[i];
		}

	}
	*pTotal = (*pHigh-*pLow)+1;
}

GL_API void GL_APIENTRY glDrawArraysEx (GLenum mode, GLint first, GLsizei count, GLenum indexType, const GLvoid *pIndexBuff)
{
	if (count == 0) return;

	int totalIndexes = -1;

	if (g_GL_SCISSOR_TEST)
	{
		if (!g_scissorRect.is_overlapped(CL_Rect(0,0,GetScreenSizeX(), GetScreenSizeY())))
		{
			//LogMsg("No point in drawing this, the whole thing is clipped");
			return;
		}
	}

	if (!g_pointerData[C_PD_VERTEX].pData)
	{
		LogError("glDrawArrays failed, vertexPointer is null");
		return;
	}

	bool bDebugThis = false;

	if (pIndexBuff)
	{
		totalIndexes = count;
		if (g_pointerData[C_PD_VERTEX].count != 0)
		{
			count = g_pointerData[C_PD_VERTEX].count; //the real count of verts we saved earlier, if applicable
		} else
		{
#ifdef _DEBUG
			//LogMsg("Warning: Using glDrawElements, should use glVertexPointerEx instead of glVertexPointer for speed and compatibility");
#endif
			//modify first to start at the real place the index buffer wants us to
			assert(indexType == GL_UNSIGNED_SHORT && "We only handle index buffers of this type right now");
			//first = ((uint16*)pIndexBuff)[0];	
		}

#ifdef _DEBUG
		int high, low, total;
		CountUsedVertsInIndexBuffer((uint16*)pIndexBuff, count, &low, &high, &total);
	
		//WORK - Flash DrawElements debug stuff
		LogMsg("glDrawElements, mode %d, first %d, indexCount %d (Verts: %d), scissor: %d (%s)", mode, first, totalIndexes, count, g_GL_SCISSOR_TEST, PrintRect(g_scissorRect).c_str());
		LogMsg("%d verts used in index, low %d, high %d.", total, low, high);
		
#endif
	} else
	{
		GLResetFBOCaches();
		totalIndexes = count;
#ifdef _DEBUG

		//LogMsg("glDrawArrays: mode %d, first %d, count %d, scissor: %d (%s)", mode, first, count, g_GL_SCISSOR_TEST, PrintRect(g_scissorRect).c_str());
#endif
	}


	CL_Mat4f mat = CL_Mat4f::multiply(g_modelStack.back(), g_projectionStack.back());

	GLfloat *pVerts = (GLfloat *)g_pointerData[C_PD_VERTEX].pData;
	pVerts += (first*3) + (g_pointerData[C_PD_VERTEX].stride*first);

	inline_as3("import com.rtsoft.GLES1Adaptor;");

	GLuint indexVBO = -1;

	switch (mode)
	{
	case GL_LINES:
		return;
		break;

	case GL_TRIANGLE_STRIP:
	case GL_TRIANGLE_FAN:
		{
		totalIndexes = 3 + (count-3)*3;
		
		if (!pIndexBuff)
		{
		} else
		{
			LogMsg("We don't support fans/strips using a custom index buffer, Seth, add it!");
			assert(!"Stop!");
		}

		g_tempIndexBuff.resize(totalIndexes);
		
		if (mode == GL_TRIANGLE_FAN)
		{
			//sent as..

			/*
        C         D

			A

		B		E

		with A being vert 0, and then N verts following, all being connected to A
			
			Received as:  0 1 2 -  3 4 5*3

			converted to:

			0 1 2, 0 2 3, 0 3 4, 0 4 5 etc
			*/

			//first, turn the first three verts into a normal tri
			int inIndex = first;
			int outIndex = first;
			g_tempIndexBuff[outIndex++]=inIndex++;
			g_tempIndexBuff[outIndex++]=inIndex++;
			g_tempIndexBuff[outIndex++]=inIndex;

			//convert the rest of the fan to normal tri's like this

			if (totalIndexes > 3)
			{
				while (1)
				{
					g_tempIndexBuff[outIndex++] = 0; //the shared vert of the fan
					g_tempIndexBuff[outIndex++] = inIndex++;
					g_tempIndexBuff[outIndex++] = inIndex;

					if (outIndex == totalIndexes) break;
				}
			}
	
		} else
		{
		
			
			if (totalIndexes != 6)
			{
//#ifdef _DEBUG
				LogMsg("glDrawArrays doesn't support GL_TRIANGLE_STRIP except for rects..fix.  (indexes were %d)", totalIndexes);
//#endif
				return;
			}

			/*
			Sent like this:
			0  1
			2  3

			Index created:
			2,0,3 3,0,1
			*/

			g_tempIndexBuff[0]=2;
			g_tempIndexBuff[1]=0;
			g_tempIndexBuff[2]=3;

			g_tempIndexBuff[3]=3;
			g_tempIndexBuff[4]=0;
			g_tempIndexBuff[5]=1;
		}
		}
		break;

	case GL_TRIANGLES:

		g_tempIndexBuff.resize(totalIndexes);
		if (pIndexBuff)
		{
			assert(indexType == GL_UNSIGNED_SHORT && "We only handle index buffers of this type right now");
		}

		if (pIndexBuff)
		{
			//they brought their own indexes.. this complicates things because we may or may not know how many verts are actually used
	
			if (g_pointerData[C_PD_VERTEX].count != 0)
			{
				//we know how many verts are in the buffers
				for (int i=0; i < totalIndexes; i++)
				{
					g_tempIndexBuff[i] = ((uint16*)pIndexBuff)[i];
				}
			} else
			{
				LogMsg("Warning: should use glSetPointerEx for your verts so we know how many there are, doing it the slow way for now");

				int high, low, total;
				CountUsedVertsInIndexBuffer((uint16*)pIndexBuff, count, &low, &high, &total);
				g_pointerData[C_PD_VERTEX].count = high+1;
				count = g_pointerData[C_PD_VERTEX].count;
				//we have no clue how many verts are used, other than by scanning the the index buffer they sent in
				for (int i=0; i < totalIndexes; i++)
				{
					g_tempIndexBuff[i] = ((uint16*)pIndexBuff)[i];
				}
			}
			
		} else
		{
		//were are constructing our own index buffer
			for (int i=0; i < totalIndexes; i++)
			{
				g_tempIndexBuff[i] = i;
			}
		}

		break;


	default:
		LogMsg("glDrawArrays, unhandled mode: %d", mode);
	}


	if (g_GL_TEXTURE_COORD_ARRAY)
	{
		assert(g_pointerData[C_PD_TEX_UV].pData);
		assert(g_pointerData[C_PD_TEX_UV].size == 2 && g_pointerData[C_PD_TEX_UV].type == GL_FLOAT && "We don't handle this kind ..");
		SendTextureToFlash(0, true, g_lastBoundTexture);
		GenerateSmartBuffer(C_PD_TEX_UV, first, count);
	} else
	{
		//no texturing
		SendTextureToFlash(0, false, 0);
	}

	if (g_GL_COLOR_ARRAY)
	{
		assert(g_pointerData[C_PD_VERT_COLORS].type == GL_UNSIGNED_BYTE && "We only support GL_UNSIGNED_BYTE colors (bgra)");
		//oh, so we want to color things with these, eh. Fine, goddamit
		GenerateSmartBuffer(C_PD_VERT_COLORS, first, count);
	}

	if (g_GL_NORMAL_ARRAY)
	{
		GenerateSmartBuffer(C_PD_NORMALS, first, count);
	}


	//I guess everybody will want verts...
	GenerateSmartBuffer(C_PD_VERTEX, first, count);

	//everybody needs these

	glGenBuffers(1, &indexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndexes*sizeof(GLshort), &g_tempIndexBuff[0], GL_STATIC_DRAW);

	CL_Mat4f modelMat =g_modelStack.back();

	//the real draw
	inline_as3(
		"GLES1Adaptor.current.DrawArraysMulti(%0, %1, %2, %3, %4, %5);\n"
		: :  "r"(&mat), "r"(&modelMat), "r"(indexVBO), "r"(g_GL_TEXTURE_COORD_ARRAY), "r" (g_GL_NORMAL_ARRAY), "r" (g_GL_COLOR_ARRAY)
		);

	//****** debug and show data
#if !defined(PLATFORM_FLASH)
	//#define SHOW_VERTS 
#endif

#ifdef SHOW_VERTS
	for (int i=0; i < rt_min(36, count); i++)
	{
		float *pVertPos = pVertexPointerFinal;
		pVertPos += g_tempIndexBuff[i]* (g_vertexPointerStride/4);
		//LogMsg("Vert %d: %s", i, PrintVector3(* (((CL_Vec3f*)g_texCoordPointer)+((g_texCoordPointerStride/4)*(first+i)))).c_str());
		LogMsg("Vert %d: %s", i, PrintVector3(*(CL_Vec3f*)pVertPos).c_str());

		if (g_GL_COLOR_ARRAY)
		{
			glColorBytes * pVertColor = (glColorBytes*)pVertexColorFinal;
			pVertColor += g_tempIndexBuff[i]* (g_colorPointerStride/4);
			//LogMsg("Vert %d: %s", i, PrintVector3(* (((CL_Vec3f*)g_texCoordPointer)+((g_texCoordPointerStride/4)*(first+i)))).c_str());
			LogMsg("Vert Color %d: %s", i, PrintGLColor(*pVertColor).c_str());

		}

		if (g_GL_NORMAL_ARRAY)
		{
			float *pVertNormals = pVertexNormalFinal;
			pVertNormals += g_tempIndexBuff[i]* (g_normalsPointerStride/4);
			//LogMsg("Vert %d: %s", i, PrintVector3(* (((CL_Vec3f*)g_texCoordPointer)+((g_texCoordPointerStride/4)*(first+i)))).c_str());
			LogMsg("Vert Normal %d: %s", i, PrintVector3(*(CL_Vec3f*)pVertNormals).c_str());

		}
	}

#endif

	glDeleteBuffers(1,&indexVBO);

}

GL_API void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
	glDrawArraysEx ( mode, 0, count, type, indices);
}

GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
	glDrawArraysEx ( mode, first, count, 0, NULL);
}


GL_API void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
	//the coords have 0,0 being the LOWER left instead of upper right, let's convert that first
	y = GetScreenSizeY()- (height+y);

	if (g_scissorRect.left == x && g_scissorRect.get_width() == width && g_scissorRect.top == y && g_scissorRect.get_height() == height) return;
	g_scissorRect = CL_Rect(x,y,x+width,y+height);
	UpdateScissorRect();
}
GL_API void GL_APIENTRY glClipPlanef (GLenum plane, const GLfloat *equation)
{
#ifdef _DEBUG
	LogMsg("glClipPlanef not handled...");
#endif
}
GL_API void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean *params)
{
	switch (pname)
	{
	case GL_SCISSOR_TEST: 
		{
			*params = g_GL_SCISSOR_TEST;	 
		}
		break;

	default:
		LogMsg("Unhandled glGetBooleanv: %d", pname);
	}

}

GL_API void GL_APIENTRY glShadeModel (GLenum mode)
{
#ifdef _DEBUG
	//LogMsg("glShadeModel unsupported");
#endif
}


GL_API void GL_APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params)
{
#ifdef _DEBUG
	//LogMsg("glMaterialfv unsupported");
#endif
}

GL_API void GL_APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param)
{
#ifdef _DEBUG
	//LogMsg("glMaterialf unsupported");
#endif
}

GL_API void GL_APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param)
{
#ifdef _DEBUG
	LogMsg("Ignoring glLightf");
#endif
}

GL_API void GL_APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params)
{

	int lightID = light - GL_LIGHT0;

	assert(lightID == 0 && "We don't support more than light 0");

	CL_Vec3f *pVec = (CL_Vec3f*)params;

	switch (pname)
	{
	case GL_POSITION:
		if (params[3] != 0)
		{
			LogMsg("Warning: You're setting a spotlight, but we only support directional lights");
		}
#ifdef _DEBUG
		LogMsg("Setting pos to %s", PrintVector3(*pVec).c_str());
#endif


		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.SetLightDir(%0, %1, %2, %3);"
			: : "r"(lightID ), "r"(pVec->x),  "r"(pVec->y),  "r"(pVec->z)
			);


		break;

	case GL_AMBIENT:

#ifdef _DEBUG
		LogMsg("Setting ambient to %s", PrintVector3(*pVec).c_str());
#endif

		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.SetLightAmbient(%0, %1, %2, %3);"
			: : "r"(lightID ), "r"(pVec->x),  "r"(pVec->y),  "r"(pVec->z)
			);

		break;

	case GL_DIFFUSE:

#ifdef _DEBUG
		LogMsg("Setting diffuse to %s", PrintVector3(*pVec).c_str());
#endif

		inline_as3(
			"import com.rtsoft.GLES1Adaptor;"
			"GLES1Adaptor.current.SetLightDiffuse(%0, %1, %2, %3);"
			: : "r"(lightID ), "r"(pVec->x),  "r"(pVec->y),  "r"(pVec->z)
			);

		break;


	default:
#ifdef _DEBUG
		LogMsg("glLightfv command unhandled");
#endif
		;

	}

}
GL_API void GL_APIENTRY glAlphaFunc (GLenum func, GLclampf ref)
{
#ifdef _DEBUG
//	LogMsg("Unhandled glAlphaFunc");
#endif
}

GL_API void GL_APIENTRY glFrustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
	g_pActiveStack->back().multiply(CL_Mat4f::frustum(left, right, bottom, top, zNear, zFar));
}

GL_API void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units)
{
#ifdef _DEBUG
	//LogMsg("Unhandled glPolygonOffset");
#endif
}

GL_API void GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
#ifdef _DEBUG
	LogMsg("Unhandled glCopyTexSubImage2D");
#endif
}

GL_API void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
#ifdef _DEBUG
	LogMsg("Unhandled glColorMask");
#endif
}

GL_API void GL_APIENTRY glActiveTexture (GLenum texture)
{
#ifdef _DEBUG
	if (texture != GL_TEXTURE0)
	{
		LogMsg("Ignoring glActiveTexture");
	}
#endif
}

GL_API void GL_APIENTRY glClientActiveTexture (GLenum texture)
{
#ifdef _DEBUG
	if (texture != GL_TEXTURE0)
	{
		LogMsg("Ignoring glClientActiveTexture");
	}
#endif
}

GL_API void GL_APIENTRY glLightModelf (GLenum pname, GLfloat param)
{
#ifdef _DEBUG
	LogMsg("Ignoring glLightModelf");
#endif
}
GL_API void GL_APIENTRY glLightModelfv (GLenum pname, const GLfloat *params)
{
#ifdef _DEBUG
	LogMsg("Ignoring glLightModelfv");
#endif

}

GL_API void GL_APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param)
{
#ifdef _DEBUG
	LogMsg("Ignoring glTexEnvi");
#endif
}

#endif