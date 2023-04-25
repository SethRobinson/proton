#include "PlatformPrecomp.h"
#include "CharComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"
#include "GUI/GameMenu.h"
#include "OverlayRenderComponentSpy.h"
#include "CharManagerComponent.h"

const float C_PLAYER_SPEED = 5.0f;


CharComponent::CharComponent()
{
	SetName("Char");
}

CharComponent::~CharComponent()
{
}

void CharComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
m_moveASAP = false;
	m_soundTimer = 0;
	m_updateThink = 0;
	m_state = STATE_STOP;

	GetParent()->GetFunction("OnDamage")->sig_function.connect(1, boost::bind(&CharComponent::OnDamage, this, _1));

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
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&CharComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&CharComponent::OnUpdate, this, _1));
	GetFunction("SetPositionByFloorAndCell")->sig_function.connect(1, boost::bind(&CharComponent::SetPositionByFloorAndCell, this, _1));
	GetFunction("SetPositionByFloorAndX")->sig_function.connect(1, boost::bind(&CharComponent::SetPositionByFloorAndX, this, _1));
	GetFunction("Explode")->sig_function.connect(1, boost::bind(&CharComponent::Explode, this, _1));
	GetFunction("PreExplode")->sig_function.connect(1, boost::bind(&CharComponent::PreExplode, this, _1));
	


	m_pFloorID = &GetVar("floorID")->GetUINT32();
	m_pPaused = &GetVar("paused")->GetUINT32();
	m_pSpeed = &GetVarWithDefault("speed", 15.0f)->GetFloat();  
	m_pID = &GetVar("id")->GetUINT32();
	m_pAI = &GetVar("ai")->GetUINT32();

	EntityComponent *pComp = pEnt->AddComponent(new OverlayRenderComponentSpy());
	
	
switch(*m_pID)
{
case ID_PLAYER:
	pComp->GetVar("fileName")->Set("game/hero.rttex"); //local to component

	break;

case ID_WEAK_ENEMY:
	pComp->GetVar("fileName")->Set("game/robot.rttex"); //local to component
	//GetParent()->GetVar("colorMod")->Set(MAKE_RGBA(255,0,0,255));
	break;

case ID_DYNAMITE:
	{
	pComp->GetVar("fileName")->Set("game/items.rttex"); //local to component
	//GetParent()->GetVar("colorMod")->Set(MAKE_RGBA(255,0,0,255));
	
	EntityComponent *pPulsate = PulsateColorEntity(pEnt, false, MAKE_RGBA(255,100,100,255));
	pPulsate->GetVar("duration_ms")->Set(uint32(300));
	//GetMessageManager()->SetComponentVariable(pPulsate, 1000, "duration_ms", Variant(uint32(200)));
	
	GetMessageManager()->CallComponentFunction(this, 800, "PreExplode", NULL);

	GetMessageManager()->CallComponentFunction(this, 1000, "Explode", NULL);
	

	}

	break;

default:
	assert(!"huh");

}

	SetupAnimEntity(GetParent(), 8, 1, 0, 0);
	SetAlignmentEntity(GetParent(), ALIGNMENT_DOWN_CENTER);
	
	//LogMsg("Character %s created", GetName().c_str());
}

void CharComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void CharComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void CharComponent::UpdateMove()
{
	
	//if (m_updateThink > GetTick(TIMER_GAME)) return; //not yet

	float amount = C_PLAYER_SPEED;
	if (m_state == STATE_MOVE_LEFT) amount *= -1;
	uint32 timeMS = 40;
	
	BCell *pCell = GetBuilding()->GetCellByWorldPos((*m_pPos2d)+CL_Vec2f(amount, 0));
	if (pCell->m_type == BCell::TYPE_WALL)
	{
		//blocked?
		if (m_state == STATE_MOVE_RIGHT && amount > 0)
		{
			amount = 0;
		} else if (m_state == STATE_MOVE_LEFT && amount < 0)
		{
			amount = 0;
		}
	}
	
	SetPos2DEntity(GetParent(), GetPos2DEntity(GetParent()) + CL_Vec2f(amount*GetBaseApp()->GetDelta(), 0));
	//LogMsg("Moving %.2f", amount);
	//ZoomToPositionOffsetEntity(GetParent(), CL_Vec2f(amount, 0), timeMS, INTERPOLATE_LINEAR);
	
	
	m_updateThink = GetTick(TIMER_GAME)+ timeMS;
	
	if (m_soundTimer < GetTick(TIMER_GAME))
	{
		GetAudioManager()->Play("audio/walk_blip.wav");
		m_soundTimer = GetTick(TIMER_GAME)+ 300;
	}
}

