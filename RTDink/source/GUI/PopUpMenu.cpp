#include "PlatformPrecomp.h"
#include "PopUpMenu.h"
#include "Entity/EntityUtils.h"
#include "../App.h"

Entity * PopUpRestoreFocusToOriginalMenu(Entity *pEntClicked)
{
	Entity *pFinishMenu = GetEntityRoot()->GetEntityByName(pEntClicked->GetParent()->GetParent()->GetVar("finishMenuName")->GetString());
	assert(pFinishMenu);

	if (pFinishMenu)
	{
		if (pEntClicked->GetParent()->GetParent()->GetVar("requireMoveMessages")->GetUINT32() == 1)
		{
			AddFocusIfNeeded(pFinishMenu, true, 0);
		} else
		{
			pFinishMenu->AddComponent(new FocusInputComponent);
		}
	}
	return pFinishMenu;
}

void ReloadMainMenu(VariantList *pVList);

//general purpose pop up menu, I've hardcoded various behaviors here, it knows what to do based on the button name

void PopUpMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	FadeOutEntity(pEntClicked->GetParent()->GetParent(), true, 300);
	GetMessageManager()->CallEntityFunction(pEntClicked->GetParent()->GetParent(), 500, "OnDelete", NULL);
	DisableAllButtonsEntity(pEntClicked->GetParent()->GetParent());
	Entity *pDarken = GetEntityRoot()->GetEntityByName("pop_up_darken");
	
	if (pDarken)
	{
		FadeScreen(pDarken, 0, 0, 400, true);
		KillEntity(pDarken, 400);
		pDarken->SetName("");
	}

	//set the game pause state back to whatever it was originally
	GetApp()->SetGameTickPause(pEntClicked->GetParent()->GetParent()->GetVar("gamePaused")->GetUINT32() != 0);


	if (pEntClicked->GetName() == "url")
	{
		LogMsg("Launch url: %s", pEntClicked->GetVar("url")->GetString().c_str());
		LaunchURL(pEntClicked->GetVar("url")->GetString());
		Entity *pFinishMenu = PopUpRestoreFocusToOriginalMenu(pEntClicked);
	} else if (pEntClicked->GetName() == "url_update")
	{
		LogMsg("Launch url: %s", pEntClicked->GetVar("url")->GetString().c_str());
		LaunchURL(pEntClicked->GetVar("url")->GetString());
		Entity *pFinishMenu = PopUpRestoreFocusToOriginalMenu(pEntClicked);
		//kill current menu
		//GetMessageManager()->CallEntityFunction(pFinishMenu, 200, "OnDelete", NULL);

		PopUpCreate(pFinishMenu, "Please close the game and install the new version!", "", "cancel", "", "", "", true);

	}
	else
		
		/*
		if (pEntClicked->GetName() == "music_disable")
		{
			//add control back
			Entity *pFinishMenu = PopUpRestoreFocusToOriginalMenu(pEntClicked);

			GetApp()->GetShared()->GetVar("musicDisabled")->Set(uint32(1));
			GetAudioManager()->SetMusicEnabled(false);

			string msg = "Music disabled.  You can re-enable it from the `wOptions`` menu later.";
			PopUpCreate(pFinishMenu, msg, "", "cancel", "Continue", "", "", true);
			GetAudioManager()->Play(GetApp()->GetMainMenuMusic(), true, true); //because music is disabled this won't play, but it will remember this if
			//we enable music again so we still want it

		} else

			if (pEntClicked->GetName() == "music_on")
			{
				//add control back
				Entity *pFinishMenu = PopUpRestoreFocusToOriginalMenu(pEntClicked);
				GetAudioManager()->Play(GetApp()->GetMainMenuMusic(), true, true);

			} else

				if (pEntClicked->GetName() == "quit_game")
				{

					Entity *pGameMenu = GetEntityRoot()->GetEntityByName("GameMenu");

					//GetApp()->SetGameType(GAME_TYPE_NONE);
					//slide it off the screen and then kill the whole menu tree
					SlideScreen(pGameMenu, false, 500, 10);
					GetMessageManager()->CallEntityFunction(pGameMenu, 900, "OnDelete", NULL);
					//SummaryMenuCreate(pGameMenu->GetParent());
					GetBaseApp()->SetGameTickPause(false);


				} else
*/

					if (pEntClicked->GetName() == "cancel")
					{
						//add control back
						Entity *pFinishMenu = PopUpRestoreFocusToOriginalMenu(pEntClicked);
					}if (pEntClicked->GetName() == "cancel_update")
					{
						//add control back
						Entity *pFinishMenu = PopUpRestoreFocusToOriginalMenu(pEntClicked);
						//kill current menu
						GetMessageManager()->CallEntityFunction(pFinishMenu, 100, "OnDelete", NULL);
	
						VariantList vList(pFinishMenu->GetParent());
						GetMessageManager()->CallStaticFunction(ReloadMainMenu, 200, &vList, TIMER_SYSTEM);
					}
					else

						if (pEntClicked->GetName() == "reset")
						{
							//add control back
							Entity *pFinishMenu = PopUpRestoreFocusToOriginalMenu(pEntClicked);
							GetAudioManager()->Play("audio/tip_start.wav");
							//GetHighScoreManager()->ResetLocalScores();
							//belay that order, show another pop up...
							PopUpCreate(pFinishMenu, "Local high scores have been reset.", "", "cancel", "Continue", "", "", true);

						} else
						{
							//unhandled
							Entity *pFinishMenu = PopUpRestoreFocusToOriginalMenu(pEntClicked);

							//call this function on the original guy, just in case they want to do something with it
							VariantList vList(pFinishMenu, pEntClicked->GetName());
                            pFinishMenu->GetFunction(pEntClicked->GetName())->sig_function(&vList);

						}

						//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

