// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "collisionobjectaffectordelete.h"
#include "bulletworld.h"
#include "collisionobject.h"
#include <vector3d.h>

ICollisionObjectAffectorDelete::ICollisionObjectAffectorDelete(irr::u32 delay)
{
    EndTime = 0;
    TimeDelay = delay;
    FirstRun = true;
    Type = ECOAT_DELETE_AFFECTOR;
}

void ICollisionObjectAffectorDelete::affectObject(ICollisionObject* object, irr::u32 timeMS)
{
    if(FirstRun == true)
    {
        EndTime = timeMS + TimeDelay;
        FirstRun = false;
    }

    else
    if(timeMS >= EndTime)
    {
        HasFinished = true;

        object->getDynamicsWorld()->addToDeletionQueue(object);
    }
}

ICollisionObjectAffectorDelete::~ICollisionObjectAffectorDelete()
{
    //dtor
}
