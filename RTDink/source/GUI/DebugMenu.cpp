#include "PlatformPrecomp.h"
#include "DebugMenu.h"
#include "Entity/EntityUtils.h"
#include "../dink/dink.h"
#include "LogMenu.h"
#include "GameMenu.h"
#include "PopUpMenu.h"

extern bool g_script_debug_mode;

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
		GetAudioManager()->Play("audio/intro.ogg", true, true);
	}

	if (pEntClicked->GetName() == "music_off")
	{
		GetAudioManager()->StopMusic();
	}

	if (pEntClicked->GetName() == "log")
	{
		RemoveFocusIfNeeded(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		LogMenuCreate(pEntClicked->GetParent()->GetParent());
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
	}


	if (pEntClicked->GetName() == "Back")
	{
		//slide it off the screen and then kill the whole menu tree
		RemoveFocusIfNeeded(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		AddFocusIfNeeded(pEntClicked->GetParent()->GetParent());
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
	}

	if (pEntClicked->GetName() == "AddStrength")
	{
		//slide it off the screen and then kill the whole menu tree
		DinkModStrength(1);
	}

	if (pEntClicked->GetName() == "AddDefense")
	{
		//slide it off the screen and then kill the whole menu tree
		DinkModDefense(1);
	}

	if (pEntClicked->GetName() == "AddMagic")
	{
		//slide it off the screen and then kill the whole menu tree
		DinkModMagic(1);
	}

	if (pEntClicked->GetName() == "AddLife")
	{
		//slide it off the screen and then kill the whole menu tree
		DinkModLifeMax(10);
	}

	if (pEntClicked->GetName() == "FillLife")
	{
		//slide it off the screen and then kill the whole menu tree
		DinkFillLife();
	}

	if (pEntClicked->GetName() == "AddGold")
	{
		//slide it off the screen and then kill the whole menu tree
		DinkModGold(100);
	}

	if (pEntClicked->GetName() == "ghost_walk")
	{
		GetApp()->SetGhostMode(!GetApp()->GetGhostMode());
		if (GetApp()->GetGhostMode())
		{
			ShowQuickMessage("Ghost walk enabled - can walk through hardness and screenlocks");
		}
		else
		{
			ShowQuickMessage("Ghost walk disabled");

		}
	}

	if (pEntClicked->GetName() == "debug_mode")
	{
		
		if (g_script_debug_mode)
		{
			g_script_debug_mode = false;
			ShowQuickMessage("DinkC debug logging disabled");
		}
		else
		{
			g_script_debug_mode = true;
			ShowQuickMessage("DinkC debug logging enabled");

		}
	}

	if (pEntClicked->GetName() == "empty_cache")
	{
		DinkUnloadGraphicsCache();
		LogMsg("Cache emptied");
	}
	
	if (pEntClicked->GetName() == "AddBow")
	{
		//slide it off the screen and then kill the whole menu tree
		DinkAddBow();
	}

	
	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


Entity * DebugMenuCreate(Entity *pParentEnt)
{
	//Entity *pBG = CreateOverlayEntity(pParentEnt, "DebugMenu", "interface/generic_bg.rttex", 0,0);
	
	Entity *pBG = CreateOverlayRectEntity(pParentEnt, CL_Vec2f(0,0), GetScreenSize(), MAKE_RGBA(0,0,0,140));
	pBG->SetName("CheatMenu");

	AddFocusIfNeeded(pBG);

	Entity *pButtonEntity;
	float x = iPhoneMapX(30);
	float y = iPhoneMapY(30);
	float ySpacer = iPhoneMapY(40);

	pButtonEntity = CreateTextButtonEntity(pBG, "FPS", x, y, "Toggle FPS Display"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);
	//pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));

if (GetApp()->GetCheatsEnabled())
{
	pButtonEntity = CreateTextButtonEntity(pBG, "empty_cache", x, y, "Empty graphic cache"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "ghost_walk", x, y, "Ghost walk toggle"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

}
	pButtonEntity = CreateTextButtonEntity(pBG, "log", x, y, "View log"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "debug_mode", x, y, "Debug DinkC Toggle"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);


	pButtonEntity = CreateTextButtonEntity(pBG, "Back", x, y, "Back"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);
	//pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK);


	if (GetApp()->GetCheatsEnabled())
	{
	//buttons on right
	x = iPhoneMapX(200);
	y = iPhoneMapY(30);

	pButtonEntity = CreateTextButtonEntity(pBG, "AddStrength", x, y, "Increase Strength"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "AddDefense", x, y, "Increase Defense"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "AddMagic", x, y, "Increase Magic"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "AddLife", x, y, "Increase Life"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "FillLife", x, y, "Refill Life"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "AddGold", x, y, "Add 100 Gold"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);
	
	pButtonEntity = CreateTextButtonEntity(pBG, "AddBow", x, y, "Add Bow"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DebugMenuOnSelect);
	}
	SlideScreen(pBG, true, 500);
	
	if (GetApp()->GetVarWithDefault("cheat_warning", uint32(0))->GetUINT32() == 0)
	{
		GetApp()->GetVar("cheat_warning")->Set(uint32(1));

		PopUpCreate(pBG, "`4WARNING!``\n\nUsing cheats may break the game in strange ways and is only for testing or if you're desperate!", "", "cancel", "I get it", "", "", true);
	}


	return pBG;
}

