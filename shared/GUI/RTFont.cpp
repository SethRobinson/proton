#include "PlatformPrecomp.h"

#include "RTFont.h"
#include "BaseApp.h"
#include "util/utf8.h"

RTFont::RTFont()
{
	InitDefaults();
}

void RTFont::InitDefaults()
{
	m_hasSpaceChar = false;
	m_yOffset = 0.0f;
}

RTFont::RTFont( string fileName )
{
	InitDefaults();
	Load(fileName);
}

RTFont::~RTFont()
{
}

void RTFont::ReloadFontTextureOnly()
{
	if (m_fileName.empty()) return;

	if (m_surf.IsLoaded())
	{
#ifdef _DEBUG
		LogMsg("font %s already reloaded, ignoring", m_fileName.c_str());
#endif
return;
	}
#ifdef _DEBUG
	LogMsg("Reloading font %s", m_fileName.c_str());
#endif

	FileInstance f(m_fileName);
	if (!f.IsLoaded()) return;
	rtfont_header *pHeader = (rtfont_header*)f.GetAsBytes();
	
	//skip pas the stuff we don't care about, we're just trying to get to the bitmap image itself
	int charCount =  pHeader->lastChar - pHeader->firstChar;
	rtfont_charData *pSrcChar = (rtfont_charData*) (f.GetAsBytes()+sizeof(rtfont_header) );
	pSrcChar += charCount;
	uint8 *pSrcBytes = (uint8*)pSrcChar;
	pSrcBytes += sizeof(KerningPair)* pHeader->kerningPairCount;
	pSrcBytes += 8*m_header.fontStateCount;

	if (!m_surf.LoadFileFromMemory(pSrcBytes))
	{
		return;
	}
}

bool RTFont::Load( string fileName, bool bAddBasePath)
{
	m_kerningMap.clear();
	m_chars.clear();
	m_fontStates.clear();

	FileInstance f(fileName, bAddBasePath);
	if (!f.IsLoaded())
	{
		LogMsg("Unable to load font %s", fileName.c_str());
		return false;
	}

	rtfont_header *pHeader = (rtfont_header*)f.GetAsBytes();
	if (strncmp((char*)pHeader->rtFileHeader.fileTypeID, C_RTFILE_FONT_HEADER, 6) != 0)
	{
		LogError("%s fileName doesn't appear to be a valid font file", fileName.c_str());
		return false;
	}

	m_fileName = fileName; //remember this for later

	memcpy(&m_header, pHeader, sizeof(rtfont_header));
	int charCount =  pHeader->lastChar - pHeader->firstChar;
	m_hasSpaceChar = pHeader->firstChar <= ' ' && pHeader->lastChar > ' ';
	//LogMsg("Size of font header: %d, sizeof char header: %d", sizeof(rtfont_header), sizeof(rtfont_charData));
	m_chars.reserve(charCount);
	
	rtfont_charData *pSrcChar = (rtfont_charData*) (f.GetAsBytes()+sizeof(rtfont_header) );

	FontChar fchar;
	
	for (int i=0; i < charCount; i++)
	{
		memcpy(&fchar.data, pSrcChar, sizeof(rtfont_charData));
		m_chars.push_back(fchar);
		pSrcChar += 1;
	}

	uint8 *pSrcBytes = (uint8*)pSrcChar;

	KerningPair k;
	for (int i=0; i < pHeader->kerningPairCount; i++)
	{
		memcpy(&k, pSrcBytes, sizeof(KerningPair));
		SetKerningData(k.first, k.second, k.amount);
		
		pSrcBytes += sizeof(KerningPair);
	}

	//now load the font state items (if defined), to let `2 make the text green and stuff
	if (m_header.fontStateCount > 0)
	{
		for (int i=0; i < m_header.fontStateCount; i++)
		{
			FontState fntState(  pSrcBytes[4],  *(unsigned int*)pSrcBytes);
			m_fontStates.push_back(fntState);
			pSrcBytes += 8;
		}
	} else
	{
		m_fontStates.push_back(FontState('0', MAKE_RGB(255, 255, 255)));
	}

	//now load the actual bmp, which this pointer should be sitting at
	m_surf.SetTextureType(Surface::TYPE_GUI);

	if (!m_surf.LoadFileFromMemory(pSrcBytes))
	{
		return false;
	}
	
	SetSmoothing(GetDefaultSmoothing());

	GetBaseApp()->m_sig_unloadSurfaces.connect(1, boost::bind(&RTFont::OnUnloadSurfaces, this));
	GetBaseApp()->m_sig_loadSurfaces.connect(1, boost::bind(&RTFont::OnLoadSurfaces, this));

	
	return true;
}


