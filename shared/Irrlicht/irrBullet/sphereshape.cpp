// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/btBulletCollisionCommon.h"
#include "sphereshape.h"

using namespace irr;
using namespace core;
using namespace scene;

ISphereShape::ISphereShape(ISceneNode *n, f32 m, bool overrideMargin)
{
    node = n;
    mass = m;

    type = ECST_SPHERE;

    createShape(overrideMargin);
}

void ISphereShape::createShape(bool overrideMargin)
{
    node->updateAbsolutePosition();
    /*const aabbox3df& box = node->getTransformedBoundingBox();
    const vector3df& diag = (box.MaxEdge - box.getCenter());
    const f32 radius = diag.getLength() * 0.53;*/
    const aabbox3df& box = node->getTransformedBoundingBox();
    const vector3df& diag = (box.MaxEdge - box.getCenter()) + f32((overrideMargin) ? 0.04:0.0);
    const f32 radius = diag.getLength() * 0.5;


	shape = new btSphereShape(radius);
	//setLocalScaling(node->getScale(), ESP_COLLISIONSHAPE);
	calculateLocalInertia(getMass(), vector3df(0.0f,0.0f,0.0f));
}

ISphereShape::~ISphereShape()
{
}

