// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __COLLISION_OBJECT_AFFECTOR_ATTRACT_H__
#define __COLLISION_OBJECT_AFFECTOR_ATTRACT_H__

#include "collisionobjectaffector.h"
#include <irrTypes.h>
#include <vector3d.h>
#include <ISceneNode.h>

class ICollisionObject;

/*!
    The delete affector adds a collision object to the deletion queue(and its corresponding scene node, if RemoveNode is true)
    from the dynamics world after the given time (TimeDelay) has elapsed.
*/
class ICollisionObjectAffectorAttract : public ICollisionObjectAffector
{
    public:
        /*!
            @param delay The amount of time to wait before removing the object
            @param remNode If this is true, the corresponding scene node will also be removed.
        */
        ICollisionObjectAffectorAttract(const irr::core::vector3df& target, irr::f32 force);
        ICollisionObjectAffectorAttract(irr::scene::ISceneNode* const target, irr::f32 force);
        virtual ~ICollisionObjectAffectorAttract();

        virtual void affectObject(ICollisionObject* object, irr::u32 timeMS);


    protected:
        irr::core::vector3df TargetPosition;
        irr::core::vector3df CurrentPosition;
        irr::core::vector3df ForceDirection;
        irr::scene::ISceneNode* TargetNode;
        irr::f32 PullForce;
        bool FirstRun;
};

#endif // __COLLISION_OBJECT_AFFECTOR_DELETE_H__