void CharComponent::PlayerAI()
{

	if (m_moveASAP)
	{
		//LogMsg("Doing MOVE ASAP");
		m_moveASAP = false;
		Move(m_moveASAPRight, m_moveASAPButtonDown);
	}


	BCell *pCell = GetBuilding()->GetCellByWorldPos(*m_pPos2d);
	if (pCell->m_type == BCell::TYPE_DOOR1)
	{
		CL_Vec2f vLadderPos;
		if (GetBuilding()->IsCloseToDoor(*m_pFloorID, m_pPos2d->x, vLadderPos))
		{

			vLadderPos.y += 80;
			GetBuilding()->EraseCellByWorldPos(vLadderPos);
			
            VariantList vList(vLadderPos, EFFECT_STARS);
            GetMessageManager()->CallComponentFunction(GetCharManager(), 1, "InitEffect", &vList);

			OnGotDoor();
	
			//give power up?

			int r = Random(12);

			if (r == 0)
			{
				ShowQuickMessage("SHE GIVES YOU AN EXTRA LIFE!");
				OnModLivesLost(1);
			}
			if (r > 5)
			{
				ShowQuickMessage("SHE GIVES YOU A DYNAMITE!");
				OnModDyno(1);
			}
		}
	}

	switch (m_state)
	{
	case STATE_MOVE_LEFT:
	case STATE_MOVE_RIGHT:

		UpdateMove();
		break;

	case STATE_STOP:
		break;
	}

}

void CharComponent::Damage(CL_Vec2f *pVDamPos, CharComponent *pEnemy, float radius, float damage)
{
	if ( (*pVDamPos-*m_pPos2d).length() > radius) return; //missed
	
	if (*m_pPaused == 1) return;

	switch (*m_pID)
	{
	  case ID_PLAYER:
		  {
			  Stop();
			  m_updateThink = 0;
			  *m_pPaused = 1;
			  //ShowQuickMessage("You died!");
			  GetAudioManager()->Play("audio/death.wav");
			  FadeEntity(GetParent(), true, 0.4f, 200, 100, true);
			  //SetAlignmentEntity(GetParent(), ALIGNMENT_CENTER);
			  //MorphToFloatEntity(GetParent(), "rotation", 360, 1000, eInterpolateType::INTERPOLATE_EASE_TO);
			  int livesLeft = OnModLivesLost(-1);


			  int newFloor = rt_max(0, int(*m_pFloorID)-1);

			if (livesLeft > 0)
			{

			  BCell *pCell = GetBuilding()->GetNonWallCellOnFloor(newFloor);
			 assert(pCell);
			  CL_Vec2f vStartPos = GetBuilding()->FloorAndCellToWorldPosForCharacter(newFloor, pCell->m_cellID);

			
		      //OneTimeBobEntity(GetParent());
			  ZoomToPositionEntity(GetParent(), vStartPos, 900, INTERPOLATE_EASE_TO);
			  FadeEntity(GetParent(), true, 1, 500, 900, true);
			  
			  GetMessageManager()->SetComponentVariable(this, 1400, "paused", uint32(0));
			  GetMessageManager()->SetComponentVariable(this, 1400, "floorID", uint32(newFloor));
			} else
			{
				GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_MUSIC, "audio/lose.ogg", 1000);

			}

	  }
		break;
	  
	  case ID_WEAK_ENEMY:

		  Stop();
		  m_updateThink = 0;
		  *m_pPaused = 1;
		  GetAudioManager()->Play("audio/enemy_death.wav");
		  FadeOutAndKillEntity(GetParent(), true, 500, 500);
		  SetAlignmentEntity(GetParent(), ALIGNMENT_CENTER);
		  MorphToFloatEntity(GetParent(), "rotation", 160, 2000, INTERPOLATE_EASE_TO);
		  ZoomToPositionOffsetEntity(GetParent(), CL_Vec2f(0, -500), 2000, INTERPOLATE_EASE_TO);

		  break;
	};

}

void CharComponent::OnDamage(VariantList *pVList)
{
	CL_Vec2f vPos = pVList->Get(0).GetVector2();
	EntityComponent *pEnemy = pVList->Get(1).GetComponent();
	float radius = pVList->Get(2).GetFloat();
	float damage = pVList->Get(3).GetFloat();
	
	Damage(&vPos, (CharComponent*) pEnemy , radius, damage);


}

