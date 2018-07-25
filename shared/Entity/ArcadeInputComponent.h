//  ***************************************************************
//  ArcadeInputComponent - Creation date: 11/8/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//This component helps convert input from the various kinds of input devices into things a game can use, such as directional presses
//and releases.  It can also do rudimentry keybinding.

//It internally converts android trackball movements into fake directional key presses.  Used with GamepadManager, it can also
//route gamepad/joystick input as direction keys too,

//Check RTLooneyLadders for a working example.

/*

Example of usage:

EntityComponent *pComp = pIcon->AddComponent(new ArcadeInputComponent);
AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP, false);
AddKeyBinding(pComp, "Unlock", 'U', 'U', false);

//route messages to the entity you want to have them:
GetBaseApp()->m_sig_arcade_input.connect(pBG->GetFunction("OnArcadeInput")->sig_function);
//connect that to a function
pBG->GetShared()->GetFunction("OnArcadeInput")->sig_function.connect(&OnSelectInput);




//another example, but overriding things so the messages are not broadcast through m_sig_arcade, but to a specific entity

//this example is how I did it from inside a component:
EntityComponent *pComp = GetParent()->AddComponent(new ArcadeInputComponent);
//these arrow keys will be triggered by the keyboard, if applicable
AddKeyBinding(pComp, "Fire", VIRTUAL_KEY_CONTROL, VIRTUAL_KEY_GAME_FIRE);
pComp->GetFunction("SetOutputEntity")->sig_function(&VariantList(GetParent())); //redirect its messages to our parent entity, will call OnArcadeInput
GetParent()->GetFunction("OnArcadeInput")->sig_function.connect(1, boost::bind(&VehicleControlComponent::OnArcadeInput, this, _1));	


//If you want to delete a group of specific bindings, you can do it like this:

VariantList vList((string)"Keyboard");

//get the component, wherever it is, this would depend on the specific app
GetEntityRoot()->GetComponentByName("ArcadeInput")>GetFunction("RemoveKeyBindingsStartingWith")->sig_function(&vList);

*/


#ifndef ArcadeInputComponent_h__
#define ArcadeInputComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Manager/MessageManager.h"

enum eMoveButtonDir
{
	MOVE_BUTTON_DIR_LEFT = 0,
	MOVE_BUTTON_DIR_RIGHT,
	MOVE_BUTTON_DIR_UP,
	MOVE_BUTTON_DIR_DOWN,
	
	MOVE_BUTTON_DIR_COUNT
};


class MoveButtonState
{
public:

	MoveButtonState()
	{
		m_bIsDown = false;
		m_releaseTimer = 0;
	}

	void SetKeyType(eVirtualKeys type)
	{
		m_keyType = type;
	}

	void ReleaseIfNeeded(boost::signal<void (VariantList*)> *pCustomSignal, bool bSendChange = true);
	void OnPress(int releaseTime, boost::signal<void (VariantList*)> *pCustomSignal, bool bSendChange = true);

	void Update(boost::signal<void (VariantList*)> *pCustomSignal);
	void OnPressToggle(bool bDown, boost::signal<void (VariantList*)> *pCustomSignal, bool bSendChange = true);
	bool m_bIsDown;
	unsigned int m_releaseTimer;
	eVirtualKeys m_keyType;
};


class ArcadeKeyBind
{
public:

	ArcadeKeyBind()
	{
		m_inputkeycode = 0;
		m_outputkeycode = 0;
		m_bOutputAsNormalKeyToo = false;
		m_keyModifiersRequired = 0;
	}

	string m_name;
	uint32 m_inputkeycode;
	uint32 m_outputkeycode;

	bool m_bOutputAsNormalKeyToo; //if true, will through the normal sig_raw_keyboard in addition to m_sig_arcade_input, this
	//can be useful to say, let a gamepad mapping also trigger normal keys, which buttons may be hooked to
	
	uint32 m_keyModifiersRequired; //VIRTUAL_KEY_MODIFIER_CONTROL, VIRTUAL_KEY_MODIFIER_SHIFT and VIRTUAL_KEY_MODIFIER_ALT bits

};

typedef list<ArcadeKeyBind> ArcadeBindList;
class ArcadeInputComponent: public EntityComponent
{

public:
	ArcadeInputComponent();
	virtual ~ArcadeInputComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	bool GetDirectionKeysAsVector(CL_Vec2f *pVecOut);
	bool GetDirectionKeys(bool &bLeftOut, bool &bRightOut, bool &bUpOut, bool &bDownOut);

	void SetDirectionKey(eMoveButtonDir moveDir, bool bPressed, bool bBroadcastKeyIfChanged = true);

	void ResetDirectionKeys(bool bBroadcastKeyIfChanged);

	enum TrackballMode
	{
		TRACKBALL_MODE_WALKING, //default, suitable for moving around a character, sends up/down notifications to mimic arrow key directions
		TRACKBALL_MODE_MENU_SELECTION //suitable for moving up/down menus, sends an up/down notification every 1.0f of notches are reached
	};

	void OnRawKeyboard(VariantList *pVList); //can call or signal bind to this to feed it more keys if needed

private:

	void OnUpdate(VariantList *pVList);
	void OnTrackball(VariantList *pVList);
	void AddKeyBinding(VariantList *pVList);
	void ActivateBinding(ArcadeKeyBind *pBind, bool bDown);
	void OnTrackballModeChanged(Variant *pVar);
	void OnCustomOutputRemoved(Entity *pEnt);
	void SetOutput(VariantList *pVList);
	void RemoveKeyBindingsStartingWith(VariantList *pVList);
	
CL_Vec2f *m_pPos2d;
	/*
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/

	MoveButtonState m_buttons[MOVE_BUTTON_DIR_COUNT];
	ArcadeBindList m_bindings;
	uint32 *m_pTrackballMode;
	CL_Vec2f m_trackball;
	
	boost::signal<void (VariantList*)> *m_customSignal; //if not null, messages will be sent here
};

void AddKeyBinding(EntityComponent *pComp, string name, uint32 inputcode, uint32 outputcode, bool bAlsoSendAsNormalRawKey = false, uint32 modifiersRequired = 0);
string ProtonVirtualKeyToString(eVirtualKeys vKey); //stupid helper function that should probably be somewhere else
#endif // ArcadeInputComponent_h__