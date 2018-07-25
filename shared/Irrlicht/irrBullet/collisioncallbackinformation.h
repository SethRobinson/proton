// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __COLLISION_CALLBACK_INFORMATION_H__
#define __COLLISION_CALLBACK_INFORMATION_H__

#include <irrTypes.h>
#include <vector3d.h>
#include "irrbulletcommon.h"
#include "Bullet/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h"


class ICollisionObject;
class irrBulletWorld;

struct SManifoldPoint
{
    public:
        SManifoldPoint() {};
        SManifoldPoint(const btManifoldPoint& point)
        {
            bulletManifoldPoint = point;
        };

        void setInfo(const btManifoldPoint& newPoint) { bulletManifoldPoint = newPoint; };


        irr::f32 getDistance() const { return irr::f32(bulletManifoldPoint.getDistance()); };
        irr::u32 getLifeTime() const { return bulletManifoldPoint.getLifeTime(); };
        const irr::core::vector3df getPositionWorldOnA() const { return bulletToIrrlichtVector(bulletManifoldPoint.getPositionWorldOnA()); };
        const irr::core::vector3df getPositionWorldOnB() const { return bulletToIrrlichtVector(bulletManifoldPoint.getPositionWorldOnB()); };
        void setDistance(irr::f32 dist) { bulletManifoldPoint.setDistance(dist); };
        irr::f32 getAppliedImpulse() const { return bulletManifoldPoint.getAppliedImpulse(); };
        const irr::core::vector3df getLocalPointA() const { return bulletToIrrlichtVector(bulletManifoldPoint.m_localPointA); };
        const irr::core::vector3df getLocalPointB() const { return bulletToIrrlichtVector(bulletManifoldPoint.m_localPointB); };
        const irr::core::vector3df getNormalWorldOnB() const { return bulletToIrrlichtVector(bulletManifoldPoint.m_normalWorldOnB); };
        irr::f32 getCombinedFriction() const { return bulletManifoldPoint.m_combinedFriction; };
        irr::f32 getCombinedRestitution() const { return bulletManifoldPoint.m_combinedRestitution; };


        const btManifoldPoint& getUnderlyingManifoldPoint() const { return bulletManifoldPoint; };

    private:
        btManifoldPoint bulletManifoldPoint;
};


class ICollisionCallbackInformation
{
    public:
        ICollisionCallbackInformation(btPersistentManifold* const manifold, irrBulletWorld* world);

        /// Note: both of the getBody() functions return a collision object with a matching unique ID to the corresponding btCollisionObject
        ICollisionObject* getBody0() const;
        ICollisionObject* getBody1() const;

        SManifoldPoint& getContactPoint(irr::u32 index);

        btPersistentManifold* getPointer() const { return contactManifold; };


    private:
        btPersistentManifold* contactManifold;
        SManifoldPoint manifoldPoint;
        irrBulletWorld *dynamicsWorld;
};

#endif // __COLLISION_CALLBACK_INFORMATION_H__
