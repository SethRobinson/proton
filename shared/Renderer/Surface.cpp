#include "PlatformPrecomp.h"
#include "Surface.h"
#include "bitmap.h"
#include "BaseApp.h"
#include "RTGLESExt.h"
#include "SoftSurface.h"
#include <cmath>

#define GL_RGBA8 0x8058

const float C_TEXEL_HACK_AMOUNT= (0); //something I was experimenting around with but didn't need

GLuint g_lastBound = 0;

Surface::Surface()
{
	SetDefaults();
}

Surface::Surface( string fName )
{
	SetDefaults();
	LoadFile(fName);
}

Surface::Surface( string fName, eTextureType type )
{
	SetDefaults();
	SetTextureType(type);
	LoadFile(fName);
}

void Surface::SetDefaults()
{
	m_bSmoothing = true;
	m_texWidth = 0;
	m_texHeight = 0;
	m_memUsed = 0;
	m_glTextureID = NO_TEXTURE_LOADED;
	m_texType = TYPE_DEFAULT;
	m_blendingMode = BLENDING_NORMAL;
	m_mipMapCount = 0;
	m_frameBuffer = 0;
	m_originalHeight = 0; //sometimes useful to know, if case we had to pad the image to be power of 2 for instance
	m_originalWidth = 0;
	m_textureCreationMethod = TEXTURE_CREATION_NONE;
	m_bCreateMipMapsIfNeeded = false;
	m_bAddBasePath = true;
	
}

Surface::~Surface()
{
	Kill();
}

void Surface::Kill()
{
	if (m_glTextureID != NO_TEXTURE_LOADED)
	{
		glDeleteTextures( 1, &m_glTextureID );
		
#ifdef _DEBUG
		//LogMsg("Killing texture %s", m_textureLoaded.c_str());
#endif
		m_glTextureID = NO_TEXTURE_LOADED;
		GetBaseApp()->ModTexUsed(-m_memUsed);
		m_memUsed = 0;
	}
}

bool Surface::LoadBMPTexture(byte *pMem)
{
	
	BMPImageHeader *pBmpImageInfo = new BMPImageHeader;

		
	memcpy(pBmpImageInfo, &pMem[14], sizeof(BMPImageHeader)-14);


	unsigned short offsetToImageData;
	
	memcpy(&offsetToImageData, &pMem[10], 2);
	byte *pPixelData = &pMem[offsetToImageData];
	
#ifdef _DEBUG
LogMsg("Checking pixel data..");
#endif

//get around alignment issues on PLATFORM_HTML5	
	uint32 tempWidth, tempHeight;
	memcpy(&tempWidth, &pBmpImageInfo->Width, sizeof(uint32));
	memcpy(&tempHeight, &pBmpImageInfo->Height, sizeof(uint32));

#ifdef _DEBUG
	LogMsg("Did memcpy, width is %d", tempWidth);
#endif
	if (!IsPowerOf2(tempWidth) || !IsPowerOf2(tempHeight))
	{
		LogError("Bitmap dimensions needs to be of a power of 2, use RTPack on it first, this way it can still be used as if it was its original size");
		LogError("Or, use SoftSurface which can do this and has a better bmp loader in it.");
		SAFE_DELETE(pBmpImageInfo);
		return false;
	}

#ifdef _DEBUG
	LogMsg("It's power of 2...");
#endif

	m_texWidth = tempWidth;
	m_texHeight = tempHeight;

	m_originalWidth = m_texWidth;
	m_originalHeight = m_texHeight;

	GLenum colorFormat = GL_RGBA;
	GLenum pixelFormat = GL_UNSIGNED_BYTE;
	m_mipMapCount = 0;
	//we're going to have to convert it to 5551 or 4444 on our own

	uint16 *pFinal = 0;
	uint8 *pPixelDataToUse = pPixelData;
	
	//note, yes, bmps are stored upside down, but so do our gl textures so we don't flip the Y

	if (pBmpImageInfo->BitCount == 32)
	{
		//convert BGRA to RGBA.  I think there is actually an undocumented parm to make the iphone except RGBA but whatever
		int pixelCount = m_texWidth*m_texHeight;
		byte temp;
		for (int i=0; i < pixelCount; i++)
		{
		  temp = *pPixelData;
		  *pPixelData = pPixelData[2];
		  pPixelData[2] = temp;
		  pPixelData += 4;
		}
	} else
		if (pBmpImageInfo->BitCount == 24)
		{
			colorFormat = GL_RGB;		
			//convert BGR to RGB. 
			int pixelCount = m_texWidth*m_texHeight;
			byte temp;
			for (int i=0; i < pixelCount; i++)
			{
				temp = *pPixelData;
				*pPixelData = pPixelData[2];
				pPixelData[2] = temp;
				pPixelData += 3;
			}
		} 

	 else
	{
		LogError("Don't handle %d bit bmps yet", pBmpImageInfo->BitCount);
		SAFE_DELETE(pBmpImageInfo);
	
		return false;
	}
	
	PrepareGLForNewTexture();	
	m_bUsesAlpha = (colorFormat == GL_RGBA);
	
#ifdef _DEBUG
LogMsg("Loading BMP texture of size %d by %d", m_texWidth, m_texHeight);
#endif
	glTexImage2D( GL_TEXTURE_2D, 0, colorFormat, m_texWidth, m_texHeight, 0, colorFormat, pixelFormat, pPixelDataToUse );
	CHECK_GL_ERROR();
	
	IncreaseMemCounter(m_texWidth*m_texHeight* (pBmpImageInfo->BitCount/8));
	SetTextureStates();
	CHECK_GL_ERROR();
	SAFE_DELETE_ARRAY(pFinal);
	SAFE_DELETE(pBmpImageInfo);

	return true;
}

