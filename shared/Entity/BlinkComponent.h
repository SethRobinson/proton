//  ***************************************************************
//  BlinkComponent - Creation date: 7/13/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BlinkComponent_h__
#define BlinkComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
//#include "Renderer/SurfaceAnim.h"

class BlinkComponent: public EntityComponent
{
public:
	BlinkComponent();
	virtual ~BlinkComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnUpdate(VariantList *pVList);

	uint32 *m_pVisible;
	uint32 *m_pBlinkSpeedMS;
	uint32 m_timeOfLastBlink;
};

#endif // BlinkComponent_h__