#include "PlatformPrecomp.h"
#include "ProtonPainter.h"

ProtonPainter::ProtonPainter()
{
	m_posX = 0;
	m_posY = 0;
	m_sizeX = 100;
	m_sizeY = 100;
	m_lineColor = MAKE_RGBA(255,255,255,255);
	m_fillColor = MAKE_RGBA(0,0,255,255);
	m_fontID = FONT_SMALL;
	m_fontScale = 0.7f;
	m_fontColor = MAKE_RGBA(0,0,0,255);

}

ProtonPainter::~ProtonPainter()
{
	
}

void ProtonPainter::FillRect( int inX, int inY, int inW, int inH )
{
	DrawFilledRect(m_posX+(float)inX, m_posY+(float)inY, (float)inW, (float)inH, m_fillColor);
}

void ProtonPainter::DrawLine( float inX1, float inY1, float inX2, float inY2 )
{
	::DrawLine(m_lineColor, m_posX+(float)inX1, m_posY+(float)inY1, m_posX+(float) inX2, m_posY+(float)inY2,
		1.0f);
}

void ProtonPainter::SetSize( int x, int y )
{
	m_sizeX = x;
	m_sizeY = y;
}

void ProtonPainter::SetPos( int x, int y )
{
	m_posX = x;
	m_posY = y;
}

void ProtonPainter::SetLineColor( int inR, int inG, int inB )
{
	m_lineColor = MAKE_RGBA(inR, inG, inB, 255);
	m_fontColor =   MAKE_RGBA(inR, inG, inB, 255);
}

void ProtonPainter::SetFillColor( int inR, int inG, int inB )
{
	m_fillColor = MAKE_RGBA(inR, inG, inB, 255);
}

void ProtonPainter::DrawText( int inX, int inY, const char *inString )
{
	GetBaseApp()->GetFont(m_fontID)->DrawScaled(m_posX+(float)inX, m_posY+(float)inY, inString, m_fontScale, m_fontColor);
}

long ProtonPainter::GetFontHeight() const
{
	return (long)GetBaseApp()->GetFont(m_fontID)->GetLineHeight(m_fontScale)*0.8;
}

long ProtonPainter::CalculateTextDrawSize( const char *inString )
{
	return GetBaseApp()->GetFont(m_fontID)->MeasureText(inString, m_fontScale).x;
}

void ProtonPainter::InvertRect( int inX, int inY, int inW, int inH )
{
	DrawFilledRect(m_posX+(float)inX, m_posY+(float)inY, (float)inW, (float)inH, MAKE_RGBA(200,200,200,150));
}