void RTFont::Draw( float x, float y, string text, unsigned int color )
{
	DrawScaled(x,y,text,1.0f, color);
}

bool RTFont::IsFontCode(const char *pText, FontStateStack *pState)
{
	if (pText[0] == '`')
	{
		
		if (pText[1] == 0)
		{
			return true; //malformed font code, remove this line if you want to be able to print ` codes
		}
		//it's a formatting command that is coming
		if (pText[1] == '`') 
		{
			if (pState->size() > 1) pState->pop_front();
			return true;
		}
		
		for (unsigned int i=0; i < m_fontStates.size(); i++)
		{
			if (pText[1] == m_fontStates[i].m_triggerChar)
			{
				pState->push_front(m_fontStates[i]);
				return true;
			}
		}
	}

	return false;
}

void RTFont::MeasureText( rtRectf *pRectOut, const string &text, float scale /*= 1.0f*/ )
{
	MeasureText(pRectOut, &text[0], (int)text.length(), scale);
}

CL_Vec2f RTFont::MeasureText( const string &text, float scale /*= 1.0f*/ )
{
	//TODO: Switch to using CL_Rectf
	rtRectf r;
	MeasureText(&r, &text[0], (int)text.length(), scale);
	return CL_Vec2f(r.GetWidth(), r.GetHeight());
}

void RTFont::MeasureText( rtRectf *pRectOut, const char *pText, int len, float scale /*= 1.0f*/ )
{

	rtRectf dst(0,0,0,0);
	rtfont_charData *pCharData;
	uint16 curChar, lastChar;
	uint8 seqLen;
	FontStateStack state;

	if (!IsLoaded())
	{
		*pRectOut = dst;
		LogMsg("Error: Font not loaded!");
		return;
	}

	int lines = 0;
	float curX = 0;

	lastChar = 0;
	pCharData = NULL;

	for (int i=0; i < len; i++)
	{
		seqLen = utf8::internal::sequence_length<const char*>(&pText[i]);
		if (seqLen > 2)
		{
			//we do not support these characters, because firstChar and lastChar are shorts
			i += seqLen - 1;
			continue;
		}

		if (seqLen > 1)
		{
			try {
				utf8::utf8to16<uint16*, const char*>(&pText[i], &pText[i + seqLen], &curChar);
			}
			catch (...) {
#ifdef _DEBUG
				//LogError("Invalid UTF-16 character?!");
#endif

				i += seqLen - 1;
				continue;
			}
			i += seqLen - 1;
		}
		else curChar = pText[i];
		
		//OPTIMIZE: We don't really need IsFontCode and to calculate states to simply measure things.. unless later we handle font
		//changes..
		if (IsFontCode(&pText[i], &state))
		{
			if (pText[i+1] != 0) i++; //also advance past the color control code
			continue;
		}

		if (curChar == '\n')
		{
			lines++;
			dst.right = rt_max(dst.right, curX);
			curX = 0;
			lastChar = 0;
			continue;
		}
		if (!m_hasSpaceChar && curChar == ' ')
		{
			curX += scale * m_header.blankCharWidth;
			lastChar = 0;
			continue;
		}
		if (curChar-m_header.firstChar >= m_chars.size())
		{
#ifdef _DEBUG
			//LogMsg("Char %c (%d) is not in our font", curChar, curChar);
#endif
			lastChar = 0;
			continue;
		}

		if (lastChar)
		{
			curX += (GetKerningData(lastChar, curChar)*scale);
		}

		lastChar = curChar;
		pCharData = &m_chars[curChar-m_header.firstChar].data;

			
		if (pCharData->xadvance != 0)
		{
			curX += float(pCharData->xadvance) * scale;

		} else
		{
			curX += float(pCharData->charSizeX) * scale;
		}
		
		float letterHeight = float(pCharData->charSizeY)*scale;
		float offsetY = (float(pCharData->charBmpOffsetY+m_yOffset)*scale);
		dst.bottom = rt_max(dst.bottom, (lines*GetLineHeight(scale)) +  dst.top + letterHeight + offsetY);
	}

	dst.right = rt_max(dst.right, curX);
	*pRectOut = dst;
}

