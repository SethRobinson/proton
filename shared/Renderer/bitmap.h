#ifndef bitmap_h__
#define bitmap_h__


enum eBMPCompressionType
{
	BMP_COMPRESSION_NONE,
	BMP_COMPRESSION_RLE8,
	BMP_COMPRESSION_RLE4,
	BMP_COMPRESSION_BITFIELDS,
	BMP_COMPRESSION_JPG,
	BMP_COMPRESSION_PONG
};

typedef struct BMPFileHeader
{
	char ID[ 2 ];
	unsigned int Size;
	unsigned int Reserved;
	unsigned int PixelOffset;
} BMPFileHeader;

typedef struct BMPImageHeader
{
	unsigned int Size;
	unsigned int Width;
	unsigned int Height;
	unsigned short Planes;
	unsigned short BitCount;
	unsigned int Compression;
	unsigned int ImageSize;
	unsigned int XPixels;
	unsigned int YPixels;
	unsigned int ColorsUsed;
	unsigned int ColorsImportant;
} BMPImageHeader;

#endif // bitmap_h__