void Surface::IncreaseMemCounter(int mem)
{
	if (m_texType == TYPE_NOT_OWNER) return;

	assert(!m_memUsed && "Forgot to clear this somewhere?");
	m_memUsed = mem;
	GetBaseApp()->ModTexUsed(m_memUsed);
}
void Surface::PrepareGLForNewTexture()
{
	
	if (m_texType == TYPE_NOT_OWNER) return;

	CHECK_GL_ERROR();

	glGenTextures( 1, &m_glTextureID ); 
	CHECK_GL_ERROR();
	glBindTexture( GL_TEXTURE_2D, m_glTextureID);
	g_lastBound = m_glTextureID;
	CHECK_GL_ERROR();

	
	//glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
}

#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG			0x8C00 //35840
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG			0x8C01 //35841
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG			0x8C02 //35842
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG			0x8C03 //35843

#ifndef GL_UNSIGNED_SHORT_4_4_4_4
	#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#endif

int GetIntFromMemImplementation(byte *pMem)
{
	int temp;
	assert(sizeof(int) == 4);
	memcpy(&temp, pMem, sizeof(int));
	return temp;
}

#define GetIntFromMem(var) GetIntFromMemImplementation( (byte*)var)

void Surface::PreMultiplyAlpha(byte *pBytes, int width, int height, int format)
{
 	assert(format == GL_UNSIGNED_SHORT_4_4_4_4  || format == GL_UNSIGNED_BYTE && "This doesn't make sense premuliplying something that has no alpha!");
   
	if (pBytes == 0) 
	{
		LogMsg("Can't premult, it's null");
		return;
	}
	if (format == GL_UNSIGNED_SHORT_4_4_4_4)
	{
		uint16 *pDestImage = (uint16 *)pBytes;

		int r,g,b,a;

		for (int y=0; y < width; y++)
		{
			for (int x=0; x < height; x++)
			{
				
				//get the alpha
				a = (*pDestImage >> 0) & 15;
				
				/*
				//without modifying it, we'd do this:
				r = (((*pDestImage >> 0) & 15);
				g = ((*pDestImage >> 4) & 15);
				b = ((*pDestImage >> 8) & 15);
				*/
	
				r = (((*pDestImage >> 4) & 15)*a)/16;
				g = (((*pDestImage >> 8) & 15)*a)/16;
				b = (((*pDestImage >> 12) & 15)*a)/16;

				//LogMsg("a: %d, r %d g %d b %d", int(a), int(r), int(g), int(b));
				
				//pack it back in
				
				*pDestImage = a << 0 | r << 4 | g << 8 | b << 12;
				pDestImage++;
			}
		}

	} else if (format == GL_UNSIGNED_BYTE)
	{
 
		glColorBytes *pDestImage = (glColorBytes*)pBytes;

		//slower way that supports transparency
		for (int y=0; y < width; y++)
		{
			for (int x=0; x < height; x++)
			{
				pDestImage->r = (pDestImage->r*pDestImage->a)/255;
				pDestImage->g = (pDestImage->g*pDestImage->a)/255;
				pDestImage->b = (pDestImage->b*pDestImage->a)/255;
				pDestImage++;
			}
		}
	} else
	{
		LogError("Don't know how to premultiply this alpha");
		assert(0);
	}

}

