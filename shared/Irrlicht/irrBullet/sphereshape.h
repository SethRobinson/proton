// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __SPHERE_SHAPE_H_INCLUDED__
#define __SPHERE_SHAPE_H_INCLUDED__


#include "collisionshape.h"
#include "Bullet/BulletCollision/CollisionShapes/btSphereShape.h"


class ISphereShape : public ICollisionShape
{
public:
    ISphereShape(irr::scene::ISceneNode *n, irr::f32 m, bool overrideMargin = false);

    virtual ~ISphereShape();

    void setUnscaledRadius(irr::f32 newRadius) { static_cast<btSphereShape*>(shape)->setUnscaledRadius(btScalar(newRadius)); };

    irr::f32 getRadius() { return static_cast<btSphereShape*>(shape)->getRadius(); };

protected:
    // Functions
    virtual void createShape(bool overrideMargin);
};


#endif // __SPHERE_SHAPE_H_INCLUDED__


