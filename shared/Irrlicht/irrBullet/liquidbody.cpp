// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "liquidbody.h"

#include "bulletworld.h"
#include <IrrlichtDevice.h>
#include <irrArray.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace io;

ILiquidBody::ILiquidBody(irrBulletWorld* const world, const irr::core::vector3df& pos,
    const irr::core::aabbox3df& aabb, irr::f32 waveFrequency, irr::f32 density, bool makeInfinite) :
    DynamicsWorld(world), LiquidExtents(aabb), GlobalWaveUpdateFrequency(waveFrequency), LiquidDensity(density),
    Device(DynamicsWorld->getIrrlichtDevice()), Infinite(makeInfinite)
{
    static u32 LiquidBodyID;
    LiquidBodyID++;

    UniqueID = LiquidBodyID;

    DebugDraw = true;
    EnableLocalWaves = true;
    EnableGlobalWave = true;
    LiquidRising = true;
    InfiniteDepth = false;
    ForceActivation = true;

    MaxEdge = LiquidExtents.MaxEdge;
    MinEdge = LiquidExtents.MinEdge;

    LiquidExtents.MaxEdge += pos;
    LiquidExtents.MinEdge += pos;

    LiquidLevel = LiquidExtents.MaxEdge.Y;

    LastGlobalWaveUpdateTime = DynamicsWorld->getIrrlichtDevice()->getTimer()->getTime();

    MaxGlobalWaveHeight = 4.0f; MinGlobalWaveHeight = -4.0f; GlobalWaveChangeIncrement = 0.1f; LiquidFriction = 0.25f;

    CurrentDirection = vector3df(0,0,0); CurrentSpeed = 1.0f;

    LocalWaveForceMax = 10, LocalWaveForceMin = 1, LocalWaveForceMultiplier = 0.1f;

    AngularLimitation = 0.995f;
}

void ILiquidBody::updateLiquidBody()
{
    if(EnableGlobalWave)
    {
        if(Device->getTimer()->getTime() >= (LastGlobalWaveUpdateTime+GlobalWaveUpdateFrequency))
        {
            if(LiquidRising)
            {
                if(LiquidExtents.MaxEdge.Y <= (LiquidLevel+MaxGlobalWaveHeight))
                    LiquidExtents.MaxEdge.Y += GlobalWaveChangeIncrement;

                else
                    LiquidRising = false;
            }

            else if(!LiquidRising)
            {
                if(LiquidExtents.MaxEdge.Y >= (LiquidLevel+MinGlobalWaveHeight))
                    LiquidExtents.MaxEdge.Y += -GlobalWaveChangeIncrement;

                else
                    LiquidRising = true;
            }

            LastGlobalWaveUpdateTime = Device->getTimer()->getTime();
        }
    }

    if(DebugDraw)
    {
        Device->getVideoDriver()->setMaterial(DynamicsWorld->getDebugMaterial());
        Device->getVideoDriver()->setTransform(irr::video::ETS_WORLD, irr::core::matrix4());

        Device->getVideoDriver()->draw3DBox(LiquidExtents, irr::video::SColor(255, 0, 0, 100));
    }

    for(u32 i=0; i < DynamicsWorld->getNumCollisionObjects(); i++)
    {
        ICollisionObject* obj = DynamicsWorld->getCollisionObject(i);

        if(obj->isLiquidSimulationEnabled())
        {
            if(obj->getObjectType() == ECOT_RIGID_BODY)
            {
                if((!Infinite && LiquidExtents.intersectsWithBox(static_cast<IRigidBody*>(obj)->getLiquidBox()))
                    ||
                    (Infinite && (static_cast<IRigidBody*>(obj)->getLiquidBox().MinEdge.Y<LiquidExtents.MaxEdge.Y
                    && (InfiniteDepth ? InfiniteDepth:(static_cast<IRigidBody*>(obj)->getLiquidBox().MaxEdge.Y>LiquidExtents.MinEdge.Y)))))
                    affectRigidBody(static_cast<IRigidBody*>(obj));
            }
        }
    }
}

void ILiquidBody::affectRigidBody(IRigidBody* const body)
{
    // Make sure bodies don't "fall asleep" while they're in water
    if(ForceActivation && (body->getActivationState() == EAS_SLEEPING || body->getActivationState() == EAS_WANTS_DEACTIVATION))
        body->activate();

    irr::core::vector3df CurrentPosition = irr::core::vector3df(0,0,0);
    irr::core::vector3df ForceDirection = irr::core::vector3df(0,0,0);

    if(body->getObjectType() != ECOT_RIGID_BODY)
        return;

    CurrentPosition = body->getCollisionShape()->getSceneNode()->getAbsolutePosition();
    /*else
        CurrentPosition = static_cast<ISoftBody*>(object)->getSceneNode()->getAbsolutePosition();*/

    const irr::core::array<SBuoyancyPoint>& points = body->getBuoyancyPoints();

    body->setAngularVelocity(body->getAngularVelocity()*(AngularLimitation));

    for(irr::u32 i=0; i < points.size(); i++)
    {
        if(body->getObjectType() == ECOT_RIGID_BODY)
        {
            irr::core::matrix4 mat = body->getCollisionShape()->getSceneNode()->getAbsoluteTransformation();
            const irr::core::matrix4 mat2 = mat;

            irr::core::vector3df offset(points[i].localPoint);

            mat.transformVect(offset);

            mat.setTranslation(offset);

            const irr::core::matrix4 w2n(mat2, irr::core::matrix4::EM4CONST_INVERSE);

            mat = (w2n*mat);

            if((!Infinite && LiquidExtents.isPointInside(offset)) ||
                (Infinite && ((offset.Y < LiquidExtents.MaxEdge.Y) && (InfiniteDepth ? InfiniteDepth:(offset.Y > LiquidExtents.MinEdge.Y)))))
            {
                if(DebugDraw)
                {
                    Device->getVideoDriver()->draw3DLine(
                        offset, offset+irr::core::vector3df(0,10,0), irr::video::SColor(255, 0, 0, 255));
                }

                ForceDirection = irr::core::vector3df(0,LiquidDensity*((LiquidExtents.MaxEdge.Y-offset.Y)*points[i].buoyancy),0);
                ForceDirection += (CurrentDirection*CurrentSpeed);

                //ForceDirection.Y = LiquidExtents.MaxEdge.Y-offset.Y;
                if(ForceDirection.Y > 10) ForceDirection.Y = 10;


                if(EnableLocalWaves)
                {
                    srand(Device->getTimer()->getTime()+body->getUniqueID());

                    ForceDirection.Y += (rand() % LocalWaveForceMax + LocalWaveForceMin)*LocalWaveForceMultiplier;
                }

                body->applyForce(-(body->getVelocityInLocalPoint(mat.getTranslation())*LiquidFriction), mat.getTranslation());
                body->applyForce(ForceDirection, mat.getTranslation());
            }
        }
    }

    //if(EnableWaves)
        //LiquidExtents.MaxEdge.Y = y;
}

ILiquidBody::~ILiquidBody()
{
    printf("irrBullet: Removing Liquid body (%u)\n", UniqueID);
}
