// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __BULLET_WORLD_H_INCLUDED__
#define __BULLET_WORLD_H_INCLUDED__


#include "irrbulletcommon.h"
#include "rigidbody.h"
#include "softbody.h"
#include "physicsdebug.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "collisioncallbackinformation.h"
#include "irrbullet_compile_config.h"

class IRaycastVehicle;

namespace irr
{
    namespace gui
    {
        class IGUIStaticText;
    }
}

class btDefaultCollisionConfiguration;
class btDispatcher;
class btBroadphaseInterface;
class btConstraintSolver;
struct btVehicleRaycaster;
class ILiquidBody;



/// The interface that drives the physics simulation and debug drawer.
class irrBulletWorld
{
public:
    /*!
        Constructor.
        @param Device A pointer to Irrlicht's device
        @param useGImpact If true, GImpactMeshShapes can collide with other tri-mesh shapes
            (the GImpact algorithm will be registered).
        @param useDebugDrawer If true, the debug drawer will be enabled.
    */
    irrBulletWorld(irr::IrrlichtDevice* const Device, bool useGImpact, bool useDebugDrawer);

    ~irrBulletWorld();

    /*!
        Steps the simulation once. Should be used with a delta time for framerate-independent simulation.
        If maxSubSteps > 0, it will interpolate motion between fixedTimeStep's.
        @param timeStep The timestep to use.
        @param maxSubSteps Bullet subdivides the timestep. This is how many times it can do this on one step.
        @return 0
    */
    int stepSimulation(irr::f32 timeStep, irr::u32 maxSubSteps=1, irr::f32 fixedTimeStep=irr::f32(1.)/irr::f64(60.));

    /// Internal use only
    void updateLiquidBodies();

    /// Internal use only
    void updateCollisionObjects();

    /*!
        irrBullet provides this feature to easily pause and unpause the simulation.
        @param p Can be used to toggle the pause value.
    */
    void pauseSimulation(bool p) { isPaused = p; };

    /*!
        This can be used to register the GImpact algorithm if it was not registered in the
        world's constructor. It will not register it if it has already been registered.
    */
    void registerGImpactAlgorithm();

    void synchronizeMotionStates();

    void synchronizeSingleMotionState(IRigidBody* const body);

    void setGravity(const irr::core::vector3df &gravity);

    /// Adds an object to the list of objects to be safely deleted (mainly used for object affectors to avoid crashes)
    void addToDeletionQueue(ICollisionObject* const obj);

    ILiquidBody* const addLiquidBody(const irr::core::vector3df& pos, const irr::core::aabbox3df& aabb,
        irr::f32 waveFrequency=40000.0f, irr::f32 density=0.4f);


    /*!
        Adds a new rigid body to the world.
        @param shape The collision shape for the rigid body to use.
        @return The newly created rigid body
    */
    IRigidBody* const addRigidBody(ICollisionShape* const shape);

    /*!
        This is the same as the other addRigidBody, with the exception that it includes collision masking.
        @param shape The collision shape for the rigid body to use.
        @param group The collision group that the body belongs to.
        @param mask The collision group(s) that this body will interact with.
        @return The newly created rigid body
    */
    IRigidBody* const addRigidBody(ICollisionShape* const shape, irr::s32 group, irr::s32 mask);

    /// Adds a soft body to the world
    ISoftBody* const addSoftBody(irr::scene::IMeshSceneNode* const node);

    /*!
        Adds the special constraint known as a raycast vehicle.
        This simulates wheels by using raycasting, rather than an actual rigid body as the wheels.
        @param body This is the rigid body that will be affected by the vehicle.
        @param coordSys This is the coordination system that the vehicle will use.
            0 = sideways, 1 = upward, 2 = forward
            This should not be changed unless you wish to use another system.
            The vector is not meant as the representation itself, it only uses the floats in the index order.
        @return The newly created raycast vehicle.
    */
    IRaycastVehicle* const addRaycastVehicle(IRigidBody* const body, const irr::core::vector3d<irr::s32>& coordSys=irr::core::vector3d<irr::s32>(0,1,2));

