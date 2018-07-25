#include "PlatformPrecomp.h"
#include "GLESUtils.h"
#include "../Renderer/Surface.h"
#include "BaseApp.h"

float g_renderOrthoRenderSizeX;
float g_renderOrthoRenderSizeY;

void  GenerateFillRect( GLuint rgba, float x, float y, float w, float h );

void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
			   GLfloat centerx, GLfloat centery, GLfloat centerz,
			   GLfloat upx, GLfloat upy, GLfloat upz)

{
	GLfloat m[16];
	GLfloat x[3], y[3], z[3];
	GLfloat mag;

	/* Make rotation matrix */
	/* Z vector */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;

	mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);

	if (mag)
	{                   /* mpichler, 19950515 */
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	/* Y vector */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	/* X vector = Y cross Z */
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];

	/* Recompute Y = Z cross X */
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];

	/* mpichler, 19950515 */
	/* cross product gives area of parallelogram, which is < 1.0 for
	* non-perpendicular unit-length vectors; so normalize x, y here
	*/

	mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);

	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);

	if (mag) 
	{
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

#define M(row,col)  m[col*4+row]

	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;

#undef M

	glMultMatrixf(m);
	/* Translate Eye to Origin */
	glTranslatef(-eyex, -eyey, -eyez);
}
float g_extraScreenRotation = 0;
void SetExtraScreenRotationDegrees(float degrees)
{
	g_extraScreenRotation = degrees;
}


float OrientationToDegrees(int orientation)
{
	switch (GetOrientation())
	{
        case ORIENTATION_DONT_CARE:
            return 0;
            
	case ORIENTATION_PORTRAIT:
		return 0;
		break;
	case ORIENTATION_PORTRAIT_UPSIDE_DOWN:
		return 180;

	case ORIENTATION_LANDSCAPE_RIGHT:
		return 90;

	case ORIENTATION_LANDSCAPE_LEFT:
		return -90;
		break;
	}

	assert(0);
	return 0;
}
void RotateGLIfNeeded()
{
	if (GetBaseApp()->GetManualRotationMode())
	{
		float degrees = OrientationToDegrees(GetOrientation());
		glRotatef(degrees, 0.0f, 0.0f, 1.0f);

		
	}
    
    if (g_extraScreenRotation != 0)
    {
        glRotatef(-g_extraScreenRotation, 0,0,1);
    }

}

void RotateGLIfNeeded(CL_Mat4f &mat)
{

	if (GetBaseApp()->GetManualRotationMode())
	{
		float degrees = OrientationToDegrees(GetOrientation());
		mat = mat.rotate(CL_Angle(degrees, cl_degrees), 0,0,1.0f);
	}
    
    if (g_extraScreenRotation != 0)
    {
        mat = mat.rotate(CL_Angle(-g_extraScreenRotation, cl_degrees), 0,0,1.0f);
	}

}

void GenerateSetPerspectiveFOV(float fovy, float aspect, float zNear, float zFar)
{
	PrepareForGL();

	glMatrixMode(GL_PROJECTION);
	
	//we use Clanlib to setup our matrix manually because if we do it in HW we can't always extract it again as android gl doesn't support that
	glLoadIdentity();
	CL_Mat4f mat = CL_Mat4f::identity();

	RotateGLIfNeeded(mat);
	
	float xmin, xmax, ymin, ymax;

	ymax = zNear * float(tan(fovy * M_PI / 360.0f));
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	mat.multiply( CL_Mat4f::frustum(xmin, xmax, ymin, ymax, zNear, zFar));
	*GetBaseApp()->GetProjectionMatrix() = mat;
	//glFrustumf(xmin, xmax, ymin, ymax, zNear, zFar);
	glLoadMatrixf((GLfloat*)mat);

	glMatrixMode(GL_MODELVIEW);
}


void DrawFilledSquare( float x, float y, float size/*=10*/, uint32 color /*= MAKE_RGBA(255,255,255,255)*/, bool bCentered )
{
	if (bCentered)
	{
		x -= size/2;
		y -= size/2;
	}
	GenerateFillRect(color, x,y, size, size);
}

