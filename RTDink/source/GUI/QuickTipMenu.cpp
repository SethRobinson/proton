#include "PlatformPrecomp.h"
#include "QuickTipMenu.h"
#include "../App.h"
#include "Entity/EntityUtils.h"
#include "Entity/SelectButtonWithCustomInputComponent.h"

void QuickTipMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	DisableAllButtonsEntity(pEntClicked->GetParent());

	GetAudioManager()->Play("audio/tip_end.wav");
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
	Entity *pDarken = GetEntityRoot()->GetEntityByName("pop_up_darken");
	FadeScreen(pDarken, 0, 0, 400, true);
	KillEntity(pDarken, 400);
	pDarken->SetName("");

	//set the game pause state back to whatever it was originally
	GetApp()->SetGameTickPause(pEntClicked->GetParent()->GetParent()->GetVar("gamePaused")->GetUINT32() != 0);

	if (pEntClicked->GetName() == "continue")
	{
		//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);

		Entity *pFinishMenu = GetEntityRoot()->GetEntityByName(pEntClicked->GetParent()->GetVar("finishMenuName")->GetString());
		assert(pFinishMenu);
		if (pFinishMenu)
		{
			if (pEntClicked->GetParent()->GetVar("requireMoveMessages")->GetUINT32() == 1)
			{
				AddFocusIfNeeded(pFinishMenu, true, 100);
			} else
			{
				AddFocusIfNeeded(pFinishMenu, false, 100);
			}
		}

		return;
	}

}

Entity * CreateQuickTipFirstTimeOnly(Entity *pParentEnt, string tipFileName, bool bRequireMoveMessages)
{
	//we access the app database to make sure this is only shown once
	string key = "tip_"+tipFileName;

#ifdef _DEBUG
	//cheat to always show quicktips
	//GetApp()->GetShared()->DeleteVarsStartingWith("tip_"); //for testing
#endif

	if (GetApp()->GetShared()->GetVarIfExists(key))
	{
		return NULL; //already showed this I guess
	}

	//mark it as shown
	GetApp()->GetVar(key)->Set("shown");
	return CreateQuickTip(pParentEnt, tipFileName, bRequireMoveMessages);
}

Entity * CreateQuickTip(Entity *pParentEnt, string tipFileName, bool bRequireMoveMessages)
{
	tipFileName = ReplaceWithLargeInFileNameAndOSSpecific(tipFileName);
	
	pParentEnt->RemoveComponentByName("FocusInput");

	bool bGamePaused = GetBaseApp()->GetGameTickPause();
	GetBaseApp()->SetGameTickPause(true);

	Entity *pEnt;
	//let's build our own menu right on the GUI branch of the tree
	pEnt = GetEntityRoot()->GetEntityByName("GUI");
	Entity *pDarken = pEnt->AddEntity(new Entity("pop_up_darken"));
	pDarken->AddComponent(new FocusRenderComponent); 
	pDarken->AddComponent(new FocusUpdateComponent); 
	FadeScreen(pDarken, 0, 0.5, 400, false); //fade the whole GUI

	Entity *pBG = CreateOverlayEntity(GetEntityRoot()->GetEntityByName("GUI"), "QuickTipMenu", "", 0,0);
	
	SurfaceAnim *pSurf = new SurfaceAnim;
	pSurf->LoadFile(tipFileName);

	OverlayRenderComponent *pOverlay = (OverlayRenderComponent*) pBG->GetComponentByName("OverlayRender");
	pOverlay->SetSurface(pSurf, true);

	
	if (bRequireMoveMessages)
	{
		pBG->GetVar("requireMoveMessages")->Set(uint32(1));
	}
	CL_Vec2f vSize = pBG->GetVar("size2d")->GetVector2();

	pBG->GetVar("pos2d")->Set( (GetScreenSize()/2) - (vSize/2) );
	pBG->GetVar("gamePaused")->Set(uint32(bGamePaused != 0)); //remember this for later

	string parentName = pParentEnt->GetName();
	assert(!parentName.empty());
	pBG->GetVar("finishMenuName")->Set(parentName);
	AddFocusIfNeeded(pBG, false, 1000); //don't allow input for a bit so they don't accidentally dismiss the tip

	Entity *pButtonEntity;
	
	//back button	
	pButtonEntity = CreateButtonHotspot(pBG, "continue", CL_Vec2f(0,0), GetScreenSize());
	
	GetMessageManager()->AddComponent(pButtonEntity, 1000, new SelectButtonWithCustomInputComponent);

	//pButtonEntity = CreateOverlayButtonEntity(pBG, "continue", "interface/quicktips/tip_continue.rttex", 178, 184);
	pButtonEntity->GetFunction("OnButtonSelected")->sig_function.connect(&QuickTipMenuOnSelect);
	SlideScreen(pBG, true);
	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
	//FadeInEntity(pBG, true, 300);
	GetAudioManager()->Play("audio/tip_start.wav");

	return pBG;
}
