//  ***************************************************************
//  TyperComponent - Creation date: 06/10/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//You know how on documentaries the text overlay sometimes "types" itself out over the screen?  Well, this
//does that, by adding chars to the TextRender component that is also attached to the parent entity. (up to you to make
//sure one exists)

#ifndef TyperComponent_h__
#define TyperComponent_h__

#include "Component.h"
class Entity;

class TyperComponent: public EntityComponent
{
public:
	TyperComponent();
	virtual ~TyperComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	enum eMode
	{
		MODE_ONCE_AND_REMOVE_SELF,
		MODE_REPEAT
	};
	
	void OnUpdate(VariantList *pVList);
	//our stuff

private:

	void RemoveActiveChanges();

	uint32 *m_pMode;
	uint32 *m_pUpdateSpeedMS;
	uint32 m_timer;
	int m_curPos;
	uint32 *m_pPaused;
	string *m_pText;
	int m_timerToAddMS;
};

#endif // TyperComponent_h__