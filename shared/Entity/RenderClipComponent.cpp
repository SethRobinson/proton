#include "PlatformPrecomp.h"
#include "RenderClipComponent.h"
#include "BaseApp.h"


RenderClipComponent::RenderClipComponent()
{
	SetName("RenderClip");
	if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
	{
		LogError("ANDROID WARNING: using glclipplane KILLS your fps on the Nexus One, and isn't supported on the G1. You sure you want to use RenderClipComponent?!");
		assert(0);
	}

	if (GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
	{
		LogError("HTML5 warning:  RenderClipComponent disabled for HTML5, the legacy G1 emulation doesn't seem to find the function at all");
		return;
	}
}

RenderClipComponent::~RenderClipComponent()
{
}

void RenderClipComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//register to get updated every frame

	GetParent()->OnFilterAdd();
	GetParent()->GetFunction("FilterOnRender")->sig_function.connect(1, boost::bind(&RenderClipComponent::FilterOnRender, this, _1));
	GetParent()->GetFunction("PostOnRender")->sig_function.connect(1, boost::bind(&RenderClipComponent::PostOnRender, this, _1));

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pClipMode = &GetParent()->GetVarWithDefault("clipMode", uint32(CLIP_MODE_BOTTOM))->GetUINT32();

	//our own vars
}

void RenderClipComponent::OnRemove()
{
	GetParent()->OnFilterRemove();
	EntityComponent::OnRemove();
}

void RenderClipComponent::FilterOnRender(VariantList *pVList)
{

#ifndef PLATFORM_HTML5

	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	//vFinalPos -= GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));	
	glEnable(GL_CLIP_PLANE0);
	
	//clip to the bottom of us.. see the .h to see what I plan to add for more control..
	rtPlane planeA;
	
	switch (*m_pClipMode)
	{
	case CLIP_MODE_BOTTOM:
		planeA = rtPlane(CL_Vec3f(0, vFinalPos.y+m_pSize2d->y,0), CL_Vec3f(0,-1,0));
		break;

	case CLIP_MODE_TOP:
		planeA = rtPlane(CL_Vec3f(0, vFinalPos.y,0), CL_Vec3f(0,1,0));
		break;

	case CLIP_MODE_LEFT:
		planeA = rtPlane(CL_Vec3f(vFinalPos.x, 0,0), CL_Vec3f(-1,0,0));
		break;

	case CLIP_MODE_RIGHT:
		planeA = rtPlane(CL_Vec3f(vFinalPos.x+m_pSize2d->x, 0,0), CL_Vec3f(1,0,0));
		break;

	}
	
	glClipPlane(GL_CLIP_PLANE0, (GLdouble*) planeA.plane);

#endif
}

void RenderClipComponent::PostOnRender(VariantList *pVList)
{
	#ifndef PLATFORM_HTML5
	g_globalBatcher.Flush();
	glDisable(GL_CLIP_PLANE0);
	#endif
}

