#include "PlatformPrecomp.h"

#include "Entity/OverlayRenderComponent.h"
#include "Entity/EntityUtils.h"
#include "Entity/TouchDragComponent.h"
#include "Entity/TouchDragMoveComponent.h"
#include "App.h"
#include "LibVlcStreamComponent.h"
#include "Entity/ScrollToZoomComponent.h"
#include "ProgressBarComponent.h"
#include "Renderer/SoftSurface.h"

LibVlcStreamComponent::LibVlcStreamComponent()
{
	SetName("LibVlcStream");
}

LibVlcStreamComponent::~LibVlcStreamComponent()
{
	m_libVlcRTSP.Release();
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
	bool bWantPaused = pList->Get(0).GetUINT32() != 0;
	//if restart-on-play is enabled and we're transitioning paused -> playing, rewind first
	if (!bWantPaused && m_bRestartOnPlay && m_libVlcRTSP.GetPause())
	{
		m_libVlcRTSP.SetPlaybackPosition(0.0f);
	}
	m_libVlcRTSP.SetPause(bWantPaused);

	//UpdateControlButtons();
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
	//UpdateControlButtons();
}

void LibVlcStreamComponent::ResetTimeOfLastTouch()
{
	m_timeOfLastTouchMS = GetTick();
}

void LibVlcStreamComponent::LoadStaticImage(string url)
{
	OverlayRenderComponent* pOverlay = (OverlayRenderComponent*)GetParent()->GetComponentByName("OverlayRender");

	pOverlay->SetSurface(NULL, false); //unload the surface and delete it

	//it's an image, let's load it
	//SAFE_DELETE(m_pSurface);

	m_pSurface = new SurfaceAnim();
	m_pSurface->LoadFile(url);
	if (!m_pSurface->IsLoaded())
	{
		LogMsg("Error loading %s", url.c_str());
		assert(!"Image is missing");
		return;
	}
	m_bUseStillPicMode = true;
	SetSurfaceSize(m_pSurface->GetWidth(), m_pSurface->GetHeight());

	SoftSurface ss;
	ss.LoadFile(url);
	m_pSurface->InitFromSoftSurface(&ss);
	
	if (pOverlay)
	{
		pOverlay->SetSurface(m_pSurface, true);
	}
	else
	{
		assert(!"Couldn't find overlay..");
	}
}
void LibVlcStreamComponent::Init(std::string url, int cacheMS, VLC_ExtraSettings settings)
{
	ResetTimeOfLastTouch();

	m_url = url;
	m_cacheMS = cacheMS;
	m_bStartPaused = settings.startPaused;
	
	//default to a size good for audio.  If it's video we load, it will auto-resize to the correct video size.
	//Compact dimensions for audio-only players - the bottom-anchored controls strip is 32px tall,
	//and the play button (32px) + 16px spacer + 100px volume slider + 16px spacer = 164px of fixed
	//horizontal width before the progress bar starts, so 240px width leaves ~76px for the bar.
	//Drag area above the controls = height - 32 = 23px (tight but enough to grab the panel).
	int width = 240;
	int height = 55;

	m_pSurface = new SurfaceAnim();

	m_pSurface->SetTextureType(Surface::TYPE_NO_SMOOTHING); //insure no mipmaps are created
	m_pSurface->InitBlankSurface(width, height);
	m_pSurface->SetSmoothing(false);

	OverlayRenderComponent* pOverlay = (OverlayRenderComponent*)GetParent()->AddComponent(new OverlayRenderComponent());
	pOverlay->SetSurface(m_pSurface, true);
	
	GetParent()->GetParent()->MoveEntityToTopByAddress(GetParent());
	SetScale2DEntity(GetParent(), CL_Vec2f(1, 1));
	AnimateEntitySetMirrorMode(GetParent(), false, true);
	//pOverlay->GetVar("borderPaddingPixels")->Set(CL_Rectf(150, 20, 150, 22));

	EntityComponent* pDragComp = GetParent()->AddComponent(new TouchDragComponent);
	EntityComponent* pDragMoveComp = GetParent()->AddComponent(new TouchDragMoveComponent);

	//you know, it would be useful if we knew when someone started dragging this window around, so let's connect to those signals
	pDragComp->GetFunction("OnTouchDragUpdate")->sig_function.connect(1, boost::bind(&LibVlcStreamComponent::OnTouchDragUpdate, this, _1));
	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&LibVlcStreamComponent::OnTouchDragUpdate, this, _1));
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&LibVlcStreamComponent::OnTouchDragUpdate, this, _1));
	
	ScrollToZoomComponent* pScrollZoomComp = (ScrollToZoomComponent*)GetParent()->AddComponent(new ScrollToZoomComponent);
	pDragComp->GetVar("limitedToThisFingerID")->Set(uint32(0)); //only allow left mouse button

	//listen in if a key is tapped while dragging this window around
	pScrollZoomComp->m_sig_input_while_mousedown.connect(1, boost::bind(&LibVlcStreamComponent::OnInputWhileMouseDown, this, _1));

	UpdateProgressBar();

	if (m_pSurface)
	{

		//if m_url ends with .png or .jpg (case insensitive) we'll do something
		string ext = ToLowerCaseString(GetFileExtension(url));
		if (ext == "png" || ext == "jpg")
		{

			LoadStaticImage(url);
			
		}

		if (!m_bUseStillPicMode)
		{
			//register ourselves to get messages from the libVlcRTSP object

			m_libVlcRTSP.m_sig_update_status.connect(1, boost::bind(&LibVlcStreamComponent::OnStatusUpdated, this, _1));

			//init the libVLC_RTSP object
			//if url starts with "webcam:" we'll cut off that part and call InitWebcam instead
			if (url.find("webcam:") == 0)
			{
				//cut off the "webcam:" part
				url = url.substr(7);
				url = StripWhiteSpace(url);

				m_libVlcRTSP.InitWebcam(url, m_pSurface, this, width, height, settings);
			}
			else
			{

				if (!m_libVlcRTSP.Init(url, cacheMS, m_pSurface, this, width, height, settings))
				{
					//show a windows text box with an error
					MessageBox(NULL, "Can't init libVLC stream.", "Error", MB_OK);
				}
				else
				{
					m_libVlcRTSP.SetLooping(*m_pLooping != 0);
					//default color, if the stream isn't video, it will stay like this forever.
					//A clearly visible dark panel so audio-only players show their bounds and feel grabbable.
					m_pSurface->FillColor(glColorBytes(40, 40, 50, 240));
				}
			}
		}
	}

	if (!m_title.empty())
	{
		//Render the title as a single-line label that auto-scales to fit the widget width.
		//We measure at scale 1.0 then compute the scale needed to fit (width - 6px padding).
		//Capped at 0.6 so short titles don't look comically huge on a small audio widget,
		//and floored at 0.25 so a really long title shrinks gracefully instead of disappearing.
		Entity* pEnt = CreateTextLabelEntity(GetParent(), "TitleText", 3, 2, m_title);

		eFont fontID = FONT_LARGE;
		float scale = 0.6f;
		float availWidth = m_pSize2d->x - 6;
		if (availWidth > 0)
		{
			CL_Vec2f vSize = GetBaseApp()->GetFont(fontID)->MeasureText(m_title, 1.0f);
			if (vSize.x > 0)
			{
				scale = availWidth / vSize.x;
				if (scale > 0.6f) scale = 0.6f;
				if (scale < 0.25f) scale = 0.25f;
			}
		}
		SetupTextEntity(pEnt, fontID, scale);
		TypeTextLabelEntity(pEnt, 0, 20);
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

void LibVlcStreamComponent::OnTouchDragUpdate(VariantList* pVList)
{
	ResetTimeOfLastTouch();
}

void LibVlcStreamComponent::OnStatusUpdated(VariantList* pVList)
{
	//LogMsg("Got status update: %d", pVList->Get(0).GetUINT32());
	switch (pVList->Get(0).GetUINT32())
	{
		case libVLC_RTSP::C_STATUS_SET_VOLUME:
		{
			float vol = pVList->Get(1).GetFloat();
			m_pVolSliderComp->SetSliderPosition(vol);
			break;
		}

		case libVLC_RTSP::C_STATUS_INITTED:
		{
			UpdateControlButtons(false);
			break;
		}
	
		case libVLC_RTSP::C_STATUS_PAUSED:
		{
			UpdateControlButtons(true);
		}
		break;

		case libVLC_RTSP::C_STATUS_UNPAUSED:
		{
			UpdateControlButtons(false);
		}
		break;
	}
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
		if (m_libVlcRTSP.GetVolume() != 0)
		{
			UpdateStatusMessage("Muted");
		}
		m_libVlcRTSP.SetVolume(0);
	}
	else
	{
		//unmute
		if (m_libVlcRTSP.GetVolume() != m_savedVolume)
		{
			m_libVlcRTSP.SetVolume(m_savedVolume);
			UpdateStatusMessage("Unmuted");
		}
	}
	
	m_bMuted = bMute;
	//UpdateControlButtons();
}

