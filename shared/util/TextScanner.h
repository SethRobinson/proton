//  ***************************************************************
//  TextScanner - Creation date: 06/09/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************


/* Simple way to read parms from a text file

Here is an example of use:

Assume a simple text file, settings.txt holds the following:

name|Jeff
lives|3

You could parse it like this:

TextScanner t("settings.txt");

//Or, if you've downloaded the file, you can just pass in the memory location:
//TextScanner t(pPointerToMem);

if (!t.IsLoaded())
{
LogError("Oh damn");
}

//Note: returns a blank string if the parm is missing
string name = t.GetParmString("name", 1, "|");

//Note: the number says which section to return, you can have multiple delimiters in the same line
int lives = atoi( t.GetParmString("lives",1,"|") );


*/



#ifndef TextScanner_h__
#define TextScanner_h__

class TextScanner
{
public:

	TextScanner();
	TextScanner(const char *pCharArray);
	TextScanner(const string &fName);
	TextScanner(const string &fName, bool bAddBasePath );
	~TextScanner();

	void Kill();

	bool LoadFile(const string &fName, bool bAddBasePath = true);
	bool SaveFile(const string &fName, bool bAddBasePath = true);
	string GetParmString(string label, int index, string token = "|");
	string GetParmStringFromLine(int lineNum, int index, string token = "|");
	int GetParmIntFromLine( int lineNum, int index, string token = "|" );
	float GetParmFloatFromLine( int lineNum, int index, string token = "|");
	string GetMultipleLineStrings(string label, string token = "|");
	string GetLine(int lineNum); //0 based, returns "" if out of range
	void Replace( const string &thisStr, const string &thatStr );
	bool IsLoaded() {return !m_lines.empty();}
	bool SetupFromMemoryAddress(const char *pCharArray);
	bool SetupFromMemoryAddressRaw( const char *pCharArray, int size );
	void DeleteLine(int lineNum);
	void StripLeadingSpaces();
	string GetAll(); //it does trim whitespace
	string GetAllRaw(); //no trimming whitespace with this one
	int GetLineCount() {return (int)m_lines.size();}
	void DumpToLog(); //sends the entire contents to the log via LogMsg(), helpful when debugging
	vector<string> TokenizeLine(int lineNum, const string &theDelimiter = "|");
	void AppendToFile(string fileName, bool bAddBasePath = true);
	bool AppendFromMemoryAddress(const char *pCharArray);
	bool AppendFromMemoryAddressRaw( const char *pCharArray, int size );
	bool AppendFromString(const string lines);
	vector<string> m_lines;

private:

	int m_lastLine; //used during searches to remember the state
};


#endif // TextScanner_h__