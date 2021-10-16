#include "PlatformPrecomp.h"
#include "ExpiredMenu.h"
#include "../App.h"
#include "Entity/EntityUtils.h"
#include "Entity/CustomInputComponent.h"

void ExpiredMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
}


Entity * ExpiredMenuCreate(Entity *pParentEnt)
{

	Entity *pBG = CreateOverlayEntity(pParentEnt, "ExpiredMenu", "interface/main_bg.rttex", 0,0);
	AddFocusIfNeeded(pBG);

	//for android, so the back key (or escape on windows) will quit out of the game
	EntityComponent *pComp = pBG->AddComponent(new CustomInputComponent);
	//tell the component which key has to be hit for it to be activated
	pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));

	CreateTextLabelEntity(pBG, "text", 20, 100, "This beta has expired.\n\nPlease visit www.codedojo.com to see if there\nis a new one.");
	return pBG;
}