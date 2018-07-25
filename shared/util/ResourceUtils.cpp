#include "PlatformPrecomp.h"
#include "ResourceUtils.h"
#include "MiscUtils.h"

#ifndef C_NO_ZLIB
#include <zlib.h>
#endif

#ifdef PLATFORM_HTML5
#include "html5/HTML5Utils.h"
#endif

bool IsPowerOf2(int n) { return (!(n & (n - 1))); }

bool SaveToFile(const string &str, FILE *fp)
{

#ifdef RT_FORCE_32BIT_INTS_FOR_FILES
	int32 size = (int32)str.size();
	fwrite(&size, sizeof(int32), 1, fp);

#else
	int size = (int)str.size();
	fwrite(&size, sizeof(int), 1, fp);

#endif
	
	if (size> 0)
	{
		fwrite(str.c_str(), size, 1, fp);
	}
	return true;
}

bool LoadFromFile(string &str, FILE *fp)
{

#ifdef RT_FORCE_32BIT_INTS_FOR_FILES
	int32 size;
	fread(&size, sizeof(int32), 1, fp);

#else
	int size;
	fread(&size, sizeof(int), 1, fp);
#endif

	if (size > 0)
	{
		str.resize(size, ' ');
		fread(&str[0], size, 1, fp);
	}
	else
	{
		str.clear();
	}

	return true;
}

#ifdef RT_FORCE_32BIT_INTS_FOR_FILES

bool SaveToFile(int32 num, FILE *fp)
{
	fwrite(&num, sizeof(int32), 1, fp);
	return true;
}


#else
bool SaveToFile(int num, FILE *fp)
{
	//assert(!"Did you mean to pass in int32?  This is unsafe if you're loading/saving things as int, because of 32/64 bit issues");

	fwrite(&num, sizeof(int), 1, fp);
	return true;
}

#endif




bool SaveToFile(uint32 num, FILE *fp)
{
	fwrite(&num, sizeof(uint32), 1, fp);
	return true;
}

bool SaveToFile(float num, FILE *fp)
{
	fwrite(&num, sizeof(float), 1, fp);
	return true;
}

bool LoadFromFile(int32 &num, FILE *fp)
{
	fread(&num, sizeof(int32), 1, fp);
	return true;
}

bool LoadFromFile(float &num, FILE *fp)
{
	fread(&num, sizeof(float), 1, fp);
	return true;
}

bool LoadFromFile(bool &num, FILE *fp)
{
	fread(&num, sizeof(bool), 1, fp);
	return true;
}
bool LoadFromFile(uint32 &num, FILE *fp)
{
	fread(&num, sizeof(uint32), 1, fp);
	return true;
}

#ifndef CLANLIB_1
bool LoadFromFile(CL_Vec2f &num, FILE *fp)
{
	fread(&num, sizeof(CL_Vec2f), 1, fp);
	return true;
}

bool LoadFromFile(CL_Vec3f &num, FILE *fp)
{
	fread(&num, sizeof(CL_Vec3f), 1, fp);
	return true;
}

bool LoadFromFile(CL_Rectf &num, FILE *fp)
{
	fread(&num, sizeof(CL_Rectf), 1, fp);
	return true;
}
#endif


bool FileExists(const string &fName)
{

//this is so my RTPacker command line util will compile using this even though it doesn't use the filemanager

#if !defined(CLANLIB_1) && !defined(_CONSOLE)
	if (GetFileManager())
	{
		return GetFileManager()->FileExists(fName, false);
	} 

#endif
	FILE *fp = fopen( (fName).c_str(), "rb");
	if (!fp)
	{
		//file not found	
		return NULL;
	}

	fclose(fp);
	return true;

}

//up to you to use SAFE_DELETE_ARRAY
byte * DecompressRTPackToMemory(byte *pMem, unsigned int *pDecompressedSize)
{
	assert(IsAPackedFile(pMem));

#ifdef C_NO_ZLIB

	assert(!"Can't decompress, zlib disabled with C_NO_ZLIB flag");
return NULL;

#else
	rtpack_header *pHeader = (rtpack_header*)pMem;
	byte *pDeCompressed = zLibInflateToMemory( pMem+sizeof(rtpack_header), pHeader->compressedSize, pHeader->decompressedSize);
	*pDecompressedSize = pHeader->decompressedSize;
	return pDeCompressed;
#endif
}

