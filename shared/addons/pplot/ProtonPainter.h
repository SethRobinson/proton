//  ***************************************************************
//  ProtonPainter - Creation date: 07/09/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ProtonPainter_h__
#define ProtonPainter_h__


#undef DrawText
#include "generic/PPlot.h"

class ProtonPainter: public Painter
{
public:
	ProtonPainter();
	virtual ~ProtonPainter();

	void SetSize(int x, int y);
	void SetPos( int x, int y );

	virtual void DrawLine (float inX1, float inY1, float inX2, float inY2);
	virtual void FillRect (int inX, int inY, int inW, int inH);
	virtual void InvertRect (int inX, int inY, int inW, int inH);;
	virtual void SetClipRect (int inX, int inY, int inW, int inH){};
	virtual long GetWidth () const {return m_sizeX;};
	virtual long GetHeight () const {return m_sizeY;};
	virtual void SetLineColor (int inR, int inG, int inB);
	virtual void SetFillColor (int inR, int inG, int inB);
	virtual long CalculateTextDrawSize (const char *inString);
	virtual long GetFontHeight () const;;
	virtual void DrawText (int inX, int inY, const char *inString);;
	virtual void DrawRotatedText (int inX, int inY, float inDegrees, const char *inString){};

protected:
	

private:

	unsigned int m_lineColor, m_fillColor;
	int m_sizeX;
	int m_sizeY;
	float m_posX;
	float m_posY;
	float m_fontScale;
	eFont m_fontID;
	uint32 m_fontColor;
};

#endif // ProtonPainter_h__