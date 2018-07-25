#include "PlatformPrecomp.h"
/*************************************************************************
PrimeSearch.cpp
**************************************************************************/
#include "PrimeSearch.h"

int PrimeSearch::prime_array[] =
{
	// choose the prime numbers to closely match the expected members
	// of the sets.

	2, 3, 5, 7,
	11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
	53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
	101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
	151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199,
	211, 223, 227, 229, 233, 239, 241,

	// begin to skip even more primes

	5003, 5101, 5209, 5303, 5407, 5501, 5623, 5701, 5801, 5903,
	6007, 6101, 6211, 6301, 6421, 6521, 6607, 6701, 6803, 6907,
	7001, 7103, 7207, 7307, 7411, 7507, 7603, 7703, 7817, 7901,
	8009, 8101, 8209, 8311, 8419, 8501, 8609, 8707, 8803, 8923, 9001,
	9103, 9203, 9311, 9403, 9511, 9601, 9719, 9803, 9901,

	// and even more
	10007, 10501, 11003, 11503, 12007, 12503, 13001, 13513, 14009, 14503,
	15013, 15511, 16033, 16519, 17011, 17509, 18013, 18503, 19001, 19501,
	20011, 20507, 21001, 21503, 22003, 22501, 23003, 23509, 24001, 24509

	// if you need more primes - go get them yourself!!!!

	// Create a bigger array of prime numbers by using this web site:
	// http://www.rsok.com/~jrm/printprimes.html
};


void PrimeSearch::Init(int elements)
{
	assert(elements>0 && "You can't do a this if you have 0 elements to search through, buddy-boy");

	maxElements = elements;

	int a = (rand()%13)+1;
	int b = (rand()%7)+1;
	int c = (rand()%5)+1;

	skip = (a * maxElements * maxElements) + (b * maxElements) + c;
	skip &= ~0xc0000000;            // this keeps skip from becoming too
	// large....

	Restart();

	currentPrime = prime_array;
	int s = sizeof(prime_array)/sizeof(prime_array[0]);

	// if this assert gets hit you didn't have enough prime numbers in your set.
	// Go back to the web site.
	assert(prime_array[s-1]>maxElements);

	while (*currentPrime < maxElements)
	{
		currentPrime++;
	}

	int test = skip % *currentPrime;
	if (!test)
		skip++;


}

PrimeSearch::PrimeSearch()
{
	//not initialized yet
	maxElements = 0;
}
PrimeSearch::PrimeSearch(int elements)
{
	Init(elements);
}

int PrimeSearch::GetNext(bool restart)
{

	assert(maxElements !=0 && "Hey, you need to call Init() on this first.");

	if (restart)
		Restart();

	if (Done())
		return -1;

	bool done = false;
	int nextMember = currentPosition;

	while (!done)
	{
		nextMember = nextMember + skip;
		nextMember %= *currentPrime;
		searches++;

		if (nextMember < maxElements)
		{
			currentPosition = nextMember;
			done = true;
		}
	}

	return currentPosition;
}
