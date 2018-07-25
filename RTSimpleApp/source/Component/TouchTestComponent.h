//  ***************************************************************
//  TouchTestComponent - Creation date: 1/5/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TouchTestComponent_h__
#define TouchTestComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
//#include "Renderer/SurfaceAnim.h"

#define MAX_TOUCHES_AT_ONCE 12 //iPad can do 11, right?  Well, whatever, let's do this to be safe.

class TouchObject
{
public:

	TouchObject()
	{
		m_bActive = false;
	}

	CL_Vec2f m_vPos;
	bool m_bActive;
	uint32 m_color;
};

class TouchTestComponent: public EntityComponent
{
public:
	TouchTestComponent();
	virtual ~TouchTestComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void OnInput( VariantList *pVList );
	void DrawTouch(uint32 touchID, CL_Vec2f vPos);
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

	TouchObject m_touch[MAX_TOUCHES_AT_ONCE];

};

#endif // TouchTestComponent_h__