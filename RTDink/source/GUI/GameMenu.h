#ifndef Game_h__
#define Game_h__

#include "../App.h"

Entity * GameCreate(Entity *pParentEnt, int gameIDToLoad, string stateToLoad, string msgToShow = "");
void KillControls(float fadeTimeMS = 300);
void BuildControls(float fadeTimeMS = 300);
void UpdateControlsGUIIfNeeded();
void ShowQuickMessage(string msg);
void GameOnSelect(VariantList *pVList);
CL_Vec2f FlipXIfNeeded(CL_Vec2f vPos);
float FlipXIfNeeded(float x);
bool IsInFlingMode();

#endif // Game_h__