void RTFont::DrawScaled( float x, float y, const string &text, float scale /*= 1.0f*/, unsigned int color/*=MAKE_RGBA(255,255,255,255)*/, FontStateStack *pState, RenderBatcher *pBatcher )
{
	if (!pBatcher) pBatcher = &g_globalBatcher;
	SetupOrtho();
	//assert(IsLoaded() && "No font loaded");
	
	
	if (!m_surf.IsLoaded())
	{
		ReloadFontTextureOnly();
	}
	
	rtRectf dst, src;
	rtfont_charData *pCharData;
	uint16 curChar, lastChar;
	uint8 seqLen;
	
	uint8 curAlpha = GET_ALPHA(color);
	//remove alpha from current color
	color -= curAlpha; //yes, you can do this, because alpha is at the first 8 bits
	float xStart = x;
	FontStateStack myState;
	if (!pState)
	{
		pState = &myState;
	}

	if (m_fontStates.empty())
	{
		//abort crash, no font is loaded
		return;
	}
	if (pState->empty())
	{
		if (color == MAKE_RGBA(255,255,255,0)) //alpha has been stripped, don't forget
		{
			pState->push_front(m_fontStates[0]);
		} else
		{
			pState->push_front(FontState('0', color));
		}
	}
	
	lastChar = 0;
	pCharData = NULL;

	for (unsigned int i=0; i < text.length(); i++)
	{
		seqLen = utf8::internal::sequence_length<const char*>(&text.c_str()[i]);
		if (seqLen > 2)
		{ 
			//we do not support these characters, because firstChar and lastChar are shorts
			i += seqLen - 1;
			continue;
		}

		if (seqLen > 1)
		{
			try {
				utf8::utf8to16<uint16*, const char*>(&text.c_str()[i], &text.c_str()[i + seqLen], &curChar);
			}
			catch (...) {
#ifdef _DEBUG
				//LogError("Invalid UTF-16 character?!");
#endif

				i += seqLen - 1;
				continue;
			}
			i += seqLen - 1;
		}
		else curChar = text[i];
		 
		if (IsFontCode(&text.c_str()[i], pState))
		{
			if (text[i+1] != 0) i++; //also advance past the color control code
			continue;
		}

		if (curChar == '\n')
		{
			y += GetLineHeight(scale);
			x = xStart;
			lastChar = 0;
			continue;
		}
		if (!m_hasSpaceChar && curChar == ' ')
		{
			x += scale * m_header.blankCharWidth;
			lastChar = 0;
			continue;
		}
		
		if (curChar-m_header.firstChar >= m_chars.size())
		{
#ifdef _DEBUG
		//LogMsg("Char %c (%d) is not in our font", text[i], int(text[i]));
#endif

			lastChar = 0;
			continue;
		}
	
		if (lastChar)
		{
			x += (GetKerningData(lastChar, curChar)*scale);
		}

		lastChar = curChar;
		pCharData = &m_chars[curChar-m_header.firstChar].data;

		dst.left = x;
		dst.top = y;
		dst.right = dst.left + pCharData->charSizeX;
		dst.bottom = dst.top + pCharData->charSizeY;

		if (scale != 1)
		{
			//also scale the offset, as this is affected
			dst.Scale(ALIGNMENT_UPPER_LEFT, CL_Vec2f(scale, scale));
			
			dst.top += float(pCharData->charBmpOffsetY+m_yOffset)*scale;
			dst.bottom += float(pCharData->charBmpOffsetY+m_yOffset)*scale;
			dst.left += float(pCharData->charBmpOffsetX)*scale;
			dst.right += float(pCharData->charBmpOffsetX)*scale;

		} else
		{
			dst.top += pCharData->charBmpOffsetY+m_yOffset;
			dst.bottom += pCharData->charBmpOffsetY+m_yOffset;
			dst.left += float(pCharData->charBmpOffsetX);
			dst.right += float(pCharData->charBmpOffsetX);
		}

		src.left = pCharData->bmpPosX;
		src.top = pCharData->bmpPosY;
		src.right = src.left + pCharData->charSizeX;
		src.bottom = src.top + pCharData->charSizeY;

		if (pBatcher)
		{
			pBatcher->BlitEx(&m_surf, dst, src, pState->front().m_color + curAlpha);
		} else
		{
			g_globalBatcher.BlitEx(&m_surf, dst, src, pState->front().m_color + curAlpha);
		}

		//instead of using the batcher, here is another way, albeit slow
		//m_surf.BlitEx(dst, src, pState->front().m_color + curAlpha);

	
		//add some space between the letters, too
		
		if (pCharData->xadvance != 0)
		{
			x += float(pCharData->xadvance) * scale;

		} else
		{
			x += float(pCharData->charSizeX) * scale;
		}
	}

	if (!pBatcher)
	{
		g_globalBatcher.Flush();
	}

}

