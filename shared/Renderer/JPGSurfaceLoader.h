//  ***************************************************************
//  JPGSurfaceLoader - Creation date: 01/11/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef JPGSurfaceLoader_h__
#define JPGSurfaceLoader_h__

extern "C" 
{
	#include "Irrlicht/source/Irrlicht/jpeglib/jpeglib.h" // use irrlicht jpeglib
	#include <setjmp.h>
}

class SoftSurface;

class JPGSurfaceLoader
{
public:
	JPGSurfaceLoader();
	virtual ~JPGSurfaceLoader();

	bool LoadFromMem(byte *pMem, int inputSize, SoftSurface *pSurf, bool bAddAlphaChannelIfPadded );
	
	
	static void init_source (j_decompress_ptr cinfo);
	static boolean fill_input_buffer (j_decompress_ptr cinfo);
	static void skip_input_data (j_decompress_ptr cinfo, long count);
	static void term_source (j_decompress_ptr cinfo);
	static void output_message(j_common_ptr cinfo);
	static void error_exit (j_common_ptr cinfo);
protected:
	

private:
};

#endif // JPGSurfaceLoader_h__