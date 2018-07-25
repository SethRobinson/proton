// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CImageLoaderRTTEX.h"

#ifdef _IRR_COMPILE_WITH_RTTEX_LOADER_

#include "CImage.h"
#include "CReadFile.h"
#include "os.h"
#include "Renderer/Surface.h"
#include "util/ResourceUtils.h"

namespace irr
{
	namespace video
	{

		//! returns true if the file maybe is able to be loaded by this class
		//! based on the file extension (e.g. ".tga")
		bool CImageLoaderRTTEX::isALoadableFileExtension(const io::path& filename) const
		{
			return core::hasFileExtension ( filename, "rttex" );
		}


		//! returns true if the file maybe is able to be loaded by this class
		bool CImageLoaderRTTEX::isALoadableFileFormat(io::IReadFile* file) const
		{
			return false;
		}


		// load in the image data
		IImage* CImageLoaderRTTEX::loadImage(io::IReadFile* file) const
		{

			if (!file) return 0;

			video::IImage* image = 0;

			
			u8 *pBuff = new u8[file->getSize()];
			file->read(pBuff, file->getSize());
			
			unsigned int decompressedSize = file->getSize();

			if (!IsARTFile(pBuff))
			{
				os::Printer::log("LOAD RTTEX: not really a RTTEX\n", file->getFileName(), ELL_ERROR);
				delete(pBuff);
				return 0;
			}

			if (IsAPackedFile(pBuff))
			{
				//let's decompress it to memory before passing it back
				byte *pDecompressedData = DecompressRTPackToMemory(pBuff, &decompressedSize);
				delete pBuff; //done with the original
				pBuff = pDecompressedData;
			}

			rttex_header *pTexHeader = (rttex_header*)pBuff;

			//we're not really going to do anything with the data yet, just pass it in and hope the CImage doesn't really
			//think it's valid RGBA data.  Later when the device dependent surface is made we'll do the real work

			image = new CImage(ECF_PVRTC, core::dimension2d<u32>(pTexHeader->width, pTexHeader->height), pBuff, true, true, decompressedSize);
		
			if (!image)
			{
				os::Printer::log("LOAD RTTEX: Internal RTTEX create image struct failure\n", file->getFileName(), ELL_ERROR);
				delete(pBuff);
				return 0;
			}

			//note, we're letting the CImage take over the pBuff and kill it for us

			return image;
		}

		IImageLoader* createImageLoaderRTTEX()
		{
			return new CImageLoaderRTTEX();
		}


	}// end namespace irr
}//end namespace video

#endif

