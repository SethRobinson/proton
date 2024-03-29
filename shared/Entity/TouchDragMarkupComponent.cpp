#include "PlatformPrecomp.h"
#include "TouchDragMarkupComponent.h"
#include "EntityUtils.h"


int g_markupPenSize = 20;

void SetGlobalMarkupPenSize(int size)
{
	g_markupPenSize = size;
}

int GetGlobalMarkupPenSize()
{
	return g_markupPenSize;
}

TouchDragMarkupComponent::TouchDragMarkupComponent()
{
	SetName("TouchDragMarkup");
}

TouchDragMarkupComponent::~TouchDragMarkupComponent()
{
	m_pEntityToFollow = NULL;
}

void TouchDragMarkupComponent::ResetTouch()
{
	if (m_bIsDraggingLook)
	{
		//m_bIgnoreNextMove = true;
	}
}

void TouchDragMarkupComponent::OnAdd(Entity* pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVar("scale2d")->GetVector2();

	Variant v;
	
	m_pEntityToFollow = GetVarWithDefault("entityToFollow", (Entity*) NULL);
	m_pEntityToFollow->GetSigOnChanged()->connect(1, boost::bind(&TouchDragMarkupComponent::OnEntityToFollowChanged, this, _1));

	EntityComponent *pDragComponent = GetParent()->GetComponentByName("TouchDrag");
	
	if (!pDragComponent)
	{
		assert(0 && "TouchDragMarkupComponent requires a TouchDragComponent to be added first");
		return;
	}

	pDragComponent->GetFunction("OnTouchDragUpdate")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnTouchDragUpdate, this, _1));
	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnOverStart, this, _1));
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnOverEnd, this, _1));
	GetParent()->GetFunction("ClearActiveMarkups")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnClearActiveMarkups, this, _1));

	//register to get notified if scalexy changes
	GetParent()->GetVar("scale2d")->GetSigOnChanged()->connect(1, boost::bind(&TouchDragMarkupComponent::OnScaleChanged, this, _1));
	
	//register to get notified if sizexy changes
	//GetParent()->GetVar("size2d")->GetSigOnChanged()->connect(1, boost::bind(&TouchDragMarkupComponent::OnSizeChanged, this, _1));

	//register to get updated every frame
	//GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&TouchDragComponent::OnInput, this, _1));

	InitMarkupBoard();
	m_surface.InitFromSoftSurface(&m_softSurf);
	m_surface.SetUsesAlpha(true);
	OverlayRenderComponent *m_pOverlayComp = (OverlayRenderComponent *) GetParent()->AddComponent(new OverlayRenderComponent());
	m_pOverlayComp->SetSurface(&m_surface, false);
	m_surface.SetupSignalsForUnloadingAndLoadingTextures();
	//we want to get notified as well
	GetBaseApp()->m_sig_enterforeground.connect(1, boost::bind(&TouchDragMarkupComponent::OnEnterForeground, this, _1));
	GetBaseApp()->m_sig_loadSurfaces.connect(1, boost::bind(&TouchDragMarkupComponent::OnLoadSurfaces, this));
	//register to get updated every frame
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnUpdate, this, _1));
}

void TouchDragMarkupComponent::OnEnterForeground(VariantList* pVList)
{
	/*
	m_softSurf.FlipY();
	m_softSurf.UpdateGLTexture(&m_surface);
	m_softSurf.FlipY();
	*/
}
void TouchDragMarkupComponent::OnLoadSurfaces()
{
	
	m_softSurf.FlipY();
	m_softSurf.UpdateGLTexture(&m_surface);
	m_softSurf.FlipY();
	
}


