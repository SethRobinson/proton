//  ***************************************************************
//  ActionButtonComponent - Creation date: 3/15/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ActionButtonComponent_h__
#define ActionButtonComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Renderer/SurfaceAnim.h"

class ActionButtonComponent: public EntityComponent
{
public:
	ActionButtonComponent();
	virtual ~ActionButtonComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();


	enum eMode
	{
		MODE_MAGIC,
		MODE_WEAPON
	};

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	bool IsMagic() {return m_mode == MODE_MAGIC;}
	void UpdateIcon();
	CL_Vec2f *m_pPos2d;

	/*
	CL_Vec2f *m_pSize2d;
	float *m_pScale;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/
	float *m_pAlpha;

	eMode m_mode;
};

Entity * CreateActionButtonEntity(Entity *pParentEnt, string name, string fileName, float x, float y);

#endif // ActionButtonComponent_h__