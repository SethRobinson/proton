#include "VariantTuner.h"

#include "BaseApp.h"

#ifdef PLATFORM_LINUX
# include <SDL/SDL.h>
# define WHEELUP_ID    SDL_BUTTON_WHEELUP
# define WHEELDOWN_ID  SDL_BUTTON_WHEELDOWN
#else
# error "No mouse wheel up and down event ids set for this platform. Add them here"
#endif

class VariantChangerComponent : public EntityComponent
{
public:
	VariantChangerComponent() :
		m_pChangeable(NULL)
	{
		SetName("VariantChanger");
	}

	virtual ~VariantChangerComponent() {
	}

	virtual void OnAdd(Entity *pEnt)
	{
		EntityComponent::OnAdd(pEnt);

		GetBaseApp()->m_sig_input.connect(boost::bind(&VariantChangerComponent::OnInput, this, _1));

		m_pStep = GetVar("step");
	}

	void SetChangeableVariant(Variant *var)
	{
		m_pChangeable = var;
		if (m_pChangeable != NULL)
		{
			LogMsg("Value is initially %s", m_pChangeable->Print().c_str());
		}
	}

private:
	void OnInput(VariantList *pVList)
	{
		uint32 fingerID = 0;
		if (pVList->Get(2).GetType() == Variant::TYPE_UINT32)
		{
			fingerID = pVList->Get(2).GetUINT32();
		}

		if (eMessageType(int(pVList->Get(0).GetFloat())) == MESSAGE_TYPE_GUI_CLICK_START)
		{
			OnPressed(fingerID);
		}
	}

	void OnPressed(uint32 fingerID)
	{
		if (m_pChangeable == NULL || m_pStep == NULL)
		{
			return;
		}

		switch (fingerID) {
		case WHEELUP_ID:
			(*m_pChangeable) += (*m_pStep);
			LogMsg("Value is %s", m_pChangeable->Print().c_str());
			break;

		case WHEELDOWN_ID:
			(*m_pChangeable) -= (*m_pStep);
			LogMsg("Value is %s", m_pChangeable->Print().c_str());
			break;
		}
	}

	Variant *m_pChangeable;
	Variant *m_pStep;
};

VariantTuner::VariantTuner()
{
}

void VariantTuner::setTunableVariant(Variant& var, const Variant& step)
{
	Entity* rootEntity = GetBaseApp()->GetEntityRoot();
	VariantChangerComponent* varChanger = dynamic_cast<VariantChangerComponent*>(rootEntity->GetComponentByName("VariantChanger"));

	if (varChanger == NULL)
	{
		varChanger = new VariantChangerComponent;
		rootEntity->AddComponent(varChanger);
	}

	varChanger->SetChangeableVariant(&var);

	varChanger->GetVar("step")->Reset();
	varChanger->GetVar("step")->Set(step);
}

void VariantTuner::resetTunableVariant()
{
	Entity* rootEntity = GetBaseApp()->GetEntityRoot();
	VariantChangerComponent* varChanger = dynamic_cast<VariantChangerComponent*>(rootEntity->GetComponentByName("VariantChanger"));

	if (varChanger != NULL)
	{
		varChanger->SetChangeableVariant(NULL);
	}
}
