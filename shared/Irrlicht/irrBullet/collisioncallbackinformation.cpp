// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "collisioncallbackinformation.h"
#include "Bullet/BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "collisionobject.h"
#include "bulletworld.h"

ICollisionCallbackInformation::ICollisionCallbackInformation(btPersistentManifold* const manifold, irrBulletWorld* world)
{
    contactManifold = manifold;
    dynamicsWorld = world;
}

ICollisionObject* ICollisionCallbackInformation::getBody0() const
{
    SCollisionObjectIdentification* identification =
        static_cast<SCollisionObjectIdentification*>(static_cast<btCollisionObject*>(contactManifold->getBody0())->getUserPointer());

    return identification->getCollisionObject();

    return 0;
}

ICollisionObject* ICollisionCallbackInformation::getBody1() const
{
    SCollisionObjectIdentification* identification =
        static_cast<SCollisionObjectIdentification*>(static_cast<btCollisionObject*>(contactManifold->getBody1())->getUserPointer());

    return identification->getCollisionObject();

    return 0;
}


SManifoldPoint& ICollisionCallbackInformation::getContactPoint(irr::u32 index)
{
    btManifoldPoint &point = contactManifold->getContactPoint(index);
    manifoldPoint.setInfo(point);
    return manifoldPoint;
}