void TouchDragMarkupComponent::OnEntityToFollowChanged(Variant* pDataObject)
{
	
	if (m_pEntityToFollow->GetEntity() != NULL)
	{
		LogMsg("Entity to follow changed to %s", m_pEntityToFollow->GetEntity()->GetName().c_str());
		m_pEntityToFollow->GetEntity()->GetVar("MarkupComponentFollowingUs")->Set(this);//in case they care
		//if this entity is destroyed, destroy us too
		m_pEntityToFollow->GetEntity()->GetFunction("OnDelete")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnEntityWeFollowGotDeleted, this, _1));
	} else
	{
		//We should remove signals to/from it
	} 
}

void TouchDragMarkupComponent::OnEntityWeFollowGotDeleted(VariantList* pVList)
{
	LogMsg("Entity we follow got deleted, deleting us too");
	m_pEntityToFollow->Set((Entity*)NULL);
	GetParent()->SetTaggedForDeletion();
}

void TouchDragMarkupComponent::OnUpdate(VariantList* pVList)
{
	if (!m_pEntityToFollow || GetParent()->GetTaggedForDeletion()) return;
	
	Entity* pEntToFollow = m_pEntityToFollow->GetEntity();

	if (!pEntToFollow) return;

	SetPos2DEntity(GetParent(), pEntToFollow->GetVar("pos2d")->GetVector2());
	
	//GetParent()->GetParent()->MoveEntityToTopByAddress(GetParent());
	GetParent()->GetParent()->MoveEntityToTopOfOtherEntityByAddress(GetParent(), pEntToFollow);
	EntitySetScaleBySize(GetParent(), pEntToFollow->GetVar("size2d")->GetVector2());
	//LogMsg("Update: Setting size to %s", PrintVector2(m_pEntityToFollow->GetEntity()->GetVar("size2d")->GetVector2()).c_str() );
	//LogMsg("Entity's scale is %s, my scale is %s", PrintVector2(pEntToFollow->GetVar("scale2d")->GetVector2()).c_str(), PrintVector2(GetParent()->GetVar("scale2d")->GetVector2()).c_str());
}

void TouchDragMarkupComponent::InitMarkupBoard()
{
	m_softSurf.Init(2048, 2048, SoftSurface::SURFACE_RGBA);
	m_softSurf.FillColor(glColorBytes(255, 255, 255, 0));
}

void TouchDragMarkupComponent::OnScaleChanged(Variant* pDataObject)
{
	//LogMsg("Markup surface changed to %s", PrintVector2(*m_pSize2d).c_str());
	ResetTouch();
}

void TouchDragMarkupComponent::OnSizeChanged(Variant* pDataObject)
{
	//LogMsg("Size:  %s  Scale: %s", PrintVector2(*m_pSize2d).c_str(), PrintVector2(*m_pScale2d).c_str());
}

void TouchDragMarkupComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TouchDragMarkupComponent::UpdateStatusMessage(string msg)
{
	int timeMS = 1000;

	Entity *pOldEnt = GetParent()->GetEntityByName("DebugText");
	
	if (pOldEnt)
	{
		pOldEnt->SetName("");
		pOldEnt->SetTaggedForDeletion();
	}

	Entity* pEnt = CreateTextLabelEntity(GetParent(), "DebugText", 0, 0, msg);
	SetupTextEntity(pEnt, FONT_LARGE, 0.66f);
	FadeOutAndKillEntity(pEnt, true, 100, timeMS);
}

void TouchDragMarkupComponent::OnTouchDragUpdate(VariantList* pVList)
{
	int fingerID = pVList->Get(2).GetUINT32(); //0 is left mouse button
	if (fingerID != m_fingerIDToTrack) return; //only allow left mouse button to drag

	CL_Vec2f vMovement = pVList->Get(1).GetVector2();
	//CL_Vec2f vCurPos = pVList->Get(3).GetVector2();
	CL_Vec2f vCurPos = GetBaseApp()->GetTouch(fingerID)->GetPos();

#ifdef _DEBUG
	//LogMsg("pos: %s, offset %s", PrintVector2(vCurPos).c_str(), PrintVector2(vMovement).c_str());
#endif
 	if (m_bIsDraggingLook)
	{
		//*m_pPos2d += vMovement;
	
		if (!m_bIgnoreNextMove)
		{
		
			CL_Vec2f vPos = vCurPos+ vMovement;
			eAlignment align = eAlignment(GetParent()->GetVar("alignment")->GetUINT32());
			vPos -= *m_pPos2d - GetAlignmentOffset(*m_pSize2d, align);

			Draw(vPos);

			//UpdateStatusMessage("Drawing at X: " + toString(vPos.x) + " Y: " + toString(vPos.y));
			//m_startPos = m_startPos + vMovement;
		}

		m_bIgnoreNextMove = false;
	}
}

