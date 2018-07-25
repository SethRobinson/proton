//  ***************************************************************
//  TextRenderComponent - Creation date: 04/11/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TextRenderComponent_h__
#define TextRenderComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/Surface.h"

class TextRenderComponent: public EntityComponent
{
public:
	TextRenderComponent();
	virtual ~TextRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	enum eVisualStyle
	{
		STYLE_NORMAL,
		STYLE_EFFECT_SIN_WAVE
	};

private:

	void OnRender(VariantList *pVList);
	void OnTextChanged(Variant *pDataObject);
	void OnScaleChanged(Variant *pDataObject);
	void OnFontChanged(Variant *pDataObject);

	void RenderAsWave(CL_Vec2f vPos, uint32 color);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	string *m_pText;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pAlignment;
	uint32 *m_pFontID;
	uint32 *m_pStyle;
	float *m_pEffectPower;
	float *m_pRotation; //in degrees
	uint32 *m_pVisible;
	uint32 *m_pDisabled;
	uint32 *m_pShadowColor; //if not 0,0,0,0, will render a shadow behind the text
};

#endif // TextRenderComponent_h__