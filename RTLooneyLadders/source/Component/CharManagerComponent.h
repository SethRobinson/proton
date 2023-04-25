//  ***************************************************************
//  CharManagerComponent - Creation date: 12/18/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef CharManagerComponent_h__
#define CharManagerComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"

#include "CharComponent.h"

const uint32 EFFECT_EXPLODE = 0;
const uint32 EFFECT_STARS = 1;
const uint32 EFFECT_TELEPORT = 2;


class CharManagerComponent: public EntityComponent
{
public:
	CharManagerComponent();
	virtual ~CharManagerComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	void AddChar(eID id, eAI ai, uint32 floorID, uint32 cell);
	void AddCharEx( eID id, eAI ai, uint32 floorID, float x );

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void InitEffect(VariantList *pVList);
	CL_Vec2f *m_pPos2d;
	
	vector<CharComponent*> m_chars;
	/*
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/

	Entity *m_pCharFolder;
	
};

CharComponent * GetPlayer();
CharManagerComponent * GetCharManager();

#endif // CharManagerComponent_h__