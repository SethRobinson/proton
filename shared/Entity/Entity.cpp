#include "PlatformPrecomp.h"

#include "Entity.h"
#include "BaseApp.h"

Entity::Entity(string name): m_name(name)
{
	OneTimeInit();
}

Entity::Entity(EntityComponent *pComponent)
{
	OneTimeInit();
	AddComponent(pComponent);
}

Entity::Entity(string name, EntityComponent *pComponent) : m_name(name)
{
	OneTimeInit();
	AddComponent(pComponent);
}

Entity::Entity()
{
	OneTimeInit();
}

void Entity::OneTimeInit()
{
	m_pPosVarCache = NULL;
	m_pAlignment = NULL;
	m_pSizeCache = NULL;
	m_recursiveFilterReferences = 0;
	m_bTaggedForDeletion = false;
	m_pParent = NULL;

	GetFunction("OnDelete")->sig_function.connect(1, boost::bind(&Entity::OnDelete, this, _1));
}

Entity::~Entity()
{
	sig_onRemoved(this); //in case anybody cared when we were deleted

	RemoveAllEntities();
	RemoveAllComponents();
}

void Entity::RemoveAllEntities()
{
	EntityListItor itor = m_children.begin();
	for (;itor != m_children.end(); )
	{

		//done this way so entities that want to do searches through entity trees because some OnDelete sig
		//was run won't crash

		Entity *pTemp = (*itor);
		itor = m_children.erase(itor);
		delete pTemp;

	}

	m_children.clear();
}

void Entity::RemoveAllComponents()
{
	ComponentListItor itor = m_components.begin();
	for (;itor != m_components.end(); itor++)
	{
		(*itor)->OnRemove();
		delete (*itor);
	}

	m_components.clear();
}

Entity * Entity::AddEntity( Entity *pEntity )
{
	pEntity->SetParent(this);
	m_children.push_back(pEntity);
	return pEntity;
}

void Entity::SetName( string name )
{
	m_name = name;
}

EntityComponent * Entity::AddComponent(EntityComponent *pComp)
{
	assert(pComp);
	m_components.push_back(pComp);

	pComp->OnAdd(this);

	return pComp;
}

Entity * Entity::GetEntityByName(const string &name)
{
	if (m_bTaggedForDeletion) return NULL;
	if (name == m_name) return this;

	EntityListItor itor = m_children.begin();

	Entity * pEnt = NULL;

	while (itor != m_children.end())
	{
		pEnt = (*itor)->GetEntityByName(name);
		if (pEnt) return pEnt;
		itor++;
	}

	return NULL;
}

void Entity::PrintTreeAsText( int indent /*= 0*/ )
{
#ifndef _DEBUG
	return;
#endif
	string us;

	for (int i =0; i < indent; i++)
	{
		us += "  "; //make the tree have branches more visually
	}
	us += m_name;

	if (!m_components.empty())
	{
		us += " (";
		ComponentListItor itor = m_components.begin();
		for (;itor != m_components.end(); itor++)
		{
			if (itor != m_components.begin()) us +=", ";
			us += (*itor)->GetName();
		}

		us +=")";
	}

	LogMsg(us.c_str());

	EntityListItor itor = m_children.begin();

	while (itor != m_children.end())
	{
		(*itor)->PrintTreeAsText(indent+1);
		itor++;
	}
}

void Entity::OnDelete(VariantList *pVList)
{
	if (GetParent())
	{
		GetParent()->RemoveEntityByAddress(this);
	} else
	{
		//no parent?  Fine, kill us anyway
		delete this;
		return;
	}

}

void Entity::CallFunctionRecursively(string funcName, VariantList *pVList)
{
	//call ours which may modify it
	GetShared()->CallFunctionIfExists(funcName, pVList);

	//call our children..
	EntityList childrenTemp = m_children; //make a copy so it's safe for entities to remove, adjust the order on the fly
	EntityListItor itor = childrenTemp.begin();

	while (itor != childrenTemp.end())
	{
		(*itor)->CallFunctionRecursively(funcName, pVList);
		itor++;
	}
}


