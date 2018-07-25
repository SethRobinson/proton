// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __COLLISION_OBJECT_AFFECTOR_H__
#define __COLLISION_OBJECT_AFFECTOR_H__

#include <irrTypes.h>

class ICollisionObject;

enum ECollisionObjectAffectorType
{
    ECOAT_DELETE_AFFECTOR,
    ECOAT_ATTRACT_AFFECTOR,
    ECOAT_AFFECTOR_COUNT
};


/*!
    The base class for all collision object affectors. Derive from this class to create your own affectors.
*/
class ICollisionObjectAffector
{
    public:
        ICollisionObjectAffector();
        virtual ~ICollisionObjectAffector();

        virtual void affectObject(ICollisionObject* object, irr::u32 timeMS) = 0;

        void setDebugDrawing(bool b) { DebugDraw = b; };

		bool hasFinished() const { return HasFinished; };

		ECollisionObjectAffectorType getAffectorType() const { return Type; };

    protected:
        ECollisionObjectAffectorType Type;
        bool HasFinished;
        bool DebugDraw;
};

#endif // __COLLISION_OBJECT_AFFECTOR_H__
