//  ***************************************************************
//  CharComponent - Creation date: 12/18/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef CharComponent_h__
#define CharComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"

#include "Character.h"


enum eID
{
	ID_PLAYER,
	ID_WEAK_ENEMY,
	ID_MIDDLE_ENEMY,
	ID_HARD_ENEMY,
	ID_DYNAMITE
};

enum eAI
{
	AI_PLAYER,
	AI_PATROL,
	AI_EXPLODE,
	AI_NONE
};


class CharComponent: public EntityComponent
{
public:
	CharComponent();
	virtual ~CharComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	void SetPositionByFloorAndCell(VariantList *pVList);
	void Move(bool bRight, bool bButtonDown);
	void UpLadder();
	void OnUse();
	void DownLadder();
	void SetPositionByFloorAndX(VariantList *pVList );

	int floorID;


	enum eState
	{
		STATE_STOP,
		STATE_MOVE_LEFT,
			STATE_MOVE_RIGHT
	};

	eState m_state;

	uint32 m_updateThink;
private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void UpdateMove();
	void Stop();
	void PlayerAI();
	void PatrolAI();
	void Damage(CL_Vec2f *pVDamPos, CharComponent *pEnemy, float radius, float damage);
	void Explode(VariantList *pVList);
	void OnDamage(VariantList *pVList);
	void PreExplode(VariantList *pVList);
	CL_Vec2f *m_pPos2d;
	
	uint32 *m_pFloorID;
	uint32 *m_pPaused;
	float *m_pSpeed;
	uint32 *m_pID, *m_pAI;

	vector<Character> m_chars;
	/*
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/
	
	bool m_moveASAP;
	bool m_moveASAPRight;
	bool m_moveASAPButtonDown;
	uint32 m_soundTimer;


};

#endif // CharComponent_h__