void RTFont::DrawScaledSolidColor( float x, float y, const string &text, float scale /*= 1.0f*/, unsigned int color/*=MAKE_RGBA(255,255,255,255)*/, FontStateStack *pState, RenderBatcher *pBatcher )
{
	if (!pBatcher) pBatcher = &g_globalBatcher;
	SetupOrtho();
	//assert(IsLoaded() && "No font loaded");
		
	if (!m_surf.IsLoaded())
	{
		ReloadFontTextureOnly();
	}
	
	rtRectf dst, src;
	rtfont_charData *pCharData;
	uint16 curChar, lastChar;
	uint8 seqLen;
	
	float xStart = x;
	FontStateStack myState;
	if (!pState)
	{
		pState = &myState;
	}

	if (m_fontStates.empty())
	{
		//abort crash, no font is loaded
		return;
	}
	if (pState->empty())
	{
		if (color == MAKE_RGBA(255,255,255,0)) //alpha has been stripped, don't forget
		{
			pState->push_front(m_fontStates[0]);
		} else
		{
			pState->push_front(FontState('0', color));
		}
	}
	
	lastChar = 0;
	pCharData = NULL;

	for (unsigned int i=0; i < text.length(); i++)
	{
		seqLen = utf8::internal::sequence_length<const char*>(&text.c_str()[i]);
		if (seqLen > 2)
		{
			//we do not support these characters, because firstChar and lastChar are shorts
			i += seqLen - 1;
			continue;
		}

		if (seqLen > 1)
		{
			try {
				utf8::utf8to16<uint16*, const char*>(&text.c_str()[i], &text.c_str()[i + seqLen], &curChar);
			}
			catch (...) {
#ifdef _DEBUG
				//LogError("Invalid UTF-16 character?!");
#endif

				i += seqLen - 1;
				continue;
			}
			i += seqLen - 1;
		}
		else curChar = text[i];
		
		if (IsFontCode(&text.c_str()[i], pState))
		{
			if (text[i+1] != 0) i++; //also advance past the color control code
			continue;
		}

		if (curChar == '\n')
		{
			y += GetLineHeight(scale);
			x = xStart;
			lastChar = 0;
			continue;
		}
		if (!m_hasSpaceChar && curChar == ' ')
		{
			x += scale * m_header.blankCharWidth;
			lastChar = 0;
			continue;
		}
		
		if (curChar-m_header.firstChar >= m_chars.size())
		{
			lastChar = 0;
			continue;
		}
	
		if (lastChar)
		{
			x += (GetKerningData(lastChar, curChar)*scale);
		}

		lastChar = curChar;
		pCharData = &m_chars[curChar-m_header.firstChar].data;

		dst.left = x;
		dst.top = y;
		dst.right = dst.left + pCharData->charSizeX;
		dst.bottom = dst.top + pCharData->charSizeY;

		if (scale != 1)
		{
			//also scale the offset, as this is affected
			dst.Scale(ALIGNMENT_UPPER_LEFT, CL_Vec2f(scale, scale));
			
			dst.top += float(pCharData->charBmpOffsetY+m_yOffset)*scale;
			dst.bottom += float(pCharData->charBmpOffsetY+m_yOffset)*scale;
			dst.left += float(pCharData->charBmpOffsetX)*scale;
			dst.right += float(pCharData->charBmpOffsetX)*scale;

		} else
		{
			dst.top += pCharData->charBmpOffsetY+m_yOffset;
			dst.bottom += pCharData->charBmpOffsetY+m_yOffset;
			dst.left += float(pCharData->charBmpOffsetX);
			dst.right += float(pCharData->charBmpOffsetX);
		}

		src.left = pCharData->bmpPosX;
		src.top = pCharData->bmpPosY;
		src.right = src.left + pCharData->charSizeX;
		src.bottom = src.top + pCharData->charSizeY;

		if (pBatcher)
		{
			pBatcher->BlitEx(&m_surf, dst, src, color);
		} else
		{
			g_globalBatcher.BlitEx(&m_surf, dst, src, color);
		}

		//instead of using the batcher, here is another way, albeit slow
		//m_surf.BlitEx(dst, src, pState->front().m_color + curAlpha);

	
		//add some space between the letters, too
		
		if (pCharData->xadvance != 0)
		{
			x += float(pCharData->xadvance) * scale;

		} else
		{
			x += float(pCharData->charSizeX) * scale;
		}
	}

	if (!pBatcher)
	{
		g_globalBatcher.Flush();
	}
}

