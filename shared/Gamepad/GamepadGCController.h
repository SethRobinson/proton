#pragma once

#ifdef PLATFORM_OSX

#include "Gamepad.h"

#ifdef __OBJC__
#import <GameController/GameController.h>
#endif

class GamepadGCController : public Gamepad
{
public:
	GamepadGCController();
	virtual ~GamepadGCController();

	virtual bool Init();
	virtual void Kill();
	virtual void Update();

#ifdef __OBJC__
	void InitWithGCController(GCController* controller);
#endif

protected:
	void* m_pGCController; // GCController* stored as void* for non-ObjC translation units
};

#endif // PLATFORM_OSX
