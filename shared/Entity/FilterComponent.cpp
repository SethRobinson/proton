#include "PlatformPrecomp.h"

#include "FilterComponent.h"
#include "BaseApp.h"

FilterComponent::FilterComponent()
{
	SetName("Filter");
}

FilterComponent::~FilterComponent()
{
}

void FilterComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//register to get updated every frame

	GetParent()->OnFilterAdd();
	
	//our own vars
	m_pFunctionName = &GetVar("functionName")->GetString();
	if (m_pFunctionName->empty())
	{
		assert(!"You need to set the 'functionName' string variable before adding this component to an entity. Example: set it to OnRender");
	}

	m_pFilterSetting = &GetVarWithDefault("filterSetting", uint32(Entity::FILTER_ALLOW))->GetUINT32();
	SetName(string("FilterComp")+*m_pFunctionName); //so people can find us by name later easier
	GetParent()->GetFunction(string("Filter")+*m_pFunctionName)->sig_function.connect(1, boost::bind(&FilterComponent::FilterOnInput, this, _1));
}

void FilterComponent::OnRemove()
{
	GetParent()->OnFilterRemove();
	EntityComponent::OnRemove();
}

void FilterComponent::FilterOnInput(VariantList *pVList)
{
	//in case another filter has already made a statement about this
	if (pVList->m_variant[Entity::FILTER_INDEX].GetUINT32() == Entity::FILTER_REFUSE_ALL) return; 
	pVList->m_variant[Entity::FILTER_INDEX].Set(*m_pFilterSetting);
}

EntityComponent * AddFilter( Entity *pEnt, string functionName, Entity::eFilterCommunication filterSetting /*= Entity::FILTER_ALLOW*/ )
{
	assert(pEnt);
	EntityComponent *pComp = new FilterComponent;
	pComp->GetVar("functionName")->Set(functionName);
	pComp->GetVar("filterSetting")->Set(uint32(filterSetting));
	pEnt->AddComponent(pComp);
	return pComp;
}

Entity::eFilterCommunication GetFilterSetting( Entity *pEnt, string functionName )
{
	EntityComponent *pComp = pEnt->GetComponentByName(string("FilterComp")+functionName);
	if (!pComp)
	{
		LogMsg("Warning: Can't find filter %s", functionName.c_str());
		return Entity::FILTER_ALLOW;
	}

	return (Entity::eFilterCommunication)pComp->GetVar("filterSetting")->GetUINT32();
}

void SetFilterSetting( Entity *pEnt, string functionName, Entity::eFilterCommunication filterSetting, int timeMS /*= 0*/ )
{
	EntityComponent *pComp = pEnt->GetComponentByName(string("FilterComp")+functionName);
	if (!pComp)
	{
		LogMsg("Warning: Can't find filter %s", functionName.c_str());
		return;
	}

	if (timeMS)
	{
		//schedule it to happen later
		GetMessageManager()->SetComponentVariable(pComp, timeMS, "filterSetting", uint32(filterSetting));
	} else
	{
		//just set it directly
		pComp->GetVar("filterSetting")->Set(uint32(filterSetting));
	}
}