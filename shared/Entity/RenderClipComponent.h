//  ***************************************************************
//  RenderClipComponent - Creation date: 09/17/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef RenderClipComponent_h__
#define RenderClipComponent_h__

#include "Component.h"

class Entity;

class RenderClipComponent: public EntityComponent
{
public:
	RenderClipComponent();
	virtual ~RenderClipComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	void FilterOnRender(VariantList *pVList);
	void PostOnRender(VariantList *pVList);
	//our stuff

	enum eClipMode
	{
		CLIP_MODE_LEFT,
		CLIP_MODE_RIGHT,
		CLIP_MODE_TOP,
		CLIP_MODE_BOTTOM,
	};


	enum eClipPosition //this is todo, a plan for when I want custom positioning.
	{
		CLIP_POSITION_FROM_SIZE,
		CLIP_POSITION_CUSTOM
	};

private:

	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pPos2d;
	uint32 *m_pAlignment;
	uint32 *m_pClipMode; //todo, right no it just defaults to bottom
};

#endif // RenderClipComponent_h__