void DrawFilledRect(const CL_Rectf &r, uint32 color)
{
	GenerateFillRect(color, r.left, r.top, r.get_width(), r.get_height());
}

void DrawFilledRect(rtRectf &r, uint32 color)
{
	GenerateFillRect(color, r.left, r.top, r.GetWidth(), r.GetHeight());
}

void DrawFilledRect(rtRect &r, uint32 color)
{
	GenerateFillRect(color, (float)r.left, (float)r.top, (float)r.GetWidth(), (float)r.GetHeight());
}
void DrawFilledRect(const rtRect32 &r, uint32 color)
{
    GenerateFillRect(color, (float)r.left, (float)r.top, (float)r.GetWidth(), (float)r.GetHeight());
}


void DrawFilledRect(float x, float y, float sizeX, float sizeY, uint32 color)
{
	GenerateFillRect(color, x, y, sizeX, sizeY);
}

void DrawFilledRect(CL_Vec2f vPos, CL_Vec2f vSize, uint32 color)
{
	GenerateFillRect(color, vPos.x, vPos.y, vSize.x, vSize.y);
}

void DrawRect(const CL_Rectf &r, uint32 color, float lineWidth)
{
	DrawRect(r.left, r.top, r.get_width(), r.get_height(), color, lineWidth);
}

void DrawFilledBitmapRect(rtRect &r, uint32 middleColor,  uint32 borderColor, SurfaceAnim *pSurf, bool bFillMiddleCloserToEdges)
{
	DrawFilledBitmapRect( CL_Rectf((float)r.left,(float) r.top,(float) r.right, (float)r.bottom), middleColor, borderColor, pSurf, bFillMiddleCloserToEdges);
}

void DrawFilledBitmapRect(const CL_Rectf &r, uint32 middleColor, uint32 borderColor, SurfaceAnim *pSurf, bool bFillMiddleCloserToEdges )
{
	//assert(r.get_width()>=C_MINIMUM_BMP_RECT_SIZE && "GUIBox rect too small to work!");
	//assert(r.get_height()>=C_MINIMUM_BMP_RECT_SIZE && "GUIBox rect too small to work!");
	assert(pSurf->GetFramesX() >= 3 && pSurf->GetFramesY() >= 3 && "The surf needs to be setup as a 3x3 (or larger) first");
	// draw corners
	pSurf->BlitAnim(r.left,r.top,0,0, borderColor);
	pSurf->BlitAnim(r.right-10,r.top,2,0, borderColor);
	pSurf->BlitAnim(r.left,r.bottom-10,0,2, borderColor);
	pSurf->BlitAnim(r.right-10,r.bottom-10,2,2, borderColor);
	CL_Vec2f scale;
	// draw top and bottom
	scale.x=(r.get_width()-20)/10;
	scale.y=1;
	pSurf->BlitScaledAnim(r.left+10,r.top,1,0,scale,ALIGNMENT_UPPER_LEFT, borderColor);
	pSurf->BlitScaledAnim(r.left+10,r.bottom-10,1,2,scale,ALIGNMENT_UPPER_LEFT, borderColor);

	// draw sides
	scale.x=1;
	scale.y=(r.get_height()-20)/10;
	pSurf->BlitScaledAnim(r.left,r.top+10,0,1,scale,ALIGNMENT_UPPER_LEFT, borderColor);
	pSurf->BlitScaledAnim(r.right-10,r.top+10,2,1,scale,ALIGNMENT_UPPER_LEFT, borderColor);

	// fill middle
	if (bFillMiddleCloserToEdges)
	{
		DrawFilledRect(r.left+7,r.top+8,r.get_width()-15,r.get_height()-16,middleColor);
	} else
	{
		DrawFilledRect(r.left+10,r.top+10,r.get_width()-20,r.get_height()-20,middleColor);
	}
}

void DrawRect(const rtRect &r, uint32 color, float lineWidth)
{
	DrawRect((float)r.left, (float)r.top, (float)r.GetWidth(), (float)r.GetHeight(), color, lineWidth);
}

void DrawRect(const rtRect32 &r, uint32 color, float lineWidth)
{
	DrawRect((float)r.left, (float)r.top, (float)r.GetWidth(), (float)r.GetHeight(), color, lineWidth);
}

