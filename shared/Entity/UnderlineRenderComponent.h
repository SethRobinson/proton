//  ***************************************************************
//  UnderlineRenderComponent - Creation date: 06/03/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef UnderlineRenderComponent_h__
#define UnderlineRenderComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/Surface.h"

class UnderlineRenderComponent: public EntityComponent
{
public:
	UnderlineRenderComponent();
	virtual ~UnderlineRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	
};

#endif // UnderlineRenderComponent_h__