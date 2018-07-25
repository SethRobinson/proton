// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __RAYCAST_VEHICLE_H_INCLUDED__
#define __RAYCAST_VEHICLE_H_INCLUDED__

#include "irrbulletcommon.h"
#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

class irrBulletWorld;
class IRigidBody;
class ICollisionObject;
struct btVehicleRaycaster;


namespace irr
{
    namespace io
    {
        class IAttributes;
    }
}


struct SWheelInfoConstructionInfo
{
    irr::core::vector3df chassisConnectionPointCS;
    irr::core::vector3df wheelDirectionCS;
    irr::core::vector3df wheelAxleCS;
    irr::f32 suspensionRestLength;
    irr::f32 wheelRadius;
    bool isFrontWheel;
};

struct SRaycastInfo
{
    irr::core::vector3df contactNormalWS;
    irr::core::vector3df contactPointWS;
    irr::f32 suspensionLength;
    irr::core::vector3df hardPointWS;
    irr::core::vector3df wheelDirectionWS;
    irr::core::vector3df wheelAxleWS;
    bool isInContact;
    void* groundObject;
};

/// The SWheelInfo struct contains all of the needed information about each wheel of raycast vehicle.
/*!
    @see getWheelInfo()
    @see updateWheelInfo()
*/
struct SWheelInfo
{
    irr::core::matrix4 worldTransform;
    irr::core::vector3df chassisConnectionPointCS;
    irr::core::vector3df wheelDirectionCS;
    irr::core::vector3df wheelAxleCS;
    irr::f32 suspensionRestLength;
    irr::f32 maxSuspensionTravelCm;
    irr::f32 wheelRadius;
    irr::f32 suspensionStiffness;
    irr::f32 wheelDampingCompression;
    irr::f32 wheelDampingRelaxation;
    irr::f32 frictionSlip;
    irr::f32 steering;
    irr::f32 wheelRotation;
    irr::f32 deltaRotation;
    irr::f32 rollInfluence;
    irr::f32 engineForce;
    irr::f32 brake;
    bool isFrontWheel;
    irr::f32 clippedInvContactDotSuspension;
    irr::f32 suspensionRelativeVelocity;
    irr::f32 wheelSuspensionForce;
    irr::f32 skidInfo;
    SRaycastInfo raycastInfo;
};

/// This vehicle raycaster allows for collision masks and groups. The original Bullet raycaster does not.
class IVehicleRaycaster : public btVehicleRaycaster
{
    public:
        IVehicleRaycaster(btDynamicsWorld* world) : m_dynamicsWorld(world)
        {
            useFilter = false;
            collisionFilterMask = 0;
            collisionFilterGroup = 0;
        }

        virtual void* castRay(const btVector3& from,const btVector3& to, btVehicleRaycasterResult& result);

        /// This has to be set to true before collision filtering will work.
        void setUseFilter(bool b) { useFilter = b; };

        /// Sets the collision filter mask. This can use the same filter mask as the chassis (IRigidBody) or another kind.
        void setCollisionFilterMask(int newMask) { collisionFilterMask = newMask; };

        /// Sets the collision filter group. This can use the same filter group as the chassis (IRigidBody) or another kind.
        void setCollisionFilterGroup(int newGroup) { collisionFilterGroup = newGroup; };


        bool getUseFilter() { return useFilter; };
        int getCollisionFilterMask() { return collisionFilterMask; };
        int getCollisionFilterGroup() { return collisionFilterGroup; };


    private:
        btDynamicsWorld* m_dynamicsWorld;
        bool useFilter;
        int collisionFilterMask;
        int collisionFilterGroup;
};

/// The raycast vehicle is a special constraint for vehicle simulation using raycasting.
/*!
    The IRaycastVehicle is a special constraint that allows vehicle physics to be simulated
    using raycasting for each wheel rather than a physical representation such as
    a constrained cylinder shape. This is a good type to use for most vehicle physics,
    and is very flexible and easy to set up.
*/
class IRaycastVehicle
{
    public:
        IRaycastVehicle(IRigidBody* const body, irrBulletWorld* const world, const irr::core::vector3d<irr::s32>& coordSys=irr::core::vector3d<irr::s32>(0,1,2));
        IRaycastVehicle(IRigidBody* const body, irrBulletWorld* const world, btVehicleRaycaster* const raycaster,
            const irr::core::vector3d<irr::s32>& coordSys=irr::core::vector3d<irr::s32>(0,1,2));

