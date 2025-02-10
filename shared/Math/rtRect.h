//  ***************************************************************
//  rtRect - Creation date: 03/26/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef rtRect_h__
#define rtRect_h__

#include "PlatformSetup.h"
#include "ClanLib-2.0/Sources/API/Core/Math/vec2.h"

//this alignment naming below suffers from "add as you go with no plan".  They aren't really clear.. -Seth

enum eAlignment
{
	ALIGNMENT_UPPER_LEFT,
	ALIGNMENT_CENTER,
	ALIGNMENT_DOWN_CENTER,
	ALIGNMENT_UPPER_RIGHT,
	ALIGNMENT_DOWN_LEFT,
	ALIGNMENT_UPPER_CENTER,
	ALIGNMENT_LEFT_CENTER,
	ALIGNMENT_DOWN_RIGHT,
	ALIGNMENT_RIGHT_CENTER,
};

enum eAspect
{
	ASPECT_NONE,
	ASPECT_WIDTH_CONTROLS_HEIGHT,
	ASPECT_HEIGHT_CONTROLS_WIDTH,
	ASPECT_FIT,
	
};

struct rtRect32;
struct rtRect;
struct rtRectf;

//forces 32 bit version, useful for some 64 bit builds that broke things.  Should probably template these but whatever.

struct rtRect32
{
	rtRect32();
	rtRect32(int32 l, int32 t, int32 r, int32 b) : left(l), top(t), right(r), bottom(b) {};
	rtRect32(rtRectf r);
	
	int32 left, top, right, bottom;
	void Clear() { left = top = right = bottom = 0; }
	int32 GetHeight() const { return bottom - top; }
	int32 GetWidth() const { return right - left; }
	void ScaleCentered(float f);
	void AdjustPosition(int32 x, int32 y); //move the rect by repositioning the upper left hand corner
	void Inflate(int32 x, int32 y);
	bool Intersects(rtRect32 r) const
	{
		if (left > r.right || r.left > right)
		{
			return false;
		}

		if (top > r.bottom || r.top > bottom)
		{
			return false;
		}

		return true;
	}
	bool Contains(rtRect32 r) const
	{
		// Check if all points of rectangle r are inside this rectangle
		if (r.left >= left && r.right <= right &&
			r.top >= top && r.bottom <= bottom)
		{
			return true;
		}
		return false;
	}

	bool operator==(const rtRect32& other) const
	{
		return left == other.left && top == other.top &&
			right == other.right && bottom == other.bottom;
	}
};


struct rtRect
{
	rtRect();
	rtRect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {};
	rtRect(rtRect32 r32) { left = r32.left; top = r32.top; right = r32.right; bottom = r32.bottom; };
	int left,top,right,bottom;
	void Clear() {left = top = right = bottom = 0;}
	int GetHeight() const {return bottom-top;}
	int GetWidth() const {return right-left;}
	void ScaleCentered(float f);
	void AdjustPosition(int x, int y); //move the rect by repositioning the upper left hand corner
	void Inflate(int x, int y);
	bool Intersects(rtRect r);
	CL_Vec2i GetCenter() const { return CL_Vec2i((left + right) / 2, (top + bottom) / 2); }

	bool Contains(rtRect r) const
	{
		// Check if all points of rectangle r are inside this rectangle
		if (r.left >= left && r.right <= right &&
			r.top >= top && r.bottom <= bottom)
		{
			return true;
		}
		return false;
	}

	bool operator==(const rtRect& other) const
	{
		return left == other.left && top == other.top &&
			right == other.right && bottom == other.bottom;
	}

};

struct rtRectf
{
	rtRectf();
	rtRectf(rtRect r){ left = (float)r.left; top = (float)r.top; right = (float)r.right; bottom =  (float)r.bottom; };
	rtRectf(rtRect32 r) { left = (float)r.left; top = (float)r.top; right = (float)r.right; bottom = (float)r.bottom; };

	rtRectf(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {};
	float left,top, right,bottom;
	void Clear() {left = top = right = bottom = 0;}
	float GetHeight() const {return bottom-top;}
	float GetWidth() const {return right-left;}
	void AdjustPosition(float x, float y); //move the rect by repositioning the upper left hand corner
	void Scale(eAlignment alignment,  CL_Vec2f vScale );

	rtRectf operator+ (const rtRectf &r) const;
	bool IsInside( float x, float y );
	bool Intersects(rtRectf r) const
	{
		if (left > r.right || r.left > right)
		{
			return false;
		}

		if (top > r.bottom || r.top > bottom)
		{
			return false;
		}

		return true;
	}
	bool Contains(rtRectf r) const
	{
		// Check if all points of rectangle r are inside this rectangle
		if (r.left >= left && r.right <= right &&
			r.top >= top && r.bottom <= bottom)
		{
			return true;
		}
		return false;
	}

	bool operator==(const rtRectf& other) const
	{
		return left == other.left && top == other.top &&
			right == other.right && bottom == other.bottom;
	}
};

#endif // rtRect_h__