#ifndef rtPlane_h__
#define rtPlane_h__

#include "PlatformSetup.h"
#include "ClanLib-2.0/Sources/API/Core/Math/vec3.h"

#ifdef _CONSOLE
#if !defined GLAPI
#include "Renderer/GL/gl.h"
	#endif
#endif
struct rtPlane
{
	rtPlane(){};

	rtPlane(CL_Vec3f vPos, CL_Vec3f vNormal)
	{

		plane[0] = vNormal.x;
		plane[1] = vNormal.y;
		plane[2] = vNormal.z;
		plane[3] = vNormal.x * -vPos.x + vNormal.y * -vPos.y + vNormal.z * -vPos.z;
	}

	GLdouble plane[4];
};
#endif // rtPlane_h__