        ~IRaycastVehicle();


        /// Adds a new wheel to the vehicle using the construction info given.
        SWheelInfo& addWheel(const SWheelInfoConstructionInfo& info);

        /*!
            Updates the given wheel's index with the latest change of wheelInfo.
            This should be called right after changing the vehicle's internal wheelInfo
            attribute that was returned using getWheelInfo().
            For example:
                *change wheel info directly*
                vehicle->updateWheelInfo(1);
            This would be the correct order.
                *change wheel info directly*
                vehicle->updateWheelInfo(1);
                vehicle->updateWheelInfo(2);
            The wheels at both index 1 and 2 would be using the same wheel info at this point.
            No individual infos are stored for each wheel. Only one struct is changed when
            getWheelInfo() is called, using the information from the wheel index supplied as a parameter.
        */
        void updateWheelInfo(irr::u32 wheelID);

        void resetSuspension();

        /// Sets the rotation of the wheel at index wheelID along the local Y axis.
        void setSteeringValue(irr::f32 steering, irr::u32 wheelID);

        /// Applies a torque to the wheel on its local X axis.
        void applyEngineForce(irr::f32 force, irr::u32 wheelID);

        /*!
            This should be called before calling getWheelInfo(), since it would get the latest
            interpolated wheel transform unless specified otherwise by interpolatedTransform.
        */
        void updateWheelTransform(irr::u32 wheelID, bool interpolatedTransform=true);

        // Undefined reference with this one. Disabled for now.
        /*void setRaycastWheelInfo(irr::u32 wheelID, bool isInContact,
            const irr::core::vector3df &hitPoint, const irr::core::vector3df &hitNormal, irr::f64 depth);*/

        void setBrake(irr::f32 brake, irr::u32 wheelID);

        void setPitchControl(irr::f32 pitch);

        void setCoordinateSystem(const irr::core::vector3d<irr::s32>& coordSys);


        /// Gets a pointer to the underlying btRaycastVehicle.
        btRaycastVehicle* const getPointer() const { return raycastVehicle; };

        /// Gets the rigid body that this vehicle is applied to.
        IRigidBody* const getRigidBody() const { return rigidBody; };

        irr::s32 getRightAxis() const { return axes.X; };

        irr::s32 getUpAxis() const { return axes.Y; };

        irr::s32 getForwardAxis() const { return axes.Z; };

        const irr::core::vector3df getForwardVector() const { return bulletToIrrlichtVector(getPointer()->getForwardVector()); };

        irr::f32 getCurrentSpeedKmHour() const { return irr::f32(getPointer()->getCurrentSpeedKmHour()); };

        irr::u32 getNumWheels() const { return irr::u32(raycastVehicle->getNumWheels()); };

        SWheelInfo& getWheelInfo(irr::u32 wheelID);

        irr::core::matrix4& getChassisWorldTransform()
        {
            btTransformToIrrlichtMatrix(getPointer()->getChassisWorldTransform(), InternalTransform);
            return InternalTransform;
        };

        irr::f32 getSteeringValue(irr::u32 wheelID) const { return irr::f32(getPointer()->getSteeringValue(wheelID)); };

        irr::core::matrix4& getWheelTransformWS(irr::u32 wheelID)
        {
            btTransformToIrrlichtMatrix(getPointer()->getWheelTransformWS(wheelID), InternalTransform);
            return InternalTransform;
        };

        irr::io::IAttributes* const getAttributes() const { return attributes; };

        /// Only use this if you don't use a custom vehicle raycaster. If you do, use getVehicleRaycasterCustom().
        IVehicleRaycaster* const getVehicleRaycaster() const { return static_cast<IVehicleRaycaster*>(vehicleRaycaster); };

        /// Use this if you use your own custom vehicle raycaster.
        btVehicleRaycaster* const getVehicleRaycasterCustom() const { return vehicleRaycaster; };



    protected:
        btRaycastVehicle::btVehicleTuning vehicleTuning;
        btVehicleRaycaster* vehicleRaycaster;
        btRaycastVehicle* raycastVehicle;
        IRigidBody *rigidBody;
        irr::core::vector3d<irr::s32> axes;
        irr::io::IAttributes *attributes;
        irr::core::matrix4 InternalTransform;

        bool usesOriginalRaycaster;

        SWheelInfo wheelInfo;
};


#endif // __RAYCAST_VEHICLE_H_INCLUDED__
