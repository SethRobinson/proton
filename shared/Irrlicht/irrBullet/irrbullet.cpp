// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "irrbullet.h"


irrBulletWorld *createIrrBulletWorld(irr::IrrlichtDevice* const device, bool useGImpact, bool useDebugDrawer)
{
    irrBulletWorld *world = new irrBulletWorld(device, useGImpact, useDebugDrawer);

    return world;
}
