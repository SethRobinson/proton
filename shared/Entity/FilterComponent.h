//  ***************************************************************
//  FilterComponent - Creation date: 01/21/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FilterComponent_h__
#define FilterComponent_h__

#include "Component.h"

class Entity;

class FilterComponent: public EntityComponent
{
public:
	FilterComponent();
	virtual ~FilterComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	void FilterOnInput(VariantList *pVList);
	//our stuff

private:

	uint32 *m_pFilterSetting;
	string * m_pFunctionName;

};


//some helpers to make it cleaner to work with.  Maybe they should go into the EntityUtils.cpp file, but then I'd have to
//fix all old projects to include this file which I'm too lazy to do.

EntityComponent * AddFilter(Entity *pEnt, string functionName, Entity::eFilterCommunication filterSetting = Entity::FILTER_ALLOW);
Entity::eFilterCommunication GetFilterSetting(Entity *pEnt, string functionName);
void SetFilterSetting(Entity *pEnt, string functionName, Entity::eFilterCommunication filterSetting, int timeMS = 0);

/*
Example of usage to disable rendering (also would work for OnInput, OnUpdate if you wanted to disable all of them, or
any user called recursive function call)

#include "Entity/FilterComponent.h"

AddFilter(pButtonEntity, "OnRender"); //add it.  Note that optionally we could have sent a third part to set the filter state.
//set it, with optional timeMS to schedule when it should be set
SetFilterSetting(pButtonEntity, "OnRender", Entity::FILTER_REFUSE_ALL, 1000); //turn it off in a second
SetFilterSetting(pButtonEntity, "OnRender", Entity::FILTER_ALLOW, 2000); //turn it back on a second later

*/

#endif // FilterComponent_h__