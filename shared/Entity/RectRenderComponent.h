//  ***************************************************************
//  RectRenderComponent - Creation date: 07/14/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef RectRenderComponent_h__
#define RectRenderComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/SurfaceAnim.h"

class VisualTrailHelper;

class RectRenderComponent: public EntityComponent
{
public:
	RectRenderComponent();
	virtual ~RectRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	enum eVisualStyle
	{
		STYLE_NORMAL, //filled rect
			STYLE_3D, //filled with crappy 3d effect
			STYLE_BORDER_ONLY
	};

private:

	void OnRender(VariantList *pVList);
	void OnUpdateBmpBorderFileName(Variant *pVariant);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	uint32 *m_pBorderColor;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	uint32 *m_pVisualStyle;
	string *m_pBmpBorderFileName; //if set, we'll do a fancy bitmap border
	SurfaceAnim *m_pSurf; //only used it using bmp borders

};

#endif // RectRenderComponent_h__