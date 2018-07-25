#include "PlatformPrecomp.h"

#include "FocusRenderComponent.h"
#include "BaseApp.h"

FocusRenderComponent::FocusRenderComponent()
{
	SetName("FocusRender");
}

FocusRenderComponent::~FocusRenderComponent()
{
}

void FocusRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//register to get updated every frame
	GetBaseApp()->m_sig_render.connect(1, boost::bind(&FocusRenderComponent::OnRender, this, _1));
}

void FocusRenderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void FocusRenderComponent::OnRender(VariantList *pVList)
{
	//the 0 is because the pt we need modified is index 0 of the VariantList
	GetParent()->CallFunctionRecursivelyWithUpdatedVar("OnRender", pVList, string("pos2d"), 0, Entity::RECURSIVE_VAR_OP_ADDITION_PLUS_ALIGNMENT_OFFSET); 
}
