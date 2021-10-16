#include "PlatformPrecomp.h"
#include "ActionButtonComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"
#include "../dink/dink.h"

ActionButtonComponent::ActionButtonComponent()
{
	SetName("ActionButton");
	m_mode = MODE_MAGIC;
}

ActionButtonComponent::~ActionButtonComponent()
{

}


void ActionButtonComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();

	if (GetParent()->GetName() != "magic")
	{
		m_mode = MODE_WEAPON;
	}
	/*
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale = &GetParent()->GetShared()->GetVarWithDefault("scale", Variant(1.0f))->GetFloat();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetShared()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetShared()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	*/

	m_pAlpha = &GetParent()->GetShared()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&ActionButtonComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&ActionButtonComponent::OnUpdate, this, _1));

	UpdateIcon();
}

void ActionButtonComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void ActionButtonComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;

	Surface *pSurf;
	
	switch (m_mode)
	{
		case MODE_MAGIC:
		pSurf = DinkGetMagicIconImage();
		break;

		case MODE_WEAPON:
		pSurf = DinkGetWeaponIconImage();
		break;
	}
	
	if (pSurf)
	{
		rtRectf srcRect = pSurf->GetRectf();
		rtRectf dstRect = srcRect;
		
		if (IsLargeScreen())
		{

			dstRect = rtRectf(0,0, 68,68);

		} else
		{

			dstRect = rtRectf(0,0, 44,39);

		}
		
		dstRect.AdjustPosition(vFinalPos.x, vFinalPos.y);
		if (IsLargeScreen())
		{
			dstRect.AdjustPosition(30, 31);
		} else
		{
			dstRect.AdjustPosition(9, 12);

		}

		
		pSurf->BlitEx(dstRect, srcRect, MAKE_RGBA(255* *m_pAlpha,255* *m_pAlpha,255* *m_pAlpha,255* *m_pAlpha));

		if (g_dglo.GetActiveView() == DinkGlobals::VIEW_ZOOMED)
		{

			int alpha =  int(255.0f * *m_pAlpha);

				if (alpha > 2)
				{


					if (m_mode ==MODE_MAGIC)
					{
						float percent = DinkGetMagicChargePercent();
						
						if (percent > 0)
						{

							
							//LogMsg("Magic: %.2f", DinkGetMagicChargePercent());
							rtRectf rBar(0,0, iPhoneMapX2X(54)*percent, iPhoneMapY2X(3));
							rBar.AdjustPosition(vFinalPos.x, vFinalPos.y);
							rBar.AdjustPosition(iPhoneMapX2X(6), iPhoneMapY2X(-4));

							if (percent == 1)
							{
								DrawFilledRect(rBar, MAKE_RGBA(0, 180, 0, 255* *m_pAlpha));
								DrawRect(rBar, MAKE_RGBA(0, 255, 0, 255* *m_pAlpha));

							} else
							{

								DrawFilledRect(rBar, MAKE_RGBA(0, 100, 0, 255* *m_pAlpha));
								DrawRect(rBar, MAKE_RGBA(0, 255, 0, 170* *m_pAlpha));
							}
						}

					} else
					{
						float percent = DinkGetHealthPercent();
						//LogMsg("Magic: %.2f", DinkGetMagicChargePercent());
						rtRectf rBar(0,0, iPhoneMapX2X(54)*percent, iPhoneMapY2X(3));
						rBar.AdjustPosition(vFinalPos.x, vFinalPos.y);
						rBar.AdjustPosition(iPhoneMapX2X(6), iPhoneMapY2X(-4));

						DrawFilledRect(rBar, MAKE_RGBA(180, 0, 0, 255* *m_pAlpha));
						DrawRect(rBar, MAKE_RGBA(255, 0, 0, 255* *m_pAlpha));

					}
					
				}

		}
	}
}

void ActionButtonComponent::OnUpdate(VariantList *pVList)
{
}

void ActionButtonComponent::UpdateIcon()
{
}

Entity * CreateActionButtonEntity(Entity *pParentEnt, string name, string fileName, float x, float y)
{

	Entity *pButtonEnt = CreateOverlayButtonEntity(pParentEnt, name, fileName, x, y);
	pButtonEnt->AddComponent(new ActionButtonComponent);
	return pButtonEnt;
}