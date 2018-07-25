// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __RIGID_BODY_H_INCLUDED__
#define __RIGID_BODY_H_INCLUDED__

#include "irrbulletcommon.h"
#include "collisionobject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "raycastvehicle.h"


class IMotionState;


struct SRigidBodyConstructionInfo
{
    ICollisionShape *collisionShape;
    irr::f32 angularDamping;
    irr::f32 linearDamping;
};

/*!
    This struct contains information about a point in a buoyancy affector
    that helps the object using the affector to stay afloat.
*/
struct SBuoyancyPoint
{
    SBuoyancyPoint(const irr::core::vector3df& point, irr::f32 pointBuoyancy)
    {
        localPoint = point;
        buoyancy = pointBuoyancy;
    }

    irr::core::vector3df localPoint;
    irr::f32 buoyancy;
};


/*!
    The transform space tells the rigid body how to apply the force/torque it receives.
    If the transform space parameter of the function is ERBTS_LOCAL, it will
    apply the force locally with the vector supplied, and vice-versa.

    For example, if you called rigidBody->applyForce(vector3df(0.0,0.0,100.0), ERBTS_LOCAL),
    it would apply a force on the rigid body of 100 in the direction the rigid body is pointing.
    If it was set to ERBTS_WORLD, it would apply a force of 100 in the direction of
    the fixed Z axis of the world. All of the functions default this parameter to ERBTS_WORLD.
*/
enum ERBTransformSpace
{
    ERBTS_LOCAL, // RigidBody forces will be applied in local space
    ERBTS_WORLD  // RigidBody forces will be applied in world space
};


/// The rigid body is the main type for all "hard" simulation objects (The opposite of a soft body).
class IRigidBody : public ICollisionObject
{
public:
    /// @param collShape The collision shape for this body to use.
    IRigidBody(irrBulletWorld* const world, ICollisionShape* const collShape);

    IRigidBody(irrBulletWorld* const world, const SRigidBodyConstructionInfo& info);
    virtual ~IRigidBody();

    void translate(const irr::core::vector3df& v);

    void updateDeactivation(irr::f32 timeStep);

    /// @return If this object is about to be deactivated due to a certain time of inactivity.
    bool wantsSleeping();

    void getAabb(irr::core::vector3df& aabbMin, irr::core::vector3df& aabbMax);

    /*!
        Linear velocity is motion in any direction.
        The velocity set here will remain the same throughout the object's lifetime
        unless changed by other means.

        @param transformSpace If this parameter is set to ERBTS_LOCAL, it will only apply
            the velocity in the local space at the time the function was called.
            The velocity will not be applied in current local directions of the rigid body.
            If you wish to achieve this, please see  applyCentralForce(), applyForce(),
            applyCentralImpulse(), applyImpulse(), and internalApplyImpulse().

            However, calling setLinearVelocity() in local space would be great for something
            such as an unguided rocket that only needs one direction at the time of its creation.
    */
    void setLinearVelocity(const irr::core::vector3df& linVel, ERBTransformSpace transformSpace=ERBTS_WORLD);

    /*!
        Angular velocity is the speed at which an object rotates.
        The velocity set here will remain the same throughout the object's lifetime
        unless changed by other means.

        @param transformSpace If this parameter is set to ERBTS_LOCAL, it will only apply
            the velocity in the local space at the time the function was called.
            The velocity will not be applied in current local directions of the rigid body.
            If you wish to achieve this, please see applyTorque(), applyTorqueImpulse() and internalApplyImpulse().
    */
    void setAngularVelocity(const irr::core::vector3df& angVel, ERBTransformSpace transformSpace=ERBTS_WORLD);

    void saveKinematicState(irr::f32 step);

    /// Applies a force from the center of the object.
    void applyCentralForce(const irr::core::vector3df& force, ERBTransformSpace transformSpace=ERBTS_WORLD);

    /*!
        Applies a force from a relative position of the object. This can be useful for things
        such as the thrust from several aircraft engines, rockets, etc.
    */
    void applyForce(const irr::core::vector3df& force, const irr::core::vector3df& relPos, ERBTransformSpace transformSpace=ERBTS_WORLD);

    /// This can be thought of as one quick application of force to induce linear motion.
    void applyCentralImpulse(const irr::core::vector3df& impulse, ERBTransformSpace transformSpace=ERBTS_WORLD);

    /*!
        This is the same as applyCentralImpulse, with the exception that it applies the force
        from a relative position of the object.
    */
    void applyImpulse(const irr::core::vector3df& impulse, const irr::core::vector3df &relPos, ERBTransformSpace transformSpace=ERBTS_WORLD);

    void applyTorque(const irr::core::vector3df& torque, ERBTransformSpace transformSpace=ERBTS_WORLD);

    /// This can be thought of as one quick application of force to induce angular motion (torque).
    void applyTorqueImpulse(const irr::core::vector3df& torque, ERBTransformSpace transformSpace=ERBTS_WORLD);

    /*!
        This simply combines applyImpulse and applyTorqueImpulse into one function.
        Impulses are useful for things like projectiles or forces that may or may not
        need to be continually applied.
        @param linTransformSpace The space to apply the linear impulse.
        @param angTransformSpace The space to apply the angular impulse.
    */
    void internalApplyImpulse(const irr::core::vector3df &linearComponent,
        const irr::core::vector3df &angularComponent, irr::f32 impulseMagnitude,
        ERBTransformSpace linTransformSpace=ERBTS_WORLD, ERBTransformSpace angTransformSpace=ERBTS_WORLD);

    void clearForces();

    void updateInertiaTensor();

    void setDamping(irr::f32 lin_damping, irr::f32 ang_damping);

    void applyGravity();

