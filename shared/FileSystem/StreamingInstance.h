//  ***************************************************************
//  StreamingInstance - Creation date: 10/01/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef StreamingInstance_h__
#define StreamingInstance_h__

class StreamingInstance
{
public:
	StreamingInstance();
	virtual ~StreamingInstance();

	virtual bool Open(string fName) = 0;
	virtual bool ReadLineOfText(string &textOut);
	virtual bool ReadLineOfText( char *pBuffer, int bufferSize );
	virtual bool IsFinished() = 0;
	virtual int Read(byte * pBufferOut, int maxBytesToRead) = 0;
	virtual void SeekFromStart(int byteCount);
	
	//some helpers that use Read() to return certain types
	int32 ReadInt32();
	float ReadFloat32();

protected:
private:
};

#endif // StreamingInstance_h__