void RTFont::DrawAligned( float x, float y, const string &text, eAlignment alignment/*= ALIGNMENT_UPPER_LEFT*/, float scale /*= 1.0f*/, unsigned int color/*=MAKE_RGBA(255,255,255,255)*/, FontStateStack *pState /*= NULL*/, RenderBatcher *pBatcher /*= NULL*/ )
{
	if (alignment != ALIGNMENT_UPPER_LEFT)
	{
		CL_Vec2f vSize = MeasureText(text, scale);
		CL_Vec2f vOffset = GetAlignmentOffset(vSize, alignment);
		x -= vOffset.x;
		y -= vOffset.y;
	}

	DrawScaled(x,y, text, scale, color, pState, pBatcher);
}


void RTFont::DrawAlignedSolidColor( float x, float y, const string &text, eAlignment alignment/*= ALIGNMENT_UPPER_LEFT*/, float scale /*= 1.0f*/, unsigned int color/*=MAKE_RGBA(255,255,255,255)*/, FontStateStack *pState /*= NULL*/, RenderBatcher *pBatcher /*= NULL*/ )
{
	if (alignment != ALIGNMENT_UPPER_LEFT)
	{
		CL_Vec2f vSize = MeasureText(text, scale);
		CL_Vec2f vOffset = GetAlignmentOffset(vSize, alignment);
		x -= vOffset.x;
		y -= vOffset.y;
	}

	DrawScaledSolidColor(x,y, text, scale, color, pState, pBatcher);
}


void RTFont::DrawAlignedBackground(float x, float y, const string& text, eAlignment alignment/*= ALIGNMENT_UPPER_LEFT*/, float scale /*= 1.0f*/, unsigned int color/*=MAKE_RGBA(255,255,255,255)*/, FontStateStack* pState /*= NULL*/, RenderBatcher* pBatcher /*= NULL*/)
{
	CL_Vec2f vSize = MeasureText(text, scale);
	CL_Vec2f vOffset = GetAlignmentOffset(vSize, alignment);
	x -= vOffset.x;
	y -= vOffset.y;
	
	rtRectf rt = rtRectf(x, y, x + vSize.x, y + vSize.y);
	g_globalBatcher.Flush();

	DrawFilledRect(rt, color);
}

#define MAKE_KERNING_KEY(first,second) uint32( (uint32(first)*65536)  + uint32(second))