bool Surface::LoadRTTexture(byte *pMem)
{
	CHECK_GL_ERROR();
	rttex_header *pTexHeader = (rttex_header*)pMem;
	rttex_mip_header *pMipSection;

	m_texWidth = pTexHeader->width;
	m_texHeight = pTexHeader->height;
	m_bUsesAlpha = pTexHeader->bUsesAlpha != 0;
	m_originalWidth = pTexHeader->originalWidth;
	m_originalHeight = pTexHeader->originalHeight;
	m_mipMapCount = pTexHeader->mipmapCount;
	byte *pCurPos = pMem + sizeof(rttex_header);

	int memUsed = 0;
#ifdef _DEBUG
	int rttexHeaderSize = sizeof(rttex_header);
	int rttexMipSectionSize = sizeof(rttex_mip_header);

	assert (rttexHeaderSize%4 == 0 && "Can't be right, structure packing changed?");
	assert (rttexMipSectionSize%4 == 0 && "Can't be right, structure packing changed?");
#endif
	int format = GetIntFromMem(&pTexHeader->format);

	if (m_bCreateMipMapsIfNeeded)
	{
		if (m_mipMapCount == 1)
		{
			m_mipMapCount = 8; //guess, exact # doesn't matter, just must be more than 1
			glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
		} else
		{
			glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
		}
	}
CHECK_GL_ERROR();
	for (int nMipLevel=0; nMipLevel < pTexHeader->mipmapCount; nMipLevel++)
	{
		pMipSection = (rttex_mip_header*)pCurPos;
		pCurPos += sizeof(rttex_mip_header);
		byte *pTextureData =  pCurPos ;
		memUsed += pMipSection->dataSize;

		//actually, let's move the texture data into something byte aligned.. it shouldn't need this but if I don't,
		//crashes in release mode on iPhone 3GS and older when using xcode 4 and targetting OS 3.2+.  It should already be
		//int aligned (at least for mip 0..) so I don't know why this is required.  -Seth

		pTextureData = new byte[pMipSection->dataSize];
		memcpy(pTextureData, pCurPos,pMipSection->dataSize );
		//let's do it manually now
		int width = GetIntFromMem(&pMipSection->width);
		int height = GetIntFromMem(&pMipSection->height);
	
		if (nMipLevel == 0)
		{
			PrepareGLForNewTexture(); //if we're not the owner, this will be ignored
		}

		if (format == RT_FORMAT_EMBEDDED_FILE)
		{
			if (*((uint16*)pTextureData) != C_JPG_HEADER_MARKER)
			{
				//if it's "embedded", but not jpg, let's assume it is raw RGB,.. this way I don't have to use jpg for all the tiny mipmaps.  
				//I may want to put the format in rttex_mip_header to allow more flexibility though..
				int colorType = GL_RGB;
				int internalColorFormat = colorType;
					#ifdef C_GL_MODE
									if (internalColorFormat == GL_RGBA) internalColorFormat = GL_RGBA8;
									if (internalColorFormat == GL_RGB) internalColorFormat = GL_RGB8;
					#endif
				
					
					glTexImage2D( GL_TEXTURE_2D, nMipLevel, internalColorFormat, width, height, 0, colorType, GL_UNSIGNED_BYTE, pTextureData );
					CHECK_GL_ERROR();
			} else
			{
			
				SoftSurface s;

				if (!s.LoadFileFromMemory(pTextureData, SoftSurface::COLOR_KEY_NONE, pMipSection->dataSize))
				{
					LogMsg("(Failed to load image inside of rttex)");
					assert(!"Failed to load image inside of rttex");
					return false;
				}
	
				SAFE_DELETE_ARRAY(pTextureData);
				bool bResult = InitFromSoftSurface(&s, false, nMipLevel);

				if (!bResult)
				{
					LogMsg("Failed to init from surface?");
					assert(0);
					return bResult;
				}
			}

		} else
		{
			if (pTexHeader->format < GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG || pTexHeader->format > GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG) //doesn't look like a compressed texture
			{
				int colorType = GL_RGBA;
				if (!m_bUsesAlpha)
				{
					colorType = GL_RGB;
				}
				
#ifdef _DEBUG
				//LogMsg("Loading surface: miplevel %d, colortype 0x%02lX, x%d y%d, format type: 0x%02lX", nMipLevel, colorType, pMipSection->width, pMipSection->height, pTexHeader->format );
#endif
				
				int internalColorFormat = colorType;
#if defined(C_GL_MODE) && !defined(PLATFORM_HTML5)

				if (internalColorFormat == GL_RGBA) internalColorFormat = GL_RGBA8;
				if (internalColorFormat == GL_RGB) internalColorFormat = GL_RGB8;
#endif

				if (m_texType == TYPE_GUI && m_bUsesAlpha)
				{
					//Go ahead and activate premultiplied alpha processing.  This will allow us to zoom in on textures and not have ugly artifacts
					//around the edges.
					
					m_blendingMode = BLENDING_PREMULTIPLIED_ALPHA;
				}

				if (m_blendingMode == BLENDING_PREMULTIPLIED_ALPHA && m_bUsesAlpha)
				{
					PreMultiplyAlpha(pTextureData, width, height, format);
				}

				glTexImage2D( GL_TEXTURE_2D, nMipLevel, internalColorFormat, width, height, 0, colorType, format, pTextureData );
CHECK_GL_ERROR();				
#ifdef _DEBUG
				if (nMipLevel == 0) LogMsg("Surface::LoadRTTexture: Loading surface: %d, %d -  internalFormat: %d, format: %d color type: %d", width, height, internalColorFormat, format, colorType);
			
				#endif
			} else
			{

#if defined(C_GL_MODE) || defined(RT_GLES_ADAPTOR_MODE) || defined(RT_USING_OSMESA)

				assert(!"You cannot use PVR compressed textures in GL mode!");
#else

				glCompressedTexImage2D(
					GL_TEXTURE_2D, 
					nMipLevel, 
					pTexHeader->format, 
					pMipSection->width, 
					pMipSection->height, 
					0, 
					pMipSection->dataSize, 
					pTextureData);
#endif
			}

		}

		pCurPos += pMipSection->dataSize;

		CHECK_GL_ERROR();
		SAFE_DELETE_ARRAY(pTextureData);

	}

	IncreaseMemCounter(memUsed);
	SetTextureStates();
#ifndef PLATFORM_HTML5
	glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
	//unknown parm in emscripten's emulated GL1 support
#endif
	CHECK_GL_ERROR();
	return true;
}

