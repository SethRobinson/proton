//  ***************************************************************
//  CustomInputComponent - Creation date: 10/04/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef CustomInputComponent_h__
#define CustomInputComponent_h__

#include "Entity/Component.h"

/**
 * A component that listens to specific keyboard input.
 *
 * The name of the component is initially set to "CustomInput".
 *
 * When this component is attached to an \c Entity it listens to keyboard
 * input and sends signals when specified keys are pressed and released.
 * When a key is pressed the "OnActivated" signal is sent. When a key is
 * released the "OnReleased" signal is sent. The variant list passed as a
 * parameter for both of these signals is empty.
 *
 * The following named variants are used inside the component itself:
 * - <b>"keys" (string):</b> specifies a set of characters that are used to
 *   filter the pressed/released keys. If the pressed/released key is found from
 *   this value then the key is accepted and the corresponding signal is sent.
 *   The default value is an empty string.
 * - <b>"keycode" (uint32):</b> specifies a single keycode that is used to
 *   filter the pressed/released keys. If the pressed/releasesd key's code is
 *   the same as this value then the key is accepted and the corresponding signal
 *   is sent. The default value is 0 (which is not a valid keycode).
 * - <b>"disabled" (uint32):</b> used to disable the component. 0 (the default) means
 *   the component is enabled, any other values disable it.
 *
 * The pressed/released keys are filtered with the "keys" and "keycode" variants. If
 * the pressed/released key is found in either of these variants the corresponding
 * signal is sent. If the pressed/released key is not in these variants it is silently
 * ignored. However if both of these variants are at their default values then
 * no filtering happens and all key presses/releases are signaled.
 *
 * \note This component attaches to the "OnInput" and "OnRawInput" functions of the
 * parent \c Entity. \c FocusInputComponent is a component that offers such functions.
 * You may want to use these two components together.
 * \see FocusInputComponent
 */
class CustomInputComponent: public EntityComponent
{
public:
	CustomInputComponent();
	virtual ~CustomInputComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:
	string * m_pKeys;
	bool isKeyAcceptable(uint32 keycode) const;
	void OnInput(VariantList *pVList);
	void OnInputRaw(VariantList *pVList);
	void OnActivated();
	void OnReleased();
	uint32 *m_pDisabled;
	uint32 *m_pKeyCode;

};

#endif // CustomInputComponent_h__
