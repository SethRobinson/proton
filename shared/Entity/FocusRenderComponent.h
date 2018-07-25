//  ***************************************************************
//  FocusRenderComponent - Creation date: 04/14/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FocusRenderComponent_h__
#define FocusRenderComponent_h__

#include "Component.h"

class Entity;

class FocusRenderComponent: public EntityComponent
{
public:
	FocusRenderComponent();
	virtual ~FocusRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);

};

#endif // FocusRenderComponent_h__