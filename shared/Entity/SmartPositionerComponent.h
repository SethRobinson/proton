

//  ***************************************************************
//  SmartPositionerComponent - Creation date: 04/09/2018
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2018 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef SmartPositionerComponent_h__
#define SmartPositionerComponent_h__

#include "Component.h"

class Entity;

class SmartPositionerComponent : public EntityComponent
{
public:
	SmartPositionerComponent();
	virtual ~SmartPositionerComponent();
	
	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	
	void SetSmartScale(CL_Vec2f vScalePercents, eAspect aspectMode);

	void OnScaleScreenPercentChanged(Variant *pVariant);
	void OnPosScreenPercentChanged(Variant *pVariant);
	void OnScreenSizeChanged();

protected:
	

private:

	void CalculatePosition();

	CL_Vec2f *m_pPosScreenPercent2D;
	CL_Vec2f *m_pScaleScreenPercent2D;
	uint32 *m_pAlignment;
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	bool m_bSetSize;


	uint32 *m_pAspectMode; //a eAspectMode enum
};

SmartPositionerComponent * AddSmartPositioner(Entity *pEnt, eAlignment align, float xPercent, float yPercent);
#endif // SmartPositionerComponent_h__
