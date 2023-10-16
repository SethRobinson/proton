//  ***************************************************************
//  LibVlcStreamComponent - Creation date: 8/31/2023 2:52:20 PM
//  -------------------------------------------------------------
//  License: Uh, check for license.txt or license.md for that?
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include "Entity/Component.h"
#include "Manager/libVLC_RTSP.h"

class LibVlcStreamComponent : public EntityComponent
{
public:
	LibVlcStreamComponent();
	virtual ~LibVlcStreamComponent();

	virtual void OnAdd(Entity* pEnt);
	virtual void OnRemove();

	void Init(std::string url, int cacheMS);
	void OnUpdate(VariantList* pVList);
	void SetSurfaceSize(int width, int height);

	libVLC_RTSP* GetLibVlcRTSP() { return &m_libVlcRTSP; }

protected:

	void OnInputWhileMouseDown(VariantList* pVList);

	string m_url;
	int m_cacheMS = 0;
	libVLC_RTSP m_libVlcRTSP;
	SurfaceAnim* m_pSurface = NULL;
	CL_Vec2f* m_pPos2d = NULL;
	CL_Vec2f* m_pSize2d = NULL;
	CL_Vec2f* m_pScale2d = NULL;
	

};

//helper
LibVlcStreamComponent* AddNewStream(std::string name, std::string url, int cacheMS, Entity* pGUIEnt);
LibVlcStreamComponent* GetStreamEntityByName(std::string name);
void SetStreamVolumeByName(std::string name, float volume);