byte * LoadFileIntoMemoryBasic(string fileName, unsigned int *length, bool bUseSavePath, bool bAddBasePath)
{
	*length = 0;

	if (bAddBasePath)
	{
		if (bUseSavePath)
		{
			fileName = GetSavePath() + fileName;
		} else
		{
			fileName = GetBaseAppPath() + fileName;
		}
	}
	
	FILE *fp = fopen(fileName.c_str(), "rb");
	if (!fp)
	{
		//file not found	
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	*length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	byte *pData = new byte[(*length) +1];
	
	if (!pData)
	{
		fclose(fp);
		*length = UINT_MAX; //signal a mem error
		return NULL;
	}
	pData[*length] = 0; 
	fread(pData, *length, 1, fp);
	fclose(fp);

	//we add an extra null at the end to be nice, when loading text files this can be useful

	return pData;
}

bool IsAPackedFile(byte *pFile)
{
	return (strncmp((char*)pFile, C_RTFILE_PACKAGE_HEADER, C_RTFILE_PACKAGE_HEADER_BYTE_SIZE) == 0);
}

bool IsARTFile(byte *pFile)
{
	//not so safe... so don't count on this
	return (strncmp((char*)pFile, "RT", 2) == 0);
}

bool CompressFile(string fName)
{
	unsigned int size;
	byte *pInput = LoadFileIntoMemoryBasic(fName, &size, false, false); //the basic means don't try to decompress it

	if (size >= C_RTFILE_PACKAGE_HEADER_BYTE_SIZE && IsAPackedFile(pInput))
	{
		SAFE_DELETE_ARRAY(pInput);
		LogMsg("%s is already packed, ignoring.", fName.c_str());
		return true; //well, no error at least
	}

	LogMsg("Compressing %s..", fName.c_str());

#ifdef C_NO_ZLIB

	assert(!"ZLIB disabled with C_NO_ZLIB flag, no can do, sir");
	return false;
#else
	int compressedSize; 
	byte *pCompressedFile = zlibDeflateToMemory(pInput, size, &compressedSize);
	SAFE_DELETE_ARRAY(pInput); //done with that part

	rtpack_header header =  BuildRTPackHeader(size, compressedSize);

	string finalFilename = fName;
	string ext = GetFileExtension(fName);

	if (ext != "rtfont" && ext != "rttex" && ext != "rtpak")
	{
		finalFilename = ModifyFileExtension(fName, "rtpak");
	}

	//save out the real file
	FILE *fp = fopen(finalFilename.c_str(), "wb");
	fwrite(&header, sizeof(rtpack_header), 1, fp);
	fwrite(pCompressedFile, compressedSize, 1, fp);
	SAFE_DELETE_ARRAY(pCompressedFile);
	fclose(fp);

	int totalBytes = sizeof(rtpack_header) + compressedSize;
	LogMsg("Compressed to %s.  (%d kb, %.0f%%%%)", finalFilename.c_str(),totalBytes/1024,  100*float(totalBytes)/float(size) );
#endif
	return true;
}



byte * CompressMemoryToRTPack(byte *pSourceMem, unsigned int sourceByteSize, unsigned int *pCompressedSizeOut)
{
	*pCompressedSizeOut = 0;
#ifdef _DEBUG
	if (sourceByteSize >= C_RTFILE_PACKAGE_HEADER_BYTE_SIZE && IsAPackedFile(pSourceMem))
	{
		assert(!"Uh, did you realize this is already packed?");
	}
#endif

	byte *pDest = 0;

#ifdef C_NO_ZLIB

	assert(!"ZLIB disabled with C_NO_ZLIB flag, no can do, sir");
	return false;
#else
	int compressedSize;
	byte *pCompressedFile = zlibDeflateToMemory(pSourceMem, sourceByteSize, &compressedSize);
	
	rtpack_header header = BuildRTPackHeader(sourceByteSize, compressedSize);


	int headerSize = sizeof(rtpack_header);

	//save out the real file... to memory
	pDest = new byte[compressedSize + headerSize + 1]; //the 1 is for a secret extra null on the end, helps with text processing

	//copy crap to memory
	memcpy(pDest, &header, headerSize);
	memcpy(pDest+ headerSize, pCompressedFile, compressedSize);
	pDest[headerSize + compressedSize] = 0; //add the null at the end
	SAFE_DELETE_ARRAY(pCompressedFile);
#endif

	*pCompressedSizeOut = compressedSize+headerSize;


	return pDest;
}


rtpack_header BuildRTPackHeader(int size, int compressedSize)
{
	rtpack_header header;
	memset(&header, 0, sizeof(rtpack_header));

	header.compressedSize = compressedSize;
	header.decompressedSize = size;
	header.compressionType = C_COMPRESSION_ZLIB;
	memcpy(header.rtFileHeader.fileTypeID, C_RTFILE_PACKAGE_HEADER, 6);
	header.rtFileHeader.version = C_RTFILE_PACKAGE_LATEST_VERSION;
	return header;
}

#ifndef C_NO_ZLIB


//you must SAFE_DELETE_ARRAY what it returns
byte * zlibDeflateToMemory(byte *pInput, int sizeBytes, int *pSizeCompressedOut)
{
	z_stream strm;
	int ret;

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	if (ret != Z_OK)
		return 0;


#define ZLIB_PADDING_BYTES (1024*5)

	byte *pOut = (byte*) new byte[sizeBytes + ZLIB_PADDING_BYTES];  //some extra padding in case the compressed version is larger than the decompressed version for some reason
	if (!pOut) return 0;
	strm.avail_in = sizeBytes;
	strm.next_in = pInput;
	strm.avail_out = sizeBytes + ZLIB_PADDING_BYTES;
	strm.next_out = pOut;

	ret = deflate(&strm, Z_FINISH);
	assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

	//	assert(ret == Z_STREAM_END);
	deflateEnd(&strm);
	*pSizeCompressedOut = strm.total_out;
	return pOut;

}

//you must SAFE_DELETE_ARRAY what it returns
byte * zLibInflateToMemory(byte *pInput, unsigned int compressedSize, unsigned int decompressedSize)
{
	int ret;
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return 0;
	byte *pDestBuff = new byte[decompressedSize+1]; //room for extra null at the end;
	if (!pDestBuff)
	{
		return 0;
	}
	pDestBuff[decompressedSize] = 0; //add the extra null, if we decompressed a text file this can be useful
	strm.avail_in = compressedSize;
	strm.next_in = pInput;
	strm.avail_out = decompressedSize;
	strm.next_out = pDestBuff;
	
	ret = inflate(&strm, Z_NO_FLUSH);
	if (! (ret == Z_OK || ret == Z_STREAM_END))
	{
		SAFE_DELETE_ARRAY(pDestBuff);
		return 0;
	}

	(void)inflateEnd(&strm);

	return pDestBuff;
}

#endif

void AppendStringToFile(const string filename, const string text)
{
	FILE *fp = NULL;

	if (GetPlatformID() == PLATFORM_ID_LINUX)
	{
		fp = fopen(filename.c_str(), "a+");

	} else
	{
		fp = fopen(filename.c_str(), "ab");

		if (!fp)
		{
			fp = fopen(filename.c_str(), "wb");
		}
	}

	if (!fp)
	{
		//Uhh.... bad idea, could create infinite loop
		//LogError("Unable to create/append to %s", text);
		return;
	}

	fwrite(text.c_str(), text.size(), 1, fp);

	fclose(fp);
}

//You must SAFE_DELETE_ARRAY the pointer it returns at some point!
byte * LoadFileIntoMemory(string fileName, unsigned int *p_ui_size, bool bUseSavePath)
{
	assert(p_ui_size && "You need to send in a valid int to be filled with the size, not a NULL.");

	byte *p_resource = LoadFileIntoMemoryBasic(fileName, p_ui_size, bUseSavePath, false);

	if (!p_resource)
	{
		return 0; //out of memory or something
	}

	return p_resource;
}

string SeparateStringSTL(string input, int index, char delimiter)
{
	//yes, this is pretty crap
	assert(input.size() < 4048 && "Fix this function..");
	char stInput[4048];
	if (SeparateString(input.c_str(), index, delimiter, stInput))
	{
		return stInput;
	} 

#ifdef _DEBUG
	LogMsg("Debug warning: SeparateStringSTL unable to find delimiter");
#endif
	return "";
}

bool SeparateString (const char str[], int num, char delimiter, char *return1) 
{
	int l = 0;
	return1[0] = 0;

	for (unsigned int k = 0; str[k] != 0; k++)
	{
		if (str[k] == delimiter)
		{
			l++;
			if (l == num+1)
				break;

			if (k < strlen(str)) strcpy(return1,"");
		}
		if (str[k] != delimiter)
			sprintf(return1, "%s%c",return1 ,str[k]);
	}

	if (l < num)
	{
		return1[0] = 0;
		return(false);
	}
	return true;
}

//snippet from Zahlman's post on gamedev:  http://www.gamedev.net/community/forums/topic.asp?topic_id=372125
void StringReplace(const std::string& what, const std::string& with, std::string& in)
{
	size_t pos = 0;
	size_t whatLen = what.length();
	size_t withLen = with.length();
	while ((pos = in.find(what, pos)) != std::string::npos)
	{
		in.replace(pos, whatLen, with);
		pos += withLen;
	}
}

int GetFileSize(const string &fName)
{
#ifdef _DEBUG
	//LogMsg("Getting filesize of %s", fName.c_str());
#endif
	FILE * file;
	int fileSizeBytes = -1;
	file = fopen(fName.c_str(),"r");
	if(file)
	{
		fseek(file, 0, SEEK_END);
		fileSizeBytes = ftell(file);
		fseek(file, 0, SEEK_SET);
		fclose(file);
	} else
	{
		LogMsg("Unable to open %s to get file size", fName.c_str());
	}
#ifdef _DEBUG
	//LogMsg("Filesize is %d", fileSizeBytes);
#endif

	return fileSizeBytes;
}

//add ipad to the filename if needed

string AddIPADToFileName(string file)
{
	
	if (!IsLargeScreen()) return file;
	size_t index = file.find_last_of('.');
	if (index == string::npos)
	{
		assert(!"Well, it doesn't have an extension to begin with");
		return file;
	}

	return file.substr(0, index) + "_ipad."+file.substr(index+1, file.length()-index);	
}

//replace lowercase iphone with ipad in string if needed
string ReplaceWithDeviceNameInFileName(const string &fName)
{
	
	if (IsIphone4Size)
	{
		string final = fName;
		StringReplace("iphone", "iphone4", final);
		return final;
	}

	if (IsTabletSize())
	{
		string final = fName;
		StringReplace("iphone", "ipad", final);
		return final;
	}

	return fName; //no change
}

string ReplaceWithLargeInFileName(const string &fName)
{
	if (!IsLargeScreen())
	{
		return fName; //no conversation done
	}

	string final = fName;
	StringReplace("iphone", "large", final);
	return final;
}

string ReplaceWithLargeInFileNameAndOSSpecific(const string &fName)
{
	if (!IsLargeScreen())
	{
		return fName; //no conversation done
	}

	string final = fName;
	
#ifdef PLATFORM_HTML5
	if (GetTouchesReceived() > 0)
	{
		//treat it like a big iphone, as it's a touch screen
		StringReplace("iphone", "large", final);
	}
#endif

	if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS
		|| GetEmulatedPlatformID() == PLATFORM_ID_OSX|| GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
	{
		StringReplace("iphone", "win", final);
	} else
	{
		//default, just large
		StringReplace("iphone", "large", final);
	}
	return final;
}

string ReplaceMP3( const string &fName)
{
	if (GetEmulatedPlatformID() != PLATFORM_ID_ANDROID)
	{
#ifndef _CONSOLE
		if (!GetAudioManager() || !GetAudioManager()->PreferOGG())
			return fName; //leave it as mp3
#else
		return fName; //leave it as mp3
#endif
		
			
	}
	
	string final = fName;

	StringReplace("mp3", "ogg", final);
	return final;
}


std::string StripColorCodes(const std::string text)
{
	std::string final;

	final.reserve(text.size());

	for (int i=0; i < (int)text.size(); i++)
	{
		if (text[i] == '`')
		{
			if (text[i+1] != 0) i++; //skip the next letter too
			continue;
		}
		final += text[i];

	}

	return final;
}


bool StringFromStartMatches(const std::string &line, const std::string textToMatch)
{
	for (uint32 i=0; i < textToMatch.size(); i++)
	{
		if (i >= line.length()) return false;
		if (line[i] != textToMatch[i]) return false;
	}
	return true;
}

bool StringFromEndMatches(const std::string &line, const std::string textToMatch)
{
	if (line.size() < textToMatch.size()) return false;
	int sizeOfTextToMatch = (int)strlen(textToMatch.c_str());
	if (strncmp( &(line.c_str()[line.size()-sizeOfTextToMatch]), textToMatch.c_str(), sizeOfTextToMatch) == 0) return true;

	return false;
}

void MemorySerialize( std::string &num, uint8 *pMem, int &offsetInOut, bool bWriteToMem)
{
	uint16 len;
	assert(num.length() < 1024*64);

	if (bWriteToMem)
	{
		len = (uint16) num.length();

		//copy how the len, up to 64k
		memcpy(&pMem[offsetInOut], &len, sizeof(len));
		offsetInOut += sizeof(len);

		//now copy the actual content
		memcpy(&pMem[offsetInOut], num.c_str(), len);
	
	} else
	{
		memcpy(&len, &pMem[offsetInOut], sizeof(len));
		offsetInOut += sizeof(len);

		num.resize(len);

		//trust me.
		memcpy((void*)num.c_str(), &pMem[offsetInOut], len);
	}
	offsetInOut += len;
}

void MemorySerializeStringEncrypted( std::string &num, uint8 *pMem, int &offsetInOut, bool bWriteToMem,int cryptID,char *secretCode)
{
	uint16 len;
	int codeLen=(int)strlen(secretCode);
	assert(codeLen>0 && codeLen<256);

	cryptID=cryptID%codeLen;	// cryptID is which position in secretCode you start at

	assert(num.length() < 1024*64);

	if (bWriteToMem)
	{
		len = (uint16) num.length();

		//copy how the len, up to 64k
		memcpy(&pMem[offsetInOut], &len, sizeof(len));
		offsetInOut += sizeof(len);

		//now copy the actual content, encrypted
		for(int i=0;i<len;i++)
		{
			uint8 b=(uint8)num.c_str()[i];
			b=b^secretCode[cryptID++];
			if(cryptID>=codeLen)
				cryptID=0;
			pMem[offsetInOut++]=b;
		}
	} 
	else
	{
		memcpy(&len, &pMem[offsetInOut], sizeof(len));
		offsetInOut += sizeof(len);

		num.resize(len);

		for(int i=0;i<len;i++)
		{
			uint8 b=pMem[offsetInOut++];
			num[i]=b^secretCode[cryptID++];
			if(cryptID>=codeLen)
				cryptID=0;
		}
	}
}

bool MemorySerializeStringLarge( std::string &num, uint8 *pMem, int &offsetInOut, bool bWriteToMem, uint32 maxBytesInPacket)
{
	uint32 len;
	
	if (bWriteToMem)
	{
		len = (uint32) num.length();

		if (maxBytesInPacket != 0 && len > (maxBytesInPacket-4))
		{
			//appears to be incorrect data, this string is bigger than the max bytes in our packet
			return false;
		}
		memcpy(&pMem[offsetInOut], &len, sizeof(len));
		offsetInOut += sizeof(len);
 
		//now copy the actual content
		memcpy(&pMem[offsetInOut], num.c_str(), len);

	} else
	{
		memcpy(&len, &pMem[offsetInOut], sizeof(len));
		offsetInOut += sizeof(len);

		num.resize(len);

		//trust me.
		memcpy((void*)num.c_str(), &pMem[offsetInOut], len);
	}
	offsetInOut += len;

	return true;
}


void MemorySerializeRaw(uint8* pVar, uint8 *pMem, int sizeBytes, int &offsetInOut, bool bWriteToMem )
{
	if (sizeBytes == 0) return;

	if (bWriteToMem)
	{
		memcpy(&pMem[offsetInOut], pVar, sizeBytes);
	} else
	{
		memcpy(pVar, &pMem[offsetInOut], sizeBytes);
	}

	offsetInOut += sizeBytes;
}
