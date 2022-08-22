#include "PlatformPrecomp.h"
#include "BuildingComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"
#include "CharManagerComponent.h"

BuildingComponent *g_pBuilding = NULL; //ugly, but whatever

CL_Vec2f g_lastCam, g_lastScale;

BuildingComponent * GetBuilding()
{
	return g_pBuilding;
}

BuildingComponent::BuildingComponent()
{
	SetName("Building");
}

BuildingComponent::~BuildingComponent()
{
 g_pBuilding = NULL;
}


void BuildingComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_cellSize = CL_Vec2f(160, 160);
	g_pBuilding = this;
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pScale2d = &GetParent()->GetShared()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	
	/*
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetShared()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetShared()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetShared()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	*/

	m_pFloors = &GetParent()->GetVar("floors")->GetUINT32();
	m_pCellsPerFloor = &GetParent()->GetVar("cellsPerFloor")->GetUINT32();
	m_pElevators = &GetParent()->GetVar("elevators")->GetUINT32();
	m_pDoors = &GetParent()->GetVar("doors")->GetUINT32();
	m_pExtraLadders = &GetParent()->GetVar("extraLadders")->GetUINT32();
	m_pWalls = &GetParent()->GetVar("walls")->GetUINT32();
	m_pEGuys = &GetParent()->GetVar("eguys")->GetUINT32();
	m_pMGuys = &GetParent()->GetVar("mguys")->GetUINT32();
	m_pHGuys = &GetParent()->GetVar("eguys")->GetUINT32();

	m_pLadderOddsPerFloor = &GetParent()->GetVar("ladderOddsPerFloor")->GetFloat();


	GetParent()->GetFunction("BuildLevel")->sig_function.connect(1, boost::bind(&BuildingComponent::OnBuildLevel, this, _1));
   //register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&BuildingComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&BuildingComponent::OnUpdate, this, _1));

	m_tiles.LoadFile("game/building_tiles.rttex");
	//m_tiles.SetSmoothing(false);
	m_tiles.SetupAnim(6, 1);

	m_overlays.LoadFile("game/building_overlays.rttex");
	//m_tiles.SetSmoothing(false);
	m_overlays.SetupAnim(6, 1);
}




bool BuildingComponent::PutItemOnFloor(BFloor *pFloor, BCell::eCellType type, int minCel, int maxCel)
{
	int tryCount = 0;

tryAgain:

	tryCount++;
	if (tryCount > 100)
	{
		LogMsg("Giving up placing %d on floor %d", type, pFloor->m_floorID);
		//give up
		return false;
	}
	int cell = RandomRange(minCel, maxCel);
	assert(cell >= 0 &&  cell < (int)* m_pCellsPerFloor);


	int floorID = pFloor->m_floorID;

	if (m_floors[floorID].m_cells[cell].m_type != BCell::TYPE_OPEN1) goto tryAgain;

	if (type == BCell::TYPE_LADDER_UP)
	{
		//special check to make sure we CAN put it above us
		if (floorID+1 >= (int) * m_pFloors)
		{
			LogMsg("Of course you can't put a ladder on floor %d", floorID);
			return false; //can't be placed
		}

		if (m_floors[floorID+1].m_cells[cell].m_type != BCell::TYPE_OPEN1)
		{
			//bad place.  try again
			goto tryAgain;
		}

		//good to go.  put the ladder going down too
		m_floors[floorID+1].m_cells[cell].m_type = BCell::TYPE_LADDER_DOWN;
	} else if (type == BCell::TYPE_WALL)
	{
		//make sure there is a blank space on both sides of us
		if (m_floors[floorID].m_cells[cell-2].m_type == BCell::TYPE_WALL) return false; //give up
		if (m_floors[floorID].m_cells[cell+2].m_type == BCell::TYPE_WALL) return false; //give up
	}

	pFloor->m_cells[cell].m_type = type;
	return true;
}


void BuildingComponent::PutItemOnRandomFloor(BCell::eCellType type, int startingFloor)
{
	int tryCount = 0;

tryAgain:

	tryCount++;
	if (tryCount > 200)
	{
		LogMsg("Giving up placing %d", type);
		//give up
		return;
	}

	int floorID = RandomRange(startingFloor, (int)m_floors.size());
	
	assert(*m_pCellsPerFloor > 5);
	
	int cellMin = 1;
	int cellMax = *m_pCellsPerFloor-2;

	switch(type)
	{
	case BCell::TYPE_WALL:
		//special handling so we always have enough space in an enclosed area
		cellMin = 3;
		cellMax = *m_pCellsPerFloor-3;


		break;
	}
	
	if (PutItemOnFloor(&m_floors[floorID], type, cellMin, cellMax))
	{
		return;
	}

	goto tryAgain;
}

void BuildingComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void BuildingComponent::GetRandomNonWallFloorAndCell(uint32 &floorID, uint32 &cell)
{
	//dangerous.  Yes.  
	while(1)
	{
		int maxFloors = (int)m_floors.size()-1;
		floorID = RandomRange(1, maxFloors);
		cell = RandomRange(0, *m_pCellsPerFloor-1);

		assert(floorID >= 0 && floorID < m_floors.size());
		assert(cell >= 0 && cell < *m_pCellsPerFloor);

		if (m_floors[floorID].m_cells[cell].m_type != BCell::TYPE_WALL) return;
	}

};

void BuildingComponent::GetRandomEmptyFloorAndCell(uint32 &floorID, uint32 &cell)
{
	//dangerous.  Yes.  
	while(1)
	{
		int maxFloors = (int)m_floors.size()-1;
		floorID = RandomRange(0, maxFloors);
		cell = RandomRange(0, *m_pCellsPerFloor-1);
		
		assert(floorID >= 0 && floorID < m_floors.size());
		assert(cell >= 0 && cell < *m_pCellsPerFloor);

		if (m_floors[floorID].m_cells[cell].m_type == BCell::TYPE_OPEN1) return;
	}

};


void BuildingComponent::SetupCharacters()
{
	//spawn guys

	for (int i=0; i < (int)*m_pEGuys; i++)
	{
		uint32 floor, cell;
		
		GetRandomNonWallFloorAndCell(floor, cell);
		GetCharManager()->AddChar(ID_WEAK_ENEMY, AI_PATROL, floor, cell);
	}
	
	/*
	for (uint32 i=1; i < *m_pFloors; i++)
	{
		BCell *pCell = GetNonWallCellOnFloor(i);

		GetCharManager()->AddChar(ID_WEAK_ENEMY, AI_PATROL, i, pCell->m_cellID);

		//PutItemOnFloor(&m_floors[i], BCell::TYPE_LADDER_UP, 0, (*m_pCellsPerFloor)-1);
	}
	*/

}

void BuildingComponent::MarkReachableCellsFromCell(FloorMap *pMap, BFloor *pFloor, int cell, eDirection dir)
{
	pMap->at(pFloor->m_floorID).at(cell) = 1;

	if (dir == DIR_NONE)
	{
		MarkReachableCellsFromCell(pMap, pFloor, cell, DIR_LEFT);
		MarkReachableCellsFromCell(pMap, pFloor, cell, DIR_RIGHT);
		return;
	}

	
	while (1)
	{
		if (dir == DIR_RIGHT)
		{
			cell++;
			if (cell >= (int)*m_pCellsPerFloor) return; //all done
		} else
		{
			cell--;
			if (cell < 0) return; //all done
		}

		BCell::eCellType type = pFloor->m_cells[cell].m_type;
		BCell *pCell = &pFloor->m_cells[cell];
		if (type != BCell::TYPE_WALL)
		{
			//quit out if already set for speed?

			pMap->at(pFloor->m_floorID).at(cell) = 1;
			
			switch (type)
			{
			case BCell::TYPE_LADDER_UP:
				MarkReachableCellsFromCell(pMap, &m_floors[pFloor->m_floorID+1], cell, DIR_LEFT);
				MarkReachableCellsFromCell(pMap, &m_floors[pFloor->m_floorID+1], cell, DIR_RIGHT);
				break;
			case BCell::TYPE_ELEVATOR:
				
				if (pMap->at(pCell->m_elevatorTargetFloor).at(pCell->m_elevatorTargetCell) == 0)
				{
					MarkReachableCellsFromCell(pMap, &m_floors[pCell->m_elevatorTargetFloor], pCell->m_elevatorTargetCell, DIR_LEFT);
					MarkReachableCellsFromCell(pMap, &m_floors[pCell->m_elevatorTargetFloor], pCell->m_elevatorTargetCell, DIR_RIGHT);
				}
				break;
		}

		} else
		{
			return;
		}
	}
}

