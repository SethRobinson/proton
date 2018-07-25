#include "PlatformPrecomp.h"
#include "TrailRenderComponent.h"
#include "BaseApp.h"

TrailRenderComponent::TrailRenderComponent()
{
	SetName("TrailRender");
}

TrailRenderComponent::~TrailRenderComponent()
{
}

void TrailRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_insideTrailDrawingNow = false;
	m_frameRecordTimer= 0;
	m_timingSystem = GetBaseApp()->GetActiveTimingSystem();

	//shared with the rest of the entity
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees
	m_pTrailAlpha = &GetParent()->GetVarWithDefault("trailAlpha", 0.5f)->GetFloat();  
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&TrailRenderComponent::OnRender, this, _1));
	
	//our own variables/settings
	m_pFrames = &GetVarWithDefault("frames", uint32(5))->GetUINT32(); 
	m_pTimeBetweenFramesMS = &GetVarWithDefault("timeBetweenFramesMS", uint32(50))->GetUINT32(); 

}

void TrailRenderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TrailRenderComponent::SetFrameFromEntity(TrailFrame *pFrame)
{
	pFrame->m_alpha = *m_pAlpha;
	pFrame->m_color = *m_pColor;
	pFrame->m_colorMod = *m_pColorMod;
	pFrame->m_pos2d = *m_pPos2d;
	pFrame->m_rotation = *m_pRotation;
	pFrame->m_scale2d = *m_pScale2d;
	pFrame->m_size2d = *m_pSize2d;
}

void TrailRenderComponent::SetEntityFromFrame(TrailFrame *pFrame)
{
	*m_pAlpha = pFrame->m_alpha;
	*m_pColor = pFrame->m_color;
	*m_pColorMod = pFrame->m_colorMod;
	*m_pPos2d = pFrame->m_pos2d;
	*m_pRotation = pFrame->m_rotation;
	*m_pScale2d = pFrame->m_scale2d;
	*m_pSize2d = pFrame->m_size2d;
}

void TrailRenderComponent::OnRender(VariantList *pVList)
{
	if (m_insideTrailDrawingNow) return; //currently drawing our trail, get out or we'll cause an inifinite loop
	
	m_insideTrailDrawingNow = true;
	//save current settings
	TrailFrame tempFrame;
	SetFrameFromEntity(&tempFrame);

	//render out all history..

	deque<TrailFrame>::iterator itor = m_history.begin();

	float frame = 1;

	while (itor != m_history.end())
	{
	     SetEntityFromFrame(&*itor);
		 
		 //tweak the alpha a bit
		 *m_pAlpha = *m_pAlpha * *m_pTrailAlpha * (frame / float(m_history.size())+2);
		 		 
		 //tweak the scale a bit
		 //*m_pScale2d = *m_pScale2d *(frame / (float(m_history.size())+2));
		 GetParent()->CallFunctionRecursivelyWithUpdatedVar("OnRender", pVList, string("pos2d"), 0, Entity::RECURSIVE_VAR_OP_ADDITION); 
		 itor++;
		 frame++;
	}

	//put it back
	SetEntityFromFrame(&tempFrame);

	//add current data to our history?

	if (m_frameRecordTimer < GetTick(m_timingSystem))
	{
		m_history.push_back(tempFrame);
		
		while (m_history.size() > *m_pFrames) 
		{
			m_history.pop_front();
		}

		m_frameRecordTimer = GetTick(m_timingSystem)+*m_pTimeBetweenFramesMS;
	}

	m_insideTrailDrawingNow = false;
}