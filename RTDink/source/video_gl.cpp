#include "PlatformPrecomp.h"
#include "video_gl.h"
#include "dink/misc_util.h"
#include <cassert>
#include "Renderer/SoftSurface.h"
#include "App.h"

LPDIRECTDRAWSURFACE lpDDSBack = NULL;      // DirectDraw back surface
SoftSurface g_palette;

extern LPDIRECTDRAWSURFACE lpDDSBackGround;

bool InitializeVideoSystem()
{
	assert(!lpDDSBack);
	lpDDSBack = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_PRIMARY_GL);
	
	g_palette.Init(8,8, SoftSurface::SURFACE_PALETTE_8BIT);
	g_palette.SetPaletteFromBMP("dink/tiles/palette.bmp", SoftSurface::COLOR_KEY_NONE);
	
	return true;
}

void KillVideoEngine()
{
	SAFE_DELETE(lpDDSBack);
}

void dderror(int hErr){};

IDirectDrawSurface * LoadBitmapIntoSurface(const char *pName, eTransparencyType trans, IDirectDrawSurface::eMode mode, byte *pMem, int pMemSize, bool bUseCheckerboardFix)
{

	IDirectDrawSurface * pSurf;
	pSurf = new IDirectDrawSurface;
	pSurf->m_mode = mode;
	pSurf->m_pSurf = new SoftSurface;
	//bUseCheckerboardFix = true;

	pSurf->m_pSurf->SetForceBlackAndWhiteOnBmpPalettes(true);
	
	if (pMem)
	{
#ifdef _DEBUG
		//LogMsg("loading DDRAW bmp from mem");
#endif
		//if this is set, ignore the filename
		pSurf->m_pSurf->LoadFileFromMemory(pMem, SoftSurface::eColorKeyType(trans), pMemSize, false, bUseCheckerboardFix);
	} else
	{
#ifdef _DEBUG
		//LogMsg("loading DDRAW bmp from file");
#endif
		pSurf->m_pSurf->LoadFile(pName, SoftSurface::eColorKeyType(trans), false, bUseCheckerboardFix);
	}
	
	//LogMsg("loaded bitmap");

	switch( mode)
	{
	case IDirectDrawSurface::MODE_SHADOW_GL:
	
	
		break;
	}
	
	return pSurf;
}

void GetSizeOfSurface(IDirectDrawSurface *pdds, int *pX, int *pY)
{
	
	if (pdds->m_pSurf)
	{
		*pX = pdds->m_pSurf->GetWidth();
		*pY = pdds->m_pSurf->GetHeight();
		return;
	} else
	{
		assert(!"Bad surface");
		return;
	}
}

IDirectDrawSurface * InitOffscreenSurface(int x, int y, IDirectDrawSurface::eMode mode, bool bHiColor, SoftSurface *pSurfaceToCopyFrom)
{
	IDirectDrawSurface *pdds;
	pdds = new IDirectDrawSurface;
	pdds->m_mode = mode;

	switch (mode)
	{
		case IDirectDrawSurface::MODE_NORMAL:
		case IDirectDrawSurface::MODE_SHADOW_GL:
			
	
			pdds->m_pSurf = new SoftSurface;
			if (bHiColor)
			{
				pdds->m_pSurf->Init(x,y, SoftSurface::SURFACE_RGBA);
				pdds->m_pSurf->SetHasPremultipliedAlpha(true);
				pdds->m_pSurf->SetUsesAlpha(true);
			} else
			{
				pdds->m_pSurf->Init(x,y, SoftSurface::SURFACE_PALETTE_8BIT);
				pdds->m_pSurf->SetPaletteFromBMP("dink/tiles/palette.bmp", SoftSurface::COLOR_KEY_NONE);
			}
			
			if (pSurfaceToCopyFrom)
			{
				pdds->m_pSurf->Blit(0,0,pSurfaceToCopyFrom);

			}
			if (mode == IDirectDrawSurface::MODE_SHADOW_GL)
			{
				pdds->m_pGLSurf = new Surface;
				
				pdds->m_pGLSurf->SetSmoothing(GetApp()->GetVar("smoothing")->GetUINT32() != 0);
				if (GetApp()->GetVar("smoothing")->GetUINT32())
				{
					pdds->m_pGLSurf->SetTextureType(Surface::TYPE_GUI);

				} else
				{
					pdds->m_pGLSurf->SetTextureType(Surface::TYPE_NO_SMOOTHING);
				}

				pdds->m_pGLSurf->InitBlankSurface(x,y);
			}
			
			break;

		case IDirectDrawSurface::MODE_PRIMARY_GL:

		break;
	}

	return pdds;
}


IDirectDrawSurface::IDirectDrawSurface()
{
	m_pSurf = NULL;
	m_mode = MODE_NORMAL;
	m_pGLSurf = NULL;
	m_gameTickOfLastUse = 0;
}

IDirectDrawSurface::~IDirectDrawSurface()
{
	SAFE_DELETE(m_pSurf);
	SAFE_DELETE(m_pGLSurf);
}