void RTFont::SetKerningData( int first, int second, signed char data )
{

	//LogMsg("First: %d, second: %d is %u, data: %d", first, second, MAKE_KERNING_KEY(first, second), data);
	//check for duplicates..
	//assert(GetKerningData(first, second) == 0); //there ARE actually duplicates, Bitmap Font Generator bug? 

	m_kerningMap[MAKE_KERNING_KEY(first, second)] = data;
}

float RTFont::GetKerningData( int first, int second )
{
	map<unsigned int, signed char>::iterator itor = m_kerningMap.find(MAKE_KERNING_KEY(first, second));
	
	if (itor != m_kerningMap.end())
	{
		//LogMsg("Found %d", itor->second);
		return float(itor->second);
	}

	return 0;
}

void RTFont::SetSmoothing( bool bSmoothing )
{
	assert(m_surf.IsLoaded());

	m_surf.SetSmoothing(bSmoothing);
}

string RTFont::GetNextLine(const CL_Vec2f &textBounds, char **pCur, float scale, CL_Vec2f &vEnclosingSizeOut)
{
	//special case to cage a cr at the start
	if ( (*pCur)[0] == '\n')
	{
		(*pCur) += 1;
		return "";
	}

	rtRectf r(0,0,0,0);
	string text;
	int lastWrapPoint =0;

	while (1)
	{
		if ((*pCur)[text.length()] == 0)
		{
			//end of text, return what we have
			(*pCur) += text.length();

			return text;
		}

#ifdef _DEBUG 
		if ( (*pCur)[text.length()] == '\r')
		 {
			 //assert(!"Don't have backslash r's (hex 0d in your strings!");
		 }
#endif

		if ((*pCur)[text.length()] == '\n')
		{
			//hardcoded cr, force word wrap here
			(*pCur) += text.length()+1; //ignore the \n part
			return text;
		}
			
		text += (*pCur)[text.length()];
		
		if ((*pCur)[text.length()] == '`')
		{
			//special color code, skip to the next part
			text += (*pCur)[text.length()];
			continue;
		}
				
		MeasureText(&r, (*pCur), (int)text.length(), scale);
		
		if (r.GetWidth() > textBounds.x)
		{
			if (lastWrapPoint == 0)		
			{
				//roughly break here, maybe the word was too long to wrap
				text.erase(text.length()-1, 1);
			} else
			{
				//break at the last space we had marked
				text.erase(lastWrapPoint, text.length()-lastWrapPoint);
				(*pCur) += 1; //also get rid of the floating space
			}
			
			(*pCur) += text.length();
			return text;

		} else
		{
			if (vEnclosingSizeOut.x < r.GetWidth()) vEnclosingSizeOut.x = r.GetWidth();

			if ((*pCur)[text.length()] == ' ')
			{
				lastWrapPoint = (int)text.length(); 
				
			}
		}
		
	}
	
	assert(!"Error");
	return "";
	
}

void RTFont::MeasureTextAndAddByLinesIntoDeque(const CL_Vec2f &textBounds, const string &text, deque<string> * pLines, float scale, CL_Vec2f &vEnclosingSizeOut)
{
	vEnclosingSizeOut = CL_Vec2f(0,0);
	
	if (textBounds.x == 0)
	{
		LogError("MeasureTextAndAddByLinesIntoDeque: Can't word wrap with boundsX being 0!");
		return;
	}
	char *pCur = (char*)&text[0];
	int lineCount = 0;
	while (pCur[0])
	{
		if (pLines)
		{
			pLines->push_back(GetNextLine(textBounds, &pCur, scale, vEnclosingSizeOut));
		} else
		{
			GetNextLine(textBounds, &pCur, scale, vEnclosingSizeOut);
		}
		lineCount++;
	}

#ifdef _DEBUG
/*
	if (pLines)
{
	assert(lineCount == pLines->size());
}
*/
#endif
	vEnclosingSizeOut.y = float(lineCount)*GetLineHeight(scale);

}

float RTFont::GetLineHeight( float scale )
{
	return m_header.lineHeight*scale;
}