bool Surface::LoadFileFromMemory( byte *pMem, int inputSize )
{
	Kill();
	
	if (!pMem)
	{
		assert(!"LoadFileFromMemory sent a null pointer?  AW HELL NAW");
		return false;
	}
	CHECK_GL_ERROR();
	bool bReturn = false;

	if (m_texWidth == 0)
	{
		if (m_textureCreationMethod == TEXTURE_CREATION_NONE)
		{
			m_textureCreationMethod = TEXTURE_CREATION_MEMORY;
		} else
		{
			//load on demand, helps with speed
		}
	
		//unload happens right away though
		GetBaseApp()->m_sig_unloadSurfaces.connect(1, boost::bind(&Surface::OnUnloadSurfaces, this));
	
#ifdef WINAPI
		GetBaseApp()->m_sig_enterforeground.connect(1, boost::bind(&Surface::OnEnterForeground, this, _1));
#endif

		CHECK_GL_ERROR();
	}

	if (*((uint16*)pMem) == C_JPG_HEADER_MARKER)
	{
		SoftSurface s;
		if (!s.LoadFileFromMemory(pMem, SoftSurface::COLOR_KEY_NONE, inputSize, false))
		{
			LogMsg("(Failed to load jpg)");
			return false;
		}
		bReturn = InitFromSoftSurface(&s);
		
	}else
	if (strncmp((char*)pMem, "BM", 2) == 0)
	{
		//we've got a bitmap on our hands it looks like
		bReturn = LoadBMPTexture(pMem);
		CHECK_GL_ERROR();

	} else if (strncmp((char*)pMem, C_RTFILE_TEXTURE_HEADER, 6) == 0)
	{
		bReturn = LoadRTTexture(pMem);
		CHECK_GL_ERROR();

	} else
	{
		LogError("Surface: Unknown file type");
		assert(!"Unknown file type");
		return false; 
	}

	CHECK_GL_ERROR();
	if (bReturn)
	{
		SetSmoothing(m_bSmoothing);
	}
	return bReturn;
}


void Surface::Bind()
{
	CHECK_GL_ERROR();
	if (m_texType == TYPE_NOT_OWNER) return;

	if (m_glTextureID == NO_TEXTURE_LOADED && !m_textureLoaded.empty())
	{
		ReloadImage();
	}

	if (g_lastBound == m_glTextureID)
	{
		//believe it or not, I saved 2 FPS on the iphone by doing my own checks
		return;
	}

	glBindTexture( GL_TEXTURE_2D, m_glTextureID);
	g_lastBound = m_glTextureID;
	CHECK_GL_ERROR();

}

bool Surface::LoadFile( string fName, bool bAddBasePath )
{
	if (fName.empty())
	{
		Kill();
		return true; //no error, but I guess they just don't want anything loaded right now
	}
	
#ifdef _DEBUG
	//LogMsg("Loading texture %s", fName.c_str());
#endif

	m_bAddBasePath = bAddBasePath;
	
	FileInstance f(fName, m_bAddBasePath);
	if (!f.IsLoaded()) 
	{
		LogMsg("Couldn't load surface %s", fName.c_str());
		return false;
	}
	
	m_textureLoaded = fName;
	m_textureCreationMethod = TEXTURE_CREATION_FILE;
	return LoadFileFromMemory(f.GetAsBytes(), f.GetSize());
}

typedef struct
{
	int         u;
	int         v;
	int         w;
	int         h;
} TextureCoords;

struct TexCoords
{
	float x, y;
};

struct Verts
{
	float x,y,z;
};

void Surface::ApplyBlendingMode(uint32 rgba)
{

	if (UsesAlpha() || rgba != PURE_WHITE || m_blendingMode != BLENDING_NORMAL)
	{
		glEnable( GL_BLEND );

		switch (m_blendingMode)
		{
		case BLENDING_NORMAL:
			glColor4x(GET_RED(rgba) << 8, GET_GREEN(rgba) << 8, GET_BLUE(rgba) << 8, GET_ALPHA(rgba) << 8);
			break;

		case BLENDING_ADDITIVE:
			glBlendFunc( GL_SRC_ALPHA, GL_ONE);
			glColor4x(GET_RED(rgba) << 8, GET_GREEN(rgba) << 8, GET_BLUE(rgba) << 8, GET_ALPHA(rgba) << 8);
			break;

		case BLENDING_PREMULTIPLIED_ALPHA: {
			glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
			const int alpha = GET_ALPHA(rgba);
			glColor4x( GET_RED(rgba) * alpha, GET_GREEN(rgba) * alpha, GET_BLUE(rgba) * alpha, alpha << 8);
										   }
			break;

		case BLENDING_MULTIPLY:
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
			glColor4x(GET_RED(rgba) << 8, GET_GREEN(rgba) << 8, GET_BLUE(rgba) << 8, GET_ALPHA(rgba) << 8);
			break;
		case BLENDING_DARKEN:
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
			glColor4x(GET_RED(rgba) << 8, GET_GREEN(rgba) << 8, GET_BLUE(rgba) << 8, GET_ALPHA(rgba) << 8);
			break;
		}
	}
}
void Surface::SetupForRender(const float rotationDegrees, const CL_Vec2f &vRotatePt, const uint32 rgba)
{
	SetupOrtho(); //upside down, makes this easier to do
	CHECK_GL_ERROR();
	g_globalBatcher.Flush();
	CHECK_GL_ERROR();
	Bind();
	
	//LogMsg("Rendering tex %d at %s at %d", m_glTextureID, PrintRect(dst).c_str(), GetTick(TIMER_GAME));

	if (rotationDegrees != 0)
	{
		PushRotationMatrix(rotationDegrees, vRotatePt);
	}
	ApplyBlendingMode(rgba);
	CHECK_GL_ERROR();
}

