//  ***************************************************************
//  FPSControlComponent - Creation date: ?/?/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FPSControlComponent_h__
#define FPSControlComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "../dink/dink.h"

#define C_FLING_JOYSTICK_Y (520+50)
class FPSControlComponent: public EntityComponent
{
public:
	FPSControlComponent();
	virtual ~FPSControlComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void OnStripUpdate(VariantList *pVList);
	void OnOverStart(VariantList *pVList);
	void OnOverEnd(VariantList *pVList);
	void ClearKeyInput();
	void ProcessLastJoystickReading();
	void ProcessArrowInput(CL_Vec2f vDir);
	void OnKillingControls(VariantList *pVList);
	void OnArcadeInput(VariantList *pVList);
	CL_Vec2f *m_pPos2d;

	/*
	CL_Vec2f *m_pSize2d;
	float *m_pScale;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/
	Entity *m_pArrowEnt;
	Entity *m_pCenterBall; //will be a child of the arrowEnt

	CL_Vec2f m_lastTouchDir;
	
	unsigned int m_timeOfLastTouch;
	bool m_bTouchingArrows;
	CL_Vec2f m_vArrowImageSizeOver2;
	float m_arrowMinTransparency;
	float m_arrowMaxTransparency;

};

void SendKey(eDinkInput key, bool bIsDown);

#endif // FPSControlComponent_h__