//like below, but calls everything in reverse order
void Entity::CallFunctionRecursivelyWithUpdatedVarBackwards( const string funcName, VariantList *pVList, const string &varName, int varIndex, eRecursiveVarOp op )
{
	vector<EntityCall> entList;

	//scan everybody we would hit, without actually calling anything
	CallFunctionRecursivelyWithUpdatedVar(funcName, pVList, varName, varIndex, op, &entList);
	//LogMsg("Got %d items", entList.size());

	CL_Vec2f vOriginal = pVList->m_variant[varIndex].GetVector2();

	//now do the actual calls, in reverse order

	for (int i=entList.size()-1; i >= 0; i--)
	{
		pVList->m_variant[varIndex].Set(entList[i].vUpdatedVar);
		entList[i].pEnt->GetShared()->CallFunctionIfExists(funcName, pVList);
	}

	pVList->m_variant[varIndex].Set(vOriginal);
}

void Entity::CallFunctionRecursivelyWithUpdatedVar( const string funcName, VariantList *pVList, const string &varName, int varIndex, eRecursiveVarOp op, vector<EntityCall> *pEntList )
{

	//OPTIMIZE  Add a AddFilter/RemoveFilter() thing so we don't have to do this check on most entities?
	FunctionObject *pFilterFunc =  NULL;

	if (m_recursiveFilterReferences > 0)
	{
		pFilterFunc = GetShared()->GetFunctionIfExists("Filter"+funcName);
	}

	if (pFilterFunc)
	{
		//we have extra filtering active to see if we should actually process this kind of message or not
		pVList->m_variant[FILTER_INDEX].Set(uint32(FILTER_ALLOW)); //set the default.  You'd better not have been using this var, it's reserved.

		pFilterFunc->sig_function(pVList);
		if (pVList->m_variant[FILTER_INDEX].GetUINT32() == FILTER_REFUSE_ALL)
		{
			pFilterFunc = GetShared()->GetFunctionIfExists("Post"+funcName);

			if (pFilterFunc)
			{
				//we have extra filtering active to see if we should actually process this kind of message or not
				pFilterFunc->sig_function(pVList);
			}

			return;
		}
	}

	//LogMsg("Calling %s on %s at %d", funcName.c_str(), GetName().c_str(), GetTick(TIMER_GAME));
	
	CL_Vec2f vOriginal = pVList->m_variant[varIndex].GetVector2();
	Variant *pVar;

	switch (op)
	{
		case RECURSIVE_VAR_OP_ADDITION_PLUS_ALIGNMENT_OFFSET:
		case RECURSIVE_VAR_OP_SUBTRACTION_PLUS_ALIGNMENT_OFFSET:
			{

				if (!m_pAlignment)
				{
					//setup the cached var pointers, purely a speed thing, not really needed.  So yeah, needed
					m_pAlignment = &GetVarWithDefault("alignment", uint32(ALIGNMENT_UPPER_LEFT))->GetUINT32();
					m_pSizeCache = &GetVarWithDefault("size2d", CL_Vec2f(0,0))->GetVector2();
					m_pPosVarCache = GetVarWithDefault("pos2d", CL_Vec2f(0,0));
				} 
				pVar = m_pPosVarCache;

				if (*m_pAlignment != ALIGNMENT_UPPER_LEFT)
				{
					CL_Vec2f vOffset = GetAlignmentOffset(*m_pSizeCache, eAlignment(*m_pAlignment));
					if (op == RECURSIVE_VAR_OP_ADDITION_PLUS_ALIGNMENT_OFFSET)
					{
						vOffset *= -1; //reverse it for this way
					}
					pVList->m_variant[varIndex].GetVector2() += vOffset;
				}
			}
		default:

			pVar = GetShared()->GetVarIfExists(varName);
			break;
	}

	if (pEntList)
	{
		//nope, just send back who we WOULD have called
		//LogMsg("Adding %s", GetName().c_str());
		pEntList->push_back(EntityCall());
		EntityCall *pEntCallItem = &pEntList->back();
		pEntCallItem->pEnt = this;
		pEntCallItem->vUpdatedVar = pVList->m_variant[varIndex].GetVector2();

	} else
	{
		//do the real call
		GetShared()->CallFunctionIfExists(funcName, pVList);
	}

	if (pVar)
	{
		switch(pVar->GetType())
		{
		case  Variant::TYPE_VECTOR2:
	
			switch (op)
			{
			case RECURSIVE_VAR_OP_ADDITION_PLUS_ALIGNMENT_OFFSET:
			case RECURSIVE_VAR_OP_ADDITION:
				pVList->m_variant[varIndex].Set(pVList->m_variant[varIndex].GetVector2() + pVar->GetVector2());
				break;

			case RECURSIVE_VAR_OP_SUBTRACTION:
			case RECURSIVE_VAR_OP_SUBTRACTION_PLUS_ALIGNMENT_OFFSET:
				pVList->m_variant[varIndex].Set(pVList->m_variant[varIndex].GetVector2() - pVar->GetVector2());
				break;
			default:
				LogError("CallFunctionRecursivelyWithUpdatedVar: Bad op");assert(0);
			}		
			break;

		default:
			LogError("Don't know how to add this type");
		}
	}

	if (pFilterFunc && pVList->m_variant[FILTER_INDEX].GetUINT32() == FILTER_REFUSE_CHILDREN)
	{
		//ignoring our kids, programming imitates life
	} else
	{
		//call our children..
		EntityList childrenTemp = m_children; //make a copy so it's safe for entities to remove, adjust the order on the fly
		EntityListItor itor = childrenTemp.begin();

		while (itor != childrenTemp.end())
		{
			(*itor)->CallFunctionRecursivelyWithUpdatedVar(funcName, pVList, varName, varIndex, op, pEntList);
			itor++;
		}
		//put it back to how it was
	}

	if (pVar)
	{
		switch(pVar->GetType())
		{
		case  Variant::TYPE_VECTOR2:
			pVList->m_variant[varIndex].Set(vOriginal);
			break;

            default:;
        }
	}

	if (m_recursiveFilterReferences > 0)
	{
		//let's also check for a post-function message, useful for things like adding/removing glclip commands.  
		//Because these checks are slower, you must call OnAddRecursiveFilter() before we'll check for these...
		pFilterFunc = GetShared()->GetFunctionIfExists("Post"+funcName);

		if (pFilterFunc)
		{
			//we have extra filtering active to see if we should actually process this kind of message or not
			pFilterFunc->sig_function(pVList);
		}
	}
}

