#include "PlatformPrecomp.h"
#include "FFReader.h"

FFReader::FFReader()
{
	m_fp = NULL;
}

FFReader::~FFReader()
{
	Kill();
}

void FFReader::Kill()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}

	m_fileHeader.clear();
	m_bUsingBaseDinkFF = false;
	m_error = ERROR_NONE;
}

int FFReader::GetFFRecordIndexFromFileName(const string &fName)
{
	assert(m_fp);

	for (unsigned int i=0; i < m_fileHeader.size(); i++)
	{
		if (strcmp(fName.c_str(), m_fileHeader[i].name) == 0)
			return i;
	}

	return -1;
}

bool FFReader::DoesFileExist( const string &fName, const string &fFirstFrame )
{

	//LogMsg("Checking for  %s", (m_basePath+fName).c_str());
	
	bool bUsingDMODDirOnly = false;


	if (fFirstFrame != fName && !m_dmodGamePath.empty())
	{
		//what if load part of a sequence from the DMOD dir, but part from the DInk dir? That would be bad
		if (FileExists(m_dmodGamePath + m_basePath + fFirstFrame))
		{
			bUsingDMODDirOnly = true;
		}
	}

	if (m_fp)
	{
		if (
			(!m_bUsingBaseDinkFF || !bUsingDMODDirOnly) &&
			(GetFFRecordIndexFromFileName(fName) != -1)
			) return true;
	}

	if (!m_dmodGamePath.empty())
	{
		if (FileExists(m_dmodGamePath+m_basePath+fName)) return true;
	}
	
	if (!bUsingDMODDirOnly && FileExists(m_gamePath+m_basePath+fName))
	{
		return true;
	}
	

	return false;
}

void FFReader::Init( const string &gamePath, const string &dmodGamePath, const string &basePath, bool bUsingDinkPak )
{

	if (m_basePath == basePath && m_dmodGamePath == dmodGamePath)
	{
		//optimization, detect when we don't have to reload this
		return;
	}

	Kill();

	m_dmodBasePath = basePath;

	if (!bUsingDinkPak)
	{
		m_gamePath = GetBaseAppPath()+gamePath;
		m_basePath = basePath;

	} else
	{
		//using dink.pak
		m_basePath = gamePath+basePath;
		
	}
	
	m_dmodGamePath = dmodGamePath;
	//first scan for an .ff file
	assert(!m_fp);

	

	if (!dmodGamePath.empty())
	{
		m_fp = fopen((m_dmodGamePath+m_dmodBasePath+"dir.ff").c_str(), "rb");
	}
	
	if (!m_fp)
	{
		m_fp = fopen((m_gamePath+m_basePath+"dir.ff").c_str(), "rb");

		if (m_fp)
		{
			m_bUsingBaseDinkFF = true;
		}
	}

	if (!m_fp) return; //no ff to scan in this case

	int totalFiles;
	FFFileInfo f;
	fread(&totalFiles, sizeof(int), 1, m_fp);
	
		for (int i=0; i < totalFiles; i++)
	{
		fread(&f, sizeof(f), 1, m_fp);
		assert(strlen(f.name) < 13);
		ToLowerCase(f.name);
		m_fileHeader.push_back(f);
	}
	
		//LogMsg("FFScanning %s, %d entries", basePath.c_str(), m_fileHeader.size());

}

int FFReader::GetNextFFIndex(int index)
{
	if (m_fileHeader[index+1].offset != 0)
	{
		return index+1;
	}

	assert(index+2 < m_fileHeader.size() );	
	return index+2;
}

void FFReader::SetError(eErrorType error)
{
	m_error = error;
	if (error == ERROR_LOW_MEM)
	{
		GetBaseApp()->SetLastError(BaseApp::ERROR_MEM);
	}
}

byte * FFReader::LoadFFIntoMemory(int index, int *pSizeOut)
{
	int byteSize = m_fileHeader[GetNextFFIndex(index)].offset-m_fileHeader[index].offset;
	if (pSizeOut) *pSizeOut = byteSize;

	fseek(m_fp, m_fileHeader[index].offset, SEEK_SET);


	byte *pMem = new byte[byteSize+1]; //extra one for a null we're going to attach
	
#ifdef _DEBUG
/*
	if (Random(60) == 1)
	{
		//fake a memory error
		SAFE_DELETE_ARRAY(pMem);
		
	}
*/
#endif
	if (!pMem)
	{
		LogError("Out of mem!");
		SetError(ERROR_LOW_MEM);
		return NULL;
	}

	pMem[byteSize] = 0; //ending null, useful if we loaded a text file
	fread(pMem, byteSize, 1, m_fp);
	return pMem;
}

