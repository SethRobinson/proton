#include "PlatformPrecomp.h"
#include "EnterURLMenu.h"
#include "Entity/EntityUtils.h"
#include "DMODMenu.h"
#include "DMODInstallMenu.h"
#include "../dink/dink.h"
#include "PopUpMenu.h"

void EnterURLMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	Entity *pMenu = pEntClicked->GetParent();
	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[0].Print().c_str());

	if (pEntClicked->GetName() == "Back")
	{
		//slide it off the screen and then kill the whole menu tree
		pEntClicked->GetParent()->RemoveComponentByName("FocusInput");
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		DMODMenuCreate(pEntClicked->GetParent()->GetParent(), true);
	}


	if (pEntClicked->GetName() == "Continue")
	{
		string name = GetEntityRoot()->GetEntityByName("name_input_box")->GetComponentByName("InputTextRender")->GetVar("text")->GetString();
		DisableAllButtonsEntity(pMenu);
		GetApp()->GetVar("dmod_download_url")->Set(name); //save it to our database so we can remember the default
		LogMsg("Read %s for the name", GetApp()->GetVar("name")->GetString().c_str());
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		
		DMODInstallMenuCreate(pEntClicked->GetParent()->GetParent(), GetApp()->GetVar("dmod_download_url")->GetString(), GetDMODRootPath() );
	}

	if (pEntClicked->GetName() == "paste")
	{
		string text = GetClipboardText();

		if (!text.empty())
		{
			GetEntityRoot()->GetEntityByName("name_input_box")->GetComponentByName("InputTextRender")->GetVar("text")->Set(""); //clear it first
			GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_PASTE, Variant(text), 0); 
		} else
		{
			//kill the keyboard
			GetEntityRoot()->GetEntityByName("name_input_box")->GetComponentByName("InputTextRender")->GetFunction("CloseKeyboard")->sig_function(NULL);
			PopUpCreate(pMenu, "Paste buffer is currently empty.", "", "cancel", "Continue", "", "", false);

		}
	}
}

Entity * EnterURLMenuCreate(Entity *pParentEnt)
{
	Entity *pBG = CreateOverlayEntity(pParentEnt, "EnterURLMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	AddFocusIfNeeded(pBG);

	Entity *pButtonEntity;
	CL_Vec2f vTextAreaPos = iPhoneMap(25,40);
	CL_Vec2f vTextAreaBounds = iPhoneMap(384,234);

	string title = "`$Add-On Download from URL";
	pButtonEntity = CreateTextLabelEntity(pBG, "title", vTextAreaPos.x, vTextAreaPos.y, title);
	pButtonEntity->GetComponentByName("TextRender")->GetVar("font")->Set(uint32(FONT_LARGE));
	pButtonEntity->GetVar("scale2d")->Set(CL_Vec2f(0.6f, 0.6f));
	vTextAreaPos.y += iPhoneMapY(25);

	string msg = "Enter a URL to a .dmod file to download and install.  (example: https://rtsoft.com/NewQuest.dmod )";

	switch (GetEmulatedPlatformID())
	{
		case PLATFORM_ID_WINDOWS:
			msg = "Enter a URL to a .dmod file to download and install.  (example: https://rtsoft.com/NewQuest.dmod ) Use Ctrl-V to paste from the clipboard.";
		break;
	}
	
	Entity *pText = CreateTextBoxEntity(pBG, "text", vTextAreaPos, vTextAreaBounds, msg);

	pButtonEntity = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(25), iPhoneMapY(284), "Back", false);
	pButtonEntity->GetFunction("OnButtonSelected")->sig_function.connect(&EnterURLMenuOnSelect);
	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK);


	//the continue button
	pButtonEntity = CreateTextButtonEntity(pBG, "Continue", iPhoneMapX(356), iPhoneMapY(284), "Continue", false);
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&EnterURLMenuOnSelect);

	//create button for pasting
if (GetEmulatedPlatformID() != PLATFORM_ID_WEBOS)
{
	pButtonEntity = CreateTextButtonEntity(pBG, "paste", vTextAreaPos.x, vTextAreaPos.y+iPhoneMapX(80), "[tap here to paste]");
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&EnterURLMenuOnSelect);
}

	//create input box

	pButtonEntity = CreateInputTextEntity(pBG, "name_input_box", vTextAreaPos.x, vTextAreaPos.y+iPhoneMapX(49), GetApp()->GetShared()->GetVarWithDefault("dmod_download_url", string(""))->GetString(),
		iPhoneMapX(450), 0);
	pButtonEntity->GetComponentByName("InputTextRender")->GetVar("inputLengthMax")->Set(uint32(255));
	pButtonEntity->GetComponentByName("InputTextRender")->GetVar("filtering")->Set(uint32(InputTextRenderComponent::FILTERING_LOOSE));
	pButtonEntity->GetComponentByName("InputTextRender")->GetVar("inputType")->Set(uint32(InputTextRenderComponent::INPUT_TYPE_URL));
	//pButtonEntity->GetComponentByName("InputTextRender")->GetVar("font")->Set(uint32(FONT_LARGE));
	
	
	if (IsDesktop())
		pButtonEntity->GetComponentByName("InputTextRender")->GetFunction("ActivateKeyboard")->sig_function(NULL); //give it focus
	

	//a way to get our CreateTextBox function called in 500 seconds, but not called if the entity doesn't exist at that time

	SlideScreen(pBG, true);
	return pBG;
}