void LibVlcStreamComponent::SetMutedVolume(float vol)
{
	m_savedVolume = vol;
	SetMute(true);
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
				ResetTimeOfLastTouch();
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
				ResetTimeOfLastTouch();
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
				ResetTimeOfLastTouch();
				m_libVlcRTSP.SetVolume(m_libVlcRTSP.GetVolume() - 0.1f);
				//ShowVolume();
			}

			if (letterPressed == '=')
			{
				ResetTimeOfLastTouch();
				//toggle looping
				m_libVlcRTSP.SetVolume(m_libVlcRTSP.GetVolume() + 0.1f);
				//ShowVolume();
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
#ifdef _DEBUG
	LogMsg("Setting libVLX surface to %d x %d", width, height);
#endif

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

	UpdateProgressBar();

}



void LibVlcStreamComponent::OnScaleChanged(Variant* pDataObject)
{
	ResetTimeOfLastTouch();
	//update the progress bar
	UpdateProgressBar();
}

void LibVlcStreamComponent::UpdateControlButtons(bool bIsPaused)
{
	if (bIsPaused)
	{
		//Set the m_pButtonPlay entity to the second frame of its bmp strip
		m_pButtonPlay->GetComponentByName("OverlayRender")->GetVar("frameX")->Set(uint32(0));
	}
	else
	{
		m_pButtonPlay->GetComponentByName("OverlayRender")->GetVar("frameX")->Set(uint32(1));

	}

}

