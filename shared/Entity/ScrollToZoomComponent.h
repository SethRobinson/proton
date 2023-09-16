//  ***************************************************************
//  ScrollToZoomComponent - Creation date: 9/13/2023 2:28:23 PM
//  -------------------------------------------------------------
//  License: Uh, check for license.txt or license.md for that?
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include "Component.h"

class ScrollToZoomComponent : public EntityComponent
{
public:
	ScrollToZoomComponent();
	virtual ~ScrollToZoomComponent();

	virtual void OnAdd(Entity* pEnt);
	virtual void OnRemove();

	void OnInput(VariantList* pVList);
	void OnTouchDragUpdate(VariantList* pVList);
	void OnOverStart(VariantList* pVList);
	void OnOverEnd(VariantList* pVList);

	CL_Vec2f* m_pPos2d = NULL;
	CL_Vec2f* m_pSize2d = NULL;
	CL_Vec2f* m_pScale2d = NULL;

	bool m_bIsDraggingLook = false;

protected:

	void UpdateStatusMessage(string msg);

};

