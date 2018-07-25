// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/btBulletCollisionCommon.h"

#include "raycastvehicle.h"
#include "collisionobject.h"
#include "irrbulletcommon.h"

// TODO: rotation values in the matrix are wrong when returned
void btTransformToIrrlichtMatrix(const btTransform& worldTrans, irr::core::matrix4 &matr)
{
    worldTrans.getOpenGLMatrix(matr.pointer());
}


void btTransformFromIrrlichtMatrix(const irr::core::matrix4& irrmat, btTransform &transform)
{
    transform.setIdentity();

    /*btScalar mat[16];

    for(irr::u32 i=0; i < 16; i++)
    {
        mat[i] = irrmat.pointer()[i];
    }*/

    transform.setFromOpenGLMatrix(irrmat.pointer());
    //transform.setFromOpenGLMatrix(mat);
}


void btWheelInfoToSWheelInfo(const btWheelInfo &btInfo, SWheelInfo &info, btTransform &transform)
{
    btTransformToIrrlichtMatrix(transform, info.worldTransform);
    info.chassisConnectionPointCS = bulletToIrrlichtVector(btInfo.m_chassisConnectionPointCS);
    info.wheelDirectionCS = bulletToIrrlichtVector(btInfo.m_wheelDirectionCS);
    info.wheelAxleCS = bulletToIrrlichtVector(btInfo.m_wheelAxleCS);
    info.suspensionRestLength = irr::f32(btInfo.m_suspensionRestLength1);
    info.maxSuspensionTravelCm = irr::f32(btInfo.m_maxSuspensionTravelCm);
    info.wheelRadius = irr::f32(btInfo.m_wheelsRadius);
    info.suspensionStiffness = irr::f32(btInfo.m_suspensionStiffness);
    info.wheelDampingCompression = irr::f32(btInfo.m_wheelsDampingCompression);
    info.wheelDampingRelaxation = irr::f32(btInfo.m_wheelsDampingRelaxation);
    info.frictionSlip = irr::f32(btInfo.m_frictionSlip);
    info.steering = irr::f32(btInfo.m_steering);
    info.wheelRotation = irr::f32(btInfo.m_rotation);
    info.deltaRotation = irr::f32(btInfo.m_deltaRotation);
    info.rollInfluence = irr::f32(btInfo.m_rollInfluence);
    info.engineForce = irr::f32(btInfo.m_engineForce);
    info.brake = irr::f32(btInfo.m_brake);
    info.isFrontWheel = btInfo.m_bIsFrontWheel;
    info.clippedInvContactDotSuspension = irr::f32(btInfo.m_clippedInvContactDotSuspension);
    info.suspensionRelativeVelocity = irr::f32(btInfo.m_suspensionRelativeVelocity);
    info.wheelSuspensionForce = irr::f32(btInfo.m_wheelsSuspensionForce);
    info.skidInfo = irr::f32(btInfo.m_skidInfo);

    info.raycastInfo.contactNormalWS = bulletToIrrlichtVector(btInfo.m_raycastInfo.m_contactNormalWS);
    info.raycastInfo.contactPointWS = bulletToIrrlichtVector(btInfo.m_raycastInfo.m_contactPointWS);
    info.raycastInfo.suspensionLength = irr::f32(btInfo.m_raycastInfo.m_suspensionLength);
    info.raycastInfo.hardPointWS = bulletToIrrlichtVector(btInfo.m_raycastInfo.m_hardPointWS);
    info.raycastInfo.wheelDirectionWS = bulletToIrrlichtVector(btInfo.m_raycastInfo.m_wheelDirectionWS);
    info.raycastInfo.wheelAxleWS = bulletToIrrlichtVector(btInfo.m_raycastInfo.m_wheelAxleWS);
    info.raycastInfo.isInContact = btInfo.m_raycastInfo.m_isInContact;
    info.raycastInfo.groundObject = btInfo.m_raycastInfo.m_groundObject;
}


void btWheelInfoFromSWheelInfo(const SWheelInfo &info, btWheelInfo &btInfo)
{
    btTransform transform;
    btTransformFromIrrlichtMatrix(info.worldTransform, transform);
    btInfo.m_worldTransform = transform;
    btInfo.m_chassisConnectionPointCS = irrlichtToBulletVector(info.chassisConnectionPointCS);
    btInfo.m_wheelDirectionCS = irrlichtToBulletVector(info.wheelDirectionCS);
    btInfo.m_wheelAxleCS = irrlichtToBulletVector(info.wheelAxleCS);
    btInfo.m_suspensionRestLength1 = btScalar(info.suspensionRestLength);
    btInfo.m_maxSuspensionTravelCm = btScalar(info.maxSuspensionTravelCm);
    btInfo.m_wheelsRadius = btScalar(info.wheelRadius);
    btInfo.m_suspensionStiffness = btScalar(info.suspensionStiffness);
    btInfo.m_wheelsDampingCompression = btScalar(info.wheelDampingCompression);
    btInfo.m_wheelsDampingRelaxation = btScalar(info.wheelDampingRelaxation);
    btInfo.m_frictionSlip = btScalar(info.frictionSlip);
    btInfo.m_steering = btScalar(info.steering);
    btInfo.m_rotation = btScalar(info.wheelRotation);
    btInfo.m_deltaRotation = btScalar(info.deltaRotation);
    btInfo.m_rollInfluence = btScalar(info.rollInfluence);
    btInfo.m_engineForce = btScalar(info.engineForce);
    btInfo.m_brake = btScalar(info.brake);
    btInfo.m_bIsFrontWheel = info.isFrontWheel;
    btInfo.m_clippedInvContactDotSuspension = btScalar(info.clippedInvContactDotSuspension);
    btInfo.m_suspensionRelativeVelocity = btScalar(info.suspensionRelativeVelocity);
    btInfo.m_wheelsSuspensionForce = btScalar(info.wheelSuspensionForce);
    btInfo.m_skidInfo = btScalar(info.skidInfo);
}


irr::core::vector3df compensateForNodeType(const irr::core::vector3df &scale, irr::scene::ESCENE_NODE_TYPE type)
{
    irr::f32 compensator = 1.0;

    if(type == irr::scene::ESNT_CUBE)
        compensator = 0.5;

    else
    if(type == irr::scene::ESNT_SPHERE)
        compensator = 5.0;

    else
    if(type == irr::scene::ESNT_CAMERA)
        compensator *= 0.001;

    return irr::core::vector3df(scale) * compensator;
}


btVector3 irrlichtToBulletVector(const irr::core::vector3df &vec)
{
    return btVector3(vec.X,vec.Y,vec.Z);
}


irr::core::vector3df bulletToIrrlichtVector(const btVector3 &vec)
{
    return irr::core::vector3df(vec.getX(),vec.getY(),vec.getZ());
}