void PopUpCreate(Entity *pEnt, string msg, string url, string button1Action, string button1Label, string button2Action, string button2Label, 
				 bool bRequireMoveMessages, string button3Action, string button3Label)
{
	pEnt->RemoveComponentByName("FocusInput");

	bool bGamePaused = GetBaseApp()->GetGameTickPause();
	GetBaseApp()->SetGameTickPause(true);
	//	GetMessageManager()->RemoveComponentByName(pEnt, 201, "FocusInput"); //hack that works around a problem of pending FocusInput messages coming in

	//remember where we should give focus to later
	string parentName = pEnt->GetName();
	assert(!parentName.empty());
	
	//let's build our own menu right on the GUI branch of the tree
	pEnt = GetEntityRoot()->GetEntityByName("GUI");
	Entity *pDarken = pEnt->AddEntity(new Entity("pop_up_darken"));
	pDarken->AddComponent(new FocusRenderComponent); 
	pDarken->AddComponent(new FocusUpdateComponent); 
	FadeScreen(pDarken, 0, 0.7, 400, false); //fade the whole GUI

	//add our prompt
	Entity *pBG = CreateOverlayEntity(pEnt, "pop_up", ReplaceWithLargeInFileName("interface/iphone/pop_up.rttex"), 0,0);
	
	//Ok, at this point we can check the image dimensions and center it based on the bitmap size itself
	pBG->GetVar("pos2d")->Set( (GetScreenSize()/2) - pBG->GetVar("size2d")->GetVector2()/2);
	
	//	pBG->AddComponent(new FocusInputComponent);
	pBG->GetVar("finishMenuName")->Set(parentName);

	if (bRequireMoveMessages)
	{
		pBG->GetVar("requireMoveMessages")->Set(uint32(1));
	}

	AddFocusIfNeeded(pBG);
	CL_Vec2f vTextArea = pBG->GetVar("size2d")->GetVector2();
	float padding = 30;
	vTextArea.x -= iPhoneMapX2X(padding*2);

	//add our msg and word wrap it
	Entity *pText = CreateTextBoxEntity(pBG, "pop_up_text", (pBG->GetVar("size2d")->GetVector2()/2)+CL_Vec2f(0, iPhoneMapY2X(-17)), vTextArea, msg);
	SetAlignmentEntity(pText, ALIGNMENT_CENTER);
	float textHeight = pText->GetVar("size2d")->GetVector2().y;

	FadeInEntity(pBG, true, 300);

	//pText->GetVar("color")->Set(MAKE_RGBA(203,177,137,255));

	pBG->GetVar("gamePaused")->Set(uint32(bGamePaused != 0)); //remember this for later
	Entity *pButton = NULL;
	CL_Vec2f vButtonSize;
	Entity *pLabel;
	Entity *pButton1, *pButton2;

	float buttonHeight = iPhoneMapY2X(120);
	if (textHeight > iPhoneMapY2X(50))
	{
		//well, we need more space for this much text.  Move the buttons down a bit.
		buttonHeight = iPhoneMapY2X(135);
	}
	//add the buttons
	pButton = CreateOverlayEntity(pBG, "button1",  ReplaceWithLargeInFileName("interface/iphone/pop_up_button.rttex"), iPhoneMapX2X(21), buttonHeight);
	pButton1 = pButton;
	vButtonSize = pButton->GetVar("size2d")->GetVector2();
	//add the text label
	pLabel = CreateTextButtonEntity(pButton, button1Action, vButtonSize.x/2, vButtonSize.y/2, "`w"+button1Label, false);
	pLabel->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	pLabel->GetFunction("OnButtonSelected")->sig_function.connect(&PopUpMenuOnSelect);
	pLabel->GetVar("url")->Set(url); //just in case we want to know this later, store it in the button itself
	
	if (!url.empty())
	{
		SetButtonStyleEntity(pLabel, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH); //to get around HTML5 rules on uploading, required because it's only
	}
	FadeInEntity(pButton, true, 300, 250);
	if (button2Label.empty())
	{
		//we only have one button?  Fine, center it.
		pButton->GetVar("pos2d")->Set(iPhoneMapX(88), buttonHeight);
	} else
	{
		pButton = CreateOverlayEntity(pBG, "button2", ReplaceWithLargeInFileName("interface/iphone/pop_up_button.rttex"), iPhoneMapY2X(180), buttonHeight);
		pButton2 = pButton;
		vButtonSize = pButton->GetVar("size2d")->GetVector2();
		//add the text label
		pLabel = CreateTextButtonEntity(pButton, button2Action, vButtonSize.x/2, vButtonSize.y/2, "`w"+button2Label, false);
		pLabel->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
		pLabel->GetFunction("OnButtonSelected")->sig_function.connect(&PopUpMenuOnSelect);
		pLabel->GetVar("url")->Set(url); //just in case we want to know this later, store it in the button itself
		
		if (!url.empty())
		{
			SetButtonStyleEntity(pLabel, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH); //to get around HTML5 rules on uploading, required because it's only
		}
		FadeInEntity(pButton, true, 300, 350);

		if (button3Label.empty())
		{
			//we only have two buttons?  fine.  Done then
		} else
		{
			pButton1->GetVar("pos2d")->Set(iPhoneMapX2X(20), buttonHeight);
			pButton2->GetVar("pos2d")->Set(iPhoneMapX2X(180), buttonHeight);

			//move stuff around and add a third button
			pButton = CreateOverlayEntity(pBG, "button2", ReplaceWithLargeInFileName("interface/iphone/pop_up_button.rttex"), iPhoneMapX2X(100), buttonHeight);
			vButtonSize = pButton->GetVar("size2d")->GetVector2();
			//add the text label
			pLabel = CreateTextButtonEntity(pButton, button3Action, vButtonSize.x/2, vButtonSize.y/2, "`w"+button3Label, false);
			pLabel->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
			pLabel->GetFunction("OnButtonSelected")->sig_function.connect(&PopUpMenuOnSelect);
			pLabel->GetVar("url")->Set(url); //just in case we want to know this later, store it in the button itself
			
			if (!url.empty())
			{
				SetButtonStyleEntity(pLabel, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH); //to get around HTML5 rules on uploading, required because it's only
			}
			FadeInEntity(pButton, true, 300, 450);
		}
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}
