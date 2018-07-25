//  ***************************************************************
//  TemplateComponent - Creation date: ?/?/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TemplateComponent_h__
#define TemplateComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
//#include "Renderer/SurfaceAnim.h"

class TemplateComponent: public EntityComponent
{
public:
	TemplateComponent();
	virtual ~TemplateComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);

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

};

#endif // TemplateComponent_h__