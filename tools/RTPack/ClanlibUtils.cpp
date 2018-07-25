#include "ClanlibUtils.h"
#include "util/ResourceUtils.h"
#include "ClanLib/core.h"

#ifdef _WIN32
#include "process.h"
#endif

using namespace std;

#ifdef _WIN32
bool RunRTPackByCommandLine(string command_string)
{

	string exePath = CL_System::get_exe_path()+"RTPack.exe";

	TCHAR szDirectory[MAX_PATH] = "";

	if(!::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory))
	{
	}

	int result = (int)ShellExecute(NULL,NULL, exePath.c_str(), command_string.c_str() ,szDirectory, SW_SHOWDEFAULT  );

	if (result > 32) return true;

	MessageBox(NULL, "Error running command line of: ", command_string.c_str(), 0);
	//error!
	return false;
}
#endif


bool TextScanner::LoadFile( string fName )
{
	Kill();

	m_pBuff = (char*)LoadFileIntoMemory(fName, &m_size);
	if (!m_pBuff)
	{
		LogError("Can't find %s", fName.c_str());
		return false;
	}

	m_lines = CL_String::tokenize(m_pBuff, "\n", true);

	for (unsigned int i=0; i < m_lines.size(); i++)
	{
		StringReplace("\r", "", m_lines[i]);
		//StringReplace("\n", "", m_lines[i]);

	}

	return m_pBuff != NULL;
}

std::string TextScanner::GetParmString( string label, int index )
{
	if (!m_pBuff)
	{
		LogError("Load a file first");
		return "";
	}

	for (unsigned int i=0; i < m_lines.size(); i++)
	{
		if (m_lines[i].empty()) continue;
		vector<string> line = CL_String::tokenize(m_lines[i], "|", true);
		if (line[0] == label)
		{
			//found it
			return line[index];
		}
	}

	return "";
}

TextScanner::TextScanner( string fName )
{
	m_lastLine = 0;
	m_pBuff = 0;
	LoadFile(fName);
}

TextScanner::~TextScanner()
{
	Kill();
}

void TextScanner::Kill()
{
	SAFE_DELETE_ARRAY(m_pBuff);
}

std::string TextScanner::GetMultipleLineStrings( string label, string token )
{
	for (unsigned int i=m_lastLine; i < m_lines.size(); i++)
	{
		if (m_lines[i].empty()) continue;
		vector<string> line = CL_String::tokenize(m_lines[i], token, true);
		if (line[0] == label)
		{
			//found it
			m_lastLine = i+1;
			return m_lines[i];
		}
	}
	m_lastLine = 0; //reset it
	return "";
}