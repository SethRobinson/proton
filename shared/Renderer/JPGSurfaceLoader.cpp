#include "PlatformPrecomp.h"
#include "JPGSurfaceLoader.h"
#include "SoftSurface.h"
#include "util/ResourceUtils.h"
#include "addons/TinyEXIF-master/TinyEXIF.h"

#define TINYEXIF_NO_XMP_SUPPORT
#include "addons/TinyEXIF-master/TinyEXIF.cpp"

extern "C"
{
	#include <setjmp.h>
}

// struct for handling jpeg errors
struct irr_jpeg_error_mgr
{
	// public jpeg error fields
	struct jpeg_error_mgr pub;

	// for longjmp, to return to caller on a fatal error
	jmp_buf setjmp_buffer;
};

JPGSurfaceLoader::JPGSurfaceLoader()
{
}

JPGSurfaceLoader::~JPGSurfaceLoader()
{
}

void JPGSurfaceLoader::error_exit (j_common_ptr cinfo)
{
	// unfortunately we need to use a goto rather than throwing an exception
	// as gcc crashes under linux crashes when using throw from within
	// extern c code

	// Always display the message
	(*cinfo->err->output_message) (cinfo);

	// cinfo->err really points to a irr_error_mgr struct
	irr_jpeg_error_mgr *myerr = (irr_jpeg_error_mgr*) cinfo->err;

	longjmp(myerr->setjmp_buffer, 1);
}

void JPGSurfaceLoader::init_source (j_decompress_ptr cinfo)
{
	// DO NOTHING
}



boolean JPGSurfaceLoader::fill_input_buffer (j_decompress_ptr cinfo)
{
	// DO NOTHING
	return 1;
}



void JPGSurfaceLoader::skip_input_data (j_decompress_ptr cinfo, long count)
{
	jpeg_source_mgr * src = cinfo->src;
	if(count > 0)
	{
		src->bytes_in_buffer -= count;
		src->next_input_byte += count;
	}
}



void JPGSurfaceLoader::term_source (j_decompress_ptr cinfo)
{
	// DO NOTHING
}

void JPGSurfaceLoader::output_message(j_common_ptr cinfo)
{
	// display the error message.
	char temp1[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, temp1);
	LogError("JPEG FATAL ERROR: %s",temp1);
}

