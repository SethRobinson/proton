//  ***************************************************************
//  DragControlComponent - Creation date: 4/27/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef DragControlComponent_h__
#define DragControlComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "../dink/dink.h"

class DragUnit
{
public:
	DragUnit(CL_Vec2f vPos, unsigned int time)
	{
		m_vPos = vPos;
		m_timeMade = time;
	}
	CL_Vec2f m_vPos;
	unsigned int m_timeMade; //in game ticks

};

class DragControlComponent: public EntityComponent
{

 
public:
	DragControlComponent();
	virtual ~DragControlComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void OnStripUpdate(VariantList *pVList);
	void SendKey(eDinkInput key, bool bIsDown);
	void OnOverStart(VariantList *pVList);
	void OnOverEnd(VariantList *pVList);
	void OnOverMove(VariantList *pVList);
	void ClearKeyInput();
	void ProcessLastJoystickReading();
	void ProcessArrowInput(CL_Vec2f vDir);
	void OnKillingControls(VariantList *pVList);
	void AddSample(DragUnit v);
	void OnArcadeInput(VariantList *pVList);
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;

	/*
		float *m_pScale;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/
	CL_Vec2f m_lastTouch;

	unsigned int m_timeOfLastTouch;

	std::deque< DragUnit> m_samples;

	float SAMPLE_COUNT;
	float LENGTH_REQUIRED_FOR_MOVE;
};

#endif // DragControlComponent_h__