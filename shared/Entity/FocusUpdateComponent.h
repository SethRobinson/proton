//  ***************************************************************
//  FocusUpdateComponent - Creation date: 04/14/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FocusUpdateComponent_h__
#define FocusUpdateComponent_h__

#include "Component.h"
class Entity;

class FocusUpdateComponent: public EntityComponent
{
public:
	FocusUpdateComponent();
	virtual ~FocusUpdateComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	void OnUpdate(VariantList *pVList);
	//our stuff

private:


};

#endif // FocusUpdateComponent_h__