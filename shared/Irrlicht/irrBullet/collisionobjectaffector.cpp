// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include <iostream>
#include "collisionobjectaffector.h"
#include "irrbullet_compile_config.h"

ICollisionObjectAffector::ICollisionObjectAffector()
{
    #ifdef IRRBULLET_DEBUG_MODE
        printf("irrBullet: Creating object affector\n");
    #endif

    DebugDraw = false;
    HasFinished = false;
}

ICollisionObjectAffector::~ICollisionObjectAffector()
{
    #ifdef IRRBULLET_DEBUG_MODE
        printf("irrBullet: Deleting object affector\n");
    #endif
}
