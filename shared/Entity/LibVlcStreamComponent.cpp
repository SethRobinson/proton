#include "PlatformPrecomp.h"

#include "Entity/OverlayRenderComponent.h"
#include "Entity/EntityUtils.h"
#include "Entity/TouchDragComponent.h"
#include "Entity/TouchDragMoveComponent.h"
#include "App.h"
#include "LibVlcStreamComponent.h"
#include "Entity/ScrollToZoomComponent.h"

LibVlcStreamComponent::LibVlcStreamComponent()
{
	SetName("LibVlcStream");
}

LibVlcStreamComponent::~LibVlcStreamComponent()
{
}

void LibVlcStreamComponent::OnAdd(Entity* pEnt)
{
	EntityComponent::OnAdd(pEnt);
	
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&LibVlcStreamComponent::OnUpdate, this, _1));
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	
	m_pLooping = &GetVarWithDefault("looping", (uint32)0)->GetUINT32();
	
	//every time looping gets changed, let's get notified
	GetVar("looping")->GetSigOnChanged()->connect(1, boost::bind(&LibVlcStreamComponent::OnLoopingChanged, this, _1));

	GetFunction("SetPause")->sig_function.connect(1, boost::bind(&LibVlcStreamComponent::OnSetPause, this, _1));
	GetFunction("SetPlaybackPosition")->sig_function.connect(1, boost::bind(&LibVlcStreamComponent::OnSetPlaybackPosition, this, _1));

}

void LibVlcStreamComponent::OnSetPause(VariantList* pList)
{
	//LogMsg("Looping changed to %d", pVariant->GetUINT32());
	//m_libVlcRTSP
	m_libVlcRTSP.SetPause(pList->Get(0).GetUINT32() != 0);
}

void LibVlcStreamComponent::OnSetPlaybackPosition(VariantList* pList)
{
	//LogMsg("Looping changed to %d", pVariant->GetUINT32());
	//m_libVlcRTSP
	m_libVlcRTSP.SetPlaybackPosition(pList->Get(0).GetFloat());
}


void LibVlcStreamComponent::OnLoopingChanged(Variant* pVariant)
{
	//LogMsg("Looping changed to %d", pVariant->GetUINT32());
	m_libVlcRTSP.SetLooping(pVariant->GetUINT32() != 0);

}

void LibVlcStreamComponent::Init(std::string url, int cacheMS)
{

	m_url = url;
	m_cacheMS = cacheMS;

	int width = 640;
	int height = 480;

	m_pSurface = new SurfaceAnim();

	m_pSurface->SetTextureType(Surface::TYPE_NO_SMOOTHING); //insure no mipmaps are created
	m_pSurface->InitBlankSurface(width, height);
	m_pSurface->FillColor(glColorBytes(255, 255, 0, 255));
	m_pSurface->SetSmoothing(false);

	//m_pSurface->UpdateSurfaceRect(rtRect(0, 0, width, height), s.GetPixelData());

	OverlayRenderComponent* pOverlay = (OverlayRenderComponent*)GetParent()->AddComponent(new OverlayRenderComponent());
	pOverlay->SetSurface(m_pSurface, true);
	GetParent()->GetParent()->MoveEntityToTopByAddress(GetParent());
	//EntitySetScaleBySize(GetParent(), CL_Vec2f(width / 2.0f, height / 2.0f));
	SetScale2DEntity(GetParent(), CL_Vec2f(1, 1));
	AnimateEntitySetMirrorMode(GetParent(), false, true);
	//pOverlay->GetVar("borderPaddingPixels")->Set(CL_Rectf(150, 20, 150, 22));

	EntityComponent* pDragComp = GetParent()->AddComponent(new TouchDragComponent);
	EntityComponent* pDragMoveComp = GetParent()->AddComponent(new TouchDragMoveComponent);
	ScrollToZoomComponent* pScrollZoomComp = (ScrollToZoomComponent*)GetParent()->AddComponent(new ScrollToZoomComponent);
	pDragComp->GetVar("limitedToThisFingerID")->Set(uint32(0)); //only allow left mouse button

	//listen in if a key is tapped while dragging this window around
	pScrollZoomComp->m_sig_input_while_mousedown.connect(1, boost::bind(&LibVlcStreamComponent::OnInputWhileMouseDown, this, _1));

	if (m_pSurface)
	{
		//init the libVLC_RTSP object
		if (!m_libVlcRTSP.Init(url, cacheMS, m_pSurface, this, width, height))
		{
			//show a windows text box with an error
			MessageBox(NULL, "Can't init libVLC stream.", "Error", MB_OK);
		}
		else
		{
			m_libVlcRTSP.SetLooping(*m_pLooping != 0);
		}

	}
}

void LibVlcStreamComponent::UpdateStatusMessage(string msg)
{

	int timeMS = 1000;

	Entity* pOldEnt = GetParent()->GetEntityByName("DebugText");
	if (pOldEnt)
	{
		pOldEnt->SetName("");
		pOldEnt->SetTaggedForDeletion();
	}

	Entity* pEnt = CreateTextLabelEntity(GetParent(), "DebugText", 0, 0, msg);
	SetupTextEntity(pEnt, FONT_LARGE, 0.66f);
	FadeOutAndKillEntity(pEnt, true, 100, timeMS);
}

