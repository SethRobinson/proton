//  ***************************************************************
//  FilterInputComponent - Creation date: 07/31/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FilterInputComponent_h__
#define FilterInputComponent_h__

#include "Component.h"

class Entity;

class FilterInputComponent: public EntityComponent
{
public:
	FilterInputComponent();
	virtual ~FilterInputComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	enum eMode
	{
		MODE_CLIP_INPUT_TO_ABSOLUTE_CLIP_RECT, //to use this, set the "clipRect" var in this component to something. not relative to entity position or alignment
		MODE_CLIP_INPUT_TO_SIZE, //Clip OnInput touch-start and move messages to things in our size, we don't touch other messages because usually.  Respects entity position and alignment.
		//we want to know when the release happens no matter where on the screen their finger is
		MODE_DISABLE_INPUT_ALL, //stop all OnInput messages from reaching this entity and its children
		MODE_DISABLE_INPUT_CHILDREN, //stop all OnInput messages from reaching its children, but handle it itself
		MODE_IDLE, //don't actually do anything
		MODE_CLIP_INPUT_TO_ABSOLUTE_CLIP_RECT_AND_DISABLE_INPUT_CHILDREN,
		MODE_IGNORE_ABSOLUTE_CLIP_RECT,
		MODE_CLIP_INPUT_TO_SIZE_STRICT, //like MODE_CLIP_INPUT_TO_SIZE but also clips release messages
		MODE_CLIP_INPUT_TO_CLIP_RECT //respects the position and alignment of the entity - must set the clipRect var in this component to something.
	};
	void FilterOnInput(VariantList *pVList);
	//our stuff

private:

	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pPos2d;
	uint32 *m_pMode;
	uint32 *m_pAlignment;
	CL_Rectf *m_pClipRect;

};

#endif // FilterInputComponent_h__