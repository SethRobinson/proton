#include "PlatformPrecomp.h"
#include "WiringPiManager.h"

#include "../wiringPi/devLib/lcd.h"
#include "../wiringPi/wiringPi/mcp23017.h"



// Defines for the Adafruit Pi LCD interface board


// User-Defined character test

static unsigned char newChar[8] =
{
  0b00100,
  0b00100,
  0b00000,
  0b00100,
  0b01110,
  0b11011,
  0b11011,
  0b10001,
};

WiringPiManager::WiringPiManager()
{
	lcdHandle = 0;

}

WiringPiManager::~WiringPiManager()
{
}


void WiringPiManager::LCDClear()
{
	LogMsg("Clearing LCD");

#ifndef WINAPI
	lcdClear(lcdHandle);
#endif
	LogMsg("Cleared");

}

void WiringPiManager::LCDPosition(int x, int y)
{
#ifndef WINAPI
	lcdPosition(lcdHandle, x, y);
#endif
}

void WiringPiManager::LCDPrint(string text)
{
#ifndef WINAPI
	lcdPrintf(lcdHandle, text.c_str());
#else
	LogMsg("LCDPrint: %s", text.c_str());
#endif
}

void setBacklightColour(int colour)
{
	colour &= 7;

	digitalWrite(AF_RED, !(colour & 1));
	digitalWrite(AF_GREEN, !(colour & 2));
	digitalWrite(AF_BLUE, !(colour & 4));
}


void WiringPiManager::SetupLCD(int colour = 7) //7 is white
{
	LogMsg("Setting up LCD");

#ifndef WINAPI

	mcp23017Setup(AF_BASE, 0x20);

	int i;

	//	Backlight LEDs

	pinMode(AF_RED, OUTPUT);
	pinMode(AF_GREEN, OUTPUT);
	pinMode(AF_BLUE, OUTPUT);
	setBacklightColour(colour);

	//	Input buttons

	for (i = 0; i <= 4; ++i)
	{
		pinMode(AF_BASE + i, INPUT);
		pullUpDnControl(AF_BASE + i, PUD_UP);	// Enable pull-ups, switches close to 0v
	}

	// Control signals

	pinMode(AF_RW, OUTPUT); digitalWrite(AF_RW, LOW);	// Not used with wiringPi - always in write mode

  // The other control pins are initialised with lcdInit ()

	lcdHandle = lcdInit(2, 16, 4, AF_RS, AF_E, AF_DB4, AF_DB5, AF_DB6, AF_DB7, 0, 0, 0, 0);

	if (lcdHandle < 0)
	{
		LogMsg( "lcdInit failed\n");
		
	}


#endif
	LogMsg("LCD setup");


	
}



void SystemUpdate()
{

	const int C_DELAY_BETWEEN_LOOP_MS = 1;

#ifdef WINAPI
	Sleep(C_DELAY_BETWEEN_LOOP_MS);
#else
	usleep(C_DELAY_BETWEEN_LOOP_MS * 1000);
#endif

	//LogMsg("Looping!");
}

void SmartDelay(int ms)
{
	uint32 timeToStop = GetTick() + ms;

	while (timeToStop > GetTick())
	{
		SystemUpdate();
	}

}

#ifdef WINAPI
//dummy functions so windows compiles will still work

int wiringPiSetup()
{

	return 0; //-1 would be error
}



void pinMode(int pin, int mode)
{
	LogMsg("pinMode: pin %d to mode %d", pin, mode);

}

void digitalWrite(int pin, int value)
{
	LogMsg("digitalWrite: pin %d to value %d", pin, value);
}

int digitalRead(int pin)
{
	return HIGH;
}

void pullUpDnControl(int pin, int pud)
{
	LogMsg("pullUpDnControl: ping %d set to %d", pin, pud);
}

#endif
