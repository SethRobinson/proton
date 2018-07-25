#include "SpriteAnimationRenderComponent.h"
#include "Renderer/SpriteSheetSurface.h"
#include "Renderer/SpriteAnimation.h"
#include "BaseApp.h"

SpriteAnimationRenderComponent::SpriteAnimationRenderComponent() :
	m_pPos2d(NULL),
	m_pSize2d(NULL),
	m_pScale2d(NULL),
	m_pFrameSize(NULL),
	m_pRotation(NULL),
	m_pColor(NULL),
	m_pColorMod(NULL),
	m_pAlpha(NULL),
	m_pSpriteAnimationSet(NULL),
	m_pSpriteSheet(NULL),
	m_pFileName(NULL),
	m_pAnimationName(NULL),
	m_pPhase(NULL),
	m_pVisible(NULL),
	m_pFlipX(NULL),
	m_pFlipY(NULL),
	m_pCurrentAnimation(NULL)
{
	SetName("SpriteAnimationRender");
}

SpriteAnimationRenderComponent::~SpriteAnimationRenderComponent()
{
}

void SpriteAnimationRenderComponent::UpdateSizeVars()
{
	if (m_pCurrentAnimation)
	{
		CL_Sizef s = m_pCurrentAnimation->GetBoundingBox().get_size();
		GetVar("frameSize2d")->Set(s.width, s.height);

		m_pCurrentAnimBBCenter.x = m_pCurrentAnimBB.left + m_pCurrentAnimBB.right;
		m_pCurrentAnimBBCenter.y = m_pCurrentAnimBB.top + m_pCurrentAnimBB.bottom;
		m_pCurrentAnimBBCenter *= (*m_pScale2d) * 0.5f;
	} else
	{
		GetVar("frameSize2d")->Set(0.0f, 0.0f);

		m_pCurrentAnimBBCenter.x = m_pCurrentAnimBBCenter.y = 0.0f;
	}

	GetParent()->GetVar("size2d")->Set((*m_pFrameSize) * (*m_pScale2d));
}

void SpriteAnimationRenderComponent::OnFileNameChanged(Variant *pFileName)
{
	m_pSpriteAnimationSet = GetResourceManager()->GetSpriteAnimationSet(pFileName->GetString());

	if (m_pSpriteAnimationSet)
	{
		m_pSpriteSheet = GetResourceManager()->GetSurfaceResource<SpriteSheetSurface>(m_pSpriteAnimationSet->GetSpriteSheetImageName());
	}
	
	UpdateSizeVars();
}

void SpriteAnimationRenderComponent::OnScaleChanged(Variant *pDataObject)
{
	UpdateSizeVars();
}

void SpriteAnimationRenderComponent::OnAnimationNameChanged(Variant *pAnimationName)
{
	m_pCurrentAnimation = m_pSpriteAnimationSet->GetAnimation(*m_pAnimationName);

	if (m_pCurrentAnimation != NULL)
	{
		m_pCurrentAnimBB = m_pCurrentAnimation->GetBoundingBox();
	} else
	{
		m_pCurrentAnimBB.left = m_pCurrentAnimBB.top = m_pCurrentAnimBB.right = m_pCurrentAnimBB.bottom = 0.0f;
	}

	GetVar("phase")->Set(0.0f);

	UpdateSizeVars();
}

void SpriteAnimationRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	// shared with the rest of the entity
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(PURE_WHITE))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(PURE_WHITE))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pVisible = &GetParent()->GetVarWithDefault("visible", uint32(1))->GetUINT32();
	
	m_pFlipX = &GetVar("flipX")->GetUINT32();
	m_pFlipY = &GetVar("flipY")->GetUINT32();

	m_pFileName = &GetVar("fileName")->GetString();
	m_pAnimationName = &GetVar("animationName")->GetString();
	m_pPhase = &GetVarWithDefault("phase", 0.0f)->GetFloat();
	
	m_pFrameSize = &GetVarWithDefault("frameSize2d", Variant(0.0f, 0.0f))->GetVector2();

	GetVar("fileName")->GetSigOnChanged()->connect(boost::bind(&SpriteAnimationRenderComponent::OnFileNameChanged, this, _1));
	GetVar("animationName")->GetSigOnChanged()->connect(boost::bind(&SpriteAnimationRenderComponent::OnAnimationNameChanged, this, _1));
	pEnt->GetVar("scale2d")->GetSigOnChanged()->connect(boost::bind(&SpriteAnimationRenderComponent::OnScaleChanged, this, _1));
	
	// register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&SpriteAnimationRenderComponent::OnRender, this, _1));
}

void SpriteAnimationRenderComponent::OnRender(VariantList *pVList)
{
	if (*m_pVisible == 0) return;

	if (m_pCurrentAnimation && m_pSpriteSheet && m_pSpriteSheet->IsLoaded() && *m_pAlpha > 0.01)
	{
		if (m_pScale2d->x == 0 || m_pScale2d->y == 0) return;

		CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2() + *m_pPos2d;

		vFinalPos += m_pCurrentAnimBBCenter;

		if (*m_pRotation == 0) // if rotation is enabled, we don't do this early exit check as it could be incorrect
		{
			if (vFinalPos.y < -m_pSize2d->y || vFinalPos.y > GetOrthoRenderSizeYf()) return;
		}
	
		unsigned int finalColor = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);
		if (GET_ALPHA(finalColor) == 0) return;

		CL_Vec2f vRotationPt(vFinalPos);
		vRotationPt += *m_pSize2d / 2;

		const SpriteFrame* frame = m_pCurrentAnimation->GetFrameAtPhase(*m_pPhase);
		for (int i = 0; i < frame->GetCellCount(); ++i)
		{
			const SpriteCell* cell = frame->GetCell(i);
			const CL_Rectf& cellBB(cell->GetBoundingBox());
			m_pSpriteSheet->BlitFrame(vFinalPos.x + (cellBB.left - m_pCurrentAnimBB.left) * m_pScale2d->x, vFinalPos.y + (cellBB.top - m_pCurrentAnimBB.top) * m_pScale2d->y, cell->GetSpriteName(), *m_pScale2d, finalColor, *m_pRotation, vRotationPt, *m_pFlipX != 0, *m_pFlipY != 0);
		}
	}
}