CL_Vec2f RTFont::DrawWrapped(rtRect &r, const string &txt, bool centerX, bool centerY, unsigned int color, float scale, bool bMeasureOnly, uint32 bgColor)
{
	deque<string> deq;
	CL_Vec2f enclosingRect;

 	MeasureTextAndAddByLinesIntoDeque(CL_Vec2f((float)r.GetWidth(), (float)r.GetHeight()), txt, &deq, scale, enclosingRect);

	if (bMeasureOnly)
	{
		return enclosingRect;
	}

	if (centerY)
	{
		r.top += (r.GetHeight()-(int)enclosingRect.y)/2;
	}

	//draw bg?

	if (GET_ALPHA(bgColor) != 0)
	{
		float borderSizeX = GetLineHeight(scale)/6;
		float offsetX = ((r.GetWidth()-(int)enclosingRect.x)/2)-borderSizeX;

		if (deq.size() == 1 && centerX)
		{
			//special code for when there is just one line and x centering
			DrawFilledRect(CL_Rectf((float)r.left + offsetX, (float)r.top, float(r.left)+offsetX+enclosingRect.x+borderSizeX*2, float(r.top)+enclosingRect.y), bgColor);

		} else
		{
			DrawFilledRect(CL_Rectf((float)r.left + offsetX+(r.GetWidth()-(int)enclosingRect.x)/2, (float)r.top, float(r.left)+enclosingRect.x+borderSizeX*2, float(r.top)+enclosingRect.y), bgColor);
		}
	}

	//draw it
	FontStateStack state;

	for (;deq.size();)
	{
		float offsetX = 0;

		if (centerX) 
		{
			rtRectf lineRect;

			MeasureText(&lineRect, deq.front(), scale);

			offsetX += (r.GetWidth()-(int)lineRect.right)/2;
		}
				
		DrawScaled((float)r.left+offsetX, (float)r.top, deq.front(), scale, color, &state, &g_globalBatcher);
		deq.pop_front();
		r.top += (int)GetLineHeight(scale);
	}

	//g_globalBatcher.Flush();

	return enclosingRect;

}


//doesn't really draw, but does update the color state
void RTFont::DrawScaledFakeToUpdateState( const string &text, unsigned int color, FontStateStack *pState)
{

	uint8 curAlpha = GET_ALPHA(color);
	//remove alpha from current color
	color -= curAlpha; //yes, you can do this, because alpha is at the first 8 bits
	FontStateStack myState;
	if (!pState)
	{
		pState = &myState;
	}

	if (pState->empty())
	{
		if (color == MAKE_RGBA(255,255,255,0)) //alpha has been stripped, don't forget
		{
			pState->push_front(m_fontStates[0]);
		} else
		{
			pState->push_front(FontState('0', color));
		}
	}

	for (unsigned int i=0; i < text.length(); i++)
	{
		if (IsFontCode(&text.c_str()[i], pState))
		{
			if (text[i+1] != 0) i++; //also advance past the color control code
			continue;
		}

	}
}

void RTFont::OnUnloadSurfaces()
{

}

void RTFont::OnLoadSurfaces()
{
	ReloadFontTextureOnly();
}

bool RTFont::IsValidChar(uint16 c, bool bStrict) 
{
	//check if we have this char in font
	if (c - m_header.firstChar >= m_chars.size()) return false;
	if (m_chars[c - m_header.firstChar].data.charSizeX < 1) return false;

	if (!bStrict)
	{
		return true;
	}

	if (
		(c >= 45 && c <= 46) ||
		(c >= 48 && c <= 57) ||
		(c >= 65 && c <= 90) ||
		(c >= 97 && c <= 122) ||
		(c >= 192 && c <= 214) ||
		(c >= 216 && c <= 246) ||
		(c >= 248 && c <= 255) ||
		(c >= 256 && c <= 383) ||
		(c == 402) ||
		(c >= 506 && c <= 511) ||
		(c == 902) ||
		(c >= 904 && c <= 906) ||
		(c == 908) ||
		(c >= 910 && c <= 929) ||
		(c >= 931 && c <= 974) ||
		(c >= 1025 && c <= 1036) ||
		(c >= 1038 && c <= 1103) ||
		(c >= 1105 && c <= 1116) ||
		(c >= 1118 && c <= 1119) ||
		(c >= 1168 && c <= 1169)) return true;

	return false;
}

