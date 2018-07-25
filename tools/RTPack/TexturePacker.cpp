#include "TexturePacker.h"

using namespace pvrtexlib;
using namespace std;

TexturePacker::TexturePacker()
{
	m_pixType = OGL_PVRTC4;
}

TexturePacker::~TexturePacker()
{
}
 
void CreateTransparencyFromColorKey(CL_PixelBuffer &pixBuff, CL_Color color)
{
	unsigned int *p_data = (unsigned int*)pixBuff.get_data();

	for (int x=0; x < pixBuff.get_width(); x++)
	{
		for (int y=0; y < pixBuff.get_height(); y++)
		{
			if (pixBuff.get_pixel(x,y) == CL_Color(255,0,255))
			{
				//convert this pixel to transparent
				p_data[y*(pixBuff.get_pitch()/(pixBuff.get_format().get_depth()/8))+ (x)] = CL_Color(0,0,0,0).color;
			}
		}
	}
}

int GetLowestPowerOf2(int n)
{
	int lowest = 1;
	while(lowest < n) lowest <<= 1;
	return lowest;
}

bool UsesTransparency(CL_PixelBuffer &pixBuff)
{

	if (pixBuff.get_format().get_depth() == 32)
	{
	
		//if (GetApp()->GetForceAlpha())
		//return true; //forcing it, because later I found I could avoid outline glitches when scaling up textured that were resized here
		//by using premultiplied alpha.

		
		//well, there IS an alpha layer, but let's check to see if it's actually used or not

		unsigned int *p_data = (unsigned int*)pixBuff.get_data();
	
		for (int x=0; x < pixBuff.get_width(); x++)
		{
			for (int y=0; y < pixBuff.get_height(); y++)
			{
				::uint32 pixel = p_data[y*(pixBuff.get_pitch()/(pixBuff.get_format().get_depth()/8))+ (x)];

				if (GET_ALPHA(pixel) != 255)
				{
					//LogMsg("Found something.  Alpha is %d", GET_ALPHA(pixel));
					return true; //they actually need the alpha
				}
			}
		}
		return false; //not using the alpha stuff
	}

if (pixBuff.get_format().get_depth() != 8)
{
	//don't care
	return false;
}

	if (pixBuff.get_palette().colors[0] == CL_Color(255,0,255))
	{
		for (int x=0; x < pixBuff.get_width(); x++)
		{
			for (int y=0; y < pixBuff.get_height(); y++)
			{
				if (pixBuff.get_pixel(x,y) == CL_Color(255,0,255))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void ClearPixelBuffer(CL_PixelBuffer* pPixelBuffer, CL_Color color)
{
	cl_assert(pPixelBuffer && "Invalid buffer!");
	int bytes_pp = pPixelBuffer->get_format().get_depth()/8;
	//cl_assert(bytes_pp == 4 && "We only support 32 bit 8888 format right now.");
	if (bytes_pp != 4)
	{
		return;
	}

	unsigned int dest_format_color = color.to_pixelformat(pPixelBuffer->get_format());
	int copy_count = pPixelBuffer->get_pitch()/4;

	unsigned int *p_data = (unsigned int*)pPixelBuffer->get_data();

	pPixelBuffer->lock();
	for (int y=0; y < pPixelBuffer->get_height(); y++)
	{
		for (int x=0; x < copy_count; x++)
		{
			*p_data = dest_format_color;
			p_data++;
		}
	}
	pPixelBuffer->unlock();
}

bool ImageCanBeUltraCompressed(bool bUsesTransparency, int width, int height)
{
	//later, we may wish to limit using this to only larger sizes etc, that's why I pass in size now

	if (!bUsesTransparency && GetApp()->GetUltraCompressQuality() != 0)
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void FileWriteRAWPVR(string pathAndFileName, CPVRTexture* texture, int nNumMipLevels, bool bUsesTransparency, int originalWidth, int originalHeight)
//-----------------------------------------------------------------------------
{
	FILE* pFileOut = NULL;
	int nMipLevel;

#ifdef _WIN32
	fopen_s(&pFileOut, pathAndFileName.c_str(), "wb");
#else
	pFileOut = fopen(pathAndFileName.c_str(), "wb");
#endif

	if (pFileOut == NULL )
	{
		throw CL_Error("Could not open writing output file");
	}

	//from PVRTglesExt.h
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG			0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG			0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG			0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG			0x8C03
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033

#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405

#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_SHORT_5_6_5           0x8363

	const ::uint32 PVRTC2_MIN_TEXWIDTH		= 16;
	const ::uint32 PVRTC2_MIN_TEXHEIGHT		= 8;
	const ::uint32 PVRTC4_MIN_TEXWIDTH		= 8;
	const ::uint32 PVRTC4_MIN_TEXHEIGHT		= 8;
	const ::uint32 ETC_MIN_TEXWIDTH			= 4;
	const ::uint32 ETC_MIN_TEXHEIGHT		= 4;

	rttex_header rtTexHeader;
	ZeroMemory(&rtTexHeader, sizeof(rttex_header));
	memcpy(rtTexHeader.rtFileHeader.fileTypeID, C_RTFILE_TEXTURE_HEADER, 6);

	float bytesPerPixel = 1;
	rtTexHeader.bAlreadyCompressed = 0;

	bool bUltraCompress = false;
	if (texture->getPixelType() == OGL_PVRTC2)
	{
		rtTexHeader.format = bUsesTransparency ? GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG :GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		bytesPerPixel = 0.25f;
	} else if (texture->getPixelType() == OGL_PVRTC4)
	{
		rtTexHeader.format = bUsesTransparency ? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG :GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		bytesPerPixel = 0.5f;
	} else if (texture->getPixelType() == OGL_RGBA_4444)
	{
		rtTexHeader.format = GL_UNSIGNED_SHORT_4_4_4_4;
		bytesPerPixel = 2;
	} else if (texture->getPixelType() == OGL_RGB_565)
	{
		rtTexHeader.format = GL_UNSIGNED_SHORT_5_6_5;
		bytesPerPixel = 2;
	}else if (texture->getPixelType() == OGL_RGBA_8888)
	{
		bytesPerPixel = 4;
		rtTexHeader.format = GL_UNSIGNED_BYTE;
	} else if (texture->getPixelType() == OGL_RGB_888)
	{
		bytesPerPixel = 3;
		rtTexHeader.format = GL_UNSIGNED_BYTE;

		if (ImageCanBeUltraCompressed(bUsesTransparency, texture->getWidth(), texture->getHeight()))
		{
			bUltraCompress = true;
			rtTexHeader.format = RT_FORMAT_EMBEDDED_FILE; //game will check the header to know it's actually a jpg or whatever
			GetApp()->SetPixelTypeText("Ultra compress RGB bit");
				rtTexHeader.bAlreadyCompressed = 1;
		}

	} else
	{
		LogError("Don't know how to process (%d) %s", GetApp()->GetPixelType(), GetApp()->GetPixelTypeText().c_str());
		assert(!"Bad texture type");
		return;
	}

	rtTexHeader.height = texture->getHeight();
	rtTexHeader.width = texture->getWidth();
	rtTexHeader.originalHeight = originalHeight;
	rtTexHeader.originalWidth = originalWidth;
	rtTexHeader.mipmapCount = nNumMipLevels;
	if (bUsesTransparency)
	{
		rtTexHeader.bUsesAlpha = 1;
	} else rtTexHeader.bUsesAlpha = 0;

	fwrite(&rtTexHeader, 1, sizeof(rttex_header), pFileOut);

	int lastWidth = texture->getWidth();
	int lastHeight = texture->getHeight();
	assert(texture->getNumSurfaces() == 1 && "We don't support more yet");

	int dataOffset = 0;

	StringReplace("\\", "/", pathAndFileName);

	string fName = GetFileNameFromString(pathAndFileName);
	string path = GetPathFromString(pathAndFileName);
	if (!path.empty()) path += "/";

	for (nMipLevel=0; nMipLevel<nNumMipLevels; nMipLevel++)
	{
		rttex_mip_header mipHeader;
		ZeroMemory(&mipHeader, sizeof(rttex_mip_header));
		mipHeader.height = lastHeight;
		mipHeader.width = lastWidth;
	
		::uint32 CompressedImageSize = int(float(mipHeader.height) * float(mipHeader.width)*bytesPerPixel);

		if (rtTexHeader.format== GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG || rtTexHeader.format== GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
		{
			CompressedImageSize = ( rt_max(mipHeader.width, PVRTC2_MIN_TEXWIDTH) * rt_max(mipHeader.height, PVRTC2_MIN_TEXHEIGHT) * 2 + 7) / 8;
		}

		if (rtTexHeader.format== GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG || rtTexHeader.format== GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
		{
			CompressedImageSize = ( rt_max(mipHeader.width, PVRTC4_MIN_TEXWIDTH) * rt_max(mipHeader.height, PVRTC4_MIN_TEXHEIGHT) * 4 + 7) / 8;
		}
	
		mipHeader.mipLevel = nMipLevel;

		if (bUltraCompress)
		{
			//convoluted way to convert the data to a jpg, then pack it in the rttex.

			CL_PixelBuffer finalBuff(mipHeader.width, mipHeader.height,mipHeader.width*3, CL_PixelFormat::bgr888);
		
			finalBuff.lock();
			memcpy(finalBuff.get_data(),  texture->getSurfaceData(0)+dataOffset, CompressedImageSize);
			finalBuff.unlock(); 
			
			//save and load the jpg because I don't know how to get clanlib to write it directly to memory and don't feel like digging
			//into it right now
			string tempFilePathAndName = path+"rt_temp_"+ModifyFileExtension(fName, "jpg");
			try
			{
				CL_JPEGProvider::save(finalBuff, tempFilePathAndName,0, GetApp()->GetUltraCompressQuality());

				if (GetApp()->GetOutput() == App::JPG)
				{
					//if "-o jpg" was specified on the command line, we'll write out the mipmaps for debugging purposes
					CL_JPEGProvider::save(finalBuff, path+"test_mip_"+toString(mipHeader.mipLevel)+ModifyFileExtension(fName, "jpg"),0, GetApp()->GetUltraCompressQuality());


				}

			}
			catch (CL_Error e)
			{
				LogError("Error: %s", e.message.c_str());
				return ;
			}
		
			unsigned int size;
			byte *pJpgData = LoadFileIntoMemory(tempFilePathAndName, &size);
			RemoveFile(tempFilePathAndName);
			
			//note:  this would work to mix jpgs and raw rgb data (for tiny mipmaps) in  the same .rttex, but I'd need to vflip
			//the raw data.. maybe play with this later if I noticed jpg decompression being an issue

			bool bUseJpg = true;

			if (bUseJpg)
			{
				//write the jpg out
				mipHeader.dataSize = size;
				//LogMsg("MIP %d: %d X %d", mipHeader.mipLevel, mipHeader.width,  mipHeader.height);
				fwrite(&mipHeader, 1, sizeof(rttex_mip_header), pFileOut);
				fwrite(pJpgData, sizeof(unsigned char),mipHeader.dataSize, pFileOut);
			} else
			{
				//write pure data instead, better than jpg for small texture sizes?
				mipHeader.dataSize = CompressedImageSize;
				//LogMsg("MIP %d: %d X %d", mipHeader.mipLevel, mipHeader.width,  mipHeader.height);
				fwrite(&mipHeader, 1, sizeof(rttex_mip_header), pFileOut);
				fwrite(texture->getSurfaceData(0)+dataOffset, sizeof(unsigned char),mipHeader.dataSize, pFileOut);

			}

			SAFE_FREE(pJpgData);

			//LogMsg("Writing jpg");
		} else
		{
			mipHeader.dataSize = CompressedImageSize;
			//LogMsg("MIP %d: %d X %d", mipHeader.mipLevel, mipHeader.width,  mipHeader.height);
			fwrite(&mipHeader, 1, sizeof(rttex_mip_header), pFileOut);
			fwrite(texture->getSurfaceData(0)+dataOffset, sizeof(unsigned char),mipHeader.dataSize, pFileOut);
		}

		dataOffset += CompressedImageSize;
		lastHeight /=2;
		lastWidth /=2;

		if (lastHeight == 0) lastHeight = 1;
		if (lastWidth == 0) lastWidth = 1;
	}

	fclose(pFileOut);
}

//-----------------------------------------------------------------------------
int CountNumMipLevels(int nWidth, int nHeight)
//-----------------------------------------------------------------------------
{
	int nNumPowersOfTwo = 1;
	while (nWidth>1 || nHeight>1)
	{
		nNumPowersOfTwo++;
		nWidth >>=1;
		nHeight>>=1;
	}
	return nNumPowersOfTwo;
}




void EmbedImageFileAsRTTEX(string outputFile, string fileToImbed, CL_PixelBuffer &pixBuff, int originalWidth, int originalHeight)
{
	FILE* pFileOut = NULL;
	int nMipLevel;
#ifdef _WIN32
	fopen_s(&pFileOut, outputFile.c_str(), "wb");
#else
	pFileOut = fopen(outputFile.c_str(), "wb");
#endif
	int embedFileSize = GetFileSize(fileToImbed);
	if (embedFileSize == 0)
	{
		throw CL_Error("Could not open temp file to embed");
	}
	if (pFileOut == NULL )
	{
		throw CL_Error("Could not open writing output file");
	}

	rttex_header rtTexHeader;
	ZeroMemory(&rtTexHeader, sizeof(rttex_header));
	memcpy(rtTexHeader.rtFileHeader.fileTypeID, C_RTFILE_TEXTURE_HEADER, 6);
	
	rtTexHeader.format = RT_FORMAT_EMBEDDED_FILE; //game will check the header to know it's actually a jpg or whatever
	
	rtTexHeader.height = pixBuff.get_height();
	rtTexHeader.width = pixBuff.get_width();
	rtTexHeader.originalHeight = originalHeight;
	rtTexHeader.originalWidth = originalWidth;
	rtTexHeader.mipmapCount = 1; //no extra mipmaps, just the main file
	rtTexHeader.bUsesAlpha = 0;
	rtTexHeader.bAlreadyCompressed = 1;
	fwrite(&rtTexHeader, 1, sizeof(rttex_header), pFileOut);

	int nNumMipLevels = 1;
	int lastWidth = pixBuff.get_height();
	int lastHeight = pixBuff.get_width();
	
	int dataOffset = 0;
	
	for (nMipLevel=0; nMipLevel<nNumMipLevels; nMipLevel++)
	{
		rttex_mip_header mipHeader;
		ZeroMemory(&mipHeader, sizeof(rttex_mip_header));
		mipHeader.height = lastHeight;
		mipHeader.width = lastWidth;

		mipHeader.dataSize = embedFileSize;

		mipHeader.mipLevel = nMipLevel;
		LogMsg("(Activating ultra compress - file crunched to %d bytes", embedFileSize);

		fwrite(&mipHeader, 1, sizeof(rttex_mip_header), pFileOut);
	
		//load temp file
		byte *pBuff = new byte[embedFileSize];
	
		FILE *fEmbed = fopen(fileToImbed.c_str(), "rb");
		if (!fEmbed)
		{
			throw CL_Error("Could not open temp file to embed");
		}

		fread(pBuff, embedFileSize, 1, fEmbed);
		
		//append it to our current file we're writing
		fwrite(pBuff, embedFileSize,1, pFileOut);
		SAFE_DELETE_ARRAY(pBuff);
		fclose(fEmbed);
		dataOffset += mipHeader.dataSize;
	}
	fclose(pFileOut);
}


bool TexturePacker::ProcessTexture( string fName )
{
#ifdef _WIN32
	CL_SetupCore::set_instance(GetModuleHandle(NULL));
#endif
	CL_SetupCore::init(true);
	// Initialize the ClanLib display component
	CL_SetupDisplay setup_display;
	CL_PixelBuffer pixBuff;
	string path = CL_String::get_path(fName);
	string fileNameOnly = CL_String::get_filename(fName);

#ifdef _WIN32
	TCHAR szDirectory[MAX_PATH] = "";
	if(!::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory))
#else
	const int MAX_PATH = 4096;
	char szDirectory[MAX_PATH] = "";
	if(!getcwd(szDirectory, sizeof(szDirectory)))
#endif
	{
	}

	try {
	pixBuff = CL_ProviderFactory::load(fName);
	} 
	catch (CL_Error e)
	{
		LogError("Unable to open %s\n\nExe Location %s\n\nActive dir: %s\n\nReason: %s", fName.c_str(), CL_System::get_exe_path().c_str(),
			szDirectory, e.message.c_str());
		return false;
	}
	
	catch (...)
	{
			LogError("Unable to open %s\n\nExe Location %s\n\nActive dir: %s", fName.c_str(), CL_System::get_exe_path().c_str(),
			szDirectory);
		return false;
	}

	if (pixBuff.get_format().get_depth() == 4)
	{
		throw CL_Error("We don't support 4 bit stuff");
	}

	int originalX = pixBuff.get_width();
	int originalY = pixBuff.get_height();

	if ( rt_min(pixBuff.get_width(), pixBuff.get_height() ) < 8)
	{
		if (GetApp()->GetPixelType() == pvrtexlib::OGL_PVRTC2 || GetApp()->GetPixelType() == OGL_PVRTC4)
		{
			//LogError("File should be at least 8x8 if you want to compress it with pvrtc or its compressor crashes.  Don't blame me!");
			//return false;	

			LogMsg("INFO: changing format, texture too small for prtc");
			GetApp()->SetMaxMipLevel(1);
			GetApp()->SetPixelType(pvrtexlib::OGL_RGBA_4444);
		}
	}
	
	if (UsesTransparency(pixBuff))
	{
		if (pixBuff.get_format().get_depth() == 8)
			pixBuff.set_colorkey(true, 0);

		m_bUsesTransparency = true;
	} else
	{
		m_bUsesTransparency = false;
	}

	CL_PixelFormat finalFormat = CL_PixelFormat::abgr8888; //the PVR converter needs 32 bit, so we do this regardless of alpha or format at first
	

	CL_Rect finalRect;
	CL_Rect tempRect; 
	CL_Rect stretchRect;

	finalRect = CL_Rect(0,0,pixBuff.get_width(), pixBuff.get_height());
	tempRect = finalRect;
	stretchRect = finalRect; 

	if (!GetApp()->GetNoPowerOfTwo())
	if (!IsPowerOf2(pixBuff.get_width()) || !IsPowerOf2(pixBuff.get_height()))
	{
		//Not a power of two.  Let's remedy that little problem...
		finalRect.right = GetLowestPowerOf2(finalRect.right);
		finalRect.bottom = GetLowestPowerOf2(finalRect.bottom);
		if (finalRect.right > 1024 || finalRect.bottom > 1024)
		{
			LogError("Unless something changed with our limitations, this texture is too big for the HW.");
			//return false;
		}
		
		if (!GetApp()->GetStretchImage())
		{
			LogMsg("Padding %s from %dX%d to be %dX%d", fName.c_str(),  pixBuff.get_width(), pixBuff.get_height(), finalRect.right, finalRect.bottom);
		}
	}
  
	if (GetApp()->GetPixelTypeIfNotSquareOrTooBig() != 0)
	{
		if (finalRect.right >= 1024 || finalRect.bottom >= 1024)
		{
			GetApp()->SetPixelType(GetApp()->GetPixelTypeIfNotSquareOrTooBig());
			
		}
	}


	if (GetApp()->GetForceSquare())
	{
		//um.. compressed formats must be square
		finalRect.right = rt_max(finalRect.right, finalRect.bottom);
		finalRect.bottom = finalRect.right;
	}

	if (GetApp()->GetPixelTypeIfNotSquareOrTooBig() != 0)
	{
		if (finalRect.right != finalRect.bottom)
		{
			GetApp()->SetPixelType(GetApp()->GetPixelTypeIfNotSquareOrTooBig());
		}
	}

	//SETH
	
	if (!m_bUsesTransparency)
	{
		if (GetApp()->GetForceAlpha())
		{
			m_bUsesTransparency = true;
			LogMsg("(-force_alpha causing us to leave the unnecessary alpha channel in)");
		} else
		{
			switch (GetApp()->GetPixelType())
			{
			case OGL_RGBA_8888:
				GetApp()->SetPixelType(pvrtexlib::OGL_RGB_888);
				break;

			case OGL_RGBA_4444:
				GetApp()->SetPixelType(pvrtexlib::OGL_RGB_565);
				
				if (ImageCanBeUltraCompressed(m_bUsesTransparency,finalRect.right, finalRect.bottom))
				{
					//let's use jpg compression, probably smaller
					GetApp()->SetPixelType(pvrtexlib::OGL_RGB_888);
				}
				break;

			}
		}
	}


	if (GetApp()->GetStretchImage() && finalRect != tempRect)
	{
		LogMsg("Stretching %s from %dX%d to be %dX%d", fName.c_str(),  pixBuff.get_width(), pixBuff.get_height(), finalRect.right, finalRect.bottom);
		stretchRect = finalRect;
		finalRect = tempRect;
		originalX = stretchRect.get_width();
		originalY = stretchRect.get_height();
	} else
	{
		stretchRect = finalRect;
	}

	CL_PixelBuffer finalBuff(finalRect.get_width(), finalRect.get_height(),finalRect.get_width()*finalFormat.get_depth()/8, finalFormat);
	ClearPixelBuffer(&finalBuff, CL_Color(0,0,0,0));
	
	//actually copy over the data
	CL_Rect inputRect(0, 0, pixBuff.get_width(), pixBuff.get_height());
	pixBuff.convert(finalBuff.get_data(), finalBuff.get_format(), finalBuff.get_pitch(), inputRect, inputRect);

	if (finalFormat == CL_PixelFormat::abgr8888 && m_bUsesTransparency && pixBuff.get_format().get_depth() == 8)
	{
		//Clanlib doesn't copy over the colorkey data.. I should probably fix that.. but for now, let's do it manually
		CreateTransparencyFromColorKey(finalBuff, CL_Color(255,0,255));
	}
	
	//if we wanted to save out a .png for testing
	if (GetApp()->GetOutput() == App::PNG)
	{
		string outputName = path + string("/") + "test_"+ModifyFileExtension(fileNameOnly, "png");
		try
		{
			CL_ProviderFactory::save(finalBuff, outputName);
		}
		catch (CL_Error e)
		{
			LogError("Error: %s", e.message.c_str());
			return false;
		}
		
		LogMsg("(Wrote png of final output out to %s", outputName.c_str());
	}

	if (GetApp()->GetOutput() == App::JPG)
	{
		int quality = 100;
		if (GetApp()->GetUltraCompressQuality() != 0) quality = GetApp()->GetUltraCompressQuality();
		string outputName =  path + string("/") + "test_"+ModifyFileExtension(fileNameOnly, "jpg");
		try
		{
			CL_JPEGProvider::save(finalBuff,outputName,0, quality);
		}
		catch (CL_Error e)
		{
			LogError("Error: %s", e.message.c_str());
			return false;
		}
		
		//LogMsg("(Wrote jpg of final output out to %s", outputName.c_str());
	}

	// write to file specified 
	string fileName = ModifyFileExtension(fName, "rttex");
	int nNumMipLevels = 1;

	/*
	if (ImageCanBeUltraCompressed(bUsesTransparency,finalBuff->getWidth(), finalBuff->getHeight()))
	{
		//this is the version we'll use when we don't need mipmaps

		//write out a temp file.. CL 1.x doesn't support writing this to a stream I think...
		string tempFilePathAndName = path + string("/") + "rt_temp_"+ModifyFileExtension(fileNameOnly, "jpg");
		CL_JPEGProvider::save(finalBuff, tempFilePathAndName,0, GetApp()->GetUltraCompressQuality());

		//pack it as .rttex
		string outputFilename =  path + string("/") +ModifyFileExtension(fileNameOnly, "rttex");
		EmbedImageFileAsRTTEX(outputFilename, tempFilePathAndName, finalBuff, originalX, originalY );
		RemoveFile(tempFilePathAndName);
		
	} else
	*/
	
	bool bFlipped = !GetApp()->GetFlipV();
	
	
	if (GetApp()->GetPixelType() == OGL_RGB_888 && ImageCanBeUltraCompressed(m_bUsesTransparency,finalBuff.get_width(), finalBuff.get_height()))
	{
		//Jpgs should not be flipped.  In fact, probably nothing should be (??) this is legacy, I'm not sure which way is right.  But jpgs
		//shouldn't be..
		bFlipped = !bFlipped;
	}
	
	{
// The singleton interface is no more in PVRTexLib version >= 3.20
#if (PVRTLMAJORVERSION * 1000 + PVRTLMINORVERSION) < (3 * 1000 + 20)
		// get the utilities instance 
		PVRTextureUtilities &PVRU = *(PVRTextureUtilities::getPointer()); 
#else
		PVRTextureUtilities PVRU;
#endif

		CPVRTexture sOriginalTexture( 
			finalBuff.get_width(),      // u32Width, 
			finalBuff.get_height(),    // u32Height, 
			0,    // u32MipMapCount, 
			1,    // u32NumSurfaces, 
			false,     // bBorder, 
			false,     // bTwiddled, 
			false,     // bCubeMap, 
			false,     // bVolume, 
			false,     // bFalseMips, 
			m_bUsesTransparency,     // bHasAlpha, 
			bFlipped, //flipped
			DX10_R8G8B8A8_UNORM, // ePixelType, 
			0.0f,    // fNormalMap, 
			(pvrtexlib::uint8*)finalBuff.get_data()   // pPixelData 
			); 

		sOriginalTexture.convertToPrecMode(ePREC_INT8);
		// make an empty texture for the destination of the preprocessing 
		// copying the header settings 
		CPVRTextureHeader texHeader(sOriginalTexture.getHeader());

		texHeader.setFalseMips(false);

		texHeader.setHeight(stretchRect.get_height());
		texHeader.setWidth(stretchRect.get_width());

		nNumMipLevels = CountNumMipLevels(stretchRect.get_width(), stretchRect.get_height());
		nNumMipLevels = min(nNumMipLevels, GetApp()->GetMaxMipLevel()); 

		texHeader.setMipMapCount(nNumMipLevels-1); 

		// specify desired normal map height factor 
		//sProcessTexture.setNormalMap(5.0f); 

		try
		{
			PVRU.ProcessRawPVR(sOriginalTexture,texHeader); 
		}
		PVRCATCH(myException) 
		{ 
			LogError("Could not preprocess texture:\n%s\n",myException.what()); 
		} 

		// create texture to encode to 
		CPVRTexture sCompressedTexture(sOriginalTexture.getHeader()); 

		// set required encoded pixel type 
		sCompressedTexture.setPixelType(GetApp()->GetPixelType()); 
		sCompressedTexture.convertToPrecMode(ePREC_INT8);

		// encode texture 
		try
		{
			PVRU.CompressPVR(sOriginalTexture, sCompressedTexture); 
		}
		PVRCATCH(myException) 
		{ 
			LogError("Could not compress texture:\n%s\n",myException.what()); 
		} 


		if (GetApp()->GetOutput() == App::PVR)
		{
			sCompressedTexture.writeToFile((fileName = ModifyFileExtension(fName, "pvr")).c_str());  
		} else if  (GetApp()->GetOutput() == App::PNG)
		{

		} else
		{

			FileWriteRAWPVR(fileName.c_str(), &sCompressedTexture, nNumMipLevels, m_bUsesTransparency, originalX, originalY);
		}

		
	}

	

	LogMsg("Saved out %s (%d X %d) with %d mipmaps. %s (%s format)", fileName.c_str(), 
		pixBuff.get_width(), pixBuff.get_height(), nNumMipLevels, m_bUsesTransparency==0 ? "" : "(uses alpha)", GetApp()->GetPixelTypeText().c_str());

	
	return true;

}