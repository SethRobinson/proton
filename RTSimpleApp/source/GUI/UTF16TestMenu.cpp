#include "PlatformPrecomp.h"
#include "UTF16TestMenu.h"
#include "Entity/EntityUtils.h"
#include "MainMenu.h"

void UTF16TestMenuOnSelect(VariantList* pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity* pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(), pVList->m_variant[1].Print().c_str());

	if (pEntClicked->GetName() == "Back")
	{
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
	}

	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

Entity* UTF16TestMenuCreate(Entity* pParentEnt)
{
	Entity* pBG = CreateOverlayEntity(pParentEnt, "UTF16Test", "interface/bkgd_stone.rttex", 0, 0);
	AddFocusIfNeeded(pBG);

	//Adding some UTF-16 text in a UTF-8 string
	string testMsg = "Съешь ещё этих мягких французских булок, да выпей же чаю!";
	Entity * pTextSmall = CreateTextBoxEntity(pBG, "testTextSmall", CL_Vec2f(20, 20), GetScreenSize() - 40.0f, testMsg);
	SetupTextEntity(pTextSmall, FONT_SMALL);

	CL_Vec2f largeTextPos = { 20, GetSize2DEntity(pTextSmall).y + 30 };
	CL_Vec2f largeTextBounds = GetScreenSize() - 20.0f;
	largeTextBounds.y -= largeTextPos.y;
	Entity* pTextLarge = CreateTextBoxEntity(pBG, "testTextLarge", largeTextPos, largeTextBounds, testMsg);
	SetupTextEntity(pTextLarge, FONT_LARGE);
	
	Entity* pButtonEntity;
	pButtonEntity = CreateTextButtonEntity(pBG, "Back", 240, 290, "Back");
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&UTF16TestMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK); //for android's back button, or escape key in windows

	SlideScreen(pBG, true);
	return pBG;
}