EntityComponent * Entity::GetComponentByName( string const &name, bool bAlsoCheckParents )
{
	ComponentListItor itor = m_components.begin();
	for (;itor != m_components.end(); itor++)
	{
		if (name == (*itor)->GetName()) 
		{
			return *itor;
		}
	}
	if (!bAlsoCheckParents || !GetParent()) return NULL;

	return GetParent()->GetComponentByName(name, bAlsoCheckParents);
}


bool Entity::RemoveEntityByName(const string &name, bool bRecursive)
{
	EntityListItor itor = m_children.begin();

	bool bRemoved = false;

	while (itor != m_children.end())
	{
		if ((*itor)->GetName() == name)
		{
			Entity *pTemp = (*itor);
			itor = m_children.erase(itor);
			delete (pTemp);
			bRemoved = true;
			continue;
		} else
		{
			if (bRecursive)
			{
				if ((*itor)->RemoveEntityByName(name, bRecursive))
				{
					bRemoved = true;
				}
			}
		}

		itor++;
	}

	return bRemoved;
}



bool Entity::RemoveEntitiesByNameThatStartWith(const string &name, bool bRecursive)
{
	EntityListItor itor = m_children.begin();

	bool bRemoved = false;

	while (itor != m_children.end())
	{
		if ( StringFromStartMatches( (*itor)->GetName(), name) )
		{
			Entity *pTemp = (*itor);
			itor = m_children.erase(itor);
			delete (pTemp);
			bRemoved = true;
			continue;
		} else
		{
			if (bRecursive)
			{
				if ((*itor)->RemoveEntitiesByNameThatStartWith(name, bRecursive))
				{
					bRemoved = true;
				}
			}
		}

		itor++;
	}

	return bRemoved;
}