int BuildingComponent::PrintMappingStatistics(FloorMap *pMap, bool bRequireEmpty)
{

	int reachable = 0;
	int unreachable = 0;
	
	for (int i=0; i < pMap->size(); i++)
	{
		for (int c=1; c < (int)*m_pCellsPerFloor-2; c++)
		{
			if (pMap->at(i).at(c) == 1)
			{
				if (!bRequireEmpty ||  m_floors[i].m_cells[c].m_type == BCell::TYPE_OPEN1)
				reachable++;
			} else
			{
				if (m_floors[i].m_cells[c].m_type != BCell::TYPE_WALL)
				{
					//don't count walls
					unreachable++;
				}
			}
		}

	}

	LogMsg("mapping statistics: Reachable: %d, unreachable: %d", reachable, unreachable);
	return reachable;
}

void BuildingComponent::GetReachableCellByCount(FloorMap *pMap,int index, uint32 *pFloorIDOut, uint32 *pCellOut, bool bRequireEmpty)
{
	int reachable = 0;
	int unreachable = 0;

	for (int i=0; i < pMap->size(); i++)
	{
		for (int c=1; c < (int)*m_pCellsPerFloor-2; c++)
		{
			if (pMap->at(i).at(c) == 1)
			{
				if (!bRequireEmpty ||  m_floors[i].m_cells[c].m_type == BCell::TYPE_OPEN1)
				{

				if (reachable == index)
				{
					*pFloorIDOut = m_floors[i].m_floorID;
					*pCellOut = c;

				}
				reachable++;
				}

			} else
			{
				if (m_floors[i].m_cells[c].m_type != BCell::TYPE_WALL)
				{
					//don't count walls
					unreachable++;
				}
			}
		}

	}
}
BFloor * BuildingComponent::GetFirstUnreachableFromBottomLeft(FloorMap *pMap,  int *cellOut, bool bMustBeEmpty)
{
	
	int reachable = 0;
	int unreachable = 0;

	for (int i=0; i < pMap->size(); i++)
	{
		for (int c=1; c < (int)*m_pCellsPerFloor-2; c++)
		{
			if (pMap->at(i).at(c) == 1)
			{
				reachable++;
			} else
			{
				if (m_floors[i].m_cells[c].m_type != BCell::TYPE_WALL)
				{
					if (!bMustBeEmpty ||m_floors[i].m_cells[c].m_type == BCell::TYPE_OPEN1 )
					{
						//found one, and it ain't a wall
						*cellOut = c;
						return &m_floors[i];

					}
					
//					unreachable++;
				}
			}
		}

	}

	return NULL;
}

void BuildingComponent::MakeElevatorPair(uint32 floorIDA, uint32 cellA, uint32 floorIDB, uint32 cellB)
{
	uint32 color = GetBrightColor();

	m_floors[floorIDA].m_cells[cellA].m_type = BCell::TYPE_ELEVATOR;
	m_floors[floorIDA].m_cells[cellA].m_elevatorTargetFloor = floorIDB;
	m_floors[floorIDA].m_cells[cellA].m_elevatorTargetCell = cellB;
	m_floors[floorIDA].m_cells[cellA].m_color = color;

	m_floors[floorIDB].m_cells[cellB].m_type = BCell::TYPE_ELEVATOR;
	m_floors[floorIDB].m_cells[cellB].m_elevatorTargetFloor = floorIDA;
	m_floors[floorIDB].m_cells[cellB].m_elevatorTargetCell = cellA;
	m_floors[floorIDB].m_cells[cellB].m_color = color;

};

void BuildingComponent::MakeAllRoomsReachable()
{
  FloorMap map;

  
  map.resize(m_floors.size());
  for (int i=0; i < m_floors.size(); i++)
  {
	  map[i].resize(*m_pCellsPerFloor);

	  memset( &map[i][0], 0, sizeof(int)**m_pCellsPerFloor);
  }

  //map
  MarkReachableCellsFromCell(&map, &m_floors[0], 1, DIR_NONE);

  int cell;
  BFloor *pFloor = NULL;

  while (pFloor =GetFirstUnreachableFromBottomLeft(&map, &cell, true))
  {
	  //let's make this reachable.. by someone..
	  int reachable = PrintMappingStatistics(&map, true);
	  
	  uint32 floorIDA = pFloor->m_floorID;
	  uint32 cellA = cell;
	  
	  uint32 floorIDB, cellB;
	  GetReachableCellByCount(&map, RandomRange(0, reachable-1), &floorIDB, &cellB, true);

	  //make elevator
	  MakeElevatorPair(floorIDA, cellA, floorIDB, cellB);
	  MarkReachableCellsFromCell(&map, &m_floors[floorIDA], cellA, DIR_NONE);
  }

  PrintMappingStatistics(&map, false);
}

