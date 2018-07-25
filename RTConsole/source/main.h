#pragma once

//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#include "PlatformPrecomp.h"

class App;

class MainHarness
{
public:

	MainHarness()
	{
	}

	bool ParmExistsWithData(string parm, string *parmData);
	bool ParmExists(string parm); //not case sensitive
	vector<string> m_parms;
	string GetLastParm();
	uint32 GetTick();

private:
	
};

App * GetApp();