    void setGravity(const irr::core::vector3df& grav);

    void applyDamping(irr::f32 timeStep);

    void setMassProps(irr::f32 mass, const irr::core::vector3df& inertia);

    /*! See setAngularFactor(). This is like setAngularFactor, except for linear forces. If you don't want your
        object to move, then you would set the linear factor to zero (0.0). You could also set the mass of the object
        to zero, and it would have the same effect. With this you can also slow down or speed up an object's linear movement.
    */
    void setLinearFactor(const irr::core::vector3df &linearFactor);

    void setInvInertiaDiagLocal(const irr::core::vector3df &diagInvInertia);

    /// Sets the minimum velocity thresholds before the object enters a sleep state.
    void setSleepingThresholds(irr::f32 linear, irr::f32 angular);

    /// Like setAngularFactor(irr::f32 angFac) except that you can set the factor for each individual axis.
    void setAngularFactor(const irr::core::vector3df &angFac);

    /*! Sets the factor by which to multiply all angular forces before they are applied.
        If you do not want your object to be able to rotate based on interaction with the scene
        (for example, tumbling when it collides with something), then you can set the angular
        factor to zero (0.0) so that it will no longer rotate based on forces. Useful for character
        controllers and a few other uses. (For instance, tracer bullets or bombs where you
        align the object based on the velocity of the rigid body, and you don't want angular
        velocity to mess up the effect.)

        With this you can also slow down or speed up an object's angular movement.
    */
    void setAngularFactor(irr::f32 angFac);

    /*! Sets a reference to an IRaycastVehicle. Useful if your vehicle has a delete animator or you don't want to
        have to keep track of raycast vehicles.
        @note This vehicle will be removed from the world when the rigid body is removed.
    */
    void setVehicleReference(IRaycastVehicle* const vehicle) { vehicleReference = vehicle; };

    /// Aligns the rigidbody to point to the given target position. Useful for things like guided weapons and artificial intelligence.
    void faceTarget(const irr::core::vector3df& targetPosition);

    /// Sets the collision shape for the object to use.
    void setCollisionShape(ICollisionShape* const shape);

    void setBuoyancyPoints(const irr::core::array<SBuoyancyPoint>& points) { BuoyancyPoints = points; BuoyancyPointCount = points.size(); };

    void updateLiquidBox();

    void setDebugLiquidBox(bool b) { DebugLiquidBox = b; };

    const irr::core::array<SBuoyancyPoint>& getBuoyancyPoints() const { return BuoyancyPoints; };

    bool compare(btRigidBody* const other) const
    {
        irr::u32 otherID = static_cast<SCollisionObjectIdentification*>(other->getUserPointer())->getCollisionObject()->getUniqueID();
        return (this->uniqueID == otherID);
    };

    bool compare(const btRigidBody& other) const
    {
        irr::u32 otherID = static_cast<SCollisionObjectIdentification*>(other.getUserPointer())->getCollisionObject()->getUniqueID();
        return (this->uniqueID == otherID);
    };



    /// @return A pointer to the underlying btRigidBody.
    inline btRigidBody *getPointer() const { return static_cast<btRigidBody*>(object); };

    const irr::core::vector3df getLinearVelocity()const ;

    const irr::core::vector3df getAngularVelocity() const;

    const irr::core::vector3df getVelocityInLocalPoint(const irr::core::vector3df &relPos) const;

    irr::f32 computeImpulseDenominator(const irr::core::vector3df &pos, const irr::core::vector3df &normal) const;

    irr::f32 computeAngularImpulseDenominator(const irr::core::vector3df &axis) const;

    const irr::core::vector3df getGravity() const;

    irr::f32 getLinearDamping() const;

    irr::f32 getAngularDamping() const;

    irr::f32 getLinearSleepingThreshold() const;

    irr::f32 getAngularSleepingThreshold() const;

    const irr::core::vector3df getLinearFactor() const;

    irr::f32 getInvMass() const;

    void integrateVelocities(irr::f32 step);

    const irr::core::vector3df getTotalForce() const;

    const irr::core::vector3df getTotalTorque() const;

    const irr::core::vector3df getInvInertiaDiagLocal() const;

    const irr::core::vector3df getAngularFactor() const;

    /// @return Whether or not the rigid body is in the dynamics world being simulated.
    bool isInWorld() const;

    bool checkCollideWithOverride(ICollisionObject* const co);

    /*! @return The number of constraints that are being referenced by this object.
        Constraints include raycast vehicles, springs, etc.
    */
    irr::u32 getNumConstraintRefs() const;

    IMotionState* const getMotionState() const { return motionState; };

    /// Only use this function if you actually set the vehicle with setVehicleReference(); returns 0 if no vehicle was set
    IRaycastVehicle* const getVehicleReference() const { if(vehicleReference) return vehicleReference; else return 0; };

    /// @return Current collision shape in use by the object.
    ICollisionShape* const getCollisionShape() const { return shape; }

    const irr::core::aabbox3d<irr::f32>& getLiquidBox() const { return LiquidBox; };

    irr::u32 getNumBuoyancyPoints() const { return BuoyancyPointCount; };

    SBuoyancyPoint& getBuoyancyPointByIndex(irr::u32 index) { return BuoyancyPoints[index]; };


protected:
    ICollisionShape *shape;
    irr::core::matrix4 worldTransform;
    IMotionState *motionState;
    IRaycastVehicle* vehicleReference;
    irr::core::array<SBuoyancyPoint> BuoyancyPoints;
    irr::u32 BuoyancyPointCount;
    irr::core::aabbox3d<irr::f32> LiquidBox;
    bool DebugLiquidBox;
};




#endif // __RIGID_BODY_H_INCLUDED__
