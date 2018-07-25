//  ***************************************************************
//  ResourceManager - Creation date: 07/01/2009
//  -------------------------------------------------------------
//  Robinson Technologies (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ResourceManager_h__
#define ResourceManager_h__

#include "Renderer/Surface.h"
class SurfaceAnim;

#ifdef RT_SPRITEANIMATION
class SpriteAnimationSet;
#endif

/**
 * A manager for resources.
 *
 * A resource is something that can be loaded from a file, such as an image.
 *
 * The \c ResourceManager is useful in preventing the reading of the same file over
 * and over again. The file reading only needs to happen once and thereafter the data
 * is accessible through this class.
 *
 * The resources are identified by their filenames from which they are loaded.
 */
class ResourceManager
{
public:
	ResourceManager();
	virtual ~ResourceManager();

	/**
	 * Gets a \c SurfaceAnim resource.
	 *
	 * This is identical to calling:
	 * \code
	 * GetSurfaceResource<SurfaceAnim>(fileName, Surface::TYPE_GUI);
	 * \endcode
	 *
	 * \see GetSurfaceResource()
	 */
	SurfaceAnim * GetSurfaceAnim(const string &fileName, bool bAddBasePath = true);

	/**
	 * Gets a resource of any class that is or is inherited from \c Surface.
	 *
	 * Returns \c NULL if the resource is not found or it has been previously loaded
	 * into a different type of \c Surface (that can't be dynamically cast to the
	 * requested type).
	 *
	 * The \a textureType argument sets the texture type for the \c Surface. This
	 * argument however only has effect if the \c Surface has not been previously loaded.
	 */
	template<class T>
	T * GetSurfaceResource(const string &fileName, Surface::eTextureType textureType = Surface::TYPE_DEFAULT,
		bool bAddBasePath = true)
	{
		if (fileName.empty()) return NULL;

		Resource *pData = FindDataByKey(fileName);
		if (!pData)
		{
			Surface *pSurf = new T;

			pSurf->SetTextureType(textureType);

			if (!pSurf->LoadFile(fileName, bAddBasePath))
			{
				delete pSurf;

				LogMsg("ResourceManager::GetSurfaceResource: Unable to load %s", fileName.c_str());
				return NULL;
			}

			pData = new Resource;
			if (!pData)
			{
				delete pSurf;
				return NULL;
			}

			pData->m_type = Resource::TYPE_SURFACE;
			pData->m_pSurface = pSurf;
			m_data[fileName] = pData;
		}

		if (pData->m_type == Resource::TYPE_SURFACE)
		{
			return dynamic_cast<T*>(pData->m_pSurface);
		} else
		{
			return NULL;
		}
	}

	bool IsResourceLoaded(string fName);

#ifdef RT_SPRITEANIMATION
	/**
	 * Gets a \c SpriteAnimationSet resource loaded from file \a fileName.
	 *
	 * Returns \c NULL if the resource is not found or can't be loaded for any reason.
	 */
	SpriteAnimationSet * GetSpriteAnimationSet(const string &fileName);
#endif

	/**
	 * Removes and destroys all the resources currently in this resource manager.
	 */
	void KillAllResources();
	void RemoveTexturesNotInExclusionList(const vector<string> &exclusionList);
	bool RemoveResource(const string &fileName, bool bUnloadFileButDontDelete);

private:
	class Resource
	{
	public:
		enum eResourceType
		{
			TYPE_UNKNOWN,
			TYPE_SURFACE
#ifdef RT_SPRITEANIMATION
			, TYPE_SPRITE_ANIMATION_SET
#endif
		};

		Resource() { m_pSurface = NULL; m_type = TYPE_UNKNOWN; }
		~Resource();

		union {
			Surface *m_pSurface;
#ifdef RT_SPRITEANIMATION
			SpriteAnimationSet *m_pSpriteAnimationSet;
#endif
		};

		eResourceType m_type;
	};

	Resource * FindDataByKey(const string &keyName);
	typedef std::map<string, Resource*> ResourceMap;
	ResourceMap m_data;
};

#endif // ResourceManager_h__
