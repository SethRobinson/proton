// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __COLLISION_OBJECT_H_INCLUDED__
#define __COLLISION_OBJECT_H_INCLUDED__

#include "irrbulletcommon.h"
#include "collisionshape.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "collisionobjectaffector.h"
#include <IAttributes.h>

enum EActivationState
{
    EAS_ACTIVE = 1,
    EAS_SLEEPING = 2,
    EAS_WANTS_DEACTIVATION = 3,
    EAS_DISABLE_DEACTIVATION = 4,
    EAS_DISABLE_SIMULATION = 5
};


enum ECollisionObjectType
{
  ECOT_COLLISION_OBJECT = 1,
  ECOT_RIGID_BODY,
  ECOT_GHOST_OBJECT,
  ECOT_SOFT_BODY,
  ECOT_HF_FLUID
};


enum ECollisionFlag
{
    ECF_STATIC_OBJECT = 1,
    ECF_KINEMATIC_OBJECT = 2,
    ECF_NO_CONTACT_RESPONSE = 4,
    ECF_CUSTOM_MATERIAL_CALLBACK = 8,
    ECF_CHARACTER_OBJECT = 16
};

class ICollisionObject;

struct SCollisionObjectIdentification
{
    SCollisionObjectIdentification()
    {
        name = "IdentificationInvalid";
        object = 0;
    }

    void setCollisionObject(ICollisionObject* const obj) { object = obj; };

    void setName(const irr::core::stringc& newName) { name = newName; };

    const irr::core::stringc& getName() { return name; };

    ICollisionObject* const getCollisionObject() const { return object; };

private:
    ICollisionObject* object;
    irr::core::stringc name;
};

class irrBulletWorld;

/// The base class of all world objects.
class ICollisionObject
{
    public:
        ICollisionObject();

        virtual ~ICollisionObject();

        void updateObject();

        /*!
            Sets the Continuous Collision Detection values of the world object.
            This helps to avoid tunneling (when an object "shoots" through
            another object when it is not supposed to, due to traveling
            too far in one step, sometimes actually passing by a collision
            face before it is stepped again, where it should have collided) at high speeds.
            @param motionThreshold The simulation doesn't do CCD unless the motion (in one step) is more than motionThreshold.
            @param radius Swept sphere radius. (0.0 by default)
        */
        void setCcdValues(irr::f32 motionThreshold, irr::f32 radius);

        /// Sets the new world transform of this object. (rotation and position)
        void setWorldTransform(const irr::core::matrix4& irrmat);

        void setAnisotropicFriction(const irr::core::vector3df& friction);

        /*!
            The constraint solver can discard solving contacts if the distance is above this threshold.

            0 by default. Note that using contacts with positive distance can improve stability.
            It increases, however, the chance of colliding with degerate contacts, such as 'interior' triangle edges
        */
        void setContactProcessingThreshold(irr::f32 cpt);

        void setActivationState(EActivationState newState);

        void forceActivationState(EActivationState newState);

        /// If sleeping, this will "wake" the object up.
        void activate(bool forceActivation=false);

        /// Sets how "bouncy" the object is. Higher values will make it more bouncy.
        void setRestitution(irr::f32 rest);

        /*!
            Sets the friction of the object. Objects with higher friction values tend
            to tumble and roll more, and will not slide as far as it would with
            lower friction values.
        */
        void setFriction(irr::f32 friction);

        void setInterpolationLinearVelocity(const irr::core::vector3df &linvel);

        void setInterpolationAngularVelocity(const irr::core::vector3df &angvel);

        void setIslandTag(irr::u32 tag);

        void setCompanionId(irr::u32 id);

        void setHitFraction(irr::f32 hitFraction);

        void setCollisionFlags(ECollisionFlag flags);

        /// Adds a new collision object animator to the list of animators to affect this object
        void addAffector(ICollisionObjectAffector* const affector);

        /// Removes a collision object animator at the given index
        void removeAffector(ICollisionObjectAffector* const affector);

