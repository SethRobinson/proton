#include "PlatformPrecomp.h"
#include "JPGSurfaceLoader.h"
#include "SoftSurface.h"

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

bool JPGSurfaceLoader::LoadFromMem( byte *pMem, int inputSize, SoftSurface *pSurf, bool bAddAlphaChannelIfNotPowerOfTwo )
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
	SAFE_DELETE_ARRAY(output);
	
	//if we wanted to test our decompression..
	//pSurf->WriteBMPOut("CRAP.BMP");
	return true; //success
}