//  ***************************************************************
//  UnpackArchiveComponent - Creation date: 1/3/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef UnpackArchiveComponent_h__
#define UnpackArchiveComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "util/archive/TarHandler.h"

class UnpackArchiveComponent: public EntityComponent
{
public:
	UnpackArchiveComponent();
	virtual ~UnpackArchiveComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);

	CL_Vec2f *m_pPos2d;

	/*
	CL_Vec2f *m_pSize2d;
	float *m_pScale;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/

	string *m_pSrcFileName;
	string *m_pDestDirectory;
	uint32 *m_pDeleteSourceOnFinish;
	uint32 *m_pLimitToSingleSubdir;

	TarHandler m_tarHandler; 
};

#endif // UnpackArchiveComponent_h__