bool JPGSurfaceLoader::LoadFromMem( uint8 *pMem, int inputSize, SoftSurface *pSurf, bool bAddAlphaChannelIfNotPowerOfTwo )
{
	// allocate and initialize JPEG decompression object
	struct jpeg_decompress_struct cinfo;
	struct irr_jpeg_error_mgr jerr;
	uint8 **rowPtr=NULL;
	memset(&cinfo, 0, sizeof(cinfo));
	//We have to set up the error handler first, in case the initialization
	//step fails.  (Unlikely, but it could happen if you are out of memory.)
	//This routine fills in the contents of struct jerr, and returns jerr's
	//address which we place into the link field in cinfo.

	cinfo.err = jpeg_std_error(&jerr.pub);
	cinfo.err->error_exit = error_exit;
	cinfo.err->output_message = output_message;

	// compatibility fudge:
	// we need to use setjmp/longjmp for error handling as gcc-linux
	// crashes when throwing within external c code
	if (setjmp(jerr.setjmp_buffer))
	{
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object and return.

		jpeg_destroy_decompress(&cinfo);

		// if the row pointer was created, we delete it.
			SAFE_DELETE_ARRAY(rowPtr);
		// return null pointer
			return 0;
	}

	jpeg_create_decompress(&cinfo);

	// specify data source
	jpeg_source_mgr jsrc;

	// Set up data pointer
	jsrc.bytes_in_buffer = inputSize;
	jsrc.next_input_byte = (JOCTET*)pMem;
	cinfo.src = &jsrc;

	jsrc.init_source = init_source;
	jsrc.fill_input_buffer = fill_input_buffer;
	jsrc.skip_input_data = skip_input_data;
	jsrc.resync_to_restart = jpeg_resync_to_restart;
	jsrc.term_source = term_source;

	// Decodes JPG input from whatever source
	// Does everything AFTER jpeg_create_decompress
	// and BEFORE jpeg_destroy_decompress
	// Caller is responsible for arranging these + setting up cinfo

	// read file parameters with jpeg_read_header()
	jpeg_read_header(&cinfo, TRUE);

	bool useCMYK=false;
	if (cinfo.jpeg_color_space==JCS_CMYK)
	{
		cinfo.out_color_space=JCS_CMYK;
		cinfo.out_color_components=4;
		useCMYK=true;
	}
	else
	{
		cinfo.out_color_space=JCS_RGB;
		cinfo.out_color_components=3;
	}
	cinfo.do_fancy_upsampling=FALSE;


	//let's figure out if the exif header has marked that it's rotated

	TinyEXIF::EXIFInfo exifInfo;
	exifInfo.parseFrom(pMem, inputSize);

	int orientation = exifInfo.Orientation;

	// Start decompressor
	jpeg_start_decompress(&cinfo);
	// Get image data
	uint16 rowspan = cinfo.image_width * cinfo.out_color_components;
	uint32 width = cinfo.image_width;
	uint32 height = cinfo.image_height;

	// Allocate memory for buffer
	uint8* output = new uint8[rowspan * height];

	// Here we use the library's state variable cinfo.output_scanline as the
	// loop counter, so that we don't have to keep track ourselves.
	// Create array of row pointers for lib
	rowPtr = new uint8* [height];

	for( uint32 i = 0; i < height; i++ )
		rowPtr[i] = &output[ i * rowspan ];

	uint32 rowsRead = 0;

	while( cinfo.output_scanline < cinfo.output_height )
		rowsRead += jpeg_read_scanlines( &cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead );

	delete [] rowPtr;
	// Finish decompression

	jpeg_finish_decompress(&cinfo);

	// Release JPEG decompression object
	// This is an important step since it will release a good deal of memory.
	jpeg_destroy_decompress(&cinfo);

	//ok, we've got the image, but it's not in rgba. Let's initialize a new buffer the correct size, then put it in there.

	SoftSurface::eSurfaceType surfType = SoftSurface::SURFACE_RGB;

	if (bAddAlphaChannelIfNotPowerOfTwo &&
		( !IsPowerOf2(width) || !IsPowerOf2(height) )
		)
	{
		//why would we add an alpha channel? Well, due to padding the texture, we can get artifacts around the
		//edges when zooming unless we use premultiplied alpha.. which we need an alpha channel for.  Trust me.
		surfType = SoftSurface::SURFACE_RGBA;
	}

	pSurf->Init(width, height, surfType);
	
	const uint32 size = pSurf->GetBytesPerPixel()*width*height;
	uint8* data = pSurf->GetPixelData();
	
	if (data)
	{
		if (pSurf->GetBytesPerPixel() == 4)
		{
			//copy and add alpha
			pSurf->FillColor(glColorBytes(0,0,0,0));

			if (cinfo.out_color_components == 3)
			{
					for (uint32 i=0,j=0; i<size; i+=4, j+=3)
					{
						data[i+0] = output[j+0];
						data[i+1] = output[j+1];
						data[i+2] = output[j+2];
						data[i+3] = 255;
					}
			} else
			{
				for (uint32 i=0,j=0; i<size; i+=4, j+=4)
				{
					// Also works without K, but has more contrast with K multiplied in
					data[i+0] = (char)(output[j+0]*(output[j+3]/255.f));
					data[i+1] = (char)(output[j+1]*(output[j+3]/255.f));
					data[i+2] = (char)(output[j+2]*(output[j+3]/255.f));
					data[i+3] = 255;
				}
			}
		} else
		{

			//no alpha

			if (cinfo.out_color_components == 3)
			{
				/*
					for (uint32 i=0,j=0; i<size; i+=3, j+=3)
					{
						data[i+0] = output[j+0];
						data[i+1] = output[j+1];
						data[i+2] = output[j+2];
					}
				*/
				memcpy(data, output, size);
			} else
			{
				for (uint32 i=0,j=0; i<size; i+=3, j+=4)
				{
					// Also works without K, but has more contrast with K multiplied in
					data[i+0] = (char)(output[j+0]*(output[j+3]/255.f));
					data[i+1] = (char)(output[j+1]*(output[j+3]/255.f));
					data[i+2] = (char)(output[j+2]*(output[j+3]/255.f));
				}
			}
		}
	}

	//jpg will be upside down if we don't do this.. uhh.. investigate why later.
	pSurf->FlipY();


	switch (orientation)
	{
	case 1:
		// Normal orientation, do nothing
		break;
	case 2:
		// Flip horizontally
		// You'll need to implement this if it's not already in SoftSurface
		pSurf->FlipX();
		break;
	case 3:
		// Rotate 180 degrees (rotate left twice)
		pSurf->Rotate90Degrees(true);
		pSurf->Rotate90Degrees(true);
		break;
	case 4:
		// Flip vertically
		pSurf->FlipY();
		break;
	case 5: 
		// Transpose (Flip horizontally then rotate left)
		pSurf->FlipX();
		pSurf->Rotate90Degrees(true);
		break;
	case 6:
		// Rotate 90 degrees CW (rotate left)
		pSurf->Rotate90Degrees(true);
		break;
	case 7:
		// Transverse (Flip horizontally then rotate right)
		pSurf->FlipX();
		pSurf->Rotate90Degrees(false);
		break;
	case 8:
		// Rotate 90 degrees CCW (rotate right)
		pSurf->Rotate90Degrees(false);
		break;
	}

	SAFE_DELETE_ARRAY(output);
	
	//if we wanted to test our decompression..
	//pSurf->WriteBMPOut("CRAP.BMP");
	return true; //success
}


