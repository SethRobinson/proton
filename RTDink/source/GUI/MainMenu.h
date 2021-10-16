#ifndef MainMenu_h__
#define MainMenu_h__

#include "../App.h" //vita being weird...

#define BACK_BUTTON_Y 293
Entity * MainMenuCreate(Entity *pParentEnt, bool bFadeIn = false);
Entity *  AddTitle( Entity *pEnt, string title);
#endif // MainMenu_h__