void DrawRect(const rtRectf &r, uint32 color, float lineWidth)
{
	DrawRect((float)r.left, (float)r.top, (float)r.GetWidth(), (float)r.GetHeight(), color, lineWidth);
}


void DrawRect(float x, float y, float width, float height, uint32 color, float lineWidth)
{
	//OPTIMIZE:  This could be done as a single GL call instead... or optimize GenerateFillRect to smartly handle render states
	GenerateFillRect(color, x, y, width, lineWidth); //top bar
	GenerateFillRect(color, x, (y+height)-lineWidth, width, lineWidth); //bottom bar
	GenerateFillRect(color, x, y+lineWidth, lineWidth, (height-lineWidth*2)); //left side
	GenerateFillRect(color, (x+width)-lineWidth, y+lineWidth, lineWidth, (height-lineWidth*2)); //left side
}


void DrawEllipse (const int segments, CL_Vec2f vPos, float radianWidth, float radiusHeight, bool vFilled, uint32 color)
{

	SetupOrtho();

	glPushMatrix();
	glTranslatef(vPos.x, vPos.y, 0.0);
	vector<float> vertices;
	vertices.resize(segments*2);
	glEnable (GL_LINE_SMOOTH);
	int count=0;
	for (GLfloat i = 0; i < 360.0f; i+=(360.0f/segments))
	{
		vertices[count++] = (float(cos(DEG2RAD(i)))*radianWidth);
		vertices[count++] = (float(sin(DEG2RAD(i)))*radiusHeight);
	}
	glColor4x( (color >>8 & 0xFF)*256,  (color>>16& 0xFF)*256, (color>>24& 0xFF)*256, (color&0xFF)*256);
	if (GET_ALPHA(color) != 255)
	{
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);

	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	
	glDisable(GL_TEXTURE_2D);	
	glVertexPointer (2, GL_FLOAT , 0, &vertices.at(0)); 
	glDrawArrays ((vFilled) ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, segments);
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	glEnable(GL_TEXTURE_2D);	
	if (GET_ALPHA(color) != 255)
	{
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);

	}
	glColor4x(1 << 16, 1 << 16, 1 << 16, 1 << 16);
	glPopMatrix();
}


void DrawRect(CL_Vec2f &vPos, CL_Vec2f &vSize, uint32 color, float lineWidth)
{
	DrawRect(vPos.x, vPos.y, vSize.x, vSize.y, color, lineWidth);
}

//old way using GL_LINES, but doesn't work on the Flash target