bool JPGSurfaceLoader::SaveToFile(SoftSurface *pSource, string fileName, int quality)
{
	//credit to the Clanlib team, this code is based on their jpg writing code

	SoftSurface convertSurface;

	if (pSource->GetSurfaceType() != SoftSurface::SURFACE_RGB)
	{
		//it doesn't make sense to save a jpg in this format.  Convert it to RGB
		convertSurface.Init(pSource->GetWidth(), pSource->GetHeight(), SoftSurface::SURFACE_RGB);
		convertSurface.Blit(0, 0, pSource);
		pSource = &convertSurface; //use this instead
	}
	
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

						/* Step 1: allocate and initialize JPEG compression object */

						/* We have to set up the error handler first, in case the initialization
						* step fails.  (Unlikely, but it could happen if you are out of memory.)
						* This routine fills in the contents of struct jerr, and returns jerr's
						* address which we place into the link field in cinfo.
						*/
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	/* Here we use the library-supplied code to send compressed data to a
	* stdio stream.  You can also write your own code to do something else.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to write binary files.
	*/
	if ((outfile = fopen(fileName.c_str(), "wb")) == NULL)
	{
#ifdef _DEBUG
		LogError("Unable to write %s", fileName.c_str());
#endif
		return false;
	}

	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width =pSource->GetWidth(); 	/* image width and height, in pixels */
	cinfo.image_height = pSource->GetHeight();
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
										/* Now use the library's routine to set default compression parameters.
										* (You must set at least cinfo.in_color_space before calling this,
										* since the defaults depend on the source color space.)
										*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = pSource->GetWidth() * 3;	/* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) 
	{
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		row_pointer[0] = &static_cast<unsigned char*>(pSource->GetPixelData())[cinfo.next_scanline * row_stride];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	return true;
}
