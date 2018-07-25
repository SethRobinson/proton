//  ***************************************************************
//  ScrollBarRenderComponent - Creation date: 07/08/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ScrollBarRenderComponent_h__
#define ScrollBarRenderComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/SurfaceAnim.h"

/*
Note:  This component is not affected by the RenderScissorComponent.  If you need that for some reason, add that as
an optional setting

If it finds a sister component named "Scroll", it will connect to it to get bounds and progress data.  If it doesn't,
you can manually set the vars in its parent entity to make it perform.

The things it will check is:

pos2d (where the upper left corner of the view area is)
sized2d (size of the window where we can see stuff)
boundsRect (size of the total content minus the display area.  So a bounds of 0, -50, 0, 0 means it can scroll up by 50 pixels.
two to figure out how big the bar should be)

//optional

fileName - set a custom image to use for the scroll bar.  default is interface/scroll_bar_caps.rttex
color - color tinting (defaults to MAKE_RGBA(224,188,130,255), a sort of brown)


*/

class ScrollBarRenderComponent: public EntityComponent
{
public:
	ScrollBarRenderComponent();
	virtual ~ScrollBarRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnUpdate(VariantList *pVList);
	void OnRender(VariantList *pVList);
	void OnTargetOverStart(VariantList *pVList);
	void OnTargetOverEnd(VariantList *pVList);
	void OnBoundsChanged(Variant *pVariant);
	void OnFileNameChanged(Variant *pDataObject);
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	float *m_pAlpha;
	CL_Rectf *m_pBoundsRect;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	CL_Vec2f *m_pProgress2d;
	SurfaceAnim *m_pSurf;
	string *m_pFileName;
	bool m_bUsingScrollComponent;
	
};

#endif // ScrollBarRenderComponent_h__