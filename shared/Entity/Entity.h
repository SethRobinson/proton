//  ***************************************************************
//  Entity - Creation date: 04/11/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Entity_h__
#define Entity_h__

#include "Component.h"

class Entity;

typedef list<Entity*> EntityList;
typedef list<Entity*>::iterator EntityListItor;


class EntityCall
{
public:
	Entity *pEnt;
	CL_Vec2f vUpdatedVar;

};

class Entity: public boost::signals::trackable
{
public:
	Entity(string name);
	Entity(EntityComponent *pComponent);  //add a component right away so you don't have to use AddComponent, just saves time
	Entity(string name, EntityComponent *pComponent);  //add a component right away so you don't have to use AddComponent, just saves time
	Entity();
	~Entity();

	enum eRecursiveVarOp
	{
		RECURSIVE_VAR_OP_ADDITION,
		RECURSIVE_VAR_OP_SUBTRACTION,
		RECURSIVE_VAR_OP_ADDITION_PLUS_ALIGNMENT_OFFSET,
		RECURSIVE_VAR_OP_SUBTRACTION_PLUS_ALIGNMENT_OFFSET,
	};

	enum eFilterSlotInfo
	{
		FILTER_INDEX = 3 //where we should stick return info in the pVList for filters
	};
	
	//a way to control recursive messages we receive, to break a chain of input or such
	enum eFilterCommunication
	{
		FILTER_ALLOW,
		FILTER_REFUSE_ALL,
		FILTER_REFUSE_CHILDREN
	
	};

	void SetName(string name);
	string GetName() {return m_name;}

	Entity * AddEntity(Entity *pEntity);
	/**
	 * Searches for an \c Entity with the given \a name.
	 * If the name of this \c Entity is \a name then returns the object itself. Otherwise the
	 * search descends recursively to the children \c Entities. If no \c Entity with the given
	 * \a name is found then \c NULL is returned.
	 */
	Entity * GetEntityByName(const string &name);
	bool RemoveEntityByName(const string &name, bool bRecursive = false); //if recursive flag, multiple entities might be deleted...
	bool RemoveEntityByNameSafe(const string &name, bool bRecursive); //tags for deletion, safely removed ASAP.  Safe to call from inside their own Update()'s etc
	bool RemoveEntityByAddress(Entity *pEntToDelete, bool DeleteAlso = true);
	void MoveEntityToTopByAddress(Entity *pEnt); //top of the stack, draws last
	void MoveEntityToBottomByAddress(Entity *pEnt); //bottom of the stack, draws first
	void RemoveAllEntities();
	bool RemoveEntitiesByNameThatStartWith(const string &name, bool bRecursive = false);

	EntityComponent * AddComponent(EntityComponent *pComp);
	/**
	 * Searches for an \c EntityComponent with the given \a name and returns it.
	 *
	 * The \c EntityComponent is searched from this \c Entity's components.
	 *
	 * If \a bAlsoCheckParents is \c true and the named \c EntityComponent is not found
	 * from this \c Entity then the parent \c Entity is searched also. The search continues
	 * up the \c Entity tree until the named \c EntityComponent is found or the top of the
	 * tree is reached.
	 *
	 * If no such \c EntityComponent is found \c NULL is returned.
	 */
	EntityComponent * GetComponentByName(string const &name, bool bAlsoCheckParents = false);
	bool RemoveComponentByAddress( EntityComponent *pCompToDelete, bool bDeleteAlso = true);
	bool RemoveComponentByName( const string &name);
	void MoveComponentToTopByAddress(EntityComponent *pComp);
	void MoveComponentToBottomByAddress( EntityComponent *pComp );

	void RemoveAllComponents();
	void PrintTreeAsText(int indent = 0); //recursively print every member of this tree to text log, for debugging
	VariantDB * GetShared() {return &m_sharedDB;}
	/**
	 * Gets a named \c Variant from this \c Entity's variant DB.
	 *
	 * If the named variant doesn't exist in the database a default variant is constructed
	 * and inserted to the database. This new variant can then be set to any value and type.
	 */
	Variant * GetVar(const string &varName) {return m_sharedDB.GetVar(varName);}
	/**
	 * Gets a named \c Variant from this \c Entity's variant DB.
	 *
	 * If the named variant doesn't exist in the database a new variant is constructed
	 * from the supplied default value and inserted to the database.
	 */
	Variant * GetVarWithDefault(const string &varName, const Variant &var) {return m_sharedDB.GetVarWithDefault(varName, var);}
	FunctionObject * GetFunction(const string &funcName) {return m_sharedDB.GetFunction(funcName);}
	
	Entity * GetParent(){return m_pParent;}
	void SetParent(Entity *pEntity) {m_pParent = pEntity;}
	bool GetTaggedForDeletion() {return m_bTaggedForDeletion;}
	void SetTaggedForDeletion();

	void CallFunctionRecursively(string funcName, VariantList *pVList); //call this function on us and all children
	
	//same as above but also add a number var before calling children, and remove it when done, this allows things like a Render() to
	//send local coordinates of the parent to take into account
	void CallFunctionRecursivelyWithUpdatedVar(const string funcName, VariantList *pVList, const string &varName, int varIndex, eRecursiveVarOp op, vector<EntityCall> *pEntList = NULL); 

	//same as above but magically hits everything in reverse order, good for handling input touches, which happens in the
	//reverse order that you draw things in
	void CallFunctionRecursivelyWithUpdatedVarBackwards( const string funcName, VariantList *pVList, const string &varName, int varIndex, eRecursiveVarOp op );

	boost::signal<void (Entity*)> sig_onRemoved;

	EntityList * GetChildren() {return &m_children;}
	ComponentList * GetComponents() {return &m_components;}

	void OnFilterAdd() {m_recursiveFilterReferences++;};
	void OnFilterRemove() {m_recursiveFilterReferences--;  assert(m_recursiveFilterReferences >= 0 && "Add/remove filters unbalanced?");};
	void AddEntitiesToVectorRescursively(vector<Entity*> *pEntVec); //get all children entities recursively

private:

	void OneTimeInit();
	void OnDelete(VariantList *pVList);
	string m_name;
	EntityList m_children;
	ComponentList m_components;
	VariantDB m_sharedDB;
	Entity *m_pParent;
	bool m_bTaggedForDeletion;
	int m_recursiveFilterReferences; //if > 0 then we also need to check for filters when recursive functions are called.  Useful for limiting who we repeat to intelligently
	
	//some hacks for speed
	Variant *m_pPosVarCache;
	CL_Vec2f *m_pSizeCache;
	uint32 *m_pAlignment;
	
};


#endif // Entity_h__
