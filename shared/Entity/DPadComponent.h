//  ***************************************************************
//  DPadComponent - Creation date: 12/22/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this class puts a dpad on the screen and has it send CHAR_RAW events that mimic the arrow keys on real keyboards.
//change the component variable "dpad_image" to set the image to load.  default is dpad.rttex
//this dpad is NOT analog - it's just on/off for 8 directions


#ifndef DPadComponent_h__
#define DPadComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"


#define DPAD_MAX_DIRECTIONS 8
#define DPAD_MAX_KEYS 4 //up + left makes a diagonal dir, we're just  mimicking the arrow keys of a keyboard here

class DPADButton
{
public:
	
	DPADButton()
	{
		m_bDown = false;
	}
	void OnButtonChange(int key, bool bDown);

	bool m_bDown;
};

class DPadComponent: public EntityComponent
{
public:
	DPadComponent();
	virtual ~DPadComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	enum eButton
	{
		BUTTON_LEFT,
		BUTTON_RIGHT,
		BUTTON_UP,
		BUTTON_DOWN
	};
	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void OnStripUpdate(VariantList *pVList);
	void OnOverStart(VariantList *pVList);
	void OnOverEnd(VariantList *pVList);
	void ClearKeyInput();
	void ProcessLastJoystickReading();
	void ProcessArrowInput(CL_Vec2f vDir);

	void OnArcadeInput(VariantList *pVList);
	void SendKey(eVirtualKeys key, bool bIsDown);
	void OnScaleChanged(Variant *pVariant);
	void SetupClipRectOnDPad();
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pScale2d;

	Entity *m_pArrowEnt;
	Entity *m_pCenterBall; //will be a child of the arrowEnt
	string *m_pDpadImage;
	float *m_ptouchAreaPadding;
	CL_Vec2f m_lastTouchDir;
	
	unsigned int m_timeOfLastTouch;
	bool m_bTouchingArrows;
	CL_Vec2f m_vArrowImageSizeOver2;
	float m_arrowMinTransparency;
	float m_arrowMaxTransparency;
	DPADButton m_button[DPAD_MAX_KEYS];
	int m_lastDirectionID;

};

#endif // DPadComponent_h__