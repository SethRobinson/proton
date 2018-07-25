#ifndef PrimeSearch_h__
#define PrimeSearch_h__

/******************************************************************
PrimeSearch.h

This class enables you to visit each and every member of an array
exactly once in an apparently random order.

NOTE: If you want the search to start over at the beginning again -
you must call the Restart() method, OR call GetNext(true).

*******************************************************************/

// From the book GameCoding Complete by Mike McShaffry

#include "PlatformSetup.h"
#include "CRandom.h"

class PrimeSearch
{
public:

	PrimeSearch(int elements);
	PrimeSearch();

	int GetNext(bool restart=false); //returns # as 0 index based
	bool Done() { return (searches==*currentPrime); }
	void Restart() { currentPosition=0; searches=0; }
	void Init(int elements); //must be called before use, or pass in a num when constructing it

	static int prime_array[];

	int skip;
	int currentPosition;
	int maxElements;
	int *currentPrime;
	int searches;

	CRandom r;


};

#endif // PrimeSearch_h__
