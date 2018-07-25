// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include <IrrlichtDevice.h>
#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>
#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/btBulletCollisionCommon.h"
#include "Bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include <Bullet/BulletSoftBody/btSoftSoftCollisionAlgorithm.h>
#include "Bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "Bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "liquidbody.h"

#include "bulletworld.h"
#include "raycastvehicle.h"
#include "irrbullet_compile_config.h"


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;


irrBulletWorld::irrBulletWorld(irr::IrrlichtDevice* const Device, bool useGImpact, bool useDebugDrawer) : device(Device)
{
    collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
	//collisionConfiguration->setConvexConvexMultipointIterations();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	pairCache = new btDbvtBroadphase();
	constraintSolver = new btSequentialImpulseConstraintSolver();

    world = new btSoftRigidDynamicsWorld(dispatcher, pairCache,
        constraintSolver, collisionConfiguration);

    // Initialize the softbody world info
    softBodyWorldInfo.m_broadphase = pairCache;
    softBodyWorldInfo.m_dispatcher = dispatcher;

    softBodyWorldInfo.m_sparsesdf.Initialize();
    softBodyWorldInfo.m_gravity.setValue(0,-10.0,0);
    softBodyWorldInfo.air_density = (btScalar)1.2;
    softBodyWorldInfo.water_density = 0;
    softBodyWorldInfo.water_offset = 0;
    softBodyWorldInfo.water_normal = btVector3(0,0,0);

    LiquidBodyCount = 0;
    CollisionObjectCount = 0;


    isPaused = false;

    if(useGImpact == true)
    {
        gimpactEnabled = true;
        btGImpactCollisionAlgorithm::registerAlgorithm((btCollisionDispatcher*)dispatcher);
    }

    else
    {
        gimpactEnabled = false;
    }


    debug = 0;

    if(useDebugDrawer)
    {
        debug = new IPhysicsDebugDraw(device);
        world->setDebugDrawer(debug);

        debugMat.Lighting = false;
    }

    // For displaying debugging properties
	if (device->getGUIEnvironment())
	{
    propertyText = device->getGUIEnvironment()->addStaticText(L"",
            rect<s32>(10,10,120,240), false);
	}


    printf("irrBullet %i.%i.%i\n", IRRBULLET_VER_MAJOR, IRRBULLET_VER_MINOR, IRRBULLET_VER_MICRO);
}


int irrBulletWorld::stepSimulation(f32 timeStep, u32 maxSubSteps, f32 fixedTimeStep)
{
    if(isPaused == false)
    {
        getPointer()->stepSimulation(timeStep, maxSubSteps, fixedTimeStep);
        updateCollisionObjects();
        updateLiquidBodies();
    }

    return 0;
}


void irrBulletWorld::updateLiquidBodies()
{
    core::list<ILiquidBody*>::Iterator it = liquidBodies.begin();

    for(; it != liquidBodies.end(); it++)
    {
        (*it)->updateLiquidBody();
    }
}


void irrBulletWorld::updateCollisionObjects()
{
    core::list<ICollisionObject*>::Iterator cbit = collisionObjects.begin();

    for(; cbit != collisionObjects.end(); cbit++)
    {
        ICollisionObject* obj = (*cbit);

        if(obj->getObjectType() == ECOT_SOFT_BODY)
        {
            static_cast<ISoftBody*>(obj)->updateSoftBody();
        }

        if(obj)
        {
            for(u32 j=0; j < obj->getNumAffectors(); j++)
            {
                ICollisionObjectAffector* affector = obj->getAffector(j);
                if(affector->hasFinished() == false)
                {
                    affector->affectObject(obj, device->getTimer()->getTime());
                }
            }
        }
    }

    core::list<ICollisionObject*>::Iterator dlit = deletionList.begin();

    for(; dlit != deletionList.end(); )
    {
        this->removeCollisionObject((*dlit));
        dlit = deletionList.erase(dlit);
    }
}


void irrBulletWorld::registerGImpactAlgorithm()
{
    if(gimpactEnabled == false)
    {
        gimpactEnabled = true;
        btGImpactCollisionAlgorithm::registerAlgorithm((btCollisionDispatcher*)getPointer()->getDispatcher());
    }
}


