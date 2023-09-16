//  ***************************************************************
//  TouchDragMoveComponent - Creation date: 8/31/2023 8:51:38 AM
//  -------------------------------------------------------------
//  License: Uh, check for license.txt or license.md for that?
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include "Component.h"

class TouchDragMoveComponent : public EntityComponent
{
public:
	TouchDragMoveComponent();
	virtual ~TouchDragMoveComponent();

	virtual void OnAdd(Entity* pEnt);
	virtual void OnRemove();


	void OnTouchDragUpdate(VariantList* pVList);
	void OnOverStart(VariantList* pVList);
	void OnOverEnd(VariantList* pVList);

	CL_Vec2f* m_pPos2d = NULL;
	CL_Vec2f* m_pSize2d = NULL;

	bool m_bIsDraggingLook = false;

protected:

	void UpdateStatusMessage(string msg);

};
