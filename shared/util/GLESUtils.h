#ifndef GLESUtils_h__
#define GLESUtils_h__

#include "RenderUtils.h"
#include "MathUtils.h"

class SurfaceAnim;

const float C_MINIMUM_BMP_RECT_SIZE = 21.0f;

void GenerateSetPerspectiveFOV(float fovy, float aspect, float zNear, float zFar);
void DrawFilledSquare(float x, float y, float size=10, uint32 color = MAKE_RGBA(255,255,255,255), bool bCentered = false); //it just calls GenerateFillRect
void DrawFilledRect(float x, float y, float sizeX, float sizeY, uint32 color  = MAKE_RGBA(255,255,255,255));
void DrawRect(float x, float y, float width, float height, uint32 color = MAKE_RGBA(255,255,255,255), float lineWidth=1);
void DrawRect(CL_Vec2f &vPos, CL_Vec2f &vSize, uint32 color= MAKE_RGBA(255,255,255,255), float lineWidth=1);
void DrawRect(const CL_Rectf &r, uint32 color = MAKE_RGBA(255,255,255,255), float lineWidth = 1.0f);
void DrawRect(const rtRect &r, uint32 color  = MAKE_RGBA(255,255,255,255), float lineWidth  = 1.0f);
void DrawRect(const rtRect32 &r, uint32 color = MAKE_RGBA(255, 255, 255, 255), float lineWidth = 1.0f);
void DrawRect(const rtRectf &r, uint32 color  = MAKE_RGBA(255,255,255,255), float lineWidth = 1.0f);
void DrawFilledRect(const CL_Rectf &r, uint32 color);
void DrawFilledRect(rtRectf &r, uint32 color);
void DrawFilledRect(rtRect &r, uint32 color);
void DrawFilledRect(const rtRect32 &r, uint32 color);
void DrawFilledRect(CL_Vec2f vPos, CL_Vec2f vSize, uint32 color);
void DrawFilledBitmapRect(const CL_Rectf &r, uint32 middleColor, uint32 borderColor, SurfaceAnim *pSurf, bool bFillMiddleCloserToEdges = false );
void DrawFilledBitmapRect(rtRect &r, uint32 middleColor, uint32 borderColor, SurfaceAnim *pSurf, bool bFillMiddleCloserToEdges = false );
void DrawLine( GLuint rgba, float ax, float ay, float bx, float by, float lineWidth = 2.0f);
void DrawSurface2D( Surface* pSurf, GLint *pDestCoords, GLint *pSrcRect, unsigned int color);
void SetupOrtho();
void PrepareForGL();
void transform_point(GLfloat out[4], const GLfloat m[16], const GLfloat in[4]);
GLint gluUnProject(float winx, float winy, float winz,
			 const float modelMatrix[16], 
			 const float projMatrix[16],
			 const GLint viewport[4],
			 float *objx, float *objy, float *objz);
void RotateGLIfNeeded();
void PushRotationMatrix(float rotation, CL_Vec2f vRotatePt);
void PopRotationMatrix();
void DrawEllipse (const int segments, CL_Vec2f vPos, float radianWidth, float radiusHeight, bool vFilled, uint32 color = MAKE_RGBA(255,255,255,255));
void SetOrthoRenderSize(float x, float y, int screenOffsetX, int screenOffsetY);
void RemoveOrthoRenderSize();
float GetOrthoRenderSizeXf();
float GetOrthoRenderSizeYf();
void SetExtraScreenRotationDegrees(float degrees); // a way to add an additional rotation, I needed it for 4 player splitscreen with Irrlicht
float OrientationToDegrees(int orientation);
#endif // GLESUtils_h__
