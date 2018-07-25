#ifndef ResourceUtils_h__
#define ResourceUtils_h__

#include "RTFileFormat.h"

#ifndef CLANLIB_1
	#include "Math/rtRect.h"
	#include "MathUtils.h"
	bool LoadFromFile(CL_Vec2f &num, FILE *fp);
	bool LoadFromFile(CL_Vec3f &num, FILE *fp);
	bool LoadFromFile(CL_Rectf &num, FILE *fp);
#endif

//NOTE: Use must SAFE_DELETE_ARRAY() the return from this..
byte * LoadFileIntoMemory(std::string fileName, unsigned int *p_ui_size, bool bUseSavePath = false); //automatically decompresses if needed
byte * LoadFileIntoMemoryBasic(std::string fileName, unsigned int *length, bool bUseSavePath = false, bool bAddBasePath = true); //won't try to automatically decompress

bool FileExists(const std::string &fName);

std::string SeparateStringSTL(std::string input, int index, char delimiter);
bool SeparateString (const char str[], int num, char delimiter, char *return1);
void StringReplace(const std::string& what, const std::string& with, std::string& in);
bool CompressFile(std::string fName);
bool IsAPackedFile(byte *pFile);
bool IsARTFile(byte *pFile); //not full proof, but helps with catching errors
rtpack_header BuildRTPackHeader(int size, int compressedSize);

byte * zlibDeflateToMemory(byte *pInput, int sizeBytes, int *pSizeCompressedOut); //you must SAFE_DELETE_ARRAY what it returns
byte * zLibInflateToMemory(byte *pInput, unsigned int compressedSize, unsigned int decompressedSize); //you must SAFE_DELETE_ARRAY what it returns

//some helpers with file handling
bool LoadFromFile(std::string &str, FILE *fp);
bool LoadFromFile(float &num, FILE *fp);
bool LoadFromFile(int32 &num, FILE *fp);
bool LoadFromFile(uint32 &num, FILE *fp);
bool LoadFromFile(bool &num, FILE *fp);
bool SaveToFile(float num, FILE *fp);

//being careful not to break things in older apps that may depend on saving INT as 64 bit
#ifdef RT_FORCE_32BIT_INTS_FOR_FILES
bool SaveToFile(int32 num, FILE *fp);
#else
bool SaveToFile(int num, FILE *fp);
#endif
bool SaveToFile(uint32 num, FILE *fp);
bool SaveToFile(const std::string &str, FILE *fp);

//same thing but for mem

bool IsPowerOf2(int n);
byte * DecompressRTPackToMemory(byte *pMem, unsigned int *pDecompressedSize=NULL);
byte * CompressMemoryToRTPack(byte *pSourceMem, unsigned int sourceByteSize, unsigned int *pCompressedSizeOut);

int GetFileSize(const std::string &fName);
std::string AddIPADToFileName(std::string file); //appends _ipad to a file name if we are indeed running on an ipad (or large screen)
std::string ReplaceWithDeviceNameInFileName(const std::string &fName); //replace "iphone" with "ipad" in filename, if on ipad
std::string ReplaceWithLargeInFileName(const std::string &fName); //replace "iphone" with "ipad" in filename, if on ipad
std::string ReplaceWithLargeInFileNameAndOSSpecific(const std::string &fName); //like above, but also changes to "win" if Windows is detected
std::string ReplaceMP3( const std::string &fName); //changes mp3 to ogg in a filename if not on iOS
void AppendStringToFile(const std::string filename, const std::string text);
std::string StripColorCodes(const std::string text);
bool StringFromStartMatches(const std::string &line, const std::string textToMatch); //like an strnstr for strings
bool StringFromEndMatches(const std::string &line, const std::string textToMatch);

template <class myType>
void MemorySerialize( myType &num, uint8 *pMem, int &offsetInOut, bool bWriteToMem)
{
	if (bWriteToMem)
	{
		memcpy(&pMem[offsetInOut], &num, sizeof(myType));
	} else
	{
		memcpy(&num, &pMem[offsetInOut], sizeof(myType));
	}

	offsetInOut += sizeof(myType);

}

//specialized version, not done as a template for compatibility with older compilers
void MemorySerialize( std::string &num, uint8 *pMem, int &offsetInOut, bool bWriteToMem);
void MemorySerializeRaw(uint8* pVar, uint8 *pMem, int sizeBytes, int &offsetInOut, bool bWriteToMem ); //for pure data read/writes
bool MemorySerializeStringLarge( std::string &num, uint8 *pMem, int &offsetInOut, bool bWriteToMem, uint32 maxBytesInPacket = 0); //like above but uses an int32 for length instead of int16
void MemorySerializeStringEncrypted( std::string &num, uint8 *pMem, int &offsetInOut, bool bWriteToMem,int cryptID, char *secretCode);	
// normal string serialize, but it encrypts/decrypts with a simple XOR encryption - as long as you use the same cryptID when loading as you did when saving
#endif // ResourceUtils_h__
 