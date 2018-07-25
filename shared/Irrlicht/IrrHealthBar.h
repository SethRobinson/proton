//  ***************************************************************
//  IrrHealthBar - Creation date: 02/02/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef IrrHealthBar_h__
#define IrrHealthBar_h__

#include "IrrlichtManager.h"

class IrrHealthBar
{
public:
	IrrHealthBar();
	virtual ~IrrHealthBar();

	bool Init(scene::ISceneNode *pParent, core::vector3df vOffset, core::dimension2df vSize, float healthPercent);
	void Update(); //run every tick for the animation/interpolation

	void SetHealthTarget(float target);
	void SetVisible(bool bNew);

protected:
	

private:

	core::dimension2df m_vSize;
	scene::IBillboardSceneNode* m_bb;
};

#endif // IrrHealthBar_h__