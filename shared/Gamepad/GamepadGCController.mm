#include "PlatformPrecomp.h"

#ifdef PLATFORM_OSX

#include "GamepadGCController.h"
#include "GamepadManager.h"

// Button index constants matching SDL_CONTROLLER_BUTTON_* values,
// used as raw indices into m_buttons[]. No SDL headers needed here.
enum {
	GCC_BTN_A             = 0,
	GCC_BTN_B             = 1,
	GCC_BTN_X             = 2,
	GCC_BTN_Y             = 3,
	GCC_BTN_BACK          = 4,
	GCC_BTN_GUIDE         = 5,
	GCC_BTN_START         = 6,
	GCC_BTN_LEFTSTICK     = 7,
	GCC_BTN_RIGHTSTICK    = 8,
	GCC_BTN_LEFTSHOULDER  = 9,
	GCC_BTN_RIGHTSHOULDER = 10,
	GCC_BTN_DPAD_UP       = 11,
	GCC_BTN_DPAD_DOWN     = 12,
	GCC_BTN_DPAD_LEFT     = 13,
	GCC_BTN_DPAD_RIGHT    = 14,
};

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
		m_pGCController = nullptr;
	}
}

void GamepadGCController::Update()
{
	Gamepad::Update();
}

void GamepadGCController::InitWithGCController(GCController* controller)
{
	m_pGCController = (void*)controller;

	const char* pName = [controller.vendorName UTF8String];
	if (pName)
	{
		LogMsg("GCController detected: %s", pName);
		m_name = pName;
	}

	// Same button mapping as GamepadSDL2 Windows path
	SetRightStickAxis(2, 3);

	m_buttons[GCC_BTN_A].m_virtualKey             = VIRTUAL_DPAD_BUTTON_DOWN;
	m_buttons[GCC_BTN_B].m_virtualKey             = VIRTUAL_DPAD_BUTTON_RIGHT;
	m_buttons[GCC_BTN_X].m_virtualKey             = VIRTUAL_DPAD_BUTTON_LEFT;
	m_buttons[GCC_BTN_Y].m_virtualKey             = VIRTUAL_DPAD_BUTTON_UP;
	m_buttons[GCC_BTN_LEFTSHOULDER].m_virtualKey  = VIRTUAL_DPAD_LBUTTON;
	m_buttons[GCC_BTN_RIGHTSHOULDER].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
	m_buttons[GCC_BTN_BACK].m_virtualKey          = VIRTUAL_DPAD_SELECT;
	m_buttons[GCC_BTN_START].m_virtualKey         = VIRTUAL_DPAD_START;
	m_buttons[GCC_BTN_GUIDE].m_virtualKey         = VIRTUAL_DPAD_MENU;
	m_buttons[GCC_BTN_LEFTSTICK].m_virtualKey     = VIRTUAL_JOYSTICK_BUTTON_LEFT;
	m_buttons[GCC_BTN_RIGHTSTICK].m_virtualKey    = VIRTUAL_JOYSTICK_BUTTON_RIGHT;
	m_buttons[GCC_BTN_DPAD_UP].m_virtualKey       = VIRTUAL_KEY_DIR_UP;
	m_buttons[GCC_BTN_DPAD_DOWN].m_virtualKey     = VIRTUAL_KEY_DIR_DOWN;
	m_buttons[GCC_BTN_DPAD_LEFT].m_virtualKey     = VIRTUAL_KEY_DIR_LEFT;
	m_buttons[GCC_BTN_DPAD_RIGHT].m_virtualKey    = VIRTUAL_KEY_DIR_RIGHT;

	GCExtendedGamepad* pad = controller.extendedGamepad;
	if (!pad)
	{
		LogMsg("GCController has no extendedGamepad profile, ignoring");
		return;
	}

	// Capture a weak ref to self for the block
	GamepadGCController* self = this;

	pad.buttonA.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_A);
	};
	pad.buttonB.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_B);
	};
	pad.buttonX.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_X);
	};
	pad.buttonY.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_Y);
	};
	pad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_LEFTSHOULDER);
	};
	pad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_RIGHTSHOULDER);
	};
	pad.leftThumbstickButton.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_LEFTSTICK);
	};
	pad.rightThumbstickButton.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_RIGHTSTICK);
	};
	pad.buttonOptions.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_BACK);
	};
	pad.buttonMenu.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_START);
	};

	// D-pad
	pad.dpad.up.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_DPAD_UP);
	};
	pad.dpad.down.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_DPAD_DOWN);
	};
	pad.dpad.left.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_DPAD_LEFT);
	};
	pad.dpad.right.valueChangedHandler = ^(GCControllerButtonInput*, float, BOOL pressed) {
		self->OnButton(pressed, GCC_BTN_DPAD_RIGHT);
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