#if !defined(PLATFORM_FLASH) && !defined(PLATFORM_HTML5) && !defined(PLATFORM_ANDROID)
void  DrawLine( GLuint rgba,   float ax, float ay, float bx, float by, float lineWidth )
{
	SetupOrtho();

	glDisable( GL_TEXTURE_2D );

	glEnableClientState(GL_VERTEX_ARRAY);	
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	GLfloat	vertices[] = 
	{
		ax,ay, 0,
		bx, by, 0 
	};
	
		glLineWidth(lineWidth); 
		glEnable (GL_LINE_SMOOTH);
		//glDisable(GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

		glVertexPointer(3, GL_FLOAT, 0, vertices);
		//glColor4f(1, 1, 1, 1);
		glEnable( GL_BLEND );
		glColor4x( (rgba >>8 & 0xFF)*256,  (rgba>>16& 0xFF)*256, (rgba>>24& 0xFF)*256, (rgba&0xFF)*256);

		glDrawArrays(GL_LINES, 0, 2);
		glColor4x(1 << 16, 1 << 16, 1 << 16, 1 << 16);

		glDisable( GL_BLEND );
		glEnable( GL_TEXTURE_2D );
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
		CHECK_GL_ERROR();
}
#else
void  DrawLine( GLuint rgba,   float ax, float ay, float bx, float by, float lineWidth )
{
	SetupOrtho();
	g_globalBatcher.Flush();

	glDisable( GL_TEXTURE_2D );

	glEnableClientState(GL_VERTEX_ARRAY);	
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_CULL_FACE);

	static GLfloat	vertices[3*4];

	CL_Vec2f start = CL_Vec2f(ax, ay);
	CL_Vec2f end = CL_Vec2f(bx, by);

	float dx = ax - bx;
	float dy = ay - by;

	CL_Vec2f rightSide = CL_Vec2f(dy, -dx);

	if (rightSide.length() > 0) 
	{
		rightSide.normalize();
		rightSide *= lineWidth/2;
	}
	
	CL_Vec2f  leftSide =CL_Vec2f(-dy, dx);
	
	if (leftSide.length() > 0) 
	{
		leftSide.normalize();
		leftSide *= lineWidth/2;
	}

	CL_Vec2f one = leftSide + start;

	CL_Vec2f two = rightSide + start;

	CL_Vec2f three = rightSide + end;

	CL_Vec2f four = leftSide = end;

	vertices[0*3+0] = one.x; vertices[0*3+1] = one.y;
	vertices[1*3+0] = two.x; vertices[1*3+1] = two.y;
	vertices[2*3+0] = three.x; vertices[2*3+1] = three.y;
	vertices[3*3+0] = four.x; vertices[3*3+1] = four.y;

	//set the Z
	vertices[0*3+2] = 0;
	vertices[1*3+2] = 0;
	vertices[2*3+2] = 0;
	vertices[3*3+2] = 0;
	
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	//glColor4f(1, 1, 1, 1);
	glEnable( GL_BLEND );
	glColor4x( (rgba >>8 & 0xFF)*256,  (rgba>>16& 0xFF)*256, (rgba>>24& 0xFF)*256, (rgba&0xFF)*256);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glColor4x(1 << 16, 1 << 16, 1 << 16, 1 << 16);

	glEnable(GL_CULL_FACE);

	glDisable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	CHECK_GL_ERROR();
}

#endif
void  GenerateFillRect( GLuint rgba, float x, float y, float w, float h )
{
	SetupOrtho();

	//disable depth testing and depth writing
	glDisable( GL_TEXTURE_2D );
	
	if (GetBaseApp()->GetDisableSubPixelBlits())
	{
		//fix issue for cracks when scaling when 2d tile blits

		x = ceil(x);
		y = ceil(y);
		w = ceil(w);
		h = ceil(h);
	}


	//3 2
	//0 1
	
	GLfloat	vertices[] = {
		x,			y,			0.0,
		x + w,		y,			0.0,
		x +w,			y+h,		0.0,
		x ,		 y+h,		0.0 };

		glEnableClientState(GL_VERTEX_ARRAY);	
	
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		//glColor4f(1, 1, 1, 1);
		assert((rgba&0xFF)*256 != 0 && "Why send something with zero alpha?");
		glEnable( GL_BLEND );
		glDisable( GL_TEXTURE_2D );
		glEnable(GL_ALPHA_TEST);

		glColor4x( (rgba >>8 & 0xFF)*256,  (rgba>>16& 0xFF)*256, (rgba>>24& 0xFF)*256, (rgba&0xFF)*256);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);	

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glColor4x(1 << 16, 1 << 16, 1 << 16, 1 << 16);

		glDisable( GL_BLEND );
		glEnable( GL_TEXTURE_2D );
		glDisable(GL_ALPHA_TEST);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	CHECK_GL_ERROR();	
}

void PushRotationMatrix(float rotationDegrees, CL_Vec2f vRotatePt)
{
	glPushMatrix();
	glTranslatef(vRotatePt.x, vRotatePt.y, 0);
	glRotatef(rotationDegrees, 0, 0, 1);
}

void PopRotationMatrix()
{
	//Wrap this in case I do something more fancy later
	glPopMatrix();
}

bool g_OrthoRenderSizeActive = false;

void SetOrthoRenderSize(float x, float y, int screenOffsetX, int screenOffsetY)
{
	
	SetupOrtho();

	if (g_OrthoRenderSizeActive)
	{
		RemoveOrthoRenderSize();
	}
	
	g_OrthoRenderSizeActive = true;
	glMatrixMode(GL_PROJECTION);
	
	glPopMatrix();
	glPushMatrix();

	glLoadIdentity();
	RotateGLIfNeeded();
	g_renderOrthoRenderSizeX = x;
	g_renderOrthoRenderSizeY = y;
	
	float offset = 0;
	glOrthof( (-screenOffsetX)+offset, (x+offset), y+offset, screenOffsetY+offset,  -1, 1 );		
	//glTranslatef(0.5f, 0.5f, 0); //fixes a gl glitch where pixels don't know which side to be on

	glMatrixMode(GL_MODELVIEW);
	CHECK_GL_ERROR();
}


