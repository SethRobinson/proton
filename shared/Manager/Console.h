//  ***************************************************************
//  Console - Creation date:  03/06/2009
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Console_h__
#define Console_h__

#include "Manager/VariantDB.h" //to get the sig/slot includes

using namespace std;

class Console
{
public:
	Console();
	virtual ~Console();

	void AddLine(std::string line);
	void SetMaxLines(unsigned int num) {m_maxLines = num;}
	string GetAsSingleString();
	void Clear();
	size_t GetTotalLines(){return m_log.size();}
	string GetLine(int index) {return m_log.at(index);}
	
	boost::signal<void()> m_sig_on_text_added; //if you want notification when text is added here, connect to this

private:

	unsigned int m_maxLines;
	std::deque<std::string> m_log;
};

#endif // Console_h__