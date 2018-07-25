//  ***************************************************************
//  TarHandler - Creation date: 01/01/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************


//This can open a tar.bz2 file and extract all files in a single pass.  Doesn't support anything else!
//Works under windows and iphone
//Doesn't preserve filetimes



#ifndef TarHandler_h__
#define TarHandler_h__

#include "BaseApp.h"
#include "../bzip2/bzlib.h"

#pragma pack(push, 1)
/* our version of the tar header structure */
struct tar_header
{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char padding[12];
};

#pragma pack(pop)

#define C_BZIP_BUFFER_SIZE (1024*100)

class TarHandler
{
public:
	TarHandler();
	virtual ~TarHandler();

	bool OpenFile(const string &fName, const string &destPath);
	void Kill();
	bool ProcessChunk(); //called until processing is complete.  returning false means it's done or there was an error.
	
	enum eState
	{
		STATE_NONE,
		STATE_BZIPPING,
		STATE_ERROR,
		STATE_DONE
	};

	eState GetState() {return m_state;}
	int GetError() {return m_error;} //valid if GetState() returns STATE_ERROR
	int GetTotalBytes() { return 0;} //uh, I guess we actually don't know
	int GetBytesWritten() {return m_totalBytesWritten;}
	string GetFirstDirCreated() {return m_firstDirCreated;}
	void SetLimitOutputToSingleSubDir(bool bLimitIt);

private:

	bool uncompressStream ( FILE *zStream);
	void panic(char *pMessage);
	void OnBZIPError(int error);
	bool WriteBZipStream(byte *pData, int size);
	
	FILE *m_fp;
	string m_destPath;
	bool m_bLimitOutputToSingleSubDir;

	BZFILE* m_bzf;;
	int   m_bzerr, m_bzerr_dummy, m_ret, m_nread, m_streamNo, m_i;
	byte   * m_pBzipBuffer;
	byte   m_bzipReservedBuffer[BZ_MAX_UNUSED];
	int   m_bzipnUnused;
	void*   m_bzipunusedTmpV;
	byte *  m_bzipunusedTmp;
	eState m_state;
	int m_error; //valid if state is STATE_ERROR

	enum eTarState
	{
		TAR_STATE_FILLING_HEADER,
		TAR_STATE_WRITING_FILE,
		TAR_STATE_READING_BLANK_PART,
		TAR_STATE_FINISHED
	};
	
	//for untarring
	tar_header m_tarHeader;
	int m_tarHeaderBytesRead;
	FILE *m_fpOut;
	eTarState m_tarState;
	int m_tarFileOutBytesLeft;
	int m_totalBytesWritten;
	int m_bytesNeededToReachBlock;
	bool m_bForceLowerCaseFileNames;
	string m_firstDirCreated; //Dink needs this info to figure out which DMOD dir to run fast
};

#endif // TarHandler_h__