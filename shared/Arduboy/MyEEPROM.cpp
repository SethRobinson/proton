#include "PlatformPrecomp.h"
#include "MyEEPROM.h"

MyEEPROM::MyEEPROM()
{
	m_fileName = "arduboy_eeprom.dat";

	Clear();
	//load eeprom from file if it's there

	unsigned int sizeLoaded = 0;
	byte *pMem = LoadFileIntoMemoryBasic(m_fileName, &sizeLoaded, false, true);
	if (pMem && sizeLoaded == EEPROM_SIZE)
	{
		memcpy(m_buffer, pMem, sizeLoaded);
	}
	SAFE_DELETE_ARRAY(pMem);
	
}

MyEEPROM::~MyEEPROM()
{
	//save out the data
	FILE *fp = fopen( (GetBaseAppPath()+m_fileName).c_str(), "wb");
	if (fp)
	{
		fwrite(m_buffer, EEPROM_SIZE, 1, fp);
	}
	fclose(fp);
}

uint8_t MyEEPROM::read( int idx )
{
	return m_buffer[idx];
}

void MyEEPROM::write( int idx, uint8_t val )
{
	 m_buffer[idx] = val;
}

void MyEEPROM::update( int idx, uint8_t val )
{
	write(idx, val); //we don't care about saving cycles so this is fine
}

void MyEEPROM::Clear()
{
	memset(m_buffer, 0xFF, EEPROM_SIZE); 
}
