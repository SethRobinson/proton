//  ***************************************************************
//  StreamingInstanceZip - Creation date: 10/01/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef StreamingInstanceZip_h__
#define StreamingInstanceZip_h__

#include "StreamingInstance.h"
#include "util/unzip/unzip.h"
#include "FileSystemZip.h"

class StreamingInstanceZip : public StreamingInstance
{
public:
	StreamingInstanceZip();
	virtual ~StreamingInstanceZip();
	
	virtual bool Open(string fName);
	virtual bool IsFinished();
	virtual int Read(byte * pBufferOut, int maxBytesToRead);
	virtual void Close();
	virtual void SeekFromStart(int byteCount);

	bool Init(string zipFileName);
	
	void SetRootDirectory(string rootDir);
	bool OpenWithCacheEntry(ZipCacheEntry *pCacheEntry);
	
protected:

private:

	static const int BUFFER_SIZE = 1024*8;

	int FillBufferWithCachedBytes(byte *pBufferOut, int maxBytesToRead);
	bool OpenFileAtCurrentLocation();
	unzFile m_uf;
	unz_file_info m_file_info;

	byte m_buffer[BUFFER_SIZE];
	int m_bufferCurIndex;
	int m_bufferBytesLeft;
	bool m_bIsFinished;
	string m_rootDir;
	string m_zipFileName;
	FILE *m_pFile; //if the file is uncompressed, we can optionally convert it to a regular file handle internally to do 
	//seek operations etc, instead of mess with minizip/zlib at all
};

#endif // StreamingInstanceZip_h__