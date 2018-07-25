#include "PlatformPrecomp.h"

#include "InterpolateComponent.h"
#include "BaseApp.h"

InterpolateComponent::InterpolateComponent()
{
	SetName("Interpolate");
	m_pVar = NULL;
	m_pVarTarget = NULL;
	m_bActive= false;
	m_bDirForward = true;
}

InterpolateComponent::~InterpolateComponent()
{
}

void InterpolateComponent::OnVarNameChanged(Variant *pDataObject)
{
	if (m_pComponentName->length())
	{
		EntityComponent * pComp = GetParent()->GetComponentByName(*m_pComponentName);
		if (!pComp)
		{
			LogError("InterpolateComponent %s is unable to find component %s to set its var %s",
				GetName().c_str(), m_pComponentName->c_str(), pDataObject->GetString().c_str());
			assert(!"Check out your log!");
			return;
		}

		m_pVar = pComp->GetVar(pDataObject->GetString());

		// If the target component gets deleted we mustn't access its variants anymore
		pComp->GetFunction("OnDelete")->sig_function.connect(1, boost::bind(&InterpolateComponent::NullifyVarPointer, this, _1));
	} else
	{
		m_pVar = GetParent()->GetVar(pDataObject->GetString());
	}
}

void InterpolateComponent::OnDurationChanged(Variant *pDataObject)
{
	if (!m_pVar)
	{
		LogError("Must set var_name before setting duration_ms, which starts the process");
		return;
	}
	
	if (*m_pDuration == 0)
	{
		//just show it off
		m_bActive = false;
		return;
	}
	
	m_startTimeMS = GetBaseApp()->GetTickTimingSystem(eTimingSystem(*m_pTimingSystem));
	
	m_pVarStartPoint = *m_pVar;
	m_bActive = true;
	(*m_pPlayCount) = 0;
}

void InterpolateComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//if game is currently paused, assuming we should use a system timer so we don't pause with it
	
	m_pTimingSystem = &GetVarWithDefault("timingSystem", Variant(uint32(GetBaseApp()->GetActiveTimingSystem())))->GetUINT32();
	m_pVarName = &GetVar("var_name")->GetString(); //local to us
	m_pDuration = &GetVar("duration_ms")->GetUINT32();
	m_pDeleteAfterPlayCount = &GetVar("deleteAfterPlayCount")->GetUINT32();
	m_pPlayCount = &GetVar("playCount")->GetUINT32();
	m_pOnFinish = &GetVarWithDefault("on_finish", Variant( uint32(ON_FINISH_DIE)))->GetUINT32();
	m_pInterpolateType = &GetVarWithDefault("interpolation", Variant( uint32(INTERPOLATE_LINEAR)))->GetUINT32();
	m_pVarTarget = GetVar("target");

	m_pComponentName = &GetShared()->GetVar("component_name")->GetString();

	GetVar("var_name")->GetSigOnChanged()->connect(1, boost::bind(&InterpolateComponent::OnVarNameChanged, this, _1));
	GetVar("duration_ms")->GetSigOnChanged()->connect(1, boost::bind(&InterpolateComponent::OnDurationChanged, this, _1));

	//register to get updated every frame, assuming our entity gets an OnUpdate call
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&InterpolateComponent::OnUpdate, this, _1));
}

void InterpolateComponent::OnRemove()
{
	SetEndValue();
	EntityComponent::OnRemove();
}

void InterpolateComponent::NullifyVarPointer(VariantList *pVList)
{
	m_pVar = NULL;
}

void InterpolateComponent::SetEndValue()
{
	if (m_pVar)
	{
		Variant *pV = GetShared()->GetVarIfExists("set_value_on_finish");
		if (pV)
		{
			m_pVar->Set(*pV);
		}
	}
}

void InterpolateComponent::OnUpdate(VariantList *pVList)
{
	
	if (m_bActive && m_pVar && m_pVar->GetType() != Variant::TYPE_UNUSED && m_pVarTarget->GetType() != Variant::TYPE_UNUSED)
	{
	
		Variant *pA;
		Variant *pB;

		if (m_bDirForward)
		{
			pA = &m_pVarStartPoint;
			pB = m_pVarTarget;
		} else
		{
			pB = &m_pVarStartPoint;
			pA = m_pVarTarget;
		}

		float fPercentDone = (float(GetBaseApp()->GetTickTimingSystem(eTimingSystem(*m_pTimingSystem))-m_startTimeMS)/ float(*m_pDuration));

#ifdef _DEBUG
		//LogMsg("%.2f percent done changing %s (is now %s)", fPercentDone, m_pVarName->c_str(), m_pVar->Print().c_str());
#endif
			if ( fPercentDone >= 1)
			{
				//we're finished
				m_pVar->Set(*pB);
				bool bReturnAfter = true;

				switch (*m_pOnFinish)
				{
				
				case ON_FINISH_STOP:
					m_bActive = false;
					SetEndValue();
					return;
					break;

				case ON_FINISH_DIE:
					m_bActive = false;
					//we're done, kill ourself
					GetParent()->RemoveComponentByAddress(this);
					return;
					break;

				case ON_FINISH_BOUNCE:
					m_bDirForward = !m_bDirForward;
					m_startTimeMS = GetBaseApp()->GetTickTimingSystem(eTimingSystem(*m_pTimingSystem));
					bReturnAfter = false;
					break;

				case ON_FINISH_REPEAT:
					m_startTimeMS = GetBaseApp()->GetTickTimingSystem(eTimingSystem(*m_pTimingSystem));
					m_pVar->Set(m_pVarStartPoint);
					break;

				default:
					LogError("Unknown OnFinish type");
					assert(0);
				}
				(*m_pPlayCount)++;

				if (bReturnAfter)
				{
					return;
				}
			}
				
			if (*m_pDeleteAfterPlayCount != 0 &&  *m_pPlayCount >= *m_pDeleteAfterPlayCount)
			{
				m_bActive = false;
				//we're done, kill ourself
				GetParent()->RemoveComponentByAddress(this);
				return;
			}
		//set var where it should be on the scale
		m_pVar->Interpolate(pA, pB, fPercentDone, eInterpolateType(*m_pInterpolateType));
	}

}