void Surface::RemoveBlendingMode(uint32 rgba)
{

	if (UsesAlpha() || rgba != PURE_WHITE || m_blendingMode != BLENDING_NORMAL)
	{
		glColor4x(1 << 16, 1 << 16, 1 << 16, 1 << 16);
		glDisable( GL_BLEND );

		if (m_blendingMode != BLENDING_NORMAL)
		{
			//put it back to how it was
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}
	}

}

void Surface::HardKill()
{
	Kill();
	m_originalHeight = m_originalWidth = 0; //this means it won't auto recreate the surface if surfaces are lost
}

void Surface::EndRender(const float rotationDegrees,  const uint32 rgba)
{

	RemoveBlendingMode(rgba);

	if (rotationDegrees != 0)
	{
		PopRotationMatrix();
	}

}

void Surface::BlitEx(rtRectf dst, rtRectf src, unsigned int rgba, float rotation, CL_Vec2f vRotatePt)
{
	if (!IsLoaded()) return;

	if (dst.bottom < 0) return;
	if (dst.top > GetOrthoRenderSizeYf()) return;

	if ( GET_ALPHA(rgba) == 0)
	{
		return;	
	}

if (GetBaseApp()->GetDisableSubPixelBlits())
{
	//fix issue for cracks when scaling when 2d tile blits

	dst.left = ceil(dst.left)+C_TEXEL_HACK_AMOUNT;
	dst.top = ceil(dst.top)+C_TEXEL_HACK_AMOUNT;
	dst.bottom = ceil(dst.bottom)-C_TEXEL_HACK_AMOUNT;
	dst.right = ceil(dst.right)-C_TEXEL_HACK_AMOUNT;;
}


	SetupForRender(rotation, vRotatePt, rgba);

	if (rotation != 0)
	{
		dst.AdjustPosition(-vRotatePt.x, -vRotatePt.y);
	}

	//	0 1
	//	3 2

	static GLfloat	vertices[3*4];

	vertices[0*3+0] = dst.left; vertices[0*3+1] = dst.top;
	vertices[1*3+0] = dst.right; vertices[1*3+1] = dst.top;
	vertices[2*3+0] = dst.right; vertices[2*3+1] = dst.bottom;
	vertices[3*3+0] = dst.left; vertices[3*3+1] = dst.bottom;

	//set the Z
	vertices[0*3+2] = 0;
	vertices[1*3+2] = 0;
	vertices[2*3+2] = 0;
	vertices[3*3+2] = 0;


	static GLfloat vTexCoords[8];
	//another step to convert the coordinates into ratios
	static float texW;
	texW = float(m_originalWidth)/float(m_texWidth);
	static float texH;
	texH = float(m_originalHeight)/float(m_texHeight);
	static TexCoords *pTex;

	pTex = (TexCoords*)vTexCoords;

	//vert 0
	pTex->x = src.left/float(m_originalWidth) * texW;
	pTex->y =  (1-texH) + texH*( (m_originalHeight-src.top) /float(m_originalHeight));
	pTex++;

	//vert 1
	pTex->x = src.right/float(m_originalWidth) * texW;
	pTex->y = (1-texH) + texH*( (m_originalHeight-src.top) /float(m_originalHeight));
	pTex++;

	//vert 2
	pTex->x = src.right/float(m_originalWidth) * texW;
	pTex->y = 1-(texH*(src.bottom /(float(m_originalHeight))));
	pTex++;

	//vert 3
	pTex->x = src.left/float(m_originalWidth) * texW;
	pTex->y = 1-(texH*(src.bottom /(float(m_originalHeight))));
	pTex++;

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT,  0, vTexCoords);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	CHECK_GL_ERROR();
	EndRender(rotation, rgba);
}

void Surface::BlitScaled( float x, float y, CL_Vec2f vScale, eAlignment alignment, unsigned int rgba, float rotation, RenderBatcher *pRenderBatcher)
{
	BlitScaledWithRotatePoint(x,y,vScale,alignment,rgba,rotation,CL_Vec2f(x,y), pRenderBatcher);
}

void Surface::BlitScaledWithRotatePoint( float x, float y, CL_Vec2f vScale, eAlignment alignment, unsigned int rgba, float rotation, CL_Vec2f vScreenRotationPt, RenderBatcher *pRenderBatcher)
{
	assert(vScale.x != 0 && vScale.y != 0 && "Dahell?");

	CL_Vec2f vStart = CL_Vec2f(x,y);
	rtRectf src(0,0,(float)m_originalWidth,(float)m_originalHeight);
	rtRectf dst = src;
	vStart -= GetAlignmentOffset(CL_Vec2f(float(m_originalWidth), float(m_originalHeight)), eAlignment(alignment));
	dst.AdjustPosition(vStart.x, vStart.y);
	dst.Scale(alignment, vScale);

	if (pRenderBatcher)
	{
		//rotation ignored for now
		pRenderBatcher->BlitEx(this, dst, src, rgba);
	} else
	{
		BlitEx(dst, src, rgba, rotation, vScreenRotationPt);
	}
}


