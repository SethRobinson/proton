#include "PlatformPrecomp.h"

#include "ScrollBarRenderComponent.h"
#include "util/GLESUtils.h"
#include "BaseApp.h"
#include "Entity/EntityUtils.h"

ScrollBarRenderComponent::ScrollBarRenderComponent()
{
	m_pSurf = NULL;
	SetName("ScrollBarRender");
	m_bUsingScrollComponent = false;
	m_isCapsuleDragging = false;
	m_pScrollComp = NULL;
}


ScrollBarRenderComponent::~ScrollBarRenderComponent()
{

}

void ScrollBarRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	//shared with the rest of the entity
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(0.3f))->GetFloat();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(224,188,130,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	
	m_pFileName = &GetVar("fileName")->GetString(); //local to us
	
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&ScrollBarRenderComponent::OnUpdate, this, _1));
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&ScrollBarRenderComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&ScrollBarRenderComponent::OnTargetOverStart, this, _1));
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&ScrollBarRenderComponent::OnTargetOverEnd, this, _1));
	GetParent()->GetFunction("OnOverMove")->sig_function.connect(1, boost::bind(&ScrollBarRenderComponent::OnTargetOverMove, this, _1));

	//if "fileName" is set, we'll know about it here
	GetVar("fileName")->GetSigOnChanged()->connect(boost::bind(&ScrollBarRenderComponent::OnFileNameChanged, this, _1));

	GetVar("fileName")->Set("interface/scroll_bar_caps.rttex"); //default

	m_pScrollComp = (ScrollComponent*) GetParent()->GetComponentByName("Scroll");
	if (!m_pScrollComp)
	{
		//assume our stuff will get set from the outside
		m_pBoundsRect = &GetParent()->GetVar("boundsRect")->GetRect();
		m_pProgress2d = &GetParent()->GetVar("progress2d")->GetVector2();
		
	} else
	{
		m_bUsingScrollComponent = true; //I keep track of this becuse it looks like the bounds is calculated a little
		//differently with scroll components.. ??
		m_pBoundsRect = &m_pScrollComp->GetVar("boundsRect")->GetRect();
		m_pProgress2d = &m_pScrollComp->GetVar("progress2d")->GetVector2();
	}

}


void ScrollBarRenderComponent::OnFileNameChanged(Variant *pDataObject)
{
	SAFE_DELETE(m_pSurf);

	m_pSurf = GetResourceManager()->GetSurfaceAnim(pDataObject->GetString());
	if (m_pSurf)
	{
		m_pSurf->SetupAnim(1,2);
	} else
	{
		LogMsg("ScrollBarRenderComponent: Can't load scroll bar pieces");
	}

}

bool ScrollBarRenderComponent::GetRectOfScrollCapsule(CL_Rectf *pRectout)
{

	float barHeight;
	float barWidth;
	CL_Vec2f vFinalPos;
	float contentAreaRatio;
	contentAreaRatio = (m_pBoundsRect->get_height() + m_pSize2d->y) / m_pSize2d->y;

	if (!m_bUsingScrollComponent && m_pBoundsRect->get_height() < (m_pSize2d->y + 1)) //I don't really know why I need that +1..but it works..
	{
		contentAreaRatio = 0; //definitely don't need to scroll here
	}

	if (contentAreaRatio > 1)
	{
		//render vertical scroll bar
	
		const float touchPaddingWidth = 3;
		barHeight = GetCapsuleHeight()+touchPaddingWidth;
		barWidth = m_pSurf->GetFrameWidth()+touchPaddingWidth;

//		LogMsg("percent scrolled is %.2f, contentAreaRation is %.2f", m_pProgress2d->y, contentAreaRatio);

		vFinalPos = *m_pPos2d + CL_Vec2f(m_pSize2d->x, 0);

		if (vFinalPos.x >= GetScreenSizeXf())
		{
			//position on the inside, not the outside
			vFinalPos.x -= (barWidth + (iPadMapX(8))); //adjust the spacer with the screensize
		}
		//slide it down to the right position:
		vFinalPos.y += (m_pSize2d->y - barHeight) * m_pProgress2d->y;

		
		CL_Rectf r = CL_Rectf(0, 0, barWidth, barHeight);
		ApplyOffset(&r, vFinalPos);
		//DrawFilledRect(r, *m_pColor);
		*pRectout = r;
		return true;
	}

	return false;
}