inline bool ends_with(const std::string & value, const std::string & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool isBmpFile(const std::string& path)
{
	return ends_with(path, ".bmp");
}

inline std::string getPngPath(const std::string& path)
{
	return path.substr(0, path.size() - 4) + ".png";
}

byte * FFReader::LoadFileIntoMemory( string const &fName, int *pSizeOut, const string &fFirstFrame)
{
	
#ifdef _DEBUG
	//LogMsg("loading for  %s", (m_basePath+fName).c_str());
#endif
	byte *pBuff = NULL;

	bool bUsingDMODDirOnly = false;


	if (fFirstFrame != fName && !m_dmodGamePath.empty())
	{
		//what if load part of a sequence from the DMOD dir, but part from the DInk dir? That would be bad
		if (FileExists(m_dmodGamePath + m_basePath + fFirstFrame) ||
			(isBmpFile(fFirstFrame) && FileExists(getPngPath(m_dmodGamePath + m_basePath + fFirstFrame))))
		{
			bUsingDMODDirOnly = true;
		}
	}

	if (m_fp)
	{
		if (m_bUsingBaseDinkFF && !m_dmodGamePath.empty())
		{
			//you know what?  Let's do a last minute try in the dmod dir as well.

			if (isBmpFile(fName) && FileExists(getPngPath(m_dmodGamePath + m_dmodBasePath + fName)))
			{
				//pBuff = LoadFileIntoMemoryBasic(m_dmodGamePath+m_basePath+fName, &len, false, false);
				pBuff = GetFileManager()->Get(getPngPath(m_dmodGamePath + m_dmodBasePath + fName), pSizeOut, false);

				if (!pBuff) SetError(ERROR_LOW_MEM);
				return pBuff;
			}
			else if (FileExists(m_dmodGamePath + m_dmodBasePath + fName))
			{
				//pBuff = LoadFileIntoMemoryBasic(m_dmodGamePath+m_basePath+fName, &len, false, false);
				pBuff = GetFileManager()->Get(m_dmodGamePath + m_dmodBasePath + fName, pSizeOut, false);

				if (!pBuff) SetError(ERROR_LOW_MEM);
				return pBuff;
			}
		}

		if (!bUsingDMODDirOnly)
		{
			int index = GetFFRecordIndexFromFileName(fName);

			if (index != -1)
			{
				//we found it!
				return LoadFFIntoMemory(index, pSizeOut);
			}
		}
	}
	


		if (!m_dmodGamePath.empty())
		{
			if (isBmpFile(fName) && FileExists(getPngPath(m_dmodGamePath + m_dmodBasePath + fName)))
			{
				//pBuff =  LoadFileIntoMemoryBasic(m_dmodGamePath+m_basePath+fName, &len, false, false);
				pBuff = GetFileManager()->Get(getPngPath(m_dmodGamePath + m_dmodBasePath + fName), pSizeOut, false);
				if (!pBuff) SetError(ERROR_LOW_MEM);
				return pBuff;
			}
			else if (FileExists(m_dmodGamePath+m_dmodBasePath+fName))
			{
				//pBuff =  LoadFileIntoMemoryBasic(m_dmodGamePath+m_basePath+fName, &len, false, false);
				pBuff = GetFileManager()->Get(m_dmodGamePath+m_dmodBasePath+fName, pSizeOut,false);
				if (!pBuff) SetError(ERROR_LOW_MEM);
				return pBuff;
			}
		}

		if (!bUsingDMODDirOnly)
		{
			if (isBmpFile(fName) && FileExists(getPngPath(m_gamePath + m_basePath + fName)))
			{
				//pBuff = LoadFileIntoMemoryBasic(m_gamePath+m_basePath+fName, &len, false, false);
				pBuff = GetFileManager()->Get(getPngPath(m_gamePath + m_basePath + fName), pSizeOut, false);
				if (*pSizeOut == UINT_MAX) SetError(ERROR_LOW_MEM);
				return pBuff;
			}
			else if (FileExists(m_gamePath + m_basePath + fName))
			{
				//pBuff = LoadFileIntoMemoryBasic(m_gamePath+m_basePath+fName, &len, false, false);
				pBuff = GetFileManager()->Get(m_gamePath + m_basePath + fName, pSizeOut, false);
				if (*pSizeOut == UINT_MAX) SetError(ERROR_LOW_MEM);
				return pBuff;
			}
		}

	
	return NULL;
}