IRigidBody* const irrBulletWorld::addRigidBody(ICollisionShape* shape)
{
    IRigidBody* b = new IRigidBody(this, shape);
    collisionObjects.push_back(b);
    getPointer()->addRigidBody(b->getPointer());

    CollisionObjectCount++;

    return b;
}


IRigidBody* const irrBulletWorld::addRigidBody(ICollisionShape *shape, s32 group, s32 mask)
{
    IRigidBody* b = new IRigidBody(this, shape);
    collisionObjects.push_back(b);
    getPointer()->addRigidBody(b->getPointer(), group, mask);

    CollisionObjectCount++;

    return b;
}


ISoftBody* const irrBulletWorld::addSoftBody(IMeshSceneNode* const node)
{
    ISoftBody* b = new ISoftBody(this, node);
    collisionObjects.push_back(b);
    getPointer()->addSoftBody(b->getPointer());

    CollisionObjectCount++;

    return b;
}


void irrBulletWorld::addToDeletionQueue(ICollisionObject* obj)
{
    if(!obj)
        return;
    deletionList.push_back(obj);
}

ILiquidBody* const irrBulletWorld::addLiquidBody(const irr::core::vector3df& pos, const irr::core::aabbox3df& aabb,
    irr::f32 waveFrequency, irr::f32 density)
{
    ILiquidBody *liquidBody = new ILiquidBody(this, pos, aabb, waveFrequency, density);

    liquidBodies.push_back(liquidBody);

    LiquidBodyCount++;

    return liquidBody;
}


IRaycastVehicle* const irrBulletWorld::addRaycastVehicle(IRigidBody* const body, const vector3d<s32>& coordSys)
{
    IRaycastVehicle *vehicle = new IRaycastVehicle(body, this, coordSys);

    raycastVehicles.push_back(vehicle);

    getPointer()->addVehicle(vehicle->getPointer());

    return vehicle;
}

IRaycastVehicle* const irrBulletWorld::addRaycastVehicle(IRigidBody* const body, btVehicleRaycaster* const raycaster, const vector3d<s32>& coordSys)
{
    IRaycastVehicle *vehicle = new IRaycastVehicle(body, this, raycaster, coordSys);

    raycastVehicles.push_back(vehicle);

    getPointer()->addVehicle(vehicle->getPointer());

    return vehicle;
}

void irrBulletWorld::removeCollisionObject(ICollisionObject* const obj, bool deleteObject)
{
    if(obj)
    {
        core::list<ICollisionObject*>::Iterator cbit = collisionObjects.begin();

        for(; cbit != collisionObjects.end(); )
        {
            if((*cbit) == obj)
            {
                if((*cbit)->getObjectType() == ECOT_RIGID_BODY)
                {
                    #ifdef IRRBULLET_DEBUG_MODE
                        printf("irrBullet: Removing rigid body (%i)\n", obj->getUniqueID());
                    #endif
                    if(static_cast<IRigidBody*>(obj)->getVehicleReference() != 0)
                        removeRaycastVehicle(static_cast<IRigidBody*>(obj)->getVehicleReference());
                    getPointer()->removeRigidBody(static_cast<IRigidBody*>(obj)->getPointer());
                }

                else
                if((*cbit)->getObjectType() == ECOT_SOFT_BODY)
                {
                    #ifdef IRRBULLET_DEBUG_MODE
                        printf("irrBullet: Removing soft body (%i)\n", obj->getUniqueID());
                    #endif
                    getPointer()->removeSoftBody(static_cast<ISoftBody*>(obj)->getPointer());
                }

                if(deleteObject == true)
                {
                    if((*cbit)->getObjectType() == ECOT_RIGID_BODY)
                    {
                        delete static_cast<IRigidBody*>(*cbit);
                        (*cbit) = 0;
                    }

                    else
                    {
                        delete static_cast<ISoftBody*>(*cbit);
                        (*cbit) = 0;
                    }
                }
                cbit = collisionObjects.erase(cbit);

                CollisionObjectCount--;

            }

            else
                cbit++;
        }
    }
}

