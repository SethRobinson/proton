#include "PlatformPrecomp.h"

#include "TapSequenceDetectComponent.h"
#include "BaseApp.h"

TapSequenceDetectComponent::TapSequenceDetectComponent()
{
	SetName("TapSequenceDetect");
}

TapSequenceDetectComponent::~TapSequenceDetectComponent()
{
}


void TapSequenceDetectComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//shared with the rest of the entity
	m_curTapTarget = 0;
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();

	GetFunction("AddTapRegion")->sig_function.connect(1, boost::bind(&TapSequenceDetectComponent::AddTapRegion, this, _1));
	GetParent()->GetFunction("OnTouchStart")->sig_function.connect(1, boost::bind(&TapSequenceDetectComponent::OnTouchStart, this, _1));


	//register ourselves to render if the parent does (for debugging, it shows the tap regions)
	//GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&TapSequenceDetectComponent::OnRender, this, _1));
}

void TapSequenceDetectComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TapSequenceDetectComponent::OnTouchStart(VariantList *pVList)
{
	//LogMsg("Clicked button at %s", PrintVector2(pVList->m_variant[0].GetVector2()).c_str());

	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()-*m_pPos2d;
	
	for (unsigned int i=0; i < m_tapRegions.size(); i++)
	{
		CL_Rectf r = ScaleRect2D(m_tapRegions[i], *m_pScale2d);
		 if (r.contains(vFinalPos))
		 {
			 //LogMsg("Tapped region %d", i);
			 if (m_curTapTarget == i)
			 {
				 m_curTapTarget++;
			 
				 if (m_curTapTarget == m_tapRegions.size())
				 {
					 //they tapped the last one
					 pVList->m_variant[1].Set(GetParent());
					 m_curTapTarget = 0;
					 GetParent()->GetFunction("OnButtonSelected")->sig_function(pVList); //sends a vec2 with position and this entity
				 }
				
				 return;
			 }
		 }
	}

	m_curTapTarget = 0; //reset it, they failed to click something in the correct order
}

void TapSequenceDetectComponent::AddTapRegion(VariantList *pVList)
{
	//LogMsg("Adding %s", PrintRect(pVList->m_variant[0].GetRect()).c_str());
	m_tapRegions.push_back(pVList->m_variant[0].GetRect());
}

void TapSequenceDetectComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	//vFinalPos -= GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));
	
	for (unsigned int i=0; i < m_tapRegions.size(); i++)
	{
		CL_Rectf r = ScaleRect2D(m_tapRegions[i], *m_pScale2d);
		r.translate( CL_Point(vFinalPos));
		DrawRect(r); //useful for debugging our touch hotspot

	}
}