        void removeAffectors();

        /// @param b If this parameter is true, then the node will be included in the removal process.
         void includeNodeOnRemoval(bool b) { IncludeNodeOnRemoval = b; };

        /// Sets the name of this object to newName
        void setName(const irr::core::stringc& newName) { collID->setName(newName); };

        /// Removes the object from the dynamics world without explicit use of irrBulletWorld's removal functions.
        void remove();

        void setLiquidSimulationEnabled(bool b) { LiquidSimulationEnabled = b; };


        // Operators
        bool operator==(const ICollisionObject& other) const { return (this->uniqueID == other.getUniqueID()); };

        /*bool operator==(const btCollisionObject& other) const
        {
            irr::u32 otherID = static_cast<SCollisionObjectIdentification*>(other.getUserPointer())->ID;
            return (this->uniqueID == otherID);
        };
        bool operator!=(const btCollisionObject& other) const { return !(*this == other); };*/

        bool operator!=(const ICollisionObject& other) const { return !(*this == other); }



        /// @return Underlaying btCollisionObject pointer.
        btCollisionObject* const getPointer() const;

        /// @return The irrBullet world that this object is being simulated in
        irrBulletWorld* const getDynamicsWorld() const { return dynamicsWorld; };

        /*!
        In irrBullet, irr::core::matrix4 takes the place of Bullet's btTransform.
        getWorldTransform() will return a btTransform in Bullet, but returns a matrix4 in irrBullet.
        This matrix includes rotation and position.
        */
        const irr::core::matrix4& getWorldTransform()
        {
            btTransformToIrrlichtMatrix(getPointer()->getWorldTransform(), worldTransform);
            return worldTransform;
        }

        bool mergesSimulationIslands() const;

        irr::core::vector3df getAnisotropicFriction() const;

        bool hasAnisotropicFriction() const;

        irr::f32 getContactProcessingThreshold() const;

        bool isStaticObject() const { return object->isStaticObject(); };
        bool isKinematicObject() const { return object->isKinematicObject(); };
        bool isStaticOrKinematicObject() const { return object->isStaticOrKinematicObject(); };
        bool hasContactResponse() const;

        EActivationState getActivationState() const;

        bool isActive() const;

        /// @return How "bouncy" the object is.
        irr::f32 getRestitution() const;

        irr::f32 getFriction() const;

        irr::core::vector3df getInterpolationLinearVelocity() const;

        irr::core::vector3df getInterpolationAngularVelocity() const;

        irr::u32 getIslandTag() const;

        irr::u32 getCompanionId() const;

        irr::f32 getHitFraction() const;

        ECollisionFlag getCollisionFlags() const;

        irr::u32 getUniqueID() const { return uniqueID; };

        ECollisionObjectType getObjectType() const { return objectType; };

        bool checkCollideWith(ICollisionObject* const collObj) const;

        irr::io::IAttributes *getAttributes() const { return attributes; };

        irr::u32 getNumAffectors() const;

        ICollisionObjectAffector* const getAffector(irr::u32 index);

        //bool hasCollidedWithAttribute(irr::c8 *attributeName) const;

        bool isNodeIncludedInRemoval() const { return IncludeNodeOnRemoval; };

        /// @return The name of this collision object
        const irr::core::stringc& getName() const { return collID->getName(); };

        bool isLiquidSimulationEnabled() const { return LiquidSimulationEnabled; };



    protected:
        irrBulletWorld *dynamicsWorld;
        btCollisionObject *object;
        irr::u32 uniqueID;
        ECollisionObjectType objectType;
        irr::io::IAttributes *attributes;
        irr::core::list<ICollisionObjectAffector*> affectors;
        bool IncludeNodeOnRemoval;
        bool LiquidSimulationEnabled;
        btTransform internalTransform;
        irr::core::matrix4 worldTransform;

        SCollisionObjectIdentification *collID;
};

#endif // __COLLISION_OBJECT_H_INCLUDED__
