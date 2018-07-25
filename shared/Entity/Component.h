//  ***************************************************************
//  Component - Creation date: 04/11/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Component_h__
#define Component_h__

#include "../Manager/VariantDB.h"

class EntityComponent;
class Entity;

typedef list<EntityComponent *> ComponentList;

typedef list<EntityComponent *>::iterator ComponentListItor;

//Changed from Component to EntityComponent because of a naming conflict in OSX and I don't want to namespace it

class EntityComponent: public boost::signals::trackable
{
public:
	EntityComponent();
	virtual ~EntityComponent();
	EntityComponent(string name);

	void SetName(string name);
	string GetName() {return m_name;}
	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	Entity * GetParent() {return m_parent;}
	VariantDB * GetShared() {return &m_sharedDB;}
	Variant * GetVar(string const &varName) { return m_sharedDB.GetVar(varName);}
	Variant * GetVarWithDefault(const string &varName, const Variant &var) {return m_sharedDB.GetVarWithDefault(varName, var);}
	FunctionObject * GetFunction(const string &funcName) {return m_sharedDB.GetFunction(funcName);}

private:

	string m_name;
	Entity * m_parent;
	VariantDB m_sharedDB;
};

#endif // Component_h__