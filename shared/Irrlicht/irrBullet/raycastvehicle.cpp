// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/btBulletCollisionCommon.h"
#include <ISceneManager.h>
#include <IFileSystem.h>
#include "raycastvehicle.h"
#include "bulletworld.h"
#include "rigidbody.h"
#include "irrbullet_compile_config.h"

using namespace irr;
using namespace core;

IRaycastVehicle::IRaycastVehicle(IRigidBody* const body, irrBulletWorld* const world, const vector3d<s32>& coordSys) :
    rigidBody(body), axes(coordSys)
{
    #ifdef IRRBULLET_DEBUG_MODE
        printf("irrBullet: Creating raycast vehicle (BODY: %i)\n", body->getUniqueID());
    #endif

    vehicleRaycaster = new IVehicleRaycaster(world->getPointer());
    usesOriginalRaycaster = true;

    raycastVehicle = new btRaycastVehicle(vehicleTuning,rigidBody->getPointer(), vehicleRaycaster);

    raycastVehicle->setCoordinateSystem(axes.X,axes.Y,axes.Z); // 0 = Sideways, 1 = Up, 2 = Forward

    attributes = getRigidBody()->getCollisionShape()->getSceneNode()->getSceneManager()->getFileSystem()->createEmptyAttributes(
        getRigidBody()->getCollisionShape()->getSceneNode()->getSceneManager()->getVideoDriver());
}

IRaycastVehicle::IRaycastVehicle(IRigidBody* const body, irrBulletWorld* const world,
    btVehicleRaycaster* const raycaster, const vector3d<s32>& coordSys) : vehicleRaycaster(raycaster),
    rigidBody(body), axes(coordSys), usesOriginalRaycaster(false)
{
    #ifdef IRRBULLET_DEBUG_MODE
        printf("irrBullet: Creating raycast vehicle (BODY: %i)\n", body->getUniqueID());
    #endif

    raycastVehicle = new btRaycastVehicle(vehicleTuning,rigidBody->getPointer(), vehicleRaycaster);

    raycastVehicle->setCoordinateSystem(axes.X,axes.Y,axes.Z); // 0 = Sideways, 1 = Up, 2 = Forward

    attributes = getRigidBody()->getCollisionShape()->getSceneNode()->getSceneManager()->getFileSystem()->createEmptyAttributes(
        getRigidBody()->getCollisionShape()->getSceneNode()->getSceneManager()->getVideoDriver());
}



SWheelInfo& IRaycastVehicle::addWheel(const SWheelInfoConstructionInfo& info)
{
	btWheelInfo btInfo = raycastVehicle->addWheel(irrlichtToBulletVector(info.chassisConnectionPointCS),
        irrlichtToBulletVector(info.wheelDirectionCS),
        irrlichtToBulletVector(info.wheelAxleCS),
        info.suspensionRestLength,
        info.wheelRadius,
        vehicleTuning,
        info.isFrontWheel);

    // Convert btWheelInfo to SWheelInfo
    btWheelInfoToSWheelInfo(btInfo, wheelInfo, btInfo.m_worldTransform);
    return wheelInfo;
}



void IRaycastVehicle::updateWheelInfo(u32 wheelID)
{
    btWheelInfo& wid = getPointer()->getWheelInfo(wheelID);

    // Convert SWheelInfo to btWheelInfo
    btWheelInfoFromSWheelInfo(wheelInfo, wid);
}


void IRaycastVehicle::resetSuspension()
{
    getPointer()->resetSuspension();
}


void IRaycastVehicle::setSteeringValue(f32 steering, u32 wheelID)
{
    getPointer()->setSteeringValue(steering, wheelID);
}


void IRaycastVehicle::applyEngineForce(irr::f32 force, irr::u32 wheelID)
{
    getPointer()->applyEngineForce(force, wheelID);
}


void IRaycastVehicle::updateWheelTransform(irr::u32 wheelID, bool interpolatedTransform)
{
    getPointer()->updateWheelTransform(wheelID, interpolatedTransform);
}


// Undefined reference with this one. Disabled for now.
/*void IRaycastVehicle::setRaycastWheelInfo(irr::u32 wheelID, bool isInContact,
    const irr::core::vector3df &hitPoint, const irr::core::vector3df &hitNormal, irr::f64 depth)
{
    getPointer()->setRaycastWheelInfo(wheelID, isInContact,
        irrlichtToBulletVector(hitPoint), irrlichtToBulletVector(hitNormal), depth);
}*/


void IRaycastVehicle::setBrake(irr::f32 brake, irr::u32 wheelID)
{
    getPointer()->setBrake(brake, wheelID);
}


void IRaycastVehicle::setPitchControl(irr::f32 pitch)
{
    getPointer()->setPitchControl(pitch);
}


void IRaycastVehicle::setCoordinateSystem(const irr::core::vector3d<irr::s32>& coordSys)
{
    axes = coordSys;
    getPointer()->setCoordinateSystem(coordSys.X,coordSys.Y,coordSys.Z);
}




// Only one struct for wheel info is kept inside IRaycastVehicle. When getWheelInfo
// is called, it populates that struct with information from the corresponding btWheelInfo's data.
// When updateWheelInfo is called, it updates the corresponding btWheelInfo with data
// from wheelInfo. This should be called after modifying a wheel's data to be sure
// the data is actually meant for the desired wheel.
SWheelInfo& IRaycastVehicle::getWheelInfo(u32 wheelID)
{
    btWheelInfo wid = getPointer()->getWheelInfo(wheelID);

    // Convert btWheelInfo to SWheelInfo
    btWheelInfoToSWheelInfo(wid, wheelInfo, wid.m_worldTransform);

    return wheelInfo;
}


IRaycastVehicle::~IRaycastVehicle()
{
    #ifdef IRRBULLET_DEBUG_MODE
        printf("irrBullet: Deleting raycast vehicle (BODY: %i)\n", rigidBody->getUniqueID());
    #endif
    if(vehicleRaycaster)
        delete vehicleRaycaster;

    if(raycastVehicle)
        delete raycastVehicle;

	if (attributes)
		attributes->drop(); //SETH
	
}


void* IVehicleRaycaster::castRay(const btVector3& from,const btVector3& to, btVehicleRaycasterResult& result)
{
	btCollisionWorld::ClosestRayResultCallback rayCallback(from,to);

	if(useFilter)
	{
        rayCallback.m_collisionFilterMask = collisionFilterMask;
        rayCallback.m_collisionFilterGroup = collisionFilterGroup;
	}

	m_dynamicsWorld->rayTest(from, to, rayCallback);

	if (rayCallback.hasHit())
	{

		btRigidBody* body = btRigidBody::upcast(rayCallback.m_collisionObject);
        if (body && body->hasContactResponse())
		{
			result.m_hitPointInWorld = rayCallback.m_hitPointWorld;
			result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld;
			result.m_hitNormalInWorld.normalize();
			result.m_distFraction = rayCallback.m_closestHitFraction;
			return body;
		}
	}
	return 0;
}