void CharComponent::PreExplode(VariantList *pVList)
{
    VariantList vList(*m_pPos2d, EFFECT_EXPLODE);
	GetMessageManager()->CallComponentFunction(GetCharManager(), 1, "InitEffect", &vList);
}

void CharComponent::Explode(VariantList *pVList)
{

	float radius = 150;
	float damage = 10;
	
	Entity *pFolder = GetParent()->GetParent();

    VariantList vList(*m_pPos2d, this, radius, damage);
	pFolder->CallFunctionRecursively("OnDamage", &vList);

	KillEntity(GetParent());

}

void CharComponent::PatrolAI()
{
	if (GetPlayer())
	{
		GetPlayer()->Damage(m_pPos2d, this, 20, 10);
	}
	
	if (m_updateThink > GetTick(TIMER_GAME)) return;

	bool bRight;

	if (m_state == STATE_MOVE_RIGHT)
	{
		bRight = true;
	} else
	{
		bRight = false;
	}

	//patrol ahead until a wall is hit

	BCell *pWall = GetBuilding()->GetClosestCellByTypeInThisDir(bRight, BCell::TYPE_WALL, *m_pFloorID, m_pPos2d->x);
	float dist = m_pPos2d->x - pWall->m_cellID*GetBuilding()->GetCellSize()->x;
	
	if (!bRight && dist > 0)
	{
		//turn around
		bRight = true;
	}

	if (bRight && dist < 0)
	{
		//turn around
		bRight = false;
	}

	pWall = GetBuilding()->GetClosestCellByTypeInThisDir(bRight, BCell::TYPE_WALL, *m_pFloorID, m_pPos2d->x);
	dist = fabs(m_pPos2d->x - pWall->m_cellID*GetBuilding()->GetCellSize()->x);

	Move(bRight, true);
	int timeMS = (int32)(dist * 10.0f);
	
	if (!bRight)
	{
		dist -= GetBuilding()->GetCellSize()->x;
		dist *= -1;
	}

	ZoomToPositionOffsetEntity(GetParent(), CL_Vec2f(dist, 0), timeMS, INTERPOLATE_LINEAR);
	m_updateThink = GetTick(TIMER_GAME)+ timeMS-100;
}

void CharComponent::OnUpdate(VariantList *pVList)
{
	if (*m_pPaused) return;

	switch (*m_pAI)
	{
	case AI_PLAYER:
		PlayerAI();
		break;

	case AI_PATROL:
		PatrolAI();
		break;

	case AI_EXPLODE:
		;
		break;
	}
}

void CharComponent::SetPositionByFloorAndCell(VariantList *pVList )
{
	 uint32 floorID = pVList->Get(0).GetUINT32();
	 uint32 cell = pVList->Get(1).GetUINT32();

	*m_pPos2d = GetBuilding()->FloorAndCellToWorldPosForCharacter(floorID, cell);
	*m_pFloorID = floorID;
}

void CharComponent::SetPositionByFloorAndX(VariantList *pVList )
{
	uint32 floorID = pVList->Get(0).GetUINT32();
	float x = pVList->Get(1).GetFloat();
	*m_pFloorID = floorID;

	*m_pPos2d = GetBuilding()->FloorAndCellToWorldPosForCharacter(floorID, (uint32)x);
	m_pPos2d->x = x;
}

void CharComponent::Stop()
{
	m_state = STATE_STOP;
	m_updateThink = 0;
	AnimateStopEntityAndSetFrame(GetParent(), 0, 0, 0);
}

void CharComponent::Move( bool bRight, bool bButtonDown)
{
	if (*m_pPaused)
	{
		if (bButtonDown)
		{
			m_moveASAP = true;
			m_moveASAPRight = bRight;
			m_moveASAPButtonDown = bButtonDown;
		} else
		{
			m_moveASAP = false;
		}
		return;
	}

	if (bButtonDown)
	{
		if (bRight) m_state = STATE_MOVE_RIGHT; else m_state = STATE_MOVE_LEFT;
		m_updateThink = GetTick(TIMER_GAME);
		AnimateEntity(GetParent(), 1, 2, 150, InterpolateComponent::ON_FINISH_REPEAT, 0);
		AnimateEntitySetMirrorMode(GetParent(), !bRight, false);
	} else
	{
		if (bRight && m_state == STATE_MOVE_RIGHT || !bRight && m_state == STATE_MOVE_LEFT)
		{
			Stop();	
		}
	}
}