void ScrollBarRenderComponent::StartCapsuleDrag(CL_Vec2f vDragOffset)
{
	m_isCapsuleDragging = true;
	m_capsuleDragOffset = vDragOffset;
	if (m_pScrollComp)
	{
		m_pScrollComp->SetDraggingByContentEnabled(false);
	}
	//a drag is started
	//LogMsg("Start drag...");
}

void ScrollBarRenderComponent::StopCapsuleDrag()
{
	m_isCapsuleDragging = false;
	//LogMsg("Stop Drag...");
	if (m_pScrollComp)
	{
		m_pScrollComp->SetDraggingByContentEnabled(true);
	}
}

void ScrollBarRenderComponent::OnTargetOverStart(VariantList *pVList)
{	
	CL_Vec2f vMousePos = pVList->m_variant[0].GetVector2();
	int touchID = pVList->m_variant[2].GetUINT32();

	CL_Rectf r;
	bool bIsUsed = GetRectOfScrollCapsule(&r);

	if (bIsUsed && touchID == 0)
	{
		//LogMsg("Clicked %s, touch id %d.  Capsule rect is %s", PrintVector2(vMousePos).c_str(), touchID, PrintRect(r).c_str());
		
		if (r.contains(vMousePos))
		{
			StartCapsuleDrag(r.get_center() - vMousePos);
		
			GetBaseApp()->GetTouch(0)->SetWasHandled(true, GetParent());
		}

	}
	FadeEntity(GetParent(), false, 0.6f, 100);
}

int ScrollBarRenderComponent::GetCapsuleHeight()
{
	float contentAreaRatio;
	contentAreaRatio = (m_pBoundsRect->get_height() + m_pSize2d->y) / m_pSize2d->y;
	float barHeight = m_pSize2d->y / contentAreaRatio;

	if (barHeight < m_pSurf->GetFrameHeight() * 5) barHeight = m_pSurf->GetFrameHeight() * 5;

	return barHeight;
}
void ScrollBarRenderComponent::OnTargetOverMove(VariantList* pVList)
{
	if (!m_isCapsuleDragging) return; //don't care

	CL_Vec2f vMousePos = pVList->m_variant[0].GetVector2();
	int touchID = pVList->m_variant[2].GetUINT32();
	Entity* pPosParent = pVList->Get(1).GetEntity();

	if (touchID != 0) return; //only care about the first touch (for now
	GetBaseApp()->GetTouch(0)->SetWasHandled(true, GetParent());

	float barHeight = GetCapsuleHeight();

	CL_Vec2f percentProgressTemp = *m_pProgress2d;

	//this math is suspect to say the least.  It doesn't quite work right
	percentProgressTemp.y = ( ((vMousePos.y - (barHeight/2))) / (m_pSize2d->y - barHeight))-0.05f;
	
	if (m_pScrollComp)
	{
		//m_pScrollComp->SetPositionByPercent(percentProgressTemp);
		VariantList vList;
		vList.Get(0).Set(percentProgressTemp);
		m_pScrollComp->SetProgress(&vList);
	}

	//LogMsg("updating drag... mouse: %s,Offset: %s Temp is %s,  final progress is %s", PrintVector2(vMousePos).c_str(), 
	//	PrintVector2(m_capsuleDragOffset).c_str(), PrintVector2(percentProgressTemp).c_str(),  PrintVector2(*m_pProgress2d).c_str());

}

void ScrollBarRenderComponent::OnTargetOverEnd(VariantList *pVList)
{	
	CL_Vec2f vMousePos = pVList->m_variant[0].GetVector2();
	int touchID = pVList->m_variant[2].GetUINT32();

	if (m_isCapsuleDragging && touchID == 0)
	{
		StopCapsuleDrag();
	}


	FadeEntity(GetParent(), false, 0.3f, 1000);
}

void ScrollBarRenderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void ScrollBarRenderComponent::OnUpdate(VariantList *pVList)
{	
}