void LibVlcStreamComponent::SetControlsTransparency(float alpha)
{

	if (m_pControlsEnt)
	{
		if (m_lastTargetAlpha != alpha)
		{
			//LogMsg("Updating transparency: %.2f", alpha);
			
			int fadeTimeMS = 1000;
			if (alpha >= 50)
			{
				//do it faster
				fadeTimeMS = 0;
			}
			FadeEntity(m_pControlsEnt, true, alpha, fadeTimeMS, 0, false);
			m_lastTargetAlpha = alpha;
		}
	}
	else
	{
		LogMsg("Transparency controls can't be set because they aren't initted yet");
	}
	
}

void LibVlcStreamComponent::SetControlsAlwaysVisible(bool bAlwaysVisible)
{
	m_bControlsAlwaysVisible = bAlwaysVisible;
	if (bAlwaysVisible)
	{
		//show right away; UpdateProgressBar will keep it pinned from now on
		ResetTimeOfLastTouch();
		SetControlsTransparency(1.0f);
	}
}

void LibVlcStreamComponent::OnPlayButtonClicked(VariantList* pVList)
{
	//LogMsg("They clicked the play toggle button");
	bool bIsPaused = m_libVlcRTSP.GetPause();

	//Handle the "never started" state created by VLC_ExtraSettings.startPaused: the
	//player is loaded but has never entered the playing state, so GetPause() is
	//false (state is NothingSpecial/Stopped, not libvlc_Paused) AND is_playing is
	//also false.  The plain toggle below would call SetPause(true), which falls
	//through libVLC_RTSP::SetPause's else-branch and does nothing - leaving the
	//play button visually inert.  Detect that case and start playback explicitly.
	libvlc_media_player_t* pMP = m_libVlcRTSP.GetMP();
	bool bIsPlaying = pMP && libvlc_media_player_is_playing(pMP) != 0;
	if (!bIsPaused && !bIsPlaying)
	{
		if (m_bRestartOnPlay) m_libVlcRTSP.SetPlaybackPosition(0.0f);
		m_libVlcRTSP.SetPause(false); //else-branch in SetPause kicks playback off
		return;
	}

	//if restart-on-play is enabled and we're transitioning paused -> playing, rewind first
	if (bIsPaused && m_bRestartOnPlay)
	{
		m_libVlcRTSP.SetPlaybackPosition(0.0f);
	}
	m_libVlcRTSP.SetPause(!bIsPaused);
	//UpdateControlButtons();
}


