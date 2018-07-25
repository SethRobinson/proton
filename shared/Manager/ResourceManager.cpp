#include "PlatformPrecomp.h"
#include "ResourceManager.h"
#include "Renderer/SurfaceAnim.h"

#ifdef RT_SPRITEANIMATION
#include "Renderer/SpriteAnimation.h"
#endif

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	KillAllResources();
}

ResourceManager::Resource * ResourceManager::FindDataByKey(const string &keyName)
{
	ResourceMap::iterator itor = m_data.find(keyName);

	if (itor != m_data.end())
	{
		//found it
		return itor->second;
	}
	return NULL; //fail
}

bool ResourceManager::RemoveResource(const string &fileName, bool bUnloadFileButDontDelete)
{
	ResourceMap::iterator itor = m_data.find(fileName);
	if (itor != m_data.end())
	{
		if (bUnloadFileButDontDelete)
		{
			itor->second->m_pSurface->Kill();
		} else
		{
			delete itor->second;
			m_data.erase(itor);
		}
	   
		//LogMsg("Removing %s from memory", fileName.c_str());
		return true;
	}
	
	return false;
}

SurfaceAnim * ResourceManager::GetSurfaceAnim(const string &fileName, bool bAddBasePath)
{
	return GetSurfaceResource<SurfaceAnim>(fileName, Surface::TYPE_GUI, bAddBasePath);
}

#ifdef RT_SPRITEANIMATION
SpriteAnimationSet * ResourceManager::GetSpriteAnimationSet(const string &fileName)
{
	if (fileName.empty()) return NULL;

	Resource *pData = FindDataByKey(fileName);
	if (!pData)
	{
		SpriteAnimationSet *pSpriteAnimationSet = new SpriteAnimationSet;

		if (!pSpriteAnimationSet->LoadFile(fileName))
		{
			delete pSpriteAnimationSet;

			LogMsg("ResourceManager::GetSpriteAnimationSet: Unable to load %s", fileName.c_str());
			return NULL;
		}

		pData = new Resource;
		if (!pData)
		{
			delete pSpriteAnimationSet;
			return NULL;
		}

		pData->m_type = Resource::TYPE_SPRITE_ANIMATION_SET;
		pData->m_pSpriteAnimationSet = pSpriteAnimationSet;
		m_data[fileName] = pData;
	}

	if (pData->m_type == Resource::TYPE_SPRITE_ANIMATION_SET)
	{
		return pData->m_pSpriteAnimationSet;
	} else
	{
		LogMsg("ResourceManager::GetSpriteAnimationSet: Requested resource %s is not of type 'animation set'.", fileName.c_str());
		return NULL;
	}
}
#endif

void ResourceManager::KillAllResources()
{
	ResourceMap::iterator itor = m_data.begin();
	while (itor != m_data.end())
	{
		delete itor->second;
		itor++;
	}

	m_data.clear();
}

void ResourceManager::RemoveTexturesNotInExclusionList( const vector<string> &exclusionList )
{
	ResourceMap::iterator itor = m_data.begin();
	while (itor != m_data.end())
	{
		if (itor->second->m_type != Resource::TYPE_SURFACE)
		{
			itor++;
			continue;
		}

		bool bIgnore = false;

		for (unsigned int i=0; i < exclusionList.size(); i++)
		{
			if (itor->first == exclusionList[i])
			{
				bIgnore = true;
				break;
			}
		}

		if (bIgnore)
		{
			itor++;
			continue;
		}
		ResourceMap::iterator itorTemp = itor;
		itor++;

		delete itorTemp->second;
		m_data.erase(itorTemp);
	}
}

bool ResourceManager::IsResourceLoaded( string fName )
{
	return FindDataByKey(fName) != NULL;
}

ResourceManager::Resource::~Resource()
{
	switch (m_type)
	{
	case TYPE_SURFACE:
		delete m_pSurface;
		break;

#ifdef RT_SPRITEANIMATION
	case TYPE_SPRITE_ANIMATION_SET:
		delete m_pSpriteAnimationSet;
		break;
#endif

	default:
		LogMsg("Wat is type %d", m_type);
		assert(!"Huh?");
	}
}