void BuildingComponent::OnBuildLevel(VariantList *pVList)
{
	LogMsg("Building level");

	//init structure

	m_floors.resize(*m_pFloors);

	for (uint32 i=0; i < *m_pFloors; i++)
	{
		m_floors[i].m_floorID = i;
		m_floors[i].m_cells.resize(*m_pCellsPerFloor);

		for (uint32 j=0; j < *m_pCellsPerFloor; j++)
		{
			m_floors[i].m_cells[j].m_cellID = j;
		}

		//put walls on the sides
		m_floors[i].m_cells[0].m_type = BCell::TYPE_WALL;
		m_floors[i].m_cells[*m_pCellsPerFloor-1].m_type = BCell::TYPE_WALL;
	}

	//populate it
	for (uint32 i=0; i < *m_pFloors; i++)
	{
		if (RandomRangeFloat(0,1) < *m_pLadderOddsPerFloor)
		{
			PutItemOnFloor(&m_floors[i], BCell::TYPE_LADDER_UP, 0, (*m_pCellsPerFloor)-1);
		}
	}

	//place some random obstacles

	uint32 walls = *m_pWalls;
	for (uint32 i=0; i < walls; i++)
	{
		PutItemOnRandomFloor(BCell::TYPE_WALL, 1);
	}

	for (int i=0; i < (int)*m_pExtraLadders; i++)
	{
		PutItemOnRandomFloor(BCell::TYPE_LADDER_UP, 1);

	}

	//how about some extra elevators?

	for (int i=0; i < (int)*m_pElevators; i++)
	{
		uint32 floorIDA, floorIDB, cellA, cellB;

		GetRandomEmptyFloorAndCell(floorIDA, cellA);
		m_floors[floorIDA].m_cells[cellA].m_type = BCell::TYPE_ELEVATOR;

		GetRandomEmptyFloorAndCell(floorIDB, cellB);

		MakeElevatorPair(floorIDA, cellA, floorIDB, cellB);

		//PutItemOnFloor(&m_floors[i], BCell::TYPE_LADDER_UP, 0, (*m_pCellsPerFloor)-1);
	}

	MakeAllRoomsReachable();

	//add treasure
	for (uint32 i=0; i < *m_pDoors; i++)
	{
		PutItemOnRandomFloor(BCell::TYPE_DOOR1, 0);
	}


	//set camera position
	GetParent()->GetVar("pos2d")->Set(CL_Vec2f(0, -GetScreenSizeYf()));
}


void BuildingComponent::DrawCell(BCell *pCell, CL_Vec2f vScreenPos, CL_Vec2f vScale)
{
	
	//rtRect r(vScreenPos.x, vScreenPos.y, vScreenPos.x+ m_cellSize.x, vScreenPos.y+m_cellSize.y);
	//DrawRect(rtRect(r));
	
	bool bDrawBG = false;

	switch (pCell->m_type)
	{
	case BCell::TYPE_ELEVATOR:
	case BCell::TYPE_DOOR1:
		bDrawBG = true;
		break;
	}

	if (bDrawBG)
	{
		m_tiles.BlitScaledAnim(vScreenPos.x, vScreenPos.y, BCell::TYPE_OPEN1, 0, vScale, ALIGNMENT_UPPER_LEFT, MAKE_RGBA(255,255,255,255));
		m_overlays.BlitScaledAnim(vScreenPos.x, vScreenPos.y, pCell->m_type, 0, vScale, ALIGNMENT_UPPER_LEFT, pCell->m_color);

	} else
	{
		m_tiles.BlitScaledAnim(vScreenPos.x, vScreenPos.y, pCell->m_type, 0, vScale, ALIGNMENT_UPPER_LEFT, pCell->m_color);
	}
}

CL_Vec2f WorldToScreenPos(CL_Vec2f vWorldPos)
{
	CL_Vec2f vPos;
	vPos.x = vWorldPos.x *= g_lastScale.x;
	vPos.y = vWorldPos.y *= g_lastScale.y;

	return vWorldPos- g_lastCam;
}

float BuildingComponent::FloorToY(uint32 floorID)
{
	return -(floorID*m_cellSize.y) - m_cellSize.y;
}

