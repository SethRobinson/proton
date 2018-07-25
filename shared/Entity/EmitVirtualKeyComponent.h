//  ***************************************************************
//  EmitVirtualKeyComponent - Creation date: 12/27/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*
If attached to an entity that also has a TouchHandlerComponent, pressing the button will emit this virtual key.
(both an up and down message)

The idea is to add support for other kinds of things if needed later.  Like, it could check for a "DetectShakeComponent" and
emit instead of just TouchHandler.  Possibly I should add a mode so it will respond to only a Button2D's OnButtonSelected as well.

*/

#ifndef EmitVirtualKeyComponent_h__
#define EmitVirtualKeyComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
//#include "Renderer/SurfaceAnim.h"

class EmitVirtualKeyComponent: public EntityComponent
{
public:
	EmitVirtualKeyComponent();
	virtual ~EmitVirtualKeyComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnTouchStart(VariantList *pVList);
	void OnTouchEnd(VariantList *pVList);

	uint32 *m_pKeyCode;
};

#endif // EmitVirtualKeyComponent_h__