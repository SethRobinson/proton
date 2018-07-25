#include "PlatformPrecomp.h"

#include "TyperComponent.h"
#include "BaseApp.h"
#include "EntityUtils.h"

TyperComponent::TyperComponent()
{
	m_timerToAddMS = 0;
	SetName("Typer");
}

TyperComponent::~TyperComponent()
{
}

void TyperComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	
	m_pText = &GetVarWithDefault("text", Variant("..."))->GetString();
	m_pMode = &GetVarWithDefault("mode", Variant(uint32(MODE_REPEAT)))->GetUINT32();
	m_pUpdateSpeedMS = &GetVarWithDefault("speedMS", Variant(uint32(350)))->GetUINT32();
	m_pPaused = &GetVarWithDefault("paused", Variant(uint32(0)))->GetUINT32();

	m_curPos = 0;
	m_timer = GetBaseApp()->GetTick();

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&TyperComponent::OnUpdate, this, _1));

}

void TyperComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TyperComponent::RemoveActiveChanges()
{
	EntityComponent *pTextRender = GetParent()->GetComponentByName("TextRender");

	if (!pTextRender)
	{
		pTextRender = GetParent()->GetComponentByName("TextBoxRender");
	}
	if (!pTextRender) 
	{
		m_curPos = 0;	
		return;
	}
	
	string text = pTextRender->GetVar("text")->GetString();
	text.erase(text.length()-m_curPos, m_curPos);
	pTextRender->GetVar("text")->Set(text);
	m_curPos = 0;
	m_timerToAddMS = 0;

}
void TyperComponent::OnUpdate(VariantList *pVList)
{
	
	if (*m_pPaused != 0) return;

	if (m_timer < GetBaseApp()->GetTick())
	{
		m_timerToAddMS += *m_pUpdateSpeedMS+ GetBaseApp()->GetTick()-m_timer;
		m_timer = GetBaseApp()->GetTick()+*m_pUpdateSpeedMS;

		while(m_timerToAddMS >= (int)*m_pUpdateSpeedMS)
		{

			m_timerToAddMS -= *m_pUpdateSpeedMS;
			if (m_curPos == m_pText->length() && m_pText->length() != 0)
			{
				//done, now what?
				switch (*m_pMode)
				{
				case MODE_REPEAT:
					RemoveActiveChanges();
					return;
					break;

				case MODE_ONCE_AND_REMOVE_SELF:
					GetParent()->RemoveComponentByAddress(this);
					return;
					break;

				}
			}
			
			EntityComponent *pTextRender = GetParent()->GetComponentByName("TextRender");
			
			if (!pTextRender)
			{
				pTextRender = GetParent()->GetComponentByName("TextBoxRender");
			}

			if (!pTextRender)
			{
				LogMsg("Typer requires a TextRender or TextBoxRender to work...");
				return;
			}

			string text = pTextRender->GetVar("text")->GetString();
			if (!m_pText->empty())
			{
				text += m_pText->at(m_curPos);
			}
			pTextRender->GetVar("text")->Set(text);

			m_curPos++;
		}
	
	}


}
