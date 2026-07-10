#pragma once

#ifdef PLATFORM_OSX

#include "Gamepad.h"
#include <vector>

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
	void QueueButton(bool bDown, int buttonID);

	void* m_pGCController; // GCController* stored as void* for non-ObjC translation units

	//button changes reported by GCController's handler blocks, held until Update()
	//runs inside the game tick. OnButton() fires game signals synchronously, and
	//firing them from the dispatch callback runs game code (menu creation, texture
	//loads) outside the update/draw cycle where no GL context is current.
	struct ButtonEvent { bool m_bDown; int m_buttonID; };
	std::vector<ButtonEvent> m_pendingButtonEvents;
};

#endif // PLATFORM_OSX
