#include "PlatformPrecomp.h"
#include "FileManager.h"
#include "../util/ResourceUtils.h"
#ifndef _CONSOLE
#include "BaseApp.h"
#else
FileManager * GetFileManager();
#endif
#include "FileSystem.h"
#include "StreamingInstanceFile.h"

FileInstance::FileInstance( const string &fileName )
{
	m_pData = NULL;
	//if it errors out, fine, so be it, hopefully the user checks with IsLoaded()
	Load(fileName);
}

FileInstance::FileInstance( const string &fileName, bool bAddBasePath )
{
	m_pData = NULL;
	//if it errors out, fine, so be it, hopefully the user checks with IsLoaded()
	Load(fileName, bAddBasePath);
}
void FileInstance::Kill()
{
	SAFE_DELETE_ARRAY(m_pData);
	m_size = 0;
}


FileInstance::~FileInstance()
{
	Kill();
}

bool FileInstance::Load( string fileName, bool bAddBasePath )
{
	Kill();
	m_pData = GetFileManager()->Get(fileName, &m_size, bAddBasePath);

	return IsLoaded();
}

FileManager::FileManager()
{
	//LogMsg("File manager initted");
}

FileManager::~FileManager()
{
	list<FileSystem*>::iterator itor = m_fileSystems.begin();

	while (!m_fileSystems.empty())
	{
		FileSystem *pFileSystem = *m_fileSystems.begin();
		m_fileSystems.pop_front();
		delete pFileSystem;
	}

}

StreamingInstance * FileManager::GetStreaming( string fileName, int *pSizeOut, bool bAddBasePath )
{
	if (bAddBasePath)
	{
		fileName = GetBaseAppPath() + fileName;
	}

	StreamingInstance *pStreaming = NULL;

	//first check any mounted systems in reverse order of the mounting
	list<FileSystem*>::reverse_iterator itor = m_fileSystems.rbegin();

	while (itor != m_fileSystems.rend())
	{
		//(*itor)->GetDeliveryTime() > m->GetDeliveryTime())
		pStreaming = (*itor)->GetStreaming(fileName, pSizeOut);
		if (pStreaming) return pStreaming;
		itor++;
	}

	StreamingInstanceFile *pStreamingFile = new StreamingInstanceFile();
	if (!pStreamingFile->Open(fileName))
	{
#ifdef _DEBUG
LogMsg("Failed to stream %s, it won't open", fileName.c_str());
#endif
		delete pStreamingFile;
		return NULL;
	}

	return pStreamingFile;

}

byte * FileManager::Get( string fileName, int *pSizeOut, bool bAddBasePath, bool bAutoDecompress)
{
	
		byte * pData = NULL;

		//first check any mounted systems in reverse order of the mounting
		list<FileSystem*>::reverse_iterator itor = m_fileSystems.rbegin();

		while (itor != m_fileSystems.rend())
		{
			//(*itor)->GetDeliveryTime() > m->GetDeliveryTime())
			pData = (*itor)->Get(fileName, pSizeOut);
			if (pData) break; //guess we found it
			itor++;
		}

		if (!pData)
		{
	
			if (bAddBasePath)
			{
				fileName = GetBaseAppPath() + fileName;
			}

			//just try to load it from the default filesystem.  Should I create a FileSystemDefault for this to make it cleaner?
		
			FILE *fp = fopen(fileName.c_str(), "rb");
			if (!fp)
			{
				
				//not really an error, we might just want to know if a file exists
				//LogMsg("Warning: Proton FileManager says can't open %s", fileName.c_str());
				//file not found	
				if (!fp) return NULL;
			}

			fseek(fp, 0, SEEK_END);
			*pSizeOut = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			pData = (byte*)new byte[( (*pSizeOut) +1)];
			if (!pData)
			{
				LogError("Out of memory opening %s?", fileName.c_str());
				return 0;
			}
			//we add an extra null at the end to be nice, when loading text files this can be useful
			pData[*pSizeOut] = 0; 
			fread(pData, *pSizeOut, 1, fp);
			fclose(fp);
		}

		//also detect and perform any decompression here by checking the header..
		if (bAutoDecompress && IsAPackedFile(pData))
		{
			//let's decompress it to memory before passing it back
			unsigned int decompressedSize;
			byte *pDecompressedData = DecompressRTPackToMemory(pData, &decompressedSize);
			*pSizeOut = decompressedSize;
			SAFE_DELETE_ARRAY(pData);
			return pDecompressedData;
		}

		return pData;
}

void FileManager::MountFileSystem( FileSystem* pFileSystem )
{
	m_fileSystems.push_back(pFileSystem);
}

bool FileManager::FileExists( string fileName, bool bAddBasePath)
{
	if (bAddBasePath)
	{
		fileName = GetBaseAppPath() + fileName;
	}
	
	//first check any mounted systems in reverse order of the mounting
	list<FileSystem*>::reverse_iterator itor = m_fileSystems.rbegin();

	while (itor != m_fileSystems.rend())
	{
		//(*itor)->GetDeliveryTime() > m->GetDeliveryTime())
		if ((*itor)->FileExists(fileName)) return true;
		itor++;
	}

	//vanilla file system version

	FILE *fp = fopen( (fileName).c_str(), "rb");
	if (!fp)
	{
		//file not found	
		return NULL;
	}

	fclose(fp);
	return true;
}

int FileManager::GetFileSize( string fileName, bool bAddBasePath /*= true*/ )
{
	
	//first check any mounted systems in reverse order of the mounting
	list<FileSystem*>::reverse_iterator itor = m_fileSystems.rbegin();

	while (itor != m_fileSystems.rend())
	{
		//(*itor)->GetDeliveryTime() > m->GetDeliveryTime())
		int fileSize = ((*itor)->GetFileSize(fileName));
		if (fileSize >= 0) return fileSize;
		itor++;
	}

	if (bAddBasePath)
	{
		fileName = GetBaseAppPath() + fileName;
	}

	//vanilla file system version

	return ::GetFileSize(fileName);
}

FileSystem * FileManager::GetFileSystem( int index )
{
	if (index < 0 || index > (int)m_fileSystems.size()) return 0;

	list<FileSystem*>::iterator itor = m_fileSystems.begin();

	int count = 0;
	while (itor != m_fileSystems.end())
	{
		if (count++ == index) return (*itor);

		itor++;
	}


	return NULL;
}

bool FileManager::Copy(string srcFile, string dstFile, bool bAddBasePath)
{
	if (bAddBasePath)
	{
		srcFile = GetBaseAppPath() + srcFile;
		dstFile = GetBaseAppPath() + dstFile;
	}

	int size;
	StreamingInstance *pSrc = GetFileManager()->GetStreaming(srcFile, &size, false);
	if (!pSrc) 
	{
		LogMsg("Copy: Can't open input file of %s", srcFile.c_str());
		return false;
	}

	const int bufferSize = 512;
	byte buff[bufferSize];

	FILE *fp = fopen(dstFile.c_str(), "wb");
	if (!fp)
	{
		LogError("Unable to create file %s", dstFile.c_str());
		SAFE_DELETE(pSrc);
		return false;
	}

	while (!pSrc->IsFinished())
	{
		int bytesRead = pSrc->Read(buff, bufferSize);

		if (bytesRead > 0)
		{
			fwrite(buff, bytesRead, 1, fp);
		}
	}

	fclose(fp);
	SAFE_DELETE(pSrc);

	return true;
}
