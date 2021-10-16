//  ***************************************************************
//  InventoryComponent - Creation date: ?/?/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef InventoryComponent_h__
#define InventoryComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "../dink/dink.h"

class InventoryComponent: public EntityComponent
{
public:
	InventoryComponent();
	virtual ~InventoryComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);

	void OnInput( VariantList *pVList );
	void OnUpdatePos(CL_Vec2f vPos);
	CL_Vec2f *m_pPos2d;
	int m_activeFinger;

	Entity *m_pArrowEnt;
	
	bool m_bGotFirstClick;

};

#endif // InventoryComponent_h__