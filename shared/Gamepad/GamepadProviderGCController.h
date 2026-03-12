#pragma once

#ifdef PLATFORM_OSX

#include "GamepadProvider.h"

class GamepadProviderGCController : public GamepadProvider
{
public:
	GamepadProviderGCController();
	virtual ~GamepadProviderGCController();

	virtual string GetName() { return "GCController"; }
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

protected:
	void AddControllerByIndex(int index);

	void* m_pConnectObserver;    // id<NSObject> stored as void*
	void* m_pDisconnectObserver; // id<NSObject> stored as void*
};

#endif // PLATFORM_OSX
