//  ***************************************************************
//  SelectButtonWithCustomInputComponent - Creation date: 8/27/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef SelectButtonWithCustomInputComponent_h__
#define SelectButtonWithCustomInputComponent_h__

#include "Entity/Component.h"

class SelectButtonWithCustomInputComponent: public EntityComponent
{
public:
	SelectButtonWithCustomInputComponent();
	virtual ~SelectButtonWithCustomInputComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:
	string * m_pKeys;
	void OnInput( VariantList *pVList );
	void ClickButton();
	void OnInputRaw( VariantList *pVList );
	uint32 *m_pDisabled;
	uint32 *m_pKeyCode;

};

#endif // SelectButtonWithCustomInputComponent_h__