void RemoveOrthoRenderSize()
{
	g_globalBatcher.Flush();
	if (NeedsOrthoSet()) return;
	
	if (g_OrthoRenderSizeActive)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPushMatrix();

		glLoadIdentity();
		RotateGLIfNeeded();
		g_renderOrthoRenderSizeX = GetScreenSizeXf();
		g_renderOrthoRenderSizeY = GetScreenSizeYf();
	
		float offset = 0.0f;
		glOrthof( offset,  g_renderOrthoRenderSizeX+offset, g_renderOrthoRenderSizeY+offset, offset,  -1, 1 );		
		glMatrixMode(GL_MODELVIEW);
		CHECK_GL_ERROR();
	}

}

void SetupOrtho()
{
	if (!NeedsOrthoSet()) return;

	CHECK_GL_ERROR();
	g_globalBatcher.Flush();

	CHECK_GL_ERROR();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	glDisableClientState(GL_COLOR_ARRAY);	
	glDisableClientState(GL_NORMAL_ARRAY);

	CHECK_GL_ERROR();
	RotateGLIfNeeded();


	
	//Note:  We could setup our projection matrix upside down so the upper left would be 0,0, but.. then you have to wind your
	//triangles backwards (or keep switching to front face culling) which I find even more confusing than dealing with the y offsets.


	//When PrepareForGL() is called (removing our 2d ortho stuff), it gets turned back into normal back face culling

#ifdef _IRR_STATIC_LIB_
	glFrontFace( GL_CW );
	glCullFace(GL_BACK);
#else
	//glFrontFace( GL_CCW );
	glCullFace(GL_FRONT);
#endif
	

	g_renderOrthoRenderSizeX = GetScreenSizeXf();
	g_renderOrthoRenderSizeY = GetScreenSizeYf();
	
	float tempX = g_renderOrthoRenderSizeX;
	float tempY = g_renderOrthoRenderSizeY;

    CHECK_GL_ERROR();

	glOrthof( 0,  tempX, tempY, 0,  -1, 1 );		
	
	CHECK_GL_ERROR();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//disable depth testing and depth writing
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	SetOrthoModeFlag();
	CHECK_GL_ERROR();
}

void PrepareForGL()
{
	if (NeedsOrthoSet()) return; //not needed

	g_globalBatcher.Flush();
	CHECK_GL_ERROR();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	CHECK_GL_ERROR();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	CHECK_GL_ERROR();
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);   
	CHECK_GL_ERROR();
	glCullFace(GL_BACK);
	glLoadIdentity();
	CHECK_GL_ERROR();
	ResetOrthoFlag();
}

/*
* Transform a point (column vector) by a 4x4 matrix.  I.e.  out = m * in
* Input:  m - the 4x4 matrix
*         in - the 4x1 vector
* Output:  out - the resulting 4x1 vector.
*/
void transform_point(GLfloat out[4], const GLfloat m[16], const GLfloat in[4])
{
#define M(row,col)  m[col*4+row]
	out[0] =
		M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
	out[1] =
		M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
	out[2] =
		M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
	out[3] =
		M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}