void LibVlcStreamComponent::ShowVolume()
{
	UpdateStatusMessage("Vol: " + toString((int) (m_libVlcRTSP.GetVolume()*100)) + "%");
}

void LibVlcStreamComponent::SetMute(bool bMute)
{

	//toggle looping
	if (bMute)
	{
		//mute
		m_savedVolume = m_libVlcRTSP.GetVolume();
		m_libVlcRTSP.SetVolume(0);
		UpdateStatusMessage("Muted");
	}
	else
	{
		//unmute
		m_libVlcRTSP.SetVolume(m_savedVolume);
		UpdateStatusMessage("Unmuted");
	}
	
	m_bMuted = bMute;

}

void LibVlcStreamComponent::OnInputWhileMouseDown(VariantList* pVList)
{
	switch (eMessageType(int(pVList->Get(0).GetFloat())))
	{
	
		case MESSAGE_TYPE_GUI_CHAR:
		{
			char letterPressed = (char)pVList->Get(2).GetUINT32();

	#ifdef _DEBUG
			//LogMsg("Got char: %c (%d)", letterPressed, pVList->Get(3).GetUINT32());
	#endif
			if (letterPressed == 'r')
			{
				LogMsg("Reconnecting with stream...");

				m_libVlcRTSP.Release();
				if (!m_libVlcRTSP.Init(m_url, m_cacheMS, m_pSurface, this,m_pSurface->GetWidth(), m_pSurface->GetHeight()))
				{
					//show a windows text box with an error
					MessageBox(NULL, "Can't init libVLC stream.", "Error", MB_OK);
				}
			}

			if (letterPressed == ' ')
			{
				m_libVlcRTSP.TogglePause();
			}


			if (letterPressed == 'l')
			{
				//toggle looping
				*m_pLooping = *m_pLooping == 0 ? 1 : 0;
				m_libVlcRTSP.SetLooping(*m_pLooping != 0);
			}

			//if m is hit, mute

			if (letterPressed == 'm')
			{
				SetMute(!m_bMuted);
			}

			//volume up and down 10% by hitting - and =
			if (letterPressed == '-')
			{
				//toggle looping
				m_libVlcRTSP.SetVolume(m_libVlcRTSP.GetVolume() - 0.1f);
				ShowVolume();

			}

			if (letterPressed == '=')
			{
				//toggle looping
				m_libVlcRTSP.SetVolume(m_libVlcRTSP.GetVolume() + 0.1f);
				ShowVolume();
			}

			/*
			//if delete is hit
			if (letterPressed == 46) //del key
			{
				LogMsg("Deleting...");
				//toggle looping
				m_libVlcRTSP.Release();
				GetParent()->SetTaggedForDeletion();
			}
			*/


		}
		break;
	}
}

void LibVlcStreamComponent::SetSurfaceSize(int width, int height)
{
	m_pSurface->InitBlankSurface(width, height);
	m_pSurface->FillColor(glColorBytes(255, 255, 0, 255));
	m_pSurface->SetSmoothing(false);
	//grab the OverlayARenderComponent and set the frame size
	
	OverlayRenderComponent* pOverlay = (OverlayRenderComponent*)GetParent()->GetComponentByName("OverlayRender");
	if (pOverlay)
	{
		pOverlay->UpdateFrameSizeVar();
	}
	
	SetSize2DEntity(GetParent(), CL_Vec2f((float)width, (float)height));

}

void LibVlcStreamComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void LibVlcStreamComponent::OnUpdate(VariantList* pVList)
{
	m_libVlcRTSP.Update();
}

LibVlcStreamComponent* GetStreamEntityByName(std::string name)
{
	Entity *pEnt = GetEntityRoot()->GetEntityByName(name);
	if (pEnt)
	{
		LibVlcStreamComponent *pVlcComp = (LibVlcStreamComponent*) pEnt->GetComponentByName("LibVlcStream");
		if (pVlcComp)
		{
			return pVlcComp;
		}
	}
	return NULL;
}


void SetStreamVolumeByName(std::string name, float volume)
{
	//set the volume, show error if can't find stream
    LibVlcStreamComponent* pVlcComp = GetStreamEntityByName(name);
	if (pVlcComp)
	{
		pVlcComp->GetLibVlcRTSP()->SetVolume(volume);
	}
	else
	{
		ShowTextMessageSimple("Can't find stream to set volume.", 50);
	}

}	

//write function for above
LibVlcStreamComponent* AddNewStream(std::string name, std::string url, int cacheMS, Entity* pGUIEnt, bool bIgnoreIfExists)
{
	LibVlcStreamComponent* pVlcComp = NULL;

	if (bIgnoreIfExists)
	{
		//if the stream already exists, return that instead
		pVlcComp = GetStreamEntityByName(name);
		if (pVlcComp) return pVlcComp;
	}

	Entity *pEnt = pGUIEnt->AddEntity(new Entity(name));
//	SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);
	pVlcComp = (LibVlcStreamComponent*) pEnt->AddComponent(new LibVlcStreamComponent());

	GetApp()->Update();
	//ShowTextMessageSimple("Initting libVLC stream...", 50);
	GetApp()->Draw();
	ForceVideoUpdate();

	pVlcComp->Init(url, cacheMS);
	return pVlcComp;
}
