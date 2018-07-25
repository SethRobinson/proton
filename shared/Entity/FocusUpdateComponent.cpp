#include "PlatformPrecomp.h"

#include "FocusUpdateComponent.h"
#include "BaseApp.h"
#include "EntityUtils.h"

FocusUpdateComponent::FocusUpdateComponent()
{
	SetName("FocusUpdate");
}

FocusUpdateComponent::~FocusUpdateComponent()
{
}

void FocusUpdateComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//register to get updated every frame
	GetBaseApp()->m_sig_update.connect(1, boost::bind(&FocusUpdateComponent::OnUpdate, this, _1));

}

void FocusUpdateComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void FocusUpdateComponent::OnUpdate(VariantList *pVList)
{
	
	GetParent()->CallFunctionRecursively("OnUpdate", pVList);
}