/*
* Perform a 4x4 matrix multiplication  (product = a x b).
* Input:  a, b - matrices to multiply
* Output:  product - product of a and b
*/
static void
matmul(GLfloat * product, const GLfloat * a, const GLfloat * b)
{
	/* This matmul was contributed by Thomas Malik */
	GLfloat temp[16];
	GLint i;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define T(row,col)  temp[(col<<2)+row]

	/* i-te Zeile */
	for (i = 0; i < 4; i++) {
		T(i, 0) =
			A(i, 0) * B(0, 0) + A(i, 1) * B(1, 0) + A(i, 2) * B(2, 0) + A(i,
			3) *
			B(3, 0);
		T(i, 1) =
			A(i, 0) * B(0, 1) + A(i, 1) * B(1, 1) + A(i, 2) * B(2, 1) + A(i,
			3) *
			B(3, 1);
		T(i, 2) =
			A(i, 0) * B(0, 2) + A(i, 1) * B(1, 2) + A(i, 2) * B(2, 2) + A(i,
			3) *
			B(3, 2);
		T(i, 3) =
			A(i, 0) * B(0, 3) + A(i, 1) * B(1, 3) + A(i, 2) * B(2, 3) + A(i,
			3) *
			B(3, 3);
	}

#undef A
#undef B
#undef T
	memcpy(product, temp, 16 * sizeof(GLfloat));
}



