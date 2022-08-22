//  ***************************************************************
//  Character - Creation date: 12/18/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Character_h__
#define Character_h__

class Character
{
public:
	Character();
	virtual ~Character();

	void SetPositionByFloorAndCell(uint32 floorID, int cell);
	
private:
	int floorID;

};

#endif // Character_h__