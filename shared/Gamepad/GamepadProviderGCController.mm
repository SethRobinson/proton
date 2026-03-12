#include "PlatformPrecomp.h"

#ifdef PLATFORM_OSX

#include "GamepadProviderGCController.h"
#include "GamepadGCController.h"
#include "GamepadManager.h"

#import <GameController/GameController.h>

// Use the GCController pointer address as a stable unique ID
static eGamepadID IDFromController(GCController* c)
{
	return (eGamepadID)(uintptr_t)(void*)c;
}

GamepadProviderGCController::GamepadProviderGCController()
	: m_pConnectObserver(nullptr)
	, m_pDisconnectObserver(nullptr)
{
}

GamepadProviderGCController::~GamepadProviderGCController()
{
	Kill();
}

void GamepadProviderGCController::AddControllerByIndex(int index)
{
	NSArray<GCController*>* controllers = [GCController controllers];
	if (index < 0 || index >= (int)[controllers count])
		return;

	GCController* controller = controllers[index];

	if (!controller.extendedGamepad)
	{
		LogMsg("GCController at index %d has no extendedGamepad, skipping", index);
		return;
	}

	eGamepadID uid = IDFromController(controller);

	if (GetGamepadManager()->GetGamepadByUniqueID(uid))
	{
		LogMsg("GCController already registered, ignoring");
		return;
	}

	GamepadGCController* pPad = new GamepadGCController();
	pPad->SetProvider(this);
	pPad->Init();
	GetGamepadManager()->AddGamepad(pPad, uid);
	pPad->InitWithGCController(controller);
}

bool GamepadProviderGCController::Init()
{
	LogMsg("Initting GCController gamepad provider");

	// Enumerate already-connected controllers
	NSArray<GCController*>* controllers = [GCController controllers];
	LogMsg("GCController: %d controller(s) already connected", (int)[controllers count]);
	for (int i = 0; i < (int)[controllers count]; i++)
	{
		AddControllerByIndex(i);
	}

	// Watch for future connect/disconnect
	GamepadProviderGCController* self = this;

	id connectObs = [[NSNotificationCenter defaultCenter]
		addObserverForName:GCControllerDidConnectNotification
		object:nil
		queue:[NSOperationQueue mainQueue]
		usingBlock:^(NSNotification* note) {
			GCController* controller = note.object;
			if (!controller.extendedGamepad)
			{
				LogMsg("GCController connected but has no extendedGamepad, skipping");
				return;
			}
			eGamepadID uid = IDFromController(controller);
			if (GetGamepadManager()->GetGamepadByUniqueID(uid))
				return;
			GamepadGCController* pPad = new GamepadGCController();
			pPad->SetProvider(self);
			pPad->Init();
			GetGamepadManager()->AddGamepad(pPad, uid);
			pPad->InitWithGCController(controller);
		}];

	id disconnectObs = [[NSNotificationCenter defaultCenter]
		addObserverForName:GCControllerDidDisconnectNotification
		object:nil
		queue:[NSOperationQueue mainQueue]
		usingBlock:^(NSNotification* note) {
			GCController* controller = note.object;
			eGamepadID uid = IDFromController(controller);
			LogMsg("GCController disconnected, uid %ld", (long)uid);
			GetGamepadManager()->RemoveGamepadByUniqueID(uid);
		}];

	m_pConnectObserver    = (void*)CFRetain((__bridge CFTypeRef)connectObs);
	m_pDisconnectObserver = (void*)CFRetain((__bridge CFTypeRef)disconnectObs);

	return true;
}

void GamepadProviderGCController::Kill()
{
	if (m_pConnectObserver)
	{
		id obs = (__bridge id)m_pConnectObserver;
		[[NSNotificationCenter defaultCenter] removeObserver:obs];
		CFRelease((CFTypeRef)m_pConnectObserver);
		m_pConnectObserver = nullptr;
	}
	if (m_pDisconnectObserver)
	{
		id obs = (__bridge id)m_pDisconnectObserver;
		[[NSNotificationCenter defaultCenter] removeObserver:obs];
		CFRelease((CFTypeRef)m_pDisconnectObserver);
		m_pDisconnectObserver = nullptr;
	}
}

void GamepadProviderGCController::Update()
{
	// GCController callbacks are delivered on the main queue — nothing to poll
}

#endif // PLATFORM_OSX
