//  ***************************************************************
//  RTFont - Creation date: 03/26/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef RTFont_h__
#define RTFont_h__

#include "RTFontFileFormat.h"
#include "../Renderer/Surface.h"

class RenderBatcher;

class RTFont
{
public:
	RTFont();
	RTFont(string fileName);

	virtual ~RTFont();
	void Draw(float x, float y, string text, unsigned int color=MAKE_RGBA(255,255,255,255));
	bool Load(string fileName, bool bAddBasePath = true);

	void MeasureText( rtRectf *pRectOut, const string &text, float scale = 1.0f);
	void MeasureText( rtRectf *pRectOut, const char *pText, int len, float scale /*= 1.0f*/ );
	CL_Vec2f MeasureText( const string &text, float scale = 1.0f);
	void MeasureTextAndAddByLinesIntoDeque(const CL_Vec2f &textBounds, const string &text, deque<string> *pLines, float scale, CL_Vec2f &vEnclosingSizeOut);
	int CountCharsThatFitX(float sizeX, const string &text, float scale = 1.0f);
	void DrawScaled( float x, float y, const string &text, float scale = 1.0f, unsigned int color=MAKE_RGBA(255,255,255,255),  FontStateStack *pState = NULL, RenderBatcher *pBatcher = NULL);
	void DrawScaledSolidColor( float x, float y, const string &text, float scale=1.0f, unsigned int color=MAKE_RGBA(255,255,255,255), FontStateStack *pState = NULL, RenderBatcher *pBatcher = NULL);
	CL_Vec2f DrawWrapped(rtRect &r, const string &txt, bool centerX=false, bool centerY=false, unsigned int color=MAKE_RGBA(255,255,255,255), float scale=1.0f, bool bMeasureOnly = false, uint32 bgColor = MAKE_RGBA(0,0,0,0));
	void DrawAligned(float x, float y, const string &text, eAlignment alignment = ALIGNMENT_UPPER_LEFT, float scale = 1.0f, unsigned int color=MAKE_RGBA(255,255,255,255), FontStateStack *pState = NULL, RenderBatcher *pBatcher = NULL);
	void DrawAlignedSolidColor(float x, float y, const string &text, eAlignment alignment = ALIGNMENT_UPPER_LEFT, float scale = 1.0f, unsigned int color=MAKE_RGBA(255,255,255,255), FontStateStack *pState = NULL, RenderBatcher *pBatcher = NULL);
	void SetSmoothing(bool bSmoothing); //false would disable linear texture filting
	float GetLineHeight(float scale = 1.0f);
	void DrawScaledFakeToUpdateState( const string &text, unsigned int color, FontStateStack *pState);
	bool IsLoaded() {return m_surf.IsLoaded();}

	unsigned int GetColorFromString(const char *pText);
	Surface * GetSurface() {return &m_surf;}

protected:
	
private:

	bool IsFontCode(const char *pText, FontStateStack *pState);
	void SetKerningData(int first, int second, signed char data);
	float GetKerningData(int first, int second);
	string GetNextLine(const CL_Vec2f &textBounds, char **pCur, float scale, CL_Vec2f &vEnclosingSizeOut);
	void ReloadFontTextureOnly();
	
	void OnUnloadSurfaces();
	void OnLoadSurfaces();
	void InitDefaults();
	rtfont_header m_header;
	vector<FontChar> m_chars;
	bool m_hasSpaceChar;
	Surface m_surf;
	vector<FontState> m_fontStates;
	float m_yOffset; //for some reason the font generator has sort of large vertical offsets, like 10, when it doesn't need them
	map<unsigned int, signed char> m_kerningMap; 
	string m_fileName;
};

#endif // RTFont_h__