void Surface::Blit( float x, float y, unsigned int rgba, float rotationDegrees, CL_Vec2f vRotatePt)
{
	if (!IsLoaded()) return;

	if (GetBaseApp()->GetDisableSubPixelBlits())
	{
		//fix issue for cracks when scaling when 2d tile blits
		x = ceil(x);
		y = ceil(y);
	}

	if (rotationDegrees != 0)
	{
		x -= vRotatePt.x;
		y -= vRotatePt.y;
	}

	SetupForRender(rotationDegrees, vRotatePt, rgba);

	//LogMsg("Rendering tex %d at %.2f ,%.2f at time %d", m_glTextureID, x,y, GetTick(TIMER_GAME));

	GLfloat	vertices[] = {
		x,			y,			0.0,
		x + m_originalWidth,		y,			0.0,
		x + m_originalWidth,		m_originalHeight+y,		0.0,
		x ,			m_originalHeight+y,		0.0 };

		//3 2
		//0 1
		
		float texW = float(m_originalWidth)/float(m_texWidth);
		float texH = float(m_originalHeight)/float(m_texHeight);

		GLfloat vTexCoords[] = 
		{
			0, 1,
			texW, 1,
			texW, 1-texH,
			0, 1-texH
		};
	
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT,  0, vTexCoords);
		CHECK_GL_ERROR();

		assert((rgba&0xFF)*256 != 0 && "Why send something with zero alpha?");
	
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		CHECK_GL_ERROR();

		EndRender(rotationDegrees, rgba);
}

void Surface::SetTextureType( eTextureType type )
{
	assert(!IsLoaded() && "You should change this to able to work on an existing texture");
	m_texType = type;
}

void Surface::SetSmoothing( bool bSmoothing )
{
	if (IsLoaded())
	{
		Bind();
		if (!bSmoothing)
		{
			glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		} else
		{
			glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		}
		CHECK_GL_ERROR();
	}
	
	m_bSmoothing = bSmoothing; //remember for later if we have to restore surfaces
}

template <typename T>
T nextPowerOfTwo(T n)
{
	std::size_t k=1;
	--n;
	do {
		n |= n >> k;
		k <<= 1;
	} while (n & (n + 1));
	return n + 1;
}


/*
//! Bind Render Target Texture
void COGLES1Texture::bindRTT()
{
	glViewport(0, 0, getSize().Width, getSize().Height);
}


//! Unbind Render Target Texture
void COGLES1Texture::unbindRTT()
{
	glBindTexture(GL_TEXTURE_2D, getOGLES1TextureName());

	// Copy Our ViewPort To The Texture
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, getSize().Width, getSize().Height);
}
*/


enum eBlitMode
{
	BLIT_MODE_NORMAL,
	BLIT_MODE_ROTATE_LEFT
};

void BlitBmp(int posX, int posY, byte *pDest, int dstX, int dstY, int dstPixelType, byte *pSrc, int srcX, int srcY, int srcPixelType, eBlitMode mode)
{
	assert(dstPixelType == GL_RGBA && srcPixelType == GL_RGBA);

	int dstBytesPerPix = 4;
	int dstPitch = dstX*dstBytesPerPix;

	//memset(pDest+dstPitch, 0, dstX* (dstY-10 )*4 );

	//return;

	byte *pDstCur = pDest+(  (posY+ (dstY-srcY)   )*dstPitch)+posX;

	for (int y=0; y < srcY; y++)
	{
		memcpy(pDstCur, pSrc, srcX*dstBytesPerPix);
		pSrc += srcX*dstBytesPerPix;
		pDstCur += dstPitch;
	}
}

void Surface::CopyFromScreen()
{
	int readX = GetPrimaryGLX();
	int readY = GetPrimaryGLY();

	int bytesPerPix = 4;

	byte * pBuff = new byte [readX*readY*bytesPerPix];

	//rotate it before we copy it
	int srcY = 0;
	CHECK_GL_ERROR();

	glReadPixels(0, srcY, readX, readY, GL_RGBA, GL_UNSIGNED_BYTE, pBuff);
	CHECK_GL_ERROR();
	//now copy it to our texture
	assert(m_glTextureID != NO_TEXTURE_LOADED && "Texture can't be used yet");

	//BlitBmp(0,0, pFinal, m_texWidth, m_texHeight, GL_RGBA, pBuff, readX, readY, GL_RGBA, BLIT_MODE_ROTATE_LEFT);

	Bind();
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_texWidth, m_texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,  pFinal );
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_texHeight-readY, readX, readY, GL_RGBA, GL_UNSIGNED_BYTE, pBuff);
	CHECK_GL_ERROR();
	delete [] pBuff;
}

//used in debugging
void Surface::FillRandomCrap()
{
		int width = GetWidth();
		int height = GetHeight();
		int bytesPerPixel = 4;

		byte *pBuf = new byte[width*height*bytesPerPixel];

		glColorBytes *pPixel = (glColorBytes*)pBuf;
		
		for (int i = 0; i < width*height; i++)
		{
			//*pPixel = color;
			pPixel->a = 255;
			pPixel->r = Random(255);
			pPixel->g = Random(255);
			pPixel->b = Random(255);

			pPixel++;
		}

		UpdateSurfaceRect(rtRect(0, 0, width, height), pBuf);
		delete[] pBuf;
	
}

