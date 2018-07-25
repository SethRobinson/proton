#include "PlatformSetup.h"
#include "util/CRandom.h"

//Class to let you use random on objects that you want to appear at certain frequencies. 
//For instance, you could enter the letters of the alphabets with their frequency in english
//usage and get nice numbers for a word game. (E will pop up much more than Z, etc)

//Seth A Robinson (www.rtsoft.com)  7-21-2003


/*

//example of usage:

CWeightRand rand;
rand.AddChoice(0, 6);
//the second number is the 'weighting' It's compared to the other choices' weights.
//If you had only two choices, and they were weighted 1 and 10, the 10 one would
// get chosen 90% of the time.
rand.AddChoice(1, 6);
rand.AddChoice(2, 2); 

int i_choice = rand.GetRandom();

*/
#ifndef WeightRand_h__
#define WeightRand_h__

class CWeightRand
{
public:

	CWeightRand();
	void AddChoice(int i_index, int i_odds);
	int GetRandom();
	int GetRandom(CRandom &rng);
	void Clear(); //start over
	int GetOdds(int i_choice); //in case you want to know what the odds were on something
	void ModChoice(int index, int mod);
	bool Save(FILE *fp); //save to file stream
	bool Load(FILE *fp); //load from file stream
	unsigned int GetCount(){return (unsigned int)a_odds.size();}
	std::string TestRun(int runs=100000);
	long GetTotalOdds();
private:

	void ComputeOdds();
	int CalcNumber(float f_rand);

	vector<int> a_odds;  //save original
	vector<float> a_computed_odds; 
	bool m_b_needs_computation; //true if we've never computed or a new number was added
};

class MiniRandElement
{
	public:
		MiniRandElement(int n,int o) {num=n; odds=o; computed_odds=0;}
		int num;
		int odds;
		float computed_odds;
};
// use this instead when you have huge gaps - i.e. your list of choices includes #10, #938, and #8498. A regular CWeightRand will include zeroes for all the inbetweens,
// a CMiniWeightRand only has entries for the ones you include. It is slower, but much more memory efficient.
class CMiniWeightRand
{
public:
	CMiniWeightRand();
	void AddChoice(int i_index, int i_odds);
	int GetRandom();
	int GetRandom(CRandom &rng);
	void Clear(); //start over
	int GetOdds(int i_choice); //in case you want to know what the odds were on something
	int GetItemByPos(int pos);
	unsigned int GetCount(){return (unsigned int)m_elements.size();}
	std::string TestRun(int runs=100000);
	long GetTotalOdds();
private:

	void ComputeOdds();
	int CalcNumber(float f_rand);

	vector<MiniRandElement> m_elements;
	bool m_b_needs_computation; //true if we've never computed or a new number was added
};

#endif // WeightRand_h__