glColorBytes RGBA_TO_GLCOLOR(const unsigned int color)
{
	return glColorBytes(GET_RED(color), GET_GREEN(color), GET_BLUE(color), GET_ALPHA(color));
}

unsigned int GLCOLOR_TO_RGBA(const glColorBytes glColor)
{
	return MAKE_RGBA(glColor.r, glColor.g, glColor.b, glColor.a);
}

Surface * IDirectDrawSurface::GetGLSuface()
{
	assert(m_mode == MODE_SHADOW_GL && "We're only using this for item icons, and they always would be gl shadowed!");

	UpdateShadowSurface();
	return m_pGLSurf;
}

int IDirectDrawSurface::Blt( rtRect32 *pDestRect, IDirectDrawSurface * pSrcSurf, rtRect32 *pSrcRect, uint32 flags, DDBLTFX *pFX )
{
	if (pSrcSurf)
	{
		pSrcSurf->UpdateLastUsedTime();
	}

	switch (m_mode)
	{
	
	case MODE_SHADOW_GL:
	case MODE_NORMAL:
		if (flags & DDBLT_COLORFILL)
		{
			assert(pFX);
			assert(pDestRect == NULL && "Well, we only support modifying the entire screen");
			
			//don't ask me why, but the original directx had these backwards. Palette is correct
			if (pFX->dwFillColor == 0)
			{
				pFX->dwFillColor = 255;
			}
			else if (pFX->dwFillColor == 255)
			{
				pFX->dwFillColor = 0;
			}

			if (m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGB || m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA)
			{
				m_pSurf->FillColor(g_palette.GetPalette()[pFX->dwFillColor]);
			}
			else
			{
				//sort of a hack for 8 bit index passing
				glColorBytes palIndex(pFX->dwFillColor, 0, 0, 255);
				m_pSurf->FillColor(palIndex);

			}
			

			return DD_OK;
		}
		if (pSrcSurf && pSrcSurf->m_pSurf && pSrcSurf->m_pSurf->GetSurfaceType() != SoftSurface::SURFACE_NONE)
		{
			//assert(!"Dumbass alert");
			m_pSurf->Blit(pDestRect->left, pDestRect->top, pSrcSurf->m_pSurf, pSrcRect->left, pSrcRect->top, pSrcRect->right-pSrcRect->left, pSrcRect->bottom-pSrcRect->top);
		}
		
	break;

	case MODE_PRIMARY_GL:
		{
			if (flags & DDBLT_COLORFILL)
			{
	
				assert(pFX);
				DrawRect(*pDestRect, GLCOLOR_TO_RGBA(g_palette.GetPalette()[pFX->dwFillColor]));
				return DD_OK;
			}

			if (!pSrcSurf)
			{
				assert(!"huh?!");
				return DD_OK;
			}
			if (pSrcSurf->m_mode == MODE_SHADOW_GL)
			{
				//blit from a GL surface instead
				pSrcSurf->UpdateShadowSurface();
				pSrcSurf->m_pGLSurf->BlitEx(rtRectf(*pDestRect) + rtRectf(0,0, 0.5f, 0.5f), rtRectf(*pSrcRect));
				break;
			}
			
			SoftSurface s;
			s.Init(pSrcRect->GetWidth(), pSrcRect->GetHeight(), SoftSurface::SURFACE_RGBA);
			
			//s.FillColor(glColorBytes(0,0,0,0));

			s.Blit(0,0, pSrcSurf->m_pSurf, pSrcRect->left,pSrcRect->top, pSrcRect->GetWidth(), pSrcRect->GetHeight());
				if (pSrcSurf->m_pSurf->GetUsesAlpha())
				{
					s.SetUsesAlpha(true);
				}
				
				g_globalBatcher.BlitRawImage(pDestRect->left,pDestRect->top, s);
		}

		break;
	
	}
	return DD_OK;
}
 
