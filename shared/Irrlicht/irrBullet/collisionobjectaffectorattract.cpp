// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "collisionobjectaffectorattract.h"
#include "bulletworld.h"
#include "collisionobject.h"
#include <vector3d.h>
#include <IrrlichtDevice.h>
#include <IVideoDriver.h>
#include <SColor.h>
#include <IMeshSceneNode.h>

ICollisionObjectAffectorAttract::ICollisionObjectAffectorAttract(const irr::core::vector3df& target, irr::f32 force)
{
    TargetPosition = target;
    TargetNode = 0;
    PullForce = force;
    Type = ECOAT_ATTRACT_AFFECTOR;
}

ICollisionObjectAffectorAttract::ICollisionObjectAffectorAttract(irr::scene::ISceneNode* const target, irr::f32 force)
{
    TargetNode = target;
    TargetPosition = TargetNode->getAbsolutePosition();
    PullForce = force;
    Type = ECOAT_ATTRACT_AFFECTOR;
}

void ICollisionObjectAffectorAttract::affectObject(ICollisionObject* object, irr::u32 timeMS)
{
    if(object->getObjectType() == ECOT_RIGID_BODY)
        CurrentPosition = static_cast<IRigidBody*>(object)->getCollisionShape()->getSceneNode()->getAbsolutePosition();
    else
        CurrentPosition = static_cast<ISoftBody*>(object)->getSceneNode()->getAbsolutePosition();

    if(TargetNode != 0)
        TargetPosition = TargetNode->getAbsolutePosition();

    ForceDirection = ((TargetPosition-CurrentPosition)*PullForce)*(1/CurrentPosition.getDistanceFrom(TargetPosition));

    /*object->getDynamicsWorld()->getIrrlichtDevice()->getVideoDriver()->setMaterial(object->getDynamicsWorld()->getDebugMaterial());
    object->getDynamicsWorld()->getIrrlichtDevice()->getVideoDriver()->setTransform(irr::video::ETS_WORLD, irr::core::matrix4());

    object->getDynamicsWorld()->getIrrlichtDevice()->getVideoDriver()->draw3DLine(
        CurrentPosition, ForceDirection, irr::video::SColor(255, 255, 255, 255));*/

    if(object->getObjectType() == ECOT_RIGID_BODY)
    {
        static_cast<IRigidBody*>(object)->applyCentralForce(ForceDirection);
    }

    else if(object->getObjectType() == ECOT_SOFT_BODY)
    {
        static_cast<ISoftBody*>(object)->addForce(ForceDirection);
    }
}

ICollisionObjectAffectorAttract::~ICollisionObjectAffectorAttract()
{
    //dtor
}