void irrBulletWorld::removeRaycastVehicle(IRaycastVehicle* const vehicle)
{
    if(vehicle)
    {
        core::list<IRaycastVehicle*>::Iterator rvit = raycastVehicles.begin();

        for(; rvit != raycastVehicles.end(); )
        {
            if((*rvit) == vehicle)
            {
                #ifdef IRRBULLET_DEBUG_MODE
                    printf("irrBullet: Removing raycast vehicle (BODY: %i)\n", vehicle->getRigidBody()->getUniqueID());
                #endif
                getPointer()->removeVehicle(vehicle->getPointer());

                delete (*rvit);
                (*rvit) = 0;
                rvit = raycastVehicles.erase(rvit);

            }

            else
                rvit++;
        }
    }
}

void irrBulletWorld::removeLiquidBody(ILiquidBody* const liquidBody)
{
    if(liquidBody)
    {
        core::list<ILiquidBody*>::Iterator rvit = liquidBodies.begin();

        for(; rvit != liquidBodies.end(); )
        {
            if((*rvit) == liquidBody)
            {
                #ifdef IRRBULLET_DEBUG_MODE
                    printf("irrBullet: Removing raycast vehicle (BODY: %i)\n", liquidBody->getUniqueID());
                #endif

                delete (*rvit);
                (*rvit) = 0;
                rvit = liquidBodies.erase(rvit);

            }

            else
                rvit++;
        }
    }
}

void irrBulletWorld::clearForces()
{
    getPointer()->clearForces();
}


void irrBulletWorld::setDebugMode(u32 mode)
{
    if(debug != 0)
        debug->setDebugMode(mode);
}


void irrBulletWorld::debugDrawWorld(bool setDriverMaterial)
{
    if(debug != 0)
    {
        if(setDriverMaterial)
        {
            device->getVideoDriver()->setMaterial(debugMat);
            device->getVideoDriver()->setTransform(irr::video::ETS_WORLD, irr::core::matrix4());
        }
        getPointer()->debugDrawWorld();
    }
}

irr::core::stringc irrBulletWorld::debugDrawProperties(bool b, const SColor& col)
{

	
  //  if(b == true)
        const u32 numObjects = getNumCollisionObjects();
        u32 active = 0;
        u32 sleeping = 0;

        for(u32 i=0; i < numObjects; i++)
        {
            if(getCollisionObject(i)->getActivationState() == EAS_ACTIVE)
                active++;

            else
            if(getCollisionObject(i)->getActivationState() == EAS_SLEEPING)
                sleeping++;
        }

        // Shows percentage of active objects.
        const f32 diff = active - sleeping;
        const s32 perc = (diff / active) * 100;


        stringc str = "MPF: "; // Milliseconds Per Frame
        str += (1000 / (device->getVideoDriver()->getFPS()));
        str += "\nObjects: ";
        str += numObjects;
        str += "\nActive: ";
        str += active;
        str += "\nSleeping: ";
        str += sleeping;
        str += "\nPerc. Active: ";
        str += (perc > 0) ? (stringc(perc) + "%") : "0%";

/*       
		propertyText->setText(str.c_str());

        if(propertyText->getOverrideColor() != col)
            propertyText->setOverrideColor(col);
			*/
   
/*
    else
    if(b == false)
    {
        if(propertyText->getText() != L"")
        {
            propertyText->setText(L"");
        }
    }
	*/

	return str;
}


void irrBulletWorld::synchronizeMotionStates()
{
    getPointer()->synchronizeMotionStates();
}

void irrBulletWorld::synchronizeSingleMotionState(IRigidBody *body)
{
    getPointer()->synchronizeSingleMotionState(body->getPointer());
}

void irrBulletWorld::setGravity(const vector3df &gravity)
{
    getPointer()->setGravity(btVector3(gravity.X, gravity.Y, gravity.Z));
}


/*u32 irrBulletWorld::getNumCollisionObjects()
{
    u32 num = 0;

    core::list<ICollisionObject*>::Iterator it = collisionObjects.begin();

    for(; it != collisionObjects.end(); it++)
    {
        num++;
    }

    return num;
}*/


/*u32 irrBulletWorld::getNumCollisionObjects(EActivationState state)
{
    u32 num = 0;

    core::list<ICollisionObjects*>::Iterator it = collisionObjects.begin();

    for(; it != collisionObjects.end(); it++)
    {
        const bool isActive = (*it)->getPointer()->isActive();
        switch(isActive)
        {
            case true:
            {
                if(state == EAS_ACTIVE)
                {
                    num++;
                }
                break;
            }

            case false:
            {
                if(state == EAS_SLEEPING)
                {
                    num++;
                }
                break;
            }

            default:
                break;
        }
    }

    return num;
}*/


