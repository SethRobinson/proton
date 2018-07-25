#ifndef RTFileFormat_h__
#define RTFileFormat_h__

//all RT file formats start with the same main header...

//Don't change!
#define C_RTFILE_PACKAGE_LATEST_VERSION 0
#define C_RTFILE_PACKAGE_HEADER "RTPACK"
#define C_RTFILE_PACKAGE_HEADER_BYTE_SIZE 6

struct RTFileHeader
{
	char fileTypeID[C_RTFILE_PACKAGE_HEADER_BYTE_SIZE];
	byte version;
	byte reserved[1];
};


enum eCompressionType
{
	C_COMPRESSION_NONE = 0,
	C_COMPRESSION_ZLIB = 1
};



struct rtpack_header
{
	RTFileHeader rtFileHeader;
	unsigned int compressedSize;
	unsigned int decompressedSize;
	byte compressionType; //one of eCompressionType
	byte reserved[15];
};

#define RT_FORMAT_EMBEDDED_FILE 20000000
#define C_RTFILE_TEXTURE_HEADER "RTTXTR"
struct rttex_header
{
	RTFileHeader rtFileHeader;

	//our custom header
	int height;
	int width;
	int format; // probably GL_UNSIGNED_BYTE , or GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, or RT_FORMAT_EMBEDDED_FILE for jpgs, etc
	int originalHeight; //before we padded to be a power of 2, if applicable.
	int originalWidth; //before we padded to be a power of 2, if applicable.
	unsigned char bUsesAlpha;
	unsigned char bAlreadyCompressed; //if 1, it means we don't require additional compression
	unsigned char reservedFlags[2]; //keep this struct packed right
	int mipmapCount; //how many tex infos are followed...

	int reserved[16];

	//texture info to follow, 1 for each mip map
};


struct rttex_mip_header
{
	int height;
	int width;
	int dataSize;
	int mipLevel;
	int reserved[2];
};



#endif // RTFileFormat_h__