/*
* Compute inverse of 4x4 transformation matrix.
* Code contributed by Jacques Leroy jle@star.be
* Return GL_TRUE for success, GL_FALSE for failure (singular matrix)
*/
static GLboolean
invert_matrix(const GLfloat * m, GLfloat * out)
{
	/* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS(a, b) { GLfloat *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

	GLfloat wtmp[4][8];
	GLfloat m0, m1, m2, m3, s;
	GLfloat *r0, *r1, *r2, *r3;

	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

	r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
		r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
		r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
		r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
		r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
		r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
		r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
		r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
		r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
		r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
		r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
		r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

	/* choose pivot - or die */
	if (fabs(r3[0]) > fabs(r2[0]))
		SWAP_ROWS(r3, r2);
	if (fabs(r2[0]) > fabs(r1[0]))
		SWAP_ROWS(r2, r1);
	if (fabs(r1[0]) > fabs(r0[0]))
		SWAP_ROWS(r1, r0);
	if (0.0 == r0[0])
		return GL_FALSE;

	/* eliminate first variable     */
	m1 = r1[0] / r0[0];
	m2 = r2[0] / r0[0];
	m3 = r3[0] / r0[0];
	s = r0[1];
	r1[1] -= m1 * s;
	r2[1] -= m2 * s;
	r3[1] -= m3 * s;
	s = r0[2];
	r1[2] -= m1 * s;
	r2[2] -= m2 * s;
	r3[2] -= m3 * s;
	s = r0[3];
	r1[3] -= m1 * s;
	r2[3] -= m2 * s;
	r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0) {
		r1[4] -= m1 * s;
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r0[5];
	if (s != 0.0) {
		r1[5] -= m1 * s;
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r0[6];
	if (s != 0.0) {
		r1[6] -= m1 * s;
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r0[7];
	if (s != 0.0) {
		r1[7] -= m1 * s;
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}

	/* choose pivot - or die */
	if (fabs(r3[1]) > fabs(r2[1]))
		SWAP_ROWS(r3, r2);
	if (fabs(r2[1]) > fabs(r1[1]))
		SWAP_ROWS(r2, r1);
	if (0.0 == r1[1])
		return GL_FALSE;

	/* eliminate second variable */
	m2 = r2[1] / r1[1];
	m3 = r3[1] / r1[1];
	r2[2] -= m2 * r1[2];
	r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3];
	r3[3] -= m3 * r1[3];
	s = r1[4];
	if (0.0 != s) {
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r1[5];
	if (0.0 != s) {
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r1[6];
	if (0.0 != s) {
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r1[7];
	if (0.0 != s) {
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}

	/* choose pivot - or die */
	if (fabs(r3[2]) > fabs(r2[2]))
		SWAP_ROWS(r3, r2);
	if (0.0 == r2[2])
		return GL_FALSE;

	/* eliminate third variable */
	m3 = r3[2] / r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
		r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];

	/* last check */
	if (0.0 == r3[3])
		return GL_FALSE;

	s = 1.0f / r3[3];		/* now back substitute row 3 */
	r3[4] *= s;
	r3[5] *= s;
	r3[6] *= s;
	r3[7] *= s;

	m2 = r2[3];			/* now back substitute row 2 */
	s = 1.0f / r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
		r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
		r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
		r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

	m1 = r1[2];			/* now back substitute row 1 */
	s = 1.0f / r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
		r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
		r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

	m0 = r0[1];			/* now back substitute row 0 */
	s = 1.0f / r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
		r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

	MAT(out, 0, 0) = r0[4];
	MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
	MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
	MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
	MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
	MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
	MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
	MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
	MAT(out, 3, 3) = r3[7];

	return GL_TRUE;

#undef MAT
#undef SWAP_ROWS
}



/* projection du point (objx,objy,obz) sur l'ecran (winx,winy,winz) */
GLint gluProject(GLfloat objx, GLfloat objy, GLfloat objz,
		   const GLfloat model[16], const GLfloat proj[16],
		   const GLint viewport[4],
		   GLfloat * winx, GLfloat * winy, GLfloat * winz)
{
	/* matrice de transformation */
	GLfloat in[4], out[4];

	/* initilise la matrice et le vecteur a transformer */
	in[0] = objx;
	in[1] = objy;
	in[2] = objz;
	in[3] = 1.0;
	transform_point(out, model, in);
	transform_point(in, proj, out);

	/* d'ou le resultat normalise entre -1 et 1 */
	if (in[3] == 0.0)
		return GL_FALSE;

	in[0] /= in[3];
	in[1] /= in[3];
	in[2] /= in[3];

	/* en coordonnees ecran */
	*winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
	*winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;
	/* entre 0 et 1 suivant z */
	*winz = (1 + in[2]) / 2;
	return GL_TRUE;
}



/* transformation du point ecran (winx,winy,winz) en point objet */
GLint gluUnProject(GLfloat winx, GLfloat winy, GLfloat winz,
			 const GLfloat model[16], const GLfloat proj[16],
			 const GLint viewport[4],
			 GLfloat * objx, GLfloat * objy, GLfloat * objz)
{
	/* matrice de transformation */
	GLfloat m[16], A[16];
	GLfloat in[4], out[4];

	/* transformation coordonnees normalisees entre -1 et 1 */
	in[0] = (winx - viewport[0]) * 2 / viewport[2] - 1.0f;
	in[1] = (winy - viewport[1]) * 2 / viewport[3] - 1.0f;
	in[2] = 2 * winz - 1.0f;
	in[3] = 1.0f;

	/* calcul transformation inverse */
	matmul(A, proj, model);
	invert_matrix(A, m);

	/* d'ou les coordonnees objets */
	transform_point(out, m, in);
	if (out[3] == 0.0)
		return GL_FALSE;
	*objx = out[0] / out[3];
	*objy = out[1] / out[3];
	*objz = out[2] / out[3];
	return GL_TRUE;
}


/*
* New in GLU 1.3
* This is like gluUnProject but also takes near and far DepthRange values.
*/

GLint gluUnProject4(GLfloat winx, GLfloat winy, GLfloat winz, GLfloat clipw,
			  const GLfloat modelMatrix[16],
			  const GLfloat projMatrix[16],
			  const GLint viewport[4],
			  GLclampf nearZ, GLclampf farZ,
			  GLfloat * objx, GLfloat * objy, GLfloat * objz,
			  GLfloat * objw)
{
	/* matrice de transformation */
	GLfloat m[16], A[16];
	GLfloat in[4], out[4];
	GLfloat z = nearZ + winz * (farZ - nearZ);

	/* transformation coordonnees normalisees entre -1 et 1 */
	in[0] = (winx - viewport[0]) * 2 / viewport[2] - 1.0f;
	in[1] = (winy - viewport[1]) * 2 / viewport[3] - 1.0f;
	in[2] = 2.0f * z - 1.0f;
	in[3] = clipw;

	/* calcul transformation inverse */
	matmul(A, projMatrix, modelMatrix);
	invert_matrix(A, m);

	/* d'ou les coordonnees objets */
	transform_point(out, m, in);
	if (out[3] == 0.0)
		return GL_FALSE;
	*objx = out[0] / out[3];
	*objy = out[1] / out[3];
	*objz = out[2] / out[3];
	*objw = out[3];
	return GL_TRUE;
}

float GetOrthoRenderSizeXf()
{
	return g_renderOrthoRenderSizeX;
}

float GetOrthoRenderSizeYf()
{
	return g_renderOrthoRenderSizeY;
}