void LibVlcStreamComponent::OnSliderVolumeChanged(Variant* pDataObject)
{
	m_bMuted = false;
	float sliderVal = pDataObject->GetFloat();
	//LogMsg("Slider cool changed to %.2f", sliderVal);
	//Set volume
	this->ResetTimeOfLastTouch();

	m_libVlcRTSP.SetVolume(sliderVal);

}


void LibVlcStreamComponent::UpdateProgressBar()
{
	int barHeight = 32;
	int spaceButtonsTakeUpX = 32;
	float spacerX = 16;
	float volumeWidthX = 100;

	CL_Rectf playSliderRect = CL_Rectf(spaceButtonsTakeUpX+ volumeWidthX+spacerX+ spacerX, m_pSize2d->y - barHeight, m_pSize2d->x, m_pSize2d->y);
	 
	//one time init?
	if (!m_pProgressEnt)
	{
		//create entity to hold all this
		m_pControlsEnt = GetParent()->AddEntity(new Entity("playControls"));
		SetAlignmentEntity(m_pControlsEnt, ALIGNMENT_UPPER_LEFT);

		//create rect component to show progress
		m_pProgressEnt = m_pControlsEnt->AddEntity(new Entity("bar"));

		EntityComponent* pBar = m_pProgressEnt->AddComponent(new ProgressBarComponent);
		pBar->GetVar("interpolationTimeMS")->Set(uint32(40)); //update faster
		pBar->GetVar("interpolation")->Set(uint32(INTERPOLATE_SMOOTHSTEP));

		//when our parent changes scale, so also doth our progress bar's entity
		SetAlignmentEntity(m_pProgressEnt, ALIGNMENT_UPPER_LEFT);
		GetParent()->GetVar("scale2d")->GetSigOnChanged()->connect(boost::bind(&LibVlcStreamComponent::OnScaleChanged, this, _1));
		m_pProgressEnt->GetVar("color")->Set(MAKE_RGBA(200, 0, 0, 170));
		pBar->GetVar("borderColor")->Set(MAKE_RGBA(0, 0, 0, 180));
		pBar->GetVar("backgroundColor")->Set(MAKE_RGBA(100, 100, 100, 150));
		pBar->GetVar("visualPixelModY")->Set(-25.0f); //add this many pixels off the top and button of the bar we're drawing

		SetProgressBarPercent(m_pProgressEnt, m_libVlcRTSP.GetPlaybackPosition(), true);
		//all well and good, but let's also let the person tap on the progress bar to set the position of the media
		
		//Add a TouchStripComponent and respond to its event
		EntityComponent* pTouchStrip = m_pProgressEnt->AddComponent(new TouchStripComponent);
		pTouchStrip->GetParent()->GetFunction("OnTouchStripUpdate")->sig_function.connect(1, boost::bind(&LibVlcStreamComponent::OnStripUpdate, this, _1));
		SetTouchPaddingEntity(m_pProgressEnt, CL_Rectf(0, 0, 0, 0));

		//oh, hey, we should add a button (based on buttons.png for the image)
		//buttons.png frames: 0 = play-arrow icon, 1 = pause-bars icon.  Start on frame 0
		//(play-arrow) so audio widgets that haven't played yet show the right icon.  Video
		//streams that auto-play will flip to frame 1 within a frame or two via the
		//C_STATUS_UNPAUSED event handler in OnStatusUpdated.
		Entity* pButton = CreateOverlayButtonEntity(m_pControlsEnt, "buttonPlay", "interface/buttons.png",0,0);
		SetAlignmentEntity(pButton, ALIGNMENT_UPPER_LEFT);
		SetupAnimEntity(pButton, 6, 1, 0, 0);
		SetButtonClickSound(pButton, "");
		pButton->GetFunction("OnButtonSelected")->sig_function.connect(1, boost::bind(&LibVlcStreamComponent::OnPlayButtonClicked, this, _1));
		SetButtonRepeatDelayMS(pButton, 50);
		SetTouchPaddingEntity(pButton, CL_Rectf(0, 0, 0, 0));
		m_pButtonPlay = pButton; //remember for later

		//oh, let's add a slider control too
		
		m_pVolSliderComp = (SliderComponent*)CreateSlider(m_pControlsEnt, spaceButtonsTakeUpX+spacerX, 15, volumeWidthX, "interface/slider_button.png", "", "", "");
		m_pVolSliderComp->GetVar("progress")->Set(GetApp()->GetVar("slider_cool")->GetFloat()); //set to a default value
		m_pVolSliderComp->GetVar("progress")->GetSigOnChanged()->connect(1, boost::bind(&LibVlcStreamComponent::OnSliderVolumeChanged, this, _1));
		
		m_pVolSliderComp->SetSliderPosition(1.0f);

	}

	//place the controls at the bottom left
	SetPos2DEntity(m_pControlsEnt, CL_Vec2f(0, playSliderRect.top));
	//LogMsg("VLC reporting progress as %.2f", m_libVlcRTSP.GetPlaybackPosition());
	
	SetPos2DEntity(m_pProgressEnt, CL_Vec2f(playSliderRect.left, 0));
	m_pProgressEnt->GetVar("size2d")->Set(playSliderRect.get_width(), playSliderRect.get_height());
	SetProgressBarPercent(m_pProgressEnt, m_libVlcRTSP.GetPlaybackPosition(), true);

	//compare m_timeOfLastTouchMS to GetTick(), if more than 2 seconds have passed, we'll fade it out
	//(skip the fade if a script has pinned the controls visible via set_controls_locked)
	if (m_bControlsAlwaysVisible)
	{
		SetControlsTransparency(1.0f);
	}
	else if (GetTick() - m_timeOfLastTouchMS > 2000)
	{
		SetControlsTransparency(0.0f);
	}
	else
	{
		SetControlsTransparency(1.0f);
	}
}


void LibVlcStreamComponent::OnStripUpdate(VariantList* pVList)
{

#ifdef _DEBUG
	LogMsg("X touched at %.2f", pVList->Get(1).GetVector2().x);
#endif
	this->ResetTimeOfLastTouch();
	m_libVlcRTSP.SetPlaybackPosition(pVList->Get(1).GetVector2().x);

}

void LibVlcStreamComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void LibVlcStreamComponent::OnUpdate(VariantList* pVList)
{
	m_libVlcRTSP.Update();
	UpdateProgressBar();
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

LibVlcStreamComponent* AddNewStream(std::string name, std::string url, int cacheMS, Entity* pGUIEnt, bool bIgnoreIfExists, string title, VLC_ExtraSettings settings)
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
	pVlcComp->SetTitle(title);
	pVlcComp->Init(url, cacheMS, settings);

	return pVlcComp;
}
