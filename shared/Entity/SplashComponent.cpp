#include "PlatformPrecomp.h"
#include "SplashComponent.h"
#include "Entity/EntityUtils.h"


SplashComponent::SplashComponent()
{
	SetName("Splash");
}

SplashComponent::~SplashComponent()
{
}


Entity * SplashComponent::ActivateSplash(SplashScreenItem &item)
{
	Entity *pBG = GetParent()->AddEntity(new Entity("SplashChild"));

	Entity *pImage = CreateOverlayButtonEntity(pBG, "Splash", item.m_fileName, GetScreenSizeXf()/2, GetScreenSizeYf()/2);
	
	SetAlignmentEntity(pImage, ALIGNMENT_CENTER);
	
	if (item.m_bScaleToFitScreen)
	{
		EntitySetScaleBySize(pImage, GetScreenSize(), item.m_bPerserveAspectRatio); //scale to full screen;
	}

	if (item.m_bPerserveAspectRatio || !item.m_bScaleToFitScreen)
	{
		//create a bg rect of the color to fill the dead parts
		Entity *pRect = CreateOverlayRectEntity(pBG, CL_Vec2f(0,0), GetScreenSize(), item.m_bgColor);
		pBG->MoveEntityToBottomByAddress(pRect);
	}

	//note: we use TIMER_SYSTEM so a game pause won't stop this from occurring
	KillEntity(pBG, item.m_timeOutMS, TIMER_SYSTEM); //kill us a timeout period automatically, in case the player doesn't tap us
	//make it skippable by tapping it
	pImage->GetFunction("OnButtonSelected")->sig_function.connect(pBG->GetFunction("OnDelete")->sig_function);
	return pBG;
}

void SplashComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	
	/*
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetShared()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetShared()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetShared()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetShared()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	*/

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&SplashComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&SplashComponent::OnUpdate, this, _1));
}

void SplashComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void SplashComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}


void SplashComponent::AddSplash( string fileName, int timeOutMS, bool bScaleToFitScreen, bool bPerserveAspectRatio, unsigned int bgColor )
{
	m_splashes.push_back(SplashScreenItem());
	SplashScreenItem &item = m_splashes.back();

	item.m_fileName = fileName;
	item.m_timeOutMS = timeOutMS;
	item.m_bScaleToFitScreen = bScaleToFitScreen;
	item.m_bPerserveAspectRatio = bPerserveAspectRatio;
	item.m_bgColor = bgColor;
}

void SplashComponent::OnUpdate(VariantList *pVList)
{
	Entity *pSplashChild = GetParent()->GetEntityByName("SplashChild");

	if (!pSplashChild)
	{
		//either launch one, or signal that we're done by killing ourself
		if (m_splashes.empty())
		{
			//all done!
			GetParent()->SetTaggedForDeletion();
			DestroyUnusedTextures();
		} else
		{
			DestroyUnusedTextures();
			Entity *pEnt = ActivateSplash(m_splashes.front());
			VariantList vList(pEnt,  m_splashes.front().m_fileName);
			m_splashes.pop_front();
			
			//send notification in case anybody is listening to our signal
			m_sig_on_splash_change(&vList);
			
		}
	}
}



