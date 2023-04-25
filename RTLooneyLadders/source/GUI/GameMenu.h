#ifndef Game_h__
#define Game_h__

#include "App.h"
Entity * GameMenuCreate(Entity *pParentEnt);
#include "Component/BuildingComponent.h"
void OnGotDoor();
int OnModLivesLost(int mod);
int OnModDyno(int mod);
void ShowQuickMessage(string msg);
void AttachGamepadsIfPossible();
#endif // Game_h__