//  ***************************************************************
//  MyEEPROM - Creation date: 05/26/2016
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2016 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//Emulates the eeprom, saves/loads it to "arduboy_eeprom.dat" in the same dir as the .exe

#ifndef MyEEPROM_h__
#define MyEEPROM_h__

#include "core/core.h"

#define EEPROM_SIZE 1024 //I guess?


class MyEEPROM
{
public:
	MyEEPROM();
	virtual ~MyEEPROM();

	uint8_t read( int idx );
	void write( int idx, uint8_t val ) ;
	void update( int idx, uint8_t val ) ;

	//only in proton, for testing
	void Clear(); 


private:

	string m_fileName;
	uint8_t m_buffer[EEPROM_SIZE];
};

#endif // MyEEPROM_h__