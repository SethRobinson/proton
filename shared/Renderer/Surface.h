//  ***************************************************************
//  Surface - Creation date: 03/24/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Surface_h__
#define Surface_h__

#include "util/RenderUtils.h"
#include "util/boost/boost/signal.hpp"

class VariantList;

struct glColorBytes
{
	glColorBytes(){};
	glColorBytes(GLubyte _r, GLubyte _g, GLubyte _b, GLubyte _a) : r(_r), g(_g), b(_b), a(_a){};
	bool Compare(const glColorBytes &c) {return *(int*)&r == *(int*)&c.r;} //I couldn't get == working on this struct, a weird GUID error under windows?!, so I did this..
	GLubyte r, g, b, a;
};

struct glColorBytesRGB
{
	glColorBytesRGB() {};
	glColorBytesRGB(GLubyte _r, GLubyte _g, GLubyte _b) : r(_r), g(_g), b(_b) {};
	bool Compare(const glColorBytesRGB &c) { return *(int*)&r == *(int*)&c.r; } //I couldn't get == working on this struct, a weird GUID error under windows?!, so I did this..
	GLubyte r, g, b;
};

class SoftSurface;
class RenderBatcher;

#define NO_TEXTURE_LOADED (2000000000) //I had this as 4294967294u but saw some weirdness with some compiles

class Surface: public boost::signals::trackable
{
public:
	/**
	 * Type for the texture that the \c Surface class uses.
	 */
	enum eTextureType
	{
		/// Linear filtering, wrap mode is repeat, uses mipmaps.
		TYPE_DEFAULT,
		/// Linear filtering, wrap mode is clamp to edge, no mipmaps.
		TYPE_GUI,
		/// The texture is owned by somebody else and is responsible for setting the appropriate parameters.
		TYPE_NOT_OWNER,
		/// Nearest pixel filtering, wrap mode is clamp to edge, no mipmaps.
		TYPE_NO_SMOOTHING
	};

	enum eBlendingMode
	{
		BLENDING_NORMAL,
		BLENDING_ADDITIVE,
		BLENDING_PREMULTIPLIED_ALPHA,
		BLENDING_MULTIPLY,
		BLENDING_DARKEN
	};

	Surface();
	Surface (string fName); //load in an image right away
	Surface (string fName, eTextureType type); //load in an image right away
	virtual ~Surface();
	
	bool LoadFile(string fName, bool bAddBasePath = true); //will autodetect what kind of file it is
	virtual bool LoadFileFromMemory( byte *pMem, int inputSize=0 ); //will autodetect what kind of file it is
	void Bind();
	bool IsLoaded() {return m_glTextureID != NO_TEXTURE_LOADED || !m_textureLoaded.empty();}
	bool UsesAlpha() {return m_bUsesAlpha;}
	void SetUsesAlpha(bool bNew){m_bUsesAlpha = bNew;}

	int GetWidth() {return m_originalWidth;}
	int GetHeight() {return m_originalHeight;}

	int GetRawTextureWidth() {return m_texWidth;}
	int GetRawTextureHeight() {return m_texHeight;}

	/**
	 * Sets the texture type for this \c Surface.
	 *
	 * The default type is \link Surface::TYPE_DEFAULT \c TYPE_DEFAULT \endlink.
	 *
	 * \note You can only call this method before loading the texture.
	 */
	void SetTextureType(eTextureType type);

	virtual void Blit(  float x, float y, unsigned int rgba = MAKE_RGBA(255,255,255,255), float rotationDegrees = 0, CL_Vec2f vRotatePt = CL_Vec2f(0,0));
	virtual void BlitEx(rtRectf dst, rtRectf src, unsigned int rgba = MAKE_RGBA(255,255,255,255), float rotationDegrees = 0, CL_Vec2f vRotatePt = CL_Vec2f(0,0)); //more advanced version, can do scaling and sub-texture blits

