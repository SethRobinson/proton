//  ***************************************************************
//  LibVlcStreamComponent - Creation date: 8/31/2023 2:52:20 PM
//  -------------------------------------------------------------
//  License: Uh, check for license.txt or license.md for that?
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include "Entity/Component.h"
#include "Entity/SliderComponent.h"
#include "Manager/libVLC_RTSP.h"


//plays a stream from a URL or file using libVLC

/*

	//example initting a new video/stream


	Entity *pEnt = pGUIEnt->AddEntity(new Entity(name));
	pVlcComp = (LibVlcStreamComponent*) pEnt->AddComponent(new LibVlcStreamComponent());
	pVlcComp->Init(urlOrFileName, cacheMS);

	//to enable looping video:
	pVlcComp->GetVar("looping")->Set(uint32(1));


*/

class LibVlcStreamComponent : public EntityComponent
{
public:
	LibVlcStreamComponent();
	virtual ~LibVlcStreamComponent();

	virtual void OnAdd(Entity* pEnt);
	void OnSetPause(VariantList* pList);
	void OnSetPlaybackPosition(VariantList* pList);
	virtual void OnRemove();
	void SetMutedVolume(float vol);
	float GetMutedVolume() { return m_savedVolume; }
	void Init(std::string url, int cacheMS);
	void OnUpdate(VariantList* pVList);
	void SetSurfaceSize(int width, int height);

	libVLC_RTSP* GetLibVlcRTSP() { return &m_libVlcRTSP; }
	void SetMute(bool bMute);
	bool GetMute() { return m_bMuted; }

	float GetVolume() { return m_libVlcRTSP.GetVolume(); }
	void SetVolume(float vol) { m_libVlcRTSP.SetVolume(vol); }  // Set the volume level. Values range between 0 and 1
	void OnScaleChanged(Variant* pDataObject);
	string GetTitle() { return m_title; }
	void SetTitle(string title) { m_title = title; }

protected:

	void OnSliderVolumeChanged(Variant* pDataObject);
	void UpdateControlButtons(bool bIsPaused);
	void OnPlayButtonClicked(VariantList* pVList);
	void OnStripUpdate(VariantList* pVList);
	void UpdateProgressBar();
	void ShowVolume();
	void OnInputWhileMouseDown(VariantList* pVList);
	void OnLoopingChanged(Variant* pVariant);
	void UpdateStatusMessage(string msg);
	void OnStatusUpdated(VariantList* pVList);

	string m_url;
	int m_cacheMS = 0;
	libVLC_RTSP m_libVlcRTSP;
	SurfaceAnim* m_pSurface = NULL;
	CL_Vec2f* m_pPos2d = NULL;
	CL_Vec2f* m_pSize2d = NULL;
	CL_Vec2f* m_pScale2d = NULL;
	uint32 * m_pLooping = NULL;
	bool m_bMuted = false;
	float m_savedVolume = 1.0f;
	Entity* m_pProgressEnt = NULL;
	Entity* m_pControlsEnt = NULL;
	Entity* m_pButtonPlay = NULL;
	SliderComponent* m_pVolSliderComp = NULL;
	string m_title;
};

//helper
LibVlcStreamComponent* AddNewStream(std::string name, std::string url, int cacheMS, Entity* pGUIEnt, bool bIgnoreIfExists = true, string title = "");
LibVlcStreamComponent* GetStreamEntityByName(std::string name);
void SetStreamVolumeByName(std::string name, float volume);