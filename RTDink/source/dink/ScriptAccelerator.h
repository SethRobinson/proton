//  ***************************************************************
//  ScriptAccelerator - Creation date: 01/23/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ScriptAccelerator_h__
#define ScriptAccelerator_h__

#include "PlatformSetup.h"

class ScriptPosition
{
public:

	ScriptPosition(){};
	ScriptPosition(int pos) : current (pos){};


	int current;
};


typedef map<string, ScriptPosition> ScriptMap;


class ScriptAccelerator
{
public:
	ScriptAccelerator();
	virtual ~ScriptAccelerator();

	void Kill();
	ScriptPosition * GetPositionByName(string label);
	void AddPosition(string label, int current);

private:

	ScriptMap m_data;

};

#endif // ScriptAccelerator_h__