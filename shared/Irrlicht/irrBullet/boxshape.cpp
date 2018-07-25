// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include <ISceneNode.h>
#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/btBulletCollisionCommon.h"
#include "boxshape.h"

using namespace irr;
using namespace core;
using namespace scene;

IBoxShape::IBoxShape(ISceneNode *n, f32 m, bool overrideMargin)
{
    node = n;
    mass = m;

    type = ECST_BOX;

    createShape(overrideMargin);
}

void IBoxShape::createShape(bool overrideMargin)
{
    node->updateAbsolutePosition();

    const vector3df& extent = node->getBoundingBox().getExtent() + f32((overrideMargin) ? 0.04:0.0);

    if(node->getType() == ESNT_CUBE)
    {
        shape = new btBoxShape(irrlichtToBulletVector(extent));
    }

    else
    {
        shape = new btBoxShape(irrlichtToBulletVector(extent * 0.5));
    }

	setLocalScaling(node->getScale(), ESP_COLLISIONSHAPE);
	calculateLocalInertia(getMass(), vector3df(0.0f,0.0f,0.0f));
}

IBoxShape::~IBoxShape()
{
}
