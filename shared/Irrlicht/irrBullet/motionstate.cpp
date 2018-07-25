// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/btBulletCollisionCommon.h"
#include "motionstate.h"
#include "bulletworld.h"
#include <ISceneManager.h>

using namespace irr;
using namespace core;
using namespace scene;

IMotionState::IMotionState(const btTransform &initialPos)
{
    worldTransform = initialPos;
    ManualRotation = false;
    ManualPosition = false;
    VelocityAsRotation = false;
}

void IMotionState::getWorldTransform(btTransform &worldTrans) const
{
    worldTrans = worldTransform;
}

irr::core::matrix4& IMotionState::getWorldTransformationMatrix()
{
    btTransformToIrrlichtMatrix(worldTransform, TransformMatrix);
    return TransformMatrix;
}

void IMotionState::setWorldTransform(const btTransform &worldTrans)
{
    if(RigidBody)
    {
        ISceneNode *node = RigidBody->getCollisionShape()->getSceneNode();

        irr::core::matrix4 matr;
        btTransformToIrrlichtMatrix(worldTrans, matr);

        /*if(node->getParent() != 0)
        {
            if(node->getParent() != node->getSceneManager()->getRootSceneNode())
            {
                node->getParent()->updateAbsolutePosition();
                node->getParent()->getParent()->updateAbsolutePosition();
                const core::matrix4 w2n(node->getParent()->getAbsoluteTransformation(), core::matrix4::EM4CONST_INVERSE);

                matr = (w2n*matr);
            }
        }*/

        if(!ManualPosition)
            node->setPosition(matr.getTranslation());

        node->updateAbsolutePosition();

        if(!ManualRotation)
        {
            // The velocity as rotation calculation is wrong and doesn't work. If you can figure out how to fix this
            // before it's fixed in a future release, please email me: fighterstw@hotmail.com
            if(VelocityAsRotation)
            {
                vector3df rot, dif = matr.getTranslation() - RigidBody->getLinearVelocity();
                rot.Y = atan2( dif.X, dif.Z ) * 180.f / irr::core::PI;
                rot.X = atan2( dif.Y, sqrt( dif.X * dif.X + dif.Z * dif.Z ) ) * 180.f / irr::core::PI;
                node->setRotation(rot);
            }
            else
                node->setRotation(matr.getRotationDegrees());
        }

        if(RigidBody->isLiquidSimulationEnabled())
            RigidBody->updateLiquidBox();

        worldTransform = worldTrans;
    }

    else if(failed == false)
            printf("irrBullet: [ERR] Object could not be updated\n");
}


IMotionState::~IMotionState()
{
}

