#include "PlatformPrecomp.h"
#include "StreamingInstanceFile.h"

StreamingInstanceFile::StreamingInstanceFile()
{
	m_fp = NULL;
}

StreamingInstanceFile::~StreamingInstanceFile()
{
	Close();
}

void StreamingInstanceFile::Close()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

bool StreamingInstanceFile::Open( string fName )
{
	Close();

	m_fp = fopen(fName.c_str(), "rb");
	if (!m_fp)
	{
		LogMsg("Warning: Can't open %s", fName.c_str());
		return false;
	}
	return true;
}


//returns how many bytes were read
int StreamingInstanceFile::Read(byte * pBufferOut, int maxBytesToRead)
{
	if (!m_fp)
	{
		LogMsg("Huh? File not opened.");
		return 0;
	}

	int bytesRead = (int)fread(pBufferOut, 1, maxBytesToRead, m_fp);
	return bytesRead;
}

bool StreamingInstanceFile::IsFinished()
{
	if (!m_fp) return true;

	return feof(m_fp) != 0;
}

void StreamingInstanceFile::SeekFromStart( int byteCount )
{
	assert(m_fp);
	if (!m_fp) return;
	fseek(m_fp, byteCount, SEEK_SET);

}