void ScrollBarRenderComponent::OnRender(VariantList *pVList)
{
	CHECK_GL_ERROR();
	//LogMsg("Drawing progress bar: %.2f", progress);
	if (*m_pAlpha <= 0.07)
	{
		return; //not ready
	}

	float contentAreaRatio;
	
	GLboolean bScissorEnabled = false;
	glGetBooleanv(GL_SCISSOR_TEST, &bScissorEnabled);

	if (bScissorEnabled)
	{
		g_globalBatcher.Flush();
		//disable it temporarily
		glDisable(GL_SCISSOR_TEST);
	}
	float barHeight;
	float barWidth;
	CL_Vec2f vFinalPos;
	uint32 color = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);

	if (!m_pSurf) return; //can't do anything without the graphics loaded
	
	contentAreaRatio = (m_pBoundsRect->get_height()+m_pSize2d->y)/m_pSize2d->y;

	if (!m_bUsingScrollComponent && m_pBoundsRect->get_height() < (m_pSize2d->y+1)) //I don't really know why I need that +1..but it works..
	{
		contentAreaRatio = 0; //definitely don't need to scroll here
	}

	if (contentAreaRatio > 1)
	{

		barHeight = GetCapsuleHeight();
		barWidth = m_pSurf->GetFrameWidth();
		//LogMsg("percent scrolled is %.2f, contentAreaRation is %.2f", m_pProgress2d->y, contentAreaRatio);

		vFinalPos = pVList->m_variant[0].GetVector2()+ *m_pPos2d + CL_Vec2f(m_pSize2d->x, 0);

		if (vFinalPos.x >= GetScreenSizeXf())
		{
			//position on the inside, not the outside
			vFinalPos.x -= ( barWidth+(iPadMapX(8) )); //adjust the spacer with the screensize
		}
		//slide it down to the right position:
		vFinalPos.y += (m_pSize2d->y - barHeight)* m_pProgress2d->y;

		//draw the top of the capsule
		m_pSurf->BlitAnim(vFinalPos.x, vFinalPos.y,0,0, color);
		vFinalPos.y += m_pSurf->GetFrameHeight(); 
		barHeight -=  m_pSurf->GetFrameHeight()*2;
		//draw the bottom end cap
		m_pSurf->BlitAnim(vFinalPos.x, vFinalPos.y+barHeight,0,1, color);
		//first draw the first end cap
		CL_Rectf r = CL_Rectf(0, 0, barWidth, barHeight);
		ApplyOffset(&r, vFinalPos);
		DrawFilledRect(r, color);
	}

	contentAreaRatio = (m_pBoundsRect->get_width()+m_pSize2d->x)/m_pSize2d->x;

	if (!m_bUsingScrollComponent && m_pBoundsRect->get_width() < (m_pSize2d->x+1)) //I don't really know why I need that +1..but it works..
	{
		contentAreaRatio = 0; //definitely don't need to scroll here
	}
	
	if (contentAreaRatio > 1)
	{
		//render horizontal scroll bar
		m_pSurf->SetupAnim(2,1); //repurpose the graphics for horizontal...

		barWidth = m_pSize2d->x/contentAreaRatio;

		if (barWidth < m_pSurf->GetFrameWidth()*2) barWidth = m_pSurf->GetFrameWidth()*2;

		barHeight= m_pSurf->GetFrameHeight();
		//LogMsg("percent scrolled is %.2f, contentAreaRation is %.2f", m_pProgress2d->x, contentAreaRatio);

		vFinalPos = pVList->m_variant[0].GetVector2()+ *m_pPos2d + CL_Vec2f(0, m_pSize2d->y);

		if (vFinalPos.y >= GetScreenSizeYf())
		{
			//position on the inside, not the outside
			vFinalPos.y -= ( barHeight+(iPadMapY(6) )); //adjust the spacer with the screensize
		}
		//slide it down to the right position:
		vFinalPos.x += (m_pSize2d->x - barWidth)* m_pProgress2d->x;

		//draw the top of the capsule
		m_pSurf->BlitAnim(vFinalPos.x, vFinalPos.y,0,0, color);
		vFinalPos.x += m_pSurf->GetFrameWidth(); 
		barWidth -=  m_pSurf->GetFrameWidth()*2;
		//draw the bottom end cap
		m_pSurf->BlitAnim(vFinalPos.x+barWidth, vFinalPos.y,1,0, color);
		//first draw the first end cap
		CL_Rectf r = CL_Rectf(0, 0, barWidth, barHeight);
		ApplyOffset(&r, vFinalPos);
		DrawFilledRect(r, color);
		CHECK_GL_ERROR();

	}
	
	
	if (bScissorEnabled)
	{
		g_globalBatcher.Flush();
		glEnable(GL_SCISSOR_TEST);
	}
	CHECK_GL_ERROR();

}