#include "PlatformPrecomp.h"
#include "SpriteSheetSurface.h"

SpriteSheetSurface::SpriteSheetSurface()
{
}

SpriteSheetSurface::~SpriteSheetSurface()
{
}

void SpriteSheetSurface::AddFrame(const std::string& frameName, const CL_Rect& frameRect)
{
	if (!frameName.empty()) {
		m_frames[frameName] = frameRect;
	}
}

CL_Vec2f SpriteSheetSurface::GetFrameSize(const std::string &frameName)
{
	CL_Vec2f ret;

	FrameDictConstIt it(m_frames.find(frameName));
	if (it != m_frames.end()) {
		CL_Sizef s(it->second.get_size());
		ret.x = s.width;
		ret.y = s.height;
	}

	return ret;
}

void SpriteSheetSurface::BlitFrame(float x, float y, const std::string& frameName, const CL_Vec2f& vScale, unsigned int rgba, float rotation, const CL_Vec2f& vRotationPt, bool flipX, bool flipY)
{
	FrameDictConstIt it(m_frames.find(frameName));
	if (it != m_frames.end()) {
		// first calculate the rect we need for the frame
		rtRectf src(it->second.left, it->second.top, it->second.right, it->second.bottom);

		if (flipX)
		{
			swap(src.left, src.right);
		}
		if (flipY)
		{
			swap(src.top, src.bottom);
		}

		// calculate the target
		rtRectf dst(x, y, x + it->second.get_width(), y + it->second.get_height());
		dst.Scale(ALIGNMENT_UPPER_LEFT, vScale);

		BlitEx(dst, src, rgba, rotation, vRotationPt);
	}
}

bool SpriteSheetSurface::InitBlankSurface(int x, int y)
{
	m_frames.clear();
	return Surface::InitBlankSurface(x,y);
}

bool SpriteSheetSurface::InitFromSoftSurface(SoftSurface *pSurf)
{
	m_frames.clear();
	return Surface::InitFromSoftSurface(pSurf);
}
