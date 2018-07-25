//===============================================================================
//
// LinearParticle Copyright (c) 2006 Wong Chin Foo
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software in a
// product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
//===============================================================================


#ifndef L_DEFINATION_H
#define L_DEFINATION_H


#include <climits>
//#define L_USING_DOUBLE


#ifdef L_DEBUG_MODE
	#include <iostream>
#endif


#ifdef L_USING_DOUBLE
	#define L_REAL double
	#define L_SIN(x) sin(x)
	#define L_COS(x) cos(x)
	#define L_SQRT(x) sqrt(x)
	#define L_ATAN2(y,x) atan2(y,x)
	#define L_REAL_MIN DBL_MIN

#else
	#define L_REAL float
	#define L_SIN(x) sinf(x)
	#define L_COS(x) cosf(x)
	#define L_SQRT(x) sqrtf(x)
	#define L_ATAN2(y,x) atan2f(y,x)
	#define L_REAL_MIN 1.175494351e-38F
#endif


#define L_PARTICLE_TYPE_LIMIT 10

#define L_PI 3.1415926535898
#define L_INVERSED_PI 0.31830988618379
#define L_2PI 6.28318530717959


#define L_RADIAN_TO_DEGREE(x) (x*57.2957795130823)
#define L_DEGREE_TO_RADIAN(x) (x*0.0174532925199433)


#define L_INFINITE_LIFE INT_MAX


const L_REAL L_ONE_OVER_RAND_MAX_1 = 1 / (L_REAL)RAND_MAX; //don't convert this to RT_RAND_MAX, it breaks things
const L_REAL L_ONE_OVER_RAND_MAX_2 = 1 / ( L_REAL(RAND_MAX) + 1 ); //don't convert this to RT_RAND_MAX, it breaks things

// 0 <= random value <= 1
#define L_RAND_REAL_1() ( (L_REAL)rand() * L_ONE_OVER_RAND_MAX_1 )

// 0 <= random value < 1
#define L_RAND_REAL_2() ( (L_REAL)rand() * L_ONE_OVER_RAND_MAX_2 )

//when I replace L_Vector with CL_Vec2f there were a few functions missing, I worked around with the functions below -Seth

void linear_set2(CL_Vec2f &v, L_REAL magnitude, L_REAL radian);
void linear_set_magnitude(CL_Vec2f &v, L_REAL magnitude);
L_REAL linear_get_sqr_magnitude(CL_Vec2f &v);
L_REAL linear_get_radian(CL_Vec2f &v);

#endif