void TouchDragMarkupComponent::OnOverStart(VariantList* pVList)
{

	int fingerID =  pVList->Get(2).GetUINT32(); //0 is left mouse button
	
	//LogMsg("Detected fingerID %d", fingerID);
	if (fingerID != m_fingerIDToTrack) return; //only allow left mouse button to drag
	m_lastPosDrawn = CL_Vec2f(-1, -1);

	m_bIsDraggingLook = true;

	//move it to the top layer
	//GetParent()->GetParent()->MoveEntityToTopByAddress(GetParent());
	
	CL_Vec2f vPos = pVList->Get(0).GetVector2();

	vPos -= *m_pPos2d;
	m_startPos = vPos;
	Draw(vPos);

}
void TouchDragMarkupComponent::Draw(CL_Vec2f vEndPos)
{
	//LogMsg("Trying to draw at %s", PrintVector2(vEndPos).c_str());

	// Convert to local coordinates of our surface
	vEndPos.x = vEndPos.x / m_pScale2d->x;
	vEndPos.y = vEndPos.y / m_pScale2d->y;

	//LogMsg("Now it's %s", PrintVector2(vEndPos).c_str());

	CL_Vec2f vStartPos = m_lastPosDrawn;

	// Check if we have a valid last position
	if (vStartPos != CL_Vec2f(-1, -1))
	{
		// Calculate the distance between vStartPos and vEndPos
		float dx = vEndPos.x - vStartPos.x;
		float dy = vEndPos.y - vStartPos.y;
		float distance = sqrt(dx * dx + dy * dy);

		int steps = (int)distance; // Number of steps based on pixel distance
		if (steps < 1) steps = 1; // Ensure at least one step

		for (int i = 0; i <= steps; i++)
		{
			float t = (float)i / steps;
			float interpolatedX = vStartPos.x + t * dx;
			float interpolatedY = vStartPos.y + t * dy;
			CL_Vec2f interpolatedPos(interpolatedX, interpolatedY);

			m_softSurf.DrawCircleSafe((int)interpolatedPos.x, (int)interpolatedPos.y, glColorBytes(255, 0, 0, 255), GetGlobalMarkupPenSize());
		}
	}
	else
	{
		// Draw a single circle at the end position if we don't have a valid start position
		m_softSurf.DrawCircleSafe((int)vEndPos.x, (int)vEndPos.y, glColorBytes(255, 0, 0, 255), GetGlobalMarkupPenSize());
	}

	m_lastPosDrawn = vEndPos;

	m_softSurf.FlipY();
	m_softSurf.UpdateGLTexture(&m_surface);
	m_softSurf.FlipY();
}

void TouchDragMarkupComponent::OnOverEnd(VariantList* pVList)
{
	int fingerID = pVList->Get(2).GetUINT32(); //0 is left mouse button
	if (fingerID != m_fingerIDToTrack) return; //only allow left mouse button to drag

	m_bIsDraggingLook = false;
}

void TouchDragMarkupComponent::OnClearActiveMarkups(VariantList* pVList)
{
	LogMsg("%s clearing markups", GetParent()->GetName().c_str());

	InitMarkupBoard();
	m_softSurf.FlipY();
	m_softSurf.UpdateGLTexture(&m_surface);
	m_softSurf.FlipY();
}