float BuildingComponent::FloorToYForCharacter(uint32 floorID)
{
	return (-(floorID*m_cellSize.y) - m_cellSize.y)+145;
}

void BuildingComponent::DrawFloor(BFloor *pFloor, CL_Vec2f vCamPos, CL_Vec2f vScale)
{
	
	CL_Vec2f vScreenPos, vWorldPos;

	for (int i= (int)pFloor->m_cells.size()-1; i >= 0; i--)
	{
		vWorldPos = CL_Vec2f(i*m_cellSize.x,FloorToY(pFloor->m_floorID)); 
		vScreenPos = WorldToScreenPos(vWorldPos);
		
		//we don't really want sub-pixel rendering.. do this
		vScreenPos.x = ceil(vScreenPos.x);
		vScreenPos.y = ceil(vScreenPos.y);

		//LogMsg("Drawing floor %d, cell %d (%s local) at %s", pFloor->m_floorID, i, PrintVector2(vWorldPos).c_str(),  PrintVector2(vScreenPos).c_str());	
		DrawCell(&pFloor->m_cells[i], vScreenPos, vScale);
	}
}

void BuildingComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;

	//LogMsg("Pos is %s", PrintVector2(*m_pPos2d).c_str());
	
	g_lastCam = vFinalPos;
	g_lastScale = *m_pScale2d;
	for (uint32 i=0; i < m_floors.size(); i++)
	{
		DrawFloor(&m_floors[i], vFinalPos, g_lastScale);
	}
}

void BuildingComponent::FocusCamera(VariantList *pVList)
{
	CL_Vec2f vFocus = pVList->Get(0).GetVector2();

	vFocus.x *= m_pScale2d->x;
	vFocus.y *= m_pScale2d->y;

//	LogMsg("Player: %s", PrintVector2(vFocus).c_str());
	vFocus.x -= GetScreenSizeXf()/2;
	vFocus.y -= GetScreenSizeYf()/2;
	vFocus.y = rt_min(vFocus.y, -GetScreenSizeYf());
	//hug left side
	vFocus.x = rt_max(vFocus.x, 0);
	//hug right side
	vFocus.x = rt_min(vFocus.x, ((*m_pCellsPerFloor*m_cellSize.x)*g_lastScale.x) - GetScreenSizeXf()) ;

	m_cameraTarget = vFocus;
}

void BuildingComponent::OnUpdate(VariantList *pVList)
{

	m_pPos2d->x = LerpFloat(m_pPos2d->x, m_cameraTarget.x, 0.1f*GetBaseApp()->GetDelta());
	m_pPos2d->y = LerpFloat(m_pPos2d->y, m_cameraTarget.y, 0.1f*GetBaseApp()->GetDelta());

	if (GetPlayer())
	{
		//focus camera in on player
        VariantList vList(GetPlayer()->GetParent()->GetVar("pos2d")->GetVector2());
		FocusCamera(&vList);

	}
}

CL_Vec2f BuildingComponent::FloorAndCellToWorldPos( uint32 floorID, uint32 cell )
{
	return CL_Vec2f(cell*m_cellSize.x, FloorToY(floorID));

}

CL_Vec2f BuildingComponent::FloorAndCellToWorldPosForCharacter( uint32 floorID, uint32 cell )
{
	return CL_Vec2f(cell*m_cellSize.x+m_cellSize.x/2, FloorToY(floorID)+145);
}

bool BuildingComponent::IsCloseToLadder( uint32 floorID, float posX, CL_Vec2f &pVDoorPosOut )
{
	bool bFound = GetClosestObject(floorID, BCell::TYPE_LADDER_UP, posX, pVDoorPosOut);
	
	const float distanceNeededToAccessLadder = 34;
	if (bFound)
	{
		if (fabs(pVDoorPosOut.x-posX) < distanceNeededToAccessLadder)
		{
			return true;
		}
	}
	return false; //too close
}

bool BuildingComponent::IsCloseToElevator( uint32 floorID, float posX, CL_Vec2f &pVDoorPosOut )
{
	bool bFound = GetClosestObject(floorID, BCell::TYPE_ELEVATOR, posX, pVDoorPosOut);

	const float distanceNeededToAccessLadder = 70;
	if (bFound)
	{
		if (fabs(pVDoorPosOut.x-posX) < distanceNeededToAccessLadder)
		{
			return true;
		}
	}
	return false; //too close
}