	//these actually just call the above ones
	void BlitScaled( float x, float y, CL_Vec2f vScale, eAlignment alignment = ALIGNMENT_CENTER, unsigned int rgba  = MAKE_RGBA(255,255,255,255), float rotation=0,  RenderBatcher *pRenderBatcher = NULL);
	void BlitScaledWithRotatePoint( float x, float y, CL_Vec2f vScale, eAlignment alignment, unsigned int rgba, float rotation, CL_Vec2f vRotationPt, RenderBatcher *pRenderBatcher = NULL);
	void BlitRotated( float x, float y, CL_Vec2f vScale, eAlignment alignment = ALIGNMENT_CENTER, unsigned int rgba  = MAKE_RGBA(255,255,255,255), float rotation=0, CL_Vec2f vRotationPtLocalCoords =  CL_Vec2f(0,0), RenderBatcher *pRenderBatcher = NULL);

	rtRectf GetRectf() {return rtRectf(0,0, float(m_originalWidth), float(m_originalHeight));}
	void SetSmoothing( bool bSmoothing);
	void SetBlendingMode(eBlendingMode mode) {m_blendingMode = mode;}
	eBlendingMode GetBlendingMode() {return m_blendingMode;}
	int GetMIPMapCount() {return m_mipMapCount;}
	void SetCreateMipMapsIfNeeded(bool bCreateMipMapsIfNeeded) {m_bCreateMipMapsIfNeeded = bCreateMipMapsIfNeeded;}
	virtual bool InitBlankSurface(int x, int y); //initialize a blank surface to do whatever to
	virtual bool InitFromSoftSurface(SoftSurface *pSurf, bool bCreateSurface = true, int mipLevel = 0);
	bool IsRenderTarget() {return m_frameBuffer != 0;}
	void CopyFromScreen(); //grabs whatever is currently in the gl buffer and creates a new texture with it
	void FillRandomCrap();
	void UpdateSurfaceRect(rtRect dstRect, byte *pPixelData, bool bUpsideDownMode = true);
	void FillColor(glColorBytes color);
	virtual void Kill();
	void OnLoadSurfaces();
	void OnUnloadSurfaces();

	//if you wanted to just use the surface to load and setup states, then do your own manual blits but don't feel like
	//subclasses, you can use these:

	void SetupForRender(const float rotationDegrees = 0, const CL_Vec2f &vRotatePt = CL_Vec2f(0,0), const uint32 rgba = MAKE_RGBA(255,255,255,255)); //setup texture states, rotation if needed
	void EndRender(const float rotationDegrees = 0, const uint32 rgba = MAKE_RGBA(255,255,255,255)); //put stuff back to how it was
	void ApplyBlendingMode(uint32 rgba); //does the GL blending states on the active texture, used in renderbatcher
	void RemoveBlendingMode(uint32 rgba); //undoes the above, used in renderbatcher

	void HardKill(); //like kill, but also forces the texture to forget things about itself so it won't auto reinit if surfaces are lost

protected:

	
	virtual void ReloadImage();

	enum eTextureCreationMethod //helps me to know how to restore it when losing surfaces
	{
		TEXTURE_CREATION_NONE, //we haven't made a texture yet
		TEXTURE_CREATION_FILE, //we'll reload automatically
		TEXTURE_CREATION_MEMORY, //we'll lose it, but not restore it
		TEXTURE_CREATION_BLANK //we'll reinitialize the texture as blank, up to you to redraw it
	};


private:

	bool LoadBMPTexture(byte *pMem); //handles 32 bit (alpha ok) and 24 bit, no alpha
	bool LoadRTTexture(byte *pMem); //must be created with the RTPack.exe utility
	void IncreaseMemCounter(int mem);
	void SetTextureStates();
	void PrepareGLForNewTexture();
	void SetDefaults();
	void PreMultiplyAlpha(byte *pBytes, int width, int height, int format);
	void OnEnterForeground(VariantList *pVList); //only wired up in Windows currently, don't need it anywhere
	//else, windows is a special case
	GLuint m_glTextureID;
	int m_texWidth, m_texHeight;
	int m_originalWidth, m_originalHeight; //if the texture is padded, this info helps us to still draw it as if it wasn't
	bool m_bUsesAlpha;
	eTextureType m_texType;
	eBlendingMode m_blendingMode;
	int m_mipMapCount;
	GLuint m_frameBuffer;
	int m_memUsed; //for internal memory monitering
	string m_textureLoaded;
	bool m_bCreateMipMapsIfNeeded;
	eTextureCreationMethod m_textureCreationMethod;
	bool m_bSmoothing;
	bool m_bAddBasePath;
	
};

extern GLuint g_lastBound;
std::string PrintGLColor(glColorBytes color);

#endif // Surface_h__
