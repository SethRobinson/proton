//  ***************************************************************
//  BuildingComponent - Creation date: 12/18/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BuildingComponent_h__
#define BuildingComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
//#include "Renderer/SurfaceAnim.h"


class BCell
{
public:

	BCell()
	{
		m_type = TYPE_OPEN1;
		m_color = MAKE_RGBA(255,255,255,255);
	}
	enum eCellType
	{
		TYPE_OPEN1,
		TYPE_LADDER_UP,
		TYPE_LADDER_DOWN,
		TYPE_WALL,
		TYPE_ELEVATOR,
		TYPE_DOOR1
	};

	uint32 m_cellID;
	eCellType m_type;
	uint32 m_elevatorTargetFloor;
	uint32 m_elevatorTargetCell;
	uint32 m_color;
};

class BFloor
{
public:

	uint32 m_floorID;
	vector<BCell> m_cells;
};


typedef vector< vector<int> > FloorMap;

class BuildingComponent: public EntityComponent
{
public:

	enum eDirection
	{
		DIR_NONE,
		DIR_LEFT,
		DIR_RIGHT
	};
	BuildingComponent();
	virtual ~BuildingComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	float FloorToY(uint32 floorID);
	CL_Vec2f FloorAndCellToWorldPos(uint32 floorID, uint32 cell);
	bool IsCloseToLadder(uint32 floorID, float posX, CL_Vec2f &pVDoorPosOut);
	bool GetClosestObject(uint32 floorID,BCell::eCellType type,  float posX, CL_Vec2f &pVDoorPosOut);
	float CellMiddleToWorldX(int cell);
	CL_Vec2f FloorAndCellToWorldPosForCharacter( uint32 floorID, uint32 cell );
	float FloorToYForCharacter(uint32 floorID);
	bool IsCloseToDownLadder( uint32 floorID, float posX, CL_Vec2f &pVDoorPosOut );
	void FocusCamera(VariantList *pVList);
	int GetCellCount() {return *m_pCellsPerFloor;}
	bool IsCloseToElevator( uint32 floorID, float posX, CL_Vec2f &pVDoorPosOut );
	BCell * GetCellByWorldPos(CL_Vec2f vPos);
	CL_Vec2f * GetScale2D();
	BCell* GetEmptyCellOnFloor(uint32 floorID);
	void SetupCharacters();
	BCell * GetClosestCellByTypeInThisDir(bool bRight, BCell::eCellType type, uint32 floorID, float x);
	CL_Vec2f * GetCellSize() {return &m_cellSize;}
	BCell* GetNonWallCellOnFloor(uint32 floorID);
	bool IsCloseToDoor( uint32 floorID, float posX, CL_Vec2f &pVDoorPosOut );
	void EraseCellByWorldPos(CL_Vec2f vWorldPos);
	void GetRandomNonWallFloorAndCell(uint32 &floorID, uint32 &cell);
private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void OnBuildLevel(VariantList *pVList);
	void DrawFloor(BFloor *pFloor, CL_Vec2f vCamPos, CL_Vec2f vScale);
	void DrawCell(BCell *pCell, CL_Vec2f vWorldDrawPos, CL_Vec2f vScale);
	bool PutItemOnFloor(BFloor *pFloor, BCell::eCellType type, int minCel, int maxCel);
	void GetRandomEmptyFloorAndCell(uint32 &floorID, uint32 &cell);
	void PutItemOnRandomFloor(BCell::eCellType type, int startingFloor = 0);
	void MakeAllRoomsReachable();
	void MarkReachableCellsFromCell(FloorMap *pMap, BFloor *pFloor, int cell, eDirection dir);
	int PrintMappingStatistics(FloorMap *pMap, bool bRequireEmpty);
	BFloor * GetFirstUnreachableFromBottomLeft(FloorMap *pMap, int *cellOut, bool bMustBeEmpty);
	void GetReachableCellByCount(FloorMap *pMap,int index,uint32 *pFloorIDOut, uint32 *pCellOut,bool bRequireEmpty);
	void MakeElevatorPair(uint32 floorIDA, uint32 cellA, uint32 floorIDB, uint32 cellB);
	CL_Vec2f *m_pPos2d;
	uint32 *m_pFloors;
	uint32 *m_pCellsPerFloor;
	uint32 *m_pElevators;
	uint32 *m_pExtraLadders;
	uint32 *m_pWalls;
	uint32 *m_pDoors;
	float *m_pLadderOddsPerFloor;
	uint32 *m_pEGuys;
	uint32 *m_pMGuys;
	uint32 *m_pHGuys;

	CL_Vec2f m_cellSize, m_cameraTarget;
	
	SurfaceAnim m_tiles;
	SurfaceAnim m_overlays;
	CL_Vec2f *m_pScale2d;
	/*
	CL_Vec2f *m_pSize2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/

	
	vector<BFloor> m_floors;
};

BuildingComponent * GetBuilding();
CL_Vec2f WorldToScreenPos(CL_Vec2f vWorldPos);
#endif // BuildingComponent_h__