void Surface::FillColor(glColorBytes color)
{
	int width = GetWidth();
	int height = GetHeight();
	int bytesPerPixel = 4;

	byte *pBuf = new byte[width*height*bytesPerPixel];
	
	glColorBytes *pPixel = (glColorBytes*)pBuf;
	for (int i=0; i < width*height; i++)
	{
		*pPixel = color;
		pPixel++;
	}

	UpdateSurfaceRect(rtRect(0,0,width,height), pBuf);
	delete [] pBuf;
}

void Surface::UpdateSurfaceRect(rtRect dstRect, byte *pPixelData, bool bUpsideDownMode)
{
	Bind();
	int yStart = dstRect.top;
	
	if (bUpsideDownMode)
	{
		yStart += (m_texHeight-m_originalHeight);
	} 

	glTexSubImage2D(GL_TEXTURE_2D, 0, dstRect.left, yStart, dstRect.GetWidth(), dstRect.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, pPixelData);
	CHECK_GL_ERROR();
}

void Surface::SetTextureStates()
{
CHECK_GL_ERROR();
	switch(m_texType)
	{
	case TYPE_DEFAULT:
		glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

		if (m_mipMapCount > 1)
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		break;

	case TYPE_GUI:
		glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;


	case TYPE_NO_SMOOTHING:
		glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterx( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
            
        default: ;

	}

	CHECK_GL_ERROR();
}



bool Surface::InitBlankSurface( int x, int y)
{
	HardKill();

	assert(x && y);
	GLenum colorFormat = GL_RGBA;
	GLenum pixelFormat = GL_UNSIGNED_BYTE;
	int bytesPerPixel = 4;
	m_textureCreationMethod = TEXTURE_CREATION_BLANK;
		
	if (m_texWidth == 0)
	{
		//load manually, these kinds of surfaces are probably needed right away
		GetBaseApp()->m_sig_loadSurfaces.connect(1, boost::bind(&Surface::OnLoadSurfaces, this));

		//unload happens right away though
		GetBaseApp()->m_sig_unloadSurfaces.connect(1, boost::bind(&Surface::OnUnloadSurfaces, this));

#ifdef WINAPI
		GetBaseApp()->m_sig_enterforeground.connect(1, boost::bind(&Surface::OnEnterForeground, this, _1));
#endif
	}


	m_originalWidth = x;
	m_originalHeight = y;

	m_texWidth =  nextPowerOfTwo(x);
	m_texHeight =  nextPowerOfTwo(y);
	
	//make a surface filled with 'unknown'

	PrepareGLForNewTexture();	
	
	m_bUsesAlpha = (colorFormat == GL_RGBA);
	
	byte *pPixelData = NULL;
	
	int dataSize = m_texWidth*m_texHeight*bytesPerPixel;

	bool bClearFirst = true;

if (bClearFirst)
{
	pPixelData = new byte[dataSize];
	assert(pPixelData);
	if (!pPixelData)
	{
		LogMsg("Low mem?");
		return false;
	}
	memset(pPixelData, 0, dataSize);
}

	int internalColorFormat = colorFormat;

/*
#ifdef C_GL_MODE
	if (internalColorFormat == GL_RGBA) internalColorFormat = GL_RGBA8;
	if (internalColorFormat == GL_RGB) internalColorFormat = GL_RGB8;

#endif
 */
	
	glTexImage2D( GL_TEXTURE_2D, 0, internalColorFormat, m_texWidth, m_texHeight, 0, colorFormat, pixelFormat, pPixelData );

	SAFE_DELETE_ARRAY(pPixelData);
	IncreaseMemCounter(dataSize);
	SetTextureStates();
	CHECK_GL_ERROR();


	CHECK_GL_ERROR();
	return true;
}

void Surface::ReloadImage()
{
	LoadFile(m_textureLoaded, m_bAddBasePath);
}

void Surface::OnLoadSurfaces()
{
	if (m_glTextureID != NO_TEXTURE_LOADED) return; //already loaded I guess


		switch (m_textureCreationMethod)
		{
		case TEXTURE_CREATION_FILE:

			#ifdef _DEBUG		
			LogMsg("Reloading texture %s", m_textureLoaded.c_str());
			#endif
			ReloadImage();
			break;

		case TEXTURE_CREATION_BLANK:

			if (m_originalWidth != 0)
			{
			//if m_originalWidth != 0 then HardKill was probably used, which means it doesn't want us to do this
				
	#ifdef _DEBUG		
				LogMsg("Recreating surface of %d, %d", m_originalWidth, m_originalHeight);
	#endif
				InitBlankSurface(m_originalWidth, m_originalHeight);
			}
			else
			{
			#ifdef _DEBUG	
				LogMsg("Not restoring surface, it has no originalWidth data");
			#endif	

			}
			break;
                
            default: ;
		}
}

void Surface::OnUnloadSurfaces()
{

	if (m_glTextureID != NO_TEXTURE_LOADED)
	{
	#ifdef _DEBUG
		LogMsg("Unloading %d (%s)", m_glTextureID, m_textureLoaded.c_str());
	#endif
		Kill();
	}
}

bool Surface::InitFromSoftSurface( SoftSurface *pSurf, bool bCreateSurface, int mipLevel )
{
	int dataSize = 0;
	assert(pSurf->GetWidth() && pSurf->GetHeight());
	GLenum colorFormat = GL_RGBA;
	GLenum pixelFormat = GL_UNSIGNED_BYTE;
	int bytesPerPixel = 4;

	if (pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGB) 
	{
		colorFormat = GL_RGB;
		bytesPerPixel = 3;
	}

	if (mipLevel == 0)
	{

		m_texWidth =  nextPowerOfTwo(pSurf->GetWidth());
		m_texHeight =  nextPowerOfTwo(pSurf->GetHeight());

		if (m_originalHeight == 0) m_originalHeight = pSurf->GetOriginalHeight();
		if (m_originalWidth == 0) m_originalWidth = pSurf->GetOriginalWidth();
		dataSize = m_texWidth*m_texHeight*bytesPerPixel;

	} else
	{
		//it's a mipmap.  We assume these are power of 2 already
		
		dataSize = pSurf->GetWidth()*pSurf->GetHeight()*bytesPerPixel;
	}

	if (bCreateSurface)
	{
		Kill();
		PrepareGLForNewTexture();
		if (m_textureLoaded.empty())
		{
			m_textureCreationMethod = TEXTURE_CREATION_BLANK;
		}
	}

	m_bUsesAlpha = (colorFormat == GL_RGBA);
	
	int internalColorFormat = colorFormat;
		
	if ( (m_texHeight == pSurf->GetHeight() && m_texWidth == pSurf->GetWidth()) || mipLevel > 0)
	{
		//great, no size trickery needed
		//pSurf->FillColor(glColorBytes(255,0,0,255));
#ifdef _DEBUG
		//write out every texture loaded this way, used it for debugging a problem with mimaps
/*
		static int debugCount = 0;
		debugCount++;
		pSurf->WriteBMPOut(GetBaseAppPath()+"test_"+toString(debugCount)+"_"+toString(mipLevel)+".bmp");
		*/

#endif

		//assert(colorFormat == GL_RGB);
		glTexImage2D( GL_TEXTURE_2D, mipLevel, internalColorFormat, pSurf->GetWidth(), pSurf->GetHeight(), 0, colorFormat, pixelFormat, pSurf->GetPixelData() );
	
	} else
	{
		SoftSurface s;
		SoftSurface::eSurfaceType tempSurfType = SoftSurface::SURFACE_RGB;

		if (colorFormat == GL_RGBA)
		{
			tempSurfType = SoftSurface::SURFACE_RGBA;
		}

		s.Init(m_texWidth, m_texHeight, tempSurfType);
		byte *pPixelData = s.GetPixelData();

		assert(pPixelData);
		if (!pPixelData)
		{
			LogMsg("Low mem?");
			return false;
		}

		memset(pPixelData,0, dataSize);
	
		#ifdef RT_GLES_ADAPTOR_MODE
			//do it the simpler way, for Flash
			int yStart = 0;
			bool bUpsideDownMode = true;
			if (bUpsideDownMode)
			{
				yStart += (m_texHeight-m_originalHeight);
			} 
			s.Blit(0,yStart, pSurf);
			glTexImage2D( GL_TEXTURE_2D, mipLevel, internalColorFormat, m_texWidth, m_texHeight, 0, colorFormat, pixelFormat, pPixelData );

		#else
			//normal way
			glTexImage2D( GL_TEXTURE_2D, mipLevel, internalColorFormat, m_texWidth, m_texHeight, 0, colorFormat, pixelFormat, pPixelData );

			int yStart = 0;
			bool bUpsideDownMode = true;
			if (bUpsideDownMode)
			{
				yStart += (m_texHeight-m_originalHeight);
			} 

			assert(pSurf->GetWidth()%4 == 0 && "This will probably crash glTexSubImage2D.  Pad your textures to multiples of 4, or use the .rttex format.");
			//note: We could get around the error above with glPixelStorei(GL_PACK_ALIGNMENT, 1), but I don't think
			//it's available on all platforms so not going to bother
			
			glTexSubImage2D(GL_TEXTURE_2D, mipLevel, 0, yStart, pSurf->GetWidth(), pSurf->GetHeight(), internalColorFormat, pixelFormat, pSurf->GetPixelData());
		#endif
	}

	if (bCreateSurface && mipLevel == 0)
	{
		IncreaseMemCounter(dataSize);
		SetTextureStates();
	}

	CHECK_GL_ERROR();
	return true;
}


void Surface::BlitRotated( float x, float y, CL_Vec2f vScale, eAlignment alignment /*= ALIGNMENT_CENTER*/,
						  unsigned int rgba /*= MAKE_RGBA(255,255,255,255)*/, float rotation/*=0*/,
						  CL_Vec2f vRotationPtLocalCoords /*= CL_Vec2f(0,0)*/, RenderBatcher *pRenderBatcher)
{
	BlitScaledWithRotatePoint(x,y,vScale, alignment, rgba, rotation, CL_Vec2f(x,y)+vRotationPtLocalCoords, pRenderBatcher);
}

void Surface::OnEnterForeground(VariantList *pVList)
{
	//only called on windows, to restore lost properties after doing a Alt-Enter full screen toggle or changing
	//the res.  Surfaces aren't actually reloaded but they lose some GL settings.. maybe a OnChangeResize signal
	//would be better for this.

	SetSmoothing(m_bSmoothing);
}

string PrintGLColor(glColorBytes color)
{
	char st[128];
	sprintf(st, "%d, %d, %d, %d", color.r, color.g, color.b, color.a);
	return string(st);
}