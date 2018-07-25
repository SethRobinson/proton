#include "PlatformPrecomp.h"
#include "Component.h"

EntityComponent::EntityComponent()
{
	m_parent = NULL;
}

EntityComponent::~EntityComponent()
{
	if (!m_parent)
	{
		OnRemove();
	}
}
EntityComponent::EntityComponent( string name )
{
	SetName(name);
}
void EntityComponent::SetName( string name )
{
	m_name = name;
}

void EntityComponent::OnAdd(Entity *pEnt)
{
	m_parent = pEnt;
}

void EntityComponent::OnRemove()
{
	FunctionObject *pFunc = GetShared()->GetFunctionIfExists("OnDelete");
	if (pFunc)
	{
		//looks like someone wanted notification
		VariantList vList(this);
        pFunc->sig_function(&vList);
	}
}
