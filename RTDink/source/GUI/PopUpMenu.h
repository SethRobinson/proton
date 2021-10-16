#ifndef PopUpMenu_h__
#define PopUpMenu_h__
#include "PlatformSetup.h"
class Entity;
void PopUpCreate(Entity *pEnt, string msg, string url, string button1Action, string button1Label, string button2Action , string button2Label,
				 bool bRequireMoveMessages, string button3Action= "", string button3Label="");
#endif // PopUpMenu_h__
