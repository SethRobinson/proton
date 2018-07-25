//  ***************************************************************
//  SpriteSheetSurface - Creation date: 04/02/2012
//  -------------------------------------------------------------
//
//  ***************************************************************
//  Programmer(s):  Aki Koskinen
//  ***************************************************************

#ifndef SpriteSheetSurface_h__
#define SpriteSheetSurface_h__

#include "Surface.h"

class SpriteSheetSurface : public Surface
{
public:
	SpriteSheetSurface();
	virtual ~SpriteSheetSurface();

	/**
	 * Adds a new named frame to this sprite sheet.
	 * If \a frameName is an empty string this method does nothing - a frame must have a name.
	 * If a frame with the given name already exists in the sprite sheet the previous frame
	 * is discarded and the new frame is taken into use instead.
	 */
	void AddFrame(const std::string& frameName, const CL_Rect& frameRect);
	void BlitFrame(float x, float y, const std::string& frameName, const CL_Vec2f& vScale = CL_Vec2f(1.0f, 1.0f), unsigned int rgba = PURE_WHITE, float rotation = .0f, const CL_Vec2f& vRotationPt = CL_Vec2f(.0f, .0f), bool flipX = false, bool flipY = false);

	/**
	 * Returns the size of the named frame. If no such frame exists returns 0,0.
	 */
	CL_Vec2f GetFrameSize(const std::string& frameName);

	virtual bool InitBlankSurface( int x, int y);
	virtual bool InitFromSoftSurface(SoftSurface *pSurf);

private:
	typedef std::map<std::string, CL_Rect> FrameDict;
	typedef FrameDict::const_iterator FrameDictConstIt;
	FrameDict m_frames;
	
};
#endif // SpriteSheetSurface_h__