string RTFont::FilterOutInvalidChars(const string& input, bool bStrict) {
	string output;

	uint8 seqLen = 0;
	uint16 curChar = 0, resChar = 0; //curChar is used to check char in font, resChar is actually added to output

	for (size_t i = 0; i < input.length(); i++)
	{
		seqLen = utf8::internal::sequence_length<const char*>(&input.c_str()[i]);
		if (seqLen > 2)
		{
			i += seqLen - 1;
			continue;
		}
		if (seqLen < 1) seqLen = 1;

		if (seqLen > 1)
		{
			resChar = *(uint16*)&input.c_str()[i];
			try {
				utf8::utf8to16<uint16*, const char*>(&input.c_str()[i], &input.c_str()[i + seqLen], &curChar);
			}
			catch (...) {
				i += seqLen - 1;
				continue;
			}
			i += seqLen - 1;
		}
		else {
			curChar = input[i];
			resChar = curChar;
		}

		if (IsValidChar(curChar, bStrict))
		{
			output.append((char*)&resChar, seqLen);
		}
	}

	return output;
}

unsigned int RTFont::GetColorFromString( const char *pText )
{
	if (pText[0] == '`')
	{

		if (pText[1] == 0)
		{
			LogError("RTFont::GetColorFromString> Bad code");
			return MAKE_RGBA(255,255,255,255); //malformed font code, remove this line if you want to be able to print ` codes
		}
		//it's a formatting command that is coming
	
		for (unsigned int i=0; i < m_fontStates.size(); i++)
		{
			if (pText[1] == m_fontStates[i].m_triggerChar)
			{
				return m_fontStates[i].m_color;
			}
		}
	}

	return MAKE_RGBA(255,255,255,255);
}

int RTFont::CountCharsThatFitX( float sizeX, const string &text, float scale /*= 1.0f*/ )
{
	rtfont_charData *pCharData;
	uint16 curChar, lastChar;
	uint8 seqLen;

	int lines = 0;
	float curX = 0;
	lastChar = 0;
	pCharData = NULL;
	int lastGood = 0;
	FontStateStack state;

	for (uint32 i=0; i < text.size(); i++)
	{
		if (curX >= sizeX)
		{
			return lastGood;
		}

		lastGood = i;

		seqLen = utf8::internal::sequence_length<const char*>(&text.c_str()[i]);
		if (seqLen > 2)
		{
			//we do not support these characters, because firstChar and lastChar are shorts
			i += seqLen - 1;
			continue;
		}

		if (seqLen > 1)
		{
			try {
				utf8::utf8to16<uint16*, const char*>(&text.c_str()[i], &text.c_str()[i + seqLen], &curChar);
			}
			catch (...) {
#ifdef _DEBUG
				//LogError("Invalid UTF-16 character?!");
#endif

				i += seqLen - 1;
				continue;
			}
			i += seqLen - 1;
		}
		else curChar = text[i];

		if (IsFontCode(&text[i], &state))
		{
			if (text[i+1] != 0) i++; //also advance past the color control code
			continue;
		}

		if (!m_hasSpaceChar && curChar == ' ')
		{
			curX += scale * m_header.blankCharWidth;
			continue;
		}
	
		if (curChar-m_header.firstChar >= m_chars.size())
		{
#ifdef _DEBUG
			//LogMsg("Char %c (%d) is not in our font", text[i], int(byte(text[i])));
#endif
			continue;
		}

		if (lastChar)
		{
			curX += (GetKerningData(uint8(text[i-1]), text[i])*scale);
		}

		lastChar = curChar;
		pCharData = &m_chars[curChar-m_header.firstChar].data;

		if (pCharData->xadvance != 0)
		{
			curX += float(pCharData->xadvance) * scale;
		} else
		{
			curX += float(pCharData->charSizeX) * scale;
		}
	}

	if (curX >= sizeX)
	{
		return lastGood;
	}
	return (int)text.size(); //they all fit
}
