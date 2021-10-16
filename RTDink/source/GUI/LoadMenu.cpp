#include "PlatformPrecomp.h"
#include "LoadMenu.h"
#include "Entity/EntityUtils.h"
#include "MainMenu.h"
#include "../dink/dink.h"
#include "GameMenu.h"

#define C_SAVE_GAME_COUNT 10

void LoadMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity", pEntClicked->GetName().c_str());

	if (pEntClicked->GetName() == "Back")
	{
		//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
		//LoadMenuCreate(GetParent()
	}

	int num = atol(pEntClicked->GetName().c_str());

	if (num > 0)
	{
		//let's load this sucka
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		
		if (num == 10)
		{
			string fName = GetSavePath()+"dink/"+"autosave.dat";
			GameCreate(pEntClicked->GetParent()->GetParent(), 0, fName);
			
		} else
		{
			GameCreate(pEntClicked->GetParent()->GetParent(), num, "");
		}
		
	}
	//GetEntityRoot()->PrintTreeAsText(); //useful for Loading
}

void SetupLoadButton(Entity *pParent, int x, float *pY, int gameID)
{
	gameID++; //it's 1 based, not 0 based

	const float ySpacer = 46;
	if (gameID > 5) x += 200;

	char stTemp[256];
	char stFormatted[256];
	string clickKey = ""; //none
	int gameTime;
	
	if (gameID == 10)
	{
		string autoSave = DinkGetSavePath() + "autosave.dat";

		if (!FileExists(autoSave))
		{
			sprintf(stFormatted, "Auto Save - None yet", gameID);
		} else
		{
			gameTime = 0;
			string description = "Unknown";

			VariantDB db;
			bool bFileExisted = false;

			if (db.Load(DinkGetSavePath()+"autosavedb.dat", &bFileExisted, false) && bFileExisted )
			{
				gameTime = db.GetVar("minutes")->GetUINT32();
				description = db.GetVar("description")->GetString();
			}


			sprintf(stFormatted, "Auto Save - %d:%02d - %s", (gameTime / 60), gameTime - ((gameTime / 60) * 60), description.c_str());
		clickKey = toString(gameID);
		}
	} else
	
	if (load_game_small(gameID, stTemp, &gameTime))
	{
		sprintf(stFormatted, "Slot %d - %d:%02d - %s", gameID, (gameTime / 60), gameTime - ((gameTime / 60) * 60) , stTemp);
		clickKey = toString(gameID);
	} else
	{
		sprintf(stFormatted, "Slot %d - Empty", gameID);
	}
	
	string butText = stFormatted;
	Entity * pButtonEntity = CreateTextButtonEntity(pParent, clickKey, iPhoneMapX(x), iPhoneMapY(*pY), butText, false); *pY += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&LoadMenuOnSelect);
	//pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
}

Entity * LoadMenuCreate(Entity *pParentEnt)
{
	Entity *pBG = CreateOverlayEntity(pParentEnt, "LoadMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	AddFocusIfNeeded(pBG, true);

	Entity *pButtonEntity;
	float x = 80;
	float yStart = 40;
	float y = yStart;
	float ySpacer = 50;

	for (int i=0; i < C_SAVE_GAME_COUNT; i++)
	{
		if (i == 5) y = yStart;

		SetupLoadButton(pBG, x, &y, i);
	}

	pButtonEntity = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(x), iPhoneMapY(y), "Back"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&LoadMenuOnSelect);
	//pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));

	SlideScreen(pBG, true, 500);
	return pBG;
}

