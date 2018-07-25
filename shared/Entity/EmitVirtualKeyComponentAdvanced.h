//  ***************************************************************
//  EmitVirtualKeyComponentAdvanced - Creation date: 10/24/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*
If attached to an entity that also has a TouchHandlerComponent, pressing the button will emit this virtual key.
(both an up and down message)

This version also claims ownership of the button, as well as responds even if you slide ONTO the button, so good for say, virtual
control pads.

*/

#ifndef EmitVirtualKeyComponentAdvanced_h__
#define EmitVirtualKeyComponentAdvanced_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
//#include "Renderer/SurfaceAnim.h"

class EmitVirtualKeyComponentAdvanced: public EntityComponent
{
public:
	EmitVirtualKeyComponentAdvanced();
	virtual ~EmitVirtualKeyComponentAdvanced();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnOverStart(VariantList *pVList);
	void OnOverEnd(VariantList *pVList);
	void OnDisabledChanged(Variant *pDataObject);

	uint32 *m_pDisabled;
	uint32 *m_pKeyCode;
};

#endif // EmitVirtualKeyComponentAdvanced_h__