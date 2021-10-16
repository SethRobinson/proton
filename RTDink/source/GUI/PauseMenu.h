#ifndef PauseMenu_h__
#define PauseMenu_h__

#include "../App.h"

Entity * PauseMenuCreate(Entity *pParentEnt);
Entity * DinkQuitGame(); //kills dink and loads the appropriate last menu and returns a pointer to it
void PlayMenuMusic();
Entity * DinkRestartGame();
Entity * DinkQuitGame();
#endif // PauseMenu_h__