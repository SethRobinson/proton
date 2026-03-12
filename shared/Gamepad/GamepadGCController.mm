#include "PlatformPrecomp.h"

#ifdef PLATFORM_OSX

#include "GamepadGCController.h"
#include "GamepadManager.h"

GamepadGCController::GamepadGCController()
{
	m_pGCController = nullptr;
}

GamepadGCController::~GamepadGCController()
{
	Kill();
}

bool GamepadGCController::Init()
{
	m_axisUsedCount = 6;
	m_buttonsUsedCount = 16;
	m_name = "GCController";
	return true;
}

void GamepadGCController::Kill()
{
	if (m_pGCController)
	{
		GCController* controller = (__bridge GCController*)m_pGCController;
		(void)controller;
		m_pGCController = nullptr;
	}
}

void GamepadGCController::Update()
{
	Gamepad::Update();
}

void GamepadGCController::InitWithGCController(GCController* controller)
{
	m_pGCController = (__bridge void*)controller;

	const char* pName = [controller.vendorName UTF8String];
	if (pName)
	{
		LogMsg("GCController detected: %s", pName);
		m_name = pName;
	}

	// Same button mapping as GamepadSDL2 Windows path
	SetRightStickAxis(2, 3);

	m_buttons[SDL_CONTROLLER_BUTTON_A].m_virtualKey            = VIRTUAL_DPAD_BUTTON_DOWN;
	m_buttons[SDL_CONTROLLER_BUTTON_B].m_virtualKey            = VIRTUAL_DPAD_BUTTON_RIGHT;
	m_buttons[SDL_CONTROLLER_BUTTON_X].m_virtualKey            = VIRTUAL_DPAD_BUTTON_LEFT;
	m_buttons[SDL_CONTROLLER_BUTTON_Y].m_virtualKey            = VIRTUAL_DPAD_BUTTON_UP;
	m_buttons[SDL_CONTROLLER_BUTTON_LEFTSHOULDER].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
	m_buttons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].m_virtualKey= VIRTUAL_DPAD_RBUTTON;
	m_buttons[SDL_CONTROLLER_BUTTON_BACK].m_virtualKey         = VIRTUAL_DPAD_SELECT;
	m_buttons[SDL_CONTROLLER_BUTTON_START].m_virtualKey        = VIRTUAL_DPAD_START;
	m_buttons[SDL_CONTROLLER_BUTTON_GUIDE].m_virtualKey        = VIRTUAL_DPAD_MENU;
	m_buttons[SDL_CONTROLLER_BUTTON_LEFTSTICK].m_virtualKey    = VIRTUAL_JOYSTICK_BUTTON_LEFT;
	m_buttons[SDL_CONTROLLER_BUTTON_RIGHTSTICK].m_virtualKey   = VIRTUAL_JOYSTICK_BUTTON_RIGHT;
	m_buttons[SDL_CONTROLLER_BUTTON_DPAD_UP].m_virtualKey      = VIRTUAL_KEY_DIR_UP;
	m_buttons[SDL_CONTROLLER_BUTTON_DPAD_DOWN].m_virtualKey    = VIRTUAL_KEY_DIR_DOWN;
	m_buttons[SDL_CONTROLLER_BUTTON_DPAD_LEFT].m_virtualKey    = VIRTUAL_KEY_DIR_LEFT;
	m_buttons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT].m_virtualKey   = VIRTUAL_KEY_DIR_RIGHT;

	GCExtendedGamepad* pad = controller.extendedGamepad;
	if (!pad)
	{
		LogMsg("GCController has no extendedGamepad profile, ignoring");
		return;
	}

	// Capture a weak ref to self for the block
	GamepadGCController* self = this;

	pad.buttonA.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_A);
	};
	pad.buttonB.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_B);
	};
	pad.buttonX.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_X);
	};
	pad.buttonY.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_Y);
	};
	pad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
	};
	pad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
	};
	pad.leftThumbstickButton.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_LEFTSTICK);
	};
	pad.rightThumbstickButton.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
	};
	pad.buttonOptions.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_BACK);
	};
	pad.buttonMenu.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_START);
	};

	// D-pad
	pad.dpad.up.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_DPAD_UP);
	};
	pad.dpad.down.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	};
	pad.dpad.left.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	};
	pad.dpad.right.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
	};

	// Left stick
	pad.leftThumbstick.xAxis.valueChangedHandler = ^(GCControllerAxisInput*, float value) {
		self->SetAxis(0, value);
	};
	pad.leftThumbstick.yAxis.valueChangedHandler = ^(GCControllerAxisInput*, float value) {
		self->SetAxis(1, -value); // GCController Y is inverted vs SDL convention
	};

	// Right stick
	pad.rightThumbstick.xAxis.valueChangedHandler = ^(GCControllerAxisInput*, float value) {
		self->SetAxis(2, value);
	};
	pad.rightThumbstick.yAxis.valueChangedHandler = ^(GCControllerAxisInput*, float value) {
		self->SetAxis(3, -value);
	};

	// Triggers (axes 4 and 5)
	pad.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL) {
		self->SetAxis(4, value);
	};
	pad.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL) {
		self->SetAxis(5, value);
	};
}

#endif // PLATFORM_OSX
