// Copyright (C) 2002-2009 Nikolaus Gebhardt
// Copyright (C) 2009 Seth A. Robinson
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

// Loads .RTTEX (Robinson Technologies texture format)  - it's a container that supports multiple filetypes including pvrtc, it
// 'remembers' what the original size was if padding to power of 2, (not useful in irrlicht, but in my GUI stuff it is), it is
// optionally compressed using zlib

#ifndef __C_IMAGE_LOADER_RTTEX_H_INCLUDED__
#define __C_IMAGE_LOADER_RTTEX_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_RTTEX_LOADER_

#include "IImageLoader.h"

namespace irr
{
	namespace video
	{

		//!  Surface Loader for RTTEX files
		class CImageLoaderRTTEX : public IImageLoader
		{
		public:

			//! returns true if the file maybe is able to be loaded by this class
			//! based on the file extension (e.g. ".RTTEX")
			virtual bool isALoadableFileExtension(const io::path& filename) const;

			//! returns true if the file maybe is able to be loaded by this class
			virtual bool isALoadableFileFormat(io::IReadFile* file) const;

			//! creates a surface from the file
			virtual IImage* loadImage(io::IReadFile* file) const;
		};


	} // end namespace video
} // end namespace irr

#endif
#endif

