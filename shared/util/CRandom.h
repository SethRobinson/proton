#ifndef CRandom_h__
#define CRandom_h__

//A random generator based on the Mersenne Twister originally developed by Takuji Nishimura and Makoto Matsumoto.
// From the book GameCoding Complete by Mike McShaffry

#include "PlatformSetup.h"

/* Period parameters */
#define CMATH_N 624
#define CMATH_M 397
#define CMATH_MATRIX_A 0x9908b0df /* constant vector a */
#define CMATH_UPPER_MASK 0x80000000 /* most significant w-r bits */
#define CMATH_LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define CMATH_TEMPERING_MASK_B 0x9d2c5680
#define CMATH_TEMPERING_MASK_C 0xefc60000
#define CMATH_TEMPERING_SHIFT_U(y) (y >> 11)
#define CMATH_TEMPERING_SHIFT_S(y) (y << 7)
#define CMATH_TEMPERING_SHIFT_T(y) (y << 15)
#define CMATH_TEMPERING_SHIFT_L(y) (y >> 18)
 
class CRandom {
	// DATA
	unsigned int             rseed;
	unsigned long mt[CMATH_N];       /* the array for the state vector */
	int mti;                 /* mti==N+1 means mt[N] is not initialized */

	// FUNCTIONS
public:
	CRandom(void);

	unsigned int Random( unsigned int n );
	int RandomRange( int min, int max);

	float RandomRangeFloat(float rangeMin, float rangeMax)
	{
		return float(float(RandomRange(int(rangeMin*1000), int(rangeMax*1000))))/1000;
	}

	void   SetRandomSeed(unsigned int n);
	unsigned int     GetRandomSeed(void);
	void   Randomize(void);
	unsigned int rand();
};

#endif // CRandom_h__
