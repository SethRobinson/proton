#include "PlatformPrecomp.h"
#include "EnterNameMenu.h"
#include "Entity/EntityUtils.h"
#include "MainMenu.h"

string HighScoreNameFilter(string s)
{
	string temp;

	for (unsigned int i=0; i < s.length(); i++)
	{
		if ( 
			(s[i] >= 65 && s[i] <= 90)
			|| (s[i] >= 97 && s[i] <= 122)
			)
		{
			temp += s[i];		
		}
	}

	return temp;
}

void EnterNameMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[0].Print().c_str());

	if (pEntClicked->GetName() == "check_question")
	{
		bool bChecked = pEntClicked->GetVar("checked")->GetUINT32()!=0;
		if (bChecked)
		{
			LogMsg("clicked checkbox.");
		} else
		{
			LogMsg("un-clicked checkbox.");
		}

		//either way, save it to our db which will get saved when the app closes.
		GetApp()->GetVar("like_checkboxes")->Set(uint32(bChecked));

	}

	if (pEntClicked->GetName() == "Continue")
	{
		string name = GetEntityRoot()->GetEntityByName("name_input_box")->GetComponentByName("InputTextRender")->GetVar("text")->GetString();
		GetApp()->GetVar("name")->Set(HighScoreNameFilter(name)); //save it to our database so we can remember the default
		LogMsg("Read %s for the name", GetApp()->GetVar("name")->GetString().c_str());
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
	}
}

Entity * EnterNameMenuCreate(Entity *pParentEnt)
{
	Entity *pBG = CreateOverlayEntity(pParentEnt, "EnterNameMenu", "interface/summary_bg.rttex", 0,0);
	AddFocusIfNeeded(pBG);

	Entity *pButtonEntity;
	CL_Vec2f vTextAreaPos = CL_Vec2f(45,40);
	CL_Vec2f vTextAreaBounds = CL_Vec2f(384,170);
	
	string title = "`$Input some stuff!";
	pButtonEntity = CreateTextLabelEntity(pBG, "title", vTextAreaPos.x, vTextAreaPos.y, title);
	pButtonEntity->GetComponentByName("TextRender")->GetVar("font")->Set(uint32(FONT_LARGE));
	pButtonEntity->GetVar("scale2d")->Set(CL_Vec2f(0.6f, 0.6f));
	vTextAreaPos.y += 25;
	
	string msg = "`wTap below`` to set your name. Hey, word wrapping, cool.";
	Entity *pText = CreateTextBoxEntity(pBG, "text", vTextAreaPos, vTextAreaBounds, msg);

	float nameEntryY = vTextAreaPos.y + pText->GetVar("size2d")->GetVector2().y; //get the exact size that the
	//final word wrapped text actually took.
	nameEntryY += 10; //add some spacer too
	pButtonEntity = CreateTextLabelEntity(pBG, "name", vTextAreaPos.x, nameEntryY, "`$Name: ");
//	pButtonEntity->GetComponentByName("TextRender")->GetVar("font")->Set(uint32(FONT_LARGE));
	float nameWidth = pButtonEntity->GetVar("size2d")->GetVector2().x;

	//create input box
	pButtonEntity = CreateInputTextEntity(pBG, "name_input_box", vTextAreaPos.x+nameWidth, nameEntryY, GetApp()->GetShared()->GetVarWithDefault("name", string("Player"))->GetString());
	
	EntityComponent *pTextRenderComp = pButtonEntity->GetComponentByName("InputTextRender");

	//To allow spaces and punctuation, we active loose filtering. Use FILTERING_STRICT instead for say, a highscore name
	pTextRenderComp->GetVar("filtering")->Set(uint32(InputTextRenderComponent::FILTERING_LOOSE));

	//you also need to do this for loose filtering to make sure the best keyboard is chosen on the device
	pTextRenderComp->GetVar("inputType")->Set(uint32(InputTextRenderComponent::INPUT_TYPE_ASCII_FULL));

	//control how many characters the user can enter
	pTextRenderComp->GetVar("inputLengthMax")->Set(uint32(18));

	//show *'s, password mode
	//pTextRenderComp->GetVar("visualStyle")->Set((uint32)InputTextRenderComponent::STYLE_PASSWORD);


	//if you wanted text to appear until it's activated:
	//pTextRenderComp->GetVar("placeHolderText")->Set("Tap here to enter your name");

	//to change the font of the text in the input box
	//pTextRenderComp->GetVar("font")->Set(uint32(FONT_LARGE));


	//let's add a checkbox too, for fun
	bool bLikeCheckboxes = GetApp()->GetVar("like_checkboxes")->GetUINT32() != 0;
	nameEntryY += 40;

	Entity *pCheckbox = CreateCheckbox(pBG, "check_question", "I like checkboxes", vTextAreaPos.x, nameEntryY, bLikeCheckboxes);
	pCheckbox->GetFunction("OnButtonSelected")->sig_function.connect(&EnterNameMenuOnSelect);


	//the continue button
	pButtonEntity = CreateOverlayButtonEntity(pBG, "Continue","interface/summary_continue.rttex", 100, 282);
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&EnterNameMenuOnSelect);
	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK); //for android's back button, or escape key in windows


	SlideScreen(pBG, true);
	return pBG;
}