bool Entity::RemoveEntityByNameSafe(const string &name, bool bRecursive)
{
	EntityListItor itor = m_children.begin();

	bool bRemoved = false;

	while (itor != m_children.end())
	{
		if ((*itor)->GetName() == name)
		{
			(*itor)->SetTaggedForDeletion();
			bRemoved = true;
		} else
		{
			if (bRecursive)
			{
				if ((*itor)->RemoveEntityByNameSafe(name, bRecursive))
				{
					bRemoved = true;
				}
			}
		}

		itor++;
	}

	return bRemoved;
}


void Entity::SetTaggedForDeletion()
{
	if (m_bTaggedForDeletion) return; //already did it
	m_bTaggedForDeletion = true;
	//actually tell the deletion handler to do it ASAP
    VariantList vList(this);
	GetMessageManager()->CallEntityFunction(this, 0, "OnDelete", &vList);
}

bool Entity::RemoveEntityByAddress(Entity *pEntToDelete, bool bDeleteAlso)
{
	EntityListItor itor = m_children.begin();

	while (itor != m_children.end())
	{
		if ((*itor) == pEntToDelete)
		{
			Entity *pTemp = (*itor);
			itor = m_children.erase(itor);
			if (bDeleteAlso)
			{
				delete (pTemp);
			}
			return true;
		}
		itor++;
	}

	return false;
}

void Entity::MoveEntityToBottomByAddress(Entity *pEnt)
{
	if (!RemoveEntityByAddress(pEnt, false))
	{
		LogError("Unable to find entity to remove");
		return;
	}

	m_children.push_front(pEnt);
}

void Entity::MoveEntityToTopByAddress(Entity *pEnt)
{
	if (!RemoveEntityByAddress(pEnt, false))
	{
		LogError("Unable to find entity to remove");
		return;
	}

	m_children.push_back(pEnt);
}

bool Entity::RemoveComponentByAddress( EntityComponent *pCompToDelete, bool bDeleteAlso )
{
	ComponentListItor itor = m_components.begin();

	assert(pCompToDelete);

	while (itor != m_components.end())
	{
		if ((*itor) == pCompToDelete)
		{
			if (bDeleteAlso)
			{
				(*itor)->OnRemove();
				delete (*itor);
			} else
			{
				//we're probably just moving it in the draw list, we don't actually delete it
			}
			itor = m_components.erase(itor);
			return true;
		}
		itor++;
	}

	return false;
}


bool Entity::RemoveComponentByName( const string &name )
{
	ComponentListItor itor = m_components.begin();

	while (itor != m_components.end())
	{
		if ((*itor)->GetName() == name)
		{
			(*itor)->OnRemove();
			delete (*itor);
			itor = m_components.erase(itor);
			return true;
		}
		itor++;
	}

	return false;
}

void Entity::MoveComponentToTopByAddress( EntityComponent *pComp )
{
	//first locate it

	if (!RemoveComponentByAddress(pComp, false))
	{
		LogError("Unable to find component to delete");
		return;
	}

	//delete it from our list, re-add it now
	m_components.push_back(pComp);
}


void Entity::MoveComponentToBottomByAddress( EntityComponent *pComp )
{
	//first locate it

	if (!RemoveComponentByAddress(pComp, false))
	{
		LogError("Unable to find component to delete");
		return;
	}

	//delete it from our list, re-add it now
	m_components.push_front(pComp);
}

void Entity::AddEntitiesToVectorRescursively( vector<Entity*> *pEntVec )
{
	EntityListItor itor = m_children.begin();

	while (itor != m_children.end())
	{
		pEntVec->push_back( (*itor));
		(*itor)->AddEntitiesToVectorRescursively(pEntVec);
		itor++;
	}

}