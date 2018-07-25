#include "PlatformPrecomp.h"
#include "TouchTestMenu.h"
#include "Entity/EntityUtils.h"
#include "MainMenu.h"
#include "Component/TouchTestComponent.h"

void TouchTestOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	if (pEntClicked->GetName() == "Back")
	{
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
	}

	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

Entity * TouchTestMenuCreate(Entity *pParentEnt)
{
	Entity *pBG = CreateOverlayEntity(pParentEnt, "TouchTest", "interface/summary_bg.rttex", 0,0);
	AddFocusIfNeeded(pBG);

	Entity *pTouchTestEnt = pBG->AddEntity(new Entity(new TouchTestComponent));

	Entity *pButtonEntity;
	pButtonEntity = CreateTextButtonEntity(pBG, "Back", 240, 290, "Back"); 
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&TouchTestOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK); //for android's back button, or escape key in windows

	SlideScreen(pBG, true);
	return pBG;
}

