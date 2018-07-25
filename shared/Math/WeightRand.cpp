#include "PlatformPrecomp.h"
#include "WeightRand.h"
#include <cassert>
#include "../MiscUtils.h"

CWeightRand::CWeightRand()
{
	m_b_needs_computation = true;
}

int CWeightRand::GetOdds(int i_choice)
{
	assert(i_choice >= 0 && i_choice < int(a_odds.size()) && "Invalid index!");
	return a_odds[i_choice];
}

void CWeightRand::Clear()
{
	a_odds.clear();
	a_computed_odds.clear();
	m_b_needs_computation = true;

}

bool CWeightRand::Save(FILE *fp)       
{
	//save our info out into a filestream
	int i_elements = (int)a_odds.size();
	fwrite(&i_elements, sizeof(i_elements), 1, fp);

	//next write out the elements

	for (int i=0; i < i_elements; i++)
	{
		fwrite(&a_odds[i], sizeof(int), 1, fp);
	}
	return true;
}

bool CWeightRand::Load(FILE *fp)       
{
	//save our info out into a filestream
	int i_elements = 0;

	fread(&i_elements, sizeof(i_elements), 1, fp);

	//next read in all the elements
	Clear();

	for (int i=0; i < i_elements; i++)
	{
		int i_odds;
		fread(&i_odds, sizeof(int), 1, fp);
		AddChoice(i, i_odds);
	}
	return true;
}


void CWeightRand::ModChoice(int index, int mod)
{
	AddChoice(index, GetOdds(index)+mod);
}

void CWeightRand::AddChoice(int i_index, int i_odds)
{
	if (i_index+1 > int(a_odds.size()))
	{
		a_odds.resize(i_index+1); //make sure it's big enough
		a_computed_odds.resize(i_index+1); //make sure it's big enough
	}

	a_odds[i_index] = i_odds;
	m_b_needs_computation = true;
}

void CWeightRand::ComputeOdds()
{
	//cycle through and figure out the percentages

	long l_total = 0;    
	int i;

	l_total=GetTotalOdds();

	for (i=0; i < int(a_odds.size()); i++)
	{
		a_computed_odds[i] = float(a_odds[i])/ float(l_total);
	}

	m_b_needs_computation = false; //all done, ready to compute crap
}

long CWeightRand::GetTotalOdds()
{
	long l_total = 0;    
	int i;

	for (i=0; i < int(a_odds.size()); i++)
	{
		l_total += a_odds[i];
	}

	return l_total;
}

int CWeightRand::CalcNumber(float f_rand)
{
	//scroll through and figure out which thing we chose

	float f_temp = 0;

	for (int i=0; i < int(a_computed_odds.size()); i++)
	{
		f_temp += a_computed_odds[i];
		if (f_temp > f_rand)
		{
			//this must be it.
			return i;
		}
	}

	return 0;
}

int CWeightRand::GetRandom(CRandom &rng)
{

	if (m_b_needs_computation) this->ComputeOdds();

	float f_rand = ((float)rng.rand() / (float)RT_RAND_MAX);

	return CalcNumber(f_rand);
}

int CWeightRand::GetRandom()
{

	if (m_b_needs_computation) this->ComputeOdds();

	float f_rand = (float)rand() / (float)RT_RAND_MAX;

	//LogMsg("Odds: %.6f", f_rand);
	return CalcNumber(f_rand);
}

string CWeightRand::TestRun(int runs)
{
	int i;

	vector<int> times;
	times.resize(a_odds.size());
	for(i=0;i<runs;i++)
	{
		int p=GetRandom();
		times[p]++;
	}
	string result="Did "+toString(runs)+" runs, results:\n";
	for(i=0;i<(int)a_odds.size();i++)
	{
		if(a_odds[i]>0)	// this one exists
		{
			result+="Item #"+toString(i)+": "+FloatToMoney(((float)times[i]/(float)runs)*100.0f,5)+"% ("+toString(times[i])+" times)\n";
		}
	}
	return result;
}

// ---------------- CMiniWeightRand

CMiniWeightRand::CMiniWeightRand()
{
	m_b_needs_computation = true;
}

int CMiniWeightRand::GetOdds(int i_choice)
{
	for(uint32 i=0;i<m_elements.size();i++)
		if(m_elements[i].num==i_choice)
			return m_elements[i].odds;
	return 0;
}

void CMiniWeightRand::Clear()
{
	m_elements.clear();
	m_b_needs_computation = true;
}

void CMiniWeightRand::AddChoice(int i_index, int i_odds)
{
	m_elements.push_back(MiniRandElement(i_index,i_odds));
	m_b_needs_computation = true;
}

void CMiniWeightRand::ComputeOdds()
{
	//cycle through and figure out the percentages
	long l_total = 0;    
	
	l_total=GetTotalOdds();
	for (uint32 i=0; i < m_elements.size(); i++)
		m_elements[i].computed_odds=float(m_elements[i].odds)/(float)l_total;
	
	m_b_needs_computation = false; //all done, ready to compute crap
}

int CMiniWeightRand::GetItemByPos(int pos)
{
	if(pos<0 || pos>=(int)m_elements.size())
		return 0;
	return m_elements[pos].num;
}

long CMiniWeightRand::GetTotalOdds()
{
	long l_total = 0;    
	
	for (uint32 i=0; i < m_elements.size(); i++)
	{
		l_total += m_elements[i].odds;
	}

	return l_total;
}

int CMiniWeightRand::CalcNumber(float f_rand)
{
	//scroll through and figure out which thing we chose
	if (m_b_needs_computation) ComputeOdds();
	float f_temp = 0;

	for (uint32 i=0; i < m_elements.size(); i++)
	{
		f_temp += m_elements[i].computed_odds;
		if (f_temp >= f_rand)
		{
			//this must be it.
			return m_elements[i].num;
		}
	}

	return 0;
}

int CMiniWeightRand::GetRandom(CRandom &rng)
{
	if (m_b_needs_computation) ComputeOdds();

	float f_rand = ((float)rng.rand() / (float)RT_RAND_MAX);

	return CalcNumber(f_rand);
}

int CMiniWeightRand::GetRandom()
{
	if (m_b_needs_computation) ComputeOdds();

	float f_rand = (float)rand() / (float)RT_RAND_MAX;

	return CalcNumber(f_rand);
}

string CMiniWeightRand::TestRun(int runs)
{
	int i;

	vector<int> times;
	times.resize(m_elements.size());
	for(i=0;i<runs;i++)
	{
		int p=GetRandom();
		times[p]++;
	}
	string result="Did "+toString(runs)+" runs, results:\n";
	for(i=0;i<(int)m_elements.size();i++)
	{
		result+="Item #"+toString(m_elements[i].num)+": "+FloatToMoney(((float)times[i]/(float)runs)*100.0f,5)+"% ("+toString(times[i])+" times)\n";
	}
	return result;
}