    /// Like addRaycastVehicle() only with this you can use a custom vehicle raycaster
    IRaycastVehicle* const addRaycastVehicle(IRigidBody* const body, btVehicleRaycaster* const raycaster, const irr::core::vector3d<irr::s32>& coordSys=irr::core::vector3d<irr::s32>(0,1,2));

    void removeLiquidBody(ILiquidBody* const liquidBody);

    /// Removes a rigid body from the world
    void removeCollisionObject(ICollisionObject* const obj, bool deleteObject=true);

    /// Removes a raycast vehicle from the world
    void removeRaycastVehicle(IRaycastVehicle* const vehicle);

    /// Clears all forces acting on each object.
    void clearForces();

    /// Changes the debugger's draw modes.
    void setDebugMode(irr::u32 mode);

    /*!
        Draws the debugging data once based on the flags set.
        Needs to be called every frame for data to be visible.
        @param setDriverMaterial Sets whether or not the debugger will automatically change the driver material so that
            the debugging data will be visible. Defaults to false. Leave false if you do this manually.
    */
    void debugDrawWorld(bool setDriverMaterial=false);

    /*!
        Draws the text in the GUI environment to show internal physics properties
        and debugging data.
        @param b Can be used to toggle whether or not the data is visible.
        @param col If present, this will be the override color of the property text. Defaults to white.
    */
	irr::core::stringc debugDrawProperties(bool b, const irr::video::SColor& col = irr::video::SColor(255,255,255,255));

    /// @return Whether or not the simulation is paused.
    bool simulationPaused() const { return isPaused; };

    /// @return The total number of collision objects in the world (both soft and rigid)
    irr::u32 getNumCollisionObjects() const { return CollisionObjectCount; };



    /// @return The collision object at the given index
    ICollisionObject* const getCollisionObject(irr::u32 index) const;

    ILiquidBody* const getLiquidBody(irr::u32 index) const;

    ILiquidBody* const getLiquidBodyByID(irr::u32 ID) const;

    /// @return The collision object with the given ID (useful for animators or collision callbacks)
    ICollisionObject* const getCollisionObjectByID(irr::u32 ID) const;

    /// @return The first object in the list with the given name (to avoid problems, be sure to use unique names!)
    ICollisionObject* const getCollisionObjectByName(const irr::core::stringc& name) const;

    /*!
        Useful to use internal Bullet world functions if desired.
        @return The underlaying Bullet physics simulation world.
    */
    btSoftRigidDynamicsWorld* const getPointer() const { return world; };

    /// @return Whether or not the GImpact algorithm has been registered.
    bool isGImpactEnabled() const;

    /// @return Number of collision manifolds
    irr::u32 getNumManifolds() const { return dispatcher->getNumManifolds(); };

    /// @return The collision callback information at the given index
    ICollisionCallbackInformation* const getCollisionCallback(irr::u32 index) const;

    /// @return A reference to the soft body world info (gravity, water density, air density, dispatcher, etc.)
    btSoftBodyWorldInfo& getSoftBodyWorldInfo() { return softBodyWorldInfo; };

    irr::IrrlichtDevice* const getIrrlichtDevice() const { return device; };

    const irr::video::SMaterial& getDebugMaterial() const { return debugMat; };


private:
    btSoftRigidDynamicsWorld *world;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btDispatcher* dispatcher;
    btBroadphaseInterface* pairCache;
    btConstraintSolver*	constraintSolver;
    btSoftBodyWorldInfo softBodyWorldInfo;

    IPhysicsDebugDraw *debug;
    irr::video::SMaterial debugMat;


    irr::IrrlichtDevice *device;
    irr::gui::IGUIStaticText *propertyText;

    irr::core::list<ICollisionObject*> collisionObjects;
    irr::core::list<IRaycastVehicle*> raycastVehicles;
    irr::core::list<ICollisionObject*> deletionList;
    irr::core::list<ILiquidBody*> liquidBodies;
    irr::core::list<ICollisionCallbackInformation*> collisionCallbacks;

    bool gimpactEnabled;
    bool isPaused;

    irr::u32 LiquidBodyCount;
    irr::u32 CollisionObjectCount;
};

#endif // __BULLET_WORLD_H_INCLUDED__