ICollisionObject* const irrBulletWorld::getCollisionObject(irr::u32 index) const
{
    core::list<ICollisionObject*>::ConstIterator it = collisionObjects.begin();

    it += index;
    ICollisionObject *obj = (*it);
    if(obj)
        return obj;
    return 0;
}

ILiquidBody* const irrBulletWorld::getLiquidBody(irr::u32 index) const
{
    core::list<ILiquidBody*>::ConstIterator it = liquidBodies.begin();

    it += index;
    ILiquidBody *body = (*it);
    if(body)
        return body;
    return 0;
}

ILiquidBody* const irrBulletWorld::getLiquidBodyByID(irr::u32 ID) const
{
    core::list<ILiquidBody*>::ConstIterator it = liquidBodies.begin();

    for(; it != liquidBodies.end(); it++)
    {
        ILiquidBody* obj = (*it);
        if(obj->getUniqueID() == ID)
            return obj;

    }
    return 0;
}

ICollisionObject* const irrBulletWorld::getCollisionObjectByID(irr::u32 ID) const
{
    core::list<ICollisionObject*>::ConstIterator it = collisionObjects.begin();

    for(; it != collisionObjects.end(); it++)
    {
        ICollisionObject* obj = (*it);
        if(obj->getUniqueID() == ID)
            return obj;

    }
    return 0;
}

ICollisionObject* const irrBulletWorld::getCollisionObjectByName(const irr::core::stringc& name) const
{
    core::list<ICollisionObject*>::ConstIterator it = collisionObjects.begin();

    for(; it != collisionObjects.end(); it++)
    {
        ICollisionObject* obj = (*it);
        if(obj->getName() == name)
            return obj;

    }

    return 0;
}

ICollisionCallbackInformation* const irrBulletWorld::getCollisionCallback(irr::u32 index) const
{
    ICollisionCallbackInformation *callback = new ICollisionCallbackInformation(dispatcher->getManifoldByIndexInternal(index), (irrBulletWorld*)this);
    return callback;
}


bool irrBulletWorld::isGImpactEnabled() const
{
    return gimpactEnabled;
}


irrBulletWorld::~irrBulletWorld()
{
    printf("-- irrBullet: Freeing memory --\n");

    // remove constraints
    /*for (u32 i = 0; i < world->getNumConstraints(); i++)
	{
		btTypedConstraint* constraint = world->getConstraint(i);
		if(constraint)
		{
		    printf("irrBullet: Removing constraint\n");
            world->removeConstraint(constraint);
            delete constraint;
		}
	}*/

	core::list<ILiquidBody*>::Iterator wbit = liquidBodies.begin();

    for(; wbit != liquidBodies.end(); )
    {
        ILiquidBody *liquidBody = (*wbit);
        if(liquidBody)
        {
            delete liquidBody;
            liquidBody = 0;
        }

        wbit = liquidBodies.erase(wbit);
    }



    // remove the raycast vehicles
    core::list<IRaycastVehicle*>::Iterator rvit = raycastVehicles.begin();

    for(; rvit != raycastVehicles.end(); )
    {
        IRaycastVehicle *vehicle = (*rvit);
        if(vehicle)
        {
            getPointer()->removeVehicle(vehicle->getPointer());
            delete vehicle;
            vehicle = 0;
        }

        rvit = raycastVehicles.erase(rvit);
    }

    // Remove the collision objects
    /*core::list<ICollisionObject*>::Iterator rbit = collisionObjects.begin();

    for(; rbit != collisionObjects.end(); )
    {
        if(*rbit)
        {
            delete (*rbit);
            (*rbit) = 0;

            rbit = collisionObjects.erase(rbit);
        }
    }*/
    while(getNumCollisionObjects() > 0)
    {
        removeCollisionObject(getCollisionObject(0));
    }



    if(debug != 0)
        delete debug;


    if(world)
        delete world;

    delete constraintSolver;
    delete pairCache;
    delete dispatcher;
    delete collisionConfiguration;
    printf("-- irrBullet: Finished freeing memory --\n");
}

