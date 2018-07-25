/*
irrBullet Bullet physics wrapper for the Irrlicht rendering engine.
Copyright (C) 2009-2011 Josiah H. (Skyreign Software)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Josiah Hartzell fighterstw@hotmail.com
Irrlicht Engine - http://www.irrlicht.sf.net
Bullet Physics Engine - http://www.bulletphysics.com/

The above copyright notice and its accompanying information may not be removed.

The same applies for all other copyright notices at the top of files included with this library.
*/

#ifndef __IRR_BULLET_H_INCLUDED__
#define __IRR_BULLET_H_INCLUDED__

#include "irrbullet_compile_config.h"
#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/btBulletCollisionCommon.h"
#include "Bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "irrbulletcommon.h"
#include "bulletworld.h"
#include "boxshape.h"
#include "sphereshape.h"
#include "gimpactmeshshape.h"
#include "bvhtrianglemeshshape.h"
#include "convexhullshape.h"
#include "motionstate.h"
#include "raycastvehicle.h"
#include "collisionobjectaffector.h"
#include "collisionobjectaffectordelete.h"
#include "collisionobjectaffectorattract.h"
//#include "collisionobjectaffectorbuoyancy.h"
#include "collisioncallbackinformation.h"
#include "liquidbody.h"


/*!
    @mainpage irrBullet 0.1.7 wrapper documentation


    @section intro Introduction

    Welcome to the irrBullet physics wrapper documentation.

    Here you will find all the information you will need to integrate physics into
    your application using irrBullet, which wraps the Bullet physics library with
    the Irrlicht rendering engine and provides many extensions not found in Bullet or other Bullet-Irrlicht wrappers.
    irrBullet is easy to use, has a similar programming
    style to Irrlicht's style, and is powerful. It also adds features that
    are game-specific to let the physics programmer focus on doing the really fun part of physics,
    not worrying about how to integrate, structure, and optimize it.

    If you have any questions or suggestions, please email them to me at fighterstw@hotmail.com.


    @section irrbulletexample irrBullet Example

    Using irrBullet with Irrlicht is very straight-forward and simple.

    Here's a simple integration (from 0.1):

    @code
    #include <irrlicht.h>
    #include <irrbullet.h>

    using namespace irr;
    using namespace core;
    using namespace scene;
    using namespace video;

    int main()
    {
        IrrlichtDevice *device = createDevice( video::EDT_OPENGL, dimension2d<u32>(640, 480), 16, false, false, false, 0);

        if (device == 0)
            return 1;

        device->setWindowCaption(L"irrBullet Example");

        // Create the irrBullet world
        world = createIrrBulletWorld(device, true, true);

        world->setDebugMode(EPDM_DrawAabb |
            EPDM_DrawContactPoints);


        // Create a static triangle mesh object
        IMeshSceneNode *Node = smgr->addMeshSceneNode(smgr->getMesh("terrainMain.b3d")->getMesh(0));
        Node->setPosition(vector3df(0,0,0));
        Node->setMaterialFlag(video::EMF_LIGHTING, false);
        Node->getMesh()->setHardwareMappingHint(EHM_STATIC);

        // For the terrain, instead of adding a cube or sphere shape, we are going to
        // add a BvhTriangleMeshShape. This is the standard trimesh shape
        // for static objects. The first parameter is of course the node to control,
        // the second parameter is the collision mesh, incase you want a low-poly collision mesh,
        // and the third parameter is the mass.
        ICollisionShape *shape = new IBvhTriangleMeshShape(Node, smgr->getMesh("terrainMain.b3d"), 0.0);

        shape->setMargin(0.07);

        // The rigid body will be placed at the origin of the node that the collision shape is controlling,
        // so we do not need to set the position of the rigid body after creating it.
        IRigidBody *terrain = world->addRigidBody(shape);
        terrain->setGravity(vector3df(0,0,0));


        // This will scale both the collision object and the scene node it controls.
        shape->setLocalScaling(vector3df(4,4,4), ESP_BOTH);




        // When setting a rigid body to a static object, please be sure that you have
        // that object's mass set to 0.0. Otherwise, undesired results will occur.
        terrain->setCollisionFlags(ECF_STATIC_OBJECT);


        u32 TimeStamp = device->getTimer()->getTime(), DeltaTime = 0;


        while(device->run())
        {
            driver->beginScene(true, true, SColor(255,100,101,140));

            DeltaTime = device->getTimer()->getTime() - TimeStamp;
            TimeStamp = device->getTimer()->getTime();

            // Step the simulation with our delta time
            world->stepSimulation(DeltaTime*0.001f, 120);

            // Draw the 3d debugging data.
            world->debugDrawWorld(true);

            // This call will draw the technical properties of the physics simulation
            // to the GUI environment.
            world->debugDrawProperties(true);


            smgr->drawAll();
            env->drawAll();

            driver->endScene();

        }
        // We're done with the IrrBullet world, so we free the memory that it takes up.
        delete world;
        device->drop();

        return 0;
    }
    @endcode

    That's it! You now have a basic irrBullet example with a static triangle mesh.

    irrBullet is capable of a lot more than what is shown in this example,
    but it should give you a good idea of how to get started.

    Example source code and projects can be found in (<irrBulletDir> / examples),
    and compiled examples can be found in (<irrBulletDir> / bin / win32_gcc).


    @section linkerandsearchdirs Linker and Search Directories Settings

    Before you can begin integrating irrBullet in your application, there are a few things
    to add to your project first.

    Link against these static libraries (included in (<irrBulletDir> / lib), pre-compiled):

        <b>libirrBullet.a
        libbulletdynamics.a
        libbulletsoftbody.a
        libGIMPACTUtils.a
        liblinearmath.a
        libbulletcollision.a</b>

    This is for most features and with GImpact included. Other libs must be linked against for some features,
    such as libconvexdecomposition.a for decomposing objects.

    <i>It is important that they are linked against in this order to avoid linker errors.</i>


    Next, add these directories to your Search Directories list:

        <b>
        <irrBulletDirectory> / source
        <irrBulletDirectory> / source / bheaders
        <irrBulletDirectory> / source / bheaders / bullet
        </b>

    Then just include irrBullet.h into the top of your files using irrBullet.


    You are now ready to begin the integration process!

*/

irrBulletWorld *createIrrBulletWorld(irr::IrrlichtDevice* const device, bool useGImpact = false, bool useDebugDrawer = false);

#endif // __I_BULLET_WORLD_H_INCLUDED__



