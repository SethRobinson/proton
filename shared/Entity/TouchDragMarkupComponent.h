//  ***************************************************************
//  TouchDragMarkupComponent - Creation date: 1/10/2024
//  -------------------------------------------------------------
//  License: Uh, check for license.txt or license.md for that?
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include "Component.h"
#include "Renderer/SoftSurface.h"
#include "Entity/OverlayRenderComponent.h"

//IF this is added, this entity can be dragged around the screen in 2d space.  It requires a TouchMoveComponent to exist on the component as well and will
//assert if it isn't found.

//this is set to only work with fingerID = (first touch, or left mouse button), but a way to set this could be added...

class TouchDragMarkupComponent : public EntityComponent
{
public:
	TouchDragMarkupComponent();
	virtual ~TouchDragMarkupComponent();

	virtual void OnAdd(Entity* pEnt);
	void OnRender(VariantList* pVList);
	virtual void OnRemove();

	CL_Vec2f* m_pPos2d = NULL;
	CL_Vec2f* m_pSize2d = NULL;
	CL_Vec2f* m_pScale2d = NULL;

	bool m_bIsDraggingLook = false;

	const int m_fingerIDToTrack = 1;
	CL_Vec2f m_lastPosDrawn;
	
protected:

	void OnEntityToFollowChanged(Variant* pDataObject);

	void OnEntityWeFollowGotDeleted(VariantList* pVList);

	void OnUpdate(VariantList* pVList);
	void OnScaleChanged(Variant* pDataObject);
	void OnSizeChanged(Variant* pDataObject);
	void ResetTouch();
	void InitMarkupBoard();
	void UpdateStatusMessage(string msg);
	void Draw(CL_Vec2f pPos);

	void OnTouchDragUpdate(VariantList* pVList);
	void OnOverStart(VariantList* pVList);
	void OnOverEnd(VariantList* pVList);
	void OnClearActiveMarkups(VariantList* pVList);

	OverlayRenderComponent* m_pOverlayComp;
	SoftSurface m_softSurf;
	SurfaceAnim m_surface;
	CL_Vec2f m_startPos;

	bool m_bIgnoreNextMove = false;

	Variant *m_pEntityToFollow = NULL;

};

void SetGlobalMarkupPenSize(int size);
int GetGlobalMarkupPenSize();