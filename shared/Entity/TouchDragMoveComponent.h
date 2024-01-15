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

//IF this is added, this entity can be dragged around the screen in 2d space.  It requires a TouchMoveComponent to exist on the component as well and will
//assert if it isn't found.

//this is set to only work with fingerID = (first touch, or left mouse button), but a way to set this could be added...


//Note, to not show the coordinates when moving, do:
//pTouchDragMove->GetVar("showCoords")->Set(uint32(0));


class TouchDragMoveComponent : public EntityComponent
{
public:
	TouchDragMoveComponent();
	virtual ~TouchDragMoveComponent();

	virtual void OnAdd(Entity* pEnt);
	void OnScaleChanged(Variant* pDataObject);
	virtual void OnRemove();


	void OnTouchDragUpdate(VariantList* pVList);
	void OnOverStart(VariantList* pVList);
	void OnOverEnd(VariantList* pVList);
	void ResetTouch();
	CL_Vec2f* m_pPos2d = NULL;
	CL_Vec2f* m_pSize2d = NULL;

	bool m_bIsDraggingLook = false;


protected:

	bool m_bIgnoreNextMove = false;
	void UpdateStatusMessage(string msg);

};
