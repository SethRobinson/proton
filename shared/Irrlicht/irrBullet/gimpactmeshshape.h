// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __G_IMPACT_MESH_SHAPE_H_INCLUDED__
#define __G_IMPACT_MESH_SHAPE_H_INCLUDED__

#include "irrbulletcommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "trianglemeshshape.h"


/// The GImpact mesh shape is a triangle mesh shape for concave collision detection for dynamic objects.
/*!
    This is useful for accurate shapes, such as the car frame in
    a racing game, or the aircraft body in a flight simulator.
*/
class IGImpactMeshShape : public ITriangleMeshShape
{
    public:
        IGImpactMeshShape(irr::scene::ISceneNode *n, irr::scene::IMesh* collMesh, irr::f32 m);
        virtual ~IGImpactMeshShape();

        void updateBound();


    protected:
        btGImpactMeshShape* gimpactShape;
        virtual void createShape(irr::scene::IMesh* const collMesh);
};

#endif // __G_IMPACT_MESH_SHAPE_H_INCLUDED__