void CharComponent::UpLadder()
{
	if (*m_pPaused) return;

	CL_Vec2f vLadderPos;

	if (GetBuilding()->IsCloseToLadder(*m_pFloorID, m_pPos2d->x, vLadderPos))
	{
		float climbSpeed = 1100;

		Stop();
		vLadderPos.y = GetBuilding()->FloorToYForCharacter((*m_pFloorID)+1);
		ZoomToPositionEntity(GetParent(), vLadderPos, (int32)climbSpeed);
		GetAudioManager()->Play("audio/ladder_up.wav");

		AnimateEntitySetMirrorMode(GetParent(), false, false);
		AnimateEntity(GetParent(), 4, 5, 250, InterpolateComponent::ON_FINISH_REPEAT, 0);

		GetMessageManager()->SetComponentVariable(this, 0, "paused", uint32(1));
		AnimateStopEntityAndSetFrame(GetParent(), (int32)climbSpeed, 0, 0);
		GetMessageManager()->SetComponentVariable(this, (int32)climbSpeed, "floorID", (*m_pFloorID)+1);
		GetMessageManager()->SetComponentVariable(this, (int32)climbSpeed, "paused", uint32(0));
		return;
	} 

	if (GetBuilding()->IsCloseToElevator(*m_pFloorID, m_pPos2d->x, vLadderPos))
	{
		//GetMessageManager()->SetComponentVariable(this, climbSpeed, "floorID", (*m_pFloorID)+1);
		Stop();
		GetMessageManager()->SetComponentVariable(this, 0, "paused", uint32(1));

		BCell *pCell = GetBuilding()->GetCellByWorldPos(vLadderPos);

		assert(pCell->m_type == BCell::TYPE_ELEVATOR);
		GetAudioManager()->Play("audio/elevator.wav");
		VariantList vList(pCell->m_elevatorTargetFloor, uint32(pCell->m_elevatorTargetCell));
        GetMessageManager()->CallComponentFunction(this, 500,"SetPositionByFloorAndCell",  &vList);

		CL_Vec2f vNewLocation = GetBuilding()->FloorAndCellToWorldPosForCharacter(pCell->m_elevatorTargetFloor, pCell->m_elevatorTargetCell);
		CL_Vec2f vOffset = CL_Vec2f(0, 70);
        
        vList = VariantList(vLadderPos+vOffset, EFFECT_TELEPORT);
                            GetMessageManager()->CallComponentFunction(GetCharManager(), 300, "InitEffect", &vList);

		vList = VariantList(vNewLocation, EFFECT_TELEPORT);
        GetMessageManager()->CallComponentFunction(GetCharManager(), 500, "InitEffect", &vList);

		FadeEntity(GetParent(), false, 0, 500, 0, false);
		FadeEntity(GetParent(), false, 1, 500, 500, true);
		GetMessageManager()->SetComponentVariable(this, 1000, "paused", uint32(0));
		return;
	}

		ShowQuickMessage("There is no ladder or elevator here!");
}

void CharComponent::DownLadder()
{
	if (*m_pPaused) return;

	CL_Vec2f vLadderPos;
	float climbSpeed = 700;

	if (GetBuilding()->IsCloseToDownLadder(*m_pFloorID, m_pPos2d->x, vLadderPos))
	{
		Stop();
		vLadderPos.y = GetBuilding()->FloorToYForCharacter((*m_pFloorID)-1);
		ZoomToPositionEntity(GetParent(), vLadderPos, (int32)climbSpeed);
		GetAudioManager()->Play("audio/ladder_down.wav");

		AnimateEntitySetMirrorMode(GetParent(), false, false);
		AnimateEntity(GetParent(), 4, 5, 100, InterpolateComponent::ON_FINISH_REPEAT, 0);

		GetMessageManager()->SetComponentVariable(this, 0, "paused", uint32(1));
		AnimateStopEntityAndSetFrame(GetParent(), (int32)climbSpeed, 0, 0);
		GetMessageManager()->SetComponentVariable(this, (int32)climbSpeed, "floorID", (*m_pFloorID)-1);
		GetMessageManager()->SetComponentVariable(this, (int32)climbSpeed, "paused", uint32(0));
		return;

	} else
	{
		ShowQuickMessage("Don't see a ladder here!");

	}
}

void CharComponent::OnUse()
{
	//spawn dynamite

	int dynoLeft = OnModDyno(0);
	if (dynoLeft > 0)
	{
		GetCharManager()->AddCharEx(ID_DYNAMITE, AI_EXPLODE, *m_pFloorID, m_pPos2d->x);
		OnModDyno(-1);
	} else
	{
		ShowQuickMessage("Out of dynamite!");
	}

}