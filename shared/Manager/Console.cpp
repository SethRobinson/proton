#include "PlatformPrecomp.h"
#include "Console.h"

Console::Console()
{
	SetMaxLines(100);
}

Console::~Console()
{
}

void Console::Clear()
{
	m_log.clear();
}

void Console::AddLine( string line )
{
	//OutputDebugString(line.c_str());
	m_log.push_back(line);

	while (m_log.size() > m_maxLines)
	{
		m_log.pop_front();
	}

	m_sig_on_text_added();
}

string Console::GetAsSingleString()
{
	string combined ;

	for (unsigned int i=0; i < m_log.size(); i++)
	{
		combined +=m_log.at(i)+"\n";
	}

	return combined;
}