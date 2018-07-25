#include "PlatformPrecomp.h"
#include "L_Defination.h"


void linear_set2(CL_Vec2f &v, L_REAL magnitude, L_REAL radian)
{
	v.x = magnitude * L_COS(radian);
	v.y = magnitude * L_SIN(radian);
}

void linear_set_magnitude(CL_Vec2f &v, L_REAL magnitude)
{
	v.normalize();
	v.x *= magnitude;
	v.y *= magnitude;
}

L_REAL linear_get_sqr_magnitude(CL_Vec2f &v)
{
	return v.x*v.x + v.y*v.y;
}

L_REAL linear_get_radian(CL_Vec2f &v)
{
	if( v.x != 0 && v.y != 0 )
		return L_ATAN2(v.y, v.x);

	return L_REAL_MIN;
}
