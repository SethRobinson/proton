
#ifndef MathUtils_h__
#define MathUtils_h__

#include "Math/rtRect.h"
#include "Math/rtPlane.h"
#include "CRandom.h"
#include "ClanLib-2.0/Sources/API/Core/Math/vec2.h"
#include "ClanLib-2.0/Sources/API/Core/Math/vec3.h"
#include "ClanLib-2.0/Sources/API/Core/Math/rect.h"

#include "ClanLib-2.0/Sources/API/Core/Math/mat3.h"
#include "ClanLib-2.0/Sources/API/Core/Math/mat4.h"
#include "ClanLib-2.0/Sources/API/Core/Math/line_segment.h"
#include "ClanLib-2.0/Sources/API/Core/Math/circle.h"

using namespace std;

string PrintVector2(CL_Vec2f v);
string PrintVector3(CL_Vec3f v);
string PrintRect(CL_Rectf v);
string PrintRect(rtRectf v);
string PrintMatrix(CL_Mat4f v);
string PrintColor(uint32 color);

CL_Vec2f GetAlignmentOffset(const CL_Vec2f &vSize, eAlignment alignment);
void ApplyPadding(CL_Rectf *pR, const CL_Vec2f &vPadding);
void ApplyPadding(CL_Rectf *pR, const CL_Rectf &vPadding);
void ApplyOffset(CL_Rectf *pR, const CL_Vec2f &vOffset);
unsigned int EncryptPiece(byte *data, unsigned int size, int key);
unsigned int DecryptPiece(byte *data, unsigned int size, int key);
CL_Rectf ScaleRect(const CL_Rectf &r, float scale);
CL_Rectf ScaleRect2D(const CL_Rectf &r, CL_Vec2f vScale);

#define SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x))) //thanks to sol_hsa at http://sol.gfxile.net/interpolation/index.html
#define EASE_TO(x) (1 - (1 - (x)) * (1 - (x)))
#define EASE_FROM(x) ((x)*(x))
#define SMOOTHSTEP_INVERSE(x) pow( ((x)/0.5)-1,3)

// converting degrees to radians and vice versa
#define RT_PI (3.14159265359f)
#define TO_DEGREES(x) ((x)*180.0f/RT_PI)
#define TO_RADIANS(x) ((x)*RT_PI/180.0f)

bool CircleSegmentIntersect(CL_Vec2f C, float r, CL_Vec2f A, CL_Vec2f B, CL_Vec2f& P);
float ModNearestInt(float a, float b);
bool AnglesAreClose(float a, float b, float angleTolerance);
float GetAngleBetweenTwoAnglesRadians(float a, float b);
float AngleBetweenPoints(CL_Vec2f target,CL_Vec2f me);
float AngleBetweenPointsInDegrees(CL_Vec2f target,CL_Vec2f me);

void TurnAngleToward_Degrees(float *angle,float target,float amount);	// rotates angle towards target by amount, taking the shortest route

CL_Vec3f LerpVector(const CL_Vec3f &vOriginal, const CL_Vec3f &Target, float f_percent);
CL_Vec2f RotateGUIPoint(CL_Vec2f vPos, CL_Rectf r, float rotation, CL_Vec2f destRectSize = CL_Vec2f(0,0)); //used for calculating screen positions for a splitscreen mode
CL_Rectf RotateGUIRect(CL_Rectf vRect, CL_Rectf inputRect, float angle, CL_Vec2f destRectSize = CL_Vec2f(0,0)); //used for calculating screen positions for a splitscreen mode
#endif // MathUtils_h__
