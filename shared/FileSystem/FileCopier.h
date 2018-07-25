//  ***************************************************************
//  FileCopier - Creation date: 07/06/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*
A non-essential little helper that let's you copy any file in the FileManager to a new location - why do it as a class?

Because I want to be able to do it in chunks, like to copy a 30 mb file from an android .jar (automatically mounted in the proton
file system) into a temp folder for some reason, and want to stream bits at a time.

Note: Will delete the file being copied if app is closed in the middle of the copy, or there is an error

Example of use:

FileCopier f;

if (!f.Init("source.dat", GetAppCachePath()+"dest.dat"))
	{
		LogMsg("Error.. source file doesn't exist or we can't open the dest file.  Statis is %d", f.GetStatus());
	}

while (1)
{
	f.Update(1024*10)); //perform the copy in 10 KB chunks
	if (f.GetStatus() == FileCopier::STATUS_FINISHED)
	{
		break;
	}

	if (f.GetStatus() != FileCopier::STATUS_OK)
	{
		LogMsg("Probably out of space");
		break;
	}
}


or, do:

f.InitAndCopy("source.dat", GetAppCachePath()+"dest.dat");

*/

class StreamingInstance;

#ifndef FileCopier_h__
#define FileCopier_h__

const int FILE_COPY_BUFFER_SIZE = 4096;
class FileCopier
{
public:
	FileCopier();
	virtual ~FileCopier();
	
	enum eStatus
	{
		STATUS_OK,
		STATUS_FINISHED,
		STATUS_ERROR_GENERIC,
		STATUS_ERROR_LOW_SPACE
	};

	bool Init(string fileNameSource, string fileNameDest); //you should include full paths
	bool InitAndCopy(string fileNameSource, string fileNameDest); //like above but does it in a single operation, no calling Update()
	bool Update(int maxBytesToWrite);
	eStatus GetStatus() {return m_status;}

	


protected:
	
	FILE *m_destFP;
	StreamingInstance *m_pSrcStreamInstance;
	eStatus m_status;
	int m_sourceSizeBytes;
	string m_destFileName;
	byte m_buffer[FILE_COPY_BUFFER_SIZE];
private:
};

#endif // FileCopier_h__