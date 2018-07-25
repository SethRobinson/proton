#include "PlatformPrecomp.h"

#include "FilterInputComponent.h"
#include "BaseApp.h"

FilterInputComponent::FilterInputComponent()
{
	SetName("FilterInput");
}

FilterInputComponent::~FilterInputComponent()
{
}

void FilterInputComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//register to get updated every frame
	
	GetParent()->OnFilterAdd();
	GetParent()->GetFunction("FilterOnInput")->sig_function.connect(1, boost::bind(&FilterInputComponent::FilterOnInput, this, _1));
	
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();

	//our own vars
	m_pMode = &GetVarWithDefault("mode", uint32(MODE_CLIP_INPUT_TO_SIZE))->GetUINT32();
	m_pClipRect = &GetVar("clipRect")->GetRect();

}

void FilterInputComponent::OnRemove()
{
	GetParent()->OnFilterRemove();
	EntityComponent::OnRemove();
}

void FilterInputComponent::FilterOnInput(VariantList *pVList)
{
	
	if (pVList->m_variant[Entity::FILTER_INDEX].GetUINT32() == Entity::FILTER_REFUSE_ALL) return; 

	switch (*m_pMode)
	{
	case MODE_CLIP_INPUT_TO_SIZE:
		
		//let's only apply this to some messages
		switch (eMessageType( int(pVList->Get(0).GetFloat())))
		{
		case MESSAGE_TYPE_GUI_CLICK_START:
		case MESSAGE_TYPE_GUI_CLICK_MOVE:
			CL_Vec2f pt = pVList->Get(1).GetVector2();
			pt += GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

			CL_Rectf r(*m_pPos2d, CL_Sizef(m_pSize2d->x, m_pSize2d->y));
		
			if (!r.contains(pt))
			{
				pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_ALL));
				return;
			} else
			{
				//fine, you may pass
			}
           break;
                
        }

		break;

	case MODE_CLIP_INPUT_TO_SIZE_STRICT:

		//let's only apply this to some messages
		switch (eMessageType( int(pVList->Get(0).GetFloat())))
		{
		case MESSAGE_TYPE_GUI_CLICK_START:
		case MESSAGE_TYPE_GUI_CLICK_MOVE:
		case MESSAGE_TYPE_GUI_CLICK_END:
			CL_Vec2f pt = pVList->Get(1).GetVector2();
			pt += GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

			CL_Rectf r(*m_pPos2d, CL_Sizef(m_pSize2d->x, m_pSize2d->y));

			if (!r.contains(pt))
			{
				pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_ALL));
				return;
			} else
			{
				//fine, you may pass
			}
		}

		break;

	case MODE_DISABLE_INPUT_ALL:
		pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_ALL));
		break;

	case MODE_DISABLE_INPUT_CHILDREN:
		pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_CHILDREN));
		break;

	case MODE_IDLE:

		//let it know it can process this message like normal
		//pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_ALLOW));
		break;

	case MODE_CLIP_INPUT_TO_ABSOLUTE_CLIP_RECT_AND_DISABLE_INPUT_CHILDREN:

		//let's only apply this to some messages
		switch (eMessageType( int(pVList->Get(0).GetFloat())))
		{
		case MESSAGE_TYPE_GUI_CLICK_START:
		case MESSAGE_TYPE_GUI_CLICK_MOVE:
		case MESSAGE_TYPE_GUI_CLICK_END:

			CL_Vec2f pt = pVList->Get(1).GetVector2();
			//pt -= GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

			if (!m_pClipRect->contains(pt))
			{
				pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_ALL));
				return;

			} else
			{
				//fine, you may pass
			}
			
		}

		//let it know it can process this message like normal
		pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_CHILDREN));

		break;

	case MODE_CLIP_INPUT_TO_ABSOLUTE_CLIP_RECT:
	
		//let's only apply this to some messages
		switch (eMessageType( int(pVList->Get(0).GetFloat())))
		{
		case MESSAGE_TYPE_GUI_CLICK_START:
		case MESSAGE_TYPE_GUI_CLICK_MOVE:
		case MESSAGE_TYPE_GUI_CLICK_END:

			CL_Vec2f pt = pVList->Get(1).GetVector2();
		
			if (!m_pClipRect->contains(pt))
			{
				pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_ALL));
				return;

			} else
			{
				//fine, you may pass
			}
			
		}

		break;

	case MODE_IGNORE_ABSOLUTE_CLIP_RECT:

		//let's only apply this to some messages
		switch (eMessageType( int(pVList->Get(0).GetFloat())))
		{
		case MESSAGE_TYPE_GUI_CLICK_START:
		case MESSAGE_TYPE_GUI_CLICK_MOVE:
		case MESSAGE_TYPE_GUI_CLICK_END:

			CL_Vec2f pt = pVList->Get(1).GetVector2();

			if (m_pClipRect->contains(pt))
			{
				pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_ALL));
				return;

			} else
			{
				//fine, you may pass
			}

		}

		break;

	case MODE_CLIP_INPUT_TO_CLIP_RECT:

		//let's only apply this to some messages
		switch (eMessageType( int(pVList->Get(0).GetFloat())))
		{
		case MESSAGE_TYPE_GUI_CLICK_START:
		case MESSAGE_TYPE_GUI_CLICK_MOVE:
		case MESSAGE_TYPE_GUI_CLICK_END:
			CL_Vec2f pt = pVList->Get(1).GetVector2();
			pt += GetAlignmentOffset(CL_Vec2f(m_pClipRect->get_width(), m_pClipRect->get_height()), eAlignment(*m_pAlignment));

			CL_Rectf r(*m_pPos2d, m_pClipRect->get_size());

			if (!r.contains(pt))
			{
				pVList->m_variant[Entity::FILTER_INDEX].Set(uint32(Entity::FILTER_REFUSE_ALL));
				return;
			} else
			{
				//fine, you may pass
			}
		}

		break;

	default:
		LogError("FilterInputComponent: Unknown mode %d", *m_pMode);
	}
}
