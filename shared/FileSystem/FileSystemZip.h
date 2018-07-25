//  ***************************************************************
//  FileSystemZip - Creation date: 09/25/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FileSystemZip_h__
#define FileSystemZip_h__

#include "FileSystem.h"
#include "util/unzip/unzip.h"


//it's hella slow to find zips using just minizip, so we construct our own stl::map for faster lookups
struct ZipCacheEntry
{
	unz_file_pos m_filepos;
};

typedef std::map<string, ZipCacheEntry> zipCacheMap;


class FileSystemZip : public FileSystem
{
public:
	FileSystemZip();
	virtual ~FileSystemZip();
	bool Init(string apkFileName);
	vector<string> GetContents();
	void SetRootDirectory(string rootDir);


	virtual byte * Get( string fileName, int *pSizeOut);
	virtual StreamingInstance * GetStreaming(string fileName, int *pSizeOut); //pSizeOut currently always set to 0.  Returns null on fail. You must DELETE !
	virtual bool FileExists(string fileName);
	virtual int GetFileSize( string fileName );


private:

	void CacheIndex();
	unzFile m_uf;
	string m_rootDir;
	string m_zipFileName;
	zipCacheMap m_cache;
};

#endif // FileSystemZip_h__