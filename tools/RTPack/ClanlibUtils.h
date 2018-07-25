#ifndef ClanlibUtils_h__
#define ClanlibUtils_h__

#include "PlatformSetup.h"

class TextScanner
{
public:

	TextScanner(std::string fName);
	~TextScanner();

	void Kill();

	bool LoadFile(std::string fName);

	std::string GetParmString(std::string label, int index);

	std::string GetMultipleLineStrings(std::string label, std::string token = "|");

	std::vector<std::string> m_lines;

private:
	char *m_pBuff;
	unsigned int m_size;
	int m_lastLine; //used during searches to remember the state
};

bool RunRTPackByCommandLine(std::string command_string);

#endif // ClanlibUtils_h__