void IDirectDrawSurface::UpdateShadowSurface()
{
	assert(m_mode == MODE_SHADOW_GL && "Don't call this on other modes!");
	assert(m_pSurf);

	if (m_pSurf->GetModified())
	{
		//init the surface if needed
		if (!m_pGLSurf)
		{
			m_pGLSurf = new Surface;
			
			m_pGLSurf->SetSmoothing(GetApp()->GetVar("smoothing")->GetUINT32() != 0);

			if (GetApp()->GetVar("smoothing")->GetUINT32())
			{
				m_pGLSurf->SetTextureType(Surface::TYPE_GUI);			
			} else
			{
				m_pGLSurf->SetTextureType(Surface::TYPE_NO_SMOOTHING);
			}


			m_pGLSurf->InitBlankSurface(m_pSurf->GetWidth(),m_pSurf->GetHeight());
		}


		if (m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT)
		{
			
			
			SoftSurface s;
			s.Init(m_pSurf->GetWidth(), m_pSurf->GetHeight(), SoftSurface::SURFACE_RGBA);
			s.FillColor(glColorBytes(0,0,0,0));
			s.Blit(0,0, m_pSurf);
			s.FlipY();
		
			
			
			//put it on the GL surface
			m_pGLSurf->UpdateSurfaceRect(rtRect(0,0, s.GetWidth(), s.GetHeight()), s.GetPixelData());
			m_pGLSurf->SetUsesAlpha(m_pSurf->GetUsesAlpha());
	
			if (s.GetHasPremultipliedAlpha()) 
			{
				m_pGLSurf->SetBlendingMode(Surface::BLENDING_PREMULTIPLIED_ALPHA);
			}


		//	if (m_pSurf) m_pSurf->SetModified(false); //WARNING: Seth changed on 8/21/2017, seems like this fixes issue with constantly re-initting surfaces


			//		SAFE_FREE(m_pSurf);
		} 
	else
		{

			assert(m_pSurf);
			assert(m_pGLSurf);
			assert(m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA);
			
			if (m_pSurf->GetSurfaceType() != SoftSurface::SURFACE_RGBA)
			{
				LogMsg("Don't know how to deal with surface type %d", m_pSurf->GetSurfaceType());
			}
			//m_pSurf->FillColor(glColorBytes(0,0,0,0));
			if (m_pSurf->GetPixelData())
			{
				m_pSurf->FlipY();
				m_pGLSurf->UpdateSurfaceRect(rtRect(0,0, m_pSurf->GetWidth(), m_pSurf->GetHeight()), m_pSurf->GetPixelData(), true);
				m_pSurf->FlipY();
				m_pGLSurf->SetUsesAlpha(m_pSurf->GetUsesAlpha());
			
				if (m_pSurf->GetHasPremultipliedAlpha()) 
				{
					m_pGLSurf->SetBlendingMode(Surface::BLENDING_PREMULTIPLIED_ALPHA);
				}

			}

		}

		if (m_pSurf) m_pSurf->SetModified(false);
		
		//assert(m_pGLSurf->GetBlendingMode() == Surface::BLENDING_PREMULTIPLIED_ALPHA);
	}
}

int IDirectDrawSurface::BltFast( int x, int y, IDirectDrawSurface *pSrcSurf, rtRect32 *pSrcRect, uint32 dwTrans )
{

	if (pSrcSurf)
	{
		pSrcSurf->UpdateLastUsedTime();
	}

	switch (m_mode)
	{
	case MODE_SHADOW_GL:
	case MODE_NORMAL:
		
		
		if (pSrcSurf->m_mode == MODE_PRIMARY_GL)
		{
			//we need to copy from what is already on the screen
			m_pSurf->BlitFromScreen(x, y, pSrcRect->left, pSrcRect->top, pSrcRect->GetWidth(), pSrcRect->GetHeight());
			//m_pSurf->Blit(x, y, lpDDSBackGround->m_pSurf, pSrcRect->left, pSrcRect->top, pSrcRect->GetWidth(), pSrcRect->GetHeight());
			//m_pSurf->SetUsesAlpha(false);
		} else
		{
			m_pSurf->Blit(x, y, pSrcSurf->m_pSurf, pSrcRect->left, pSrcRect->top, pSrcRect->GetWidth(), pSrcRect->GetHeight());
		}
		break;

	case MODE_PRIMARY_GL:
		
		if (!pSrcSurf) 
		{
			assert(!"Shit!");
			return DD_OK;
		}
 		
		if (pSrcSurf->m_mode == MODE_SHADOW_GL)
		{
			//blit from a GL surface instead
			pSrcSurf->UpdateShadowSurface();

			//skip if too big for surface
			if (
				(pSrcRect->GetHeight()+y > C_DINK_SCREENSIZE_Y) || 
				(pSrcRect->GetWidth()+x > C_DINK_SCREENSIZE_X)
				)
			{
#ifdef _DEBUG
				//LogMsg("Skipping blit, original Dink 1.08 would have rejected it for not fitting");
#endif
				break;

			}

			//pSrcSurf->m_pGLSurf->SetBlendingMode(Surface::BLENDING_NORMAL);
			pSrcSurf->m_pGLSurf->BlitEx(rtRectf(x, y, x+pSrcRect->GetWidth(), y +pSrcRect->GetHeight())+ rtRectf(0,0, 0.5f, 0.5f), rtRectf(*pSrcRect));
			break;
		}
		
		SoftSurface s;
		s.Init(pSrcRect->GetWidth(), pSrcRect->GetHeight(), SoftSurface::SURFACE_RGBA);
		s.Blit(0,0, pSrcSurf->m_pSurf, pSrcRect->left,pSrcRect->top, pSrcRect->GetWidth(), pSrcRect->GetHeight());
		
		if (pSrcSurf->m_pSurf->GetUsesAlpha())
		{
			s.SetUsesAlpha(true);
		}
		g_globalBatcher.BlitRawImage(x,y, s);
		break;

	}
	return DD_OK;
}

void IDirectDrawSurface::UpdateLastUsedTime()
{
	m_gameTickOfLastUse = GetBaseApp()->GetGameTick();
}