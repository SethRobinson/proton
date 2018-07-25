//  ***************************************************************
// InterpolateComponent - Creation date: 04/14/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef InterpolateComponent_h__
#define InterpolateComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/Surface.h"


class InterpolateComponent: public EntityComponent
{
public:
	
	InterpolateComponent();
	virtual ~InterpolateComponent();
	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	void OnUpdate(VariantList *pVList);
	void OnVarNameChanged(Variant *pDataObject);
	void OnDurationChanged(Variant *pDataObject);
	//our stuff

	enum eOnFinish
	{
		ON_FINISH_DIE = 0,
		ON_FINISH_BOUNCE, //after playing forward, will play again in reverse forever, or until m_pDeleteAfterPlayCount is reached
		ON_FINISH_REPEAT, //loop from the start position again, will play forever or until m_pDeleteAfterPlayCount is reached
		ON_FINISH_STOP //play once and stop, don't worry about deleting itself
	};

private:

	void NullifyVarPointer(VariantList *pVList);
	void SetEndValue();

	Variant *m_pVar, *m_pVarTarget;
	Variant m_pVarStartPoint;
	string * m_pVarName;
	uint32 m_startTimeMS;
	uint32 *m_pDuration;
	bool m_bActive;
	uint32 *m_pInterpolateType;
	uint32 *m_pOnFinish;
	bool m_bDirForward;
	uint32 *m_pDeleteAfterPlayCount;
	uint32 *m_pPlayCount;
	uint32 *m_pTimingSystem;
	string *m_pComponentName; //"" if target var is not in a component, otherwise, its name


};

#endif // InterpolateComponent_h_
