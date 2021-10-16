//  ***************************************************************
//  FFReader - Creation date: 12/31/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FFReader_h__
#define FFReader_h__

#include "BaseApp.h"

#pragma pack(push, 1)
struct FFFileInfo
{
	int offset;
	char name[13];
};
#pragma pack(pop)

class FFReader
{
public:
	enum eErrorType
	{
		ERROR_NONE,
		ERROR_LOW_MEM
	};
	
	
	FFReader();
	virtual ~FFReader();

	bool DoesFileExist(const string &fName, const string &fFirstFrame);
	void Init( const string &gamePath, const string &dmodGamePath, const string &baseDir, bool bUsingDinkPak);
	byte * LoadFileIntoMemory(string const &fName, int *pSizeout, const string &fFirstFrame); //you need to delete [] what this gives you on your own
	eErrorType GetLastError() {return m_error;}

private:

	void Kill();
	int GetFFRecordIndexFromFileName(const string &fName);
	byte * LoadFFIntoMemory(int index, int *pSizeOut);
	int GetNextFFIndex(int index);
	void SetError(eErrorType error);
	string m_gamePath;
	string m_dmodGamePath;
	string m_basePath;
	FILE *m_fp;
	bool m_bUsingBaseDinkFF;
	eErrorType m_error;
	string m_dmodBasePath;

	vector<FFFileInfo> m_fileHeader;

};

#endif // FFReader_h__