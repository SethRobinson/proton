//  ***************************************************************
//  WiringPiManager - Creation date: 01/29/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*
This is designed to run on a Raspberry Pi that has the wiringPi lib installed. (don't forget
to link it)

On Windows builds fake versions are provided so it compiles, but instead of actually doing
anything it just prints debug statements.  Useful for using MSVC for dev

//Some of this is based on Gordon Henderson's wiringPi example code.  See http://wiringpi.com/
*/


#ifndef WiringPiEmu_h__
#define WiringPiManager_h__

#ifndef WINAPI
#include <wiringPi.h>
#endif
#include "../wiringPi/wiringPi/wiringPi.h"


#define	AF_BASE		100
#define	AF_RED		(AF_BASE + 6)
#define	AF_GREEN	(AF_BASE + 7)
#define	AF_BLUE		(AF_BASE + 8)

#define	AF_E		(AF_BASE + 13)
#define	AF_RW		(AF_BASE + 14)
#define	AF_RS		(AF_BASE + 15)

#define	AF_DB4		(AF_BASE + 12)
#define	AF_DB5		(AF_BASE + 11)
#define	AF_DB6		(AF_BASE + 10)
#define	AF_DB7		(AF_BASE +  9)

#define	AF_SELECT	(AF_BASE +  0)
#define	AF_RIGHT	(AF_BASE +  1)
#define	AF_DOWN		(AF_BASE +  2)
#define	AF_UP		(AF_BASE +  3)
#define	AF_LEFT		(AF_BASE +  4)


class WiringPiManager
{
public:
	WiringPiManager();
	virtual ~WiringPiManager();

	void LCDClear();
	void LCDPosition(int x, int y);
	void LCDPrint(string text);
	void SetupLCD(int colour);

protected:

	int lcdHandle;
};

int wiringPiSetup();
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
void pullUpDnControl(int pin, int pud);

#endif // WiringPiEmu_h__#pragma once
