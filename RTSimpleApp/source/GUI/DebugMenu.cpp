#include "PlatformPrecomp.h"
#include "DebugMenu.h"
#include "Entity/EntityUtils.h"

void DebugMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity", pEntClicked->GetName().c_str());

	if (pEntClicked->GetName() == "FPS")
	{
		GetBaseApp()->SetFPSVisible(!GetBaseApp()->GetFPSVisible());
	}
	if (pEntClicked->GetName() == "music_on")
	{
		GetAudioManager()->Play(ReplaceMP3("audio/techno.mp3"), true, true, true, false);

		//((AudioManagerFlash*)GetAudioManager())->Testy();

	}

	if (pEntClicked->GetName() == "music_off")
	{
		GetAudioManager()->StopMusic();
	}

	if (pEntClicked->GetName() == "toggle_fullscreen")
	{
		GetBaseApp()->OnFullscreenToggleRequest();
		
		//if you wanted to set a specific size instead:
		//GetBaseApp()->SetVideoMode(200, 200, false);
	}

	if (pEntClicked->GetName() == "Back")
	{
		//slide it off the screen and then kill the whole menu tree
		RemoveFocusIfNeeded(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		AddFocusIfNeeded(pEntClicked->GetParent()->GetParent());
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
	}

	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


Entity * DebugMenuCreate(Entity *pParentEnt)
{
	Entity *pBG = CreateOverlayEntity(pParentEnt, "DebugMenu", "interface/summary_bg.rttex", 0,0);
	AddFocusIfNeeded(pBG);

	Entity *pButtonEntity;
	float x = 80;
	float y = 40;
	float ySpacer = 40;

	pButtonEntity = CreateTextButtonEntity(pBG, "FPS", x, y, "Toggle FPS Display"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);
	
	pButtonEntity = CreateTextButtonEntity(pBG, "music_on", x, y, "Play bg music"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);
	
	pButtonEntity = CreateTextButtonEntity(pBG, "music_off", x, y, "Stop bg music"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);
	
	//if (IsDesktop()) //normally you'd want this so this only shows up in desktop builds
	{
		pButtonEntity = CreateTextButtonEntity(pBG, "toggle_fullscreen", x, y, "Toggle fullscreen (or Alt-Enter)"); y += ySpacer;
		pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

	}


	pButtonEntity = CreateTextButtonEntity(pBG, "Back", x, y, "Back"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);
	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK); //for android's back button, or escape key in windows

	SlideScreen(pBG, true, 500);
	return pBG;
}