void BuildingComponent::EraseCellByWorldPos(CL_Vec2f vWorldPos)
{
	BCell *pCell = GetCellByWorldPos(vWorldPos);
	if (pCell)
	{	

		pCell->m_type = BCell::TYPE_OPEN1;
	} else
	{
		assert(0);
	}
}
bool BuildingComponent::IsCloseToDoor( uint32 floorID, float posX, CL_Vec2f &pVDoorPosOut )
{
	bool bFound = GetClosestObject(floorID, BCell::TYPE_DOOR1, posX, pVDoorPosOut);

	const float distanceNeededToAccessLadder = 20;
	if (bFound)
	{
		if (fabs(pVDoorPosOut.x-posX) < distanceNeededToAccessLadder)
		{
			return true;
		}
	}
	return false; //too close
}



bool BuildingComponent::IsCloseToDownLadder( uint32 floorID, float posX, CL_Vec2f &pVDoorPosOut )
{
	bool bFound = GetClosestObject(floorID, BCell::TYPE_LADDER_DOWN, posX, pVDoorPosOut);

	const float distanceNeededToAccessLadder = 34;
	if (bFound)
	{
		if (fabs(pVDoorPosOut.x-posX) < distanceNeededToAccessLadder)
		{
			return true;
		}
	}
	return false; //too close
}

float BuildingComponent::CellMiddleToWorldX(int cell)
{
	float x = (cell* m_cellSize.x) + m_cellSize.x/2;
	return x;
}

bool BuildingComponent::GetClosestObject(uint32 floorID,BCell::eCellType type,  float posX, CL_Vec2f &pVDoorPosOut)
{
	
	bool bFound = false;
	float smallestDistSoFar = 100000000;

	for (int i=0; i < (int)*m_pCellsPerFloor; i++)
	{
		if (m_floors[floorID].m_cells[i].m_type == type)
		{
			//well, it has one, but is it close?
			float itemX = CellMiddleToWorldX(i);

			float dist =  fabs(itemX-posX);
			bFound = true;
			if (dist < smallestDistSoFar)
			{
				smallestDistSoFar = dist;
				pVDoorPosOut = CL_Vec2f(itemX, FloorToY(floorID));
			}
		}
	}
	return bFound;
}

BCell * BuildingComponent::GetCellByWorldPos( CL_Vec2f vPos )
{
	vPos.y += 1; 
	uint32 floor = (uint32)(-(vPos.y/m_cellSize.y));
	uint32 cell = (uint32)vPos.x/m_cellSize.x;
	assert(floor >= 0 && floor < m_floors.size());
	assert(cell >= 0 && cell < *m_pCellsPerFloor);

	return &m_floors[floor].m_cells[cell];
}

CL_Vec2f * BuildingComponent::GetScale2D()
{
	return m_pScale2d;
}

BCell* BuildingComponent::GetNonWallCellOnFloor(uint32 floorID)
{
	int tries =0;

	while (tries++ < 500)
	{
		int cellID = RandomRange(0, *m_pCellsPerFloor);
		if (m_floors[floorID].m_cells[cellID].m_type != BCell::TYPE_WALL)
		{
			return &m_floors[floorID].m_cells[cellID];
		}
	}

	return NULL;
}

BCell* BuildingComponent::GetEmptyCellOnFloor(uint32 floorID)
{
	int tries =0;
  
	while (tries++ < 500)
	{
		int cellID = RandomRange(0, *m_pCellsPerFloor);
		if (m_floors[floorID].m_cells[cellID].m_type == BCell::TYPE_OPEN1)
		{
			return &m_floors[floorID].m_cells[cellID];
		}
	}

	return NULL;
}

BCell * BuildingComponent::GetClosestCellByTypeInThisDir( bool bRight, BCell::eCellType type, uint32 floorID, float x )
{
	uint32 cell = x/m_cellSize.x;

	if (bRight)
	{
		for (int i=cell; i < (int)*m_pCellsPerFloor; i++)
		{
			if (m_floors[floorID].m_cells[i].m_type == type)
			{
				return &m_floors[floorID].m_cells[i];
			}
		}
	} else
	{
		for (int i=cell; i >= 0; i--)
		{
			if (m_floors[floorID].m_cells[i].m_type == type)
			{
				return &m_floors[floorID].m_cells[i];
			}
		}
	}

	assert(!"This should never happen");
	return NULL;
}

