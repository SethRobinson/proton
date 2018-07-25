#include "PlatformPrecomp.h"
#include "SurfaceAnim.h"
#include "Renderer/RenderBatcher.h"

SurfaceAnim::SurfaceAnim()
{
	m_framesX = 1;
	m_framesY = 1;
	m_frameWidth = 0;
	m_frameHeight = 0;
}

SurfaceAnim::~SurfaceAnim()
{
}
 
void SurfaceAnim::SetupAnim( int framesX, int framesY )
{
	assert(IsLoaded() && "Load your image before doing SetupAnim on it");

	m_framesX = framesX;
	m_framesY = framesY;

	//assert(! (GetWidth() % m_framesX) && "image Doesn't evenly divide by frame count");
	//assert(! (GetHeight() % m_framesY) && "image Doesn't evenly divide by frame count");

	m_frameWidth = (float)(int(GetWidth()/framesX));
	m_frameHeight = (float)(int(GetHeight()/framesY));
}

void SurfaceAnim::SetupAnimBySize( int frameWid,int frameHei )
{
	assert(IsLoaded() && "Load your image before doing SetupAnim on it");

	m_framesX = int(GetWidth()/frameWid);
	m_framesY = int(GetWidth()/frameHei);

	m_frameWidth = (float)frameWid;
	m_frameHeight = (float)frameHei;
}

void SurfaceAnim::BlitAnim(float x, float y, int frameX, int frameY, unsigned int rgba, float rotationDegrees, CL_Vec2f vRotationPt)
{
	if (GetFrameWidth() == GetWidth() && GetFrameHeight() == GetHeight())
	{
		Blit(x,y, rgba, rotationDegrees, vRotationPt); //don't need the anim code
		return;
	}

	//first calculate the rect we need for the frame
	rtRectf src;
	src.left = m_frameWidth*frameX;
	src.top = m_frameHeight*frameY;
	src.right = src.left+m_frameWidth;
	src.bottom = src.top + m_frameHeight;

	//calculate the target
	rtRectf dst(x,y, x+m_frameWidth, y+m_frameHeight);
	BlitEx(dst, src, rgba, rotationDegrees, vRotationPt);
}

bool SurfaceAnim::LoadFileFromMemory( byte *pMem, int inputSize )
{
	if (!Surface::LoadFileFromMemory(pMem, inputSize)) return false;

	m_frameWidth = (float)GetWidth();
	m_frameHeight = (float)GetHeight();
	return true;
}


void SurfaceAnim::BlitScaledAnim( float x, float y, int frameX , int frameY, CL_Vec2f vScale, eAlignment alignment /*= ALIGNMENT_CENTER*/,
								 unsigned int rgba /*= MAKE_RGBA(255,255,255,255)*/, float rotation, CL_Vec2f vRotationPt, bool flipX, bool flipY, RenderBatcher *pBatcher, int padding)
{
	if (vScale.x == 0 && vScale.y == 0) return;

	if (GetFrameWidth() == GetWidth() && GetFrameHeight() == GetHeight() && !flipX && !flipY) 
	{
		BlitScaledWithRotatePoint(x,y, vScale, alignment, rgba, rotation, vRotationPt, pBatcher); //don't need the anim code
		return;
	}

	CL_Vec2f vStart = CL_Vec2f(x,y);
	rtRectf src;
	src.left = m_frameWidth*frameX + (float)padding;
	src.top = m_frameHeight*frameY + (float)padding;
	src.right = src.left+m_frameWidth - (float)(padding * 2);
	src.bottom = src.top + m_frameHeight - (float)(padding * 2);
	
	rtRectf dst(0,0, m_frameWidth, m_frameHeight);
	
	if (flipX)
	{
		swap(src.left, src.right);
	}

	if (flipY)
	{
		swap(src.top, src.bottom);
	}
	if (alignment != ALIGNMENT_UPPER_LEFT)
	{
		vStart -= GetAlignmentOffset(CL_Vec2f(GetFrameWidth(), GetFrameHeight()), alignment);
	}

	dst.AdjustPosition(vStart.x, vStart.y);
	dst.Scale(alignment, vScale);
	
	if (pBatcher && rotation == 0)
	{
		pBatcher->BlitEx(this, dst, src, rgba);
	} else
	{
		BlitEx(dst, src, rgba, rotation, vRotationPt);
	}
}

void SurfaceAnim::BlitArbitrarySection( float x, float y, CL_Rectf regionToDraw, CL_Vec2f vScale, eAlignment alignment /*= ALIGNMENT_CENTER*/,
								 unsigned int rgba /*= MAKE_RGBA(255,255,255,255)*/, bool flipX, bool flipY, RenderBatcher *pBatcher)
{
	if (vScale.x == 0 && vScale.y == 0) return;

	CL_Vec2f vStart = CL_Vec2f(x,y);
	rtRectf src;
	src.left = regionToDraw.left;
	src.top = regionToDraw.top;
	src.right = regionToDraw.right;
	src.bottom = regionToDraw.bottom;
	
	rtRectf dst(0,0, src.GetWidth(), src.GetHeight());
	
	if (flipX)
		swap(src.left, src.right);
	
	if (flipY)
		swap(src.top, src.bottom);
	
	if (alignment != ALIGNMENT_UPPER_LEFT)
	{
		vStart -= GetAlignmentOffset(CL_Vec2f(src.GetWidth(), src.GetHeight()), alignment);
	}

	dst.AdjustPosition(vStart.x, vStart.y);
	dst.Scale(alignment, vScale);
	
	if (pBatcher)
		pBatcher->BlitEx(this, dst, src, rgba);
	else
		BlitEx(dst, src, rgba);
}

void SurfaceAnim::ReloadImage()
{
	int framesX = m_framesX;
	int framesY = m_framesY;
	float frameWidth = m_frameWidth;
	float frameHeight = m_frameHeight;
	Surface::ReloadImage();

	m_framesX = framesX;
	m_framesY = framesY;
	m_frameWidth = frameWidth;
	m_frameHeight = frameHeight;
}

bool SurfaceAnim::InitBlankSurface( int x, int y )
{
	if (!Surface::InitBlankSurface(x,y)) return false;

	m_frameWidth = (float)GetWidth();
	m_frameHeight = (float)GetHeight();
	return true;
}

bool SurfaceAnim::InitFromSoftSurface( SoftSurface *pSurf )
{
	if (!Surface::InitFromSoftSurface(pSurf)) return false;

	m_frameWidth = (float)GetWidth();
	m_frameHeight = (float)GetHeight();
	return true;
}

void SurfaceAnim::BlitRotatedAnim( float x, float y, int frameX, int frameY, CL_Vec2f vScale, eAlignment alignment
								  /*= ALIGNMENT_CENTER*/, unsigned int rgba /*= MAKE_RGBA(255,255,255,255)*/, float rotation/*=0*/,
								  CL_Vec2f vRotationPt /*= CL_Vec2f(x,y)*/, bool flipX /*= false*/, bool flipY /*= false*/,
								  RenderBatcher *pBatcher)
{
	BlitScaledAnim(x,y,frameX,frameY, vScale, alignment, rgba, rotation, CL_Vec2f(x,y)+vRotationPt, flipX, flipY, pBatcher);
}

float SurfaceAnim::GetAspectRatio()
{
	if (m_frameHeight == 0)
	{
		assert(!"bad aspect ratio, image is blank, returning crap");
		return 1.0f;
	}

	return m_frameWidth/m_frameHeight;
}