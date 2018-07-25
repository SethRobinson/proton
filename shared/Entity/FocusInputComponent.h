//  ***************************************************************
//  FocusInputComponent - Creation date: 04/14/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FocusInputComponent_h__
#define FocusInputComponent_h__

#include "Component.h"

class Entity;

/**
 * A component that reacts to keyboard input and passes the events to a tree of \link Entity <tt>Entities</tt>\endlink.
 *
 * The name of the component is initially set to "FocusInput".
 *
 * This component connects to the \c BaseApp::m_sig_input signal and calls the "OnInput"
 * function of the parent \c Entity recursively.
 *
 * The following named variants are used inside the component itself:
 * - <b>"mode" (uint32):</b> this holds a value from the \link FocusInputComponent::eMode
 *   \c eMode \endlink enum. If mode is \link FocusInputComponent::MODE_START_NORMAL
 *   \c MODE_START_NORMAL \endlink (the default) then the \c BaseApp::m_sig_input signal
 *   is automatically connected when this component is attached to an \c Entity. If mode
 *   is \link FocusInputComponent::MODE_START_NONE \c MODE_START_NONE \endlink then the
 *   signal is not connected.
 *
 * \c FocusInputComponent can also listen to two other signals from \c BaseApp but neither
 * of these are automatically connected.
 * - BaseApp::m_sig_input_move signal can be connected by calling \c LinkMoveMessages().
 *   This signal causes the "OnInput" function of the parent \c Entity to be called recursively.
 * - BaseApp::m_sig_raw_keyboard signal can be connected by calling \c LinkRawMessages().
 *   This signal causes the "OnRawInput" function of the parent \c Entity to be called recursively.
 *
 * For all the functions of the parent \c Entity that this component calls
 * the same parameters are passed that the original signal from \c BaseApp sends.
 */
class FocusInputComponent: public EntityComponent
{
public:
	FocusInputComponent();
	virtual ~FocusInputComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	void OnInput(VariantList *pVList);
	void OnInputRaw(VariantList *pVList);
	/**
	 * Starts listening to the \c BaseApp::m_sig_input_move signal.
	 *
	 * Only connects the signal if the input mode is \link
	 * FocusInputComponent::INPUT_MODE_SEPARATE_MOVE_TOUCHES <tt>INPUT_MODE_SEPARATE_MOVE_TOUCHES</tt>\endlink.
	 *
	 * This method is also available through a function object called "LinkMoveMessages".
	 *
	 * \see BaseApp::GetInputMode()
	 */
	void LinkMoveMessages(VariantList *pVList);
	/**
	 * Starts listening to the \c BaseApp::m_sig_raw_keyboard signal.
	 *
	 * This method is also available through a function object called "LinkRawMessages".
	 */
	void LinkRawMessages(VariantList *pVList);
	//our stuff

	enum eMode
	{
		MODE_START_NORMAL,
		MODE_START_NONE  //don't actually wire to receive any messages, useful if you want to hand-wire what it will catch later
	};

private:


};

#endif // FocusInputComponent_h__
