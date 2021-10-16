#ifndef QuickTipMenu_h__
#define QuickTipMenu_h__
#include "PlatformSetup.h"
class Entity;

Entity * CreateQuickTipFirstTimeOnly(Entity *pParentEnt, string tipFileName, bool bRequireMoveMessages);
Entity * CreateQuickTip(Entity *pParentEnt, string tipFileName, bool bRequireMoveMessages);
#endif // QuickTipMenu_h__