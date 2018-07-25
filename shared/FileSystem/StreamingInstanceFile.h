//  ***************************************************************
//  StreamingInstanceFile - Creation date: 10/01/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef StreamingInstanceFile_h__
#define StreamingInstanceFile_h__

#include "StreamingInstance.h"

class StreamingInstanceFile: public StreamingInstance
{
public:
	StreamingInstanceFile();
	virtual ~StreamingInstanceFile();

	virtual bool Open(string fName);
	virtual bool IsFinished();
	virtual int Read(byte * pBufferOut, int maxBytesToRead);
	virtual void Close();
	virtual void SeekFromStart(int byteCount);
protected:
	

private:
	FILE *m_fp;
};

#endif // StreamingInstanceFile_h__