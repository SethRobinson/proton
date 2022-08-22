#include "PlatformPrecomp.h"
#include "CharManagerComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"
#include "GUI/GameMenu.h"
#include "Component/ExplosionComponent.h"

CharComponent *g_pPlayer = NULL;
CharComponent * GetPlayer() 
{
	return g_pPlayer;
}

CharManagerComponent *g_pCharManager = NULL;
CharManagerComponent * GetCharManager() 
{
	return g_pCharManager;
}

CharManagerComponent::CharManagerComponent()
{
	SetName("CharManager");
}

CharManagerComponent::~CharManagerComponent()
{
g_pPlayer = NULL;
g_pCharManager = NULL;
}

void CharManagerComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	g_pCharManager = this;
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
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&CharManagerComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&CharManagerComponent::OnUpdate, this, _1));

	m_pCharFolder = GetParent()->AddEntity(new Entity("chars"));
	
	GetFunction("InitEffect")->sig_function.connect(1, boost::bind(&CharManagerComponent::InitEffect, this, _1));

	Entity *player = m_pCharFolder->AddEntity(new Entity("player"));
	
	CharComponent *pChar = new CharComponent();
	g_pPlayer = pChar;
	pChar->GetVar("ai")->Set(uint32(AI_PLAYER));
	pChar->GetVar("id")->Set(uint32(ID_PLAYER));
	player->AddComponent(pChar);

	//m_chars.push_back(pChar);

	BCell *pCell = GetBuilding()->GetNonWallCellOnFloor(0);

	if (!pCell)
	{
		assert(!"error");
		return;
	}
    VariantList vList(uint32(0), uint32(pCell->m_cellID));
	pChar->GetFunction("SetPositionByFloorAndCell")->sig_function( &vList);
}

void CharManagerComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void CharManagerComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void CharManagerComponent::OnUpdate(VariantList *pVList)
{
}

void CharManagerComponent::AddCharEx( eID id, eAI ai, uint32 floorID, float x )
{
	Entity *pEnt = m_pCharFolder->AddEntity(new Entity("guy"));

	CharComponent *pChar = new CharComponent();
	pChar->GetVar("ai")->Set(uint32(ai));
	pChar->GetVar("id")->Set(uint32(id));

	pEnt->AddComponent(pChar);
	//	m_chars.push_back(pChar);

    VariantList vList(uint32(floorID), (float)x);
	
    pChar->GetFunction("SetPositionByFloorAndX")->sig_function( &vList);

	}

void CharManagerComponent::InitEffect(VariantList *pVList)
{
	CL_Vec2f vWorldPos = pVList->Get(0).GetVector2();
	uint32 effect = pVList->Get(1).GetUINT32();
	
	switch (effect)
	{
	case EFFECT_EXPLODE:
		GetAudioManager()->Play("audio/explode.wav");

		break;

	case EFFECT_STARS:

		GetAudioManager()->Play("audio/get.wav");

		break;
	}
	Entity *pEnt = m_pCharFolder->AddEntity(new Entity("particlesystem"));
	EntityComponent *pEffect= new ExplosionComponent;
	pEffect->GetVar("effect")->Set(effect);
	pEnt->AddComponent(pEffect);
	pEnt->GetVar("pos2d")->Set(vWorldPos);
	KillEntity(pEnt, 6000);
}

void CharManagerComponent::AddChar( eID id, eAI ai, uint32 floorID, uint32 cell )
{
	CL_Vec2f vPos = GetBuilding()->FloorAndCellToWorldPosForCharacter(floorID, cell);
	AddCharEx(id, ai, floorID, vPos.x);
}