//  ***************************************************************
//  InputTextRenderComponent - Creation date: 07/21/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef InputTextRenderComponent_h__
#define InputTextRenderComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/Surface.h"

/*

Single line input and render component.

Example of usage in RTSimpleApp

Can listen to its "hasFocus" variant to know when it gets/loses focus.

Examples of some parms you can change:

//control how many characters the user can enter
pButtonEntity->GetComponentByName("InputTextRender")->GetVar("inputLengthMax")->Set(uint32(18));

//show *'s, password mode
pButtonEntity->GetComponentByName("InputTextRender")->GetVar("visualStyle")->Set((uint32)InputTextRenderComponent::STYLE_PASSWORD);

//Truncate text to fit input box
pButtonEntity->GetComponentByName("InputTextRender")->GetVar("truncateTextIfNeeded")->Set(uint32(1));

To close the keyboard and cause the current thing being edited to lose focus, you can do this:

Entity *pEnt = GetEntityWithNativeUIFocus();
if (pEnt)
{
	pEnt->GetComponentByName("InputTextRender")->GetFunction("CloseKeyboard")->sig_function(NULL);
}

Calling function "ActivateKeyboard" can be used to give it focus and bring up the touch keyboard (if applicable)

*/

class InputTextRenderComponent: public EntityComponent
{
public:
	InputTextRenderComponent();
	virtual ~InputTextRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	enum eVisualStyle
	{
		STYLE_NORMAL,
		STYLE_PASSWORD //shows *'s instead of the real text
	};

	enum eInputType
	{
		INPUT_TYPE_ASCII, //allows letters and numbers, no spaces.  Good for entering names
		INPUT_TYPE_NUMBERS, //numbers only (plus negative symbol) - on an iPhone will bring up a number only keypad
		INPUT_TYPE_URL,
		INPUT_TYPE_ASCII_FULL, //allows things like spaces, commas, ect.  Good for text chat input
		INPUT_TYPE_EMAIL
	};

	enum eInputFiltering
	{
		FILTERING_STRICT, //no spaces or weird symbols allowed
		FILTERING_LOOSE //allows everything that could be a say, a URL
	};
private:

	void OnRender(VariantList *pVList);
	void OnTextChanged(Variant *pDataObject);
	void OnScaleChanged(Variant *pDataObject);
	void OnFontChanged(Variant *pDataObject);
	void ActivateKeyboard(VariantList *pVList);
	void CloseKeyboard(VariantList *pVList);
	void OnTouchEnd(VariantList *pVList);
	void OnTouchStart(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void OnInput( VariantList *pVList );
	void OnLosingNativeGUIFocus(VariantList *pVList);
	void OnEnterForeground(VariantList *pVList);
	void OnEnterBackground(VariantList *pVList);
	void OnVisibilityChanged(Variant *pDataObject);
	void OnVisualStyleChanged(Variant *pDataObject);
	void OnDisabledChanged(Variant *pDataObject);
	string TrimText(string *pText);
	void OnTruncateTextIfNeededChanged(Variant *pDataObject);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pTextOffsetPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pTextSize2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	string *m_pText;
	string *m_pPlaceHolderText;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pAlignment;
	uint32 *m_pFontID;
	uint32 *m_pVisualStyle;
	uint32 *m_pCursorColor;
	uint32 *m_pHasFocus;
	uint32 *m_pInputLengthMax;
	uint32 *m_pBorderColor;
	uint32 *m_pDisabled;
	uint32 *m_pInputType;
	uint32 *m_pFiltering;
	uint32 *m_pVisible;
	uint32 *m_pGetFocusOnEnter; //if true, hitting enter (while no keyboard is on the screen and nothing else has focus) will automatically  give this focus
	uint32 *m_pTruncateTextIfNeeded ; //if true, will truncate text to match screen size, and also limit input
	string m_displayText;
};

#endif // InputTextRenderComponent_h__