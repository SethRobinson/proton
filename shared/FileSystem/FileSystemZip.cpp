#include "PlatformPrecomp.h"
#include "FileSystemZip.h"
#include "FileSystem/StreamingInstanceZip.h"

#define CASESENSITIVITY (0)

FileSystemZip::FileSystemZip()
{
	m_uf = NULL;
}

FileSystemZip::~FileSystemZip()
{
	if (m_uf)
	unzClose(m_uf);
}

bool FileSystemZip::Init(string zipFileName)
{
	assert(!m_uf && "We don't handle calling this twice yet");

	m_uf = unzOpen(zipFileName.c_str());
	if (!m_uf)
	{
		LogError("Cannot mount virtual file system using %s",zipFileName.c_str());
		return false;
	}
	LogMsg("Mounted zip as file system: %s", zipFileName.c_str());

	m_zipFileName = zipFileName; //save it in case we need to spawn more zip readers for streaming operations
	
	CacheIndex();
	return true; //success
}

void FileSystemZip::CacheIndex()
{
	assert(m_cache.empty() && "Why would you want to call this twice?");

	uLong i;
	unz_global_info gi;
	int err;

	err = unzGetGlobalInfo (m_uf,&gi);

	if (err!=UNZ_OK)
	{
		LogError("error %d with zipfile in unzGetGlobalInfo \n",err);
		return;
	}
	unzGoToFirstFile(m_uf);

	ZipCacheEntry entry;

	for (i=0;i<gi.number_entry;i++)
	{
		char filename_inzip[512];
		unz_file_info file_info;
		err = unzGetCurrentFileInfo(m_uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
	
		if (err!=UNZ_OK)
		{
			LogError("error %d with zipfile in unzGetCurrentFileInfo\n",err);
			break;
		}
		
		err = unzGetFilePos(m_uf, &entry.m_filepos);
		if (err!=UNZ_OK)
		{
			LogError("error %d with zipfile in unzGetFilePos\n",err);
			break;
		}
		m_cache[filename_inzip] = entry;

		if ((i+1)<gi.number_entry)
		{
			err = unzGoToNextFile(m_uf);
			if (err!=UNZ_OK)
			{
				LogError("error %d with zipfile in unzGoToNextFile\n",err);
				break;
			}
		}
	}

	LogMsg("Cache has %d files.", m_cache.size());
}

vector<string> FileSystemZip::GetContents()
{
	vector<string> m_contents;

	uLong i;
	unz_global_info gi;
	int err;

	err = unzGetGlobalInfo (m_uf,&gi);

	if (err!=UNZ_OK)
	{
		LogError("error %d with zipfile in unzGetGlobalInfo \n",err);
		return m_contents;
	}

	unzGoToFirstFile(m_uf);

	for (i=0;i<gi.number_entry;i++)
	{
		char filename_inzip[512];
		unz_file_info file_info;
		err = unzGetCurrentFileInfo(m_uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
		
		if (err!=UNZ_OK)
		{
			LogError("error %d with zipfile in unzGetCurrentFileInfo\n",err);
			break;
		}
		m_contents.push_back(string(filename_inzip));

		if ((i+1)<gi.number_entry)
		{
			err = unzGoToNextFile(m_uf);
			if (err!=UNZ_OK)
			{
				LogError("error %d with zipfile in unzGoToNextFile\n",err);
				break;
			}
		}
	}

	return m_contents;
}


byte * FileSystemZip::Get( string fileName, int *pSizeOut )
{
	zipCacheMap::iterator itor = m_cache.find(m_rootDir+fileName);

	if (itor == m_cache.end())
	{
		return NULL; //not found in this zip
		//bingo!
	}
	
	int err = UNZ_OK;

	err = unzGoToFilePos(m_uf, &itor->second.m_filepos);
	
	if (err!=UNZ_OK)
	{
		LogError("error %d with zipfile in unzGoToFilePos",err);
		return NULL;
	}

	/*
	//old slow way of locating a file
	if (unzLocateFile(m_uf,(m_rootDir+fileName).c_str(),CASESENSITIVITY)!=UNZ_OK)
	{
		return NULL;
	}
	*/

	unz_file_info file_info;

	char st_filename_inzip[512];

	
	err = unzGetCurrentFileInfo(m_uf,&file_info,st_filename_inzip,sizeof(st_filename_inzip),NULL,0,NULL,0);

	if (err!=UNZ_OK)
	{
		LogError("error %d with zipfile in unzGetCurrentFileInfo",err);
		return NULL;
	}
	

	//let's allocate our own memory and pass the pointer back to them.
	byte *pBytes = new byte[file_info.uncompressed_size+1]; //the extra is because I will add a null later, helps when processing
	//text files and can't hurt
	
	
	if (pBytes)
	{
		//memory allocated
		*pSizeOut =  file_info.uncompressed_size;
		pBytes[file_info.uncompressed_size] = 0;
	}   else
	{
		LogError("Couldn't allocate the required %d bytes to unzip into.", file_info.uncompressed_size+1);
		return NULL;
	}

	err = unzOpenCurrentFile(m_uf);

	if (err!=UNZ_OK)
	{
		LogError("error %d with zipfile in unzOpenCurrentFile",err);
		return NULL;
	}

	err = unzReadCurrentFile(m_uf,pBytes,file_info.uncompressed_size);
	if (err<0)	
	{
		LogError("error %d with zipfile in unzReadCurrentFile",err);
		return NULL;

	}

	err = unzCloseCurrentFile(m_uf);
	if (err!=UNZ_OK)
	{
		LogError("error %d with zipfile in unzCloseCurrentFile",err);
		return NULL;
	}

	return pBytes;
}

void FileSystemZip::SetRootDirectory( string rootDir )
{
	m_rootDir = rootDir+"/";
}

StreamingInstance * FileSystemZip::GetStreaming( string fileName, int *pSizeOut )
{
	zipCacheMap::iterator itor = m_cache.find(m_rootDir+fileName);

	if (itor == m_cache.end())
	{
		return NULL; //not found in this zip
	}

	
	//ok, we now now that it exists.  Let's create a streaming zip reader and return it.
	//To be able to properly handle multiple file access on the same zip, we have to create a new object that tracks the
	//zip itself.


//	int err = UNZ_OK;

//	err = unzGoToFilePos(m_uf, &itor->second.m_filepos);

	

	StreamingInstanceZip *pStream = new StreamingInstanceZip;

	if (!pStream->Init(m_zipFileName))
	{
		LogMsg("Error opening zip %s for stream", m_zipFileName.c_str());
		delete pStream;
		return NULL;
	}

	if (!m_rootDir.empty())
	{
		pStream->SetRootDirectory(m_rootDir.substr(0, m_rootDir.length()-1));
	}
	if (!pStream->OpenWithCacheEntry(&itor->second))
	{
		LogMsg("Error opening the file %s from the zip %s.", fileName.c_str(), m_zipFileName.c_str());
		delete pStream;
		return NULL;
	}
	
	return pStream;
}

bool FileSystemZip::FileExists( string fileName )
{
	
	zipCacheMap::iterator itor = m_cache.find(m_rootDir+fileName);

	if (itor == m_cache.end())
	{
		return NULL; //not found in this zip
	}
	return true;
}

int FileSystemZip::GetFileSize( string fileName )
{
	
	zipCacheMap::iterator itor = m_cache.find(m_rootDir+fileName);

	if (itor == m_cache.end())
	{
		return -1; //not found in this zip
	}

	int err = UNZ_OK;

	err = unzGoToFilePos(m_uf, &itor->second.m_filepos);

	if (err!=UNZ_OK)
	{
		LogError("error %d with unzGoToFilePos in unzGetCurrentFileInfo",err);
		return -1;
	}

	unz_file_info file_info;
	char st_filename_inzip[512];

	err = unzGetCurrentFileInfo(m_uf,&file_info,st_filename_inzip,sizeof(st_filename_inzip),NULL,0,NULL,0);

	if (err!=UNZ_OK)
	{
		LogError("error %d with zipfile in unzGetCurrentFileInfo",err);
		return false;
	}

//	LogMsg("File is filesize %d", (int)file_info.uncompressed_size );
	return (int)file_info.uncompressed_size;
}